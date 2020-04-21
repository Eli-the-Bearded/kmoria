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
//#include "kMoriaRsc.h"
#include "kMoria.h"
#include "lock-externs.h"

/* Scrolls for the reading				-RAK-	*/
// Assumes item_val IS a scroll.
// None of these will also have to do which-direction (get_dir) foo...?
void read_scroll(Short item_val)
{
  ULong i;
  Short j, k, y, x;
  Short tmp[6], flag, used_up;
  Char out_val[160], tmp_str[160];
  Short ident, l;
  inven_type *i_ptr;

  free_turn_flag = true;
  if (pyflags.blind > 0)
    message("You can't see to read the scroll.");
  else if (no_light())
    message("You have no light to read by.");
  else if (pyflags.confused > 0)
    message("You are too confused to read a scroll.");
  //  else if (inven_ctr == 0)
  //    message("You are not carrying anything!");
  //  else if (!find_range(TV_SCROLL1, TV_SCROLL2, &j, &k))
  //    message ("You are not carrying any scrolls!");
  //  else if (get_item(&item_val, "Read which scroll?", j, k, CNIL, CNIL)) {
  else {
    i_ptr = &inventory[item_val];
    free_turn_flag = false;
    used_up = true;
    i = i_ptr->flags;
    ident = false;

    while (i != 0) {
      j = bit_pos(&i) + 1;
      if (i_ptr->tval == TV_SCROLL2)
	j += 32;

      /* Scrolls.			*/
      switch(j) {
      case 1: // enchant to-hit
	i_ptr = &inventory[INVEN_WIELD];
	if (i_ptr->tval != TV_NOTHING) {
	  Short tohit = i_ptr->tohit;
	  Short todam = i_ptr->todam;
	  Short toac  = i_ptr->toac;
	  objdes(tmp_str, i_ptr, false);
	  StrPrintF(out_val, "Your %s glows faintly!", tmp_str);
	  message(out_val);
	  if (enchant(&tohit, 10)) {
	    invy_set_hitdamac(inventory, INVEN_WIELD, tohit, todam, toac);
	    invy_set_flags(inventory, INVEN_WIELD, i_ptr->flags & ~TR_CURSED);
	    calc_bonuses();
	  } else
	    message("The enchantment fails.");
	  ident = true;
	}
	break;
      case 2: // enchant to-dam
	i_ptr = &inventory[INVEN_WIELD];
	if (i_ptr->tval != TV_NOTHING) {
	  Short tohit = i_ptr->tohit;
	  Short todam = i_ptr->todam;
	  Short toac  = i_ptr->toac;
	  objdes(tmp_str, i_ptr, false);
	  StrPrintF(out_val, "Your %s glows faintly!", tmp_str);
	  message(out_val);
	  if ((i_ptr->tval >= TV_HAFTED)&&(i_ptr->tval <= TV_DIGGING))
	    j = i_ptr->damage[0] * i_ptr->damage[1];
	  else /* Bows' and arrows' enchantments should not be limited
		  by their low base damages */
	    j = 10; 
	  if (enchant(&todam, j)) {
	    invy_set_hitdamac(inventory, INVEN_WIELD, tohit, todam, toac);
	    invy_set_flags(inventory, INVEN_WIELD, i_ptr->flags & ~TR_CURSED);
	    calc_bonuses ();
	  } else
	    message("The enchantment fails.");
	  ident = true;
	}
	break;
      case 3: // enchant armor
	k = 0;
	l = 0;
	if (inventory[INVEN_BODY].tval != TV_NOTHING)
	  tmp[k++] = INVEN_BODY;
	if (inventory[INVEN_ARM].tval != TV_NOTHING)
	  tmp[k++] = INVEN_ARM;
	if (inventory[INVEN_OUTER].tval != TV_NOTHING)
	  tmp[k++] = INVEN_OUTER;
	if (inventory[INVEN_HANDS].tval != TV_NOTHING)
	  tmp[k++] = INVEN_HANDS;
	if (inventory[INVEN_HEAD].tval != TV_NOTHING)
	  tmp[k++] = INVEN_HEAD;
	/* also enchant boots */
	if (inventory[INVEN_FEET].tval != TV_NOTHING)
	  tmp[k++] = INVEN_FEET;

	if (k > 0)  l = tmp[randint(k)-1];
	if (TR_CURSED & inventory[INVEN_BODY].flags)
	  l = INVEN_BODY;
	else if (TR_CURSED & inventory[INVEN_ARM].flags)
	  l = INVEN_ARM;
	else if (TR_CURSED & inventory[INVEN_OUTER].flags)
	  l = INVEN_OUTER;
	else if (TR_CURSED & inventory[INVEN_HEAD].flags)
	  l = INVEN_HEAD;
	else if (TR_CURSED & inventory[INVEN_HANDS].flags)
	  l = INVEN_HANDS;
	else if (TR_CURSED & inventory[INVEN_FEET].flags)
	  l = INVEN_FEET;

	if (l > 0) {
	  Short toac;
	  i_ptr = &inventory[l];
	  toac  = i_ptr->toac;
	  objdes(tmp_str, i_ptr, false);
	  StrPrintF(out_val, "Your %s glows faintly!", tmp_str);
	  message(out_val);
	  if (enchant(&toac, 10)) {
	    invy_set_hitdamac(inventory, l, i_ptr->tohit, i_ptr->todam, toac);
	    invy_set_flags(inventory, l, i_ptr->flags & ~TR_CURSED);
	    calc_bonuses ();
	  } else
	    message("The enchantment fails.");
	  ident = true;
	}
	break;
      case 4: // identify
	message("This is an identify scroll.");
	ident = true;
	used_up = ident_spell(); // user has option to abort w/o using it up

	/* The identify may merge objects, causing the identify scroll
	   to move to a different place.	Check for that here.  It can
	   move arbitrarily far if an identify scroll was used on
	   another identify scroll, but it always moves down. */
	while (i_ptr->tval != TV_SCROLL1 || i_ptr->flags != 0x00000008) {
	  item_val--;
	  i_ptr = &inventory[item_val];
	}
	break;
      case 5: // remove curse
	if (remove_curse()) {
	  message("You feel as if someone is watching over you.");
	  ident = true;
	}
	break;
      case 6: // light
	ident = light_area(char_row, char_col);
	break;
      case 7: // summon monster
	for (k = 0; k < randint(3); k++) {
	  y = char_row;
	  x = char_col;
	  ident |= summon_monster(&y, &x, false);
	}
	break;
      case 8: // phase door
	teleport(10);
	ident = true;
	break;
      case 9: // teleport
	teleport(100);
	ident = true;
	break;
      case 10: // teleport level
	dun_level += (-3) + 2*randint(2);
	if (dun_level < 1)
	  dun_level = 1;
	new_level_flag = true;
	ident = true;
	break;
      case 11: // monster confusion
	if (pyflags.confuse_monster == 0) {
	  message("Your hands begin to glow.");
	  pyflags.confuse_monster = true;
	  ident = true;
	}
	break;
      case 12: // magic mapping
	ident = true;
	map_area();
	break;
      case 13: // sleep monster
	ident = sleep_monsters1(char_row, char_col);
	break;
      case 14: // rune of protection
	ident = true;
	warding_glyph();
	break;
      case 15: // treasure detection
	ident = detect_treasure();
	break;
      case 16: // object detection
	ident = detect_object();
	break;
      case 17: // trap detection
	ident = detect_trap();
	break;
      case 18: // door/stair location
	ident = detect_sdoor();
	break;
      case 19: // mass genocide
	message("This is a mass genocide scroll.");
	mass_genocide();
	ident = true;
	break;
      case 20: // detect invisible
	ident = detect_invisible();
	break;
      case 21: // aggravate monster
	message("There is a high pitched humming noise.");
	aggravate_monster(20);
	ident = true;
	break;
      case 22: // trap creation
	ident = trap_creation();
	break;
      case 23: // trap/door destruction
	ident = td_destroy();
	break;
      case 24: // door creation
	ident = door_creation();
	break;
      case 25: // recharging
	message("This is a Recharge-Item scroll.");
	ident = true;
	used_up = recharge(60);
	break;
      case 26: // genocide
	message("This is a genocide scroll.");
	genocide();
	ident = true;
	break;
      case 27: // darkness
	ident = unlight_area(char_row, char_col);
	break;
      case 28: // protection from evil
	ident = protect_evil();
	break;
      case 29: // create food
	ident = true;
	create_food();
	break;
      case 30: // dispell undead
	ident = dispel_creature(CD_UNDEAD, 60);
	break;
      case 33: // *enchant weapon*
	i_ptr = &inventory[INVEN_WIELD];
	if (i_ptr->tval != TV_NOTHING) {
	  Short tohit = i_ptr->tohit;
	  Short todam = i_ptr->todam;
	  Short toac  = i_ptr->toac;
	  objdes(tmp_str, i_ptr, false);
	  StrPrintF(out_val, "Your %s glows brightly!", tmp_str);
	  message(out_val);
	  flag = false;
	  for (k = 0; k < randint(2); k++)
	    if (enchant(&tohit, 10))
	      flag = true;
	  if ((i_ptr->tval >= TV_HAFTED)&&(i_ptr->tval <= TV_DIGGING))
	    j = i_ptr->damage[0] * i_ptr->damage[1];
	  else /* Bows' and arrows' enchantments should not be limited
		  by their low base damages */
	    j = 10; 
	  for (k = 0; k < randint(2); k++)
	    if (enchant(&todam, j))
	      flag = true;
	  if (flag) {
	    invy_set_hitdamac(inventory, INVEN_WIELD, tohit, todam, toac);
	    invy_set_flags(inventory, INVEN_WIELD, i_ptr->flags & ~TR_CURSED);
	    calc_bonuses ();
	  } else
	    message("The enchantment fails.");
	  ident = true;
	}
	break;
      case 34: // curse weapon
	i_ptr = &inventory[INVEN_WIELD];
	if (i_ptr->tval != TV_NOTHING) {
	  objdes(tmp_str, i_ptr, false);
	  StrPrintF(out_val,"Your %s glows black, fades.",tmp_str);
	  message(out_val);
	  unmagic_name(INVEN_WIELD);// xxx
	  invy_set_hitdamac(inventory, INVEN_WIELD, -randint(5) - randint(5),
				 -randint(5) - randint(5), 0);
	  //	  i_ptr->tohit = -randint(5) - randint(5);
	  //	  i_ptr->todam = -randint(5) - randint(5);
	  //	  i_ptr->toac = 0;
	  /* Must call py_bonuses() before set (clear) flags, and
	     must call calc_bonuses() after set (clear) flags, so that
	     all attributes will be properly turned off. */
	  py_bonuses(i_ptr, -1);
	  invy_set_flags(inventory, INVEN_WIELD, TR_CURSED);
	  calc_bonuses ();
	  ident = true;
	}
	break;
      case 35: // *enchant armor*
	k = 0;
	l = 0;
	if (inventory[INVEN_BODY].tval != TV_NOTHING)
	  tmp[k++] = INVEN_BODY;
	if (inventory[INVEN_ARM].tval != TV_NOTHING)
	  tmp[k++] = INVEN_ARM;
	if (inventory[INVEN_OUTER].tval != TV_NOTHING)
	  tmp[k++] = INVEN_OUTER;
	if (inventory[INVEN_HANDS].tval != TV_NOTHING)
	  tmp[k++] = INVEN_HANDS;
	if (inventory[INVEN_HEAD].tval != TV_NOTHING)
	  tmp[k++] = INVEN_HEAD;
	/* also enchant boots */
	if (inventory[INVEN_FEET].tval != TV_NOTHING)
	  tmp[k++] = INVEN_FEET;

	if (k > 0)  l = tmp[randint(k)-1];
	if (TR_CURSED & inventory[INVEN_BODY].flags)
	  l = INVEN_BODY;
	else if (TR_CURSED & inventory[INVEN_ARM].flags)
	  l = INVEN_ARM;
	else if (TR_CURSED & inventory[INVEN_OUTER].flags)
	  l = INVEN_OUTER;
	else if (TR_CURSED & inventory[INVEN_HEAD].flags)
	  l = INVEN_HEAD;
	else if (TR_CURSED & inventory[INVEN_HANDS].flags)
	  l = INVEN_HANDS;
	else if (TR_CURSED & inventory[INVEN_FEET].flags)
	  l = INVEN_FEET;

	if (l > 0) {
	  Short toac;
	  i_ptr = &inventory[l];
	  toac  = i_ptr->toac;
	  objdes(tmp_str, i_ptr, false);
	  StrPrintF(out_val,"Your %s glows brightly!", tmp_str);
	  message(out_val);
	  flag = false;
	  for (k = 0; k < randint(2) + 1; k++)
	    if (enchant(&toac, 10))
	      flag = true;
	  if (flag) {
	    invy_set_hitdamac(inventory, l, i_ptr->tohit,i_ptr->todam, toac);
	    invy_set_flags(inventory, i, i_ptr->flags & ~TR_CURSED);
	    calc_bonuses ();
	  } else
	    message("The enchantment fails.");
	  ident = true;
	}
	break;
      case 36: // curse armor
	if ((inventory[INVEN_BODY].tval != TV_NOTHING)
	    && (randint(4) == 1))
	  k = INVEN_BODY;
	else if ((inventory[INVEN_ARM].tval != TV_NOTHING)
		 && (randint(3) ==1))
	  k = INVEN_ARM;
	else if ((inventory[INVEN_OUTER].tval != TV_NOTHING)
		 && (randint(3) ==1))
	  k = INVEN_OUTER;
	else if ((inventory[INVEN_HEAD].tval != TV_NOTHING)
		 && (randint(3) ==1))
	  k = INVEN_HEAD;
	else if ((inventory[INVEN_HANDS].tval != TV_NOTHING)
		 && (randint(3) ==1))
	  k = INVEN_HANDS;
	else if ((inventory[INVEN_FEET].tval != TV_NOTHING)
		 && (randint(3) ==1))
	  k = INVEN_FEET;
	else if (inventory[INVEN_BODY].tval != TV_NOTHING)
	  k = INVEN_BODY;
	else if (inventory[INVEN_ARM].tval != TV_NOTHING)
	  k = INVEN_ARM;
	else if (inventory[INVEN_OUTER].tval != TV_NOTHING)
	  k = INVEN_OUTER;
	else if (inventory[INVEN_HEAD].tval != TV_NOTHING)
	  k = INVEN_HEAD;
	else if (inventory[INVEN_HANDS].tval != TV_NOTHING)
	  k = INVEN_HANDS;
	else if (inventory[INVEN_FEET].tval != TV_NOTHING)
	  k = INVEN_FEET;
	else
	  k = 0;

	if (k > 0) {
	  i_ptr = &inventory[k];
	  objdes(tmp_str, i_ptr, false);
	  StrPrintF(out_val,"Your %s glows black, fades.",tmp_str);
	  message(out_val);
	  unmagic_name(k);
	  invy_set_flags(inventory, k, TR_CURSED); // xxx
	  invy_set_hitdamac(inventory, k, 0, 0, -randint(5) - randint(5));
	  //	  i_ptr->tohit = 0;
	  //	  i_ptr->todam = 0;
	  //	  i_ptr->toac = -randint(5) - randint(5);
	  calc_bonuses ();
	  ident = true;
	}
	break;
      case 37: // summon undead
	ident = false;
	for (k = 0; k < randint(3); k++) {
	  y = char_row;
	  x = char_col;
	  ident |= summon_undead(&y, &x);
	}
	break;
      case 38: // blessing
	ident = true;
	bless(randint(12)+6);
	break;
      case 39: // holy chant
	ident = true;
	bless(randint(24)+12);
	break;
      case 40: // holy prayer
	ident = true;
	bless(randint(48)+24);
	break;
      case 41: // word-of-recall
	ident = true;
	if (pyflags.word_recall == 0)
	  pyflags.word_recall = 25 + randint(30);
	message("The air about you becomes charged.");
	break;
      case 42: // *destruction*
	destroy_area(char_row, char_col);
	ident = true;
	break;
      default:
	message("Internal error in scroll()");
	break;
      }
      /* End of Scrolls.			       */
    }
    i_ptr = &inventory[item_val];
    if (ident) {
      if (!known1_p(i_ptr)) {
	/* round half-way case up */
	pymisc.exp += (i_ptr->level +(pymisc.lev >> 1)) / pymisc.lev;
	print_stats(STATS_QUO); //prt_experience();

	identify(&item_val);
	i_ptr = &inventory[item_val];
      }
    } else if (!known1_p(i_ptr))
      sample (i_ptr);
    if (used_up) {
      desc_remain(item_val);
      inven_destroy(item_val);
    }
  }
}
