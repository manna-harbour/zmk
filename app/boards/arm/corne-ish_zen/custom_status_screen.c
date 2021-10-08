/*
*
* Copyright (c) 2021 Darryl deHaan
* SPDX-License-Identifier: MIT
*
*/

#include "widgets/battery_status.h"
#include "widgets/output_status.h"
#include "widgets/layer_status.h"
#include "custom_status_screen.h"

#include <logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

LV_IMG_DECLARE(zenlogo);
LV_IMG_DECLARE(miryokulogo);
LV_IMG_DECLARE(layers2);

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_BATTERY_STATUS)
static struct zmk_widget_battery_status battery_status_widget;
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_OUTPUT_STATUS)
static struct zmk_widget_output_status output_status_widget;
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_LAYER_STATUS)
static struct zmk_widget_layer_status layer_status_widget;
#endif

lv_obj_t *zmk_display_status_screen() {

    lv_obj_t *screen;
    screen = lv_obj_create(NULL, NULL);


#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_BATTERY_STATUS)
    zmk_widget_battery_status_init(&battery_status_widget, screen);
    lv_obj_align(zmk_widget_battery_status_obj(&battery_status_widget), NULL, LV_ALIGN_CENTER, 0, -43);
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_OUTPUT_STATUS)
    zmk_widget_output_status_init(&output_status_widget, screen);
    lv_obj_align(zmk_widget_output_status_obj(&output_status_widget), NULL, LV_ALIGN_CENTER, 0,
                 0);
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_LAYER_STATUS)
    //lv_style_set_pad_inner(&layerstyle, LV_STATE_DEFAULT, 12);
    //lv_obj_add_style(&layer_status_widget, LV_WIDGET_PART_MAIN, &layerstyle);
    zmk_widget_layer_status_init(&layer_status_widget, screen);
    lv_obj_align(zmk_widget_layer_status_obj(&layer_status_widget), NULL, LV_ALIGN_CENTER, 0, 43);
#endif

#if CONFIG_BOARD_CORNEISH_ZEN_RIGHT
    lv_obj_t * zenlogo_icon;
    zenlogo_icon = lv_img_create(screen, NULL);
    lv_img_set_src(zenlogo_icon, &zenlogo);
    lv_obj_align(zenlogo_icon, NULL, LV_ALIGN_CENTER, 0, 43);
#endif

#if CONFIG_BOARD_CORNEISH_ZEN_LEFT
    lv_obj_t * miryokulogo_icon;
    miryokulogo_icon = lv_img_create(screen, NULL);
    lv_img_set_src(miryokulogo_icon, &miryokulogo);
    lv_obj_align(miryokulogo_icon, NULL, LV_ALIGN_CENTER, 0, 43);
#endif

    //lv_task_handler();
    lv_refr_now(NULL);
    //display_blanking_off(display_dev);

    return screen;
}
