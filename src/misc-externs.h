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
// form_recall.c
extern Short recall_this_monster;

// moria1.c - this could be a database record.. but it's only 60 bytes..
extern Short class_level_adj[MAX_CLASS][MAX_LEV_ADJ];

// spells-a.c - this could be a database record.. but it's only 160 bytes..
extern ULong player_exp[MAX_PLAYER_LEVEL];
extern class_type class[MAX_CLASS]; // ditto 186 bytes or so

// create.c - this is how big? like 240 bytes I think.
extern race_type race[MAX_RACES];
extern Short player_hp[MAX_PLAYER_LEVEL];// another 80 bytes

// magic.c
// these should probably be database records.  they are read only.
extern spell_type magic_spell[MAX_CLASS-1][31];
extern Char spell_names[62][24]; // could also be const.

// misc5.c
extern Short needed_direction_for;
extern Short needed_direction_item;

// store1.c
extern owner_type owners[MAX_OWNERS];
