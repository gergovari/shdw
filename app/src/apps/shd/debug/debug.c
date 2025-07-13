#include "debug.h"

#include <lvgl.h>
#include <stdlib.h>

#include <zephyr/drivers/rtc.h>

void shd_debug_main_entry(lv_obj_t* screen, activity_callback cb, void* input, void* user) {
}

void shd_debug_main_exit(lv_obj_t* screen) {
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
