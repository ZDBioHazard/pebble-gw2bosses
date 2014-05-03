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
#define PERSIST_KEY_TZ_OFFSET 0 /* 2 bytes */
#define PERSIST_KEY_REMINDERS 1 /* BOSS_COUNT bytes */

/*****************************************************************************/

typedef unsigned char boss_t;

struct boss {
    const unsigned char hour;
    const unsigned char min;
    const char *name;
    const char *zone;
};

/*****************************************************************************/

/* menu.c */
MenuLayer *boss_menu_layer_create( const GRect bounds );

/* boss.c */
const struct boss *get_boss_info( const bool active, const boss_t index );
signed int get_boss_timer( const boss_t index );
bool get_boss_reminder( const boss_t index );

void save_boss_reminders( void );
void load_boss_reminders( void );
void toggle_boss_reminder( const boss_t index );

void update_boss_times( const struct tm *time );

/* time.c */
time_t bad_mktime( const struct tm *time );

#endif /* #ifndef _GW2BOSSES_H */
