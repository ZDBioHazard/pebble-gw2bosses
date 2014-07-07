/******************************************************************************
 *
 * gw2bosses - A simple Guild Wars 2 boss timer display.
 *
 * Copyright 2014 Ryan "BioHazard" Turner <zdbiohazard2@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include "gw2bosses.h"

static MenuLayer *event_menu = NULL;
static TextLayer *tz_message = NULL;

/*****************************************************************************/

static void tick_second_handler( struct tm *time, const TimeUnits unit ){
    /* Bail out here if the timezone isn't set. */
    if ( have_tz_offset() == false )
        return;

    /* If the time zone message exists, we can remove it now. */
    if ( tz_message != NULL ){
        text_layer_destroy(tz_message);
        tz_message = NULL;
    }

    /* Get the UTC time and update the timers with it. */
    struct tm utc = *time;
    time_convert_local_to_utc(&utc);
    update_event_times(&utc);

    /* Reload the menu in case row counts change. */
    menu_layer_reload_data(event_menu);

    /* The menu layer is created hidden so we don't see all the timers
     * set to 0:00 before the first valid second. It can be shown now. */
    layer_set_hidden(menu_layer_get_layer(event_menu), false);
    layer_mark_dirty(menu_layer_get_layer(event_menu));
}

/*****************************************************************************/

static void window_load( Window *window ){
    Layer *window_layer = window_get_root_layer(window);

    /* Create the event menu, and bind it to this window. */
    event_menu = event_menu_layer_create(layer_get_frame(window_layer));
    menu_layer_set_click_config_onto_window(event_menu, window);
    layer_add_child(window_layer, menu_layer_get_layer(event_menu));
    layer_set_hidden(menu_layer_get_layer(event_menu), true);

    /* On the first run, the time zone offset must be fetched from the phone.
     * This creates a message box telling the user what's happening. */
    if ( have_tz_offset() == false ){
        tz_message = text_layer_create((GRect){{6, 52}, {132, 44}});
        text_layer_set_text(tz_message, "Getting time zone from your phone");
        text_layer_set_background_color(tz_message, GColorBlack);
        text_layer_set_text_alignment(tz_message, GTextAlignmentCenter);
        text_layer_set_font(tz_message, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
        text_layer_set_text_color(tz_message, GColorWhite);
        layer_add_child(window_layer, text_layer_get_layer(tz_message));
    }

    load_event_reminders();

    tick_timer_service_subscribe(SECOND_UNIT, tick_second_handler);
}

static void window_unload( Window *window ){
    save_event_reminders();

    if ( tz_message != NULL )
        text_layer_destroy(tz_message);
    menu_layer_destroy(event_menu);
}

/*****************************************************************************/

/* Receive time zone information from the phone. */
static void in_received_handler( DictionaryIterator *data, void *context ){
    Tuple *tuple = dict_find(data, APPMSG_KEY_TZ_OFFSET);

    /* Just bail here if this isn't the right type of data. */
    if ( tuple == NULL || tuple->type != TUPLE_INT )
        return;

    /* Update the offset. */
    tz_offset_t offset = tuple->value->int16;
    APP_LOG(APP_LOG_LEVEL_INFO, "Got offset %d from phone.", offset);
    set_tz_offset(offset);
}

/* Just shoot off an error log when the timezone message is dropped. */
static void in_dropped_handler( const AppMessageResult reason, void *context ){
    APP_LOG(APP_LOG_LEVEL_ERROR, "An AppMessage was dropped: %d", reason);
}

/*****************************************************************************/

int main( void ){
    /* Set-up the app messaging system so we can get an updated time zone. */
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_open(16, 16);

    /* Create the main window. */
    Window *window = window_create();
    window_set_window_handlers(window, (WindowHandlers){
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(window, true);

    app_event_loop();

    window_destroy(window);
}
