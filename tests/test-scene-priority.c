#include <stdio.h>
#include <stdlib.h>

#include <libubox/uloop.h>

#include "led.h"
#include "log.h"
#include "scene.h"
#include "scene_json.h"

unsigned int debug = 3;
#define LED_TIMER_TICK_INTERVAL 100
static struct uloop_timeout exit_timer;
static struct uloop_timeout other_timer;
const char *other = NULL;

static void
exit_timer_handler(struct uloop_timeout *t)
{
	uloop_end();
}

static void
other_handler(struct uloop_timeout *t)
{
	scene_run(other);
}

int main(int argc, char *argv[])
{
	if (argc != 4) {
		fprintf(stderr, "Usage: %s <path_to_scenes_json_dir> <scene1> <scene2>\n", argv[0]);
		return -1;
	}

	ulog_open(ULOG_STDIO, LOG_DAEMON, "test-scene-priority");

	uloop_init();
	led_init(LED_TIMER_TICK_INTERVAL);

	exit_timer.cb = exit_timer_handler;
	uloop_timeout_set(&exit_timer, LED_TIMER_TICK_INTERVAL * 20);

	scene_json_load(argv[1]);
	scene_run(argv[2]);

	other = argv[3];
	other_timer.cb = other_handler;
	uloop_timeout_set(&other_timer, LED_TIMER_TICK_INTERVAL * 3);

	uloop_run();

	uloop_done();
	led_done();
	scene_done();

	LOG("finished!\n");

	return 0;
}
