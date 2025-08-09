#include "intent_filter.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void shd_intent_filter_results_destroy(shd_intent_filter_result_node_t* node) {
	shd_intent_filter_result_node_t* next;

	while (node != NULL) {
		next = node->next;
		free(node->value);
		free(node);
		node = next;
	}
}
shd_intent_filter_result_node_t* shd_apps_intent_filter_search(shd_apps_t* apps, shd_intent_filter_func_t func, shd_intent_t* intent) {
	shd_act_node_t* activity_node;
	shd_act_t* activity;
	shd_app_t* app;

	shd_intent_filter_node_t* node;
	shd_intent_filter_t* intent_filter;

	bool ret = false;
	bool add = false;
	shd_intent_filter_result_node_t* final_node = NULL;
	shd_intent_filter_result_node_t* result_node = NULL;
	shd_intent_filter_result_t* result;

	for (size_t i = 0; i < apps->size; i++) {
		app = apps->list[i];
		activity_node = app->activities;
		
		while (activity_node != NULL) {
			activity = activity_node->activity;
			node = activity->intent_filters;

			while (node != NULL || intent->activity != NULL) {
				if (intent->activity != NULL) ret = strcmp(intent->activity, activity->id) == 0;
				
				if (node == NULL) {
					add = ret;
					if (!add) break;
				} else {
					intent_filter = node->intent_filter;
					add = func(intent_filter, app, intent) || ret;
					node = node->next;
				}

				if (add) {
					if (result_node == NULL) {
						result_node = malloc(sizeof(shd_intent_filter_result_node_t));
						final_node = result_node;
					} else {
						result_node->next = malloc(sizeof(shd_intent_filter_result_node_t));
						result_node = result_node->next;
					}
					
					if (result_node == NULL) {
						return NULL;
					} else {
						result_node->next = NULL;
						result_node->value = NULL;

						result = malloc(sizeof(shd_intent_filter_result_t));
						if (result == NULL) {
							return NULL;
						} else {
							result->app = app;
							result->activity = activity;
							result_node->value = result;

							if (ret) return final_node;
						}
					}
				}
			};

			activity_node = activity_node->next;
		};
	}
	
	return final_node;
}

bool shd_act_in_category_is(shd_act_t* activity, shd_category_t category) {
	shd_intent_filter_node_t* intent_filter_node = activity->intent_filters;
	shd_intent_filter_t* intent_filter;

	while (intent_filter_node != NULL) {
		intent_filter = intent_filter_node->intent_filter;
		
		if ((intent_filter->category & category) >= category) {
			return true;
		}

		intent_filter_node = intent_filter_node->next;
	}

	return false;
}
bool shd_act_has_action_is(shd_act_t* activity, shd_action_t action) {
	shd_intent_filter_node_t* intent_filter_node = activity->intent_filters;
	shd_intent_filter_t* intent_filter;

	while (intent_filter_node != NULL) {
		intent_filter = intent_filter_node->intent_filter;
		
		if ((intent_filter->action & action) > 0) {
			return true;
		}

		intent_filter_node = intent_filter_node->next;
	}

	return false;
}

bool shd_intent_filter_match_is(shd_intent_filter_t* intent_filter, shd_app_t* app, shd_intent_t* intent) {
	// TODO: data check
	if ((intent_filter->action & intent->action) > 0 && 
		(intent_filter->category & intent->category) >= intent->category) return true;	
	return false;
}
