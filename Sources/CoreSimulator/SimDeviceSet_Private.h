/*
 * SimDeviceSet - Private additions
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import "SimDeviceSet.h"

NS_ASSUME_NONNULL_BEGIN

@interface SimDeviceSet (Private)

- (BOOL)isDefaultSet;
- (BOOL)processDeviceSetPlist;
- (void)saveToDisk;

@end

NS_ASSUME_NONNULL_END
