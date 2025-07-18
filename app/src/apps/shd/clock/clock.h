#pragma once

#include "../../../app.h"

extern app_t shd_clock;

typedef struct shd_clock_ctx {
	const struct device* rtc;

	lv_style_t* style;
	lv_obj_t* clock;
	lv_timer_t* timer;
} shd_clock_ctx;

void shd_clock_main_entry(activity_ctx_t* ctx);
void shd_clock_main_exit(activity_ctx_t* ctx);
