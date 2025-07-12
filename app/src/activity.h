#pragma once

#include <lvgl.h>

#include "intent.h"

typedef void (*activity_callback)(void* user, int result, void* data);
typedef void (*activity_result_callback)(int result, void* data, void* user);

typedef void (*activity_entry)(lv_obj_t* screen, activity_callback cb, void* input, void* user);
typedef void (*activity_exit)(lv_obj_t* screen);

typedef struct activity_t {
	char* id;
	intent_filter_node_t* intent_filters;
	
	activity_entry entry;
	activity_exit exit;
} activity_t;

typedef struct activity_node_ts activity_node_t;
struct activity_node_ts {
	activity_t* activity;
	activity_node_t* next;
};
