#include "shd_apps.h"

#include <lvgl.h>
#include <stdio.h>

#include <zephyr/drivers/rtc.h>

void shd_clock(lv_obj_t* cont) {
	const struct device *const rtc = DEVICE_DT_GET(DT_ALIAS(rtc));
	struct rtc_time tm;
	lv_obj_t* clock = lv_label_create(cont);
	static lv_style_t style;

	lv_style_init(&style);
	lv_style_set_text_font(&style, &lv_font_montserrat_48);
	lv_obj_add_style(clock, &style, 0);
	lv_obj_center(clock);

	if (rtc_get_time(rtc, &tm) == 0) {
		lv_label_set_text(clock, "12:34");
	} else {
		lv_label_set_text(clock, "ERROR");
	}
}

void shd_dummy(lv_obj_t* cont) {
	printf("dummy opened.\n");
}
