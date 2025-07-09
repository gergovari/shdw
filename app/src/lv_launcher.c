#include "lv_launcher.h"

#include <lvgl.h>
#include <stdlib.h>

#include <stdio.h>

#include "activity_manager.h"

void lv_launcher_click_cb(lv_event_t *e) {
	lv_launcher_ctx* ctx = (lv_launcher_ctx*)lv_event_get_user_data(e);
	
	start_activity(ctx->activity, NULL);
}

void lv_launcher_entry_create(lv_launcher_ctx* ctx) {
	lv_obj_t* cont = lv_menu_cont_create(ctx->root);
	lv_obj_t* btn = lv_button_create(cont);
	lv_obj_t* label = lv_label_create(btn);

	lv_obj_add_event_cb(btn, lv_launcher_click_cb, LV_EVENT_CLICKED, ctx);
	lv_label_set_text(label, ctx->app->title);
}

void lv_launcher_main_entry(lv_obj_t* screen, activity_callback cb, void* user) {
	lv_obj_t* menu = lv_menu_create(screen);
	lv_obj_t* root = lv_menu_page_create(menu, NULL);
	lv_launcher_ctx* ctx;

	lv_obj_set_user_data(screen, ctx);

	lv_obj_set_size(menu, 
			lv_display_get_horizontal_resolution(NULL), 
			lv_display_get_vertical_resolution(NULL));
	lv_obj_center(menu);
	
	for (size_t i = 0; i < size; i++) {
		ctx = malloc(sizeof(lv_launcher_ctx));
		ctx->root = root;
		ctx->app = app;
		ctx->activity = activity;

		lv_launcher_entry_create(ctx);
	}

	lv_menu_set_page(menu, root);
}

void lv_launcher_main_exit(lv_obj_t* screen) {
	// TODO: free all item ctxs
}

intent_filter_t lv_launcher_filter = {
	.action = ACTION_MAIN,
	.category = CATEGORY_HOME
};
activity_t lv_launcher_main = {
	.id = "lv.launcher.main",
	.filters = { .filter = &lv_launcher_filter, .next = NULL },
	.entry = lv_launcher_main_entry,
	.exit = lv_launcher_main_exit
};
app_t lv_launcher = {
	.id = "lv.launcher",
	.title = "Launcher",
	.activities = { .activity = &lv_launcher_main, .next = NULL }
};
