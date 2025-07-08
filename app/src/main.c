#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>

#include <stdio.h>
#include <string.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

#include <lvgl.h>
#include <lvgl_input_device.h>

#include "lv_launcher.h"

char* app_ids[] = {
	"shd.clock",
	"shd.timer",
	"shd.alarm",
	"shd.notes",
	"shd.chats",
	"shd.weather",
	"shd.maps",
};

char* app_names[] = {
	"Clock",
	"Timer",
	"Alarm",
	"Notes",
	"Chats",
	"Weather",
	"Maps",
};

lv_obj_t* root_screen;

void shd_clock(lv_obj_t* cont) {
	lv_obj_t* clock = lv_label_create(cont);

	lv_label_set_text(clock, "12:34");
	lv_obj_center(clock);
}

void shd_dummy(lv_obj_t* cont) {
	printf("dummy opened.\n");
}

void (*app_funcs[])(lv_obj_t*) = {
	shd_clock,
	shd_dummy,
	shd_dummy,
	shd_dummy,
	shd_dummy,
	shd_dummy,
	shd_dummy,
};

void lv_run(const struct device* display) {
	lv_timer_handler();
	display_blanking_off(display);

	while (1) {
		lv_timer_handler();
		k_sleep(K_MSEC(10));
	}
}

int main(void)
{
	const struct device* display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	
	if (!device_is_ready(display)) {
		LOG_ERR("Display device not ready!");
		return 0;
	}
	
	root_screen = lv_screen_active();
	lv_launcher_create(root_screen, app_ids, app_names, app_funcs sizeof(app_ids)/sizeof(char*));

	lv_run(display);
}
