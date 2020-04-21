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
/* These are the ones that the GAME will use and not the database-builder */
/* Keep this up to date with respect to dbrecords.h */

// THis might also want to contain all the MAXLEN_foo !!!

void newrec_messagelog(); /* use if no SaveDB exists */
void add_message_to_log(Char *msg, everything *env);
void print_message_log(everything *env);
void copy_player_title(Short pclass, Short plevel, Char *buf);
void generate_background(Char *buf, everything *env);
ULong get_player_exp(Short plevel);
void newrec_playerhp(); /* use if no SaveDB exists */
void write_player_hp(Short plevel, UShort hp);
void reset_player_hp(); /* use on character creation if SaveDB exists */
UShort get_player_hp(Short plevel);
UChar rgold_adj(Short owner_race, Short player_race);
void copy_spell_name(Short spell, Char *buf);
void newrec_spellorder(); /* use if no SaveDB exists */
void reset_spell_order(); /* use on character creation if SaveDB exists */
void write_spell_order(Short new_spell, everything *env);
Short get_spell_order(Short i);
/* NOTE - Need a routine to get the store owner record */
/* NOTE - Need a routine to get the store type record */
/* NOTE - Need a routine to get the store inventory records */
/* NOTE - Need a routine to get the treasure (object) type records */
void copy_spec_name(Short spec, Char *buf);
/* NOTE - Need a routine to get the creature (monster) type records */
/* NOTE - Need a routine to get the monster attacks record */
void copy_color(Short i, Char *buf);
void copy_mushroom(Short i, Char *buf);
void copy_wood(Short i, Char *buf);
void copy_metal(Short i, Char *buf);
void copy_rock(Short i, Char *buf);
void copy_amulet(Short i, Char *buf);
void copy_syllable(Short i, Char *buf);
UShort get_normal_tab(Short i);
