#include "lv_launcher.h"

#include <lvgl.h>

int lv_launcher_launch(lv_obj_t* cont, char* id) {
	for (unsigned int i = 0; i < sizeof(app_ids)/sizeof(char*); i++) {
		if (strcmp(app_ids[i], id) == 0) {
			app_funcs[i](cont);
			return 0;
		}
	}
	return 1;
}

void lv_launcher_close_cb(lv_event_t *e) {
	lv_obj_t* screen = (lv_obj_t*)lv_event_get_user_data(e);

	lv_obj_delete(screen);
	lv_screen_load(root_screen);
}

lv_obj_t* lv_launcher_win_create(lv_obj_t *screen, char* title) {
	lv_obj_t* win = lv_win_create(screen);
	lv_obj_t* close_btn;

	lv_win_add_title(win, title);

	close_btn = lv_win_add_button(win, LV_SYMBOL_CLOSE, 40);
	lv_obj_add_event_cb(close_btn, lv_launcher_close_cb, LV_EVENT_CLICKED, screen);

	return win;
}

lv_obj_t* lv_launcher_cont_create(lv_obj_t** screen, char* title) {
	lv_obj_t* window;

	*screen = lv_obj_create(NULL);
	window = lv_launcher_win_create(*screen, title);

	return lv_win_get_content(window);
}

void lv_launcher_click_cb(lv_event_t *e) {
	void = (char*)lv_event_get_user_data(e);
	lv_obj_t* screen;
	lv_obj_t* cont = lv_launcher_cont_create(&screen, id);

	lv_screen_load(screen);
	
	if (lv_launcher_launch(cont, id) != 0) {
		printf("%s NOT launched.\n", id);
	}
}

void lv_launcher_entry_create(lv_obj_t* root, char* id, char* title) {
	lv_obj_t* cont = lv_menu_cont_create(root);
	lv_obj_t* btn = lv_button_create(cont);
	lv_obj_t* label = lv_label_create(btn); 

	lv_obj_add_event_cb(btn, lv_launcher_click_cb, LV_EVENT_CLICKED, id);
	lv_label_set_text(label, title);
}

void lv_launcher_create(lv_obj_t* parent, char *app_ids, char* app_names, char* app_funcs unsigned int size) {
	lv_obj_t* menu = lv_menu_create(parent);
	lv_obj_t* root = lv_menu_page_create(menu, NULL);

	lv_obj_set_size(menu, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL));
	lv_obj_center(menu);
	
	for (unsigned int i = 0; i < size; i++) {
		lv_launcher_entry_create(root, app_ids[i], app_names[i]);
	}

	lv_menu_set_page(menu, root);
}
