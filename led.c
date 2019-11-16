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
	int fade;
	int blink;
	int on;
	int off;
	struct uloop_timeout timer;
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

static int
safe_timeout(int duration, int from, int to)
{
	int diff = abs(from - to);
	if (diff > 0)
		return duration / diff;
	return duration;
}

static void
led_fade_out(struct led *led)
{
	int delta = 1;
	int to = led->brightness;
	int from = led->original;
	int value = led->current;
	int timeout = safe_timeout(led->off, from, to);

	if (value == 0)
		value = from;
	else
		value -= delta;

	led_set(led, value);

	if (led->current <= to) {
		led->state = led->on ? LED_FADE_IN : LED_SET;
		led->original = to;
		led->brightness = from;
	}

	uloop_timeout_set(&led->timer, timeout);
}

static void
led_fade_in(struct led *led)
{
	int delta = 1;
	int to = led->brightness;
	int from = led->original;
	int value = led->current;
	int timeout = safe_timeout(led->on, from, to);

	if (value < from)
		value = from;
	else
		value += delta;

	led_set(led, value);

	if (led->current >= to) {
		led->state = led->off ? LED_FADE_OUT : LED_SET;
		led->original = to;
		led->brightness = from;
	}

	uloop_timeout_set(&led->timer, timeout);
}

static void
led_timer_cb(struct uloop_timeout *t)
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
	uloop_timeout_set(t, timeout);
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
	uloop_timeout_cancel(&led->timer);
	led->timer.cb = led_timer_cb;

	switch (led->state) {
	case LED_SET:
		led_set(led, led->brightness);
		return;

	case LED_FADE_OUT:
		timeout = safe_timeout(led->off, led->original, led->brightness);
		break;

	case LED_FADE_IN:
		timeout = safe_timeout(led->on, led->brightness, led->original);
		break;

	case LED_BLINK_ON:
		timeout = led->on;
		led_set(led, led->brightness);
		break;

	default:
		return;
	}

	uloop_timeout_set(&led->timer, timeout);
}
