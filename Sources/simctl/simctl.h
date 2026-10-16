/*
 * simctl - open source reimplementation of Apple's simctl(1).
 *
 * Copyright (c) 2026 Sunneva N. Mariu <sunnevanattsol@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Command line utility to control the Simulator. The read-only list
 * and runtime commands are implemented natively by reading
 * CoreSimulator plist data; all other subcommands delegate to the
 * system simctl binary, which provides the CoreSimulator service.
 */

#define SIMCTL_VERSION "0.1.0"

#include <stdio.h>

/* CoreSimulator data paths */
#define CS_DEVICES_DIR    "Library/Developer/CoreSimulator/Devices"
#define CS_PROFILES_DIR   "/Library/Developer/CoreSimulator/Profiles"
#define CS_VOLUMES_DIR    "/Library/Developer/CoreSimulator/Volumes"

/* State values from CoreSimulator device.plist (SimDeviceState) */
#define CS_STATE_UNKNOWN      0
#define CS_STATE_SHUTDOWN     1
#define CS_STATE_BOOTED       2
#define CS_STATE_CREATING     3
#define CS_STATE_DELETING     4

extern const char *state_names[];

/* Dispatch functions (implemented in simctl.c / sim_list.c / sim_ops.c) */
extern int  list_dispatch(int argc, char **argv);
extern int  sim_delegate(int argc, char **argv);

/* runtime subcommand (implemented in sim_runtime.c) */
extern int  sim_runtime(int argc, char **argv);
extern void runtime_usage(FILE *out);

/* List functions (implemented in sim_list.c / sim_list_dispatch.c) */
/*
 * Output modes accepted by the list_* functions. LIST_JSON emits a
 * self-contained JSON document; LIST_JSON_MERGED emits just the
 * section (key plus value) so the caller can fold several sections
 * into a single JSON object, as `simctl list -j` does.
 */
#define LIST_PLAIN        0
#define LIST_JSON         1
#define LIST_JSON_MERGED  2

extern void list_usage(FILE *out);
extern void list_devices(int json_mode, int verbose, const char *term,
    int available_only, int escape);
extern void list_devicetypes(int json_mode, const char *term, int verbose,
    int escape);
extern void list_runtimes(int json_mode, const char *term, int escape);
extern void list_pairs(int json_mode, const char *term, int escape);

/*
 * Override the directory scanned for devices. `simctl --set <path>`
 * treats <path> as the directory whose immediate children are device
 * bundles; this replaces the default
 * $HOME/Library/Developer/CoreSimulator/Devices root.
 */
extern void list_set_device_root(const char *path);
