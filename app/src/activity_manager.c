#include "activity_manager.h"

#include "lv_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <string.h>

#include "intent_filter.h"

int shd_act_man_act_ctx_display_current_add(shd_act_man_ctx_t* manager, lv_display_t* display, shd_act_ctx_t* ctx) {
	shd_display_act_ctx_entry_node_t* node = malloc(sizeof(shd_display_act_ctx_entry_node_t));
	
	display = lv_display_or_default(display);
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

	display = lv_display_or_default(display);
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
	
	display = lv_display_or_default(display);
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

int shd_act_man_act_ctx_show(shd_act_man_ctx_t* manager, shd_act_ctx_t* ctx) {
	int ret = 0;

	if (ctx->state >= STARTED_PAUSED) {
		ret = shd_act_man_act_ctx_display_current_set(manager, lv_display_or_default(ctx->display), ctx);

		if (ret == 0) lv_screen_load(ctx->screen);
	}

	return ret;
}

int shd_act_man_act_ctx_launch(shd_act_man_ctx_t* manager, shd_act_ctx_t* ctx, lv_display_t* display) {
	int ret = -1;
	shd_act_ctx_t* current = shd_act_man_act_ctx_display_current_get(manager, display);

	if (current != NULL) shd_act_ctx_state_transition(current, STARTED_PAUSED);

	ret = shd_act_ctx_state_transition(ctx, RESUMED);
	if (ret == 0) ret = shd_act_man_act_ctx_show(manager, ctx);

	if (current != NULL) {
		if (ret == 0) shd_act_ctx_state_transition(current, CREATED_STOPPED); else shd_act_ctx_state_transition(current, RESUMED);
	}
	return ret;
}
void shd_act_man_act_ctx_kill(shd_act_man_ctx_t* manager, shd_act_ctx_t* ctx) {
	shd_act_ctx_state_transition(ctx, INITIALIZED_DESTROYED);
	shd_act_man_back_go(manager, ctx->display);
	shd_act_man_act_ctx_destroy(ctx);
}

int shd_act_man_act_launch(shd_act_man_ctx_t* manager, shd_app_t* app, shd_act_t* activity, lv_display_t* display, shd_act_result_cb_t cb, void* input, void* user) {
	int ret = -1;
	shd_act_ctx_t* ctx = NULL; 

	if (shd_act_has_action_is(activity, ACTION_MAIN)) {
		ctx = shd_act_man_act_ctx_find(manager, activity);

		if (ctx != NULL) {
			ctx->input = input;
			ctx->return_user = user;
		}
	}
	if (ctx == NULL) ctx = shd_act_man_act_ctx_create(manager, app, activity, display, cb, input, user);

	if (ctx == NULL) ret = -ENOSYS; else ret = shd_act_man_act_ctx_launch(manager, ctx, display);
	return ret;
}

int shd_act_man_act_launch_from_intent_filter_result(shd_act_man_ctx_t* manager, lv_display_t* display, shd_intent_filter_result_t* intent_filter_result,
		shd_act_result_cb_t cb, void* input, void* user) {
	return shd_act_man_act_launch(manager, intent_filter_result->app, intent_filter_result->activity, lv_display_or_default(display), cb, input, user);
}

int shd_act_man_act_launch_from_intent(shd_act_man_ctx_t* manager, lv_display_t* display, shd_intent_t* intent, shd_act_result_cb_t cb) {
	int ret = 0;

	shd_intent_filter_result_node_t* intent_filter_result_node = shd_apps_intent_filter_search(manager->apps, (shd_intent_filter_func_t)shd_intent_filter_match_is, intent);

	if (intent_filter_result_node == NULL) return -ENOSYS;

	// TODO: allow user to pick if multiple activities match
	ret = shd_act_man_act_launch_from_intent_filter_result(manager, lv_display_or_default(display), intent_filter_result_node->value, cb, intent->input, intent->user);

	shd_intent_filter_results_destroy(intent_filter_result_node); // TODO: if we decide to iterate here then do the freeing there
	return ret;
}

int shd_act_man_debug_launch(shd_act_man_ctx_t* manager, lv_display_t* display) {
	shd_intent_t intent;
	
	intent.action = ACTION_MAIN;
	intent.category = CATEGORY_DEBUG;
	intent.activity = NULL;
	
	return shd_act_man_act_launch_from_intent(manager, lv_display_or_default(display), &intent, NULL);
}
int shd_act_man_home_launch(shd_act_man_ctx_t* manager, lv_display_t* display) {
	shd_intent_t intent;
	
	intent.action = ACTION_MAIN;
	intent.category = CATEGORY_HOME;
	intent.activity = NULL;

	return shd_act_man_act_launch_from_intent(manager, lv_display_or_default(display), &intent, NULL);
}

int shd_act_man_back_go(shd_act_man_ctx_t* manager, lv_display_t* display) {
	int ret = -1;
	shd_act_ctx_t* current = shd_act_man_act_ctx_display_current_get(manager, lv_display_or_default(display));
	shd_act_ctx_t* prev;
	
	if (current != NULL) {
		prev = current->prev;
		
		if (prev != NULL) {
			ret = shd_act_ctx_state_transition(current, STARTED_PAUSED);
			if (ret == 0) {
				ret = shd_act_ctx_state_transition(prev, RESUMED);
				if (ret == 0) ret = shd_act_man_act_ctx_show(manager, prev);

				if (ret != 0) ret = shd_act_man_home_go(manager, display);
				shd_act_ctx_state_transition(current, CREATED_STOPPED); 
			}
		}
	}
	
	return ret;
}
int shd_act_man_home_go(shd_act_man_ctx_t* manager, lv_display_t* display) {
	return shd_act_man_home_launch(manager, lv_display_or_default(display));
}
