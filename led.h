/*
 * Copyright (C) 2017 John Crispin <blogic@openwrt.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

struct led;

void led_init();
void led_done();
struct led *led_add(const char *path, int brightness, int original, int blink, int fade, int on, int off);
void led_run(struct led *led);
void led_stop(struct led *led);
