/*
 * Copyright (C) 2017 John Crispin <john@phrozen.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include <libubus.h>

#include "blob_led.h"
#include "led.h"
#include "log.h"
#include "scene.h"
#include "ubus.h"

static struct blob_buf b = { 0 };
static struct ubus_auto_conn conn;

enum {
	SCENE_NAME,
	__SCENE_MAX,
};

static const struct blobmsg_policy scene_policy[] = {
	[SCENE_NAME]	= { "name", BLOBMSG_TYPE_STRING },
};

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

static int
handle_scene(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method,
		    struct blob_attr *msg)
{
	const char *name = NULL;
	struct blob_attr *tb[__SCENE_MAX];

	blobmsg_parse(scene_policy, __SCENE_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[SCENE_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	name = blobmsg_get_string(tb[SCENE_NAME]);
	if (!scene_run(name))
		return UBUS_STATUS_UNKNOWN_ERROR;

	return UBUS_STATUS_OK;
}

static int
handle_state(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method,
		    struct blob_attr *msg)
{
	blob_buf_init(&b, 0);
	leds_state_blobmsg(&b);
	scenes_state_blobmsg(&b);
	ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}

static const struct ubus_method led_methods[] = {
	UBUS_METHOD_NOARG("state", handle_state),
	{
		.name = "set",
		.handler = set_colour,
		.policy = colour_policy,
		.n_policy = COLOUR_POLICY_LEN,
	},
	UBUS_METHOD("scene", handle_scene, scene_policy),
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
