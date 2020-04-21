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

// Needs:
// teleport     misc3.c
// add_inscribe misc4.c

/* Use a staff.					-RAK-	*/
// Warning - assumes item_val IS a staff
void use(Short item_val)
{
  ULong i;
  Short j, k, chance, y, x;
  Short ident;
  inven_type *i_ptr;

  /*
    free_turn_flag = true;
    if (inven_ctr == 0) {
    message("But you are not carrying anything.");
    return;
    }
    else if (!find_range(TV_STAFF, TV_NEVER, &j, &k)) {
    message("You are not carrying any staffs.");
    }
    else if (get_item(&item_val, "Use which staff?", j, k, CNIL, CNIL)) {
  */
  // item_val contains inventory index of the desired staff
  i_ptr = &inventory[item_val];
  free_turn_flag = false;
  // decide whether we use the staff properly
  chance = pymisc.save + stat_adj(A_INT) - (Short)i_ptr->level - 5
    + (class_level_adj[pymisc.pclass][CLA_DEVICE] * pymisc.lev / 3);
  if (pyflags.confused > 0)
    chance = chance / 2;
  if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1))
    chance = USE_DEVICE; /* Give everyone a slight chance */
  if (chance <= 0)	chance = 1;
  if (randint(chance) < USE_DEVICE)
    message("You failed to use the staff properly.");
  else if (i_ptr->p1 > 0) {
    i = i_ptr->flags; // what kind of staff is it
    ident = false; // do we recognize it from its effect
    invy_set_p1(inventory, item_val, i_ptr->p1-1); // decrement # of charges
    while (i != 0){ 
      j = bit_pos(&i) + 1;
      /* Staffs. */
      switch(j) {
      case 1:
	ident = light_area(char_row, char_col);
	break;
      case 2:
	ident = detect_sdoor();
	break;
      case 3:
	ident = detect_trap();
	break;
      case 4:
	ident = detect_treasure();
	break;
      case 5:
	ident = detect_object();
	break;
      case 6:
	ident = true;
	//	teleport(100); // xxx not implemented yet
	break;
      case 7:
	ident = true;
	earthquake();
	break;
      case 8:
	ident = false;
	for (k = 0; k < randint(4); k++){
	  y = char_row;
	  x = char_col;
	  ident |= summon_monster(&y, &x, false); // now that's just weird.
	}
	break;
      case 10:
	ident = true;
	destroy_area(char_row, char_col);
	break;
      case 11:
	ident = true;
	starlite(char_row, char_col);
	break;
      case 12:
	ident = speed_monsters(1);
	break;
      case 13:
	ident = speed_monsters(-1);
	break;
      case 14:
	ident = sleep_monsters2();
	break;
      case 15:
	ident = hp_player(randint(8));
	break;
      case 16:
	ident = detect_invisible();
	break;
      case 17:
	if (pyflags.fast == 0)
	  ident = true;
	pyflags.fast += randint(30) + 15;
	break;
      case 18:
	if (pyflags.slow == 0)
	  ident = true;
	pyflags.slow += randint(30) + 15;
	break;
      case 19:
	ident = mass_poly();
	break;
      case 20:
	if (remove_curse()) {
	  if (pyflags.blind < 1)
	    message("The staff glows blue for a moment..");
	  ident = true;
	}
	break;
      case 21:
	ident = detect_evil();
	break;
      case 22:
	if ((cure_blindness()) || (cure_poison()) ||
	    (cure_confusion()))
	  ident = true;
	break;
      case 23:
	ident = dispel_creature(CD_EVIL, 60);
	break;
      case 25:
	ident = unlight_area(char_row, char_col);
	break;
      case 32:
	/* store bought flag */
	break;
      default:
	message("Internal error in staffs()");
	break;
      }
      /* End of staff actions.		*/
    }
    if (ident) {
      if (!known1_p(i_ptr)) {
	/* round half-way case up */
	pymisc.exp += (i_ptr->level + (pymisc.lev >> 1)) / pymisc.lev;
	//	  prt_experience();
	print_stats(STATS_QUO);

	identify(&item_val);
	i_ptr = &inventory[item_val];
      }
    }
    else if (!known1_p(i_ptr))
      sample(i_ptr);
    desc_charges(item_val);
  }
  else {
    message("The staff has no charges left.");
    if (!known2_p(i_ptr))
      add_inscribe(inventory, item_val, ID_EMPTY);
  }
  //  }
}
