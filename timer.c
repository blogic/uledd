#include <time.h>
#include <stdbool.h>

#include <libubox/list.h>
#include <libubox/uloop.h>

#include "log.h"
#include "timer.h"

static int singular_timer_interval;
static struct uloop_timeout singular_timer;
static struct list_head timers = LIST_HEAD_INIT(timers);

static int tv_diff(struct timeval *t1, struct timeval *t2)
{
	return
		(t1->tv_sec - t2->tv_sec) * 1000 +
		(t1->tv_usec - t2->tv_usec) / 1000;
}

static void led_timer_gettime(struct timeval *tv)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / 1000;
}

static void led_process_timers()
{
	struct timeval tv;
	struct led_timer *t;

	led_timer_gettime(&tv);

	while (!list_empty(&timers)) {
		t = list_first_entry(&timers, struct led_timer, list);

		if (tv_diff(&t->time, &tv) > 0)
			break;

		led_timer_cancel(t);
		if (t->cb)
			t->cb(t);
	}
}

static void singular_timer_cb(struct uloop_timeout *t)
{
	led_process_timers();
	uloop_timeout_set(&singular_timer, singular_timer_interval);
}

int led_timer_add(struct led_timer *t)
{
	struct led_timer *tmp;
	struct list_head *h = &timers;

	if (t->pending)
		return -1;

	list_for_each_entry(tmp, &timers, list) {
		if (tv_diff(&tmp->time, &t->time) > 0) {
			h = &tmp->list;
			break;
		}
	}

	list_add_tail(&t->list, h);
	t->pending = true;

	return 0;
}

int led_timer_set(struct led_timer *t, int msecs)
{
	struct timeval *time = &t->time;

	if (t->pending)
		led_timer_cancel(t);

	led_timer_gettime(time);

	time->tv_sec += msecs / 1000;
	time->tv_usec += (msecs % 1000) * 1000;

	if (time->tv_usec > 1000000) {
		time->tv_sec++;
		time->tv_usec -= 1000000;
	}

	return led_timer_add(t);
}

int led_timer_cancel(struct led_timer *t)
{
	if (!t->pending)
		return -1;

	list_del(&t->list);
	t->pending = false;

	return 0;
}

void led_timers_init(int tick_interval)
{
	singular_timer.cb = singular_timer_cb;
	singular_timer_interval = tick_interval;
	uloop_timeout_set(&singular_timer, tick_interval);
}

void led_timers_done()
{
	struct led_timer *t, *tmp;

	uloop_timeout_cancel(&singular_timer);
	list_for_each_entry_safe(t, tmp, &timers, list)
		led_timer_cancel(t);
}
