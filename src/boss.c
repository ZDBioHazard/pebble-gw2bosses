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

/* This is at the bottom of the file. */
static struct boss bosses[BOSS_COUNT];

/*****************************************************************************/

/* Update the timer values in the boss list. */
void update_boss_times( const struct tm *time ){
    boss_t index = 0;
    struct tm event = *time;

    for ( index = 0 ; index <= BOSS_INDEX_MAX ; index++ ){
        event = *time;
        event.tm_hour = bosses[index].hour;
        event.tm_min  = bosses[index].min;
        event.tm_sec  = 0;

        /* Add a day to events that have already happened today. */
        if ( bad_mktime(&event) < bad_mktime(time) )
            /* It seems really bizarre, but changing the date like this
             * actually works with most mktime() implementations. o.O */
            event.tm_hour += 24;

        /* Yeah, this should probably use difftime(), but that includes
         * ~3K of extra library code in the binary, and this case isn't
         * likely to trigger any of difftime()'s edge cases anyway. */
        bosses[index].time = (signed int)(bad_mktime(&event) -
                                          bad_mktime(time));
    }
}

/*****************************************************************************/

/* Return the info struct for a boss. */
struct boss *get_boss_info( const bool active, const boss_t offset ){
    boss_t index = BOSS_INDEX_MAX;

    /* Start at the end of the list and count up. Break at
     * the first entry that's larger than the previous entry. */
    for ( index = BOSS_INDEX_MAX ; index > 0 ; index-- )
        if ( bosses[index].time < bosses[index - 1].time )
            break;

    /* Only one boss can be current, so just return it now. */
    if ( active == true )
        return &bosses[(index == 0) ? BOSS_INDEX_MAX : index - 1];

    /* Wrap the index if it goes over the end of the array. */
    if ( index + offset > BOSS_INDEX_MAX )
        return &bosses[(index + offset) - BOSS_COUNT];

    return &bosses[index + offset];
}

/*****************************************************************************/

/* Luckily, GCC will deduplicate all these strings, saving us memory. */
/* TODO It would be cool if this info could be generated from a JSON file
 * via PebbleKitJS - ideally from ArenaNet APIs, but then we wouldn't have
 * the aformentioned automatic GCC memory deduplication without making a
 * complicated set of lookup tales, not to mention designing a format where
 * the data could fit in limited persistent storage chunks would be tricky. */
/* XXX Don't forget to update BOSS_INDEX_MAX above! */
/* XXX Keeping this in ascending time order is IMPORTANT! */
static struct boss bosses[BOSS_COUNT] = {
    { 0,  0, "The Shatterer", "Blazeridge Steppes", 0},
    { 0, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    { 0, 30, "Modniir Ulgoth", "Hirathi Hinterlands", 0},
    { 0, 45, "Fire Elemental", "Metrica Province", 0},
    { 1,  0, "Karka Queen", "Southsun Cove", 0},
    { 1, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    { 1, 30, "Golem Mark II", "Mount Maelstrom", 0},
    { 1, 45, "Shadow Behemoth", "Queensdale", 0},
    { 2,  0, "Tequatl the Sunless", "Sparkfly Fen", 0},
    { 2, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    { 2, 30, "Claw of Jormag", "Frostgorge Sound", 0},
    { 2, 45, "Fire Elemental", "Metrica Province", 0},
    { 3,  0, "Triple Trouble", "Bloodtide Coast", 0},
    { 3, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    { 3, 30, "Taidha Covington", "Bloodtide Coast", 0},
    { 3, 45, "Shadow Behemoth", "Queensdale", 0},
    { 4,  0, "Megadestroyer", "Mount Maelstrom", 0},
    { 4, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    /* { 4, 30, "TBD", "Unknown", 0 }, */
    { 4, 45, "Fire Elemental", "Metrica Province", 0},
    { 5,  0, "The Shatterer", "Blazeridge Steppes", 0},
    { 5, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    { 5, 30, "Modniir Ulgoth", "Hirathi Hinterlands", 0},
    { 5, 45, "Shadow Behemoth", "Queensdale", 0},
    { 6,  0, "Golem Mark II", "Mount Maelstrom", 0},
    { 6, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    { 6, 30, "Claw of Jormag", "Frostgorge Sound", 0},
    { 6, 45, "Fire Elemental", "Metrica Province", 0},
    { 7,  0, "The Shatterer", "Blazeridge Steppes", 0},
    { 7, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    { 7, 30, "Modniir Ulgoth", "Hirathi Hinterlands", 0},
    { 7, 45, "Shadow Behemoth", "Queensdale", 0},
    { 8,  0, "Golem Mark II", "Mount Maelstrom", 0},
    { 8, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    { 8, 30, "Claw of Jormag", "Frostgorge Sound", 0},
    { 8, 45, "Fire Elemental", "Metrica Province", 0},
    { 9,  0, "Taidha Covington", "Bloodtide Coast", 0},
    { 9, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    { 9, 30, "Megadestroyer", "Mount Maelstrom", 0},
    { 9, 45, "Shadow Behemoth", "Queensdale", 0},
    /* {10,  0, "TBD", "Unknown", 0 }, */
    {10, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    {10, 30, "Karka Queen", "Southsun Cove", 0},
    {10, 45, "Fire Elemental", "Metrica Province", 0},
    {11,  0, "The Shatterer", "Blazeridge Steppes", 0},
    {11, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    {11, 30, "Tequatl the Sunless", "Sparkfly Fen", 0},
    {11, 45, "Shadow Behemoth", "Queensdale", 0},
    {12,  0, "Modniir Ulgoth", "Hirathi Hinterlands", 0},
    {12, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    {12, 30, "Triple Trouble", "Bloodtide Coast", 0},
    {12, 45, "Fire Elemental", "Metrica Province", 0},
    {13,  0, "Golem Mark II", "Mount Maelstrom", 0},
    {13, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    {13, 30, "Claw of Jormag", "Frostgorge Sound", 0},
    {13, 45, "Shadow Behemoth", "Queensdale", 0},
    {14,  0, "Taidha Covington", "Bloodtide Coast", 0},
    {14, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    {14, 30, "Megadestroyer", "Mount Maelstrom", 0},
    {14, 45, "Fire Elemental", "Metrica Province", 0},
    /* {15,  0, "TBD", "Unknown", 0 }, */
    {15, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    {15, 30, "The Shatterer", "Blazeridge Steppes", 0},
    {15, 45, "Shadow Behemoth", "Queensdale", 0},
    {16,  0, "Karka Queen", "Southsun Cove", 0},
    {16, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    {16, 30, "Modniir Ulgoth", "Hirathi Hinterlands", 0},
    {16, 45, "Fire Elemental", "Metrica Province", 0},
    {17,  0, "Tequatl the Sunless", "Sparkfly Fen", 0},
    {17, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    {17, 30, "Golem Mark II", "Mount Maelstrom", 0},
    {17, 45, "Shadow Behemoth", "Queensdale", 0},
    {18,  0, "Triple Trouble", "Bloodtide Coast", 0},
    {18, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    {18, 30, "Claw of Jormag", "Frostgorge Sound", 0},
    {18, 45, "Fire Elemental", "Metrica Province", 0},
    {19,  0, "Taidha Covington", "Bloodtide Coast", 0},
    {19, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    {19, 30, "Megadestroyer", "Mount Maelstrom", 0},
    {19, 45, "Shadow Behemoth", "Queensdale", 0},
    /* {20,  0, "TBD", "Unknown", 0 }, */
    {20, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    {20, 30, "The Shatterer", "Blazeridge Steppes", 0},
    {20, 45, "Fire Elemental", "Metrica Province", 0},
    {21,  0, "Modniir Ulgoth", "Hirathi Hinterlands", 0},
    {21, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    {21, 30, "Golem Mark II", "Mount Maelstrom", 0},
    {21, 45, "Shadow Behemoth", "Queensdale", 0},
    {22,  0, "Claw of Jormag", "Frostgorge Sound", 0},
    {22, 15, "Svanir Shaman", "Wayfarer Foothills", 0},
    {22, 30, "Taidha Covington", "Bloodtide Coast", 0},
    {22, 45, "Fire Elemental", "Metrica Province", 0},
    {23,  0, "Megadestroyer", "Mount Maelstrom", 0},
    {23, 15, "Great Jungle Wurm", "Caledon Forest", 0},
    /* {23, 30, "TBD", "Unknown", 0 }, */
    {23, 45, "Shadow Behemoth", "Queensdale", 0},
};
