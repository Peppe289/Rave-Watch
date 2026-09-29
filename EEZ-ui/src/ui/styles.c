#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: PrimaryContainer
//

void init_style_primary_container_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_radius(style, 10);
    lv_style_set_bg_color(style, lv_color_hex(0xe9678f));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_primary_container_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_primary_container_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_primary_container(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_primary_container_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_primary_container(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_primary_container_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: SecondaryContainer
//

void init_style_secondary_container_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0x00407c));
    lv_style_set_bg_opa(style, 255);
    lv_style_set_radius(style, 10);
};

lv_style_t *get_style_secondary_container_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_secondary_container_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_secondary_container(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_secondary_container_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_secondary_container(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_secondary_container_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: ClockStyle
//

void init_style_clock_style_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0x000000));
    lv_style_set_border_color(style, lv_color_hex(0x000000));
    lv_style_set_border_opa(style, 0);
    lv_style_set_text_color(style, lv_color_hex(0xffffff));
    lv_style_set_text_opa(style, 255);
    lv_style_set_text_font(style, &lv_font_montserrat_48);
    lv_style_set_text_align(style, LV_TEXT_ALIGN_CENTER);
};

lv_style_t *get_style_clock_style_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_clock_style_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_clock_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_clock_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_clock_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_clock_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_primary_container,
        add_style_secondary_container,
        add_style_clock_style,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_primary_container,
        remove_style_secondary_container,
        remove_style_clock_style,
    };
    remove_style_funcs[styleIndex](obj);
}