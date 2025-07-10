#include "lv_launcher.h"

#include <stdlib.h>
#include <stdio.h>

#include <lvgl.h>

#include "activity_manager.h"

void lv_launcher_entry_click_cb(lv_event_t *e) {
	lv_launcher_entry_ctx* ctx = (lv_launcher_entry_ctx*)lv_event_get_user_data(e);
	
	start_activity(ctx->app, ctx->activity, NULL, NULL);
}

void lv_launcher_entry_create(lv_launcher_entry_ctx* ctx) {
	lv_obj_t* cont = lv_menu_cont_create(ctx->root);
	lv_obj_t* btn = lv_button_create(cont);
	lv_obj_t* label = lv_label_create(btn);

	lv_obj_add_event_cb(btn, lv_launcher_entry_click_cb, LV_EVENT_CLICKED, ctx);
	lv_label_set_text(label, ctx->app->title);
}

void lv_launcher_main_entry(lv_obj_t* screen, activity_callback cb, void* user) {
	lv_obj_t* menu = lv_menu_create(screen);
	lv_obj_t* root = lv_menu_page_create(menu, NULL);
	lv_obj_t* error_label;

	/*lv_launcher_ctx ctx;
	lv_obj_set_user_data(screen, ctx);*/
	apps_t* apps = (apps_t*)user;
	
	intent_t intent;
	intent_filter_result_node_t* intent_filter_result_node;
	intent_filter_result_node_t* old_intent_filter_result_node;
	intent_filter_result_t* intent_filter_result;

	lv_launcher_entry_ctx* entry_ctx;
	app_t* app;
	activity_t* activity;
	
	lv_obj_set_size(menu, 
			lv_display_get_horizontal_resolution(NULL), 
			lv_display_get_vertical_resolution(NULL));
	lv_obj_center(menu);
	
	intent.action = ACTION_MAIN;
	intent.category = CATEGORY_LAUNCHER;
	intent_filter_result_node = search_intent_filters(apps, 
			(bool (*)(intent_filter_t*, void*))is_intent_filter_match, 
			&intent);

	if (intent_filter_result_node == NULL) {
		error_label = lv_label_create(screen);
		lv_obj_center(error_label);
		lv_label_set_text(error_label, "No launchable apps found!");
	} else {
		do {
			intent_filter_result = &(intent_filter_result_node->intent_filter_result);
			app = intent_filter_result->app;
			activity = intent_filter_result->activity;
			old_intent_filter_result_node = intent_filter_result_node;
			intent_filter_result_node = intent_filter_result_node->next;
			free(intent_filter_result_node);

			entry_ctx = malloc(sizeof(lv_launcher_entry_ctx));
			entry_ctx->root = root;
			entry_ctx->app = app;
			entry_ctx->activity = activity;

			lv_launcher_entry_create(entry_ctx);
			// TODO: add entry_ctx to widget ctx
		} while (intent_filter_result_node != NULL);
	}

	lv_menu_set_page(menu, root); }

void lv_launcher_main_exit(lv_obj_t* screen) {
	// TODO: free all entry_ctxs
}

intent_filter_t lv_launcher_intent_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_HOME
};
intent_filter_node_t lv_launcher_intent_filter_node = {
	.intent_filter = &lv_launcher_intent_filter, 
	.next = NULL
};
activity_t lv_launcher_main = {
	.id = "lv.launcher.main",
	.intent_filters = &lv_launcher_intent_filter_node,
	.entry = lv_launcher_main_entry,
	.exit = lv_launcher_main_exit
};
activity_node_t lv_launcher_activity_node = { 
	.activity = &lv_launcher_main, 
	.next = NULL 
};
app_t lv_launcher = {
	.id = "lv.launcher",
	.title = "Launcher",
	.activities = &lv_launcher_activity_node
};
