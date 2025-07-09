#pragma once

#include <lvgl.h>

#include "app.h"

extern app_t shd_clock;

void shd_clock_main_entry(lv_obj_t* screen, activity_callback cb, void* user);
void shd_clock_main_exit(lv_obj_t* screen);

void shd_dummy_main_entry(lv_obj_t* screen, activity_callback cb, void* user);
void shd_dummy_main_exit(lv_obj_t* screen);
