#pragma once

#include <stdbool.h>

#include "intent.h"
#include "app.h"
#include "activity.h"

typedef struct shd_intent_filter_ts shd_intent_filter_t;
struct shd_intent_filter_ts {
	shd_action_t action;
	shd_category_t category;

	char* type;
};

typedef struct shd_intent_filter_node_ts shd_intent_filter_node_t;
struct shd_intent_filter_node_ts {
	shd_intent_filter_t* intent_filter;
	shd_intent_filter_node_t* next;
};

typedef struct shd_intent_filter_result_t {
	shd_app_t* app;
	shd_act_t* activity;
} shd_intent_filter_result_t;

typedef struct shd_intent_filter_result_node_ts shd_intent_filter_result_node_t;
struct shd_intent_filter_result_node_ts {
	shd_intent_filter_result_t* value;
	shd_intent_filter_result_node_t* next;
};

typedef bool (*shd_intent_filter_func_t)(shd_intent_filter_t* intent_filter, shd_app_t* app, shd_intent_t* intent);

shd_intent_filter_result_node_t* shd_apps_intent_filter_search(shd_apps_t* apps, shd_intent_filter_func_t func, shd_intent_t* intent);
void shd_intent_filter_results_destroy(shd_intent_filter_result_node_t* node);

bool shd_act_in_category_is(shd_act_t* activity, shd_category_t category);
bool shd_act_has_action_is(shd_act_t* activity, shd_action_t action);

bool shd_intent_filter_match_is(shd_intent_filter_t* intent_filter, shd_app_t* app, shd_intent_t* intent);
