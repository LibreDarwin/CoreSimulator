/*
 * SimRuntime - CoreSimulator runtime
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface SimRuntime : NSObject

@property (nonatomic, readonly, copy) NSString *name;
@property (nonatomic, readonly, copy) NSString *versionString;
@property (nonatomic, readonly, copy) NSString *identifier;
@property (nonatomic, readonly, copy) NSString *bundlePath;
@property (nonatomic, readonly) BOOL isAvailable;
@property (nonatomic, readonly, getter=isInternal) BOOL internal;
@property (nonatomic, readonly, getter=isBeta) BOOL beta;
@property (nonatomic, readonly) NSUInteger version;
@property (nonatomic, readonly, copy) NSBundle *bundle;

@end

NS_ASSUME_NONNULL_END
