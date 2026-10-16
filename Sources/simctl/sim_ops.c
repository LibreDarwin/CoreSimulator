/*
 * sim_ops - delegation helpers for subcommands that require the
 * CoreSimulator service (create, boot, shutdown, install, io, ...).
 *
 * Those operations need the CoreSimulator framework's private XPC
 * service, which is not available to an open-source reimplementation,
 * so they are handed to the system simctl binary. The read-only
 * commands (list, runtime) are implemented natively elsewhere.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "simctl.h"

/*
 * Find and return the path to the system simctl binary.
 * Returns a static buffer, or NULL if not found.
 */
static const char *
find_system_simctl(void)
{
	static char path[1024];
	const char *candidates[] = {
		"/usr/bin/simctl",
		"/Applications/Xcode.app/Contents/Developer/usr/bin/simctl",
		NULL
	};

	for (int i = 0; candidates[i]; i++) {
		if (access(candidates[i], X_OK) == 0) {
			strlcpy(path, candidates[i], sizeof path);
			return path;
		}
	}
	return NULL;
}

/*
 * Delegate a command to the system simctl binary.
 *
 * argv must be the caller's full argument vector (argv[0] is the
 * simctl binary itself, argv[1] is the subcommand). subcommand is
 * used only for the diagnostic message when the system binary is
 * missing. The exit status of the delegated process is propagated.
 */
static int
delegate_to_system(int argc, char **argv, const char *subcommand)
{
	const char *simctl_bin = find_system_simctl();
	if (simctl_bin == NULL) {
		fprintf(stderr,
		    "simctl: %s requires the system simctl binary, "
		    "but it was not found.\n", subcommand);
		return 1;
	}

	char cmd[4096];
	int pos = snprintf(cmd, sizeof cmd, "\"%s\"", simctl_bin);

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

/*
 * Delegate a subcommand to the system simctl binary. argv is the
 * full argument vector; argv[1] names the subcommand. Used by
 * subcommand implementations that cannot yet run natively.
 */
int
sim_delegate(int argc, char **argv)
{
	return delegate_to_system(argc, argv,
	    (argc > 1 && argv[1] != NULL) ? argv[1] : "simctl");
}
