/*
 * SimDeviceSet - CoreSimulator device set implementation
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import "SimDeviceSet.h"
#import "SimServiceContext.h"

@implementation SimDeviceSet

- (instancetype)initWithSetPath:(nullable NSString *)setPath serviceContext:(nullable SimServiceContext *)serviceContext
{
    self = [super init];
    if (self) {
        _setPath = setPath ? [setPath copy] : [NSHomeDirectory() stringByAppendingPathComponent:@"Library/Developer/CoreSimulator/Devices"];
        _serviceContext = serviceContext;
    }
    return self;
}

- (BOOL)subscribeToNotificationsWithError:(NSError **)error
{
    return YES;
}

@end
