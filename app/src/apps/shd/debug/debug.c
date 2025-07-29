#include "debug.h"

#include <lvgl.h>
#include <stdio.h>
#include <stdlib.h>

#include <zephyr/drivers/rtc.h>

#include "../../../activity_manager.h"
#include "../../../intent_filter.h"

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

char* shd_debug_state_to_string(shd_act_state_t state) {
	char* map[] = {
		"INITIALIZED/DESTROYED",
		"CREATED/STOPPED",
		"STARTED/PAUSED",
		"RESUMED",
	};

	return map[state];
}

void shd_debug_refresh_activities(lv_timer_t* timer) {
	shd_debug_ctx_t* ctx = (shd_debug_ctx_t*)lv_timer_get_user_data(timer);
	shd_act_man_ctx_t* manager = ctx->activity_ctx->manager;

	lv_obj_t* list = ctx->list;
	lv_obj_t* cont;
	lv_obj_t* label;
	lv_obj_t* matrix;
	lv_obj_t* image;
	lv_draw_buf_t* snapshot;
	static const char* matrix_map[] = {"Launch", "Kill", "\n", 
			"INITIALIZED\nDESTROYED", "CREATED\nSTOPPED", "STARTED\nPAUSED", "RESUMED",
			NULL};

	shd_act_ctx_node_t* node = manager->activities;
	shd_act_ctx_t* activity_ctx;
	lv_display_t* display;
	shd_app_t* app;
	char* title;
	shd_act_state_t state;

	lv_obj_clean(list);
	
	while (node != NULL) {
		activity_ctx = node->value;
		display = activity_ctx->display;
		app = activity_ctx->app;
		title = app->title;
		state = activity_ctx->state;
		snapshot = activity_ctx->snapshot;

		cont = lv_obj_create(list);
		lv_obj_set_width(cont, lv_pct(95));
		lv_obj_set_height(cont, LV_SIZE_CONTENT);
		lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
		lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

		label = lv_label_create(cont);
		lv_label_set_text_fmt(label, "%s (%p)\nstate: %s\ndisplay: %p", title, activity_ctx, shd_debug_state_to_string(state), display);
		
		matrix = lv_buttonmatrix_create(cont);
		lv_obj_set_width(matrix, lv_pct(100));
		lv_buttonmatrix_set_map(matrix, matrix_map);
		lv_buttonmatrix_set_button_ctrl(matrix, 2 + state, LV_BUTTONMATRIX_CTRL_DISABLED);
		if (shd_act_man_act_ctx_display_current_get(manager, display) == activity_ctx) {
			lv_buttonmatrix_set_button_ctrl(matrix, 0, LV_BUTTONMATRIX_CTRL_DISABLED);
			lv_buttonmatrix_set_button_ctrl(matrix, 2, LV_BUTTONMATRIX_CTRL_DISABLED);
			lv_buttonmatrix_set_button_ctrl(matrix, 3, LV_BUTTONMATRIX_CTRL_DISABLED);
			lv_buttonmatrix_set_button_ctrl(matrix, 4, LV_BUTTONMATRIX_CTRL_DISABLED);
		}

		if (snapshot != NULL) {
			image = lv_image_create(cont);
			lv_image_set_src(image, snapshot);
		}

		node = node->prev;
	}
}

void shd_debug_main_start(shd_act_ctx_t* activity_ctx) {
	lv_obj_t* screen = activity_ctx->screen;
	
	lv_obj_t* btn_cont = lv_obj_create(screen);
	lv_obj_t* list = lv_obj_create(screen);

	lv_obj_t* home_btn = lv_btn_create(btn_cont);
	lv_obj_t* home_label = lv_label_create(home_btn);
	lv_obj_t* back_btn = lv_btn_create(btn_cont);
	lv_obj_t* back_label = lv_label_create(back_btn);

	shd_debug_ctx_t* ctx = malloc(sizeof(shd_debug_ctx_t));

	activity_ctx->activity_user = ctx;
	ctx->activity_ctx = activity_ctx;
	ctx->list = list;

	lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER);

	lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_width(btn_cont, lv_pct(95));

	lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_width(list, lv_pct(95));
	lv_obj_set_height(list, 1000);

	lv_label_set_text(home_label, "HOME");
	lv_obj_add_event_cb(home_btn, shd_debug_home_cb, LV_EVENT_CLICKED, activity_ctx);

	lv_label_set_text(back_label, "BACK");
	lv_obj_add_event_cb(back_btn, shd_debug_back_cb, LV_EVENT_CLICKED, activity_ctx);
}

void shd_debug_main_resume(shd_act_ctx_t* activity_ctx) {
	shd_debug_ctx_t* ctx = (shd_debug_ctx_t*)activity_ctx->activity_user;

	ctx->timer = lv_timer_create(shd_debug_refresh_activities, 5000, ctx);
	lv_timer_ready(ctx->timer);
}
void shd_debug_main_pause(shd_act_ctx_t* activity_ctx) {
	shd_debug_ctx_t* ctx = (shd_debug_ctx_t*)activity_ctx->activity_user;
	
	lv_timer_delete(ctx->timer);
}

shd_intent_filter_t shd_debug_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_DEBUG
};
shd_intent_filter_node_t shd_debug_intent_filter_node = { 
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
