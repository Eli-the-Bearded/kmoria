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
#include <Pilot.h>
#include "kMoriaRsc.h"
#include "kMoria.h"
#include "lock-externs.h"

// spell_type: slevel, smana, sfail, sexp.
spell_type magic_spell[MAX_CLASS-1][MAX_SPELLS] = {
  {		  /* Mage	   */
     {	1,  1, 22,   1},
     {	1,  1, 23,   1},
     {	1,  2, 24,   1},
     {	1,  2, 26,   1},
     {	3,  3, 25,   2},
     {	3,  3, 25,   1},
     {	3,  3, 27,   2},
     {	3,  4, 30,   1},
     {	5,  4, 30,   6},
     {	5,  5, 30,   8},
     {	5,  5, 30,   5},
     {	5,  5, 35,   6},
     {	7,  6, 35,   9},
     {	7,  6, 50,  10},
     {	7,  6, 40,  12},
     {	9,  7, 44,  19},
     {	9,  7, 45,  19},
     {	9,  7, 75,  22},
     {	9,  7, 45,  19},
     { 11,  7, 45,  25},
     { 11,  7, 99,  19},
     { 13,  7, 50,  22},
     { 15,  9, 50,  25},
     { 17,  9, 50,  31},
     { 19, 12, 55,  38},
     { 21, 12, 90,  44},
     { 23, 12, 60,  50},
     { 25, 12, 65,  63},
     { 29, 18, 65,  88},
     { 33, 21, 80, 125},
     { 37, 25, 95, 200}
   },
   {		  /* Priest	   */
     {	1,  1, 10,   1},
     {	1,  2, 15,   1},
     {	1,  2, 20,   1},
     {	1,  2, 25,   1},
     {	3,  2, 25,   1},
     {	3,  3, 27,   2},
     {	3,  3, 27,   2},
     {	3,  3, 28,   3},
     {	5,  4, 29,   4},
     {	5,  4, 30,   5},
     {	5,  4, 32,   5},
     {	5,  5, 34,   5},
     {	7,  5, 36,   6},
     {	7,  5, 38,   7},
     {	7,  6, 38,   9},
     {	7,  7, 38,   9},
     {	9,  6, 38,  10},
     {	9,  7, 38,  10},
     {	9,  7, 40,  10},
     { 11,  8, 42,  10},
     { 11,  8, 42,  12},
     { 11,  9, 55,  15},
     { 13, 10, 45,  15},
     { 13, 11, 45,  16},
     { 15, 12, 50,  20},
     { 15, 14, 50,  22},
     { 17, 14, 55,  32},
     { 21, 16, 60,  38},
     { 25, 20, 70,  75},
     { 33, 24, 90, 125},
     { 39, 32, 80, 200}
   },
   {		  /* Rogue	   */
     { 99, 99,	0,   0},
     {	5,  1, 50,   1},
     {	7,  2, 55,   1},
     {	9,  3, 60,   2},
     { 11,  4, 65,   2},
     { 13,  5, 70,   3},
     { 99, 99,	0,   0},
     { 15,  6, 75,   3},
     { 99, 99,	0,   0},
     { 17,  7, 80,   4},
     { 19,  8, 85,   5},
     { 21,  9, 90,   6},
     { 99, 99,	0,   0},
     { 23, 10, 95,   7},
     { 99, 99,	0,   0},
     { 99, 99,	0,   0},
     { 25, 12, 95,   9},
     { 27, 15, 99,  11},
     { 99, 99,	0,   0},
     { 99, 99,	0,   0},
     { 29, 18, 99,  19},
     { 99, 99,	0,   0},
     { 99, 99,	0,   0},
     { 99, 99,	0,   0},
     { 99, 99,	0,   0},
     { 99, 99,	0,   0},
     { 99, 99,	0,   0},
     { 99, 99,	0,   0},
     { 99, 99,	0,   0},
     { 99, 99,	0,   0},
     { 99, 99,	0,   0},
   },
   {		   /* Ranger	    */
     {	3,  1, 30,   1},
     {	3,  2, 35,   2},
     {	3,  2, 35,   2},
     {	5,  3, 35,   2},
     {	5,  3, 40,   2},
     {	5,  4, 45,   3},
     {	7,  5, 40,   6},
     {	7,  6, 40,   5},
     {	9,  7, 40,   7},
     {	9,  8, 45,   8},
     { 11,  8, 40,  10},
     { 11,  9, 45,  10},
     { 13, 10, 45,  12},
     { 13, 11, 55,  13},
     { 15, 12, 50,  15},
     { 15, 13, 50,  15},
     { 17, 17, 55,  15},
     { 17, 17, 90,  17},
     { 21, 17, 55,  17},
     { 21, 19, 60,  18},
     { 23, 25, 95,  20},
     { 23, 20, 60,  20},
     { 25, 20, 60,  20},
     { 25, 21, 65,  20},
     { 27, 21, 65,  22},
     { 29, 23, 95,  23},
     { 31, 25, 70,  25},
     { 33, 25, 75,  38},
     { 35, 25, 80,  50},
     { 37, 30, 95, 100},
     { 99, 99,	0,   0}
   },
   {		  /* Paladin	   */
     {	1,  1, 30,   1},
     {	2,  2, 35,   2},
     {	3,  3, 35,   3},
     {	5,  3, 35,   5},
     {	5,  4, 35,   5},
     {	7,  5, 40,   6},
     {	7,  5, 40,   6},
     {	9,  7, 40,   7},
     {	9,  7, 40,   8},
     {	9,  8, 40,   8},
     { 11,  9, 40,  10},
     { 11, 10, 45,  10},
     { 11, 10, 45,  10},
     { 13, 10, 45,  12},
     { 13, 11, 45,  13},
     { 15, 13, 45,  15},
     { 15, 15, 50,  15},
     { 17, 15, 50,  17},
     { 17, 15, 50,  18},
     { 19, 15, 50,  19},
     { 19, 15, 50,  19},
     { 21, 17, 50,  20},
     { 23, 17, 50,  20},
     { 25, 20, 50,  20},
     { 27, 21, 50,  22},
     { 29, 22, 50,  24},
     { 31, 24, 60,  25},
     { 33, 28, 60,  31},
     { 35, 32, 70,  38},
     { 37, 36, 90,  50},
     { 39, 38, 90, 100}
   }
 };

Char spell_names[62][24] = {
  /* Mage Spells */
  "Magic Missile",  "Detect Monsters",	"Phase Door",  "Light Area",
  "Cure Light Wounds",	"Find Hidden Traps/Doors",  "Stinking Cloud",
  "Confusion",	"Lightning Bolt",  "Trap/Door Destruction", "Sleep I",
  "Cure Poison",  "Teleport Self",  "Remove Curse",  "Frost Bolt",
  "Turn Stone to Mud",	"Create Food",	"Recharge Item I",  "Sleep II",
  "Polymorph Other",  "Identify",  "Sleep III",	 "Fire Bolt",  "Slow Monster",
  "Frost Ball",	 "Recharge Item II", "Teleport Other",	"Haste Self",
  "Fire Ball", "Word of Destruction", "Genocide",
  /* Priest Spells, start at index 31 */
  "Detect Evil",  "Cure Light Wounds",	"Bless",  "Remove Fear", "Call Light",
  "Find Traps",	 "Detect Doors/Stairs",	 "Slow Poison",	 "Blind Creature",
  "Portal",  "Cure Medium Wounds",  "Chant",  "Sanctuary",  "Create Food",
  "Remove Curse",  "Resist Heat and Cold",  "Neutralize Poison",
  "Orb of Draining",  "Cure Serious Wounds",  "Sense Invisible",
  "Protection from Evil",  "Earthquake",  "Sense Surroundings",
  "Cure Critical Wounds",  "Turn Undead",  "Prayer",  "Dispel Undead",
  "Heal",  "Dispel Evil",  "Glyph of Warding",	"Holy Word"
};

/* Throw a magic spell					-RAK-	*/
extern Boolean just_browsing_book; 

void cast()
{
  //  Short i, j, dir;
  //  Short choice, chance, result;
  //  inven_type *i_ptr;
  //  spell_type *m_ptr;

  free_turn_flag = true;
  if (pyflags.blind > 0)
    message("You can't see to read your spell book!");
  else if (no_light())
    message("You have no light to read by.");
  else if (pyflags.confused > 0)
    message("You are too confused.");
  else if (class[pymisc.pclass].spell != MAGE)
    message("You can't cast spells!");
  //  else if (!find_range(TV_MAGIC_BOOK, TV_NEVER, &i, &j))
  //    message("But you are not carrying any spell-books!");
  //  else if (get_item(&item_val, "Use which spell-book?", i, j, CNIL, CNIL)) {
  else {
    Short i, ctr = 0;
    for (i = 0 ; i < inven_ctr ; i++)
      if (inventory[i].tval == TV_MAGIC_BOOK)
	ctr++;
    if (ctr == 0)
      message("But you have no spell-book!");
    else {
      just_browsing_book = false;
      FrmPopupForm(BookForm);
    }
    // Pop up a form to ask which spell
    // First ask which book.
    //    result = cast_spell("Cast which spell?", item_val, &choice, &chance);
    // Then call cast_helper(result, choice, chance);
  }
}

// Note some of these will also have to do which-direction (get_dir) foo....
// XXXXXXXX.
void cast_helper(Short result, Short choice, Short chance)
{
  Short i, dir; // j
  //  Short result;
  inven_type *i_ptr;
  spell_type *m_ptr;

  //  result = cast_spell("Cast which spell?", item_val, &choice, &chance); //xxx not implemented yet - it will be a form
  if (result < 0)
    message("You don't know any spells in that book.");
  else if (result > 0) {
    m_ptr = &magic_spell[pymisc.pclass-1][choice];
    free_turn_flag = false;

    if (randint(100) < chance)
      message("You failed to get the spell off!");
    else {
      /* Spells.  */
      switch(choice+1) {
      case 1:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  fire_bolt(GF_MAGIC_MISSILE, dir, char_row, char_col,
		    damroll(2, 6), spell_names[0]);
	break;
      case 2:
	detect_monsters();
	break;
      case 3:
	//	teleport(10); // xxx not implemented yet
	break;
      case 4:
	light_area(char_row, char_col);
	break;
      case 5:
	hp_player(damroll(4, 4));
	break;
      case 6:
	detect_sdoor();
	detect_trap();
	break;
      case 7:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  fire_ball(GF_POISON_GAS, dir, char_row, char_col, 12,
		    spell_names[6]);
	break;
      case 8:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  confuse_monster(dir, char_row, char_col);
	break;
      case 9:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  fire_bolt(GF_LIGHTNING, dir, char_row, char_col,
		    damroll(4, 8), spell_names[8]);
	break;
      case 10:
	td_destroy();
	break;
      case 11:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  sleep_monster(dir, char_row, char_col);
	break;
      case 12:
	cure_poison();
	break;
      case 13:
	//	teleport((Short)(pymisc.lev*5)); // xxx not implemented yet
	break;
      case 14:
	for (i = 22; i < INVEN_ARRAY_SIZE; i++) {
	  i_ptr = &inventory[i];
	  invy_set_flags(inventory, i, i_ptr->flags & ~TR_CURSED);
	}
	break;
      case 15:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  fire_bolt(GF_FROST, dir, char_row, char_col,
		    damroll(6, 8), spell_names[14]);
	break;
      case 16:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  wall_to_mud(dir, char_row, char_col);
	break;
      case 17:
	create_food();
	break;
      case 18:
	recharge(20);
	break;
      case 19:
	sleep_monsters1(char_row, char_col);
	break;
      case 20:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  poly_monster(dir, char_row, char_col);
	break;
      case 21:
	ident_spell();
	break;
      case 22:
	sleep_monsters2();
	break;
      case 23:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  fire_bolt(GF_FIRE, dir, char_row, char_col,
		    damroll(9, 8), spell_names[22]);
	break;
      case 24:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  (void)speed_monster(dir, char_row, char_col, -1);
	break;
      case 25:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  fire_ball(GF_FROST, dir, char_row, char_col, 48,
		    spell_names[24]);
	break;
      case 26:
	recharge(60);
	break;
      case 27:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	teleport_monster(dir, char_row, char_col);
	break;
      case 28:
	pyflags.fast += randint(20) + pymisc.lev;
	break;
      case 29:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  fire_ball(GF_FIRE, dir, char_row, char_col, 72,
		    spell_names[28]);
	break;
      case 30:
	destroy_area(char_row, char_col);
	break;
      case 31:
	genocide();
	break;
      default:
	break;
      }
      /* End of spells.				     */
      if (!free_turn_flag) {
	if ((spell_worked & (1L << choice)) == 0) {
	  pymisc.exp += m_ptr->sexp << 2;
	  spell_worked |= (1L << choice);
	  print_stats(STATS_QUO); //		      prt_experience();
	}
      }
    }
    if (!free_turn_flag) {
      if (m_ptr->smana > pymisc.cmana) {
	message("You faint from the effort!");
	pyflags.paralysis =
	  randint((Short)(5*(m_ptr->smana-pymisc.cmana)));
	pymisc.cmana = 0;
	pymisc.cmana_frac = 0;
	if (randint(3) == 1) {
	  message("You have damaged your health!");
	  dec_stat (A_CON);
	}
      }
      else
	pymisc.cmana -= m_ptr->smana;
      print_stats(STATS_QUO); // prt_cmana();
    }
  }
}

