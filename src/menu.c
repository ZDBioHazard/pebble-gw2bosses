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

#define MENU_CURRENT_COUNT 1
#define MENU_COMINGUP_COUNT 32

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
    /* TODO There might be multiple concurrent events someday. */
    return ( index == MENU_SECTION_CURRENT ) ? MENU_CURRENT_COUNT : MENU_COMINGUP_COUNT;
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
    struct boss *boss = get_boss_info(cell->section ? false : true, cell->row);
    unsigned char width_list[] = { 0, 14, 24, 28, 38, 48, 52, 62, 72 };
    unsigned char width = width_list[0];
    char time_text[9] = { 0 };
    signed int timer = 0;

    graphics_context_set_text_color(ctx, GColorBlack);

    /* Skip the timer on the current entries. */
    /* TODO Display an uptime counter for the current event. */
    if ( cell->section == MENU_SECTION_COMINGUP ){
        timer = get_boss_timer(cell->row);

        /* Create the time string. Use a different formula for > 1 hour times. */
        if ( timer >= 3600 )
            snprintf(time_text, sizeof(time_text), "%d:%02d:%02d",
                     timer / 3600, (timer / 60) % 60, timer % 60);
        else
            snprintf(time_text, sizeof(time_text), "%d:%02d",
                     timer / 60, timer % 60);

        /* Set the box widths based on time string length. I tried using
         * graphics_text_layout_get_content_size() for this, but it seemed
         * to make scrolling slower, so we're using a lookup table instead. */
        width = width_list[strlen(time_text)];

        /* Draw the event timer. */
        graphics_draw_text(ctx, time_text,
                           fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                           (GRect){{142 - width, -2},
                                   {width, MENU_CELL_HEIGHT + 2}},
                           GTextOverflowModeWordWrap,
                           GTextAlignmentRight, NULL);
    }

    /* Draw the event title. */
    graphics_draw_text(ctx, boss->name,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       (GRect){{2, -2},
                               {140 - width, (MENU_CELL_HEIGHT / 2) + 2}},
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentLeft, NULL);

    /* Draw the event location. */
    graphics_draw_text(ctx, boss->zone,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       (GRect){{2, (MENU_CELL_HEIGHT / 2) - 3},
                               {140 - width, (MENU_CELL_HEIGHT / 2) + 2}},
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentLeft, NULL);
}

/*****************************************************************************/

MenuLayer *boss_menu_layer_create( const GRect bounds ){
    MenuLayer *menu_layer = menu_layer_create(bounds);

    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
        .get_header_height = menu_get_header_height,
        .get_cell_height = menu_get_cell_height,
        .get_num_sections = menu_get_num_sections,
        .get_num_rows = menu_get_num_rows,
        .draw_header = menu_draw_header,
        .draw_row = menu_draw_row,
    });

    return menu_layer;
}
