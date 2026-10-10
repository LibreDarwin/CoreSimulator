/*
 * SimDevice - CoreSimulator device implementation
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import "SimDevice.h"
#import "SimDevice_Private.h"

@implementation SimDevice

@synthesize name = _name;
@synthesize UDID = _UDID;
@synthesize deviceTypeIdentifier = _deviceTypeIdentifier;
@synthesize runtimeIdentifier = _runtimeIdentifier;
@synthesize state = _state;
@synthesize isAvailable = _isAvailable;

@end
