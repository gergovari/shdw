#include "launcher.h"

#include <stdlib.h>
#include <stdio.h>

#include <lvgl.h>

void shd_launcher_entry_click_cb(lv_event_t *e) {
	shd_launcher_entry_ctx_t* ctx = (shd_launcher_entry_ctx_t*)lv_event_get_user_data(e);
	
	start_activity(ctx->apps, ctx->app, ctx->activity, NULL, NULL, NULL, NULL);
}

void shd_launcher_entry_create(shd_launcher_entry_ctx_t* ctx) {
	lv_obj_t* cont = lv_menu_cont_create(ctx->root);
	lv_obj_t* btn = lv_button_create(cont);
	lv_obj_t* label = lv_label_create(btn);
	
	lv_label_set_text(label, ctx->app->title);
	lv_obj_add_event_cb(btn, shd_launcher_entry_click_cb, LV_EVENT_CLICKED, ctx);
}

void shd_launcher_main_entry(activity_ctx_t* activity_ctx) {
	lv_obj_t* screen = activity_ctx->screen;
	lv_obj_t* menu = lv_menu_create(screen);
	lv_obj_t* root = lv_menu_page_create(menu, NULL);
	lv_obj_t* error_label;

	shd_launcher_ctx_t* ctx = malloc(sizeof(shd_launcher_ctx_t));
	apps_t* apps = (apps_t*)activity_ctx->apps;
	
	intent_t intent;
	intent_filter_result_node_t* intent_filter_result_node;
	intent_filter_result_node_t* next_intent_filter_result_node;
	intent_filter_result_t* intent_filter_result;

	shd_launcher_entry_ctx_t* entry_ctx;
	app_t* app;
	activity_t* activity;
	
	lv_obj_set_user_data(screen, ctx);
	ctx->entries = NULL;

	lv_obj_center(menu);
	
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
			entry_ctx->root = root;
			entry_ctx->apps = apps;
			entry_ctx->app = app;
			entry_ctx->activity = activity;

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

	lv_menu_set_page(menu, root);
}

void shd_launcher_main_exit(activity_ctx_t* activity_ctx) {
	shd_launcher_ctx_t* ctx = (shd_launcher_ctx_t*)lv_obj_get_user_data(activity_ctx->screen);
	
	shd_launcher_entry_ctx_node_t* node = ctx->entries;
	shd_launcher_entry_ctx_node_t* next_node;

	while (node != NULL) {
		next_node = node->next;
		free(node);
		node = next_node;
	}

	free(ctx);
}

intent_filter_t shd_launcher_intent_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_HOME
};
intent_filter_node_t shd_launcher_intent_filter_node = {
	.intent_filter = &shd_launcher_intent_filter, 
	.next = NULL
};
activity_t shd_launcher_main = {
	.id = "shd.launcher.main",
	.intent_filters = &shd_launcher_intent_filter_node,
	.entry = shd_launcher_main_entry,
	.exit = shd_launcher_main_exit
};
activity_node_t shd_launcher_activity_node = { 
	.activity = &shd_launcher_main, 
	.next = NULL 
};
app_t shd_launcher = {
	.id = "shd.launcher",
	.title = "Launcher",
	.activities = &shd_launcher_activity_node
};
