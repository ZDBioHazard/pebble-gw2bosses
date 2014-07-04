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

#ifndef _GW2BOSSES_H
#define _GW2BOSSES_H

#include <pebble.h>

/*****************************************************************************/

/* AppMessage keys. */
#define APPMSG_KEY_TZ_OFFSET 0

/* Persistent storage keys. */
#define PERSIST_KEY_TZ_OFFSET    0 /* int32_t bytes */
#define PERSIST_KEY_DATA_VERSION 1 /* int32_t bytes */
#define PERSIST_KEY_REMINDERS    2 /* (bool * EVENT_COUNT) bytes */

/*****************************************************************************/

typedef unsigned char event_t;
typedef signed short tz_offset_t;

struct event {
    const unsigned char hour;
    const unsigned char min;
    const char *name;
    const char *zone;
};

/*****************************************************************************/

/* menu.c */
MenuLayer *event_menu_layer_create( const GRect bounds );

/* event.c */
event_t get_event_count( const bool active );
const struct event *get_event_info( const bool active, const event_t index );
signed int get_event_timer( const event_t index );
bool get_event_reminder( const bool active, const event_t index );

void save_event_reminders( void );
void load_event_reminders( void );
void toggle_event_reminder( const bool active, const event_t index );

void update_event_times( const struct tm *time );

/* time.c */
time_t bad_difftime( const struct tm *time1, const struct tm *time2 );

void set_tz_offset( signed short offset );
bool have_tz_offset( void );

bool time_convert_utc_to_local( struct tm *time );
bool time_convert_local_to_utc( struct tm *time );

#endif /* #ifndef _GW2BOSSES_H */
