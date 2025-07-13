#pragma once

#include <lvgl.h>

#include "activity.h"
#include "intent.h"
#include "app.h"

typedef struct lv_screen_node_ts lv_screen_node_t;
struct lv_screen_node_ts {
	lv_obj_t* value;
	lv_screen_node_t* prev;
};

typedef struct activity_manager_ctx_t {
	activity_t* activity;
	lv_screen_node_t* prevs;
} activity_manager_ctx_t;

typedef struct activity_ctx_bundle_t {
	activity_manager_ctx_t* manager;
	activity_ctx_t* activity;
} activity_ctx_bundle_t;

typedef struct intent_filter_result_t {
	app_t* app;
	activity_t* activity;
} intent_filter_result_t;

typedef struct intent_filter_result_node_ts intent_filter_result_node_t;
struct intent_filter_result_node_ts {
	intent_filter_result_t intent_filter_result;
	intent_filter_result_node_t* next;
};

typedef bool (*intent_filter_func_t)(intent_filter_t* intent_filter, app_t* app, void* user);

bool is_intent_filter_match(intent_filter_t* intent_filter, app_t* app, intent_t* intent);
intent_filter_result_node_t* search_intent_filters(apps_t* apps, intent_filter_func_t func, void* user);

int start_activity(apps_t* apps, app_t* app, activity_t* activity, activity_result_callback_t cb, void* input, void* user, lv_display_t* display);
int start_activity_from_intent(apps_t* apps, intent_t* intent, activity_result_callback_t cb, lv_display_t* display);

int start_home_activity(apps_t* apps, lv_display_t* display);
int start_debug_activity(apps_t* apps, lv_display_t* display);
