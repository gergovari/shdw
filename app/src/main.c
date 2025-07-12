#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <zephyr/drivers/display.h>
#include <zephyr/drivers/rtc.h>

#include <stdio.h>
#include <time.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

#include <lvgl.h>

#include "app.h"
#include "apps/apps.h"
#include "activity_manager.h"

void lv_run(const struct device* display) {
	lv_timer_handler();
	display_blanking_off(display);

	while (1) {
		lv_timer_handler();
		k_sleep(K_MSEC(10));
	}
}

void init_rtc_time(struct rtc_time *rtc_time) {
	const char* date = __DATE__;
	const char* time = __TIME__;
	char month[4];
	int day, year;
	int month_num = 0;
	int hour, minute, second;
	struct tm time_info;

	sscanf(date, "%3s %d %d", month, &day, &year);

	if (strcmp(month, "Jan") == 0) month_num = 0;
	else if (strcmp(month, "Feb") == 0) month_num = 1;
	else if (strcmp(month, "Mar") == 0) month_num = 2;
	else if (strcmp(month, "Apr") == 0) month_num = 3;
	else if (strcmp(month, "May") == 0) month_num = 4;
	else if (strcmp(month, "Jun") == 0) month_num = 5;
	else if (strcmp(month, "Jul") == 0) month_num = 6;
	else if (strcmp(month, "Aug") == 0) month_num = 7;
	else if (strcmp(month, "Sep") == 0) month_num = 8;
	else if (strcmp(month, "Oct") == 0) month_num = 9;
	else if (strcmp(month, "Nov") == 0) month_num = 10;
	else if (strcmp(month, "Dec") == 0) month_num = 11;

	sscanf(time, "%d:%d:%d", &hour, &minute, &second);

	time_info.tm_year = rtc_time->tm_year;
	time_info.tm_mon = rtc_time->tm_mon;
	time_info.tm_mday = rtc_time->tm_mday;
	time_info.tm_hour = rtc_time->tm_hour;
	time_info.tm_min = rtc_time->tm_min;
	time_info.tm_sec = rtc_time->tm_sec;
	time_info.tm_isdst = -1;
	mktime(&time_info);

	rtc_time->tm_sec = second;
	rtc_time->tm_min = minute;
	rtc_time->tm_hour = hour;
	rtc_time->tm_mday = day;
	rtc_time->tm_mon = month_num;
	rtc_time->tm_year = year - 1900;
	rtc_time->tm_nsec = 0;
	rtc_time->tm_wday = time_info.tm_wday;
	rtc_time->tm_yday = time_info.tm_yday;
	rtc_time->tm_isdst = time_info.tm_isdst;
}

int init_rtc(const struct device* rtc) {
	struct rtc_time time;

	if (!device_is_ready(rtc)) {
		LOG_ERR("RTC device not ready!");
		return -EIO;
	}

	if (rtc_get_time(rtc, &time) != 0) {
		init_rtc_time(&time);
		if (rtc_set_time(rtc, &time) != 0) {
			LOG_ERR("RTC device time cannot be set.");
			return -EIO;
		}
	}

	return 0;
}

int init_display(const struct device* display) {
	if (!device_is_ready(display)) {
		LOG_ERR("Display device not ready!");
		return -EIO;
	}
	return 0;
}

int init_devices(const struct device* display,
		const struct device* rtc) {
	int ret = 0;
	
	ret = init_display(display);
	ret = init_rtc(rtc);
	
	return ret;
}

int main(void)
{
	const struct device* display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	const struct device* rtc = DEVICE_DT_GET(DT_ALIAS(rtc));
	
	app_t* app_list[] = {
		&shd_launcher,
		&shd_dummy,
		&shd_clock,
	};
	apps_t apps = {
		.list = app_list,
		.size = sizeof(app_list)/sizeof(app_t*)
	};

	if (init_devices(display, rtc) != 0) {
		LOG_ERR("Devices init failed!");
	}
	
	if (start_home_activity(&apps) != 0) {
		LOG_ERR("Couldn't launch HOME activity!");
	}

	lv_run(display);
}
