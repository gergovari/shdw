#pragma once

#include "intent.h"

typedef struct shd_act_ctx_ts shd_act_ctx_t;

typedef void (*shd_act_func_t)(shd_act_ctx_t* ctx);

typedef struct shd_act_t {
	char* id;
	intent_filter_node_t* intent_filters;
	
	shd_act_func_t on_create;
	shd_act_func_t on_destroy;

	shd_act_func_t on_start;
	shd_act_func_t on_restart;
	shd_act_func_t on_stop;

	shd_act_func_t on_pause;
	shd_act_func_t on_resume;
} shd_act_t;

typedef struct shd_act_node_ts shd_act_node_t;
struct shd_act_node_ts {
	shd_act_t* activity;
	shd_act_node_t* next;
};
