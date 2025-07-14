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

void load_previous_screen(activity_manager_ctx_t* ctx) {
	lv_screen_node_t* prevs = ctx->prevs;

	if (prevs == NULL) {
		start_home_activity(ctx->apps, lv_display_get_default());
	} else {
		lv_screen_load(prevs->value);
		ctx->prevs = prevs->prev;
	}
}

void close_activity(activity_ctx_bundle_t* ctx_bundle) {
	activity_manager_ctx_t* manager = ctx_bundle->manager;
	activity_ctx_t* activity_ctx = ctx_bundle->activity;
	activity_t* activity = activity_ctx->activity;
	
	load_previous_screen(manager);

	activity->exit(activity_ctx);
	lv_obj_delete(activity_ctx->screen);
}

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

activity_manager_ctx_t* get_activity_manager(lv_display_t* display) {
	// FIXME: driver data workaround as user_data crashes...
	return (activity_manager_ctx_t*)lv_display_get_driver_data(display);
}

activity_manager_ctx_t* init_activity_manager(lv_display_t* display, apps_t* apps) {
	activity_manager_ctx_t* ctx = malloc(sizeof(activity_manager_ctx_t)); 
	
	ctx->apps = apps;
	lv_display_set_driver_data(display, ctx);

	return ctx;
}

activity_manager_ctx_t* try_activity_manager(lv_display_t* display, apps_t* apps) {
	activity_manager_ctx_t* ctx = get_activity_manager(display);

	if (ctx == NULL) {
		ctx = init_activity_manager(display, apps);
	}
	return ctx;
}

lv_screen_node_t* continue_activity_manager_prevs(activity_manager_ctx_t* ctx) {
	lv_screen_node_t* old_prevs = ctx->prevs;

	ctx->prevs = malloc(sizeof(lv_screen_node_t));
	if (ctx->prevs == NULL) {
		return NULL;
	}

	return old_prevs;
}

void add_to_activity_manager_prevs(activity_manager_ctx_t* ctx, lv_screen_node_t* old_prevs) {
	ctx->prevs->prev = old_prevs;
	ctx->prevs->value = lv_screen_active();
}

void destroy_activity_manager_prevs(activity_manager_ctx_t* ctx) {
	lv_screen_node_t* prev = ctx->prevs->prev;

	while (ctx->prevs != NULL) {
		free(ctx->prevs);
		ctx->prevs = prev;
	}
}

void destroy_activity_manager(activity_manager_ctx_t* ctx) {
	destroy_activity_manager_prevs(ctx);
	free(ctx);
}

lv_obj_t* lv_screen_create_on_display(lv_display_t* display) {
	lv_display_t* old_display = lv_display_get_default();
	lv_obj_t* screen;

	lv_display_set_default(display);
	screen = lv_obj_create(NULL);
	lv_display_set_default(old_display);
	
	return screen;
}

// TODO: better activity lifecycles to have enough memory...
int start_activity(apps_t* apps, app_t* app, activity_t* activity, activity_result_callback_t cb, void* input, void* user, lv_display_t* display) {
	activity_manager_ctx_t* ctx; 
	activity_ctx_t* activity_ctx = malloc(sizeof(activity_ctx_t));
	activity_ctx_bundle_t* ctx_bundle = malloc(sizeof(activity_ctx_bundle_t));
	
	lv_obj_t* screen;
	lv_screen_node_t* old_prevs;
	
	if (activity_ctx == NULL || ctx_bundle == NULL) {
		return -ENOSR;
	}

	if (activity != NULL) {
		if (display == NULL) display = lv_display_get_default();
		
		ctx = try_activity_manager(display, apps); // TODO: cleanup
		if (ctx == NULL) return -ENOSR;

		old_prevs = continue_activity_manager_prevs(ctx);
		if (old_prevs == NULL) return -ENOSR;
		add_to_activity_manager_prevs(ctx, old_prevs);

		screen = lv_screen_create_on_display(display);
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

void go_home() {

}
