#pragma once

#include <lvgl.h>

typedef struct lv_launcher_ctx {
	lv_obj_t* parent;
	lv_obj_t* root;
	lv_obj_t* screen;

	void (*func)(lv_obj_t*);
	char *title;
} lv_launcher_ctx;

void lv_launcher_create(lv_obj_t* parent, 
		char** app_ids, 
		char** app_names, void 
		(**app_funcs)(lv_obj_t*), 
		unsigned int size);
