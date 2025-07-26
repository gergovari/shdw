#pragma once

#include <lvgl.h>

lv_obj_t* lv_screen_create(lv_display_t* display);
lv_display_t* lv_display_or_default(lv_display_t* display);
