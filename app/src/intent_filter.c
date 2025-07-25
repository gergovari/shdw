#include "intent_filter.h"

#include <stdlib.h>
#include <string.h>

void shd_intent_filter_results_destroy(shd_intent_filter_result_node_t* node) {
	shd_intent_filter_result_node_t* next;

	while (node != NULL) {
		next = node->next;
		free(node);
		node = next;
	}
}
shd_intent_filter_result_node_t* shd_apps_intent_filter_search(shd_apps_t* apps, shd_intent_filter_func_t func, shd_intent_t* intent) {
	shd_act_node_t* activity_node;
	shd_act_t* activity;
	shd_app_t* app;

	shd_intent_filter_node_t* intent_filter_node;
	shd_intent_filter_t* intent_filter;

	shd_intent_filter_result_node_t* final_node = NULL;
	shd_intent_filter_result_node_t* intent_filter_result_node = NULL;
	shd_intent_filter_result_t intent_filter_result;

	for (size_t i = 0; i < apps->size; i++) {
		app = apps->list[i];
		activity_node = app->activities;
		
		if (intent->activity != NULL) {
			while (activity_node != NULL) {
				activity = activity_node->activity;
				
				if (strcmp(intent->activity, activity->id) == 0) {
					intent_filter_result_node = malloc(sizeof(shd_intent_filter_result_node_t));

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
						intent_filter_result_node = malloc(sizeof(shd_intent_filter_result_node_t));
					} else {
						intent_filter_result_node->next = malloc(sizeof(shd_intent_filter_result_node_t));
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
