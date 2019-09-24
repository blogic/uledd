/*
 * Copyright (C) 2017 John Crispin <john@phrozen.org>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <libubox/uloop.h>
#include <libubox/utils.h>

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
	FILE *fp;
	char path[256];

	led->current = brightness;
	fprintf(stderr, "set %s->%d\n", led->path, brightness);

	snprintf(path, sizeof(path), "/sys/class/leds/%s/brightness", led->path);
	fp = fopen(path, "w");
	if (!fp)
		return;
	fprintf(fp, "%d", brightness);
	fclose(fp);
}

static int
led_get(struct led *led)
{
	return 0;
}

static void
led_fade(struct led *led, int brightness, int fade, int next)
{
	int delta = 1;

	if (led->current > brightness)
		delta = -1;

	led_set(led, led->current + delta);
	if (led->current == brightness)
		led->state = next;

	uloop_timeout_set(&led->timer, fade / abs(led->brightness - led->original));
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
		led_fade(led, led->brightness, led->on, led->off ? LED_FADE_OUT : LED_SET);
		return;

	case LED_FADE_OUT:
		led_fade(led, led->original, led->off, led->on ? LED_FADE_IN : LED_SET);
		return;

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
	else if (fade && (led->original != led->brightness))
		led->state = LED_FADE_IN;
	else
		led->state = LED_SET;
	uloop_timeout_cancel(&led->timer);
	led->timer.cb = led_timer_cb;

	switch (led->state) {
	case LED_SET:
		led_set(led, led->brightness);
		return;

	case LED_FADE_IN:
		timeout = led->on / abs(led->brightness - led->original);
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
