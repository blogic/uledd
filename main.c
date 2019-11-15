/*
 * Copyright (C) 2017 John Crispin <john@phrozen.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <libubox/uloop.h>

#include "log.h"
#include "ubus.h"

#ifdef ULEDD_DEBUG
unsigned int debug;
#endif

static char *ubus_socket = NULL;

static int
usage(const char *prog)
{
	fprintf(stderr, "Usage: %s [options]\n"
		"Options:\n"
#ifdef ULEDD_DEBUG
		"	-d <level>	Enable debug messages\n"
#endif
		"	-s <path>	Path to ubus socket\n"
		"	-S		Print messages to stdout\n"
		"\n", prog);
	return 1;
}

int main(int argc, char **argv)
{
	int ch;
	int ulog_channels = ULOG_KMSG;
#ifdef ULEDD_DEBUG
	char *dbglvl = getenv("DBGLVL");

	if (dbglvl) {
		debug = atoi(dbglvl);
		unsetenv("DBGLVL");
	}
#endif

	while ((ch = getopt(argc, argv, "d:s:S")) != -1) {
		switch (ch) {
#ifdef ULEDD_DEBUG
		case 'd':
			debug = atoi(optarg);
			break;
#endif
		case 's':
			ubus_socket = optarg;
			break;
		case 'S':
			ulog_channels = ULOG_STDIO;
			break;
		default:
			return usage(argv[0]);
		}
	}

	ulog_open(ulog_channels, LOG_DAEMON, "uledd");
	LOG("v%s started.\n", ULEDD_VERSION);

	uloop_init();
	ubus_init(ubus_socket);
	uloop_run();
	uloop_done();

	return 0;
}
