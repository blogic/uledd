/*
 * Copyright (C) 2013 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <libubox/ulog.h>

#ifdef ULEDD_DEBUG
#define DEBUG(level, fmt, ...) do { \
	if (debug >= level) { \
		ulog(LOG_DEBUG, fmt, ## __VA_ARGS__); \
	} } while (0)
#else
#define DEBUG(level, fmt, ...)
#endif

#define LOG   ULOG_INFO
#define ERROR ULOG_ERR

extern unsigned int debug;
