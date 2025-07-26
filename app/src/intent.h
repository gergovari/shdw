#pragma once

#include <stdint.h>
#include <zephyr/sys/util_macro.h>

#define ACTION_MAIN BIT64(0)

#define CATEGORY_LAUNCHER BIT64(0)
#define CATEGORY_DEFAULT BIT64(1)
#define CATEGORY_HOME BIT64(2)
#define CATEGORY_DEBUG BIT64(3)


typedef int64_t shd_action_t;
typedef int64_t shd_category_t;

typedef struct {
	char* activity;

	shd_action_t action;
	shd_category_t category;

	void* input;
	char* type;

	void* user;
} shd_intent_t;
