/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <kernel.h>
#include <bluetooth/services/bas.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/display/widgets/output_status.h>
#include <zmk/event_manager.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_selection_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static bool style_initialized = false;

K_MUTEX_DEFINE(output_status_mutex);

struct {
    enum zmk_endpoint selected_endpoint;
    bool active_profile_connected;
    bool active_profile_bonded;
    uint8_t active_profile_index;
} output_status_state;

void output_status_init() {
    if (style_initialized) {
        return;
    }

    style_initialized = true;
    lv_style_init(&label_style);
    lv_style_set_text_color(&label_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_text_font(&label_style, LV_STATE_DEFAULT, &lv_font_montserrat_16);
    lv_style_set_text_letter_space(&label_style, LV_STATE_DEFAULT, 1);
    lv_style_set_text_line_space(&label_style, LV_STATE_DEFAULT, 1);
}

void set_status_symbol(lv_obj_t *label) {
    char text[6] = {};

    k_mutex_lock(&output_status_mutex, K_FOREVER);
    enum zmk_endpoint selected_endpoint = output_status_state.selected_endpoint;
    bool active_profile_connected = output_status_state.active_profile_connected;
    bool active_profie_bonded = output_status_state.active_profile_bonded;
    uint8_t active_profile_index = output_status_state.active_profile_index;
    k_mutex_unlock(&output_status_mutex);

    switch (selected_endpoint) {
    case ZMK_ENDPOINT_USB:
        strcat(text, LV_SYMBOL_USB "   ");
        break;
    case ZMK_ENDPOINT_BLE:
        if (state.active_profile_bonded) {
            if (state.active_profile_connected) {
                snprintf(text, sizeof(text), LV_SYMBOL_WIFI "%i " LV_SYMBOL_OK,
                         state.active_profile_index);
            } else {
                snprintf(text, sizeof(text), LV_SYMBOL_WIFI "%i " LV_SYMBOL_CLOSE,
                         state.active_profile_index);
            }
        } else {
            snprintf(text, sizeof(text), LV_SYMBOL_WIFI "%i " LV_SYMBOL_SETTINGS,
                     state.active_profile_index);
        }
        break;
    }

    lv_label_set_text(label, text);
}

static void output_status_update_cb(struct output_status_state state) {
    struct zmk_widget_output_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_status_symbol(widget->obj, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_selection_changed);

#if defined(CONFIG_USB)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

int zmk_widget_output_status_init(struct zmk_widget_output_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent, NULL);

    lv_obj_set_size(widget->obj, 40, 15);

    sys_slist_append(&widgets, &widget->node);

    widget_output_status_init();
    return 0;
}

lv_obj_t *zmk_widget_output_status_obj(struct zmk_widget_output_status *widget) {
    return widget->obj;
}

void output_status_update_cb(struct k_work *work) {
    struct zmk_widget_output_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_status_symbol(widget->obj); }
}

K_WORK_DEFINE(output_status_update_work, output_status_update_cb);

int output_status_listener(const zmk_event_t *eh) {
    // Be sure we have widgets initialized before doing any work,
    // since the status event can fire before display code inits.
    if (!style_initialized) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    k_mutex_lock(&output_status_mutex, K_FOREVER);

    output_status_state.selected_endpoint = zmk_endpoints_selected();
    output_status_state.active_profile_connected = zmk_ble_active_profile_is_connected();
    output_status_state.active_profile_bonded = !zmk_ble_active_profile_is_open();
    output_status_state.active_profile_index = zmk_ble_active_profile_index();
    k_mutex_unlock(&output_status_mutex);

    k_work_submit_to_queue(zmk_display_work_q(), &output_status_update_work);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(widget_output_status, output_status_listener)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_selection_changed);

#if defined(CONFIG_USB)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif
