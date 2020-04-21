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

/* Needed:

data:
prayer.c:24: `class' undeclared (first use in this function)
prayer.c:35: `magic_spell' undeclared (first use in this function)
prayer.c:161: `spell_worked' undeclared (first use in this function)

functions:
`no_light'
`find_range'
`get_item'
`cast_spell'
`detect_evil'
`hp_player'
`bless'
`remove_fear'
`light_area'
`detect_trap'
`detect_sdoor'
`slow_poison'
`get_dir'
`confuse_monster'
`teleport'
`sleep_monsters1'
`create_food'
`cure_poison'
`fire_ball'
`detect_inv2'
`protect_evil'
`earthquake'
`map_area'
`turn_undead'
`dispel_creature'
`warding_glyph'

 */


/* Pray like HELL.					-RAK-	*/
extern Boolean just_browsing_book; 

void pray()
{
  //  Short i, j, dir;
  //  Short choice, chance, result;
  //  spell_type *s_ptr;
  //  struct misc *m_ptr;
  //  struct flags *f_ptr;
  //  inven_type *i_ptr;

  free_turn_flag = true;
  if (pyflags.blind > 0)
    message("You can't see to read your prayer!");
  else if (no_light())
    message("You have no light to read by.");
  else if (pyflags.confused > 0)
    message("You are too confused.");
  else if (class[pymisc.pclass].spell != PRIEST)
    message("Pray hard enough and your prayers may be answered.");
  //  else if (inven_ctr == 0)
  //    message ("But you are not carrying anything!");
  //  else if (!find_range(TV_PRAYER_BOOK, TV_NEVER, &i, &j))
  //    message ("You are not carrying any Holy Books!");
  //  else if (get_item(&item_val, "Use which Holy Book?", i, j, CNIL, CNIL)) {
  else {
    Short i, ctr = 0;
    for (i = 0 ; i < inven_ctr ; i++)
      if (inventory[i].tval == TV_PRAYER_BOOK)
	ctr++;
    if (ctr == 0)
      message("But you have no Holy Book!");
    else {
      just_browsing_book = false;
      FrmPopupForm(BookForm);
    }
    // Launch the recite-which-prayer form....
    // It should call pray_helper(Short choice, Short chance)
  }
}

// Note some of these will also have to do which-direction (get_dir) foo....
void pray_helper(Short result, Short choice, Short chance)
{
  Short i, dir; // j
  //  Short result;
  spell_type *s_ptr;
  //  struct misc *m_ptr;
  //  struct flags *f_ptr;
  inven_type *i_ptr;

  //  result = cast_spell("Recite which prayer?", item_val, &choice, &chance);
  if (result < 0)
    message("You don't know any prayers in that book.");
  else if (result > 0) {
    s_ptr = &magic_spell[pymisc.pclass-1][choice];
    free_turn_flag = false;

    if (randint(100) < chance)
      message("You lost your concentration!");
    else {
      /* Prayers.					*/
      switch(choice+1) {
      case 1:
	detect_evil();
	break;
      case 2:
	hp_player(damroll(3, 3));
	break;
      case 3:
	bless(randint(12)+12);
	break;
      case 4:
	remove_fear();
	break;
      case 5:
	light_area(char_row, char_col);
	break;
      case 6:
	detect_trap();
	break;
      case 7:
	detect_sdoor();
	break;
      case 8:
	slow_poison();
	break;
      case 9:
	//	if (get_dir(CNIL, &dir)) // xxx not implemented yet
	  confuse_monster(dir, char_row, char_col);
	break;
      case 10:
	//	teleport((Short)(pymisc.lev*3)); // xxx not implemented yet
	break;
      case 11:
	hp_player(damroll(4, 4));
	break;
      case 12:
	bless(randint(24)+24);
	break;
      case 13:
	sleep_monsters1(char_row, char_col);
	break;
      case 14:
	create_food();
	break;
      case 15:
	for (i = 0; i < INVEN_ARRAY_SIZE; i++) {
	  i_ptr = &inventory[i];
	  /* only clear flag for items that are wielded or worn */
	  if (i_ptr->tval >= TV_MIN_WEAR && i_ptr->tval <= TV_MAX_WEAR)
	    invy_set_flags(inventory, i, i_ptr->flags & ~TR_CURSED);
	}
	break;
      case 16:
	pyflags.resist_heat += randint(10) + 10;
	pyflags.resist_cold += randint(10) + 10;
	break;
      case 17:
	cure_poison();
	break;
      case 18:
	//	if (get_dir(CNIL, &dir))// xxx not implemented yet
	  fire_ball(GF_HOLY_ORB, dir, char_row, char_col,
		    (Short)(damroll(3, 6)+pymisc.lev),
		    "Black Sphere");
	break;
      case 19:
	hp_player(damroll(8, 4));
	break;
      case 20:
	detect_inv2(randint(24)+24);
	break;
      case 21:
	protect_evil();
	break;
      case 22:
	earthquake();
	break;
      case 23:
	map_area();
	break;
      case 24:
	hp_player(damroll(16, 4));
	break;
      case 25:
	turn_undead();
	break;
      case 26:
	bless(randint(48)+48);
	break;
      case 27:
	dispel_creature(CD_UNDEAD, (Short)(3*pymisc.lev));
	break;
      case 28:
	hp_player(200);
	break;
      case 29:
	dispel_creature(CD_EVIL, (Short)(3*pymisc.lev));
	break;
      case 30:
	warding_glyph();
	break;
      case 31:
	remove_fear();
	cure_poison();
	hp_player(1000);
	for (i=A_STR; i<=A_CHR; i++)
	  res_stat(i);
	dispel_creature(CD_EVIL, (Short)(4*pymisc.lev));
	turn_undead();
	if (pyflags.invuln < 3)
	  pyflags.invuln = 3;
	else
	  pyflags.invuln++;
	break;
      default:
	break;
      }
      /* End of prayers.				*/
      if (!free_turn_flag) {
	if ((spell_worked & (1L << choice)) == 0) {
	  pymisc.exp += s_ptr->sexp << 2;
	  print_stats(STATS_QUO);	    //		      prt_experience();
	  spell_worked |= (1L << choice);
	}
      }
    }
    if (!free_turn_flag) {
      if (s_ptr->smana > pymisc.cmana) {
	message("You faint from fatigue!");
	pyflags.paralysis = randint((Short)(5*(s_ptr->smana-pymisc.cmana)));
	pymisc.cmana = 0;
	pymisc.cmana_frac = 0;
	if (randint(3) == 1) {
	  message("You have damaged your health!");
	  dec_stat (A_CON);
	}
      }
      else
	pymisc.cmana -= s_ptr->smana;
      print_stats(STATS_QUO);	//	      prt_cmana();
    }
  }
}
