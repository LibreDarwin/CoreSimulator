# Copyright (C) 2026, LibreDarwin
# SPDX-License-Identifier: BSD-3-Clause
# Open-source reimplementation of CoreSimulator.framework and simctl
#
# Build layout: every artifact lives under build/; final tools go to
# build/release/ or build/debug/ per CONFIG.
#
# Portable to both GNU make and BSD make (bmake): no pattern rules, no
# ifeq/ifdef/.if conditionals and no $(if)/$(shell) functions.  Per-config
# flags come from make/<CONFIG>.mk so both make variants behave identically.

CONFIG ?= release
SDK    ?= /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
CC     := /Users/sunneva/xnuports-root/devel/xcode-tools/build/release/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang

-include make/$(CONFIG).mk

BUILD_DIR := build/$(CONFIG)
OBJDIR    := $(BUILD_DIR)/obj

CFLAGS := $(OPT) -std=c11 -D_DARWIN_C_SOURCE -isysroot "$(SDK)" -Isources/simctl -Wall -Wextra

SIMCTL_BIN := $(BUILD_DIR)/simctl
SIMCTL_OBJS := $(OBJDIR)/sim_list.o $(OBJDIR)/sim_list_dispatch.o $(OBJDIR)/sim_ops.o $(OBJDIR)/simctl.o

PREFIX  ?= /usr/local
DESTDIR ?=

all: $(SIMCTL_BIN)

$(SIMCTL_BIN): $(SIMCTL_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(SIMCTL_OBJS)

$(OBJDIR)/simctl.o: Sources/simctl/simctl.c Sources/simctl/simctl.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ Sources/simctl/simctl.c

$(OBJDIR)/sim_list.o: Sources/simctl/sim_list.c Sources/simctl/simctl.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ Sources/simctl/sim_list.c

$(OBJDIR)/sim_list_dispatch.o: Sources/simctl/sim_list_dispatch.c Sources/simctl/simctl.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ Sources/simctl/sim_list_dispatch.c

$(OBJDIR)/sim_ops.o: Sources/simctl/sim_ops.c Sources/simctl/simctl.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ Sources/simctl/sim_ops.c

install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 $(SIMCTL_BIN) $(DESTDIR)$(PREFIX)/bin/simctl

clean:
	rm -rf build

.PHONY: all install clean
