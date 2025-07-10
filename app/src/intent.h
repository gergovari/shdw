#pragma once

#include <stdint.h>
#include <zephyr/sys/util_macro.h>

#define ACTION_MAIN BIT64(0)

#define CATEGORY_LAUNCHER BIT64(0)
#define CATEGORY_DEFAULT BIT64(1)
#define CATEGORY_HOME BIT64(2)

typedef struct intent_filter_ts intent_filter_t;
typedef struct intent_filter_node_ts intent_filter_node_t;
typedef struct intent_ts intent_t;

struct intent_filter_ts {
	int64_t action;
	int64_t category;

	char* type;
};

struct intent_filter_node_ts {
	intent_filter_t* intent_filter;
	intent_filter_node_t* next;
};

struct intent_ts {
	char* activity;

	int64_t action;
	int64_t category;

	void* user;
	char* type;
};
