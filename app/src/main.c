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

char* app_ids[] = {
	"shd.clock",
	"shd.notes",
	"shd.chats",
	"shd.weather",
	"shd.maps"
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
	shd_dummy
};

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

lv_obj_t* lv_launcher_cont_create(lv_obj_t** screen, char* title) {
	lv_obj_t* window;
	lv_obj_t* close_btn;

	*screen = lv_obj_create(NULL);

	window = lv_win_create(*screen);
	lv_win_add_title(window, title);

	close_btn = lv_win_add_button(window, LV_SYMBOL_CLOSE, 40);
	lv_obj_add_event_cb(close_btn, lv_launcher_close_cb, LV_EVENT_CLICKED, *screen);

	return lv_win_get_content(window);
}

void lv_launcher_click_cb(lv_event_t *e) {
	char* id = (char*)lv_event_get_user_data(e);
	lv_obj_t* screen;
	lv_obj_t* cont = lv_launcher_cont_create(&screen, id);

	lv_screen_load(screen);
	
	if (lv_launcher_launch(cont, id) != 0) {
		printf("%s NOT launched.\n", id);
	}
}

void lv_launcher(void) {
	lv_obj_t* menu = lv_menu_create(lv_screen_active());
	lv_obj_t* cont;
	lv_obj_t* btn;
	lv_obj_t* label;
	lv_obj_t* root = lv_menu_page_create(menu, NULL);

	lv_obj_set_size(menu, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL));
	lv_obj_center(menu);
	
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
	
	if (!device_is_ready(display)) {
		LOG_ERR("Display device not ready!");
		return 0;
	}
	
	root_screen = lv_screen_active();
	lv_launcher();

	lv_run(display);
}
