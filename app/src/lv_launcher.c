#include "lv_launcher.h"

#include <lvgl.h>
#include <stdlib.h>

#include <stdio.h>

void lv_launcher_close_cb(lv_event_t *e) {
	lv_launcher_ctx* ctx = (lv_launcher_ctx*)lv_event_get_user_data(e);

	ctx->app->exit(ctx->cont);
	lv_screen_load(ctx->parent);
	lv_obj_delete(ctx->screen);
}

lv_obj_t* lv_launcher_win_create(lv_launcher_ctx* ctx) {
	lv_obj_t* win = lv_win_create(ctx->screen);
	lv_obj_t* close_btn;

	lv_win_add_title(win, ctx->app->title);

	close_btn = lv_win_add_button(win, LV_SYMBOL_CLOSE, 40);
	lv_obj_add_event_cb(close_btn, lv_launcher_close_cb, LV_EVENT_CLICKED, ctx);

	return win;
}

lv_obj_t* lv_launcher_cont_create(lv_launcher_ctx* ctx) {
	lv_obj_t* win;

	ctx->screen = lv_obj_create(NULL);
	win = lv_launcher_win_create(ctx);

	return lv_win_get_content(win);
}

void lv_launcher_click_cb(lv_event_t *e) {
	lv_launcher_ctx* ctx = (lv_launcher_ctx*)lv_event_get_user_data(e);

	ctx->cont = lv_launcher_cont_create(ctx);
	lv_screen_load(ctx->screen);
	ctx->app->entry(ctx->cont);
}

void lv_launcher_entry_create(lv_launcher_ctx* ctx) {
	lv_obj_t* cont = lv_menu_cont_create(ctx->root);
	lv_obj_t* btn = lv_button_create(cont);
	lv_obj_t* label = lv_label_create(btn);

	lv_obj_add_event_cb(btn, lv_launcher_click_cb, LV_EVENT_CLICKED, ctx);
	lv_label_set_text(label, ctx->app->title);
}

void lv_launcher_create(lv_obj_t* parent, 
		app_t* apps,
		unsigned int size) {
	lv_obj_t* menu = lv_menu_create(parent);
	lv_obj_t* root = lv_menu_page_create(menu, NULL);
	lv_launcher_ctx* ctx;

	lv_obj_set_size(menu, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL));
	lv_obj_center(menu);
	
	for (unsigned int i = 0; i < size; i++) {
		ctx = malloc(sizeof(lv_launcher_ctx)); // TODO: destroy ctxs if launcher destroyed
		ctx->parent = parent;
		ctx->root = root;
		ctx->app = &apps[i];

		lv_launcher_entry_create(ctx);
	}

	lv_menu_set_page(menu, root);
}
