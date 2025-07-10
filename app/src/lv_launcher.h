#pragma once

#include <lvgl.h>

#include "app.h"
#include "activity_manager.h"

extern app_t lv_launcher;

typedef struct lv_launcher_entry_ctx {
	lv_obj_t* root;

	app_t* app;
	activity_t* activity;
} lv_launcher_entry_ctx;

void lv_launcher_main_entry(lv_obj_t* screen, activity_callback cb, void* user);
void lv_launcher_main_exit(lv_obj_t* screen);
