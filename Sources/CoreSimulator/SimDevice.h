/*
 * SimDevice - CoreSimulator device
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class SimDeviceSet;
@class SimDeviceType;
@class SimRuntime;

@interface SimDevice : NSObject

@property (nonatomic, readonly, copy) NSString *name;
@property (nonatomic, readonly, copy) NSUUID *UDID;
@property (nonatomic, readonly, copy) NSString *deviceTypeIdentifier;
@property (nonatomic, weak, nullable) SimDeviceSet *deviceSet;
@property (nonatomic, copy, nullable) NSString *runtimeSpecifier;

@end

NS_ASSUME_NONNULL_END
