#pragma once

#include <lvgl.h>

#include "../../../app.h"
#include "../../../activity_manager.h"
#include "../../../activity.h"

extern shd_app_t shd_launcher;

typedef struct shd_launcher_entry_ctx_t {
	lv_obj_t* list;

	shd_app_t* app;
	shd_act_t* activity;
	
	shd_act_man_ctx_t* manager;
	lv_display_t* display;
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

void shd_launcher_main_entry(shd_act_ctx_t* ctx);
void shd_launcher_main_exit(shd_act_ctx_t* ctx);
