#include "activity_manager.h"

#include "lv_utils.h"

#include <stdlib.h>
#include <errno.h>

#include <string.h>

void destroy_intent_filter_results(intent_filter_result_node_t* node) {
	intent_filter_result_node_t* next;

	while (node != NULL) {
		next = node->next;
		free(node);
		node = next;
	}
}
intent_filter_result_node_t* search_intent_filters(shd_apps_t* apps, intent_filter_func_t func, intent_t* intent) {
	shd_act_node_t* activity_node;
	shd_act_t* activity;
	shd_app_t* app;

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

bool is_activity_in_category(shd_act_t* activity, category_t category) {
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
bool is_activity_has_action(shd_act_t* activity, action_t action) {
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
bool is_intent_filter_match(intent_filter_t* intent_filter, shd_app_t* app, intent_t* intent) {
	// TODO: data check
	if ((intent_filter->action & intent->action) > 0 && 
		(intent_filter->category & intent->category) >= intent->category) return true;	
	return false;
}

int shd_act_man_act_ctx_display_current_add(shd_act_man_ctx_t* manager, lv_display_t* display, shd_act_ctx_t* ctx) {
	shd_display_act_ctx_entry_node_t* node = malloc(sizeof(shd_display_act_ctx_entry_node_t));
	
	if (node == NULL) {
		return -ENOSR;
	} else {
		node->value = malloc(sizeof(shd_display_act_ctx_entry_t));
		if (node->value == NULL) {
			return -ENOSR;
		} else {
			node->value->display = display;
			node->value->ctx = ctx;

			node->prev = manager->current_activities;
			manager->current_activities = node;
		}
	}

	return 0;
}
int shd_act_man_act_ctx_display_current_set(shd_act_man_ctx_t* manager, lv_display_t* display, shd_act_ctx_t* ctx) {
	shd_display_act_ctx_entry_node_t* node = manager->current_activities;

	while (node != NULL) {
		if (node->value->display == display) {
			node->value->ctx = ctx;
			return 0;
		}

		node = node->prev;
	}
	
	return shd_act_man_act_ctx_display_current_add(manager, display, ctx);
}
shd_act_ctx_t* shd_act_man_act_ctx_display_current_get(shd_act_man_ctx_t* manager, lv_display_t* display) {
	shd_display_act_ctx_entry_node_t* node = manager->current_activities;

	while (node != NULL) {
		if (node->value->display == display) {
			return node->value->ctx;
		}

		node = node->prev;
	}

	return NULL;
}
bool shd_act_man_act_ctx_display_current_is(shd_act_man_ctx_t* manager, lv_display_t* display, shd_act_ctx_t* ctx) {
	return shd_act_man_act_ctx_display_current_get(manager, display) == ctx;
}

void shd_act_man_act_finished_cb(shd_act_ctx_t* ctx, int result, void* data) {
	shd_act_man_ctx_t* manager = ctx->manager;
	lv_display_t* display = lv_display_or_default(ctx->display);

	shd_act_result_cb_t cb = ctx->result_cb;
	void* user = ctx->return_user;
	
	if (shd_act_man_act_ctx_display_current_is(manager, display, ctx)) if (shd_act_man_back_go(manager, display) != 0) shd_act_man_home_go(manager, display);

	shd_act_ctx_state_transition(ctx, INITIALIZED_DESTROYED);
	if (cb != NULL) cb(result, data, user);
}
shd_act_ctx_t* shd_act_man_act_ctx_create(shd_act_man_ctx_t* manager, shd_app_t* app, shd_act_t* activity, lv_display_t* display, shd_act_result_cb_t cb, void* input, void* user) {
	shd_act_ctx_t* ctx = malloc(sizeof(shd_act_ctx_t));
	shd_act_ctx_node_t* new = malloc(sizeof(shd_act_ctx_node_t));
	
	if (new == NULL || ctx == NULL) {
		return NULL;
	} else {
		ctx->manager = manager;

		ctx->display = display;
		ctx->screen = NULL;
		ctx->snapshot = NULL;

		ctx->app = app;
		ctx->activity = activity;
		ctx->prev = shd_act_man_act_ctx_display_current_get(manager, display);
		ctx->state = INITIALIZED_DESTROYED;

		ctx->cb = (void (*)(void*, int, void*))shd_act_man_act_finished_cb;
		ctx->user = ctx; // TODO: rethink, do we really need it as a user parameter?

		ctx->result_cb = cb;
		ctx->return_user = user;

		ctx->input = input;
		ctx->activity_user = NULL;

		new->prev = manager->activities;
		new->value = ctx;
		manager->activities = new;
	}
	
	return ctx;
}
void shd_act_man_act_ctx_destroy(shd_act_ctx_t* ctx) {
	shd_act_man_ctx_t* manager = ctx->manager;
	shd_act_ctx_node_t* node = manager->activities;
	shd_act_ctx_node_t* last_node = NULL;

	while (node != NULL) {
		if (node->value == ctx) {
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

	free(ctx);
}

shd_act_man_ctx_t* shd_act_man_create(shd_apps_t* apps) {
	shd_act_man_ctx_t* manager = malloc(sizeof(shd_act_man_ctx_t)); 

	manager->apps = apps;

	manager->current_activities = NULL;
	manager->activities = NULL;

	return manager;
}
void shd_act_man_act_ctx_cleanup(shd_act_man_ctx_t* manager) {
	shd_act_ctx_node_t* node = manager->activities;
	shd_act_ctx_node_t* prev;

	while (node != NULL) {
		prev = node->prev;

		free(node->value);
		free(node);

		node = prev;
	}
}
void shd_act_man_act_ctx_current_cleanup(shd_act_man_ctx_t* manager) {
	shd_display_act_ctx_entry_node_t* node = manager->current_activities;
	shd_display_act_ctx_entry_node_t* prev;

	while (node != NULL) {
		prev = node->prev;

		free(node->value);
		free(node);

		node = prev;
	}
}
void shd_act_man_destroy(shd_act_man_ctx_t* manager) {
	shd_act_man_act_ctx_cleanup(manager);
	shd_act_man_act_ctx_current_cleanup(manager);
	free(manager);
}

shd_act_ctx_t* shd_act_man_act_ctx_find(shd_act_man_ctx_t* manager, shd_act_t* activity) {
	shd_act_ctx_node_t* node = manager->activities;

	while (node != NULL) {
		if (node->value->activity == activity) {
			return node->value;
		}

		node = node->prev;
	}

	return NULL;
}
shd_act_ctx_t* shd_act_man_act_ctx_relaunch(shd_act_man_ctx_t* manager, shd_act_t* activity, void* input, void* user) {
	shd_act_ctx_t* ctx = shd_act_man_act_ctx_find(manager, activity);

	if (ctx != NULL) {
		ctx->input = input;
		ctx->return_user = user;
		
		if (shd_act_ctx_state_transition(ctx, RESUMED) != 0) ctx = NULL;
	}

	return ctx;
}
shd_act_ctx_t* shd_act_man_act_ctx_launch(shd_act_man_ctx_t* manager, shd_app_t* app, shd_act_t* activity, lv_display_t* display, shd_act_result_cb_t cb, void* input, void* user) {
	shd_act_ctx_t* ctx = shd_act_man_act_ctx_create(manager, app, activity, display, cb, input, user);

	if (ctx != NULL) if (shd_act_ctx_state_transition(ctx, RESUMED) != 0) ctx = NULL;
	return ctx;
}

int shd_act_man_act_ctx_show(shd_act_man_ctx_t* manager, shd_act_ctx_t* ctx) {
	int ret = 0;

	if (ctx->state >= STARTED_PAUSED) {
		ret = shd_act_man_act_ctx_display_current_set(manager, lv_display_or_default(ctx->display), ctx);

		if (ret == 0) lv_screen_load(ctx->screen);
	}

	return ret;
}

int shd_act_man_act_launch(shd_act_man_ctx_t* manager, shd_app_t* app, shd_act_t* activity, lv_display_t* display, shd_act_result_cb_t cb, void* input, void* user) {
	int ret = -1;

	shd_act_ctx_t* current = shd_act_man_act_ctx_display_current_get(manager, display);
	shd_act_ctx_t* ctx = NULL; 

	if (current != NULL) shd_act_ctx_state_transition(current, STARTED_PAUSED);

	if (is_activity_has_action(activity, ACTION_MAIN)) ctx = shd_act_man_act_ctx_relaunch(manager, activity, input, user);
	if (ctx == NULL) ctx = shd_act_man_act_ctx_launch(manager, app, activity, lv_display_or_default(display), cb, input, user);

	if (ctx == NULL) {
		shd_act_ctx_state_transition(current, RESUMED);
		ret = -ENOSYS;
	} else {
		ret = shd_act_man_act_ctx_show(manager, ctx);

		if (current != NULL) {
			if (ret == 0) {
				shd_act_ctx_state_transition(current, CREATED_STOPPED);
			} else {
				shd_act_ctx_state_transition(current, RESUMED);
			}
		}
	} 

	return ret;
}

int shd_act_man_act_launch_from_intent_filter_result(shd_act_man_ctx_t* manager, lv_display_t* display, intent_filter_result_t* intent_filter_result,
		shd_act_result_cb_t cb, void* input, void* user) {
	return shd_act_man_act_launch(manager, intent_filter_result->app, intent_filter_result->activity, lv_display_or_default(display), cb, input, user);
}

int shd_act_man_act_launch_from_intent(shd_act_man_ctx_t* manager, lv_display_t* display, intent_t* intent, shd_act_result_cb_t cb) {
	int ret = 0;

	intent_filter_result_node_t* intent_filter_result_node 
		= search_intent_filters(manager->apps, (intent_filter_func_t)is_intent_filter_match, intent);

	if (intent_filter_result_node == NULL) return -ENOSYS;

	// TODO: allow user to pick if multiple activities match
	ret = shd_act_man_act_launch_from_intent_filter_result(manager, lv_display_or_default(display), &intent_filter_result_node->intent_filter_result, cb, intent->input, intent->user);

	destroy_intent_filter_results(intent_filter_result_node); // TODO: if we decide to iterate here then do the freeing there
	return ret;
}

int shd_act_man_home_launch(shd_act_man_ctx_t* manager, lv_display_t* display) {
	intent_t intent;
	
	intent.action = ACTION_MAIN;
	intent.category = CATEGORY_HOME;
	intent.activity = NULL;

	return shd_act_man_act_launch_from_intent(manager, lv_display_or_default(display), &intent, NULL);
}
int shd_act_man_back_go(shd_act_man_ctx_t* manager, lv_display_t* display) {
	int ret = -1;
	shd_act_ctx_t* current = shd_act_man_act_ctx_display_current_get(manager, display);
	shd_act_ctx_t* prev;
	
	if (current != NULL) {
		prev = current->prev;

		ret = shd_act_ctx_state_transition(current, STARTED_PAUSED);
		if (ret == 0) {
			ret = shd_act_ctx_state_transition(prev, RESUMED);

			if (ret == 0) ret = shd_act_man_act_ctx_show(manager, prev);
			if (ret != 0) shd_act_ctx_state_transition(current, RESUMED);
		}

	}
	
	return ret;
}
int shd_act_man_home_go(shd_act_man_ctx_t* manager, lv_display_t* display) {
	return shd_act_man_home_launch(manager, lv_display_or_default(display));
}
int shd_act_man_debug_launch(shd_act_man_ctx_t* manager, lv_display_t* display) {
	intent_t intent;
	
	intent.action = ACTION_MAIN;
	intent.category = CATEGORY_DEBUG;
	intent.activity = NULL;
	
	return shd_act_man_act_launch_from_intent(manager, lv_display_or_default(display), &intent, NULL);
}

