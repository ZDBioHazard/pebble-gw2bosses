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

#define BOSS_INDEX_MAX (boss_t)90
#define BOSS_COUNT (BOSS_INDEX_MAX + 1)

static const struct boss boss_info[BOSS_COUNT]; /* Defined below. */
static signed int boss_times[BOSS_COUNT] = { 0 };
static bool boss_reminders[BOSS_COUNT] = { false };

/*****************************************************************************/

/* Find and return the desired boss' array index. */
static boss_t get_boss_index( const bool active, const boss_t offset ){
    boss_t index = BOSS_INDEX_MAX;

    /* Start at the end of the list and count up. Break at
     * the first entry that's larger than the previous entry. */
    for ( index = BOSS_INDEX_MAX ; index > 0 ; index-- )
        if ( boss_times[index] < boss_times[index - 1] )
            break;

    /* Only one boss can be current, so just return it now. */
    /* TODO More than one boss can be active at a time. */
    if ( active == true )
        return (index == 0) ? BOSS_INDEX_MAX : index - 1;

    /* Wrap the index if it goes over the end of the array. */
    if ( index + offset > BOSS_INDEX_MAX )
        return (index + offset) - BOSS_COUNT;

    return index + offset;
}

/*****************************************************************************/

/* Return the info struct for a boss. */
const struct boss *get_boss_info( const bool active, const boss_t index ){
    return &boss_info[get_boss_index(active, index)];
}

/* Return the timer for a boss. */
signed int get_boss_timer( const boss_t index ){
    return boss_times[get_boss_index(false, index)];
}

/* Return the reminder status. */
bool get_boss_reminder( const boss_t index ){
    return boss_reminders[get_boss_index(false, index)];
}

/*****************************************************************************/

/* Save reminders to persistent storage. */
void save_boss_reminders( void ){
    if ( persist_write_data(PERSIST_KEY_REMINDERS, boss_reminders,
                            sizeof(boss_reminders)) != sizeof(boss_reminders) )
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error writing reminders to storage.");
    else
        APP_LOG(APP_LOG_LEVEL_INFO, "Saved reminders to storage.");
}

/* Load reminders from persistent storage. */
void load_boss_reminders( void ){
    if ( persist_exists(PERSIST_KEY_REMINDERS) == false )
        return;

    if ( persist_read_data(PERSIST_KEY_REMINDERS, boss_reminders,
                           sizeof(boss_reminders)) != sizeof(boss_reminders) )
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error reading reminders from storage.");
    else
        APP_LOG(APP_LOG_LEVEL_INFO, "Loaded reminders from storage.");
}

/* Toggle the reminder state of a boss. */
void toggle_boss_reminder( const boss_t index ){
    boss_t boss = get_boss_index(false, index);
    boss_reminders[boss] = !boss_reminders[boss];
}

/*****************************************************************************/

/* Update the timer values in the boss list. */
void update_boss_times( const struct tm *time ){
    boss_t index = 0;
    struct tm event = *time;

    for ( index = 0 ; index <= BOSS_INDEX_MAX ; index++ ){
        event.tm_hour = boss_info[index].hour;
        event.tm_min  = boss_info[index].min;
        event.tm_sec  = 0;

        /* Add a day to events that have already happened today. */
        if ( bad_mktime(&event) <= bad_mktime(time) )
            /* It seems really bizarre, but changing the date like this
             * actually works with most mktime() implementations. o.O */
            event.tm_hour += 24;

        /* Yeah, this should probably use difftime(), but that includes
         * ~3K of extra library code in the binary, and this case isn't
         * likely to trigger any of difftime()'s edge cases anyway. */
        boss_times[index] = (bad_mktime(&event) - bad_mktime(time));

        /* Alert for reminders at 10:00, 5:00, and 0:01 before event start. */
        /* FIXME If the device skips a second and misses one of these
         * times, I'm not sure how to tell, or what to do about it. */
        if ( boss_reminders[index] == true ){
            /* Do a single pulse for upcoming event alerts. */
            if ( boss_times[index] == 600 || boss_times[index] == 300 )
                vibes_short_pulse();
            /* Do a double pulse for events that are starting right now. */
            else if ( boss_times[index] == 1 )
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
/* XXX Don't forget to update BOSS_INDEX_MAX above! */
/* XXX Keeping this in ascending time order is IMPORTANT! */
static const struct boss boss_info[BOSS_COUNT] = {
    { 0,  0, "The Shatterer", "Blazeridge Steppes"},
    { 0, 15, "Svanir Shaman", "Wayfarer Foothills"},
    { 0, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    { 0, 45, "Fire Elemental", "Metrica Province"},
    { 1,  0, "Karka Queen", "Southsun Cove"},
    { 1, 15, "Great Jungle Wurm", "Caledon Forest"},
    { 1, 30, "Golem Mark II", "Mount Maelstrom"},
    { 1, 45, "Shadow Behemoth", "Queensdale"},
    { 2,  0, "Tequatl the Sunless", "Sparkfly Fen"},
    { 2, 15, "Svanir Shaman", "Wayfarer Foothills"},
    { 2, 30, "Claw of Jormag", "Frostgorge Sound"},
    { 2, 45, "Fire Elemental", "Metrica Province"},
    { 3,  0, "Triple Trouble", "Bloodtide Coast"},
    { 3, 15, "Great Jungle Wurm", "Caledon Forest"},
    { 3, 30, "Taidha Covington", "Bloodtide Coast"},
    { 3, 45, "Shadow Behemoth", "Queensdale"},
    { 4,  0, "Megadestroyer", "Mount Maelstrom"},
    { 4, 15, "Svanir Shaman", "Wayfarer Foothills"},
    /* { 4, 30, "TBD", "Unknown"}, */
    { 4, 45, "Fire Elemental", "Metrica Province"},
    { 5,  0, "The Shatterer", "Blazeridge Steppes"},
    { 5, 15, "Great Jungle Wurm", "Caledon Forest"},
    { 5, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    { 5, 45, "Shadow Behemoth", "Queensdale"},
    { 6,  0, "Golem Mark II", "Mount Maelstrom"},
    { 6, 15, "Svanir Shaman", "Wayfarer Foothills"},
    { 6, 30, "Claw of Jormag", "Frostgorge Sound"},
    { 6, 45, "Fire Elemental", "Metrica Province"},
    { 7,  0, "The Shatterer", "Blazeridge Steppes"},
    { 7, 15, "Great Jungle Wurm", "Caledon Forest"},
    { 7, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    { 7, 45, "Shadow Behemoth", "Queensdale"},
    { 8,  0, "Golem Mark II", "Mount Maelstrom"},
    { 8, 15, "Svanir Shaman", "Wayfarer Foothills"},
    { 8, 30, "Claw of Jormag", "Frostgorge Sound"},
    { 8, 45, "Fire Elemental", "Metrica Province"},
    { 9,  0, "Taidha Covington", "Bloodtide Coast"},
    { 9, 15, "Great Jungle Wurm", "Caledon Forest"},
    { 9, 30, "Megadestroyer", "Mount Maelstrom"},
    { 9, 45, "Shadow Behemoth", "Queensdale"},
    /* {10,  0, "TBD", "Unknown"}, */
    {10, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {10, 30, "Karka Queen", "Southsun Cove"},
    {10, 45, "Fire Elemental", "Metrica Province"},
    {11,  0, "The Shatterer", "Blazeridge Steppes"},
    {11, 15, "Great Jungle Wurm", "Caledon Forest"},
    {11, 30, "Tequatl the Sunless", "Sparkfly Fen"},
    {11, 45, "Shadow Behemoth", "Queensdale"},
    {12,  0, "Modniir Ulgoth", "Hirathi Hinterlands"},
    {12, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {12, 30, "Triple Trouble", "Bloodtide Coast"},
    {12, 45, "Fire Elemental", "Metrica Province"},
    {13,  0, "Golem Mark II", "Mount Maelstrom"},
    {13, 15, "Great Jungle Wurm", "Caledon Forest"},
    {13, 30, "Claw of Jormag", "Frostgorge Sound"},
    {13, 45, "Shadow Behemoth", "Queensdale"},
    {14,  0, "Taidha Covington", "Bloodtide Coast"},
    {14, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {14, 30, "Megadestroyer", "Mount Maelstrom"},
    {14, 45, "Fire Elemental", "Metrica Province"},
    /* {15,  0, "TBD", "Unknown"}, */
    {15, 15, "Great Jungle Wurm", "Caledon Forest"},
    {15, 30, "The Shatterer", "Blazeridge Steppes"},
    {15, 45, "Shadow Behemoth", "Queensdale"},
    {16,  0, "Karka Queen", "Southsun Cove"},
    {16, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {16, 30, "Modniir Ulgoth", "Hirathi Hinterlands"},
    {16, 45, "Fire Elemental", "Metrica Province"},
    {17,  0, "Tequatl the Sunless", "Sparkfly Fen"},
    {17, 15, "Great Jungle Wurm", "Caledon Forest"},
    {17, 30, "Golem Mark II", "Mount Maelstrom"},
    {17, 45, "Shadow Behemoth", "Queensdale"},
    {18,  0, "Triple Trouble", "Bloodtide Coast"},
    {18, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {18, 30, "Claw of Jormag", "Frostgorge Sound"},
    {18, 45, "Fire Elemental", "Metrica Province"},
    {19,  0, "Taidha Covington", "Bloodtide Coast"},
    {19, 15, "Great Jungle Wurm", "Caledon Forest"},
    {19, 30, "Megadestroyer", "Mount Maelstrom"},
    {19, 45, "Shadow Behemoth", "Queensdale"},
    /* {20,  0, "TBD", "Unknown"}, */
    {20, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {20, 30, "The Shatterer", "Blazeridge Steppes"},
    {20, 45, "Fire Elemental", "Metrica Province"},
    {21,  0, "Modniir Ulgoth", "Hirathi Hinterlands"},
    {21, 15, "Great Jungle Wurm", "Caledon Forest"},
    {21, 30, "Golem Mark II", "Mount Maelstrom"},
    {21, 45, "Shadow Behemoth", "Queensdale"},
    {22,  0, "Claw of Jormag", "Frostgorge Sound"},
    {22, 15, "Svanir Shaman", "Wayfarer Foothills"},
    {22, 30, "Taidha Covington", "Bloodtide Coast"},
    {22, 45, "Fire Elemental", "Metrica Province"},
    {23,  0, "Megadestroyer", "Mount Maelstrom"},
    {23, 15, "Great Jungle Wurm", "Caledon Forest"},
    /* {23, 30, "TBD", "Unknown"}, */
    {23, 45, "Shadow Behemoth", "Queensdale"},
};
