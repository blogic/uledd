#pragma once

#include <stdbool.h>

struct scene;
struct blob_led;

struct scene* scene_add(const char *name, int timeout, int priority);
void scene_led_add(struct scene *s, struct blob_led *b);
const char* scene_str(struct scene *s);
bool scene_run(const char *name);
void scene_done();
