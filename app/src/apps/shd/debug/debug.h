#pragma once

#include "../../../app.h"

extern app_t shd_debug;

void shd_debug_main_entry(lv_obj_t* screen, activity_callback cb, void* input, void* user);
void shd_debug_main_exit(lv_obj_t* screen);
