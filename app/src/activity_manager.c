#include "activity_manager.h"

#include <lvgl.h>

activity_t* search_intent_filters(app_t* apps, size_t size, bool (*func)(intent_filter_t*), void* user) {
	activity_node_t* activity_node;
	activity_t* activity;

	intent_filter_node_t* intent_filter_node;
	intent_filter_t* intent_filter;

	for (size_t i = 0; i < size; i++) {
		app = &apps[i];
		activity_node = app->activities;

		do {
			activity = activity_node->activity;
			intent_filter_node = activity->intent_filters;

			do {
				intent_filter = intent_filter_node->intent_filter;
				if (func(intent_filter, user)) return activity;

				intent_filter_node = intent_filter_node->next;
			} while (intent_filter_node != NULL);

			activity_node = activity_node->next;
		} while (activity_node != NULL);
	}
	return NULL;
}

bool is_intent_filter_match(intent_filter_t* intent_filter, intent_t* intent) {
	// TODO: data check
	if (
		(intent_filter->action & intent->action) > 0 && 
		(intent_filter->category <= intent->category)
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

int start_activity(activity_t* activity, activity_result_callback cb) {
	activity_manager_ctx* ctx;

	if (activity != NULL) {
		ctx = malloc(sizeof(activity_manager_ctx));
		ctx->activity = activity;
		ctx->cb = cb;
		ctx->prev = lv_active_screen();
		ctx->screen = lv_obj_create(NULL);
		
		lv_obj_add_event_cb(ctx->screen, activity_event_handler, LV_EVENT_KEY, ctx);
		lv_screen_load(ctx->screen);
		activity->entry(ctx->screen, finished_activity_cb, ctx);
		return 0;
	}
	return -ENOSYS;
}

int start_activity_from_intent(app_t* apps, size_t size, intent_t* intent, activity_result_callback cb) {
	app_t* app;
	activity_t* activity = search_intent_filters(apps, size, is_intent_filter_match, intent);
	
	return start_activity(activity);
}

int start_home_activity(app_t* apps, size_t size) {
	intent_t* intent;
	activity_t* activity;

	intent->action = ACTION_MAIN;
	intent->category = CATEGORY_HOME;

	activity = search_intent_filters(apps, size, is_intent_filter_match, intent);
	return start_activity(activity);	
}
