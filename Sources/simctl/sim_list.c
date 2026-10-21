/*
 * sim_list - simctl list subcommand implementation.
 *
 * Lists devices, device types, runtimes, or pairs from the
 * CoreSimulator framework data on disk.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

#include "simctl.h"
#include "sim_plist.h"
#include "simdevicetype_order.inc"

static int dt_order_index(const char *identifier);
static int dev_order_index(const char *identifier);

/*
 * Case-insensitive substring check, matching the "simple case-insensitive
 * contains" search semantics documented for `simctl list`.
 */
static int
str_contains_ci(const char *haystack, const char *needle)
{
	if (needle == NULL || needle[0] == '\0')
		return 1;
	if (haystack == NULL)
		return 0;

	for (const char *h = haystack; *h != '\0'; h++) {
		const char *a = h;
		const char *b = needle;
		while (*a != '\0' && *b != '\0' &&
		    tolower((unsigned char)*a) ==
		    tolower((unsigned char)*b)) {
			a++;
			b++;
		}
		if (*b == '\0')
			return 1;
	}
	return 0;
}

const char *state_names[] = {
    [CS_STATE_UNKNOWN]  = "Unknown",
    [CS_STATE_SHUTDOWN] = "Shutdown",
    [CS_STATE_BOOTED]   = "Booted",
    [CS_STATE_CREATING] = "Creating",
    [CS_STATE_DELETING] = "Deleting",
};

/* Find the home directory */
static const char *
home_dir(void)
{
	static char path[1024];
	const char *h = getenv("HOME");
	if (h == NULL) {
		struct passwd *pw = getpwuid(getuid());
		if (pw)
			h = pw->pw_dir;
	}
	if (h != NULL) {
		snprintf(path, sizeof path, "%s", h);
		return path;
	}
	return ".";
}

/* Extract username from home path */
static const char *
home_user(void)
{
	static char user[256];
	const char *h = home_dir();
	const char *slash = strrchr(h, '/');
	if (slash && slash[1] != '\0') {
		snprintf(user, sizeof user, "%s", slash + 1);
		return user;
	}
	return "user";
}

/* Extract OS display name from runtime identifier.
 * e.g., com.apple.CoreSimulator.SimRuntime.iOS-26-5 -> "iOS 26.5" */
static void
runtime_os_display(const char *rt, char *out, size_t outsz)
{
	const char *prefix = "com.apple.CoreSimulator.SimRuntime.";
	out[0] = '\0';
	if (strncmp(rt, prefix, strlen(prefix)) != 0)
		return;
	const char *rest = rt + strlen(prefix);
	int major, minor;
	if (sscanf(rest, "iOS-%d-%d", &major, &minor) == 2)
		snprintf(out, outsz, "iOS %d.%d", major, minor);
	else if (sscanf(rest, "tvOS-%d-%d", &major, &minor) == 2)
		snprintf(out, outsz, "tvOS %d.%d", major, minor);
	else if (sscanf(rest, "watchOS-%d-%d", &major, &minor) == 2)
		snprintf(out, outsz, "watchOS %d.%d", major, minor);
	else if (sscanf(rest, "xrOS-%d-%d", &major, &minor) == 2)
		snprintf(out, outsz, "xrOS %d.%d", major, minor);
	else if (sscanf(rest, "visionOS-%d-%d", &major, &minor) == 2)
		snprintf(out, outsz, "visionOS %d.%d", major, minor);
	else
		snprintf(out, outsz, "%s", rest);
}

/*
 * Runtime profile directories to search when determining whether a
 * runtime identifier refers to an installed runtime. CoreSimulator
 * installs runtime bundles into the system profile directory, into a
 * per-user profile directory, and (for mounted disk images) into the
 * Volumes hierarchy.
 */
static const char *
runtime_search_dirs(void)
{
	static char buf[4096];
	buf[0] = '\0';

	/*
	 * The list is rebuilt on each call because the home directory
	 * can differ between invocations only in tests; keep it simple.
	 */
	char home_runtimes[1024];
	snprintf(home_runtimes, sizeof home_runtimes,
	    "%s/Library/Developer/CoreSimulator/Profiles/Runtimes",
	    home_dir());

	snprintf(buf, sizeof buf,
	    "/Library/Developer/CoreSimulator/Profiles/Runtimes\n"
	    "%s",
	    home_runtimes);
	return buf;
}

/*
 * Determine whether a runtime identifier corresponds to an installed
 * runtime bundle. Returns 1 when a matching .simruntime bundle with the
 * given CFBundleIdentifier is found, 0 otherwise.
 */
static int
runtime_is_available(const char *runtime_id)
{
	if (runtime_id == NULL || runtime_id[0] == '\0')
		return 0;

	const char *dirs = runtime_search_dirs();
	char dirlist[4096];
	snprintf(dirlist, sizeof dirlist, "%s", dirs);

	char *saveptr = NULL;
	for (char *dir = strtok_r(dirlist, "\n", &saveptr); dir != NULL;
	    dir = strtok_r(NULL, "\n", &saveptr)) {
		DIR *rdir = opendir(dir);
		if (rdir == NULL)
			continue;

		struct dirent *entry;
		int found = 0;
		while ((entry = readdir(rdir)) != NULL) {
			if (entry->d_name[0] == '.')
				continue;
			if (strstr(entry->d_name, ".simruntime") == NULL)
				continue;

			char info_path[2048];
			snprintf(info_path, sizeof info_path,
			    "%s/%s/Contents/Info.plist", dir, entry->d_name);

			char *id = plist_read_string(info_path,
			    "CFBundleIdentifier");
			if (id != NULL) {
				if (strcmp(id, runtime_id) == 0)
					found = 1;
				free(id);
			}
			if (found)
				break;
		}
		closedir(rdir);
		if (found)
			return 1;
	}
	return 0;
}

/*
 * Escape a string for inclusion in a JSON string literal. Mirrors the
 * behaviour of NSJSONSerialization, which escapes the forward slash as
 * well as the usual backslash and double quote. Pass escape_slashes == 0
 * to emulate simctl's -e / --no-escape-slashes option.
 */
static char *
json_escape(const char *s, int escape_slashes)
{
	if (s == NULL)
		s = "";

	size_t len = strlen(s);
	char *out = malloc(len * 2 + 1);
	if (out == NULL)
		return NULL;

	char *p = out;
	for (const char *q = s; *q != '\0'; q++) {
		switch (*q) {
		case '"':
			*p++ = '\\';
			*p++ = '"';
			break;
		case '\\':
			*p++ = '\\';
			*p++ = '\\';
			break;
		case '/':
			if (escape_slashes) {
				*p++ = '\\';
				*p++ = '/';
			} else {
				*p++ = '/';
			}
			break;
		default:
			*p++ = *q;
			break;
		}
	}
	*p = '\0';
	return out;
}

/*
 * Encode a dotted runtime version (e.g. "26.0", "13.4.1") into the
 * normalized three-component string and the packed integer encoding
 * CoreSimulator uses (major << 16 | minor << 8 | patch).
 */
static void
version_encode(const char *version, char *out_str, size_t out_len,
    unsigned *out_num)
{
	unsigned parts[3] = { 0, 0, 0 };

	if (version != NULL) {
		int idx = 0;
		const char *p = version;
		while (idx < 3 && *p != '\0') {
			unsigned v = 0;
			while (*p >= '0' && *p <= '9') {
				v = v * 10 + (unsigned)(*p - '0');
				p++;
			}
			parts[idx++] = v;
			if (*p == '.')
				p++;
			else
				break;
		}
	}

	snprintf(out_str, out_len, "%u.%u.%u", parts[0], parts[1], parts[2]);
	if (out_num != NULL)
		*out_num = (parts[0] << 16) | (parts[1] << 8) | parts[2];
}

/*
 * Sum the allocated size, in bytes, of every regular file beneath a
 * directory tree. This matches what `du` reports and what CoreSimulator
 * exposes as a device's dataPathSize.
 */
static unsigned long long
dir_allocated_size(const char *path)
{
	DIR *dir = opendir(path);
	if (dir == NULL)
		return 0;

	unsigned long long total = 0;
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 ||
		    strcmp(entry->d_name, "..") == 0)
			continue;

		char child[4096];
		snprintf(child, sizeof child, "%s/%s", path, entry->d_name);

		struct stat st;
		if (lstat(child, &st) != 0)
			continue;
		if (S_ISDIR(st.st_mode))
			total += dir_allocated_size(child);
		else
			total += (unsigned long long)st.st_blocks * 512;
	}
	closedir(dir);
	return total;
}

/*
 * Decide whether a single device survives the optional search term or
 * one of the special filters. Apple treats the exact, case-sensitive
 * terms "available" and "unavailable" as magic filters that select
 * devices by availability; any other term is a case-insensitive
 * substring check against the device name or UDID.
 */
static int
device_match(const char *name, const char *udid, int available,
    const char *term, int available_only)
{
	if (available_only)
		return available;
	if (term == NULL)
		return 1;
	if (strcmp(term, "unavailable") == 0)
		return !available;
	return str_contains_ci(name, term) || str_contains_ci(udid, term);
}

/* --- list devices --- */

/*
 * Device directory override set by the top-level `--set <path>`
 * option. When non-NULL it replaces the default per-user devices
 * directory. `simctl --set <path>` uses <path> as the directory whose
 * immediate children are device bundles.
 */
static const char *device_root_override;

void
list_set_device_root(const char *path)
{
	device_root_override = path;
}

void
list_devices(int json_mode, int verbose, const char *term,
    int available_only, int escape)
{
	char dev_root[1024];
	if (device_root_override != NULL)
		snprintf(dev_root, sizeof dev_root, "%s",
		    device_root_override);
	else
		snprintf(dev_root, sizeof dev_root, "%s/%s",
		    home_dir(), CS_DEVICES_DIR);

	DIR *dir = opendir(dev_root);
	if (dir == NULL) {
		fprintf(stderr, "simctl: no CoreSimulator devices directory\n");
		return;
	}

	/* Collect devices into arrays for sorting */
	char *names[256];
	char *udids[256];
	char *device_types[256];
	char *runtimes[256];
	int states[256];
	char *states_str[256];
	int is_avail[256];
	int count = 0;

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;

		char plist_path[1024];
		snprintf(plist_path, sizeof plist_path,
		    "%s/%s/device.plist", dev_root, entry->d_name);

		struct stat st;
		if (stat(plist_path, &st) != 0)
			continue;
		if (!S_ISREG(st.st_mode))
			continue;

		char *name = plist_read_string(plist_path, "name");
		char *udid_str = plist_read_string(plist_path, "UDID");
		char *runtime = plist_read_string(plist_path, "runtime");
		char *dt = plist_read_string(plist_path, "deviceType");
		int state = plist_read_int(plist_path, "state");
		int deleted = plist_read_bool(plist_path, "isDeleted");

		if (name == NULL || udid_str == NULL) {
			free(name);
			free(udid_str);
			free(runtime);
			free(dt);
			continue;
		}

		/* Skip deleted devices unless verbose */
		if (deleted == 1 && !verbose) {
			free(name);
			free(udid_str);
			free(runtime);
			free(dt);
			continue;
		}

		if (count < 256) {
			names[count] = name;
			udids[count] = udid_str;
			runtimes[count] = runtime ? runtime : strdup("");
			device_types[count] = dt ? dt : strdup("");
			states[count] = state;
			is_avail[count] = (deleted == 0);
			states_str[count] = strdup(state_names[
			    state >= 0 && state < 5 ? state : 0]);
			count++;
		} else {
			free(name);
			free(udid_str);
			free(runtime);
			free(dt);
		}
	}
	closedir(dir);

	/*
	 * Sort by runtime, then by the device type's canonical order,
	 * then by name. Ordering devices by device type (rather than
	 * name) matches Apple's simctl, which lists each runtime's
	 * devices in the framework's device type order.
	 */
	for (int i = 0; i < count - 1; i++) {
		for (int j = 0; j < count - i - 1; j++) {
			int swap = 0;
			int rc = strcmp(runtimes[j], runtimes[j + 1]);
			if (rc > 0)
				swap = 1;
			else if (rc == 0) {
				int oi = dev_order_index(device_types[j]);
				int oj = dev_order_index(device_types[j + 1]);
				if (oi != oj)
					swap = oi > oj;
				else if (strcmp(names[j], names[j + 1]) > 0)
					swap = 1;
			}
			if (swap) {
				char *t;
				t = names[j]; names[j] = names[j + 1];
				    names[j + 1] = t;
				t = udids[j]; udids[j] = udids[j + 1];
				    udids[j + 1] = t;
				t = runtimes[j]; runtimes[j] = runtimes[j + 1];
				    runtimes[j + 1] = t;
				t = device_types[j]; device_types[j] =
				    device_types[j + 1];
				    device_types[j + 1] = t;
				t = states_str[j]; states_str[j] =
				    states_str[j + 1];
				    states_str[j + 1] = t;
				int ti = states[j]; states[j] = states[j + 1];
				    states[j + 1] = ti;
				int tia = is_avail[j]; is_avail[j] =
				    is_avail[j + 1];
				    is_avail[j + 1] = tia;
			}
		}
	}

	if (json_mode) {
		int merged = (json_mode == LIST_JSON_MERGED);
		printf("%s  \"devices\" : {\n", merged ? "" : "{\n");

		int printed_runtime = 0;
		for (int i = 0; i < count; i++) {
			if (runtimes[i] == NULL ||
			    runtimes[i][0] == '\0')
				continue;

			/* Only print each runtime once */
			int seen = 0;
			for (int j = 0; j < i; j++) {
				if (runtimes[j] &&
				    strcmp(runtimes[j], runtimes[i]) == 0) {
					seen = 1;
					break;
				}
			}
			if (seen)
				continue;

			int rt_avail = runtime_is_available(runtimes[i]);

			/*
			 * The special "available" filter keeps empty runtime
			 * groups; a search term drops any group it does not
			 * match.
			 */
			int emitted = available_only;
			if (!emitted) {
				for (int j = 0; j < count; j++) {
					if (runtimes[j] == NULL ||
					    strcmp(runtimes[j],
					    runtimes[i]) != 0)
						continue;
					if (device_match(names[j], udids[j],
					    is_avail[j] && rt_avail, term,
					    available_only)) {
						emitted = 1;
						break;
					}
				}
			}
			if (!emitted)
				continue;

			if (printed_runtime)
				printf(",\n");
			printf("    \"%s\" : [\n", runtimes[i]);
			printed_runtime = 1;

			int first = 1;
			for (int j = 0; j < count; j++) {
				if (runtimes[j] == NULL ||
				    strcmp(runtimes[j], runtimes[i]) != 0)
					continue;
				int available = is_avail[j] && rt_avail;
				if (!device_match(names[j], udids[j],
				    available, term, available_only))
					continue;
				if (!first)
					printf(",\n");
				first = 0;

				char data_dir[4096];
				snprintf(data_dir, sizeof data_dir,
				    "%s/%s/data", dev_root, udids[j]);

				char log_dir[4096];
				snprintf(log_dir, sizeof log_dir,
				    "/Users/%s/Library/Logs/CoreSimulator/%s",
				    home_user(), udids[j]);

				char *esc;

				printf("      {\n");
				if (!available) {
					printf("        \"availabilityError\" : "
					    "\"runtime profile not found "
					    "using \\\"System\\\" match "
					    "policy\",\n");
				}

				esc = json_escape(data_dir, escape);
				printf("        \"dataPath\" : \"%s\",\n",
				    esc ? esc : "");
				free(esc);

				printf("        \"dataPathSize\" : %llu,\n",
				    dir_allocated_size(data_dir));

				esc = json_escape(log_dir, escape);
				printf("        \"logPath\" : \"%s\",\n",
				    esc ? esc : "");
				free(esc);

				printf("        \"udid\" : \"%s\",\n", udids[j]);
				printf("        \"isAvailable\" : %s,\n",
				    available ? "true" : "false");
				esc = json_escape(device_types[j][0] ?
				    device_types[j] :
				    "com.apple.CoreSimulator.SimDeviceType.Unknown",
				    escape);
				printf("        \"deviceTypeIdentifier\" : "
				    "\"%s\",\n", esc ? esc : "");
				free(esc);
				printf("        \"state\" : \"%s\",\n",
				    states_str[j]);
				esc = json_escape(names[j], escape);
				printf("        \"name\" : \"%s\"\n",
				    esc ? esc : "");
				free(esc);
				printf("      }");
			}
			printf("\n    ]");
		}
		if (merged)
			printf("\n  }");
		else
			printf("\n  }\n}\n");
	} else {
		/* Non-JSON output, grouped by OS version */
		for (int i = 0; i < count; i++) {
			if (runtimes[i] == NULL ||
			    runtimes[i][0] == '\0')
				continue;

			/* Only print header once per runtime */
			int seen = 0;
			for (int j = 0; j < i; j++) {
				if (runtimes[j] &&
				    strcmp(runtimes[j], runtimes[i]) == 0) {
					seen = 1;
					break;
				}
			}
			if (seen)
				continue;

			/*
			 * When the runtime profile backing this group is
			 * not installed, simctl labels the group as
			 * unavailable and annotates each device.
			 */
			int rt_avail = runtime_is_available(runtimes[i]);

			/*
			 * The special "available" filter keeps empty runtime
			 * groups; a search term drops any group it does not
			 * match.
			 */
			int emitted = available_only;
			if (!emitted) {
				for (int j = 0; j < count; j++) {
					if (runtimes[j] == NULL ||
					    strcmp(runtimes[j],
					    runtimes[i]) != 0)
						continue;
					if (device_match(names[j], udids[j],
					    is_avail[j] && rt_avail, term,
					    available_only)) {
						emitted = 1;
						break;
					}
				}
			}
			if (!emitted)
				continue;

			if (rt_avail) {
				/* Extract OS display name from runtime id */
				char os_display[64];
				runtime_os_display(runtimes[i], os_display,
				    sizeof os_display);
				printf("-- %s --\n",
				    os_display[0] ? os_display : "Unknown");
			} else {
				printf("-- Unavailable: %s --\n", runtimes[i]);
			}

			for (int j = 0; j < count; j++) {
				if (runtimes[j] == NULL ||
				    strcmp(runtimes[j], runtimes[i]) != 0)
					continue;
				if (!device_match(names[j], udids[j],
				    is_avail[j] && rt_avail, term,
				    available_only))
					continue;
				if (rt_avail)
					printf("    %s (%s) (%s)\n",
					    names[j], udids[j], states_str[j]);
				else
					printf("    %s (%s) (%s) "
					    "(unavailable, runtime profile "
					    "not found using \"System\" "
					    "match policy)\n",
					    names[j], udids[j], states_str[j]);
			}
		}
	}

	for (int i = 0; i < count; i++) {
		free(names[i]);
		free(udids[i]);
		free(runtimes[i]);
		free(device_types[i]);
		free(states_str[i]);
	}
}

/* --- list device types --- */

/*
 * Map a CoreSimulator product family identifier to the display name
 * used by `list devicetypes` search. The identifiers are fixed by
 * CoreSimulator; the iPod touch reports family 1 (iPhone).
 */
static const char *
family_name_for_id(int id)
{
	switch (id) {
	case 1:
		return "iPhone";
	case 2:
		return "iPad";
	case 3:
		return "Apple TV";
	case 4:
		return "Apple Watch";
	case 7:
		return "Apple Vision";
	default:
		return "";
	}
}

/*
 * Infer the product family reported by CoreSimulator from a device
 * name. Note that Apple folds the iPod touch into the "iPhone"
 * family, and reports Apple Vision separately.
 */
static const char *
product_family(const char *name)
{
	if (strncmp(name, "iPhone", 6) == 0)
		return "iPhone";
	if (strncmp(name, "iPod", 4) == 0)
		return "iPhone";
	if (strncmp(name, "iPad", 4) == 0)
		return "iPad";
	if (strncmp(name, "Apple TV", 8) == 0)
		return "Apple TV";
	if (strncmp(name, "Apple Watch", 11) == 0)
		return "Apple Watch";
	if (strncmp(name, "Apple Vision", 12) == 0)
		return "Apple Vision";
	return "Other";
}

/*
 * Rank a product family in the order used by simctl when listing
 * device types: iPhone, iPad, Apple TV, Apple Watch, Apple Vision,
 * then anything unknown.
 */
static int
family_rank(const char *family)
{
	if (strcmp(family, "iPhone") == 0)
		return 0;
	if (strcmp(family, "iPad") == 0)
		return 1;
	if (strcmp(family, "Apple TV") == 0)
		return 2;
	if (strcmp(family, "Apple Watch") == 0)
		return 3;
	if (strcmp(family, "Apple Vision") == 0)
		return 4;
	return 5;
}

/*
 * Return the position of a device type identifier within Apple's
 * canonical ordering, or kSimDeviceTypeOrderCount when the identifier
 * is not present (so unknown types sort after all known ones).
 */
static int
dt_order_index(const char *identifier)
{
	for (int i = 0; i < kSimDeviceTypeOrderCount; i++) {
		if (strcmp(kSimDeviceTypeOrder[i], identifier) == 0)
			return i;
	}
	return kSimDeviceTypeOrderCount;
}

/*
 * Return the position of a device type identifier within the observed
 * default-device ordering. Identifiers not present fall back to the
 * device type catalog order, offset past every entry in the device
 * table so table entries always sort first.
 */
static int
dev_order_index(const char *identifier)
{
	for (int i = 0; i < kSimDeviceDefaultOrderCount; i++) {
		if (strcmp(kSimDeviceDefaultOrder[i], identifier) == 0)
			return i;
	}
	return kSimDeviceDefaultOrderCount + dt_order_index(identifier);
}

void
list_devicetypes(int json_mode, const char *term, int verbose, int escape)
{
	char buf[1024];
	snprintf(buf, sizeof buf, "%s/DeviceTypes", CS_PROFILES_DIR);

	DIR *dir = opendir(buf);
	if (dir == NULL) {
		fprintf(stderr, "simctl: no device types directory\n");
		return;
	}

	struct dirent *entry;
	int count = 0;
	char *names[512];
	char *ids[512];
	char *bundles[512];
	const char *families[512];

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		if (strstr(entry->d_name, ".simdevicetype") == NULL)
			continue;

		char dt_path[1024];
		snprintf(dt_path, sizeof dt_path, "%s/%s", buf,
		    entry->d_name);

		char info_path[1024];
		snprintf(info_path, sizeof info_path,
		    "%s/Contents/Info.plist", dt_path);

		struct stat st;
		if (stat(info_path, &st) != 0 || !S_ISREG(st.st_mode))
			continue;

		char *name = plist_read_string(info_path, "CFBundleName");
		char *id = plist_read_string(info_path, "CFBundleIdentifier");

		/*
		 * The reference implementation's search term is matched
		 * against the device type's name, identifier, model
		 * identifier, and primary product family name (in that
		 * order, as a case-insensitive substring). It does not
		 * match against productClass, chromeIdentifier, or
		 * springBoardConfigName.
		 */
		char profile_path[1024];
		snprintf(profile_path, sizeof profile_path,
		    "%s/Contents/Resources/profile.plist", dt_path);
		char *model = plist_read_string(profile_path, "modelIdentifier");
		const char *fam_name =
		    family_name_for_id(plist_read_family_id(profile_path));

		int match = str_contains_ci(name, term) ||
		    str_contains_ci(id, term) ||
		    str_contains_ci(model, term) ||
		    str_contains_ci(fam_name, term);
		free(model);

		if (name && id && match && count < 512) {
			names[count] = name;
			ids[count] = id;
			bundles[count] = strdup(dt_path);
			families[count] = product_family(name);
			count++;
		} else {
			free(name);
			free(id);
		}
	}
	closedir(dir);

	/*
	 * Sort by Apple's canonical device type order. Identifiers not
	 * present in the table fall back to product family rank, then
	 * name.
	 */
	for (int i = 0; i < count - 1; i++) {
		for (int j = 0; j < count - i - 1; j++) {
			int swap = 0;
			int oi = dt_order_index(ids[j]);
			int oj = dt_order_index(ids[j + 1]);
			if (oi != oj)
				swap = oi > oj;
			else {
				int fc = family_rank(families[j]) -
				    family_rank(families[j + 1]);
				if (fc > 0)
					swap = 1;
				else if (fc == 0 &&
				    strcmp(names[j], names[j + 1]) > 0)
					swap = 1;
			}
			if (swap) {
				char *t;
				t = names[j]; names[j] = names[j + 1];
				    names[j + 1] = t;
				t = ids[j]; ids[j] = ids[j + 1];
				    ids[j + 1] = t;
				t = bundles[j]; bundles[j] = bundles[j + 1];
				    bundles[j + 1] = t;
				const char *tf = families[j];
				    families[j] = families[j + 1];
				    families[j + 1] = tf;
			}
		}
	}

	if (json_mode) {
		int merged = (json_mode == LIST_JSON_MERGED);
		printf("%s  \"devicetypes\" : [\n", merged ? "" : "{\n");
		for (int i = 0; i < count; i++) {
			char profile_path[1024];
			snprintf(profile_path, sizeof profile_path,
			    "%s/Contents/Resources/profile.plist", bundles[i]);

			char *model = plist_read_string(profile_path,
			    "modelIdentifier");
			char *minv = plist_read_string(profile_path,
			    "minRuntimeVersion");
			char *maxv = plist_read_string(profile_path,
			    "maxRuntimeVersion");

			char min_str[32], max_str[32];
			unsigned min_num = 0, max_num = 0;
			version_encode(minv, min_str, sizeof min_str, &min_num);
			if (maxv != NULL) {
				version_encode(maxv, max_str, sizeof max_str,
				    &max_num);
			} else {
				snprintf(max_str, sizeof max_str,
				    "65535.255.255");
				max_num = 4294967295u;
			}

			char *esc;

			printf("    {\n");
			esc = json_escape(families[i], escape);
			printf("      \"productFamily\" : \"%s\",\n",
			    esc ? esc : "");
			free(esc);
			esc = json_escape(bundles[i], escape);
			printf("      \"bundlePath\" : \"%s\",\n",
			    esc ? esc : "");
			free(esc);
			printf("      \"maxRuntimeVersion\" : %u,\n", max_num);
			printf("      \"maxRuntimeVersionString\" : \"%s\",\n",
			    max_str);
			esc = json_escape(ids[i], escape);
			printf("      \"identifier\" : \"%s\",\n",
			    esc ? esc : "");
			free(esc);
			esc = json_escape(model ? model : "", escape);
			printf("      \"modelIdentifier\" : \"%s\",\n",
			    esc ? esc : "");
			free(esc);
			printf("      \"minRuntimeVersionString\" : \"%s\",\n",
			    min_str);
			printf("      \"minRuntimeVersion\" : %u,\n", min_num);
			esc = json_escape(names[i], escape);
			printf("      \"name\" : \"%s\"\n", esc ? esc : "");
			free(esc);
			printf("    }%s\n", i < count - 1 ? "," : "");

			free(model);
			free(minv);
			free(maxv);
		}
		if (merged)
			printf("  ]");
		else
			printf("  ]\n}\n");
	} else {
		for (int i = 0; i < count; i++) {
			if (verbose)
				printf("%s (%s) [%s]\n", names[i], ids[i],
				    bundles[i]);
			else
				printf("%s (%s)\n", names[i], ids[i]);
		}
	}

	for (int i = 0; i < count; i++) {
		free(names[i]);
		free(ids[i]);
		free(bundles[i]);
	}
}

/* --- list runtimes --- */

void
list_runtimes(int json_mode, const char *term, int escape)
{
	(void)escape;

	DIR *vdir = opendir(CS_VOLUMES_DIR);
	if (vdir == NULL) {
		fprintf(stderr, "simctl: no CoreSimulator volumes\n");
		return;
	}

	char *names[256];
	char *versions[256];
	char *buildvs[256];
	char *ids[256];
	int count = 0;

	struct dirent *ventry;
	while ((ventry = readdir(vdir)) != NULL) {
		if (ventry->d_name[0] == '.')
			continue;

		/* Volume dir name is like iOS_23F73 */
		char vol_name[512];
		snprintf(vol_name, sizeof vol_name, "%s/%s",
		    CS_VOLUMES_DIR, ventry->d_name);

		/* Extract build version from dir name */
		char *buildver = NULL;
		char *underscore = strchr(ventry->d_name, '_');
		if (underscore) {
			buildver = strdup(underscore + 1);
		}

		/* Find the .simruntime bundle */
		char rt_search[512];
		snprintf(rt_search, sizeof rt_search,
		    "%s/Library/Developer/CoreSimulator/Profiles/Runtimes",
		    vol_name);

		DIR *rdir = opendir(rt_search);
		if (rdir == NULL) {
			free(buildver);
			continue;
		}

		struct dirent *rentry;
		while ((rentry = readdir(rdir)) != NULL) {
			if (rentry->d_name[0] == '.')
				continue;
			if (strstr(rentry->d_name, ".simruntime") == NULL)
				continue;

			char rt_path[512];
			snprintf(rt_path, sizeof rt_path,
			    "%s/%s", rt_search, rentry->d_name);

			char info_path[1024];
			snprintf(info_path, sizeof info_path,
			    "%s/Contents/Info.plist", rt_path);

			struct stat st;
			if (stat(info_path, &st) != 0 || !S_ISREG(st.st_mode))
				continue;

			char *rt_name = plist_read_string(info_path,
			    "CFBundleName");
			char *rt_id = plist_read_string(info_path,
			    "CFBundleIdentifier");

			if (rt_name && rt_id && str_contains_ci(rt_name,
			    term) && count < 256) {
				if (buildver == NULL)
					buildver = strdup("");

				/* Extract version number from CFBundleName
				 * e.g., "iOS 26.5" -> "26.5" */
				char version[64];
				char *sp = strchr(rt_name, ' ');
				if (sp)
					snprintf(version, sizeof version,
					    "%s", sp + 1);
				else
					snprintf(version, sizeof version,
					    "%s", rt_name);

				names[count] = rt_name;
				versions[count] = strdup(version);
				buildvs[count] = buildver;
				ids[count] = rt_id;
				buildver = NULL; /* transfer ownership */
				count++;
			} else {
				free(rt_name);
				free(rt_id);
			}
		}
		closedir(rdir);
		free(buildver);
	}
	closedir(vdir);

	/* Sort by build version for deterministic output */
	for (int i = 0; i < count - 1; i++) {
		for (int j = 0; j < count - i - 1; j++) {
			if (strcmp(buildvs[j], buildvs[j + 1]) > 0) {
				char *t;
				t = names[j]; names[j] = names[j + 1];
				    names[j + 1] = t;
				t = versions[j]; versions[j] = versions[j + 1];
				    versions[j + 1] = t;
				t = buildvs[j]; buildvs[j] = buildvs[j + 1];
				    buildvs[j + 1] = t;
				t = ids[j]; ids[j] = ids[j + 1];
				    ids[j + 1] = t;
			}
		}
	}

	if (json_mode) {
		int merged = (json_mode == LIST_JSON_MERGED);
		printf("%s  \"runtimes\" : [\n", merged ? "" : "{\n");
		if (count == 0)
			printf("\n");
		for (int i = 0; i < count; i++) {
			printf("    {\n");
			printf("      \"isAvailable\" : true,\n");
			printf("      \"version\" : \"%s\",\n", versions[i]);
			printf("      \"isInternal\" : false,\n");
			printf("      \"buildversion\" : \"%s\",\n", buildvs[i]);
			printf("      \"supportedArchitectures\" : [\n");
			printf("        \"arm64\"\n");
			printf("      ],\n");
			printf("      \"supportedDeviceTypes\" : [],\n");
			printf("      \"bundlePath\" : \"\",\n");
			printf("      \"identifier\" : \"%s\"\n", ids[i]);
			printf("    }%s\n", i < count - 1 ? "," : "");
		}
		if (merged)
			printf("  ]");
		else
			printf("  ]\n}\n");
	} else {
		for (int i = 0; i < count; i++) {
			printf("%s (%s - %s) - %s\n",
			    names[i], versions[i], buildvs[i], ids[i]);
		}
	}

	for (int i = 0; i < count; i++) {
		free(names[i]);
		free(versions[i]);
		free(buildvs[i]);
		free(ids[i]);
	}
}

/* --- list pairs --- */

void
list_pairs(int json_mode, const char *term, int escape)
{
	(void)term;
	(void)escape;

	if (json_mode) {
		if (json_mode == LIST_JSON_MERGED)
			printf("  \"pairs\" : {\n\n  }");
		else
			printf("{\n  \"pairs\" : {\n\n  }\n}\n");
	} else
		printf("");
}
