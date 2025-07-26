#pragma once

#include "activity.h"

#include <stddef.h>

typedef struct {
	char* id;
	char* title;
	
	shd_act_node_t* activities;
} shd_app_t;

typedef struct shd_apps_t {
	shd_app_t** list;
	size_t size;
} shd_apps_t;
