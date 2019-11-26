#include <stdio.h>
#include <stdlib.h>

#include <libubox/uloop.h>

#include "log.h"
#include "timer.h"

#define LED_TIMER_TEST_COUNT 3
#define LED_TIMER_TICK_INTERVAL 100

unsigned int debug = 3;
static struct uloop_timeout exit_timer;

struct foo {
	int count;
	int interval;
	const char *nick;
	struct led_timer timer;
};

static void
foo_timer_cb(struct led_timer *t)
{
	struct foo *f = container_of(t, struct foo, timer);
	LOG("%s timer fired %dx\n", f->nick, ++f->count);
	if (f->count < LED_TIMER_TEST_COUNT)
		led_timer_set(&f->timer, f->interval);
}

static void
foo_init(struct foo *f, const char *nick, int interval)
{
	f->nick = nick;
	f->interval = interval;
	f->timer.cb = foo_timer_cb;
	led_timer_set(&f->timer, f->interval);
}

static void
test_basic_timer()
{
	static struct foo foo1;
	static struct foo foo2;
	static struct foo foo3;

	foo_init(&foo1, "foo1", LED_TIMER_TICK_INTERVAL+100);
	foo_init(&foo2, "foo2", LED_TIMER_TICK_INTERVAL+200);
	foo_init(&foo3, "foo3", LED_TIMER_TICK_INTERVAL+300);
}

static void
exit_timer_handler(struct uloop_timeout *t)
{
	uloop_end();
}

int
main(int argc, char *argv[])
{
	ulog_open(ULOG_STDIO, LOG_DAEMON, "test-timer");
	uloop_init();

	led_timers_init(LED_TIMER_TICK_INTERVAL);

	test_basic_timer();

	exit_timer.cb = exit_timer_handler;
	uloop_timeout_set(&exit_timer, 3 * (LED_TIMER_TICK_INTERVAL + 300));

	uloop_run();
	led_timers_done();
	uloop_done();

	LOG("finished!\n");

	return 0;
}
