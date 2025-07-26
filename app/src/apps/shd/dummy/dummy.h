#pragma once

#include <lvgl.h>

#include "../../../app.h"
#include "../../../activity_ctx.h"

extern shd_app_t shd_dummy;

typedef struct shd_dummy_ctx_t {
	shd_act_ctx_t* activity_ctx;
	int8_t* random;

	lv_obj_t* list;
} shd_dummy_ctx_t;

void shd_dummy_main_entry(shd_act_ctx_t* ctx);
void shd_dummy_main_exit(shd_act_ctx_t* ctx);
