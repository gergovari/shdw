#pragma once

#include <lvgl.h>

#include "app.h"
#include "activity.h"

typedef void (*shd_act_cb_t)(void* user, int result, void* data);
typedef void (*shd_act_result_cb_t)(int result, void* data, void* user);

typedef struct shd_act_man_ctx_t shd_act_man_ctx_t;

typedef enum {
	INITIALIZED_DESTROYED,
	CREATED_STOPPED,
	STARTED_PAUSED,
	RESUMED,
} shd_act_state_t;

typedef struct shd_act_ctx_ts shd_act_ctx_t;
struct shd_act_ctx_ts {
	shd_act_man_ctx_t* manager;

	lv_display_t* display;
	lv_obj_t* screen;
	lv_draw_buf_t* snapshot;
	
	shd_app_t* app;
	shd_act_t* activity;
	shd_act_ctx_t* prev;
	shd_act_state_t state;

	shd_act_cb_t cb;
	void* user;

	shd_act_result_cb_t result_cb;
	void* return_user;

	void* input;
	void* activity_user;
};

int shd_act_ctx_take_snapshot(shd_act_ctx_t* ctx);

int shd_act_ctx_screen_create(shd_act_ctx_t* ctx);
int shd_act_ctx_screen_destroy(shd_act_ctx_t* ctx);

int shd_act_ctx_create(shd_act_ctx_t* ctx);
int shd_act_ctx_destroy(shd_act_ctx_t* ctx);

int shd_act_ctx_start(shd_act_ctx_t* ctx);
int shd_act_ctx_stop(shd_act_ctx_t* ctx);

int shd_act_ctx_resume(shd_act_ctx_t* ctx);
int shd_act_ctx_pause(shd_act_ctx_t* ctx);

int shd_act_ctx_state_transition(shd_act_ctx_t* ctx, shd_act_state_t target);
