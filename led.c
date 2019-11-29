/*
 * Copyright (C) 2017 John Crispin <john@phrozen.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <libubox/blobmsg.h>
#include <libubox/uloop.h>
#include <libubox/utils.h>

#include "blob_led.h"
#include "log.h"
#include "led.h"
#include "timer.h"

#ifdef UNIT_TESTING
#define LED_SYSFS_PATH "/tmp/%s-brightness"
#else
#define LED_SYSFS_PATH "/sys/class/leds/%s/brightness"
#endif

static int timer_tick_interval;
static struct avl_tree led_tree = AVL_TREE_INIT(led_tree, avl_strcmp, false, NULL);

enum led_state {
	LED_SET = 0,
	LED_FADE_IN,
	LED_FADE_OUT,
	LED_BLINK_ON,
	LED_BLINK_OFF,
};

struct led {
	struct avl_node avl;
	enum led_state state;
	int current;
	int delta;
	struct led_timer timer;
	struct blob_led *b;
};

static void
led_set(struct led *led, int brightness)
{
	int r;
	FILE *fp;
	char path[256];

	snprintf(path, sizeof(path), LED_SYSFS_PATH, led->b->path);
	fp = fopen(path, "w");
	if (!fp)
		return;

	r = fprintf(fp, "%d", brightness);
	fclose(fp);

	if (r < 0)
		return;

	DEBUG(3, "set %s to %d\n", led->b->path, brightness);
	led->current = brightness;
}

static int
led_get(struct led *led)
{
	return 0;
}

static const char*
led_state_str(enum led_state state)
{
	struct led_state_str {
		enum led_state state;
		const char* str;
	};

	static struct led_state_str led_state_tbl[] = {
		{ LED_SET, "LED_SET" },
		{ LED_FADE_IN, "LED_FADE_IN" },
		{ LED_FADE_OUT, "LED_FADE_OUT" },
		{ LED_BLINK_ON, "LED_BLINK_ON" },
		{ LED_BLINK_OFF, "LED_BLINK_OFF" },
	};

	if (state > sizeof(led_state_tbl))
		return "unknown";

	return led_state_tbl[state].str;
}

static void
led_state_set(struct led *led, enum led_state state)
{
	led->state = state;
	DEBUG(3, "%s to %s\n", led->b->path, led_state_str(state));
}

static void
led_fade_out(struct led *led)
{
	int to = led->b->brightness;
	int from = led->b->original;
	int value = led->current;

	if (value == 0)
		value = from;
	else
		value -= led->delta;

	if (value < 0)
		value = to;

	led_set(led, value);

	if (led->current == to) {
		led->b->original = to;
		led->b->brightness = from;
		led_state_set(led, led->b->on ? LED_FADE_IN : LED_SET);
	}

	led_timer_set(&led->timer, timer_tick_interval);
}

static void
led_fade_in(struct led *led)
{
	int to = led->b->brightness;
	int from = led->b->original;
	int value = led->current;

	if (value < from)
		value = from;
	else
		value += led->delta;

	if (value > to)
		value = to;

	led_set(led, value);

	if (led->current == to) {
		led->b->original = to;
		led->b->brightness = from;
		led_state_set(led, led->b->off ? LED_FADE_OUT : LED_SET);
	}

	led_timer_set(&led->timer, timer_tick_interval);
}

static void
led_timer_cb(struct led_timer *t)
{
	struct led *led = container_of(t, struct led, timer);
	int brightness, timeout;

	switch (led->state) {
	case LED_BLINK_OFF:
		timeout = led->b->on;
		brightness = led->b->brightness;
		led->state = LED_BLINK_ON;
		break;

	case LED_BLINK_ON:
		timeout = led->b->off;
		brightness = led->b->original;
		led->state = LED_BLINK_OFF;
		break;

	case LED_FADE_IN:
		return led_fade_in(led);

	case LED_FADE_OUT:
		return led_fade_out(led);

	default:
		return;
	}

	led_set(led, brightness);
	led_timer_set(t, timeout);
}

static int
compute_delta(int duration, int range)
{
	int num_ticks = duration / timer_tick_interval;
	int delta = range / num_ticks;
	return delta > 0 ? delta : 1;
}

struct led*
led_add(struct blob_led *b)
{
	struct led *led;

	led = led_from_path(b->path);
	if (!led) {
		led = calloc(1, sizeof(*led));
		if (!led)
			return NULL;
		led->current = led_get(led);
		led->avl.key = b->path;
		avl_insert(&led_tree, &led->avl);
	}

	led->b = b;
	if (b->original >= 0)
		led->b->original = b->original;
	else
		led->b->original = led->current;

	if (b->blink && b->on && b->off)
		led->state = LED_BLINK_ON;
	else if (b->fade && (b->brightness > b->original))
		led->state = LED_FADE_IN;
	else if (b->fade && (b->brightness < b->original))
		led->state = LED_FADE_OUT;
	else
		led->state = LED_SET;

	led_timer_cancel(&led->timer);
	led->timer.cb = led_timer_cb;

	DEBUG(3, "%s\n", blob_led_str(b));
	return led;
}

void
led_run(struct led *led)
{
	int timeout;

	if (!led)
		return;

	switch (led->state) {
	case LED_SET:
		led_set(led, led->b->brightness);
		return;

	case LED_FADE_OUT:
		timeout = timer_tick_interval;
		led->delta = compute_delta(led->b->off, led->b->original);
		break;

	case LED_FADE_IN:
		timeout = timer_tick_interval;
		led->delta = compute_delta(led->b->on, led->b->brightness);
		break;

	case LED_BLINK_ON:
		timeout = led->b->on;
		led_set(led, led->b->brightness);
		break;

	default:
		return;
	}

	led_timer_set(&led->timer, timeout);

	DEBUG(3, "%s delta=%d timeout=%d brightness=%d original=%d blink=%d fade=%d on=%d off=%d\n",
	      led->b->path, led->delta, timeout, led->b->brightness, led->b->original, led->b->blink,
	      led->b->fade, led->b->on, led->b->off);
}

struct led *
led_from_path(const char *path)
{
	struct led *led;
	return avl_find_element(&led_tree, path, led, avl);
}

void
led_stop(struct led *led)
{
	if (!led)
		return;

	led_timer_cancel(&led->timer);
}

void
led_state_blobmsg(struct blob_buf *b, struct led *o)
{
	void *t = blobmsg_open_table(b, "");

	blobmsg_add_string(b, "name", o->b->path);
	blobmsg_add_string(b, "state", led_state_str(o->state));
	blobmsg_add_u32(b, "current", o->current);

	blobmsg_close_table(b, t);
}

void
leds_state_blobmsg(struct blob_buf *b)
{
	void *arr;
	struct led *led;

	arr = blobmsg_open_array(b, "leds");
	avl_for_each_element(&led_tree, led, avl)
		led_state_blobmsg(b, led);
	blobmsg_close_table(b, arr);
}

void
led_init(int tick_interval)
{
	timer_tick_interval = tick_interval;
	led_timers_init(tick_interval);
}

void
led_done()
{
	led_timers_done();
}
