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
