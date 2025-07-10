#pragma once

#include "activity.h"
#include "intent.h"

#include "app.h"

typedef struct activity_manager_ctx {
	activity_t* activity;
	activity_result_callback cb;

	lv_obj_t* prev;
	lv_obj_t* screen;
} activity_manager_ctx;

bool is_intent_filter_match(intent_filter_t* intent_filter, intent_t* intent);
activity_t* search_intent_filters(app_t* apps, size_t size, bool (*func)(intent_filter_t*, void*), void* user);

int start_activity(activity_t* activity, activity_result_callback cb);
int start_activity_from_intent(app_t* apps, size_t size, intent_t* intent, activity_result_callback cb);
int start_home_activity(app_t* apps, size_t size);
