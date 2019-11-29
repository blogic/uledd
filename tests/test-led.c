#include <stdio.h>
#include <stdlib.h>

#include <libubox/uloop.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>

#include "blob_led.h"
#include "led.h"
#include "log.h"

#define LED_TIMER_TICK_INTERVAL 100

unsigned int debug = 3;
static struct uloop_timeout exit_timer;

static struct led *red = NULL;
static struct led *green = NULL;
static struct blob_led *blob_red;
static struct blob_led *blob_green;
static struct blob_buf b = { 0 };

static void
dump_state_json()
{
	char *json;

	blob_buf_init(&b, 0);
	leds_state_blobmsg(&b);
	json = blobmsg_format_json_indent(b.head, 1, 0);
	fprintf(stdout, "%s\n", json);
	fflush(stdout);
	free(json);
}

static void
test_basic_led()
{
	blob_led_init(&blob_green, "foo:green:wps", 255, 0, 0, 1, 1000, 1000);
	green = led_add(blob_green);
	led_run(green);

	blob_led_init(&blob_red, "foo:red:error", 0, 255, 0, 1, 500, 500);
	red = led_add(blob_red);
	led_run(red);
}

static void
exit_timer_handler(struct uloop_timeout *t)
{
	blob_led_done(blob_red);
	blob_led_done(blob_green);
	uloop_end();
}

int
main(int argc, char *argv[])
{
	ulog_open(ULOG_STDIO, LOG_DAEMON, "test-led");
	uloop_init();
	led_init(LED_TIMER_TICK_INTERVAL);

	test_basic_led();
	dump_state_json();

	exit_timer.cb = exit_timer_handler;
	uloop_timeout_set(&exit_timer, LED_TIMER_TICK_INTERVAL * 15);

	uloop_run();
	led_done();
	uloop_done();

	LOG("finished!\n");

	return 0;
}
