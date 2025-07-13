#pragma once

#include <lvgl.h>

#include "intent.h"

typedef void (*activity_callback_t)(void* user, int result, void* data);
typedef void (*activity_result_callback_t)(int result, void* data, void* user);

typedef struct apps_t apps_t;
typedef struct activity_t activity_t;

typedef struct activity_ctx_t {
	apps_t* apps;
	void* user;

	lv_display_t* display;
	lv_obj_t* screen;
	
	activity_t* activity;
	activity_callback_t cb;
	activity_result_callback_t result_cb;
	void* return_user;

	void* input;
} activity_ctx_t;

typedef void (*activity_entry_t)(activity_ctx_t* ctx);
typedef void (*activity_exit_t)(activity_ctx_t* ctx);

typedef struct activity_t {
	char* id;
	intent_filter_node_t* intent_filters;
	
	activity_entry_t entry;
	activity_exit_t exit;
} activity_t;

typedef struct activity_node_ts activity_node_t;
struct activity_node_ts {
	activity_t* activity;
	activity_node_t* next;
};
