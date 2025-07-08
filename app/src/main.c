#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>

#include <lvgl.h>
#include <lvgl_input_device.h>

#include <stdio.h>
#include <string.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

void lv_launcher_click_cb(lv_event_t *e) {
	void *data = lv_event_get_user_data(e);
	printf("%s\n", (char*)data);
}

void lv_launcher(void) {
	lv_obj_t* menu = lv_menu_create(lv_screen_active());
	lv_obj_t* cont;
	lv_obj_t* btn;
	lv_obj_t* label;
	lv_obj_t* root = lv_menu_page_create(menu, NULL);

	lv_obj_set_size(menu, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL));
	lv_obj_center(menu);
	
	char *app_ids[] = {
		"clock",
		"notes",
		"chats",
		"weather",
		"maps"
	};
	
	for (unsigned int i = 0; i < sizeof(app_ids)/sizeof(char*); i++) {
		cont = lv_menu_cont_create(root);
		btn = lv_button_create(cont);
		lv_obj_add_event_cb(btn, lv_launcher_click_cb, LV_EVENT_CLICKED, app_ids[i]);
		label = lv_label_create(btn);
		lv_label_set_text(label, app_ids[i]);
	}


	lv_menu_set_page(menu, root);
}

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
	
	printf("start\n");
	if (!device_is_ready(display)) {
		LOG_ERR("Display device not ready!");
		return 0;
	}
	
	lv_launcher();

	lv_run(display);
}
