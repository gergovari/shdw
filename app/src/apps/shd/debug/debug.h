#pragma once

#include "../../../app.h"
#include "../../../activity_ctx.h"

extern shd_app_t shd_debug;

typedef struct {
	lv_timer_t* timer;
	lv_obj_t* act_list;

	lv_obj_t* display_list;

	shd_act_ctx_t* activity_ctx;

	lv_display_t* display;
} shd_debug_ctx_t;

void shd_debug_main_entry(shd_act_ctx_t* ctx);
void shd_debug_main_exit(shd_act_ctx_t* ctx);
