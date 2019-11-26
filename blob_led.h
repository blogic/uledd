#pragma once

#include <libubox/blobmsg.h>

struct blob_led {
	int brightness;
	int original;
	int fade;
	int blink;
	int on;
	int off;
	char *path;
};

#define COLOUR_POLICY_LEN 5
extern const struct blobmsg_policy colour_policy[COLOUR_POLICY_LEN];
typedef void (*blob_led_parse_cb)(struct blob_led *b, void *cb_arg);

bool blob_led_init(struct blob_led **b, const char *path, int brightness, int original, int blink, int fade, int on, int off);
void blob_led_done(struct blob_led *b);
bool blob_led_parse(struct blob_buf *blob, size_t blob_len, blob_led_parse_cb cb, void *cb_arg);
const char* blob_led_str(struct blob_led *b);
