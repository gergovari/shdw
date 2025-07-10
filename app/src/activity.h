#pragma once

#include <lvgl.h>

#include "intent.h"

typedef void (*activity_callback)(void* user, int result, void* data);
typedef void (*activity_result_callback)(int result, void* data);

typedef struct activity_t {
	char* id;
	intent_filter_node_t* intent_filters;

	void (*entry)(lv_obj_t*, activity_callback, void*);
	void (*exit)(lv_obj_t*);
} activity_t;

typedef struct activity_node_ts activity_node_t;
struct activity_node_ts {
	activity_t* activity;
	activity_node_t* next;
};
