#include "activity_manager.h"

#include <lvgl.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

intent_filter_result_node_t* search_intent_filters(apps_t* apps, 
		bool (*func)(intent_filter_t*, app_t*, void*), void* user) {
	activity_node_t* activity_node;
	activity_t* activity;
	app_t* app;

	intent_filter_node_t* intent_filter_node;
	intent_filter_t* intent_filter;

	intent_filter_result_node_t* final_node = NULL;
	intent_filter_result_node_t* intent_filter_result_node = NULL;
	intent_filter_result_t intent_filter_result;

	for (size_t i = 0; i < apps->size; i++) {
		app = apps->list[i];
		activity_node = app->activities;
		
		while (activity_node != NULL) {
			activity = activity_node->activity;
			intent_filter_node = activity->intent_filters;

			while (intent_filter_node != NULL) {
				intent_filter = intent_filter_node->intent_filter;

				if (func(intent_filter, app, user)) {
					if (intent_filter_result_node == NULL) {
						intent_filter_result_node = malloc(sizeof(intent_filter_result_node_t));
					} else {
						intent_filter_result_node->next = malloc(sizeof(intent_filter_result_node_t));
						intent_filter_result_node = intent_filter_result_node->next;
					}

					intent_filter_result_node->next = NULL;
					intent_filter_result.app = app;
					intent_filter_result.activity = activity;
					intent_filter_result_node->intent_filter_result = intent_filter_result;

					if (final_node == NULL) final_node = intent_filter_result_node;
				};

				intent_filter_node = intent_filter_node->next;
			};

			activity_node = activity_node->next;
		};
	}
	return final_node;
}

bool is_activity_in_category(activity_t* activity, category_t category) {
	intent_filter_node_t* intent_filter_node = activity->intent_filters;
	intent_filter_t* intent_filter;

	while (intent_filter_node != NULL) {
		intent_filter = intent_filter_node->intent_filter;
		
		if ((intent_filter->category & category) >= category) {
			return true;
		}

		intent_filter_node = intent_filter_node->next;
	}

	return false;
}

bool is_intent_filter_match(intent_filter_t* intent_filter, app_t* app, intent_t* intent) {
	activity_node_t* node;
	activity_t* activity;

	if (intent->activity == NULL) {
		// TODO: data check
		if (
			(intent_filter->action & intent->action) > 0 && 
			(intent_filter->category & intent->category) >= intent->category
		) return true;	
		
	} else {
		node = app->activities;
		
		while (node != NULL) {
			activity = node->activity;
			if (strcmp(intent->activity, activity->id) == 0) return true;
			node = node->next;
		}

	}

	return false;
}

void close_activity(activity_ctx_bundle_t* ctx_bundle) {
	activity_manager_ctx_t* manager = ctx_bundle->manager;
	activity_ctx_t* activity_ctx = ctx_bundle->activity;
	activity_t* activity = activity_ctx->activity;

	if (manager->prevs == NULL) {
		// TODO: start_home_activity();
	} else {
		lv_screen_load(manager->prevs->value);
		manager->prevs = manager->prevs->prev;
	}

	activity->exit(activity_ctx);

	lv_obj_delete(activity_ctx->screen);
}

// TODO: void pause_activity...

void finished_activity_cb(activity_ctx_bundle_t* ctx_bundle, int result, void* data) {
	activity_ctx_t* ctx = ctx_bundle->activity;
	activity_result_callback_t cb = ctx->result_cb;
	
	close_activity(ctx_bundle);
	if (cb != NULL) cb(result, data, ctx->return_user);

	free(ctx);
	free(ctx_bundle);
}

void activity_event_handler(lv_event_t* e) {
	activity_ctx_bundle_t* ctx_bundle = (activity_ctx_bundle_t*)lv_event_get_user_data(e);
	
	close_activity(ctx_bundle);
}

// TODO: better activity lifecycles to have enough memory...
int start_activity(apps_t* apps, app_t* app, activity_t* activity, activity_result_callback_t cb, void* input, void* user, lv_display_t* display) {
	activity_manager_ctx_t* ctx; 
	activity_ctx_t* activity_ctx = malloc(sizeof(activity_ctx_t));
	activity_ctx_bundle_t* ctx_bundle = malloc(sizeof(activity_ctx_bundle_t));
	lv_screen_node_t* old_prevs;
	
	lv_obj_t* screen;
	lv_display_t* old_display = lv_display_get_default();
	
	if (activity != NULL) {
		if (display == NULL) display = old_display;
		
		// FIXME: driver data workaround as user_data crashes...
		ctx = (activity_manager_ctx_t*)lv_display_get_driver_data(display);
		if (ctx == NULL) {
			ctx = malloc(sizeof(activity_manager_ctx_t)); // TODO: cleanup, but currently there's no lifecycle for the activity manager...
			lv_display_set_driver_data(display, ctx);
		}

		lv_display_set_default(display);
		screen = lv_obj_create(NULL);
		
		// TODO: cleanup, but no lifecycle for activity manager...
		old_prevs = ctx->prevs;
		ctx->prevs = malloc(sizeof(lv_screen_node_t));
		ctx->prevs->prev = old_prevs;
		ctx->prevs->value = lv_screen_active();

		lv_display_set_default(old_display);
		
		lv_screen_load(screen);

		ctx_bundle->manager = ctx;
		ctx_bundle->activity = activity_ctx;

		activity_ctx->apps = apps;
		activity_ctx->display = display;
		activity_ctx->screen = screen;
		activity_ctx->activity = activity;
		activity_ctx->cb = (void (*)(void*, int, void*))finished_activity_cb;
		activity_ctx->result_cb = cb;
		activity_ctx->user = ctx_bundle;
		activity_ctx->return_user = user;
		activity_ctx->input = input;

		activity->entry(activity_ctx);

		return 0;
	}
	return -ENOSYS;
}

int start_activity_from_intent_filter_result(apps_t* apps,
		intent_filter_result_t* intent_filter_result,
		activity_result_callback_t cb, void* input, void* user, lv_display_t* display) {
	return start_activity(apps, intent_filter_result->app, intent_filter_result->activity, cb, input, user, display);
}

int start_activity_from_intent(apps_t* apps, intent_t* intent, activity_result_callback_t cb, lv_display_t* display) {
	intent_filter_result_node_t* intent_filter_result_node = search_intent_filters(apps, (intent_filter_func_t)is_intent_filter_match, intent);

	if (intent_filter_result_node == NULL) return -ENOSYS;

	// TODO: allow user to pick if multiple activities match
	return start_activity_from_intent_filter_result(apps, &intent_filter_result_node->intent_filter_result, cb, intent->input, intent->user, display);
}

int start_home_activity(apps_t* apps, lv_display_t* display) {
	intent_t intent;
	
	intent.action = ACTION_MAIN;
	intent.category = CATEGORY_HOME;
	intent.activity = NULL;
	
	return start_activity_from_intent(apps, &intent, NULL, display);
}

int start_debug_activity(apps_t* apps, lv_display_t* display) {
	intent_t intent;
	
	intent.action = ACTION_MAIN;
	intent.category = CATEGORY_DEBUG;
	intent.activity = NULL;
	
	return start_activity_from_intent(apps, &intent, NULL, display);
}
