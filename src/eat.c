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

/* Needed:

function `find_range'
function `get_item'
function `draw_cave'
function `cure_poison'
function `cure_blindness'
function `cure_confusion'
function `lose_str'
function `lose_con'
function `hp_player'
function `take_hit'
function `desc_remain'
function `inven_destroy'

 */
Boolean get_item(Short * a, Char * b, Short c, Short d, Char * e, Char * f)
{
  return false;
}


/* Eat some food.					-RAK-	*/
// Warning - this assuems item_val IS some kind of food
void eat(Short item_val)
{
  ULong i;
  Short j;
  Boolean ident;
  inven_type *i_ptr;

  //  free_turn_flag = true;
  //  if (inven_ctr == 0)
  //    message("But you are not carrying anything.");
  //  else if (!find_range(TV_FOOD, TV_NEVER, &j, &k))
  //    message("You are not carrying any food.");
  //  else if (get_item(&item_val, "Eat what?", j, k, CNIL, CNIL)) {
  i_ptr = &inventory[item_val];
  free_turn_flag = false;
  i = i_ptr->flags;
  ident = false;
  while (i != 0) {
    j = bit_pos(&i) + 1;
    /* Foods					*/
    switch(j) {
    case 1:
      pyflags.poisoned += randint(10) + i_ptr->level;
      ident = true;
      break;
    case 2:
      pyflags.blind += randint(250) + 10*i_ptr->level + 100;
      draw_cave();
      message("A veil of darkness surrounds you.");
      ident = true;
      break;
    case 3:
      pyflags.afraid += randint(10) + i_ptr->level;
      message("You feel terrified!");
      ident = true;
      break;
    case 4:
      pyflags.confused += randint(10) + i_ptr->level;
      message("You feel drugged.");
      ident = true;
      break;
    case 5:
      pyflags.image += randint(200) + 25*i_ptr->level + 200;
      message("You feel drugged.");
      ident = true;
      break;
    case 6:
      ident = cure_poison();
      break;
    case 7:
      ident = cure_blindness();
      break;
    case 8:
      if (pyflags.afraid > 1) {
	pyflags.afraid = 1;
	ident = true;
      }
      break;
    case 9:
      ident = cure_confusion();
      break;
    case 10:
      ident = true;
      lose_str();
      break;
    case 11:
      ident = true;
      lose_con();
      break;
#if 0  /* 12 through 15 are not used */
    case 12:
      ident = true;
      lose_int();
      break;
    case 13:
      ident = true;
      lose_wis();
      break;
    case 14:
      ident = true;
      lose_dex();
      break;
    case 15:
      ident = true;
      lose_chr();
      break;
#endif
    case 16:
      if (res_stat (A_STR)) {
	message("You feel your strength returning.");
	ident = true;
      }
      break;
    case 17:
      if (res_stat (A_CON)) {
	message("You feel your health returning.");
	ident = true;
      }
      break;
    case 18:
      if (res_stat (A_INT)) {
	message("Your head spins a moment.");
	ident = true;
      }
      break;
    case 19:
      if (res_stat (A_WIS)) {
	message("You feel your wisdom returning.");
	ident = true;
      }
      break;
    case 20:
      if (res_stat (A_DEX)) {
	message("You feel more dextrous.");
	ident = true;
      }
      break;
    case 21:
      if (res_stat (A_CHR)) {
	message("Your skin stops itching.");
	ident = true;
      }
      break;
    case 22:
      ident = hp_player(randint(6));
      break;
    case 23:
      ident = hp_player(randint(12));
      break;
    case 24:
      ident = hp_player(randint(18));
      break;
#if 0  /* 25 is not used */
    case 25:
      ident = hp_player(damroll(3, 6));
      break;
#endif
    case 26:
      ident = hp_player(damroll(3, 12));
      break;
    case 27:
      take_hit(randint(18), "poisonous food.");
      ident = true;
      break;
#if 0 /* 28 through 30 are not used */
    case 28:
      take_hit(randint(8), "poisonous food.");
      ident = true;
      break;
    case 29:
      take_hit(damroll(2, 8), "poisonous food.");
      ident = true;
      break;
    case 30:
      take_hit(damroll(3, 8), "poisonous food.");
      ident = true;
      break;
#endif
    default:
      message("Internal error in eat()");
      break;
    }
    /* End of food actions.				*/
  }
  if (ident) {
    if (!known1_p(i_ptr)) {
      /* use identified it, gain experience */
      /* round half-way case up */
      pymisc.exp += (i_ptr->level + (pymisc.lev >> 1)) / pymisc.lev;
      print_stats(STATS_QUO);//prt_experience();

      identify (&item_val);
      i_ptr = &inventory[item_val];
    }
  }
  else if (!known1_p(i_ptr))
    sample (i_ptr);
  add_food(i_ptr->p1);
  pyflags.status &= ~(PY_WEAK|PY_HUNGRY);
  print_stats(STATS_QUO);//prt_hunger();
  desc_remain(item_val);
  inven_destroy(item_val);
  //  }
}
