#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SETTINGS_PAGE = 2,
    SCREEN_ID_DRAWER_PAGE = 3,
    _SCREEN_ID_LAST = 3
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *settings_page;
    lv_obj_t *drawer_page;
    lv_obj_t *btn_suspend;
    lv_obj_t *container_battery_voltage;
    lv_obj_t *battery_status;
    lv_obj_t *btn_settings_page;
    lv_obj_t *btn_app_drawer_page;
    lv_obj_t *obj0;
    lv_obj_t *brightness_slider;
    lv_obj_t *btn_home_page;
    lv_obj_t *obj1;
    lv_obj_t *btn_bt_toggle;
    lv_obj_t *img_bt;
    lv_obj_t *btn_home_drawer_page;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_settings_page();
void tick_screen_settings_page();

void create_screen_drawer_page();
void tick_screen_drawer_page();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/