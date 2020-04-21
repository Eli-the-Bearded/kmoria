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


/* Potions for the quaffing				-RAK-	*/
// Warning - this assumes that item_val IS a potion...
void quaff(Short item_val)
{
  Long i, l, j;
  Short ident;
  inven_type *i_ptr;

  //  free_turn_flag = true;
  //  if (inven_ctr == 0)
  //    message("But you are not carrying anything.");
  //  else if (!find_range(TV_POTION1, TV_POTION2, &j, &k))
  //    message("You are not carrying any potions.");
  //  else if (get_item(&item_val, "Quaff which potion?", j, k, CNIL, CNIL)) {
  i_ptr = &inventory[item_val];
  i = i_ptr->flags;
  free_turn_flag = false;
  ident = false;
  if (i == 0) {
    message ("You feel less thirsty.");
    ident = true;
  }
  else while (i != 0) {
    j = bit_pos(&i) + 1;
    if (i_ptr->tval == TV_POTION2)
      j += 32;
    /* Potions						*/
    switch(j) {
    case 1:
      if (inc_stat (A_STR)) {
	message("Wow!  What bulging muscles!");
	ident = true;
      }
      break;
    case 2:
      ident = true;
      lose_str();
      break;
    case 3:
      if (res_stat (A_STR)) {
	message("You feel warm all over.");
	ident = true;
      }
      break;
    case 4:
      if (inc_stat (A_INT)) {
	message("Aren't you brilliant!");
	ident = true;
      }
      break;
    case 5:
      ident = true;
      lose_int();
      break;
    case 6:
      if (res_stat (A_INT)) {
	message("You have have a warm feeling.");
	ident = true;
      }
      break;
    case 7:
      if (inc_stat (A_WIS)) {
	message("You suddenly have a profound thought!");
	ident = true;
      }
      break;
    case 8:
      ident = true;
      lose_wis();
      break;
    case 9:
      if (res_stat (A_WIS)) {
	message("You feel your wisdom returning.");
	ident = true;
      }
      break;
    case 10:
      if (inc_stat (A_CHR)) {
	message("Gee, ain't you cute!");
	ident = true;
      }
      break;
    case 11:
      ident = true;
      lose_chr();
      break;
    case 12:
      if (res_stat (A_CHR)) {
	message("You feel your looks returning.");
	ident = true;
      }
      break;
    case 13:
      ident = hp_player(damroll(2, 7));
      break;
    case 14:
      ident = hp_player(damroll(4, 7));
      break;
    case 15:
      ident = hp_player(damroll(6, 7));
      break;
    case 16:
      ident = hp_player(1000);
      break;
    case 17:
      if (inc_stat (A_CON)) {
	message("You feel tingly for a moment.");
	ident = true;
      }
      break;
    case 18:
      if (pymisc.exp < MAX_EXP) {
	l = (pymisc.exp / 2) + 10;
	if (l > 100000L)  l = 100000L;
	pymisc.exp += l;
	message("You feel more experienced.");
	print_stats(STATS_QUO);//prt_experience();
	ident = true;
      }
      break;
    case 19:
      if (!pyflags.free_act) {
	/* paralysis must == 0, otherwise could not drink potion */
	message("You fall asleep.");
	pyflags.paralysis += randint(4) + 4;
	ident = true;
      }
      break;
    case 20:
      if (pyflags.blind == 0) {
	message("You are covered by a veil of darkness.");
	ident = true;
      }
      pyflags.blind += randint(100) + 100;
      break;
    case 21:
      if (pyflags.confused == 0) {
	message("Hey!  This is good stuff!  * Hick! *");
	ident = true;
      }
      pyflags.confused += randint(20) + 12;
      break;
    case 22:
      if (pyflags.poisoned == 0) {
	message("You feel very sick.");
	ident = true;
      }
      pyflags.poisoned += randint(15) + 10;
      break;
    case 23:
      if (pyflags.fast == 0)
	ident = true;
      pyflags.fast += randint(25) + 15;
      break;
    case 24:
      if (pyflags.slow == 0)
	ident = true;
      pyflags.slow += randint(25) + 15;
      break;
    case 26:
      if (inc_stat (A_DEX)) {
	message("You feel more limber!");
	ident = true;
      }
      break;
    case 27:
      if (res_stat (A_DEX)) {
	message("You feel less clumsy.");
	ident = true;
      }
      break;
    case 28:
      if (res_stat (A_CON)) {
	message("You feel your health returning!");
	ident = true;
      }
      break;
    case 29:
      ident = cure_blindness();
      break;
    case 30:
      ident = cure_confusion();
      break;
    case 31:
      ident = cure_poison();
      break;
    case 34:
      if (pymisc.exp > 0) {
	Long m, scale;
	message("You feel your memories fade.");
	/* Lose between 1/5 and 2/5 of your experience */
	m = pymisc.exp / 5;
	if (pymisc.exp > MAX_SHORT) {
	  scale = MAX_LONG / pymisc.exp;
	  m += (randint((Short) scale) * pymisc.exp) / (scale * 5);
	}
	else
	  m += randint((Short) pymisc.exp) / 5;
	lose_exp(m);
	ident = true;
      }
      break;
    case 35:
      cure_poison();
      if (pyflags.food > 150)  pyflags.food = 150;
      pyflags.paralysis = 4;
      message("The potion makes you vomit!");
      ident = true;
      break;
    case 36:
      if (pyflags.invuln == 0)
	ident = true;
      pyflags.invuln += randint(10) + 10;
      break;
    case 37:
      if (pyflags.hero == 0)
	ident = true;
      pyflags.hero += randint(25) + 25;
      break;
    case 38:
      if (pyflags.shero == 0)
	ident = true;
      pyflags.shero += randint(25) + 25;
      break;
    case 39:
      ident = remove_fear();
      break;
    case 40:
      ident = restore_level();
      break;
    case 41:
      if (pyflags.resist_heat == 0)
	ident = true;
      pyflags.resist_heat += randint(10) + 10;
      break;
    case 42:
      if (pyflags.resist_cold == 0)
	ident = true;
      pyflags.resist_cold += randint(10) + 10;
      break;
    case 43:
      if (pyflags.detect_inv == 0)
	ident = true;
      detect_inv2(randint(12)+12);
      break;
    case 44:
      ident = slow_poison();
      break;
    case 45:
      ident = cure_poison();
      break;
    case 46:
      if (pymisc.cmana < pymisc.mana) {
	pymisc.cmana = pymisc.mana;
	ident = true;
	message("Your feel your head clear.");
	print_stats(STATS_QUO);//prt_cmana();
      }
      break;
    case 47:
      if (pyflags.tim_infra == 0) {
	message("Your eyes begin to tingle.");
	ident = true;
      }
      pyflags.tim_infra += 100 + randint(100);
      break;
    default:
      message ("Internal error in potion()");
      break;
    }
    /* End of Potions.					*/
  }
  if (ident) {
    if (!known1_p(i_ptr)) {
      /* round half-way case up */
      pymisc.exp += (i_ptr->level + (pymisc.lev >> 1)) / pymisc.lev;
      print_stats(STATS_QUO);//prt_experience();

      identify(&item_val);
      i_ptr = &inventory[item_val];
    }
  }
  else if (!known1_p(i_ptr))
    sample (i_ptr);

  add_food(i_ptr->p1);
  desc_remain(item_val);
  inven_destroy(item_val);
  //  }
}



