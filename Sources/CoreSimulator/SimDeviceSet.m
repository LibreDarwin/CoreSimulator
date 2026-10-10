/*
 * SimDeviceSet - CoreSimulator device set implementation
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import "SimDeviceSet.h"
#import "SimDeviceSet_Private.h"
#import "SimServiceContext.h"
#import "SimDevice.h"
#import "SimDevice_Private.h"
#import <sys/stat.h>
#import <dirent.h>
#import <pwd.h>

static NSString *CSStateString(NSInteger state) {
    switch (state) {
        case 0: return @"Unknown";
        case 1: return @"Shutdown";
        case 2: return @"Booted";
        case 3: return @"Creating";
        case 4: return @"Deleting";
        default: return @"Shutdown";
    }
}

@interface SimDeviceSet ()
@property (nonatomic, strong) NSMutableDictionary<NSString *, SimDevice *> *devicesByUDIDDict;
@property (nonatomic, strong) NSMutableArray<SimDevice *> *devicesArray;
@end

@implementation SimDeviceSet

- (instancetype)initWithSetPath:(nullable NSString *)setPath serviceContext:(nullable SimServiceContext *)serviceContext
{
    self = [super init];
    if (self) {
        if (setPath) {
            _setPath = [setPath copy];
        } else {
            NSString *home = NSHomeDirectory();
            _setPath = [home stringByAppendingPathComponent:@"Library/Developer/CoreSimulator/Devices"];
        }
        _serviceContext = serviceContext;
        _devicesByUDIDDict = [NSMutableDictionary dictionary];
        _devicesArray = [NSMutableArray array];
        [self processDeviceSetPlist];
    }
    return self;
}

- (BOOL)subscribeToNotificationsWithError:(NSError **)error
{
    return YES;
}

- (BOOL)isDefaultSet
{
    return YES;
}

- (NSDictionary<NSString *, SimDevice *> *)devicesByUDID
{
    return [self.devicesByUDIDDict copy];
}

- (NSArray<SimDevice *> *)devices
{
    return [self.devicesArray copy];
}

- (NSArray<SimDevice *> *)availableDevices
{
    return [self.devices filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"isAvailable == YES"]];
}

- (BOOL)processDeviceSetPlist
{
    const char *cpath = [_setPath UTF8String];
    DIR *dir = opendir(cpath);
    if (dir == NULL) return NO;
    
    struct dirent *entry;
    [self.devicesByUDIDDict removeAllObjects];
    [self.devicesArray removeAllObjects];
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        char plist_path[1024];
        snprintf(plist_path, sizeof(plist_path), "%s/%s/device.plist", cpath, entry->d_name);
        NSString *p = [NSString stringWithUTF8String:plist_path];
        NSDictionary *d = [NSDictionary dictionaryWithContentsOfFile:p];
        if (d == nil) continue;
        SimDevice *dev = [[SimDevice alloc] init];
        dev.name = d[@"name"] ?: @"Unknown";
        NSString *uuidStr = d[@"UDID"];
        dev.UDID = uuidStr ? [[NSUUID alloc] initWithUUIDString:uuidStr] : nil;
        dev.deviceTypeIdentifier = d[@"deviceType"] ?: @"";
        dev.runtimeIdentifier = d[@"runtime"] ?: @"";
        dev.state = [d[@"state"] integerValue];
        dev.isAvailable = ([d[@"isDeleted"] boolValue] == NO);
        if (dev.UDID) {
            self.devicesByUDIDDict[[dev.UDID UUIDString]] = dev;
            [self.devicesArray addObject:dev];
        }
    }
    closedir(dir);
    return YES;
}

- (void)saveToDisk
{
    // No-op for stub
}

- (NSArray *)devicePairsContainingDeviceUDID:(NSString *)udid
{
    return @[];
}

- (NSArray *)devicePairsContainingDevice:(SimDevice *)device
{
    return @[];
}

- (NSDictionary<NSString *, id> *)devicePairsByUUID
{
    return @{};
}

- (NSArray *)devicePairs
{
    return @[];
}

- (NSArray *)availableDevicePairs
{
    return @[];
}

- (void)updateDefaultDevicesAndPairingsForDeveloperDir:(NSString *)dir force:(BOOL)force
{
}

- (void)updateDefaultDevicesAndPairingsAsyncForDeveloperDir:(NSString *)dir force:(BOOL)force
{
}

- (void)updateDefaultDevicePairingsAsyncForDeveloperDir:(NSString *)dir
{
}

@end
