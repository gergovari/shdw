#pragma once

#include <lvgl.h>

#include "app.h"
#include "activity_ctx.h"

#include "intent.h"

typedef struct shd_act_ctx_node_ts shd_act_ctx_node_t;
struct shd_act_ctx_node_ts {
	shd_act_ctx_t* value;
	shd_act_ctx_node_t* prev;
};

typedef struct {
	lv_display_t* display;
	shd_act_ctx_t* ctx;
} shd_display_act_ctx_entry_t;

typedef struct shd_display_act_ctx_entry_node_ts shd_display_act_ctx_entry_node_t;
struct shd_display_act_ctx_entry_node_ts {
	shd_display_act_ctx_entry_t* value;
	shd_display_act_ctx_entry_node_t* prev;
};

typedef struct shd_act_man_ctx_t {
	shd_apps_t* apps;

	shd_display_act_ctx_entry_node_t* current_activities;
	shd_act_ctx_node_t* activities;
} shd_act_man_ctx_t;

shd_act_man_ctx_t* shd_act_man_create(shd_apps_t* apps);
void shd_act_man_destroy(shd_act_man_ctx_t* manager);

int shd_act_man_act_launch(shd_act_man_ctx_t* manager, shd_app_t* app, shd_act_t* activity, lv_display_t* display, shd_act_result_cb_t cb, void* input, void* user);
int shd_act_man_act_launch_from_intent(shd_act_man_ctx_t* manager, lv_display_t* display, shd_intent_t* intent, shd_act_result_cb_t cb);

int shd_act_man_home_launch(shd_act_man_ctx_t* manager, lv_display_t* display);
int shd_act_man_debug_launch(shd_act_man_ctx_t* manager, lv_display_t* display);

int shd_act_man_back_go(shd_act_man_ctx_t* manager, lv_display_t* display);
int shd_act_man_home_go(shd_act_man_ctx_t* manager, lv_display_t* display);
