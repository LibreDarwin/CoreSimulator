/*
 * _SimLogging - Internal logging helpers
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#import <Foundation/Foundation.h>
#import "_SimLogging.h"

void SimLog(unsigned int facility, unsigned int level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    NSString *msg = [[NSString alloc] initWithFormat:[NSString stringWithUTF8String:format] arguments:args];
    va_end(args);
    NSLog(@"[SimLog] %@", msg);
}

void SimLogFence(void)
{
}
