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
#include <errno.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>

#include "simctl.h"

extern char **environ;

/*
 * Find and return the path to the system simctl binary.
 * Returns a static buffer, or NULL if not found.
 *
 * The framework binary is preferred over the Xcode shim: the shim
 * only sets DEVELOPER_DIR and re-execs the framework binary (after a
 * version check that may try to run xcodebuild), so invoking the
 * framework binary directly yields identical behaviour without the
 * extra indirection.
 */
static const char *
find_system_simctl(void)
{
	static char path[1024];
	const char *candidates[] = {
		"/Library/Developer/PrivateFrameworks/CoreSimulator.framework"
		    "/Versions/A/Resources/bin/simctl",
		"/Applications/Xcode.app/Contents/Developer/usr/bin/simctl",
		"/usr/bin/simctl",
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
 *
 * The child is spawned directly (no shell) so argument quoting is
 * irrelevant and the delegated process's stderr and exit status are
 * reproduced verbatim.
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

	char **child_argv = calloc((size_t)argc + 1, sizeof(char *));
	if (child_argv == NULL) {
		fprintf(stderr, "simctl: out of memory\n");
		return 1;
	}

	child_argv[0] = (char *)simctl_bin;
	for (int i = 1; i < argc; i++)
		child_argv[i] = argv[i];
	child_argv[argc] = NULL;

	pid_t pid;
	int spawn_err = posix_spawn(&pid, simctl_bin, NULL, NULL,
	    child_argv, environ);
	free(child_argv);

	if (spawn_err != 0) {
		fprintf(stderr, "simctl: %s\n", strerror(spawn_err));
		return 1;
	}

	int status = 0;
	while (waitpid(pid, &status, 0) == -1) {
		if (errno != EINTR)
			return 1;
	}
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	if (WIFSIGNALED(status))
		kill(getpid(), WTERMSIG(status));
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
