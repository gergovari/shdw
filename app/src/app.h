#pragma once

#include "activity.h"

typedef struct app_t {
	char* id;
	char* title;
	
	activity_node_t* activities;
} app_t;
