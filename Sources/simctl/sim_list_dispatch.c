/*
 * list_dispatch - wrapper for the simctl list subcommand.
 *
 * Parses the list subcommand's arguments (in the same style as
 * getopt_long) and calls the appropriate list function. The full
 * upstream argument grammar is:
 *
 *   simctl list [-j | --json] [--json-fd=<fd>] [--json-output=<file path>]
 *               [-e | --no-escape-slashes] [-v]
 *               [devices|devicetypes|runtimes|pairs] [<search term>|available]
 *
 * Copyright (c) 2026 Sunneva N. Mariu <sunnevanattsol@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "simctl.h"

/* Exit status used by simctl for usage errors. */
#define LIST_USAGE_EXIT 117

void
list_usage(FILE *out)
{
	fputs(
"List available devices, device types, runtimes, or device pairs.\n"
"Usage: simctl list [-j | --json] [--json-fd=<fd>] [--json-output=<file path>] [-e | --no-escape-slashes] [-v] [devices|devicetypes|runtimes|pairs] [<search term>|available]\n"
"-j     Print as JSON (optionally specify a file descriptor with --json-fd or file path with --json-output)\n"
"-e     Encode slashes without escapes in JSON output\n"
"-v     More verbose output\n"
"\n"
"Specify one of 'devices', 'devicetypes', 'runtimes', or 'pairs' to list only items of that type. If a type filter is specified you may also specify a search term. Search terms use a simple case-insensitive contains check against the item's description. You may use the search term 'available' to only list available items.\n"
"\n"
	    , out);
}

static int
is_list_type(const char *s)
{
	return strcmp(s, "devices") == 0 ||
	    strcmp(s, "devicetypes") == 0 ||
	    strcmp(s, "runtimes") == 0 ||
	    strcmp(s, "pairs") == 0;
}

static void
list_type_dispatch(const char *type, int json_mode, int verbose,
    const char *term, int escape)
{
	int available_only = term != NULL &&
	    strcmp(type, "devices") == 0 &&
	    strcmp(term, "available") == 0;

	if (strcmp(type, "devices") == 0) {
		list_devices(json_mode, verbose, term, available_only,
		    escape);
	} else if (strcmp(type, "devicetypes") == 0) {
		if (!json_mode)
			printf("== Device Types ==\n");
		list_devicetypes(json_mode, term, verbose, escape);
	} else if (strcmp(type, "runtimes") == 0) {
		if (!json_mode)
			printf("== Runtimes ==\n");
		list_runtimes(json_mode, term, escape);
	} else if (strcmp(type, "pairs") == 0) {
		if (!json_mode)
			printf("== Device Pairs ==\n");
		list_pairs(json_mode, term, escape);
	}
}

int
list_dispatch(int argc, char **argv)
{
	static struct option longopts[] = {
		{ "json",              no_argument,       NULL, 'j' },
		{ "json-fd",           required_argument, NULL, 0x100 },
		{ "json-output",       required_argument, NULL, 0x101 },
		{ "no-escape-slashes", no_argument,       NULL, 'e' },
		{ NULL,                0,                 NULL, 0 }
	};
	int json_mode = 0;
	int verbose = 0;
	int escape = 1;
	int json_fd = -1;
	const char *json_output = NULL;
	const char *list_type = NULL;
	const char *term = NULL;
	int c;

	/*
	 * argv[0] is the "list" subcommand; options begin at index 1.
	 * getopt_long produces the same diagnostics as the reference
	 * implementation (including the BSD-style `` `opt' `` quoting).
	 */
	optind = 1;
	while ((c = getopt_long(argc, argv, "jev", longopts, NULL)) != -1) {
		switch (c) {
		case 'j':
			json_mode = 1;
			break;
		case 'e':
			escape = 0;
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
			list_usage(stderr);
			return LIST_USAGE_EXIT;
		}
	}

	for (int i = optind; i < argc; i++) {
		const char *arg = argv[i];

		if (list_type == NULL && is_list_type(arg)) {
			list_type = arg;
		} else if (term == NULL) {
			term = arg;
		} else {
			/* Too many positional arguments. */
			list_usage(stderr);
			return LIST_USAGE_EXIT;
		}
	}

	/*
	 * Redirect JSON output if requested. --json-output creates (and
	 * truncates) the target file unconditionally, mirroring the
	 * reference implementation; the JSON document is only written
	 * there when JSON mode is active. --json-fd redirects the JSON
	 * stream when JSON mode is active.
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

	/*
	 * A search term without a type, or no arguments at all, is
	 * handled specially. A bare "list" prints every section; a
	 * term without a type is a usage error.
	 */
	if (list_type == NULL) {
		if (term != NULL) {
			list_usage(stderr);
			return LIST_USAGE_EXIT;
		}

		/*
		 * Bare "list": sections are emitted in the framework's
		 * canonical order, which differs from the argument order
		 * used elsewhere. In JSON mode all four sections are
		 * folded into a single object.
		 */
		if (json_mode) {
			printf("{\n");
			list_devicetypes(LIST_JSON_MERGED, NULL, verbose, escape);
			printf(",\n");
			list_runtimes(LIST_JSON_MERGED, NULL, escape);
			printf(",\n");
			list_devices(LIST_JSON_MERGED, verbose, NULL, 0,
			    escape);
			printf(",\n");
			list_pairs(LIST_JSON_MERGED, NULL, escape);
			printf("\n}\n");
			return 0;
		}

		printf("== Device Types ==\n");
		list_devicetypes(LIST_PLAIN, NULL, verbose, escape);
		printf("== Runtimes ==\n");
		list_runtimes(LIST_PLAIN, NULL, escape);
		printf("== Devices ==\n");
		list_devices(LIST_PLAIN, verbose, NULL, 0, escape);
		printf("== Device Pairs ==\n");
		list_pairs(LIST_PLAIN, NULL, escape);
		return 0;
	}

	if (!json_mode && strcmp(list_type, "devices") == 0)
		printf("== Devices ==\n");
	list_type_dispatch(list_type, json_mode, verbose, term, escape);
	return 0;
}
