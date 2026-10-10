/*
 * SimDevice - Private additions
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import "SimDevice.h"

NS_ASSUME_NONNULL_BEGIN

@interface SimDevice (Private)

@property (nonatomic, copy) NSString *name;
@property (nonatomic, copy) NSUUID *UDID;
@property (nonatomic, copy) NSString *deviceTypeIdentifier;
@property (nonatomic, copy) NSString *runtimeIdentifier;
@property (nonatomic, assign) NSInteger state;
@property (nonatomic, assign) BOOL isAvailable;

@end

NS_ASSUME_NONNULL_END
