#pragma once

#include <lvgl.h>

#include "../../../app.h"
#include "../../../activity.h"

extern app_t shd_dummy;

typedef struct shd_dummy_ctx_t {
	activity_ctx_t* activity_ctx;
	int8_t* random;

	lv_obj_t* list;
} shd_dummy_ctx_t;

void shd_dummy_main_entry(activity_ctx_t* ctx);
void shd_dummy_main_exit(activity_ctx_t* ctx);
