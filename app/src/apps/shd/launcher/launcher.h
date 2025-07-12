#pragma once

#include <lvgl.h>

#include "../../../app.h"
#include "../../../activity_manager.h"

extern app_t shd_launcher;

typedef struct shd_launcher_entry_ctx_t {
	lv_obj_t* root;

	app_t* app;
	activity_t* activity;
} shd_launcher_entry_ctx_t;

typedef struct shd_launcher_entry_ctx_node_ts shd_launcher_entry_ctx_node_t;
struct shd_launcher_entry_ctx_node_ts {
	shd_launcher_entry_ctx_t* value;
	shd_launcher_entry_ctx_node_t* next;
};

typedef struct shd_launcher_ctx_t {
	shd_launcher_entry_ctx_node_t* entries;
} shd_launcher_ctx_t;

void shd_launcher_main_entry(lv_obj_t* screen, activity_callback cb, void* input, void* user);
void shd_launcher_main_exit(lv_obj_t* screen);
