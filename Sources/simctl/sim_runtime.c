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

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "simctl.h"

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
 * total size in bytes. The inventory is a binary plist; plutil(8) is
 * used to extract the `images` array as JSON, which is then scanned
 * for the per-image `identifier` and `sizeBytes` fields.
 */
static void
read_image_inventory(unsigned long long *count, unsigned long long *bytes)
{
	char path[1024];
	const char *home = getenv("HOME");
	const char *candidates[3];
	int n = 0;
	FILE *fp;
	char buf[65536];
	size_t got;

	*count = 0;
	*bytes = 0;

	if (home != NULL) {
		snprintf(path, sizeof path, "%s/%s", home, IMAGES_DIR_USER);
		candidates[n++] = path;
	}
	candidates[n++] = IMAGES_DIR_SYSTEM;

	for (int i = 0; i < n; i++) {
		char cmd[1200];
		if (access(candidates[i], R_OK) != 0)
			continue;
		snprintf(cmd, sizeof cmd,
		    "plutil -extract images json -o - \"%s\" 2>/dev/null",
		    candidates[i]);
		fp = popen(cmd, "r");
		if (fp == NULL)
			continue;
		got = fread(buf, 1, sizeof buf - 1, fp);
		pclose(fp);
		buf[got] = '\0';
		break;
	}
	if (got == 0)
		return;

	/*
	 * Count objects in the array. Every disk image record carries an
	 * `identifier` key; summing its occurrences across the extracted
	 * array yields the image count.
	 */
	for (const char *p = buf; (p = strstr(p, "\"identifier\"")) != NULL;
	    p += 12)
		(*count)++;

	for (const char *p = buf; (p = strstr(p, "\"sizeBytes\"")) != NULL;
	    p += 11) {
		const char *q = p + 11;
		while (*q == ' ' || *q == '\t' || *q == ':')
			q++;
		if (*q >= '0' && *q <= '9')
			*bytes += strtoull(q, NULL, 10);
	}
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

	/*
	 * argv[0] is the simctl binary and argv[1] is "runtime"; the
	 * operation is argv[2].
	 */
	if (argc < 3) {
		runtime_usage(stderr);
		return RUNTIME_USAGE_EXIT;
	}
	op = argv[2];

	if (strcmp(op, "list") == 0)
		return runtime_list(argc - 2, argv + 2);

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
