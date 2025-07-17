#include "activity_manager.h"

#include <lvgl.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

void destroy_intent_filter_results(intent_filter_result_node_t* node) {
	intent_filter_result_node_t* next;

	while (node != NULL) {
		next = node->next;
		free(node);
		node = next;
	}
}

intent_filter_result_node_t* search_intent_filters(apps_t* apps, 
		bool (*func)(intent_filter_t*, app_t*, intent_t*), intent_t* intent) {
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
		
		if (intent->activity != NULL) {
			while (activity_node != NULL) {
				activity = activity_node->activity;
				
				if (strcmp(intent->activity, activity->id) == 0) {
					intent_filter_result_node = malloc(sizeof(intent_filter_result_node_t));

					if (intent_filter_result_node != NULL) {
						intent_filter_result_node->next = NULL;

						intent_filter_result.app = app;
						intent_filter_result.activity = activity;
						intent_filter_result_node->intent_filter_result = intent_filter_result;
					}
			
					return intent_filter_result_node;
				}

				activity_node = activity_node->next;
			}
			continue;
		}
		
		while (activity_node != NULL) {
			activity = activity_node->activity;
			intent_filter_node = activity->intent_filters;

			while (intent_filter_node != NULL) {
				intent_filter = intent_filter_node->intent_filter;

				if (func(intent_filter, app, intent)) {
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

bool is_activity_has_action(activity_t* activity, action_t action) {
	intent_filter_node_t* intent_filter_node = activity->intent_filters;
	intent_filter_t* intent_filter;

	while (intent_filter_node != NULL) {
		intent_filter = intent_filter_node->intent_filter;
		
		if ((intent_filter->action & action) > 0) {
			return true;
		}

		intent_filter_node = intent_filter_node->next;
	}

	return false;
}

bool is_intent_filter_match(intent_filter_t* intent_filter, app_t* app, intent_t* intent) {
	// TODO: data check
	if ((intent_filter->action & intent->action) > 0 && 
		(intent_filter->category & intent->category) >= intent->category) return true;	
	return false;
}

void delete_activity_ctx_from_manager(activity_ctx_bundle_t* ctx_bundle) {
	activity_manager_ctx_t* manager = ctx_bundle->manager;
	activity_ctx_t* activity = ctx_bundle->activity;

	activity_ctx_node_t* node = manager->activities;
	activity_ctx_node_t* last_node = NULL;

	while (node != NULL) {
		if (node->value == activity) {
			if (last_node == NULL) {
				manager->activities = node->prev;
			} else {
				last_node->prev = node->prev;
			}

			free(node);
			break;
		}

		last_node = node;
		node = node->prev;
	}
}

int add_activity_ctx_to_manager(activity_ctx_bundle_t* ctx_bundle) {
	activity_manager_ctx_t* manager = ctx_bundle->manager;
	activity_ctx_t* activity = ctx_bundle->activity;
	activity_ctx_node_t* new = malloc(sizeof(activity_ctx_node_t));
	
	if (new == NULL) {
		return -ENOSR;
	} else {
		new->prev = manager->activities;
		new->value = activity;
		manager->activities = new;
	}
	
	return 0;
}

void show_activity(activity_ctx_bundle_t* ctx_bundle) {
	activity_manager_ctx_t* manager = ctx_bundle->manager;
	activity_ctx_t* activity = ctx_bundle->activity;

	manager->current = activity;
	lv_screen_load(activity->screen);
}

void take_snapshot(activity_ctx_t* ctx) {
	if (ctx->snapshot != NULL) lv_draw_buf_destroy(ctx->snapshot);
	ctx->snapshot = lv_snapshot_take(ctx->screen, LV_COLOR_FORMAT_ARGB8888);
}

void pause_activity(activity_ctx_bundle_t* ctx_bundle) {
	activity_ctx_t* activity_ctx = ctx_bundle->activity;
	activity_t* activity = activity_ctx->activity;
	
	take_snapshot(activity_ctx);

	if (activity->pause != NULL) activity->pause(activity_ctx);
}

void unpause_activity(activity_ctx_bundle_t* ctx_bundle) {
	activity_ctx_t* activity_ctx = ctx_bundle->activity;
	activity_t* activity = activity_ctx->activity;

	if (activity->unpause != NULL) activity->unpause(activity_ctx);
}

void close_activity(activity_ctx_bundle_t* ctx_bundle) {
	activity_ctx_t* activity_ctx = ctx_bundle->activity;
	activity_t* activity = activity_ctx->activity;
	
	// NOTE: we temporarily repurpose this bundle
	ctx_bundle->activity = activity_ctx->prev;
	show_activity(ctx_bundle);
	ctx_bundle->activity = activity_ctx;

	if (activity->exit != NULL) activity->exit(activity_ctx);
	lv_obj_delete(activity_ctx->screen);
	
	delete_activity_ctx_from_manager(ctx_bundle);
}

void finished_activity_cb(activity_ctx_bundle_t* ctx_bundle, int result, void* data) {
	activity_ctx_t* ctx = ctx_bundle->activity;
	activity_result_callback_t cb = ctx->result_cb;
	
	close_activity(ctx_bundle);
	if (cb != NULL) cb(result, data, ctx->return_user);

	free(ctx);
	free(ctx_bundle);
}

activity_manager_ctx_t* get_activity_manager(lv_display_t* display) {
	// FIXME: driver data workaround as user_data crashes...
	return (activity_manager_ctx_t*)lv_display_get_driver_data(display);
}

activity_manager_ctx_t* init_activity_manager(lv_display_t* display) {
	activity_manager_ctx_t* ctx = malloc(sizeof(activity_manager_ctx_t)); 
	ctx->current = NULL;
	ctx->activities = NULL;
	
	lv_display_set_driver_data(display, ctx);
	return ctx;
}

activity_manager_ctx_t* try_activity_manager(lv_display_t* display) {
	activity_manager_ctx_t* ctx = get_activity_manager(display);

	if (ctx == NULL) ctx = init_activity_manager(display);
	return ctx;
}

void destroy_activity_manager_ctxs(activity_manager_ctx_t* ctx) {
	activity_ctx_node_t* node = ctx->activities;
	activity_ctx_node_t* prev;

	while (node != NULL) {
		prev = node->prev;

		free(node->value);
		free(node);

		node = prev;
	}
}

void destroy_activity_manager(activity_manager_ctx_t* ctx) {
	destroy_activity_manager_ctxs(ctx);

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

activity_ctx_t* find_activity_ctx_in_manager(activity_manager_ctx_t* ctx, activity_t* activity) {
	activity_ctx_node_t* node = ctx->activities;

	while (node != NULL) {
		if (node->value->activity == activity) {
			return node->value;
		}

		node = node->prev;
	}

	return NULL;
}

int start_existing_activity(activity_ctx_bundle_t* ctx_bundle, activity_t* activity, void* input, void* user) {
	int ret = 0;
	activity_ctx_t* activity_ctx;
	
	activity_ctx = find_activity_ctx_in_manager(ctx_bundle->manager, activity);
	if (activity_ctx == NULL) {
		ret = -ESRCH;
	} else {
		ctx_bundle->activity = activity_ctx;
		
		activity_ctx->input = input;
		activity_ctx->return_user = user;
		
		unpause_activity(ctx_bundle);
	}

	return ret;
}

int start_new_activity(activity_ctx_bundle_t* ctx_bundle, 
		activity_t* activity, 
		apps_t* apps, app_t* app, 
		activity_result_callback_t cb, void* input,  void* user,
		lv_display_t* display) {
	lv_obj_t* screen;
	activity_ctx_t* activity_ctx;
	
	if (display == NULL) display = lv_display_get_default();

	ctx_bundle->activity = malloc(sizeof(activity_ctx_t));
	activity_ctx = ctx_bundle->activity; 
	
	if (activity_ctx == NULL) {
		return -ENOSR;
	} else {
		screen = lv_screen_create_on_display(display);
		
		if (screen == NULL) {
			free(activity_ctx);

			return -ENOSR;
		} else {
			activity_ctx->apps = apps;
			activity_ctx->app = app;
			activity_ctx->user = ctx_bundle;
			activity_ctx->display = display;
			activity_ctx->screen = screen;
			activity_ctx->snapshot = NULL;
			activity_ctx->prev = ctx_bundle->manager->current;
			activity_ctx->activity = activity;
			activity_ctx->cb = (void (*)(void*, int, void*))finished_activity_cb;
			activity_ctx->result_cb = cb;
			activity_ctx->return_user = user;
			activity_ctx->input = input;

			if (add_activity_ctx_to_manager(ctx_bundle) != 0) {
				free(activity_ctx);

				return -ENOSR;
			}
			
			if(activity->entry != NULL) activity->entry(activity_ctx);
		}
	}
	
	return 0;
}

// FIXME: memory leak lvgl draw buffer?
// TODO: better activity lifecycles to have enough memory...
int start_activity(apps_t* apps, app_t* app, activity_t* activity, activity_result_callback_t cb, void* input, void* user, lv_display_t* display) {
	int ret = 0;
	activity_manager_ctx_t* ctx; 
	activity_ctx_bundle_t* ctx_bundle;
	activity_ctx_t* activity_ctx;
	bool free_bundle = false;

	if (activity == NULL) {
		return -ENOSYS;
	} else {
		ctx_bundle = malloc(sizeof(activity_ctx_bundle_t));

		if (display == NULL) display = lv_display_get_default();
		
		ctx = try_activity_manager(display); // TODO: cleanup
		if (ctx == NULL) return -ENOSR;
		ctx_bundle->manager = ctx;

		if (is_activity_has_action(activity, ACTION_MAIN)) {
			ret = start_existing_activity(ctx_bundle, activity, input, user);

			if (ret == -ESRCH) {
				ret = start_new_activity(ctx_bundle, activity, apps, app, cb, input, user, display);
			} else {
				free_bundle = true;
			}
		} else {
			ret = start_new_activity(ctx_bundle, activity, apps, app, cb, input, user, display);
		}
	}
	
	if (ret == 0) {
		if (ctx->current != NULL) {
			// NOTE: we temporarily repurpose the ctx_bundle
			activity_ctx = ctx_bundle->activity;
			ctx_bundle->activity = ctx->current;
			pause_activity(ctx_bundle);
			ctx_bundle->activity = activity_ctx;
		}
		
		show_activity(ctx_bundle);
	}
	if (free_bundle) free(ctx_bundle);

	return ret;
}

int start_activity_from_intent_filter_result(apps_t* apps,
		intent_filter_result_t* intent_filter_result,
		activity_result_callback_t cb, void* input, void* user, lv_display_t* display) {
	return start_activity(apps, intent_filter_result->app, intent_filter_result->activity, cb, input, user, display);
}

int start_activity_from_intent(apps_t* apps, intent_t* intent, activity_result_callback_t cb, lv_display_t* display) {
	int ret = 0;
	intent_filter_result_node_t* intent_filter_result_node = search_intent_filters(apps, (intent_filter_func_t)is_intent_filter_match, intent);
	
	if (intent_filter_result_node == NULL) return -ENOSYS;

	// TODO: allow user to pick if multiple activities match
	ret = start_activity_from_intent_filter_result(
			apps, &intent_filter_result_node->intent_filter_result, cb, intent->input, intent->user, display);

	destroy_intent_filter_results(intent_filter_result_node); // TODO: if we decide to iterate here then do the freeing there

	return ret;
}

int start_home_activity(apps_t* apps, lv_display_t* display) {
	intent_t intent;
	
	intent.action = ACTION_MAIN;
	intent.category = CATEGORY_HOME;
	intent.activity = NULL;

	if (display == NULL) display = lv_display_get_default();
	
	return start_activity_from_intent(apps, &intent, NULL, display);
}

int start_debug_activity(apps_t* apps, lv_display_t* display) {
	intent_t intent;
	
	intent.action = ACTION_MAIN;
	intent.category = CATEGORY_DEBUG;
	intent.activity = NULL;

	if (display == NULL) display = lv_display_get_default();
	
	return start_activity_from_intent(apps, &intent, NULL, display);
}

int go_back(lv_display_t* display) {
	activity_manager_ctx_t* ctx;
	activity_ctx_bundle_t* ctx_bundle = malloc(sizeof(activity_ctx_bundle_t));
	activity_ctx_t* current;
	activity_ctx_t* prev;
	
	if (ctx_bundle == NULL) {
		return -ENOSR;
	} else {
		if (display == NULL) display = lv_display_get_default();

		ctx = try_activity_manager(display); // TODO: cleanup
		if (ctx == NULL) return -ENOSR;
		
		current = ctx->current;
		prev = current->prev;

		if (prev != NULL) {
			ctx_bundle->manager = ctx;

			ctx_bundle->activity = current;
			pause_activity(ctx_bundle);

			ctx_bundle->activity = prev;
			unpause_activity(ctx_bundle);
			show_activity(ctx_bundle);
			
		}

		free(ctx_bundle);

		return 0;
	}
}
