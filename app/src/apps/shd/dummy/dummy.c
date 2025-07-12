#include "dummy.h"

#include <stdio.h>

void shd_dummy_main_entry(lv_obj_t* screen, activity_callback cb, void* user) {
	//printf("dummy opened.\n");
	//cb(user, 0, NULL);
}
void shd_dummy_main_exit(lv_obj_t* screen) {
	//printf("dummy closed.\n");
}

intent_filter_t shd_dummy_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_LAUNCHER
};
intent_filter_node_t shd_dummy_intent_filter_node = { 
	.intent_filter = &shd_dummy_filter, 
	.next = NULL 
};
activity_t shd_dummy_main = {
	.id = "shd.dummy.main",
	.intent_filters = &shd_dummy_intent_filter_node,
	.entry = shd_dummy_main_entry,
	.exit = shd_dummy_main_exit
};
activity_node_t shd_dummy_activity_node = { 
	.activity = &shd_dummy_main, 
	.next = NULL 
};
app_t shd_dummy = {
	.id = "shd.dummy",
	.title = "Dummy",
	.activities = &shd_dummy_activity_node
};
