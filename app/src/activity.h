#pragma once

#include <lvgl.h>

#include "intent.h"

typedef struct activity_t {
	char* id;
	intent_filter_node_t* intent_filters;

	void (*entry)(lv_obj_t*);
	void (*exit)(lv_obj_t*);
} activity_t;

typedef struct activity_node_ts activity_node_t;
struct activity_node_ts {
	activity_t* activity;
	activity_node_t* next;
};
