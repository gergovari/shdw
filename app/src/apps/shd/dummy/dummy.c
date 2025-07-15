#include "dummy.h"

#include <stdio.h>
#include <stdlib.h>

#include <zephyr/random/random.h>

#include "../../../activity_manager.h"

void shd_dummy_return_cb(int result, void* data, void* user) {
	shd_dummy_ctx_t* ctx = (shd_dummy_ctx_t*)user;
	int8_t* random = (int8_t*)data;
	
	printf("dummy received: %i (list: %p)\n", *random, ctx->list);
	lv_obj_t* return_label = lv_list_add_text(ctx->list, "return");
	lv_label_set_text_fmt(return_label, "return: %i, %i", result, *random);

	free(random);
}

void shd_dummy_new_cb(lv_event_t* e) {
	shd_dummy_ctx_t* ctx = (shd_dummy_ctx_t*)lv_event_get_user_data(e);
	activity_ctx_t* activity_ctx = ctx->activity_ctx;
	intent_t intent;
	int ret;

	intent.activity = "shd.dummy.main"; // TODO: don't hardcode id
	intent.user = ctx;

	ret = start_activity_from_intent(activity_ctx->apps, &intent, (activity_result_callback_t)shd_dummy_return_cb, activity_ctx->display);
	if (ret != 0) {
		printf("dummy couldn't start new activity!\n");
	}
}

void shd_dummy_exit_cb(lv_event_t* e) {
	shd_dummy_ctx_t* ctx = (shd_dummy_ctx_t*)lv_event_get_user_data(e);
	activity_ctx_t* activity_ctx = ctx->activity_ctx;
	activity_callback_t cb = activity_ctx->cb;
	void* user = activity_ctx->user;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
	printf("dummy sending: %i (list: %p)\n", *(ctx->random), ctx->list);
	cb(user, 0, (void*)ctx->random);
#pragma GCC diagnostic pop
}

void shd_dummy_main_entry(activity_ctx_t* activity_ctx) {
	shd_dummy_ctx_t* ctx = malloc(sizeof(shd_dummy_ctx_t));

	lv_obj_t* screen = activity_ctx->screen;
	lv_obj_t* list;
	lv_obj_t* rand_label;
	lv_obj_t* new_btn;
	lv_obj_t* exit_btn;

	list = lv_list_create(screen);
	ctx->list = list;

	rand_label = lv_list_add_text(list, "random");
	new_btn = lv_list_add_button(list, NULL, "New");
	exit_btn = lv_list_add_button(list, NULL, "Exit");

	lv_obj_set_user_data(screen, ctx);
	ctx->random = malloc(sizeof(int8_t));
	*(ctx->random) = sys_rand8_get();
	ctx->activity_ctx = activity_ctx;

	lv_obj_add_event_cb(new_btn, shd_dummy_new_cb, LV_EVENT_CLICKED, ctx);
	lv_obj_add_event_cb(exit_btn, shd_dummy_exit_cb, LV_EVENT_CLICKED, ctx);

	lv_label_set_text_fmt(rand_label, "%i", *(ctx->random));

	printf("dummy opened! (list: %p)\n", ctx->list);
}
void shd_dummy_main_exit(activity_ctx_t* activity_ctx) {
	shd_dummy_ctx_t* ctx = (shd_dummy_ctx_t*)lv_obj_get_user_data(activity_ctx->screen);

	free(ctx);

	// TODO: proper logging for apps
	printf("dummy closed.\n");
}

intent_filter_t shd_dummy_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_LAUNCHER
};
intent_filter_node_t shd_dummy_intent_filter_node = { 
	.intent_filter = &shd_dummy_filter, 
	.next = NULL 
};
activity_t shd_dummy_main = {
	.id = "shd.dummy.main",
	.intent_filters = &shd_dummy_intent_filter_node,
	.entry = shd_dummy_main_entry,
	.exit = shd_dummy_main_exit
};
activity_node_t shd_dummy_activity_node = { 
	.activity = &shd_dummy_main, 
	.next = NULL 
};
app_t shd_dummy = {
	.id = "shd.dummy",
	.title = "Dummy",
	.activities = &shd_dummy_activity_node
};
