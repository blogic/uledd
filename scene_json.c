#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <libubox/blobmsg_json.h>

#include "blob_led.h"
#include "led.h"
#include "log.h"
#include "scene.h"
#include "scene_json.h"

static struct blob_buf bbuf;

enum {
	SCENE_NAME,
	SCENE_PRIORITY,
	SCENE_TIMEOUT,
	__SCENE_MAX,
};

static const struct blobmsg_policy policy[] = {
	[SCENE_NAME]	 = { "name", BLOBMSG_TYPE_STRING },
	[SCENE_PRIORITY] = { "priority", BLOBMSG_TYPE_INT32 },
	[SCENE_TIMEOUT]  = { "timeout", BLOBMSG_TYPE_INT32 },
};

static void
led_parse_handler(struct blob_led *b, void *cb_arg)
{
	struct scene *s = (struct scene*) cb_arg;
	scene_led_add(s, b);
}

static int
scene_json_load_file(const char *filename)
{
	int priority = 0;
	int timeout = 30000;
	const char *name = NULL;
	struct scene *scene = NULL;
	struct blob_attr *tb[__SCENE_MAX];

	DEBUG(2, "loading %s\n", filename);

	blob_buf_init(&bbuf, 0);
	if (!blobmsg_add_json_from_file(&bbuf, filename)) {
		ERROR("failed to parse %s\n", filename);
		return -1;
	}

	blobmsg_parse(policy, __SCENE_MAX, tb, blob_data(bbuf.head),
		      blob_len(bbuf.head));

	if (!tb[SCENE_NAME]) {
		ERROR("missing `name` in %s\n", filename);
		return -1;
	}
	name = blobmsg_get_string(tb[SCENE_NAME]);

	if (tb[SCENE_TIMEOUT])
		timeout = blobmsg_get_u32(tb[SCENE_TIMEOUT]);

	if (tb[SCENE_PRIORITY])
		priority = blobmsg_get_u32(tb[SCENE_PRIORITY]);

	scene = scene_add(name, timeout, priority);
	if (!scene) {
		ERROR("failed add scene %s\n", filename);
		return -1;
	}

	if (!blob_led_parse(blob_data(bbuf.head), blob_len(bbuf.head),
			    led_parse_handler, scene)) {
		ERROR("error with LEDs in %s\n", filename);
		return -1;
	}

	return 0;
}

void
scene_json_load(const char *config_dir)
{
	size_t i;
	glob_t gl;
	char *path;
	struct stat st;
	size_t path_len;
	const char *suffix = "/*.json";

	path_len = strlen(config_dir) + strlen(suffix) + 1;
	path = calloc(1, path_len);
	if (!path)
		return;

	snprintf(path, path_len, "%s%s", config_dir, suffix);
	if (glob(path, GLOB_NOESCAPE | GLOB_MARK, NULL, &gl)) {
		free(path);
		return;
	}

	for (i = 0; i < gl.gl_pathc; i++) {
		if (stat(gl.gl_pathv[i], &st) || !S_ISREG(st.st_mode))
			continue;

		scene_json_load_file(gl.gl_pathv[i]);
	}

	globfree(&gl);
	free(path);
}
