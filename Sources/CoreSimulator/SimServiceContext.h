/*
 * SimServiceContext - CoreSimulator service context
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class SimDeviceSet;
@class SimDeviceType;
@class SimRuntime;

@interface SimServiceContext : NSObject

+ (instancetype)sharedServiceContextForDeveloperDir:(nullable NSString *)developerDir error:(NSError **)error;

- (instancetype)initWithDeveloperDir:(nullable NSString *)developerDir connectionType:(nullable id)connectionType error:(NSError **)error NS_DESIGNATED_INITIALIZER;

- (BOOL)connectWithError:(NSError **)error;
- (nullable SimDeviceSet *)deviceSetWithPath:(nullable NSString *)path error:(NSError **)error;

@end

NS_ASSUME_NONNULL_END
