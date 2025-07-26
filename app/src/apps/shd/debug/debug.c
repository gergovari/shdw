#include "debug.h"

#include <lvgl.h>
#include <stdio.h>
#include <stdlib.h>

#include <zephyr/drivers/rtc.h>

#include "../../../activity_manager.h"

typedef struct {
	lv_timer_t* timer;
	lv_obj_t* list;
	shd_act_ctx_t* activity_ctx;
} shd_debug_ctx_t;

void shd_debug_back_cb(lv_event_t* e) {
	shd_act_ctx_t* ctx = (shd_act_ctx_t*)lv_event_get_user_data(e);

	shd_act_man_back_go(ctx->manager, NULL);
}
void shd_debug_home_cb(lv_event_t* e) {
	shd_act_ctx_t* ctx = (shd_act_ctx_t*)lv_event_get_user_data(e);

	// TODO: handle success and error by coloring the button or smt
	shd_act_man_home_go(ctx->manager, NULL);
}
void shd_debug_refresh_activities(lv_timer_t* timer) {
	shd_debug_ctx_t* ctx = (shd_debug_ctx_t*)lv_timer_get_user_data(timer);
	shd_act_man_ctx_t* manager = ctx->activity_ctx->manager;
	lv_obj_t* list = ctx->list;
	lv_obj_t* activity_ctx_label;

	shd_act_ctx_node_t* node = manager->activities;
	shd_act_ctx_t* activity_ctx;
	shd_app_t* app;
	char* title;
	shd_act_state_t state;

	lv_obj_clean(list);
	
	while (node != NULL) {
		activity_ctx = node->value;
		app = activity_ctx->app;
		title = app->title;
		state = activity_ctx->state;

		activity_ctx_label = lv_list_add_text(ctx->list, "placeholder");
		printf("%s (%p) | %i -> %p\n", title, activity_ctx, state, node->prev);
		lv_label_set_text_fmt(activity_ctx_label, "%s (%p) | %i", title, activity_ctx, state);

		node = node->prev;
	}
}

void shd_debug_main_start(shd_act_ctx_t* activity_ctx) {
	lv_obj_t* screen = activity_ctx->screen;
	
	lv_obj_t* btn_cont = lv_obj_create(screen);
	lv_obj_t* home_btn = lv_btn_create(btn_cont);
	lv_obj_t* home_label = lv_label_create(home_btn);
	lv_obj_t* back_btn = lv_btn_create(btn_cont);
	lv_obj_t* back_label = lv_label_create(back_btn);

	shd_debug_ctx_t* ctx = malloc(sizeof(shd_debug_ctx_t));

	activity_ctx->activity_user = ctx;
	ctx->activity_ctx = activity_ctx;
	ctx->list = lv_list_create(screen);

	lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN_WRAP);
	lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW_WRAP);

	lv_label_set_text(home_label, "HOME");
	lv_obj_add_event_cb(home_btn, shd_debug_home_cb, LV_EVENT_CLICKED, activity_ctx);

	lv_label_set_text(back_label, "BACK");
	lv_obj_add_event_cb(back_btn, shd_debug_back_cb, LV_EVENT_CLICKED, activity_ctx);
}

void shd_debug_main_resume(shd_act_ctx_t* activity_ctx) {
	shd_debug_ctx_t* ctx = (shd_debug_ctx_t*)activity_ctx->activity_user;

	ctx->timer = lv_timer_create(shd_debug_refresh_activities, 500, ctx);
	lv_timer_ready(ctx->timer);
}
void shd_debug_main_pause(shd_act_ctx_t* activity_ctx) {
	shd_debug_ctx_t* ctx = (shd_debug_ctx_t*)activity_ctx->activity_user;
	
	lv_timer_delete(ctx->timer);
}

intent_filter_t shd_debug_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_DEBUG
};
intent_filter_node_t shd_debug_intent_filter_node = { 
	.intent_filter = &shd_debug_filter, 
	.next = NULL 
};
shd_act_t shd_debug_main = {
	.id = "shd.debug.main",
	.intent_filters = &shd_debug_intent_filter_node,

	.on_create = NULL,
	.on_destroy = NULL,

	.on_start = shd_debug_main_start,
	.on_restart = NULL,
	.on_stop = NULL,

	.on_resume = shd_debug_main_resume,
	.on_pause = shd_debug_main_pause
};
shd_act_node_t shd_debug_activity_node = { 
	.activity = &shd_debug_main, 
	.next = NULL 
};
shd_app_t shd_debug = {
	.id = "shd.debug",
	.title = "Debug",
	.activities = &shd_debug_activity_node
};
