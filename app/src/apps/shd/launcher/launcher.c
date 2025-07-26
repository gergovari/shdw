#include "launcher.h"

#include <stdlib.h>
#include <stdio.h>

#include <zephyr/random/random.h>

#include <lvgl.h>

void shd_launcher_entry_click_cb(lv_event_t *e) {
	shd_launcher_entry_ctx_t* ctx = (shd_launcher_entry_ctx_t*)lv_event_get_user_data(e);
	
	shd_act_man_act_launch(ctx->manager, ctx->app, ctx->activity, ctx->display, NULL, NULL, NULL);
}

void shd_launcher_entry_create(shd_launcher_entry_ctx_t* ctx) {
	lv_obj_t* btn = lv_list_add_button(ctx->list, NULL, ctx->app->title);;
	
	lv_obj_add_event_cb(btn, shd_launcher_entry_click_cb, LV_EVENT_CLICKED, ctx);
}

void shd_launcher_main_create(shd_act_ctx_t* activity_ctx) {
	shd_launcher_ctx_t* ctx = malloc(sizeof(shd_launcher_ctx_t));

	ctx->entries = NULL;
	ctx->random = sys_rand8_get();
	
	activity_ctx->activity_user = ctx;
}
void shd_launcher_main_destroy(shd_act_ctx_t* activity_ctx) {
	shd_launcher_ctx_t* ctx = (shd_launcher_ctx_t*)activity_ctx->activity_user;

	free(ctx);
}

void shd_launcher_main_start(shd_act_ctx_t* activity_ctx) {
	shd_launcher_ctx_t* ctx = (shd_launcher_ctx_t*)activity_ctx->activity_user;

	lv_obj_t* screen = activity_ctx->screen;
	lv_obj_t* list = lv_list_create(screen);
	lv_obj_t* error_label;
	
	shd_act_man_ctx_t* manager = activity_ctx->manager;
	shd_apps_t* apps = manager->apps;
	
	intent_t intent;
	intent_filter_result_node_t* intent_filter_result_node;
	intent_filter_result_node_t* next_intent_filter_result_node;
	intent_filter_result_t* intent_filter_result;

	shd_launcher_entry_ctx_t* entry_ctx;
	shd_app_t* app;
	shd_act_t* activity;
	
	lv_obj_t* rand_label = lv_list_add_text(list, "random");

	lv_label_set_text_fmt(rand_label, "%i (%p)", ctx->random, activity_ctx);
	
	intent.action = ACTION_MAIN;
	intent.category = CATEGORY_LAUNCHER;
	intent.activity = NULL;

	intent_filter_result_node = search_intent_filters(apps, 
			(intent_filter_func_t)is_intent_filter_match, 
			&intent);

	if (intent_filter_result_node == NULL) {
		error_label = lv_label_create(screen);
		lv_obj_center(error_label);
		lv_label_set_text(error_label, "No launchable apps found!");
	} else {
		while (intent_filter_result_node != NULL) {
			intent_filter_result = &(intent_filter_result_node->intent_filter_result);
			app = intent_filter_result->app;
			activity = intent_filter_result->activity;
			next_intent_filter_result_node = intent_filter_result_node->next;
			free(intent_filter_result_node);
			intent_filter_result_node = next_intent_filter_result_node;

			entry_ctx = malloc(sizeof(shd_launcher_entry_ctx_t));
			entry_ctx->list = list;
			
			entry_ctx->manager = manager;
			entry_ctx->app = app;
			entry_ctx->activity = activity;

			entry_ctx->display = activity_ctx->display;

			shd_launcher_entry_create(entry_ctx);
			
			if (ctx->entries == NULL) {
				ctx->entries = malloc(sizeof(shd_launcher_entry_ctx_node_t));
			} else {
				ctx->entries->next = malloc(sizeof(shd_launcher_entry_ctx_node_t));
				ctx->entries = ctx->entries->next;
			}
			ctx->entries->value = entry_ctx;
		};
	}
}
void shd_launcher_main_stop(shd_act_ctx_t* activity_ctx) {
	shd_launcher_ctx_t* ctx = (shd_launcher_ctx_t*)activity_ctx->activity_user;
	shd_launcher_entry_ctx_node_t* node = ctx->entries;
	shd_launcher_entry_ctx_node_t* next_node;

	while (node != NULL) {
		next_node = node->next;
		free(node);
		node = next_node;
	}
}

intent_filter_t shd_launcher_intent_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_HOME
};
intent_filter_node_t shd_launcher_intent_filter_node = {
	.intent_filter = &shd_launcher_intent_filter, 
	.next = NULL
};
shd_act_t shd_launcher_main = {
	.id = "shd.launcher.main",
	.intent_filters = &shd_launcher_intent_filter_node,

	.on_create = shd_launcher_main_create,
	.on_destroy = shd_launcher_main_destroy,

	.on_start = shd_launcher_main_start,
	.on_restart = NULL,
	.on_stop = shd_launcher_main_stop,

	.on_resume = NULL,
	.on_pause = NULL 
};
shd_act_node_t shd_launcher_activity_node = { 
	.activity = &shd_launcher_main, 
	.next = NULL 
};
shd_app_t shd_launcher = {
	.id = "shd.launcher",
	.title = "Launcher",
	.activities = &shd_launcher_activity_node
};
