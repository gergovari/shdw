#pragma once

#include <lvgl.h>

#include "app.h"

typedef struct lv_launcher_ctx {
	lv_obj_t* parent;
	lv_obj_t* root;
	lv_obj_t* screen;
	lv_obj_t* cont;
	
	app_t* app;
} lv_launcher_ctx;

void lv_launcher_create(lv_obj_t* parent, 
		app_t* apps,
		unsigned int size);
