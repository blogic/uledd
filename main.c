/*
 * Copyright (C) 2017 John Crispin <john@phrozen.org>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <libubox/uloop.h>

#include "ubus.h"

int main(int argc, char **argv)
{
	uloop_init();
	ubus_init();
	uloop_run();
	uloop_done();

	return 0;
}
