#include "debug.h"

#include <lvgl.h>
#include <stdio.h>
#include <stdlib.h>

#include <zephyr/drivers/rtc.h>

void shd_debug_home_cb(lv_event_t* e) {
	printf("home\n");
}

void shd_debug_main_entry(activity_ctx_t* ctx) {
	lv_obj_t* home_btn = lv_btn_create(ctx->screen);
	lv_obj_t* home_label = lv_label_create(home_btn);

	lv_label_set_text(home_label, "HOME");
	lv_obj_add_event_cb(home_btn, shd_debug_home_cb, LV_EVENT_CLICKED, NULL);
}

void shd_debug_main_exit(activity_ctx_t* ctx) {
	// TODO
}

intent_filter_t shd_debug_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_DEBUG
};
intent_filter_node_t shd_debug_intent_filter_node = { 
	.intent_filter = &shd_debug_filter, 
	.next = NULL 
};
activity_t shd_debug_main = {
	.id = "shd.debug.main",
	.intent_filters = &shd_debug_intent_filter_node,
	.entry = shd_debug_main_entry,
	.exit = shd_debug_main_exit
};
activity_node_t shd_debug_activity_node = { 
	.activity = &shd_debug_main, 
	.next = NULL 
};
app_t shd_debug = {
	.id = "shd.debug",
	.title = "Debug",
	.activities = &shd_debug_activity_node
};
