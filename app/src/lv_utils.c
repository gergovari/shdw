#include "lv_utils.h"

lv_obj_t* lv_screen_create(lv_display_t* display) {
	lv_display_t* old_display = lv_display_get_default();
	lv_obj_t* screen;

	lv_display_set_default(display);
	screen = lv_obj_create(NULL);
	lv_display_set_default(old_display);
	
	return screen;
}

lv_display_t* lv_display_or_default(lv_display_t* display) {
	return display == NULL ? lv_display_get_default() : display;
}
