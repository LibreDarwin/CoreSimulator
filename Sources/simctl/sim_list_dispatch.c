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

/*
 * Canonical section order for the type filter. The reference
 * implementation prints matching sections in this fixed order
 * regardless of the order the types were named on the command line.
 */
enum {
	LT_DEVICETYPES = 0,
	LT_RUNTIMES,
	LT_DEVICES,
	LT_PAIRS,
	LT_COUNT
};

/*
 * JSON key order. The reference implementation builds its JSON document
 * from a hash map keyed by section name, so the order of the "devicetypes",
 * "runtimes", "devices" and "pairs" keys depends only on which sections
 * are present (not on argument order). Plain output, by contrast, always
 * uses the fixed canonical order above. This table records the observed
 * key order for each of the 16 possible section subsets, indexed by a
 * bitmask (devicetypes=1, runtimes=2, devices=4, pairs=8); unused slots
 * are -1.
 */
static const signed char json_section_order[16][4] = {
	[0]  = { -1, -1, -1, -1 },
	[1]  = { LT_DEVICETYPES, -1, -1, -1 },
	[2]  = { LT_RUNTIMES, -1, -1, -1 },
	[4]  = { LT_DEVICES, -1, -1, -1 },
	[8]  = { LT_PAIRS, -1, -1, -1 },
	[3]  = { LT_DEVICETYPES, LT_RUNTIMES, -1, -1 },
	[5]  = { LT_DEVICETYPES, LT_DEVICES, -1, -1 },
	[9]  = { LT_DEVICETYPES, LT_PAIRS, -1, -1 },
	[6]  = { LT_DEVICES, LT_RUNTIMES, -1, -1 },
	[10] = { LT_PAIRS, LT_RUNTIMES, -1, -1 },
	[12] = { LT_DEVICES, LT_PAIRS, -1, -1 },
	[7]  = { LT_DEVICETYPES, LT_RUNTIMES, LT_DEVICES, -1 },
	[11] = { LT_DEVICETYPES, LT_RUNTIMES, LT_PAIRS, -1 },
	[13] = { LT_DEVICETYPES, LT_DEVICES, LT_PAIRS, -1 },
	[14] = { LT_DEVICES, LT_RUNTIMES, LT_PAIRS, -1 },
	[15] = { LT_DEVICETYPES, LT_RUNTIMES, LT_DEVICES, LT_PAIRS }
};

/*
 * Return the section index for a type name, or -1 if the name is not a
 * recognized type.
 */
static int
list_type_index(const char *s)
{
	if (strcmp(s, "devicetypes") == 0)
		return LT_DEVICETYPES;
	if (strcmp(s, "runtimes") == 0)
		return LT_RUNTIMES;
	if (strcmp(s, "devices") == 0)
		return LT_DEVICES;
	if (strcmp(s, "pairs") == 0)
		return LT_PAIRS;
	return -1;
}

int
list_dispatch(int argc, char **argv)
{
	static struct option longopts[] = {
		{ "json",              no_argument,       NULL, 'j' },
		{ "json-fd",           required_argument, NULL, 0x100 },
		{ "json-output",       required_argument, NULL, 0x101 },
		{ "no-escape-slashes", no_argument,       NULL, 'e' },
		{ "verbose",           no_argument,       NULL, 'v' },
		{ NULL,                0,                 NULL, 0 }
	};
	int json_mode = 0;
	int verbose = 0;
	int escape = 1;
	int json_fd = -1;
	const char *json_output = NULL;
	const char *term = NULL;
	int selected[LT_COUNT] = { 0, 0, 0, 0 };
	int any_type = 0;
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

	/*
	 * Positional arguments select sections and (optionally) a search
	 * term. A type name adds its section to the filter; the first
	 * positional must be a type, and at most one non-type token (the
	 * search term) may follow it. Any other shape is a usage error.
	 */
	for (int i = optind; i < argc; i++) {
		const char *arg = argv[i];
		int idx = list_type_index(arg);

		if (idx >= 0) {
			selected[idx] = 1;
			any_type = 1;
		} else if (!any_type || term != NULL) {
			list_usage(stderr);
			return LIST_USAGE_EXIT;
		} else {
			term = arg;
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
	 * With no type filter every section is selected (a bare "list").
	 * Otherwise only the named sections are printed, in the fixed
	 * canonical order rather than argument order.
	 */
	int show_types = !any_type || selected[LT_DEVICETYPES];
	int show_runtimes = !any_type || selected[LT_RUNTIMES];
	int show_devices = !any_type || selected[LT_DEVICES];
	int show_pairs = !any_type || selected[LT_PAIRS];
	int available_only = show_devices && term != NULL &&
	    strcmp(term, "available") == 0;

	if (json_mode) {
		int mask = 0;
		if (show_types)
			mask |= 1;
		if (show_runtimes)
			mask |= 2;
		if (show_devices)
			mask |= 4;
		if (show_pairs)
			mask |= 8;

		int printed = 0;
		printf("{\n");
		for (int k = 0; k < 4; k++) {
			int sec = json_section_order[mask][k];
			if (sec < 0)
				continue;
			if (printed)
				printf(",\n");
			if (sec == LT_DEVICETYPES)
				list_devicetypes(LIST_JSON_MERGED, term,
				    verbose, escape);
			else if (sec == LT_RUNTIMES)
				list_runtimes(LIST_JSON_MERGED, term, escape);
			else if (sec == LT_DEVICES)
				list_devices(LIST_JSON_MERGED, verbose, term,
				    available_only, escape);
			else
				list_pairs(LIST_JSON_MERGED, term, escape);
			printed++;
		}
		printf("\n}\n");
		return 0;
	}

	if (show_types) {
		printf("== Device Types ==\n");
		list_devicetypes(LIST_PLAIN, term, verbose, escape);
	}
	if (show_runtimes) {
		printf("== Runtimes ==\n");
		list_runtimes(LIST_PLAIN, term, escape);
	}
	if (show_devices) {
		printf("== Devices ==\n");
		list_devices(LIST_PLAIN, verbose, term, available_only,
		    escape);
	}
	if (show_pairs) {
		printf("== Device Pairs ==\n");
		list_pairs(LIST_PLAIN, term, escape);
	}
	return 0;
}
