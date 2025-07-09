#pragma once

typedef struct app_t {
	char* id;
	char* title;

	void (*entry)(lv_obj_t*);
	void (*exit)();
} app_t;
