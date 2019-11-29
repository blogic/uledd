/*
 * Copyright (C) 2017 John Crispin <blogic@openwrt.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

struct led;
struct blob_led;

void led_init(int tick_interval);
void led_done();
struct led *led_add(struct blob_led *b);
struct led *led_from_path(const char *path);
void led_run(struct led *led);
void led_stop(struct led *led);
