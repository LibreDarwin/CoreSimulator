/*
 * SimDeviceSet - CoreSimulator device set
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class SimServiceContext;

@interface SimDeviceSet : NSObject

@property (nonatomic, readonly, copy) NSString *setPath;
@property (nonatomic, readonly, weak, nullable) SimServiceContext *serviceContext;

- (instancetype)initWithSetPath:(nullable NSString *)setPath serviceContext:(nullable SimServiceContext *)serviceContext;

- (BOOL)subscribeToNotificationsWithError:(NSError **)error;

@end

NS_ASSUME_NONNULL_END
