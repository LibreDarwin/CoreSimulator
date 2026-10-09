/*
 * SimDeviceType - CoreSimulator device type
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface SimDeviceType : NSObject

@property (nonatomic, readonly, copy) NSString *name;
@property (nonatomic, readonly, copy) NSString *identifier;
@property (nonatomic, readonly, copy) NSString *productFamily;
@property (nonatomic, readonly) NSUInteger minRuntimeVersion;
@property (nonatomic, readonly) NSUInteger maxRuntimeVersion;
@property (nonatomic, readonly, copy) NSBundle *bundle;
@property (nonatomic, readonly) BOOL supportsVision;

@end

NS_ASSUME_NONNULL_END
