#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "scene.h"
#include "scene_json.h"

unsigned int debug = 3;

static void test_file(const char *file)
{
	scene_add("foo", 100, 2);

	LOG("loading %s\n", file);
	scene_json_load(file);

	LOG("next load should fail\n");
	scene_json_load(file);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <path_to_scenes_json_dir>\n", argv[0]);
		return -1;
	}

	ulog_open(ULOG_STDIO, LOG_DAEMON, "test-scene-json");

	test_file(argv[1]);
	scene_done();

	return 0;
}
