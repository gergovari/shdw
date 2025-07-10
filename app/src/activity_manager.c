#include "activity_manager.h"

#include <lvgl.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

intent_filter_result_node_t* search_intent_filters(apps_t* apps, 
		bool (*func)(intent_filter_t*, void*), void* user) {
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

		do {
			activity = activity_node->activity;
			intent_filter_node = activity->intent_filters;

			do {
				intent_filter = intent_filter_node->intent_filter;

				if (func(intent_filter, user)) {
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
			} while (intent_filter_node != NULL);

			activity_node = activity_node->next;
		} while (activity_node != NULL);
	}
	return final_node;
}

bool is_intent_filter_match(intent_filter_t* intent_filter, intent_t* intent) {
	// TODO: data check
	if (
		(intent_filter->action & intent->action) > 0 && 
		(intent_filter->category & intent->category) >= intent->category
	) {
		return true;	
	}

	return false;
}

void close_activity(activity_manager_ctx* ctx) {
	lv_screen_load(ctx->prev);

	ctx->activity->exit(ctx->screen);
	lv_obj_delete(ctx->screen);
	free(ctx);
}

void finished_activity_cb(activity_manager_ctx* ctx, int result, void* data) {
	activity_result_callback cb = ctx->cb;
	
	close_activity(ctx);
	if (cb != NULL) cb(result, data);
}

void activity_event_handler(lv_event_t* e) {
	uint32_t key;
	activity_manager_ctx* ctx = lv_event_get_user_data(e);

	key = lv_event_get_key(e);
	if (key == LV_KEY_ESC) {
		close_activity(ctx);
	}
}

int start_activity(activity_t* activity, activity_result_callback cb, void* user) {
	activity_manager_ctx* ctx;

	if (activity != NULL) {
		ctx = malloc(sizeof(activity_manager_ctx));
		ctx->activity = activity;
		ctx->cb = cb;
		ctx->prev = lv_screen_active();
		ctx->screen = lv_obj_create(NULL);
		
		lv_obj_add_event_cb(ctx->screen, activity_event_handler, LV_EVENT_KEY, ctx);
		lv_screen_load(ctx->screen);
		activity->entry(ctx->screen, (void (*)(void*, int,  void*))finished_activity_cb, user);

		return 0;
	}
	return -ENOSYS;
}

int start_activity_from_intent(apps_t* apps, intent_t* intent, activity_result_callback cb) {
	intent_filter_result_node_t* intent_filter_result_node = search_intent_filters(apps, 
			(bool (*)(intent_filter_t*, void*))is_intent_filter_match, 
			intent);
	activity_t* activity;

	if (intent_filter_result_node == NULL) return -ENOSYS;

	// TODO: allow user to pick if multiple activities match
	activity = intent_filter_result_node->intent_filter_result.activity; 
	return start_activity(activity, cb, intent->user);
}

int start_home_activity(apps_t* apps) {
	intent_t intent;
	
	// FIXME: matching too many activities
	intent.action = ACTION_MAIN;
	intent.category = CATEGORY_HOME;
	intent.user = apps;
	
	return start_activity_from_intent(apps, &intent, NULL);
}
