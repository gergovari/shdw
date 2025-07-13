#pragma once

#include "activity.h"
#include "intent.h"

#include "app.h"

typedef struct activity_manager_ctx {
	activity_t* activity;
	activity_result_callback cb;
	void* user;
	
	lv_obj_t* prev;
	lv_obj_t* screen;
	lv_obj_t* cont;
} activity_manager_ctx;

typedef struct intent_filter_result_t {
	app_t* app;
	activity_t* activity;
} intent_filter_result_t;

typedef struct intent_filter_result_node_ts intent_filter_result_node_t;
struct intent_filter_result_node_ts {
	intent_filter_result_t intent_filter_result;
	intent_filter_result_node_t* next;
};

bool is_intent_filter_match(intent_filter_t* intent_filter, intent_t* intent);
intent_filter_result_node_t* search_intent_filters(apps_t* apps, bool (*func)(intent_filter_t*, void*), void* user);

int start_activity(app_t* app, activity_t* activity, activity_result_callback cb, void* input, void* user, lv_display_t* display);
int start_activity_from_intent(apps_t* apps, intent_t* intent, activity_result_callback cb, lv_display_t* display);

int start_home_activity(apps_t* apps, lv_display_t* display);
int start_debug_activity(apps_t* apps, lv_display_t* display);
