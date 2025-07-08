#include "shd_apps.h"

#include <lvgl.h>
#include <stdio.h>
#include <stdlib.h>

#include <zephyr/drivers/rtc.h>

typedef struct shd_clock_ctx {
	const struct device* rtc;
	lv_obj_t* clock;
} shd_clock_ctx;

void shd_clock_set_text(lv_timer_t* timer) {
	struct rtc_time tm;
	shd_clock_ctx* ctx = (shd_clock_ctx*)lv_timer_get_user_data(timer);

	if (rtc_get_time(ctx->rtc, &tm) == 0) {
		lv_label_set_text_fmt(ctx->clock, "%i:%i:%i", tm.tm_hour, tm.tm_min, tm.tm_sec);
	} else {
		lv_label_set_text(ctx->clock, "ERROR");
	}
}

void shd_clock(lv_obj_t* cont) {
	const struct device* rtc = DEVICE_DT_GET(DT_ALIAS(rtc));
	lv_obj_t* clock = lv_label_create(cont);
	static lv_style_t style;
	shd_clock_ctx* ctx = malloc(sizeof(shd_clock_ctx)); // TODO: destroy on close
	lv_timer_t* timer;

	lv_style_init(&style);
	lv_style_set_text_font(&style, &lv_font_montserrat_48);
	lv_obj_add_style(clock, &style, 0);
	lv_obj_center(clock);
	
	ctx->rtc = rtc;
	ctx->clock = clock;
	timer = lv_timer_create(shd_clock_set_text, 500, ctx); // TODO: destroy on close
	lv_timer_ready(timer);
}

void shd_dummy(lv_obj_t* cont) {
	printf("dummy opened.\n");
}
