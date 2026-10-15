/*
 * SimServiceContext - Private additions
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import "SimServiceContext.h"

NS_ASSUME_NONNULL_BEGIN

@interface SimServiceContext (Private)

- (BOOL)_registerPath:(NSString *)path error:(NSError **)error;

@end

NS_ASSUME_NONNULL_END
