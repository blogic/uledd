#pragma once

#include <stdbool.h>

struct scene;
struct blob_led;
struct blob_buf;

struct scene* scene_add(const char *name, int timeout, int priority);
void scene_led_add(struct scene *s, struct blob_led *b);
const char* scene_str(struct scene *s);
bool scene_run(const char *name);
bool scene_stop(const char *name);
void scene_done();

void scenes_state_blobmsg(struct blob_buf *b);
