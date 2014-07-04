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

#define MENU_HEADER_HEIGHT 17
#define MENU_CELL_HEIGHT 30

#define MENU_SECTION_COUNT 2
#define MENU_SECTION_CURRENT 0
#define MENU_SECTION_COMINGUP 1

/*****************************************************************************/

static int16_t menu_get_header_height( MenuLayer *layer, const uint16_t index, void *data ){
    return MENU_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height( MenuLayer *layer, MenuIndex *index, void *data ){
    return MENU_CELL_HEIGHT;
}

/*****************************************************************************/

static uint16_t menu_get_num_sections( MenuLayer *layer, void *data ){
    return MENU_SECTION_COUNT;
}

static uint16_t menu_get_num_rows( MenuLayer *layer, const uint16_t index, void *data ){
    return get_event_count(!index);
}

/*****************************************************************************/

/* Just draw a basic header. */
static void menu_draw_header( GContext *ctx, const Layer *cell, const uint16_t index, void *data ){
    char *titles[MENU_SECTION_COUNT] = { "Happening Now", "Coming Up" };

    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_text_color(ctx, GColorBlack);

    graphics_draw_rect(ctx, (GRect){{-1, 0}, {146, MENU_HEADER_HEIGHT}});
    graphics_draw_text(ctx, titles[index],
                       fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       (GRect){{0, -2}, {144, MENU_HEADER_HEIGHT}},
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter, NULL);
}

/* Draw individual rows. */
static void menu_draw_row( GContext *ctx, const Layer *layer, MenuIndex *cell, void *data ){
    const struct event *event = get_event_info(!cell->section, cell->row);
    unsigned char offset = 0;
    unsigned char width = 0;

    /* Skip the timer on the current entries. */
    /* TODO Display an uptime counter for the current event. */
    if ( cell->section == MENU_SECTION_COMINGUP ){
        unsigned char timer_width[] = { 0, 12, 20, 24, 32, 40, 44, 52, 60 };
        unsigned char start_width[] = { 0, 10, 16, 20, 26, 32, 42, 44, 50 };
        signed int time = get_event_timer(cell->row);
        char timer[9] = { 0 };
        char start[9] = { 0 };

        /* Create the time string. Use a different formula for > 1hr times. */
        if ( time >= 3600 )
            snprintf(timer, sizeof(timer), "%d:%02d:%02d",
                     time / 3600, (time / 60) % 60, time % 60);
        else
            snprintf(timer, sizeof(timer), "%d:%02d",
                     time / 60, time % 60);

        /* We need a representation of the event start time, so make one.
         * It also needs to be adjusted for the current time zone. Argh. :S */
        /* FIXME Someday, Pebble might have a working timezone system. :( */
        struct tm event_tm = { 0 };
        event_tm.tm_year = 112; /* bad_mktime() has issues with 1900. ;) */
        event_tm.tm_hour = event->hour;
        event_tm.tm_min  = event->min;
        time_convert_utc_to_local(&event_tm);

        /* Create the event start timer. */
        if ( clock_is_24h_style() == true )
            snprintf(start, sizeof(start), "@%02d:%02d",
                     event_tm.tm_hour, event_tm.tm_min);
        else /* Silly 12-hour format. :p */
            snprintf(start, sizeof(start), "%d:%02d %s",
                     ( event_tm.tm_hour == 0 ) ? 12 : event_tm.tm_hour % 12,
                     event_tm.tm_min, ( event_tm.tm_hour < 12 ) ? "AM" : "PM");

        /* Set the box widths based on time string lengths. I tried using
         * graphics_text_layout_get_content_size() for this, but it seemed
         * to make scrolling slower, so we're using lookup tableis instead. */
        width = ( timer_width[strlen(timer)] > start_width[strlen(start)] ) ?
                  timer_width[strlen(timer)] : start_width[strlen(start)];

        /* Display the timer cell white-on-black. */
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_context_set_text_color(ctx, GColorWhite);

        /* Fill the timer cell. */
        graphics_fill_rect(ctx, (GRect){{144 - width, 0},
                                        {width, MENU_CELL_HEIGHT}},
                           0, GCornerNone);

        /* Draw the event timer. */
        graphics_draw_text(ctx, timer,
                           fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                           (GRect){{142 - width, -4},
                                   {width, (MENU_CELL_HEIGHT / 2) + 4}},
                           GTextOverflowModeWordWrap,
                           GTextAlignmentRight, NULL);

        /* Draw the event real time. */
        graphics_draw_text(ctx, start,
                           fonts_get_system_font(FONT_KEY_GOTHIC_14),
                           (GRect){{142 - width, (MENU_CELL_HEIGHT / 2) - 2},
                                   {width, (MENU_CELL_HEIGHT / 2) + 2}},
                           GTextOverflowModeWordWrap,
                           GTextAlignmentRight, NULL);
    }

    /* Change the text color back to black for the left cell. */
    graphics_context_set_text_color(ctx, GColorBlack);

    /* Draw a reminder icon. */
    if ( get_event_reminder(!cell->section, cell->row) == true ){
        /* Set the text frame offset so we have space to draw. */
        offset = 10;

        /* Draw a black cell. */
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_fill_rect(ctx, (GRect){{0, 0}, {offset, MENU_CELL_HEIGHT}},
                           0, GCornerNone);

        /* Draw an exclamation point using 2 tiny rectangles. */
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_draw_rect(ctx, (GRect){{4, 6}, {2, 12}});
        graphics_draw_rect(ctx, (GRect){{4, 22}, {2, 2}});
    }

    /* Draw the event title. */
    graphics_draw_text(ctx, event->name,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       (GRect){{2 + offset, -2},
                              {140 - (width + offset),
                               (MENU_CELL_HEIGHT / 2) + 2}},
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentLeft, NULL);

    /* Draw the event location. */
    graphics_draw_text(ctx, event->zone,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       (GRect){{2 + offset, (MENU_CELL_HEIGHT / 2) - 3},
                               {140 - (width + offset),
                                (MENU_CELL_HEIGHT / 2) + 2}},
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentLeft, NULL);
}

/*****************************************************************************/

void menu_select_click( MenuLayer *layer, MenuIndex *cell, void *data ){
    toggle_event_reminder(!cell->section, cell->row);
    layer_mark_dirty(menu_layer_get_layer(layer));
}

/*****************************************************************************/

MenuLayer *event_menu_layer_create( const GRect bounds ){
    MenuLayer *menu_layer = menu_layer_create(bounds);

    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
        .get_header_height = menu_get_header_height,
        .get_cell_height = menu_get_cell_height,
        .get_num_sections = menu_get_num_sections,
        .get_num_rows = menu_get_num_rows,
        .draw_header = menu_draw_header,
        .draw_row = menu_draw_row,
        .select_click = menu_select_click,
    });

    return menu_layer;
}
