#include "clock.h"

#include <lvgl.h>
#include <stdlib.h>

#include <zephyr/drivers/rtc.h>

#include "../../../intent_filter.h"

void shd_clock_set_text(lv_timer_t* timer) {
	struct rtc_time tm;
	shd_clock_ctx* ctx = (shd_clock_ctx*)lv_timer_get_user_data(timer);

	if (rtc_get_time(ctx->rtc, &tm) == 0) {
		lv_label_set_text_fmt(ctx->clock, "%02i:%02i", tm.tm_hour, tm.tm_min);
	} else {
		lv_label_set_text(ctx->clock, "ERROR");
	}
}

void shd_clock_main_create(shd_act_ctx_t* activity_ctx) {
	shd_clock_ctx* ctx = malloc(sizeof(shd_clock_ctx));

	ctx->rtc = DEVICE_DT_GET(DT_ALIAS(rtc));

	ctx->style = malloc(sizeof(lv_style_t));
	lv_style_init(ctx->style);
	lv_style_set_text_font(ctx->style, &lv_font_montserrat_48);

	activity_ctx->activity_user = ctx;
}
void shd_clock_main_destroy(shd_act_ctx_t* activity_ctx) {
	shd_clock_ctx* ctx = (shd_clock_ctx*)activity_ctx->activity_user;
	
	lv_style_reset(ctx->style);
	free(ctx);
}

void shd_clock_main_start(shd_act_ctx_t* activity_ctx) {
	shd_clock_ctx* ctx = (shd_clock_ctx*)activity_ctx->activity_user;
	lv_obj_t* clock = lv_label_create(activity_ctx->screen);

	ctx->clock = clock;
	lv_obj_set_user_data(activity_ctx->screen, ctx);

	lv_obj_add_style(clock, ctx->style, 0);
	lv_obj_center(clock);
	
}
void shd_clock_main_stop(shd_act_ctx_t* activity_ctx) {
	shd_clock_ctx* ctx = (shd_clock_ctx*)activity_ctx->activity_user;

	ctx->clock = NULL;
}

void shd_clock_main_resume(shd_act_ctx_t* activity_ctx) {
	shd_clock_ctx* ctx = (shd_clock_ctx*)activity_ctx->activity_user;

	ctx->timer = lv_timer_create(shd_clock_set_text, 500, ctx);
	lv_timer_ready(ctx->timer);
}
void shd_clock_main_pause(shd_act_ctx_t* activity_ctx) {
	shd_clock_ctx* ctx = (shd_clock_ctx*)activity_ctx->activity_user;
	
	lv_timer_delete(ctx->timer);
}

shd_intent_filter_t shd_clock_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_LAUNCHER
};
shd_intent_filter_node_t shd_clock_intent_filter_node = { 
	.intent_filter = &shd_clock_filter, 
	.next = NULL 
};
shd_act_t shd_clock_main = {
	.id = "shd.clock.main",
	.intent_filters = &shd_clock_intent_filter_node,

	.on_create = shd_clock_main_create,
	.on_destroy = shd_clock_main_destroy,

	.on_start = shd_clock_main_start,
	.on_restart = NULL,
	.on_stop = shd_clock_main_stop,

	.on_resume = shd_clock_main_resume,
	.on_pause = shd_clock_main_pause 
};
shd_act_node_t shd_clock_activity_node = { 
	.activity = &shd_clock_main, 
	.next = NULL 
};
shd_app_t shd_clock = {
	.id = "shd.clock",
	.title = "Clock",
	.activities = &shd_clock_activity_node
};
