#pragma once

#include <lvgl.h>

#include "../../../app.h"
#include "../../../activity_manager.h"
#include "../../../activity.h"

extern app_t shd_launcher;

typedef struct shd_launcher_entry_ctx_t {
	lv_obj_t* list;

	apps_t* apps;
	app_t* app;
	activity_t* activity;
} shd_launcher_entry_ctx_t;

typedef struct shd_launcher_entry_ctx_node_ts shd_launcher_entry_ctx_node_t;
struct shd_launcher_entry_ctx_node_ts {
	shd_launcher_entry_ctx_t* value;
	shd_launcher_entry_ctx_node_t* next;
};

typedef struct shd_launcher_ctx_t {
	int8_t random;

	shd_launcher_entry_ctx_node_t* entries;
} shd_launcher_ctx_t;

void shd_launcher_main_entry(activity_ctx_t* ctx);
void shd_launcher_main_exit(activity_ctx_t* ctx);
