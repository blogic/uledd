/*
 * Copyright (C) 2017 John Crispin <john@phrozen.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <libubox/uloop.h>

#include <libubus.h>

static struct ubus_auto_conn conn;

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

	if (abs(led->brightness - led->original) != 0) 
		uloop_timeout_set(&led->timer, fade / abs(led->brightness - led->original));
	else
		uloop_timeout_set(&led->timer, fade);
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

static struct avl_tree led_tree = AVL_TREE_INIT(led_tree, avl_strcmp, false, NULL);

static void
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
		if (abs(led->brightness - led->original) != 0)
			timeout = led->on / abs(led->brightness - led->original);
		else
			timeout = led->on; 
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

enum {
	COLOUR_LEDS,
	COLOUR_BLINK,
	COLOUR_FADE,
	COLOUR_ON,
	COLOUR_OFF,
	__COLOUR_MAX,
};

static const struct blobmsg_policy colour_policy[] = {
	[COLOUR_LEDS]	= { "leds", BLOBMSG_TYPE_TABLE },
	[COLOUR_BLINK]	= { "blink", BLOBMSG_TYPE_INT32 },
	[COLOUR_FADE]	= { "fade", BLOBMSG_TYPE_INT32 },
	[COLOUR_ON]	= { "on", BLOBMSG_TYPE_INT32 },
	[COLOUR_OFF]	= { "off", BLOBMSG_TYPE_INT32 },
};

static int
set_colour(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method,
		    struct blob_attr *msg)
{
	struct blob_attr *tb[__COLOUR_MAX], *cur;
	int rem, blink = 0, fade = 0, on = 0, off = 0;

	blobmsg_parse(colour_policy, __COLOUR_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[COLOUR_LEDS])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (tb[COLOUR_BLINK])
		blink = blobmsg_get_u32(tb[COLOUR_BLINK]);

	if (tb[COLOUR_FADE])
		fade = blobmsg_get_u32(tb[COLOUR_FADE]);

	if (tb[COLOUR_ON])
		on = blobmsg_get_u32(tb[COLOUR_ON]);

	if (tb[COLOUR_OFF])
		off = blobmsg_get_u32(tb[COLOUR_OFF]);

	blobmsg_for_each_attr(cur, tb[COLOUR_LEDS], rem) {
		static struct blobmsg_policy brightness_policy[2] = {
			{ .type = BLOBMSG_TYPE_INT32 },
			{ .type = BLOBMSG_TYPE_INT32 },
		};
		struct blob_attr *brightness[2];

		if (blobmsg_type(cur) == BLOBMSG_TYPE_INT32)
			led_add(blobmsg_name(cur), blobmsg_get_u32(cur), -1, blink, fade, on, off);
		else if (blobmsg_type(cur) == BLOBMSG_TYPE_ARRAY) {
			blobmsg_parse_array(brightness_policy, 2, brightness, blobmsg_data(cur),
					    blobmsg_data_len(cur));
			if (!brightness[0] || !brightness[1])
				continue;
			led_add(blobmsg_name(cur), blobmsg_get_u32(brightness[1]), blobmsg_get_u32(brightness[0]),
				blink, fade, on, off);
		}
	}
	return 0;
}

static const struct ubus_method led_methods[] = {
	UBUS_METHOD("set", set_colour, colour_policy),
};

static struct ubus_object_type led_object_type =
	UBUS_OBJECT_TYPE("led", led_methods);

static struct ubus_object led_object = {
	.name = "led",
	.type = &led_object_type,
	.methods = led_methods,
	.n_methods = ARRAY_SIZE(led_methods),
};

static void
ubus_connect_handler(struct ubus_context *ctx)
{
	int ret;

	ret = ubus_add_object(ctx, &led_object);
	if (ret)
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
}

int main(int argc, char **argv)
{
	uloop_init();
	conn.cb = ubus_connect_handler;
	ubus_auto_connect(&conn);
	uloop_run();
	uloop_done();

	return 0;
}
