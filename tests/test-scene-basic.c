#include <stdio.h>
#include <stdlib.h>

#include <libubox/uloop.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>

#include "led.h"
#include "log.h"
#include "scene.h"
#include "scene_json.h"

unsigned int debug = 3;
#define LED_TIMER_TICK_INTERVAL 100
static struct uloop_timeout exit_timer;
static struct blob_buf b = { 0 };

static void
exit_timer_handler(struct uloop_timeout *t)
{
	uloop_end();
}

static void
dump_state_json()
{
	char *json;

	blob_buf_init(&b, 0);
	scenes_state_blobmsg(&b);
	json = blobmsg_format_json_indent(b.head, 1, 0);
	fprintf(stdout, "%s\n", json);
	fflush(stdout);
	free(json);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <path_to_scenes_json_dir>\n", argv[0]);
		return -1;
	}

	ulog_open(ULOG_STDIO, LOG_DAEMON, "test-scene-json");

	uloop_init();
	led_init(LED_TIMER_TICK_INTERVAL);

	exit_timer.cb = exit_timer_handler;
	uloop_timeout_set(&exit_timer, LED_TIMER_TICK_INTERVAL * 17);

	scene_json_load(argv[1]);
	dump_state_json();
	scene_run("short");

	uloop_run();

	uloop_done();
	led_done();
	scene_done();

	LOG("finished!\n");

	return 0;
}
