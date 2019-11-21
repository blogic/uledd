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
#include <libubox/uloop.h>
#include <libubox/utils.h>

#include "log.h"
#include "led.h"
#include "timer.h"

#define LED_TIMER_TICK_INTERVAL 10

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
	int brightness;
	int original;
	int current;
	int delta;
	int fade;
	int blink;
	int on;
	int off;
	struct led_timer timer;
	char *path;
};

static void
led_set(struct led *led, int brightness)
{
	int r;
	FILE *fp;
	char path[256];

	DEBUG(3, "set %s to %d\n", led->path, brightness);

	snprintf(path, sizeof(path), "/sys/class/leds/%s/brightness", led->path);
	fp = fopen(path, "w");
	if (!fp)
		return;

	r = fprintf(fp, "%d", brightness);
	fclose(fp);

	if (r < 0)
		return;

	led->current = brightness;
}

static int
led_get(struct led *led)
{
	return 0;
}

#ifdef ULEDD_DEBUG
static char*
led_state_str(enum led_state state)
{
	char *p = NULL;
	static char buf[16] = {0};

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
		{ 0, NULL },
	};

	p = (char *) led_state_tbl[state].str;
	snprintf(buf, sizeof(buf), "%s", p);

	return buf;
}
#endif

static void
led_state_set(struct led *led, enum led_state state)
{
	led->state = state;
	DEBUG(3, "%s to %s\n", led->path, led_state_str(state));
}

static void
led_fade_out(struct led *led)
{
	int to = led->brightness;
	int from = led->original;
	int value = led->current;

	if (value == 0)
		value = from;
	else
		value -= led->delta;

	if (value < 0)
		value = to;

	led_set(led, value);

	if (led->current == to) {
		led->original = to;
		led->brightness = from;
		led_state_set(led, led->on ? LED_FADE_IN : LED_SET);
	}

	led_timer_set(&led->timer, LED_TIMER_TICK_INTERVAL);
}

static void
led_fade_in(struct led *led)
{
	int to = led->brightness;
	int from = led->original;
	int value = led->current;

	if (value < from)
		value = from;
	else
		value += led->delta;

	if (value > to)
		value = to;

	led_set(led, value);

	if (led->current == to) {
		led->original = to;
		led->brightness = from;
		led_state_set(led, led->off ? LED_FADE_OUT : LED_SET);
	}

	led_timer_set(&led->timer, LED_TIMER_TICK_INTERVAL);
}

static void
led_timer_cb(struct led_timer *t)
{
	struct led *led = container_of(t, struct led, timer);
	int brightness, timeout;

	switch (led->state) {
	case LED_BLINK_OFF:
		timeout = led->on;
		brightness = led->brightness;
		led->state = LED_BLINK_ON;
		break;

	case LED_BLINK_ON:
		timeout = led->off;
		brightness = led->original;
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
	int num_ticks = duration / LED_TIMER_TICK_INTERVAL;
	int delta = range / num_ticks;
	return delta > 0 ? delta : 1;
}

void
led_add(const char *path, int brightness, int original, int blink, int fade, int on, int off)
{
	struct led *led;
	char *_path;
	int timeout;

	led = avl_find_element(&led_tree, path, led, avl);
	if (!led) {
		led = calloc_a(sizeof(*led), &_path, strlen(path) + 1);
		if (!led)
			return;
		led->path = strcpy(_path, path);
		led->current = led_get(led);
		led->avl.key = led->path;
		avl_insert(&led_tree, &led->avl);
	}

	led->brightness = brightness;
	led->fade = fade;
	led->blink = blink;
	led->on = on;
	led->off = off;
	if (original >= 0)
		led->original = original;
	else
		led->original = led->current;

	if (blink && on && off)
		led->state = LED_BLINK_ON;
	else if (fade && (led->brightness > led->original))
		led->state = LED_FADE_IN;
	else if (fade && (led->brightness < led->original))
		led->state = LED_FADE_OUT;
	else
		led->state = LED_SET;

	led_timer_cancel(&led->timer);
	led->timer.cb = led_timer_cb;

	switch (led->state) {
	case LED_SET:
		led_set(led, led->brightness);
		return;

	case LED_FADE_OUT:
		timeout = LED_TIMER_TICK_INTERVAL;
		led->delta = compute_delta(led->off, led->original);
		break;

	case LED_FADE_IN:
		timeout = LED_TIMER_TICK_INTERVAL;
		led->delta = compute_delta(led->on, led->brightness);
		break;

	case LED_BLINK_ON:
		timeout = led->on;
		led_set(led, led->brightness);
		break;

	default:
		return;
	}

	led_timer_set(&led->timer, timeout);
}

void
led_init()
{
	led_timers_init(LED_TIMER_TICK_INTERVAL);
}

void
led_done()
{
	led_timers_done();
}
