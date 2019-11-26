#include <stdio.h>
#include <stdlib.h>

#include <libubox/blobmsg_json.h>

#include "blob_led.h"
#include "log.h"

unsigned int debug = 3;
static struct blob_buf buf;

static void
led_parse_handler(struct blob_led *b, void *cb_arg)
{
	LOG("%s\n", blob_led_str(b));
	blob_led_done(b);
}

static void
test_basic(const char *path)
{
	blob_buf_init(&buf, 0);
	if (!blobmsg_add_json_from_file(&buf, path)) {
		ERROR("error with %s\n", path);
		return;
	}

	if (!blob_led_parse(blob_data(buf.head), blob_len(buf.head),
			    led_parse_handler, 0)) {
		ERROR("error with LEDs in %s\n", path);
		return;
	}
}

int
main(int argc, char *argv[])
{
	ulog_open(ULOG_STDIO, LOG_DAEMON, "test-blob-led");

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename.json>\n", argv[0]);
		return 3;
	}

	test_basic(argv[1]);

	return 0;
}
