#pragma once

struct led_timer;
typedef void (*led_timer_handler)(struct led_timer *t);

struct led_timer {
	bool pending;
	led_timer_handler cb;
	struct timeval time;
	struct list_head list;
};

int led_timer_add(struct led_timer *t);
int led_timer_set(struct led_timer *t, int msecs);
int led_timer_cancel(struct led_timer *t);

void led_timers_init(int tick_interval);
void led_timers_done();
