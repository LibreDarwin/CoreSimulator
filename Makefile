# Copyright (C) 2026, LibreDarwin
# SPDX-License-Identifier: BSD-3-Clause

CONFIG ?= release
SDK    ?= /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
CC     := /Users/sunneva/xnuports-root/devel/xcode-tools/build/release/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang

-include make/$(CONFIG).mk

BUILD_DIR := build/$(CONFIG)
OBJDIR    := $(BUILD_DIR)/obj

CFLAGS_SIMCTL := $(OPT) -std=c11 -D_DARWIN_C_SOURCE -isysroot "$(SDK)" -Isources/simctl -Wall -Wextra
CFLAGS_FRAME  := $(OPT) -fobjc-arc -isysroot "$(SDK)" -I$(shell pwd)/Sources/CoreSimulator -Wall -Wextra

SIMCTL_BIN := $(BUILD_DIR)/simctl
SIMCTL_OBJS := $(OBJDIR)/sim_list.o $(OBJDIR)/sim_list_dispatch.o $(OBJDIR)/sim_ops.o $(OBJDIR)/simctl.o

FRAMEWORK_DIR := $(BUILD_DIR)/CoreSimulator.framework
FRAME_OBJS := $(OBJDIR)/CoreSimulator.o $(OBJDIR)/SimServiceContext.o $(OBJDIR)/SimDeviceSet.o $(OBJDIR)/SimDevice.o $(OBJDIR)/SimDeviceType.o $(OBJDIR)/SimRuntime.o

all: $(SIMCTL_BIN) $(FRAMEWORK_DIR)

$(SIMCTL_BIN): $(SIMCTL_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS_SIMCTL) -o $@ $(SIMCTL_OBJS)

$(OBJDIR)/simctl.o: Sources/simctl/simctl.c Sources/simctl/simctl.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS_SIMCTL) -c -o $@ Sources/simctl/simctl.c

$(OBJDIR)/sim_list.o: Sources/simctl/sim_list.c Sources/simctl/simctl.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS_SIMCTL) -c -o $@ Sources/simctl/sim_list.c

$(OBJDIR)/sim_list_dispatch.o: Sources/simctl/sim_list_dispatch.c Sources/simctl/simctl.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS_SIMCTL) -c -o $@ Sources/simctl/sim_list_dispatch.c

$(OBJDIR)/sim_ops.o: Sources/simctl/sim_ops.c Sources/simctl/simctl.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS_SIMCTL) -c -o $@ Sources/simctl/sim_ops.c

$(FRAMEWORK_DIR): $(FRAME_OBJS) Sources/CoreSimulator/Info.plist Sources/CoreSimulator/CoreSimulator.h Sources/CoreSimulator/module.modulemap
	@mkdir -p $(FRAMEWORK_DIR)/Versions/A/Headers $(FRAMEWORK_DIR)/Versions/A/Modules $(FRAMEWORK_DIR)/Versions/A/Resources
	$(CC) $(CFLAGS_FRAME) -dynamiclib -o $(FRAMEWORK_DIR)/Versions/A/CoreSimulator $(FRAME_OBJS) -framework Foundation
	install -m 644 Sources/CoreSimulator/Info.plist $(FRAMEWORK_DIR)/Versions/A/Resources/Info.plist
	install -m 644 Sources/CoreSimulator/CoreSimulator.h $(FRAMEWORK_DIR)/Versions/A/Headers/
	install -m 644 Sources/CoreSimulator/SimServiceContext.h $(FRAMEWORK_DIR)/Versions/A/Headers/
	install -m 644 Sources/CoreSimulator/SimDeviceSet.h $(FRAMEWORK_DIR)/Versions/A/Headers/
	install -m 644 Sources/CoreSimulator/SimDevice.h $(FRAMEWORK_DIR)/Versions/A/Headers/
	install -m 644 Sources/CoreSimulator/SimDeviceType.h $(FRAMEWORK_DIR)/Versions/A/Headers/
	install -m 644 Sources/CoreSimulator/SimRuntime.h $(FRAMEWORK_DIR)/Versions/A/Headers/
	install -m 644 Sources/CoreSimulator/module.modulemap $(FRAMEWORK_DIR)/Versions/A/Modules/module.modulemap
	ln -sf A $(FRAMEWORK_DIR)/Versions/Current
	ln -sf Versions/Current/CoreSimulator $(FRAMEWORK_DIR)/CoreSimulator
	ln -sf Versions/Current/Headers $(FRAMEWORK_DIR)/Headers
	ln -sf Versions/Current/Modules $(FRAMEWORK_DIR)/Modules
	ln -sf Versions/Current/Resources $(FRAMEWORK_DIR)/Resources

$(OBJDIR)/CoreSimulator.o: Sources/CoreSimulator/CoreSimulator.m Sources/CoreSimulator/CoreSimulator.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS_FRAME) -c -o $@ Sources/CoreSimulator/CoreSimulator.m

$(OBJDIR)/SimServiceContext.o: Sources/CoreSimulator/SimServiceContext.m Sources/CoreSimulator/SimServiceContext.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS_FRAME) -c -o $@ Sources/CoreSimulator/SimServiceContext.m

$(OBJDIR)/SimDeviceSet.o: Sources/CoreSimulator/SimDeviceSet.m Sources/CoreSimulator/SimDeviceSet.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS_FRAME) -c -o $@ Sources/CoreSimulator/SimDeviceSet.m

$(OBJDIR)/SimDevice.o: Sources/CoreSimulator/SimDevice.m Sources/CoreSimulator/SimDevice.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS_FRAME) -c -o $@ Sources/CoreSimulator/SimDevice.m

$(OBJDIR)/SimDeviceType.o: Sources/CoreSimulator/SimDeviceType.m Sources/CoreSimulator/SimDeviceType.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS_FRAME) -c -o $@ Sources/CoreSimulator/SimDeviceType.m

$(OBJDIR)/SimRuntime.o: Sources/CoreSimulator/SimRuntime.m Sources/CoreSimulator/SimRuntime.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS_FRAME) -c -o $@ Sources/CoreSimulator/SimRuntime.m

install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 $(SIMCTL_BIN) $(DESTDIR)$(PREFIX)/bin/simctl
	install -d $(DESTDIR)/Library/Developer/PrivateFrameworks
	cp -R $(FRAMEWORK_DIR) $(DESTDIR)/Library/Developer/PrivateFrameworks/CoreSimulator.framework

clean:
	rm -rf build

.PHONY: all install clean
