/*
 * _SimLogging - Internal logging helpers
 * Copyright (C) 2026, LibreDarwin
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SIMLOGGING_H
#define _SIMLOGGING_H

void SimLog(unsigned int facility, unsigned int level, const char *format, ...) __attribute__((format(printf, 3, 4)));
void SimLogFence(void);

#endif
