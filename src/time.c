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

/* I assume there will never be a time zone with a 42-day offset. ;) */
#define BAD_TZ_OFFSET (int32_t)0xBEEFCAFE

/* Don't read tz_offset directly, use get_tz_offset(), as that will
 * attempt to initialize tz_offset it the first time it's called. */
static int32_t tz_offset = BAD_TZ_OFFSET;

/*****************************************************************************/

/* Pebble doesn't have a working mktime(), so I wrote my own.
 * Note: This only works properly for the years 1901 to 2099. */
static time_t bad_mktime( const struct tm *time ){
    uint16_t mdays[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

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

/* Return the time difference between two tm structs. */
time_t bad_difftime( const struct tm *time1, const struct tm *time2 ){
    return bad_mktime(time1) - bad_mktime(time2);
}

/*****************************************************************************/

/* Return the time zone offset. */
static int32_t get_tz_offset( void ){
    /* Load the offset from storage if it isn't set yet. */
    if ( tz_offset == BAD_TZ_OFFSET &&
         persist_exists(PERSIST_KEY_TZ_OFFSET) == true &&
         persist_get_size(PERSIST_KEY_TZ_OFFSET) == sizeof(tz_offset) ){
        tz_offset = persist_read_int(PERSIST_KEY_TZ_OFFSET);
        APP_LOG(APP_LOG_LEVEL_INFO, "Got offset %"PRId32" from storage.", tz_offset);
    }

    return tz_offset;
}

/* Set the time zone offset. */
void set_tz_offset( const int32_t offset ){
    /* Just stop if nothing changed. */
    if ( get_tz_offset() == offset )
        return;

    tz_offset = offset;

    /* Write the offset to storage if it's different than what we have. */
    APP_LOG(APP_LOG_LEVEL_INFO, "Writing offset %"PRId32" to storage.", tz_offset);
    persist_write_int(PERSIST_KEY_TZ_OFFSET, tz_offset);
}

/* Returns true if get_tz_offset() returns a valid value. */
bool have_tz_offset( void ){
    return ( get_tz_offset() == BAD_TZ_OFFSET ) ? false : true;
}

/*****************************************************************************/

/* Convert a tm struct from local time to UTC time, using the stored offset. */
bool time_convert_local_to_utc( struct tm *time ){
    if ( have_tz_offset() == false )
        return false;

    time->tm_min += get_tz_offset();
    time_t utc_ts = bad_mktime(time);
    memcpy(time, gmtime(&utc_ts), sizeof(struct tm));
    return true;
}

/* Convert a tm struct from UTC time to local time, using the stored offset. */
bool time_convert_utc_to_local( struct tm *time ){
    if ( have_tz_offset() == false )
        return false;

    time->tm_min -= get_tz_offset();
    time_t local_ts = bad_mktime(time);
    memcpy(time, localtime(&local_ts), sizeof(struct tm));
    return true;
}
