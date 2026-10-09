/*
 * SimServiceContext - CoreSimulator service context implementation
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import "SimServiceContext.h"
#import "SimDeviceSet.h"
#import <CoreFoundation/CoreFoundation.h>

@implementation SimServiceContext

+ (instancetype)sharedServiceContextForDeveloperDir:(nullable NSString *)developerDir error:(NSError **)error
{
    static SimServiceContext *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[SimServiceContext alloc] initWithDeveloperDir:developerDir connectionType:nil error:nil];
    });
    if (error && !sharedInstance) {
        *error = [NSError errorWithDomain:@"com.apple.CoreSimulator" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Failed to create shared service context"}];
    }
    return sharedInstance;
}

- (instancetype)initWithDeveloperDir:(nullable NSString *)developerDir connectionType:(nullable id)connectionType error:(NSError **)error
{
    self = [super init];
    if (self) {
        // Minimal implementation - for compatibility with simctl operations
    }
    return self;
}

- (BOOL)connectWithError:(NSError **)error
{
    return YES;
}

- (nullable SimDeviceSet *)deviceSetWithPath:(nullable NSString *)path error:(NSError **)error
{
    if (path == nil) {
        path = [NSHomeDirectory() stringByAppendingPathComponent:@"Library/Developer/CoreSimulator/Devices"];
    }
    SimDeviceSet *set = [[SimDeviceSet alloc] initWithSetPath:path serviceContext:self];
    if (set == nil && error) {
        *error = [NSError errorWithDomain:@"com.apple.CoreSimulator" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Failed to initialize SimDeviceSet"}];
    }
    return set;
}

@end
