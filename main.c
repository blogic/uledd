/*
 * Copyright (C) 2017 John Crispin <john@phrozen.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <libubox/uloop.h>

#include "led.h"
#include "log.h"
#include "ubus.h"
#include "scene.h"
#include "scene_json.h"

#define LED_TIMER_TICK_INTERVAL 10
#define JSON_CONFIG_DIR "/etc/uled.d"

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
		"	-j <path>	Path to JSON config dir\n"
		"	-S		Print messages to stdout\n"
		"\n", prog);
	return 1;
}

int main(int argc, char **argv)
{
	int ch;
	const char *json_dir = NULL;
	int ulog_channels = ULOG_KMSG;
#ifdef ULEDD_DEBUG
	char *dbglvl = getenv("DBGLVL");

	if (dbglvl) {
		debug = atoi(dbglvl);
		unsetenv("DBGLVL");
	}
#endif

	while ((ch = getopt(argc, argv, "d:s:j:S")) != -1) {
		switch (ch) {
#ifdef ULEDD_DEBUG
		case 'd':
			debug = atoi(optarg);
			break;
#endif
		case 's':
			ubus_socket = optarg;
			break;
		case 'j':
			json_dir = optarg;
			break;
		case 'S':
			ulog_channels = ULOG_STDIO;
			break;
		default:
			return usage(argv[0]);
		}
	}

	if (!json_dir)
		json_dir = JSON_CONFIG_DIR;

	ulog_open(ulog_channels, LOG_DAEMON, "uledd");
	LOG("v%s started.\n", ULEDD_VERSION);

	uloop_init();
	ubus_init(ubus_socket);
	led_init(LED_TIMER_TICK_INTERVAL);
	scene_json_load(json_dir);

	uloop_run();

	uloop_done();
	led_done();
	scene_done();

	return 0;
}
