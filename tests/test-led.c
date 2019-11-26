#include <stdio.h>
#include <stdlib.h>

#include <libubox/uloop.h>

#include "log.h"
#include "led.h"

#define LED_TIMER_TICK_INTERVAL 100

unsigned int debug = 3;
static struct uloop_timeout exit_timer;

struct led *red = NULL;
struct led *green = NULL;

static void
test_basic_led()
{
	green = led_add("foo:green:wps", 255, 0, 0, 1, 1000, 1000);
	led_run(green);

	red = led_add("foo:red:error", 0, 255, 0, 1, 500, 500);
	led_run(red);
}

static void
exit_timer_handler(struct uloop_timeout *t)
{
	uloop_end();
}

int
main(int argc, char *argv[])
{
	ulog_open(ULOG_STDIO, LOG_DAEMON, "test-led");
	uloop_init();
	led_init(LED_TIMER_TICK_INTERVAL);

	test_basic_led();

	exit_timer.cb = exit_timer_handler;
	uloop_timeout_set(&exit_timer, LED_TIMER_TICK_INTERVAL * 15);

	uloop_run();
	led_done();
	uloop_done();

	LOG("finished!\n");

	return 0;
}
