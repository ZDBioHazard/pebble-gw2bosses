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

#include <pebble.h>

/* Pebble doesn't have a working mktime(), so I wrote my own.
 * Note: This only works properly for the years 1901 to 2099. */
time_t bad_mktime( const struct tm *time ){
    short mdays[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

    return ( /* Awww yeah, dat return... */
        /* Start by converting years to days. */
        (((((((1900 + time->tm_year) - 1970) * 365) +
        /* Add days for leap years. */
        (((1900 + time->tm_year) - 1968) / 4)) -
        /* 2000 wasn't a leap year. */
        (( time->tm_year >= 100 ) ? 1 : 0)) +
        /* Add days for each month. */
        (mdays[time->tm_mon] + time->tm_mday)) *
        /* Convert all those days to seconds. */
        (24 * 60 * 60)) +
        /* Add the hours, minutes, and seconds. */
        ((time->tm_hour * 60 * 60) + (time->tm_min * 60) + time->tm_sec));
}
