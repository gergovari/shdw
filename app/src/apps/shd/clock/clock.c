#include "clock.h"

#include <lvgl.h>
#include <stdlib.h>

#include <zephyr/drivers/rtc.h>

typedef struct shd_clock_ctx {
	const struct device* rtc;
	lv_obj_t* clock;
	lv_timer_t* timer;
} shd_clock_ctx;

void shd_clock_set_text(lv_timer_t* timer) {
	struct rtc_time tm;
	shd_clock_ctx* ctx = (shd_clock_ctx*)lv_timer_get_user_data(timer);

	if (rtc_get_time(ctx->rtc, &tm) == 0) {
		lv_label_set_text_fmt(ctx->clock, "%02i:%02i", tm.tm_hour, tm.tm_min);
	} else {
		lv_label_set_text(ctx->clock, "ERROR");
	}
}

void shd_clock_main_entry(activity_ctx_t* activity_ctx) {
	const struct device* rtc = DEVICE_DT_GET(DT_ALIAS(rtc));
	lv_obj_t* clock = lv_label_create(activity_ctx->screen);
	static lv_style_t style;
	shd_clock_ctx* ctx = malloc(sizeof(shd_clock_ctx));

	lv_obj_set_user_data(activity_ctx->screen, ctx);

	lv_style_init(&style);
	lv_style_set_text_font(&style, &lv_font_montserrat_48);
	lv_obj_add_style(clock, &style, 0);
	lv_obj_center(clock);
	
	ctx->rtc = rtc;
	ctx->clock = clock;
	ctx->timer = lv_timer_create(shd_clock_set_text, 500, ctx);
	lv_timer_ready(ctx->timer);
}

void shd_clock_main_exit(activity_ctx_t* activity_ctx) {
	shd_clock_ctx* ctx = (shd_clock_ctx*)lv_obj_get_user_data(activity_ctx->screen);
	
	lv_timer_delete(ctx->timer);
	free(ctx);
}

intent_filter_t shd_clock_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_LAUNCHER
};
intent_filter_node_t shd_clock_intent_filter_node = { 
	.intent_filter = &shd_clock_filter, 
	.next = NULL 
};
activity_t shd_clock_main = {
	.id = "shd.clock.main",
	.intent_filters = &shd_clock_intent_filter_node,
	.entry = shd_clock_main_entry,
	.pause = NULL,
	.unpause = NULL,
	.exit = shd_clock_main_exit
};
activity_node_t shd_clock_activity_node = { 
	.activity = &shd_clock_main, 
	.next = NULL 
};
app_t shd_clock = {
	.id = "shd.clock",
	.title = "Clock",
	.activities = &shd_clock_activity_node
};
