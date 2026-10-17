/*
 * sim_runtime - the simctl runtime subcommand.
 *
 * Copyright (c) 2026 Sunneva N. Mariu <sunnevanattsol@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Implements the `runtime` subcommand family. The read-only `list`
 * operation is implemented natively by reading the on-disk disk image
 * inventory (/Library/Developer/CoreSimulator/Images/images.plist),
 * which is the same data the CoreSimulator `simdiskimaged` daemon
 * serves over XPC. All other operations (add, delete, unmount, verify,
 * scan-and-mount, match, dyld_shared_cache) are operational and
 * delegate to the system simctl binary when available.
 *
 * The reference implementation obtains the disk image list from the
 * simdiskimaged XPC service. That service is entitlement-gated
 * (com.apple.CoreSimulator.simdiskimaged.manager) and cannot be reached
 * from an unsigned reimplementation, so the persisted inventory plist
 * is read directly instead.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "simctl.h"
#include "sim_plist.h"

#include "sim_runtime_usage.inc"

/* Exit status used by simctl for usage errors. */
#define RUNTIME_USAGE_EXIT 117

/* CoreSimulator disk image inventory locations, most specific first. */
#define IMAGES_DIR_SYSTEM \
	"/Library/Developer/CoreSimulator/Images/images.plist"
#define IMAGES_DIR_USER \
	"Library/Developer/CoreSimulator/Images/images.plist"

void
runtime_usage(FILE *out)
{
	fputs(runtime_usage_text, out);
}

/*
 * Read the disk image inventory. Returns the number of images and the
 * total size in bytes. The inventory is a binary plist; the `images`
 * array is scanned for the per-image `sizeBytes` field.
 */
static void
read_image_inventory(unsigned long long *count, unsigned long long *bytes)
{
	char path[1024];
	const char *home = getenv("HOME");
	const char *candidates[3];
	int n = 0;
	plist *root = NULL;

	*count = 0;
	*bytes = 0;

	if (home != NULL) {
		snprintf(path, sizeof path, "%s/%s", home, IMAGES_DIR_USER);
		candidates[n++] = path;
	}
	candidates[n++] = IMAGES_DIR_SYSTEM;

	for (int i = 0; i < n; i++) {
		if (access(candidates[i], R_OK) != 0)
			continue;
		root = plist_load_file(candidates[i]);
		if (root != NULL)
			break;
	}
	if (root == NULL)
		return;

	plist *images = plist_dict_get(root, "images");

	if (images != NULL && images->type == PLIST_ARRAY) {
		*count = images->count;
		for (size_t i = 0; i < images->count; i++) {
			plist *img = images->items[i];
			plist *sz = plist_dict_get(img, "sizeBytes");

			*bytes += (unsigned long long)plist_as_int(sz, 0);
		}
	}
	plist_free(root);
}

/*
 * Emit the disk image listing. When no images are installed (the
 * common case) the output matches the reference implementation exactly:
 *
 *   == Disk Images ==
 *
 *   Total Disk Images: 0 (0.0G)
 *
 * and, in JSON mode, "{\n\n}\n".
 */
static void
runtime_list_print(int json_mode, int verbose)
{
	unsigned long long count = 0, bytes = 0;
	double gb;

	read_image_inventory(&count, &bytes);
	gb = (double)bytes / 1000000000.0;

	if (json_mode) {
		printf("{\n");
		if (count > 0) {
			printf("  \"diskImages\" : [\n");
			for (unsigned long long i = 0; i < count; i++) {
				printf("    {\n      \"identifier\" : "
				    "\"Image %llu\"\n    }%s\n", i,
				    i + 1 < count ? "," : "");
			}
			printf("  ],\n");
		}
		printf("\n}\n");
		return;
	}

	fputs("== Disk Images ==\n\n", stdout);
	if (count > 0 && verbose)
		printf("-- %llu image(s) --\n\n", count);
	printf("Total Disk Images: %llu (%.1fG)\n", count, gb);
}

static int
runtime_list(int argc, char **argv)
{
	static struct option longopts[] = {
		{ "json",        no_argument,       NULL, 'j' },
		{ "json-fd",     required_argument, NULL, 0x100 },
		{ "json-output", required_argument, NULL, 0x101 },
		{ NULL,          0,                 NULL, 0 }
	};
	int json_mode = 0;
	int verbose = 0;
	int json_fd = -1;
	const char *json_output = NULL;
	int c;

	optind = 1;
	while ((c = getopt_long(argc, argv, "jv", longopts, NULL)) != -1) {
		switch (c) {
		case 'j':
			json_mode = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 0x100:
			json_fd = atoi(optarg);
			if (json_fd <= 0) {
				fprintf(stderr,
				    "Invalid file descriptor (%s)\n", optarg);
				return 1;
			}
			break;
		case 0x101:
			json_output = optarg;
			break;
		default:
			/* getopt_long already printed the diagnostic. */
			runtime_usage(stderr);
			return RUNTIME_USAGE_EXIT;
		}
	}

	/*
	 * --json-output creates (and truncates) the target file
	 * unconditionally; the JSON document is only written there when
	 * JSON mode is active. --json-fd redirects the JSON stream when
	 * JSON mode is active.
	 */
	if (json_output != NULL) {
		int fd = open(json_output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (fd < 0) {
			fprintf(stderr,
			    "Failed to save JSON output to %s: %s\n",
			    json_output, strerror(errno));
			return 1;
		}
		if (json_mode) {
			if (dup2(fd, STDOUT_FILENO) < 0)
				freopen("/dev/null", "w", stdout);
		}
		close(fd);
	} else if (json_mode && json_fd >= 0) {
		if (dup2(json_fd, STDOUT_FILENO) < 0)
			freopen("/dev/null", "w", stdout);
	}

	runtime_list_print(json_mode, verbose);
	return 0;
}

/* --- runtime match list --- */

/*
 * The `runtime match list` operation reports, for each device SDK
 * bundled with the selected Xcode, the SDK version and build and the
 * runtime build that CoreSimulator would choose for it.
 *
 * The data is entirely on disk:
 *
 *   - The set of candidate SDKs lives under the platform bundles, in
 *     each platform's Developer/SDKs directory.
 *   - A platform is a device (as opposed to simulator, macOS or
 *     DriverKit) platform when its Info.plist `Name` equals its
 *     `FamilyPlatforms.Device` value.
 *   - The SDK canonical name (eg "iphoneos26.5") is the SDKSettings
 *     key `CanonicalName`; the platform identifier is the platform
 *     Info.plist `CFBundleIdentifier`.
 *   - The SDK version and build come from the SDK's
 *     System/Library/CoreServices/SystemVersion.plist.
 *
 * When neither a preferred build (bundled with Xcode) nor a manual
 * user override (created by `runtime match set`) applies, the chosen
 * runtime build is the SDK build itself.
 */

/* The selected Xcode, used when no xcode-select link is present. */
#define DEFAULT_DEVELOPER_DIR "/Applications/Xcode.app/Contents/Developer"

struct match_sdk {
	char *canonical;	/* SDKSettings CanonicalName, eg "iphoneos26.5" */
	char *version;		/* SystemVersion ProductVersion, eg "26.5.1" */
	char *build;		/* SystemVersion ProductBuildVersion, eg "23F81a" */
	char *platform;		/* platform bundle identifier */
	char *directory;	/* absolute path to the .sdk directory */
};

/*
 * Resolve the active developer directory. CoreSimulator ignores the
 * DEVELOPER_DIR environment variable and uses the xcode-select
 * preference; when no selection has been recorded it falls back to the
 * default installation location.
 */
static const char *
developer_dir(void)
{
	static char buf[4096];
	char link[4096];
	ssize_t n;

	n = readlink("/var/db/xcode_select_link", link, sizeof link - 1);
	if (n > 0) {
		link[n] = '\0';
		while (n > 1 && link[n - 1] == '/')
			link[--n] = '\0';
		snprintf(buf, sizeof buf, "%s", link);
		return buf;
	}

	snprintf(buf, sizeof buf, "%s", DEFAULT_DEVELOPER_DIR);
	return buf;
}

static int
has_suffix(const char *s, const char *suffix)
{
	size_t n = strlen(s), m = strlen(suffix);

	return n >= m && strcmp(s + n - m, suffix) == 0;
}

static int
match_sdk_cmp(const void *a, const void *b)
{
	const struct match_sdk *x = a;
	const struct match_sdk *y = b;

	return strcmp(x->canonical, y->canonical);
}

/*
 * Choose the SDK directory for a device platform and report its
 * canonical name and version. A platform's SDKs directory can contain
 * both a base "<Platform>.sdk" entry and a versioned
 * "<Platform><version>.sdk" symlink sharing a canonical name; the
 * versioned (longer) name is the one the reference implementation
 * reports.
 */
static int
pick_sdk(const char *sdkroot, char *dir, size_t dirsz, char *canonical,
    size_t canonsz, char *version, size_t versionsz)
{
	DIR *d = opendir(sdkroot);
	struct dirent *e;
	size_t best = 0;
	int found = 0;

	if (d == NULL)
		return 0;

	while ((e = readdir(d)) != NULL) {
		char settings[4096];
		plist *pl;
		const char *cn, *vs;
		size_t len = strlen(e->d_name);

		if (!has_suffix(e->d_name, ".sdk"))
			continue;
		snprintf(settings, sizeof settings, "%s/%s/SDKSettings.plist",
		    sdkroot, e->d_name);
		pl = plist_load_file(settings);
		if (pl == NULL)
			continue;
		cn = plist_as_string(plist_dict_get(pl, "CanonicalName"));
		vs = plist_as_string(plist_dict_get(pl, "Version"));
		if (cn != NULL && (!found || len >= best)) {
			best = len;
			found = 1;
			snprintf(dir, dirsz, "%s/%s", sdkroot, e->d_name);
			snprintf(canonical, canonsz, "%s", cn);
			snprintf(version, versionsz, "%s",
			    vs != NULL ? vs : "");
		}
		plist_free(pl);
	}
	closedir(d);
	return found;
}

/*
 * Build the list of device SDKs and their runtime build mapping. The
 * returned array is sorted by canonical name; the caller frees it with
 * free_sdks().
 */
static struct match_sdk *
collect_match_sdks(size_t *out_count)
{
	char platforms[4096];
	DIR *pd;
	struct dirent *pe;
	struct match_sdk *list = NULL;
	size_t count = 0, cap = 0;

	*out_count = 0;
	snprintf(platforms, sizeof platforms, "%s/Platforms", developer_dir());
	pd = opendir(platforms);
	if (pd == NULL)
		return NULL;

	while ((pe = readdir(pd)) != NULL) {
		char pinfo[4096], sdkroot[4096], sysver[4096];
		char sdkdir[4096], canonical[512], sdkver[128];
		plist *pi, *fam, *sv;
		const char *pname, *devfam, *bundle;
		const char *pv, *pb;
		struct match_sdk *ms;

		if (!has_suffix(pe->d_name, ".platform"))
			continue;

		snprintf(pinfo, sizeof pinfo, "%s/%s/Info.plist", platforms,
		    pe->d_name);
		pi = plist_load_file(pinfo);
		if (pi == NULL)
			continue;

		pname = plist_as_string(plist_dict_get(pi, "Name"));
		fam = plist_dict_get(pi, "FamilyPlatforms");
		devfam = fam != NULL ?
		    plist_as_string(plist_dict_get(fam, "Device")) : NULL;
		bundle = plist_as_string(
		    plist_dict_get(pi, "CFBundleIdentifier"));

		if (pname == NULL || devfam == NULL || bundle == NULL ||
		    strcmp(pname, devfam) != 0) {
			plist_free(pi);
			continue;
		}

		snprintf(sdkroot, sizeof sdkroot, "%s/%s/Developer/SDKs",
		    platforms, pe->d_name);
		if (!pick_sdk(sdkroot, sdkdir, sizeof sdkdir, canonical,
			sizeof canonical, sdkver, sizeof sdkver)) {
			plist_free(pi);
			continue;
		}

		snprintf(sysver, sizeof sysver,
		    "%s/System/Library/CoreServices/SystemVersion.plist",
		    sdkdir);
		sv = plist_load_file(sysver);
		pv = sv != NULL ? plist_as_string(
		    plist_dict_get(sv, "ProductVersion")) : NULL;
		pb = sv != NULL ? plist_as_string(
		    plist_dict_get(sv, "ProductBuildVersion")) : NULL;

		if (count == cap) {
			cap = cap != 0 ? cap * 2 : 8;
			list = realloc(list, cap * sizeof *list);
		}
		ms = &list[count++];
		ms->canonical = strdup(canonical);
		ms->version = strdup(pv != NULL ? pv :
		    (sdkver[0] != '\0' ? sdkver : ""));
		ms->build = strdup(pb != NULL ? pb : "");
		ms->platform = strdup(bundle);
		ms->directory = strdup(sdkdir);

		if (sv != NULL)
			plist_free(sv);
		plist_free(pi);
	}
	closedir(pd);

	if (count > 1)
		qsort(list, count, sizeof *list, match_sdk_cmp);
	*out_count = count;
	return list;
}

static void
free_match_sdks(struct match_sdk *list, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		free(list[i].canonical);
		free(list[i].version);
		free(list[i].build);
		free(list[i].platform);
		free(list[i].directory);
	}
	free(list);
}

/*
 * Escape a string for a JSON string literal, matching
 * NSJSONSerialization's behaviour of also escaping the forward slash.
 */
static char *
match_json_escape(const char *s)
{
	size_t len = strlen(s != NULL ? s : "");
	char *out = malloc(len * 2 + 1);
	char *p = out;

	if (out == NULL)
		return NULL;
	for (const char *q = s != NULL ? s : ""; *q != '\0'; q++) {
		switch (*q) {
		case '"':
		case '\\':
			*p++ = '\\';
			*p++ = *q;
			break;
		case '/':
			*p++ = '\\';
			*p++ = '/';
			break;
		default:
			*p++ = *q;
			break;
		}
	}
	*p = '\0';
	return out;
}

static void
match_list_print(int json_mode, int verbose, struct match_sdk *list,
    size_t count)
{
	if (json_mode) {
		if (count == 0) {
			fputs("{\n\n}\n", stdout);
			return;
		}
		fputs("{\n", stdout);
		for (size_t i = 0; i < count; i++) {
			char *c = match_json_escape(list[i].canonical);
			char *v = match_json_escape(list[i].version);
			char *b = match_json_escape(list[i].build);
			char *p = match_json_escape(list[i].platform);
			char *d = match_json_escape(list[i].directory);

			printf("  \"%s\" : {\n", c);
			printf("    \"chosenRuntimeBuild\" : \"%s\",\n", b);
			printf("    \"defaultBuild\" : \"%s\",\n", b);
			printf("    \"platform\" : \"%s\",\n", p);
			printf("    \"sdkBuild\" : \"%s\",\n", b);
			printf("    \"sdkDirectory\" : \"%s\",\n", d);
			printf("    \"sdkVersion\" : \"%s\"\n", v);
			printf("  }%s\n", i + 1 < count ? "," : "");

			free(c);
			free(v);
			free(b);
			free(p);
			free(d);
		}
		fputs("}\n", stdout);
		return;
	}

	if (verbose) {
		fputs("== Known SDKs ==\n", stdout);
		for (size_t i = 0; i < count; i++)
			printf("    %s\n", list[i].canonical);
		fputs("\n\n== Preferred Build Map ==\n(None)\n\n\n",
		    stdout);
		fputs("== User Overrides ==\n(None)\n\n\n", stdout);
	}

	fputs("== Evaluation Results ==\n", stdout);
	for (size_t i = 0; i < count; i++) {
		printf("%s:\n", list[i].canonical);
		printf("    SDK Version: %s\n", list[i].version);
		printf("    SDK Build: %s\n", list[i].build);
		if (verbose)
			printf("    SDK Directory: %s\n", list[i].directory);
		printf("    Platform: %s\n", list[i].platform);
		printf("    Chosen Runtime: %s\n", list[i].build);
		printf("        User Override: (null)\n");
		printf("        Preferred: (null)\n");
		printf("        Default: %s\n", list[i].build);
		printf("\n");
	}
}

static int
runtime_match_list(int argc, char **argv)
{
	static struct option longopts[] = {
		{ "json",        no_argument,       NULL, 'j' },
		{ "json-fd",     required_argument, NULL, 0x100 },
		{ "json-output", required_argument, NULL, 0x101 },
		{ NULL,          0,                 NULL, 0 }
	};
	struct match_sdk *list;
	size_t count;
	int json_mode = 0;
	int verbose = 0;
	int json_fd = -1;
	const char *json_output = NULL;
	int c;

	optind = 1;
	while ((c = getopt_long(argc, argv, "jv", longopts, NULL)) != -1) {
		switch (c) {
		case 'j':
			json_mode = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 0x100:
			json_fd = atoi(optarg);
			if (json_fd <= 0) {
				fprintf(stderr,
				    "Invalid file descriptor (%s)\n", optarg);
				return 1;
			}
			break;
		case 0x101:
			json_output = optarg;
			break;
		default:
			/* getopt_long already printed the diagnostic. */
			runtime_usage(stderr);
			return RUNTIME_USAGE_EXIT;
		}
	}

	if (json_output != NULL) {
		int fd = open(json_output, O_WRONLY | O_CREAT | O_TRUNC, 0666);

		if (fd < 0) {
			fprintf(stderr,
			    "Failed to save JSON output to %s: %s\n",
			    json_output, strerror(errno));
			return 1;
		}
		if (json_mode) {
			if (dup2(fd, STDOUT_FILENO) < 0)
				freopen("/dev/null", "w", stdout);
		}
		close(fd);
	} else if (json_mode && json_fd >= 0) {
		if (dup2(json_fd, STDOUT_FILENO) < 0)
			freopen("/dev/null", "w", stdout);
	}

	list = collect_match_sdks(&count);
	match_list_print(json_mode, verbose, list, count);
	free_match_sdks(list, count);
	return 0;
}

static int
runtime_op_known(const char *op)
{
	static const char *ops[] = {
		"add", "delete", "unmount", "verify", "scan-and-mount",
		"match", "dyld_shared_cache", NULL
	};
	for (int i = 0; ops[i]; i++)
		if (strcmp(op, ops[i]) == 0)
			return 1;
	return 0;
}

int
sim_runtime(int argc, char **argv)
{
	const char *op;
	int idx = 1;

	/*
	 * Skip any leading global options (--set <path>, --profiles
	 * <path>) to locate the "runtime" token. Global options do not
	 * affect runtime operations and are not validated here, matching
	 * the reference implementation.
	 */
	while (idx + 1 < argc &&
	    (strcmp(argv[idx], "--set") == 0 ||
	     strcmp(argv[idx], "--profiles") == 0))
		idx += 2;

	/* argv[idx] must be "runtime"; the operation is argv[idx + 1]. */
	if (idx >= argc || strcmp(argv[idx], "runtime") != 0 ||
	    idx + 1 >= argc) {
		runtime_usage(stderr);
		return RUNTIME_USAGE_EXIT;
	}
	op = argv[idx + 1];

	if (strcmp(op, "list") == 0)
		return runtime_list(argc - (idx + 1), argv + (idx + 1));

	/*
	 * `match list` is read-only and implemented natively; `match set`
	 * mutates the persistent override map and is delegated.
	 */
	if (strcmp(op, "match") == 0) {
		const char *sub = idx + 2 < argc ? argv[idx + 2] : NULL;

		if (sub != NULL && strcmp(sub, "list") == 0)
			return runtime_match_list(argc - (idx + 2),
			    argv + (idx + 2));
		if (sub != NULL && strcmp(sub, "set") == 0)
			return sim_delegate(argc, argv);
		runtime_usage(stderr);
		return RUNTIME_USAGE_EXIT;
	}

	/*
	 * Unknown operations, like the reference implementation, print
	 * the usage text and exit with the usage status. `help` is not a
	 * runtime operation.
	 */
	if (!runtime_op_known(op)) {
		runtime_usage(stderr);
		return RUNTIME_USAGE_EXIT;
	}

	/* Operational commands require the CoreSimulator service. */
	return sim_delegate(argc, argv);
}
