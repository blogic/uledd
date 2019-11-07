/*
 * Copyright (C) 2017 John Crispin <blogic@openwrt.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

void led_init();
void led_done();
void led_add(const char *path, int brightness, int original, int blink, int fade, int on, int off);
