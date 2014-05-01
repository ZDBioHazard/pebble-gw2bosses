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

#define BOSS_SECTION_CURRENT 0
#define BOSS_SECTION_UPCOMING 1

struct boss {
    const unsigned char hour;
    const unsigned char min;
    const char *name;
    const char *zone;
    unsigned int time;
};

/* menu.c */
MenuLayer *boss_menu_layer_create( const GRect bounds );

/* boss.c */
void update_boss_times( const struct tm *time );
struct boss *get_boss_info( const int section, const int row );

/* time.c */
time_t bad_mktime( const struct tm *time );

#endif /* #ifndef _GW2BOSSES_H */
