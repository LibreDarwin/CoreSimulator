/*
 * simctl - open source reimplementation of Apple's simctl(1).
 *
 * Copyright (c) 2026 Sunneva N. Mariu <sunnevanattsol@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Command line utility to control the Simulator. Implements list, create,
 * boot, shutdown, delete, erase, install, uninstall, spawn, io, and
 * other subcommands. Read-only commands (list) are implemented natively
 * by reading CoreSimulator plist data; destructive/operational commands
 * delegate to the system simctl binary when available.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#include "simctl.h"

#include "sim_help.inc"

/* Help text matching real simctl's help output */
static void
simctl_help(void)
{
	printf(
"usage: simctl [--set <path>] [--profiles <path>] <subcommand> ...\n"
"       simctl help [subcommand]\n"
"Command line utility to control the Simulator\n"
"\n"
"For subcommands that require a <device> argument, you may specify a device UDID\n"
"or the special \"booted\" string which will cause simctl to pick a booted device.\n"
"If multiple devices are booted when the \"booted\" device is selected, simctl\n"
"will choose one of them.\n"
"\n"
"Subcommands:\n"
"	addmedia            Add photos, live photos, videos, or contacts to the library of a device.\n"
"	appinfo             Show information about an installed application.\n"
"	boot                Boot a device or device pair.\n"
"	clone               Clone an existing device.\n"
"	create              Create a new device.\n"
"	delete              Delete specified devices, unavailable devices, or all devices.\n"
"	diagnose            Collect diagnostic information and logs.\n"
"	erase               Erase a device's contents and settings.\n"
"	get_app_container   Print the path of the installed app's container\n"
"	getenv              Print an environment variable from a running device.\n"
"	help                Prints the usage for a given subcommand.\n"
"	icloud_sync         Trigger iCloud sync on a device.\n"
"	install             Install an app on a device.\n"
"	install_app_data    Install an xcappdata package to a device, replacing the current contents of the container.\n"
"	io                  Set up a device IO operation.\n"
"	keychain            Manipulate a device's keychain\n"
"	launch              Launch an application by identifier on a device.\n"
"	list                List available devices, device types, runtimes, or device pairs.\n"
"	listapps            Show the installed applications.\n"
"	location            Control a device's simulated location\n"
"	logverbose          enable or disable verbose logging for a device\n"
"	openurl             Open a URL in a device.\n"
"	pair                Create a new watch and phone pair.\n"
"	pair_activate       Set a given pair as active.\n"
"	pbcopy              Copy standard input onto the device pasteboard.\n"
"	pbpaste             Print the contents of the device's pasteboard to standard output.\n"
"	pbsync              Sync the pasteboard content from one pasteboard to another.\n"
"	personalization     Provides utility when working with personalization manifests\n"
"	privacy             Grant, revoke, or reset privacy and permissions\n"
"	push                Send a simulated push notification\n"
"	rename              Rename a device.\n"
"	runtime             Perform operations on runtimes\n"
"	shutdown            Shutdown a device.\n"
"	spawn               Spawn a process by executing a given executable on a device.\n"
"	status_bar          Set or clear status bar overrides\n"
"	terminate           Terminate an application by identifier on a device.\n"
"	ui                  Get or Set UI options\n"
"	uninstall           Uninstall an app from a device.\n"
"	unpair              Unpair a watch and phone pair.\n"
"	upgrade             Upgrade a device to a newer runtime.\n"
	);
}

/* Delegate to the system simctl binary */
static int
delegate_to_system(int argc, char **argv)
{
	char cmd[4096];
	char *simctl_bin = getenv("SIMCTL_BIN");

	if (simctl_bin == NULL) {
		/* Try to find the system simctl */
		const char *candidates[] = {
			"/usr/bin/simctl",
			"/Applications/Xcode.app/Contents/Developer/usr/bin/simctl",
			"/usr/local/bin/simctl",
			NULL
		};
		for (int i = 0; candidates[i]; i++) {
			if (access(candidates[i], X_OK) == 0) {
				simctl_bin = (char *)candidates[i];
				break;
			}
		}
	}

	if (simctl_bin == NULL) {
		fprintf(stderr, "simctl: operation requires Xcode simctl binary, "
		    "but it was not found.\n");
		return 1;
	}

	/* Build the command line */
	int pos = 0;
	pos += snprintf(cmd + pos, sizeof cmd - pos, "\"%s\"", simctl_bin);
	for (int i = 1; i < argc && pos < (int)sizeof cmd - 256; i++) {
		pos += snprintf(cmd + pos, sizeof cmd - pos, " \"%s\"", argv[i]);
	}

	int status = system(cmd);
	if (status == -1)
		return 1;
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	return 1;
}

int
main(int argc, char **argv)
{
	const char *subcommand = NULL;
	const char *set_path = NULL;
	const char *profiles_path = NULL;
	char **sub_args = argv;
	int sub_argc = argc;
	int opt_index = 1;

	/*
	 * Parse global options. Only long options (--set, --profiles,
	 * --help) are recognized here; an unrecognized or incomplete long
	 * option prints the help text. Tokens that do not begin with "--"
	 * are treated as the subcommand, even when they begin with a
	 * single dash, matching the reference implementation.
	 */
	while (opt_index < argc) {
		if (strcmp(argv[opt_index], "--set") == 0 &&
		    opt_index + 1 < argc) {
			set_path = argv[opt_index + 1];
			opt_index += 2;
		} else if (strcmp(argv[opt_index], "--profiles") == 0 &&
		    opt_index + 1 < argc) {
			profiles_path = argv[opt_index + 1];
			opt_index += 2;
		} else if (strcmp(argv[opt_index], "--help") == 0) {
			simctl_help();
			return 1;
		} else if (strncmp(argv[opt_index], "--", 2) == 0) {
			simctl_help();
			return 1;
		} else {
			/* First non-long-option argument is the subcommand */
			subcommand = argv[opt_index];
			break;
		}
	}

	/*
	 * The profiles directory is validated globally and before any
	 * subcommand is dispatched (even `help` and no-subcommand runs).
	 */
	if (profiles_path != NULL && access(profiles_path, F_OK) != 0) {
		fprintf(stderr, "Provided profiles path does not exist: %s\n",
		    profiles_path);
		return 1;
	}

	if (subcommand == NULL) {
		simctl_help();
		return 1;
	}

	/* Set up the arguments for the subcommand */
	sub_args = argv + opt_index;
	sub_argc = argc - opt_index;

	if (subcommand != NULL && strcmp(subcommand, "help") == 0) {
		/* `help` consumes the device set, so validate it here. */
		if (set_path != NULL && access(set_path, F_OK) != 0) {
			fprintf(stderr,
			    "Provided set path does not exist: %s\n",
			    set_path);
			return 1;
		}
		if (sub_argc <= 1) {
			simctl_help();
			return 0;
		}

		{
			const char *sub = sub_args[1];
			const char *text;

			if (strcmp(sub, "list") == 0) {
				list_usage(stderr);
				return 0;
			}
			if (strcmp(sub, "runtime") == 0) {
				runtime_usage(stderr);
				return 0;
			}

			text = sim_help_lookup(sub);
			if (text != NULL) {
				fputs(text, stderr);
				return 0;
			}

			/* Unknown subcommand: the help-for-help usage, rc 117. */
			fputs(sim_help_lookup("help"), stderr);
			return 117;
		}
	}

	/* Dispatch to subcommands */
	if (strcmp(subcommand, "list") == 0) {
		/*
		 * A custom profiles directory is not modelled natively, so
		 * delegate to the reference implementation (the path has
		 * already been validated above).
		 */
		if (profiles_path != NULL)
			return delegate_to_system(argc, argv);

		/*
		 * `--set <path>` selects the device directory. The
		 * reference implementation rejects a missing path before
		 * doing any work; otherwise <path> replaces the default
		 * per-user devices directory.
		 */
		if (set_path != NULL) {
			if (access(set_path, F_OK) != 0) {
				fprintf(stderr,
				    "Provided set path does not exist: %s\n",
				    set_path);
				return 1;
			}
			list_set_device_root(set_path);
		}
		return list_dispatch(sub_argc, sub_args);
	} else if (strcmp(subcommand, "runtime") == 0) {
		return sim_runtime(argc, argv);
	}

	/*
	 * Every other subcommand requires the CoreSimulator service, so
	 * delegate to the system simctl binary. This covers the
	 * documented operational commands and the undocumented ones
	 * (for example notify_post, notify_get_state, addphoto, pbinfo,
	 * bootstatus, darwinup, legacyruntime, keyboard, and monitor).
	 * For an unknown subcommand the system binary prints the
	 * canonical "Unrecognized subcommand" message. Delegating the
	 * full argv preserves global options such as --set/--profiles.
	 */
	return delegate_to_system(argc, argv);
}
