#pragma once

#include "activity.h"

#include <stddef.h>

typedef struct shd_app_t {
	char* id;
	char* title;
	
	shd_act_node_t* activities;
} app_t;

typedef struct shd_apps_t {
	app_t** list;
	size_t size;
} shd_apps_t;
