#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: PrimaryContainer
lv_style_t *get_style_primary_container_MAIN_DEFAULT();
void add_style_primary_container(lv_obj_t *obj);
void remove_style_primary_container(lv_obj_t *obj);

// Style: SecondaryContainer
lv_style_t *get_style_secondary_container_MAIN_DEFAULT();
void add_style_secondary_container(lv_obj_t *obj);
void remove_style_secondary_container(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/