/*
 * Copyright (C) 2017 John Crispin <john@phrozen.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include <libubus.h>

#include "blob_led.h"
#include "led.h"
#include "log.h"
#include "ubus.h"

static struct ubus_auto_conn conn;

static void
led_parse_handler(struct blob_led *b, void *cb_arg)
{
	led_run(led_add(b));
	blob_led_done(b);
}

static int
set_colour(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method,
		    struct blob_attr *msg)
{
	if (!blob_led_parse(blob_data(msg), blob_len(msg), led_parse_handler, 0))
		return UBUS_STATUS_INVALID_ARGUMENT;

	return UBUS_STATUS_OK;
}

static const struct ubus_method led_methods[] = {
	{
		.name = "set",
		.handler = set_colour,
		.policy = colour_policy,
		.n_policy = COLOUR_POLICY_LEN,
	},
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
		ERROR("Failed to add object: %s\n", ubus_strerror(ret));
}

void
ubus_init(const char *socket_path)
{
	conn.path = socket_path;
	conn.cb = ubus_connect_handler;
	ubus_auto_connect(&conn);
}
