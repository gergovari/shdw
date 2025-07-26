#include "dummy.h"

#include <stdio.h>
#include <stdlib.h>

#include <zephyr/random/random.h>

#include "../../../activity_manager.h"

void shd_dummy_return_cb(int result, void* data, void* user) {
	shd_dummy_ctx_t* ctx = (shd_dummy_ctx_t*)user;
	int8_t* random = (int8_t*)data;
	
	printf("dummy received: %i\n", *random);
	lv_obj_t* return_label = lv_list_add_text(ctx->list, "return");
	lv_label_set_text_fmt(return_label, "return: %i, %i", result, *random);

	free(random);
}

void shd_dummy_new_cb(lv_event_t* e) {
	shd_dummy_ctx_t* ctx = (shd_dummy_ctx_t*)lv_event_get_user_data(e);
	shd_act_ctx_t* activity_ctx = ctx->activity_ctx;
	intent_t intent;
	int ret;

	intent.activity = "shd.dummy.new";
	intent.user = ctx;
	
	printf("launching new dummy (%p)!\n", activity_ctx);
	ret = shd_act_man_act_launch_from_intent(activity_ctx->manager, activity_ctx->display, &intent, (shd_act_result_cb_t)shd_dummy_return_cb);
	if (ret != 0) {
		printf("dummy (%p) couldn't start new activity! (ret: %i)\n", activity_ctx, ret);
	}
}
void shd_dummy_exit_cb(lv_event_t* e) {
	shd_dummy_ctx_t* ctx = (shd_dummy_ctx_t*)lv_event_get_user_data(e);
	shd_act_ctx_t* activity_ctx = ctx->activity_ctx;
	shd_act_cb_t cb = activity_ctx->cb;
	void* user = activity_ctx->user;

	printf("dummy (%p) sending: %i\n", activity_ctx, *(ctx->random));
	cb(user, 0, (void*)ctx->random);
}

void shd_dummy_new_start(shd_act_ctx_t* activity_ctx) {
	lv_obj_t* screen = activity_ctx->screen;
	shd_dummy_ctx_t* ctx = (shd_dummy_ctx_t*)activity_ctx->activity_user;

	lv_obj_t* list;
	lv_obj_t* rand_label;
	lv_obj_t* new_btn;
	lv_obj_t* back_btn;

	list = lv_list_create(screen);
	ctx->list = list;

	rand_label = lv_list_add_text(list, "random");
	new_btn = lv_list_add_button(list, NULL, "New");
	back_btn = lv_list_add_button(list, NULL, "Back");

	lv_obj_add_event_cb(new_btn, shd_dummy_new_cb, LV_EVENT_CLICKED, ctx);
	lv_obj_add_event_cb(back_btn, shd_dummy_exit_cb, LV_EVENT_CLICKED, ctx);

	lv_label_set_text_fmt(rand_label, "%i", *(ctx->random));

	printf("new dummy start (%p)\n", activity_ctx);
}

void shd_dummy_main_create(shd_act_ctx_t* activity_ctx) {
	shd_dummy_ctx_t* ctx = malloc(sizeof(shd_dummy_ctx_t));

	ctx->random = malloc(sizeof(int8_t));
	*(ctx->random) = sys_rand8_get();

	ctx->activity_ctx = activity_ctx;
	activity_ctx->activity_user = ctx;

	printf("dummy create (%p)\n", activity_ctx);
}
void shd_dummy_main_destroy(shd_act_ctx_t* activity_ctx) {
	shd_dummy_ctx_t* ctx = (shd_dummy_ctx_t*)activity_ctx->activity_user;
	
	free(ctx->random);
	free(ctx);

	printf("dummy destroy (%p)\n", activity_ctx);
}

void shd_dummy_main_start(shd_act_ctx_t* activity_ctx) {
	lv_obj_t* screen = activity_ctx->screen;
	shd_dummy_ctx_t* ctx = (shd_dummy_ctx_t*)activity_ctx->activity_user;

	lv_obj_t* list;
	lv_obj_t* rand_label;
	lv_obj_t* new_btn;
	lv_obj_t* exit_btn;

	list = lv_list_create(screen);
	ctx->list = list;

	rand_label = lv_list_add_text(list, "random");
	new_btn = lv_list_add_button(list, NULL, "New");
	exit_btn = lv_list_add_button(list, NULL, "Exit");

	lv_obj_add_event_cb(new_btn, shd_dummy_new_cb, LV_EVENT_CLICKED, ctx);
	lv_obj_add_event_cb(exit_btn, shd_dummy_exit_cb, LV_EVENT_CLICKED, ctx);

	lv_label_set_text_fmt(rand_label, "%i", *(ctx->random));

	printf("dummy start (%p)\n", activity_ctx);
}
void shd_dummy_restart(shd_act_ctx_t* activity_ctx) {
	printf("dummy restart (%p)\n", activity_ctx);
}
void shd_dummy_stop(shd_act_ctx_t* activity_ctx) {
	printf("dummy stop (%p)\n", activity_ctx);
}

void shd_dummy_resume(shd_act_ctx_t* activity_ctx) {
	printf("dummy resume (%p)\n", activity_ctx);
}
void shd_dummy_pause(shd_act_ctx_t* activity_ctx) {
	printf("dummy pause (%p)\n", activity_ctx);
}

intent_filter_t shd_dummy_main_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_LAUNCHER
};

intent_filter_node_t shd_dummy_main_intent_filter_node = { 
	.intent_filter = &shd_dummy_main_filter, 
	.next = NULL
};

shd_act_t shd_dummy_main = {
	.id = "shd.dummy.main",
	.intent_filters = &shd_dummy_main_intent_filter_node,

	.on_create = shd_dummy_main_create,
	.on_destroy = shd_dummy_main_destroy,

	.on_start = shd_dummy_main_start,
	.on_restart = shd_dummy_restart,
	.on_stop = shd_dummy_stop,

	.on_resume = shd_dummy_resume,
	.on_pause = shd_dummy_pause 
};
shd_act_t shd_dummy_new = {
	.id = "shd.dummy.new",

	.on_create = shd_dummy_main_create,
	.on_destroy = shd_dummy_main_destroy,

	.on_start = shd_dummy_new_start,
	.on_restart = shd_dummy_restart,
	.on_stop = shd_dummy_stop,

	.on_resume = shd_dummy_resume,
	.on_pause = shd_dummy_pause 
};

shd_act_node_t shd_dummy_new_activity_node = { 
	.activity = &shd_dummy_new, 
	.next = NULL 
};
shd_act_node_t shd_dummy_main_activity_node = { 
	.activity = &shd_dummy_main, 
	.next = &shd_dummy_new_activity_node
};

shd_app_t shd_dummy = {
	.id = "shd.dummy",
	.title = "Dummy",
	.activities = &shd_dummy_main_activity_node
};
