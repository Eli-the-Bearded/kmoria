/*********************************************************************
 * kMoria - Moria adapted for the PalmPilot.                         *
 * Copyright (C) 2000 Bridget Spitznagel                             *
 *                                                                   *
 * This program is derived from Umoria 5.5.2.  Moria is              *
 *   Copyright (c) 1989-94 James E. Wilson, Robert A. Koeneke        *
 *                                                                   *
 * This program is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU General Public License       *
 * as published by the Free Software Foundation; either version 2    *
 * of the License, or (at your option) any later version.            *
 *                                                                   *
 * This program is distributed in the hope that it will be useful,   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License for more details.                      *
 *                                                                   *
 * You should have received a copy of the GNU General Public License *
 * along with this program; if not, write to                         *
 * The Free Software Foundation, Inc.,                               *
 * 59 Temple Place - Suite 330,                                      *
 * Boston, MA  02111-1307, USA.                                      *
 *********************************************************************/
// globals, sigh
extern Long turn;	/* Cur trun of game    */
extern Short cur_height, cur_width;	/* Cur dungeon size    */
extern Short max_panel_rows, max_panel_cols;
extern Short panel_row, panel_col;
extern Short panel_row_min, panel_row_max;
extern Short panel_col_min, panel_col_max;
extern Short char_row;
extern Short char_col;
extern Short dun_level;

extern ULong town_seed; // to make the town not really random!

extern Short inven_weight;
extern Boolean weapon_heavy;
extern Short pack_heavy;
extern Boolean free_turn_flag, new_level_flag;
extern Short command_count;
extern Boolean light_flag, find_flag, find_prself, player_light, death;
extern Boolean find_bound; // "Running: stop when map sector changes"
extern Boolean total_winner, find_ignore_doors, find_examine, find_cut;
extern Boolean teleport_flag, prompt_carry_flag, highlight_seams;
extern Char died_from[25];
extern Short mon_tot_mult, hack_monptr;
extern ULong spell_learned, spell_worked; // bit field for spells learned

extern Short tcptr; // index of the next free slot in t_list
extern Short mfptr; // index of the next free slot in m_list
extern Short inven_ctr; // index of the next free slot in pack (inventory[])
extern Short equip_ctr; // how many things are equipped (also inventory[])


//extern player_type py; // I would be happier to make this a record
extern struct flags pyflags;
extern struct stats pystats;
extern struct misc pymisc;


// these live in lock.c
//extern UChar *cave_rec[4];
extern UChar *cave_cptr;
extern UChar *cave_tptr;
extern UChar *cave_fval;
extern UChar *cave_light;
extern inven_type *t_list;
extern treasure_type *object_list;
extern Char *object_names;
extern creature_type *c_list;
extern Char *c_names;
extern monster_type *m_list;
extern Short *t_level;
extern Short *sorted_objects;
extern Short *m_level;
extern inven_type *inventory;
extern UChar *object_ident;
extern recall_type *c_recall;
extern store_type *store;
extern inven_record *store_inven[MAX_STORES];

#include "misc-externs.h"

extern Char **amulets;
extern Char **rocks;
extern Char **woods;
extern Char **metals;
extern Char **titles;
extern Char **colors;
extern Char **mushrooms;
extern Char **special_names;
