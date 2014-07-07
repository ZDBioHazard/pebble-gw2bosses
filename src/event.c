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

#define EVENT_INDEX_MAX (event_t)95
#define EVENT_COUNT (EVENT_INDEX_MAX + 1)
#define EVENT_DATA_VERSION (int32_t)201406170
#define EVENT_DURATION (15 * 60) /* TODO Use more accurate per-event times. */

static const struct event event_info[EVENT_COUNT]; /* Defined below. */
static signed int event_times[EVENT_COUNT] = { 0 };
static bool event_reminders[EVENT_COUNT] = { false };

/*****************************************************************************/

/* Find and return the desired event's array index. */
static event_t get_event_index( const bool active, const event_t offset ){
    event_t index = EVENT_INDEX_MAX;

    /* Start at the end of the list and count up. Break at
     * the first entry that's larger than the previous entry. */
    for ( index = EVENT_INDEX_MAX ; index > 0 ; index-- )
        if ( event_times[index] < event_times[index - 1] )
            break;

    /* Count backwards for active events. */
    if ( active == true ){
        event_t count = get_event_count(active);
        if ( index - (count - offset) < 0 )
            return EVENT_COUNT - ((count - index) - offset);

        return index - (count - offset);
    }

    /* Wrap the index if it goes over the end of the array. */
    if ( index + offset > EVENT_INDEX_MAX )
        return (index + offset) - EVENT_COUNT;

    return index + offset;
}

/*****************************************************************************/

/* Return the number of events in the list. */
event_t get_event_count( const bool active ){
    event_t index = 0;
    event_t count = 0;

    /* Consider an event active if it's x-minutes less than 24-hours away. */
    for ( index = 0 ; index <= EVENT_INDEX_MAX ; index++ )
        if ( event_times[index] > (24 * 60 * 60) - EVENT_DURATION )
            count++;

    /* If the number of items is ever zero, the section will be deleted. */
    return ( active == true ) ? count : (EVENT_COUNT - count);
}

/* Return the info struct for a event. */
const struct event *get_event_info( const bool active, const event_t index ){
    return &event_info[get_event_index(active, index)];
}

/* Return the timer for a event. */
signed int get_event_timer( const event_t index ){
    return event_times[get_event_index(false, index)];
}

/* Return the reminder status. */
bool get_event_reminder( const bool active, const event_t index ){
    return event_reminders[get_event_index(active, index)];
}

/*****************************************************************************/

/* Save reminders to persistent storage. */
void save_event_reminders( void ){
    if ( persist_write_int(PERSIST_KEY_DATA_VERSION, EVENT_DATA_VERSION) < S_SUCCESS ||
         persist_write_data(PERSIST_KEY_REMINDERS, event_reminders,
                            sizeof(event_reminders)) != sizeof(event_reminders) )
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error writing reminders to storage.");
    else
        APP_LOG(APP_LOG_LEVEL_INFO, "Saved reminders to storage.");
}

/* Load reminders from persistent storage. */
void load_event_reminders( void ){
    /* Do a bunch of sanity checks while we load the data. */
    if ( persist_exists(PERSIST_KEY_DATA_VERSION) == false ||
         persist_exists(PERSIST_KEY_REMINDERS) == false ||
         persist_get_size(PERSIST_KEY_DATA_VERSION) != sizeof(int32_t) ||
         persist_get_size(PERSIST_KEY_REMINDERS) != sizeof(event_reminders) ||
         persist_read_int(PERSIST_KEY_DATA_VERSION) != EVENT_DATA_VERSION ){
        APP_LOG(APP_LOG_LEVEL_ERROR, "Reminder format mismatch; Discarding.");
        return;
    }

    /* Make sure that all the reminder data is read. */
    if ( persist_read_data(PERSIST_KEY_REMINDERS, event_reminders,
                           sizeof(event_reminders)) != sizeof(event_reminders) ){
        APP_LOG(APP_LOG_LEVEL_ERROR, "Reminder list only partially read.");
        memset(event_reminders, false, sizeof(event_reminders));
        return;
    }

    APP_LOG(APP_LOG_LEVEL_INFO, "Loaded reminders from storage.");
}

/* Toggle the reminder state of a event. */
void toggle_event_reminder( const bool active, const event_t index ){
    event_t event = get_event_index(active, index);
    event_reminders[event] = !event_reminders[event];
}

/*****************************************************************************/

/* Update the timer values in the event list. */
void update_event_times( const struct tm *time ){
    event_t index = 0;
    struct tm event = *time;

    for ( index = 0 ; index <= EVENT_INDEX_MAX ; index++ ){
        event.tm_hour = event_info[index].hour;
        event.tm_min  = event_info[index].min;
        event.tm_sec  = 0;

        /* Add a day to events that have already happened today. */
        if ( bad_difftime(&event, time) <= 0 )
            /* It seems really bizarre, but changing the date like this
             * actually works with most mktime() implementations. o.O */
            event.tm_hour += 24;

        /* Yeah, this should probably use difftime(), but that includes
         * ~3K of extra library code in the binary, and this case isn't
         * likely to trigger any of difftime()'s edge cases anyway. */
        event_times[index] = bad_difftime(&event, time);

        /* Alert for reminders at 10:00, 5:00, and 0:01 before event start. */
        /* FIXME If the device skips a second and misses one of these
         * times, I'm not sure how to tell, or what to do about it. */
        if ( event_reminders[index] == true ){
            /* Do a single pulse for upcoming event alerts. */
            if ( event_times[index] == 600 || event_times[index] == 300 )
                vibes_short_pulse();
            /* Do a double pulse for events that are starting right now. */
            else if ( event_times[index] == 1 )
                vibes_double_pulse();
        }
    }
}

/*****************************************************************************/

/* Luckily, GCC will de-duplicate all these strings, saving us memory. */
/* TODO It would be cool if this info could be generated from a JSON file
 * via PebbleKitJS - ideally from ArenaNet APIs, but then we wouldn't have
 * the aforementioned automatic GCC memory de-duplication without making a
 * complicated set of lookup tales, not to mention designing a format where
 * the data could fit in limited persistent storage chunks would be tricky. */
/* XXX Don't forget to update EVENT_INDEX_MAX above! */
/* XXX Keeping this in ascending time order is IMPORTANT! */
static const struct event event_info[EVENT_COUNT] = {
    { 0,  0, "Taidha Covington", "Bloodtide Coast"},
    { 0, 15, "Svanir Shaman", "Wayfarer Foothills"},
    { 0, 30, "Megadestroyer", "Mount Maelstrom"},
    { 0, 45, "Fire Elemental", "Metrica Province"},
    { 1,  0, "The Shatterer", "Blazeridge Steppes"},
    { 1, 15, "Great Jungle Wurm", "Caeldon Forest"},
    { 1, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    { 1, 45, "Shadow Behemoth", "Queensdale"},
    { 2,  0, "Golem Mark II", "Mount Maelstrom"},
    { 2, 15, "Svanir Shaman", "Wayfarer Foothills"},
    { 2, 30, "Claw of Jormag", "Frostgorge Sound"},
    { 2, 45, "Fire Elemental", "Metrica Province"},
    { 3,  0, "Taidha Covington", "Bloodtide Coast"},
    { 3, 15, "Great Jungle Wurm", "Caeldon Forest"},
    { 3, 30, "Megadestroyer", "Mount Maelstrom"},
    { 3, 45, "Shadow Behemoth", "Queensdale"},
    { 4,  0, "The Shatterer", "Blazeridge Steppes"},
    { 4, 15, "Svanir Shaman", "Wayfarer Foothills"},
    { 4, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    { 4, 45, "Fire Elemental", "Metrica Province"},
    { 5,  0, "Golem Mark II", "Mount Maelstrom"},
    { 5, 15, "Great Jungle Wurm", "Caeldon Forest"},
    { 5, 30, "Claw of Jormag", "Frostgorge Sound"},
    { 5, 45, "Shadow Behemoth", "Queensdale"},
    { 6,  0, "Taidha Covington", "Bloodtide Coast"},
    { 6, 15, "Svanir Shaman", "Wayfarer Foothills"},
    { 6, 30, "Megadestroyer", "Mount Maelstrom"},
    { 6, 45, "Fire Elemental", "Metrica Province"},
    { 7,  0, "The Shatterer", "Blazeridge Steppes"},
    { 7, 15, "Great Jungle Wurm", "Caeldon Forest"},
    { 7, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    { 7, 45, "Shadow Behemoth", "Queensdale"},
    { 8,  0, "Golem Mark II", "Mount Maelstrom"},
    { 8, 15, "Svanir Shaman", "Wayfarer Foothills"},
    { 8, 30, "Claw of Jormag", "Frostgorge Sound"},
    { 8, 45, "Fire Elemental", "Metrica Province"},
    { 9,  0, "Taidha Covington", "Bloodtide Coast"},
    { 9, 15, "Great Jungle Wurm", "Caeldon Forest"},
    { 9, 30, "Megadestroyer", "Mount Maelstrom"},
    { 9, 45, "Shadow Behemoth", "Queensdale"},
    {10,  0, "The Shatterer", "Blazeridge Steppes"},
    {10, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {10, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    {10, 45, "Fire Elemental", "Metrica Province"},
    {11,  0, "Golem Mark II", "Mount Maelstrom"},
    {11, 15, "Great Jungle Wurm", "Caeldon Forest"},
    {11, 30, "Claw of Jormag", "Frostgorge Sound"},
    {11, 45, "Shadow Behemoth", "Queensdale"},
    {12,  0, "Taidha Covington", "Bloodtide Coast"},
    {12, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {12, 30, "Megadestroyer", "Mount Maelstrom"},
    {12, 45, "Fire Elemental", "Metrica Province"},
    {13,  0, "The Shatterer", "Blazeridge Steppes"},
    {13, 15, "Great Jungle Wurm", "Caeldon Forest"},
    {13, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    {13, 45, "Shadow Behemoth", "Queensdale"},
    {14,  0, "Golem Mark II", "Mount Maelstrom"},
    {14, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {14, 30, "Claw of Jormag", "Frostgorge Sound"},
    {14, 45, "Fire Elemental", "Metrica Province"},
    {15,  0, "Taidha Covington", "Bloodtide Coast"},
    {15, 15, "Great Jungle Wurm", "Caeldon Forest"},
    {15, 30, "Megadestroyer", "Mount Maelstrom"},
    {15, 45, "Shadow Behemoth", "Queensdale"},
    {16,  0, "The Shatterer", "Blazeridge Steppes"},
    {16, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {16, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    {16, 45, "Fire Elemental", "Metrica Province"},
    {17,  0, "Golem Mark II", "Mount Maelstrom"},
    {17, 15, "Great Jungle Wurm", "Caeldon Forest"},
    {17, 30, "Claw of Jormag", "Frostgorge Sound"},
    {17, 45, "Shadow Behemoth", "Queensdale"},
    {18,  0, "Taidha Covington", "Bloodtide Coast"},
    {18, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {18, 30, "Megadestroyer", "Mount Maelstrom"},
    {18, 45, "Fire Elemental", "Metrica Province"},
    {19,  0, "The Shatterer", "Blazeridge Steppes"},
    {19, 15, "Great Jungle Wurm", "Caeldon Forest"},
    {19, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    {19, 45, "Shadow Behemoth", "Queensdale"},
    {20,  0, "Golem Mark II", "Mount Maelstrom"},
    {20, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {20, 30, "Claw of Jormag", "Frostgorge Sound"},
    {20, 45, "Fire Elemental", "Metrica Province"},
    {21,  0, "Taidha Covington", "Bloodtide Coast"},
    {21, 15, "Great Jungle Wurm", "Caeldon Forest"},
    {21, 30, "Megadestroyer", "Mount Maelstrom"},
    {21, 45, "Shadow Behemoth", "Queensdale"},
    {22,  0, "The Shatterer", "Blazeridge Steppes"},
    {22, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {22, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    {22, 45, "Fire Elemental", "Metrica Province"},
    {23,  0, "Golem Mark II", "Mount Maelstrom"},
    {23, 15, "Great Jungle Wurm", "Caeldon Forest"},
    {23, 30, "Claw of Jormag", "Frostgorge Sound"},
    {23, 45, "Shadow Behemoth", "Queensdale"},
};
