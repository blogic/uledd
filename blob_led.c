#include <libubox/utils.h>

#include "blob_led.h"

enum {
	COLOUR_LEDS,
	COLOUR_BLINK,
	COLOUR_FADE,
	COLOUR_ON,
	COLOUR_OFF,
	__COLOUR_MAX,
};

const struct blobmsg_policy colour_policy[COLOUR_POLICY_LEN] = {
	[COLOUR_LEDS]	= { "leds", BLOBMSG_TYPE_TABLE },
	[COLOUR_BLINK]	= { "blink", BLOBMSG_TYPE_INT32 },
	[COLOUR_FADE]	= { "fade", BLOBMSG_TYPE_INT32 },
	[COLOUR_ON]	= { "on", BLOBMSG_TYPE_INT32 },
	[COLOUR_OFF]	= { "off", BLOBMSG_TYPE_INT32 },
};

bool
blob_led_init(struct blob_led **b, const char *path, int brightness, int original, int blink, int fade, int on, int off)
{
	struct blob_led *n = NULL;

	n = calloc(1, sizeof(struct blob_led));
	if (!n)
		return false;

	n->on = on;
	n->off = off;
	n->fade = fade;
	n->blink = blink;
	n->original = original;
	n->brightness = brightness;
	n->scene_led = 0;

	n->path = strdup(path);
	if (!n->path) {
		free(n);
		return false;
	}

	*b = n;
	return true;
}

void
blob_led_done(struct blob_led *b)
{
	free(b->path);
	free(b);
	b = NULL;
}

bool
blob_led_parse(struct blob_buf *blob, size_t blob_len, blob_led_parse_cb cb, void *cb_arg)
{
	size_t rem;
	struct blob_led *led;
	struct blob_attr *tb[__COLOUR_MAX], *cur;
	int blink = 0, fade = 0, on = 0, off = 0;

	blobmsg_parse(colour_policy, __COLOUR_MAX, tb, blob, blob_len);
	if (!tb[COLOUR_LEDS])
		return false;

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

		if (blobmsg_type(cur) == BLOBMSG_TYPE_INT32) {
			if (!blob_led_init(&led, blobmsg_name(cur),
					   blobmsg_get_u32(cur), -1, blink,
					   fade, on, off)) {
				return false;
			}

			cb(led, cb_arg);
		} else if (blobmsg_type(cur) == BLOBMSG_TYPE_ARRAY) {
			blobmsg_parse_array(brightness_policy, 2, brightness,
					    blobmsg_data(cur),
					    blobmsg_data_len(cur));
			if (!brightness[0] || !brightness[1])
				continue;

			if (!blob_led_init(&led, blobmsg_name(cur),
					    blobmsg_get_u32(brightness[1]),
					    blobmsg_get_u32(brightness[0]),
					    blink, fade, on, off)) {
				return false;
			}

			cb(led, cb_arg);
		}
	}

	return true;
}

const char*
blob_led_str(struct blob_led *b)
{
#ifdef ULEDD_DEBUG
	static char buf[255] = {0};
	snprintf(buf, sizeof(buf), "%s brightness=%d original=%d blink=%d fade=%d on=%d off=%d",
		 b->path, b->brightness, b->original, b->blink, b->fade, b->on, b->off);
	return buf;
#else
	return "";
#endif
}
