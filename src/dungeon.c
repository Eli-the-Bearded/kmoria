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

static Boolean valid_countcommand(Char c) SEC_5;
static void regenhp(Short percent) SEC_5;
static void regenmana(Short percent) SEC_5;
static Boolean enchanted (inven_type *t_ptr) SEC_5;
static void go_up() SEC_5;
static void go_down() SEC_5;
static void jamdoor(Short dir) SEC_5;
static void exchange_weap() SEC_5;
static Char map_roguedir(Char comval) SEC_5;
static void ident_char(Char command) SEC_5;

extern Short recall_this_monster;


UChar need_print_stats;

// In the original,
// you exit the dungeon loop when it's time to start a new dungeon level.

Short find_count; // this was a local variable in dungeon() so was regen_amount
extern Boolean do_pickup;
/* 
 * Call this every time you enter a new level.  THis was the stuff at the
 * start of dungeon() before you enter the giant Do-While loop.
 */
//void dungeon()
void enter_new_level()
{
  /* Main procedure for dungeon.			-RAK-	*/
  /* Note: There is a lot of preliminary magic going on here at first*/
  // This procedure is just all the preliminary magic.
  need_print_stats = 0;

  /* Check light status for setup	   */
  if (inventory[INVEN_LIGHT].p1 > 0)
    player_light = true;
  else
    player_light = false;
  /* Check for a maximum level		   */
  if (dun_level > pymisc.max_dlv)
    pymisc.max_dlv = dun_level;

  /* Reset flags and initialize variables  */
  command_count = 0;
  find_count = 0;
  new_level_flag    = false;
  find_flag	= false;
  teleport_flag = false;
  mon_tot_mult	= 0;
  cave_set(char_row, char_col, CAVE_cptr, 1); // put the '@' where you are!!!
  /* Ensure we display the panel. Used to do this with a global var. -CJS- */
  panel_row = panel_col = -1;
  /* Light up the area around character	   */ // also move panel if needed.
  check_view();
  /* must do this after panel_row/col set to -1, because search_off() will
     call check_view(), and so the panel_* variables must be valid before
     search_off() is called */
  if (pyflags.status & PY_SEARCH)
    search_off();
  /* Light,  but do not move critters	    */
  creatures(false);
  /* Print the depth			   */
  //  prt_depth(); // XXXXX dude, just print_stats(mumble)
  need_print_stats |= STATS_QUO;

}

/*************************************************************************
 * This is stuff from the beginning of dungeon(),
 * the stuff that gets done BEFORE we require player input.
 * (which, remember, will be an EVENT for us and not a polled char.)
 *************************************************************************/
void start_of_turn()
{
  //  inven_type *i_ptr;
  Short plight, regen_amount, i;
  turn++;
  /* I am leaving out: game-hours restriction.  We are not a mainframe! -bas */

  /* turn over the store contents every, say, 1000 turns */
  if ((dun_level != 0) && ((turn % 1000) == 0))
    store_maint();

  /* Check for creature generation		*/
  if (randint(MAX_MALLOC_CHANCE) == 1)
    alloc_monster(1, MAX_SIGHT, false);

  /* Check light status			       */
  //  i_ptr = &inventory[INVEN_LIGHT];
  plight = inventory[INVEN_LIGHT].p1;
  if (player_light) {
    if (plight > 0) {
      plight--;
      invy_set_p1(inventory, INVEN_LIGHT, plight); // used a turn of light
      if (plight == 0) {
	player_light = false;
	message("Your light has gone out!");
	disturb(false, true);
	/* unlight creatures */
	creatures(false);
      } else if ((plight < 40) && (randint(5) == 1) &&
		 (pyflags.blind < 1)) {
	disturb(false, false);
	message("Your light is growing faint.");
      }
    } else {
      player_light = false;
      disturb(false, true);
      /* unlight creatures */
      creatures(false);
    }
  } else if (plight > 0) {
    invy_set_p1(inventory, INVEN_LIGHT, plight); // used a turn of light
    player_light = true;
    disturb(false, true);
    /* light creatures */
    creatures(false);
  } /* end of Check light status                  */
  
  /* Update counters and messages (quite long)			*/

  /* Heroism (must precede anything that can damage player)      */
  if (pyflags.hero > 0) {
    if ((PY_HERO & pyflags.status) == 0) {
      pyflags.status |= PY_HERO;
      disturb(false, false);
      pymisc.mhp += 10;
      pymisc.chp += 10;
      pymisc.bth += 12;
      pymisc.bthb+= 12;
      message("You feel like a HERO!");
      need_print_stats |= STATS_QUO;
    }
    pyflags.hero--;
    if (pyflags.hero == 0) {
      pyflags.status &= ~PY_HERO;
      disturb (false, false);
      pymisc.mhp -= 10;
      if (pymisc.chp > pymisc.mhp) {
	pymisc.chp = pymisc.mhp;
	pymisc.chp_frac = 0;
	// need_print_stats |= STATS_QUO;// omit because we do it below
      }
      pymisc.bth  -= 12;
      pymisc.bthb -= 12;
      message("The heroism wears off.");
      need_print_stats |= STATS_QUO;
    }
  } /* end Heroism                  */

  /* Super Heroism */
  if (pyflags.shero > 0) {
    if ((PY_SHERO & pyflags.status) == 0) {
      pyflags.status |= PY_SHERO;
      disturb (false, false);
      pymisc.mhp += 20;
      pymisc.chp += 20;
      pymisc.bth += 24;
      pymisc.bthb+= 24;
      message("You feel like a SUPER HERO!");
      need_print_stats |= STATS_QUO;
    }
    pyflags.shero--;
    if (pyflags.shero == 0) {
      pyflags.status &= ~PY_SHERO;
      disturb (false, false);
      pymisc.mhp -= 20;
      if (pymisc.chp > pymisc.mhp) {
	pymisc.chp = pymisc.mhp;
	pymisc.chp_frac = 0;
	// need_print_stats |= STATS_QUO; // omit, we do it below
      }
      pymisc.bth  -= 24;
      pymisc.bthb -= 24;
      message("The super heroism wears off.");
      need_print_stats |= STATS_QUO;
    }
  } /* end Super Heroism              */

  /* Check food status	       */
  regen_amount = PLAYER_REGEN_NORMAL;
  if (pyflags.food < PLAYER_FOOD_ALERT) {
    if (pyflags.food < PLAYER_FOOD_WEAK) {
      if (pyflags.food < 0)
	regen_amount = 0;
      else if (pyflags.food < PLAYER_FOOD_FAINT)
	regen_amount = PLAYER_REGEN_FAINT;
      else if (pyflags.food < PLAYER_FOOD_WEAK)
	regen_amount = PLAYER_REGEN_WEAK;
      if ((PY_WEAK & pyflags.status) == 0) {
	pyflags.status |= PY_WEAK;
	message("You are getting weak from hunger.");
	disturb (false, false);
	//	prt_hunger(); // XXXXX not implemented
      }
      if ((pyflags.food < PLAYER_FOOD_FAINT) && (randint(8) == 1)) {
	pyflags.paralysis += randint(5);
	message("You faint from the lack of food.");
	disturb (true, false);
      }
    } else if ((PY_HUNGRY & pyflags.status) == 0) {
      pyflags.status |= PY_HUNGRY;
      message("You are getting hungry.");
      disturb (false, false);
      //      prt_hunger(); // XXXXXXX not implemented
    }
  } /* end of Check food status          */

  /* Food consumption
   * Note: Speeded up characters really burn up the food!  */
  if (pyflags.speed < 0)
    pyflags.food -= pyflags.speed * pyflags.speed;
  pyflags.food -= pyflags.food_digested;
  if (pyflags.food < 0) {
    take_hit (-pyflags.food/16, "starvation");   /* -CJS- */
    // XXXX can you die here?
    disturb(true, false);
  } /* end Food consumption */

  /* Regenerate	       */
  if (pyflags.regenerate)  regen_amount *= 3 / 2;
  if ((pyflags.status & PY_SEARCH) || pyflags.rest != 0)
    regen_amount *= 2;
  if ((pyflags.poisoned < 1) && (pymisc.chp < pymisc.mhp))
    regenhp(regen_amount);
  if (pymisc.cmana < pymisc.mana)
    regenmana(regen_amount);
  /* end Regenerate */

  /* Blindness	       */
  if (pyflags.blind > 0) {
    if ((PY_BLIND & pyflags.status) == 0) {
      pyflags.status |= PY_BLIND;
      //      prt_map(); // XXXX not implemented
      //      prt_blind(); // XXXX not implemented
      disturb (false, true);
      /* unlight creatures */
      creatures (false);
    }
    pyflags.blind--;
    if (pyflags.blind == 0) {
      pyflags.status &= ~PY_BLIND;
      //      prt_blind(); // XXXX not implemented
      //      prt_map(); // XXXX not implemented
      /* light creatures */
      disturb (false, true);
      creatures(false);
      message("The veil of darkness lifts.");
    }
  } /* end Blindness */

  /* Confusion	       */
  if (pyflags.confused > 0) {
    if ((PY_CONFUSED & pyflags.status) == 0) {
      pyflags.status |= PY_CONFUSED;
      //      prt_confused(); // XXXX not implemented
    }
    pyflags.confused--;
    if (pyflags.confused == 0) {
      pyflags.status &= ~PY_CONFUSED;
      //      prt_confused(); // XXXX not implemented
      message("You feel less confused now.");
      if (pyflags.rest != 0)
	rest_off ();
    }
  } /* end Confusion */

  /* Afraid		       */
  if (pyflags.afraid > 0) {
    if ((PY_FEAR & pyflags.status) == 0) {
      if ((pyflags.shero + pyflags.hero) > 0)
	pyflags.afraid = 0;
      else {
	pyflags.status |= PY_FEAR;
	//	prt_afraid(); // XXXX not implemented
      }
    } else if ((pyflags.shero + pyflags.hero) > 0)
      pyflags.afraid = 1;
    pyflags.afraid--;
    if (pyflags.afraid == 0) {
      pyflags.status &= ~PY_FEAR;
      //      prt_afraid(); // XXXX not implemented
      message("You feel bolder now.");
      disturb (false, false);
    }
  } /* end Afraid */

  /* Poisoned	       */
  if (pyflags.poisoned > 0) {
    if ((PY_POISONED & pyflags.status) == 0) {
      pyflags.status |= PY_POISONED;
      //      prt_poisoned(); // XXXX not implemented
    }
    pyflags.poisoned--;
    if (pyflags.poisoned == 0) {
      pyflags.status &= ~PY_POISONED;
      //      prt_poisoned(); // XXXX not implemented
      message("You feel better.");
      disturb (false, false);
    } else {
      i = 0;
      switch(con_adj()) {
      case -4:  i = 4;  break;
      case -3:
      case -2:  i = 3;  break;
      case -1:  i = 2;  break;
      case 0:	  i = 1;  break;
      case 1: case 2: case 3:
	i = ((turn % 2) == 0);
	break;
      case 4: case 5:
	i = ((turn % 3) == 0);
	break;
      case 6:
	i = ((turn % 4) == 0);
	break;
      }
      take_hit (i, "poison");
      // XXX can you die here?
      disturb (true, false);
    }
  } /* end Poisoned */

  /* Fast		       */
  if (pyflags.fast > 0) {
    if ((PY_FAST & pyflags.status) == 0) {
      pyflags.status |= PY_FAST;
      change_speed(-1);
      message("You feel yourself moving faster.");
      disturb (false, false);
    }
    pyflags.fast--;
    if (pyflags.fast == 0) {
      pyflags.status &= ~PY_FAST;
      change_speed(1);
      message("You feel yourself slow down.");
      disturb (false, false);
    }
  } /* end Fast */

  /* Slow		       */
  if (pyflags.slow > 0) {
    if ((PY_SLOW & pyflags.status) == 0) {
      pyflags.status |= PY_SLOW;
      change_speed(1);
      message("You feel yourself moving slower.");
      disturb (false, false);
    }
    pyflags.slow--;
    if (pyflags.slow == 0) {
      pyflags.status &= ~PY_SLOW;
      change_speed(-1);
      message("You feel yourself speed up.");
      disturb (false, false);
    }
  } /* end Slow */

  /* Resting is over?      */
  if (pyflags.rest > 0) {
    pyflags.rest--;
    if (pyflags.rest == 0)		      /* Resting over	       */
      rest_off();
  } else if (pyflags.rest < 0) {
    /* Rest until reach max mana and max hit points.  */
    pyflags.rest++;
    if ((pymisc.chp == pymisc.mhp && pymisc.cmana == pymisc.mana) ||
	pyflags.rest == 0)
      rest_off();
  }

  /* Check for interrupts to find or rest. */
  /*
   * XXXXXXXXXX  I do not think that I can do that under PalmOS. 
   * So I have skipped that part (i.e. up to Hallucinating)
   * Hope this does not tend to unbalance gameplay.
   */

  /* Hallucinating?	 (Random characters appear!)*/
  if (pyflags.image > 0) {
    end_find ();
    pyflags.image--;
    if (pyflags.image == 0)
      ;// prt_map ();	 /* Used to draw entire screen! -CJS- */ // XXXX !impl
  } /* end Hallucinating */

  /* Paralysis	       */
  if (pyflags.paralysis > 0) {
    /* when paralysis true, you can not see any movement that occurs */
    pyflags.paralysis--;
    disturb (true, false);
  } /* end Paralysis */

  /* Protection from evil counter*/
  if (pyflags.protevil > 0) {
    pyflags.protevil--;
    if (pyflags.protevil == 0)
      message ("You no longer feel safe from evil.");
  } /* end Protection from evil */

  /* Invulnerability	*/
  if (pyflags.invuln > 0) {
    if ((PY_INVULN & pyflags.status) == 0) {
      pyflags.status |= PY_INVULN;
      disturb (false, false);
      pymisc.pac += 100;
      pymisc.dis_ac += 100;
      //      prt_pac(); // XXXX
      need_print_stats |= STATS_GOLD_AC;
      message("Your skin turns into steel!");
    }
    pyflags.invuln--;
    if (pyflags.invuln == 0) {
      pyflags.status &= ~PY_INVULN;
      disturb (false, false);
      pymisc.pac -= 100;
      pymisc.dis_ac -= 100;
      //      prt_pac(); // XXX
      need_print_stats |= STATS_GOLD_AC;
      message("Your skin returns to normal.");
    }
  } /* end Invulnerability */

  /* Blessed       */
  if (pyflags.blessed > 0) {
    if ((PY_BLESSED & pyflags.status) == 0) {
      pyflags.status |= PY_BLESSED;
      disturb (false, false);
      pymisc.bth += 5;
      pymisc.bthb+= 5;
      pymisc.pac += 2;
      pymisc.dis_ac+= 2;
      message("You feel righteous!");
      //      prt_pac(); // XXXX
      need_print_stats |= STATS_GOLD_AC;
    }
    pyflags.blessed--;
    if (pyflags.blessed == 0) {
      pyflags.status &= ~PY_BLESSED;
      disturb (false, false);
      pymisc.bth -= 5;
      pymisc.bthb-= 5;
      pymisc.pac -= 2;
      pymisc.dis_ac -= 2;
      message("The prayer has expired.");
      //      prt_pac(); // XXXX
      need_print_stats |= STATS_GOLD_AC;
    }
  } /* end Blessed */

  /* Resist Heat   */
  if (pyflags.resist_heat > 0) {
    pyflags.resist_heat--;
    if (pyflags.resist_heat == 0)
      message ("You no longer feel safe from flame.");
  } /* end Resist heat */

  /* Resist Cold   */
  if (pyflags.resist_cold > 0) {
    pyflags.resist_cold--;
    if (pyflags.resist_cold == 0)
      message ("You no longer feel safe from cold.");
  } /* end Resist cold */

  /* Detect Invisible      */
  if (pyflags.detect_inv > 0) {
    if ((PY_DET_INV & pyflags.status) == 0) {
      pyflags.status |= PY_DET_INV;
      pyflags.see_inv = true;
      /* light but don't move creatures */
      creatures (false);
    }
    pyflags.detect_inv--;
    if (pyflags.detect_inv == 0) {
      pyflags.status &= ~PY_DET_INV;
      /* may still be able to see_inv if wearing magic item */
      calc_bonuses();
      /* unlight but don't move creatures */
      creatures (false);
    }
  } /* end Detect invisible */

  /* Timed infra-vision    */
  if (pyflags.tim_infra > 0) {
    if ((PY_TIM_INFRA & pyflags.status) == 0) {
      pyflags.status |= PY_TIM_INFRA;
      pyflags.see_infra++;
      /* light but don't move creatures */
      creatures (false);
    }
    pyflags.tim_infra--;
    if (pyflags.tim_infra == 0) {
      pyflags.status &= ~PY_TIM_INFRA;
      pyflags.see_infra--;
      /* unlight but don't move creatures */
      creatures (false);
    }
  } /* end Timed infra-vision */

  /* Word-of-Recall  Note: Word-of-Recall is a delayed action	 */
  if (pyflags.word_recall > 0) {
    if (pyflags.word_recall == 1) {
      new_level_flag = true;
      pyflags.paralysis++;
      pyflags.word_recall = 0;
      if (dun_level > 0) {
	dun_level = 0;
	message("You feel yourself yanked upwards!");
      } else if (pymisc.max_dlv != 0) {
	dun_level = pymisc.max_dlv;
	message("You feel yourself yanked downwards!");
      }
    } else
      pyflags.word_recall--;
  } /* end Word-of-recall */

  /* Random teleportation  */
  if ((pyflags.teleport) && (randint(100) == 1)) {
    disturb (false, false);
    teleport(40);
  } /* end Random teleportation */

  /* end of Update counters and messages (told you it was long)     */  
  /* ok there's not much more stuff left to do... */

  /* See if we are too weak to handle the weapon or pack.  -CJS- */
  if (pyflags.status & PY_STR_WGT)
    check_strength();

  /* Decide whether to print status-words on the screen. */
  /*  // XXXXXXXX This needs to be revised for the tiny PalmOS screen
  if (pyflags.status & PY_STUDY)
    prt_study();
  if (pyflags.status & PY_SPEED) {
    pyflags.status &= ~PY_SPEED;
    prt_speed();
  }
  if ((pyflags.status & PY_PARALYSED) && (pyflags.paralysis < 1)) {
    prt_state();
    pyflags.status &= ~PY_PARALYSED;
  }
  else if (pyflags.paralysis > 0) {
    prt_state();
    pyflags.status |= PY_PARALYSED;
  }
  else if (pyflags.rest != 0)
    prt_state();
  
  if ((pyflags.status & PY_ARMOR) != 0) {
    //    prt_pac();
    need_print_stats |= STATS_GOLD_AC;
    pyflags.status &= ~PY_ARMOR;
  }
  if ((pyflags.status & PY_STATS) != 0) {
    for (i = 0; i < 6; i++)
      if ((PY_STR << i) & pyflags.status)
	// prt_stat(i);
        need_print_stats |= STATS_STR_ETC;
    pyflags.status &= ~PY_STATS;
  }
  if (pyflags.status & PY_HP) {
    //    prt_mhp();
    //    prt_chp();
    need_print_stats |= STATS_QUO;
    pyflags.status &= ~PY_HP;
  }
  if (pyflags.status & PY_MANA) {
    // prt_cmana();
    need_print_stats |= STATS_QUO;
    pyflags.status &= ~PY_MANA;
  }
  */
  /* End of printing status type stuff. */

  /* Allow for a slim chance of detect enchantment -CJS- */
  /* for 1st level char, check once every 2160 turns
     for 40th level char, check once every 416 turns */
  /* /// XXXXX Disabled pending me checking on add_inscribe.
  if ( ((turn & 0xF) == 0) &&
       (pyflags.confused == 0) &&
       (randint((Short)(10 + 750 / (5 + pymisc.lev))) == 1)) {

    Char tmp_str[80];
    
    for (i = 0; i < INVEN_ARRAY_SIZE; i++) {
      if (i == inven_ctr)
	i = 22;
      i_ptr = &inventory[i];
      // if in inventory, succeed 1 out of 50 times,
      // if in equipment list, success 1 out of 10 times
      if ( (i_ptr->tval != TV_NOTHING) &&
	   enchanted(i_ptr) &&
	   (randint(i < 22 ? 50 : 10) == 1)) {
	//	extern Char *describe_use();
	    
	StrPrintF(tmp_str,
		  "There's something about what you are %s...",
		  describe_use(i));
	disturb(false, false);
	message(tmp_str);
	add_inscribe(i_ptr, ID_MAGIK);
      }
    }
  }
  */

  /* Check the state of the monster list, and delete some monsters if
     the monster list is nearly full.  This helps to avoid problems in
     creature.c when monsters try to multiply.  Compact_monsters() is
     much more likely to succeed if called from here, than if called
     from within creatures().  */
  /* // XXXXXX Disabled in PalmOS for now although maybe it's a good idea.
  if (MAX_MALLOC - mfptr < 10)
    compact_monsters();
  */

  // OK - We have now reached a comment that says "Accept a command?"

  // Note that we're not supposed to accept a command if:
  // pyflags.paralysis >= 1   or
  // pyflags.rest != 0        or
  // death
  // The MainForm event handler will have to DEAL WITH THAT.
  // currently IT DOES NOT.
  /* if paralyzed, resting, or dead, flush output */
  /* but first move the cursor onto the player, for aesthetics */

  do_pickup = true;
  free_turn_flag = false;
  //  refresh();
}

void end_of_turn()
{
  /* Teleport?		       */
  if (teleport_flag)  teleport(100);
  /* Move the creatures	       */
  if (!new_level_flag)
    creatures(true);
  else {
    // this used to be taken care of by exiting 'while' when (new_level_flag).
    generate_cave();
    enter_new_level();
  }
}

// a c d e f i m o p s t x z ' ' G R S < > ^

/*      commands:        NOT commands:
 *   a  aim/fire wand        browse: inv
 *   c  close door           drop: inv
 *   e  equip list           look: menu
 *   t  fire/throw           quaff: inv
 *   i  inventory            read scroll: inv
 *   S# jam/spike door       take off: inv
 *   m  magic spell          version: menu
 *   o# open door/chest      wear,wield: inv
 *   p  pray                 your stats: menu
 *   s# search for trap/door eat: inv
 *   z  zap a staff          fill: inv
 *   x  exchange weapons     locate-on-map: menu
 *   <  ascend               map: menu
 *   >  descend              search mode: menu?
 *   f# force/bash           view scores: menu?
 *   ^# disarm trap/chest    set options: menu
 *   G  gain spells          help: menu
 *   R# rest                 inscribe: ????
 *   d# dig tunnel           prev msg: menu
 *    # move w/o pickup ' '  /identify char: menu   */
// I think for now I will ignore the command_count nature of MOVE and REST.
// but I will try for S, o, s, f, ^, and d
// A note on things that take a command_count:
// The various you-moving and monster_moving parts of the turn can
// 'disturb' your repetition and set command_count to 0.
// in fact, anything that prints a message will set command_count to 0
// (unless it calls count_message instead.)
// So you may end the repetition prematurely.  Make sure this works ok.
Char last_command = -1;
Boolean do_command(Char com_val)
{
  Boolean handled = true;
  Short dir = map_roguedir(com_val); // dir==5 if noop or non-direction.

  free_turn_flag = false; // by default
  if (last_command != -1) { // we wanted to do a command that needed direction
    if (dir == 5 && last_command != '?') // 'aborted' unless 'look'
      free_turn_flag = true;
    else
      do_dir_command(dir, 0);
    last_command = -1;
    return true;
  }

  last_command = -1;
  /* hack for move without pickup.  Map ' ' to a movement command. */
  if (com_val == ' ') {
    do_pickup = false;
    // XXXXX Remember command_count; Draw a get-direction pie and move.
    // This turn should be a freebie.
    free_turn_flag = true;
    return true;
  }

  switch(com_val) {
    /*
  case 'Q':	// (Q)uit		(^K)ill
    // Popup a confirmation.
    if (get_check("Do you really want to quit?")) {
      new_level_flag = true;
      death = true;
      StrCopy(died_from, "Quitting");
    }
    free_turn_flag = true;
    break;
  case CTRL('X'):	// e(^X)it and save
    if (total_winner) {
      message("You are a Total Winner,  your character must be retired.");
      message("Use /Q to when you are ready to quit.");
    } else {
      StrCopy (died_from, "(saved)");
      message ("Saving game...");
      if (save_char ())
	exit_game();
      StrCopy (died_from, "(alive and well)");
    }
    free_turn_flag = true;
    break;
    */
  case '{':		/* ({) inscribe an object    */
    scribe_object ();
    free_turn_flag = true;
    break;
  case 'b':		/* (b) down, left	(1) */
  case 'j':		/* (j) down		(2) */
  case 'n':		/* (n) down, right	(3) */
  case 'h':		/* (h) left		(4) */
  case 'l':		/* (l) right		(6) */
  case 'y':		/* (y) up, left		(7) */
  case 'k':		/* (k) up		(8) */
  case 'u':		/* (u) up, right	(9) */
    move_char(dir, do_pickup);
    break;
  case 'B':		/* (B) run down, left	(. 1) */
  case 'J':		/* (J) run down		(. 2) */
  case 'N':		/* (N) run down, right	(. 3) */
  case 'H':		/* (H) run left		(. 4) */
  case 'L':		/* (L) run right	(. 6) */
  case 'Y':		/* (Y) run up, left	(. 7) */
  case 'K':		/* (K) run up		(. 8) */
  case 'U':		/* (U) run up, right	(. 9) */
    find_init(dir);
    break;
  case '.':		/* (.) stay in one place (5) */
    move_char (5, do_pickup);
    if (command_count > 1) {
      command_count--;
      rest(); // #
    }
    break;

    /* I am not sanguine about these - might work with keyboard though.
  case CTRL('B'):		// (^B) tunnel down left	(T 1) 
    tunnel(1);
    break;
  case CTRL('M'):		// cr must be treated same as lf. 
  case CTRL('J'):		// (^J) tunnel down		(T 2) 
    tunnel(2);
    break;
  case CTRL('N'):		// (^N) tunnel down right	(T 3) 
    tunnel(3);
    break;
  case CTRL('H'):		// (^H) tunnel left		(T 4) 
    tunnel(4);
    break;
  case CTRL('L'):		// (^L) tunnel right		(T 6) 
    tunnel(6);
    break;
  case CTRL('Y'):		// (^Y) tunnel up left		(T 7) 
    tunnel(7);
    break;
  case CTRL('K'):		// (^K) tunnel up		(T 8) 
    tunnel(8);
    break;
  case CTRL('U'):		// (^U) tunnel up right		(T 9) 
    tunnel(9);
    break;
    */

  case '<':		/* (<) go down a staircase */
    go_up();
    break;
  case '>':		/* (>) go up a staircase */
    go_down();
    break;
  case 'G':		/* (G)ain magic spells */
    gain_spells();
    break;
  case 'r':		/* (R)est a while */
    rest(); // #
    break;
  case '#':		/* (#) search toggle	(S)earch toggle */
    if (pyflags.status & PY_SEARCH)
      search_off();
    else
      search_on();
    free_turn_flag = true;
    break;
  case 'e':		/* (e)quipment list */
    free_turn_flag = true;
    if (equip_ctr == 0)
      message("You are not using anything.");
    else
      FrmPopupForm(EquForm);
    break;
  case 'i':		/* (i)nventory list */
    free_turn_flag = true;
    if (inven_ctr == 0)
      message("You are not carrying anything.");
    else
      FrmPopupForm(InvForm);
    break;
  case 'm':		/* (m)agic spells */
    cast();
    break;
  case 'p':		/* (p)ray */
    pray();
    break;
  case 'x':		/* e(X)change weapons	e(x)change */
  case 'X':		/* e(X)change weapons	e(x)change */
    exchange_weap();
    break;
  case 's':		/* (s)earch for a turn */
    // This can take a multiplier in command_count.
    do {
      search(char_row, char_col, pymisc.srh); // moria2.c
      end_of_turn();
      if (command_count > 0) command_count--;
      start_of_turn();
    } while (command_count > 0);
    free_turn_flag = true; // 'cause we just did end_of and start_of.
    break;

  case 'f':		/* (f)orce		(B)ash */
  case '^':		/* (D)isarm trap */
  case 'a':		/* (a)im a wand		(a)im a wand */
  case 'c':		/* (c)lose an object */
  case 't':		/* (t)hrow something	(f)ire something */
  case 'S':		/* (S)pike a door	(j)am a door */
  case 'o':		/* (o)pen something */
  case 'd':		/* (d)ig / tunnel */
  case 'z':		/* (z)ap a staff	(u)se a staff */
  case '?':		/* look/examine */
  case '-':		/* slide */
    // draw that funky pie.....  UNLESS it's '/'
    draw_directional();
    last_command = com_val;
    free_turn_flag = true;
    break;
  case '/':		// (/) identify a symbol
    message("Click on a symbol to identify it.");
    last_command = com_val;
    free_turn_flag = true;
    break;
  default:
    //message("Type '?' for help.");
    handled = false;
    free_turn_flag = true;
  }
  do_pickup = true;
  return handled;
}

void do_xy_command(Short x, Short y)
{
  UChar monptr;
  //  Char buf[40];
  if (last_command == -1) return;
  switch(last_command) {
  case '/':		// (/) identify a symbol
    free_turn_flag = true;
    // If there's a monster there,
    // set recall_this_monster to some index of c_list.
    // Otherwise do ident_char().
    monptr = cave_cptr[y * MAX_WIDTH + x];
    if (monptr > 1) {
      //      recall_this_monster = m_list[monptr].mptr;
      recall_this_monster = m_list[monptr].mptr;
      if (recall_this_monster < 0 || recall_this_monster >= MAX_CREATURES) {
	message("m_list[].mptr assertion failed"); // this better not happen
	break;
      }
      FrmPopupForm(RecallForm);
    } else
      ident_char((Char) loc_symbol(y,x));
    break;
  }
  last_command = -1;
}

void do_dir_command(Short dir, Short XXX_item)
{
  /*Short XXX_item=0; / * This is a tmp argument.  Really these things need
		       to be setting a global variable, which you select
		       from some form or other when we do Z, z, or t. */
  if (last_command == -1) return;
  // ... Need to undraw that funky pie....
  switch(last_command) {

  case 'f':		/* (f)orce		(B)ash */
    // This can take a multiplier in command_count.
    do {
      bash(dir); // moria4.c
      end_of_turn();
      if (command_count > 0) command_count--;
      start_of_turn();
    } while (command_count > 0);
    free_turn_flag = true; // 'cause we just did end_of and start_of.
    break;
  case '^':		/* (D)isarm trap */
    // This can take a multiplier in command_count.
    do {
      disarm_trap(dir); // moria4.c
      end_of_turn();
      if (command_count > 0) command_count--;
      start_of_turn();
    } while (command_count > 0);
    free_turn_flag = true; // 'cause we just did end_of and start_of.
    break;
  case 'c':		/* (c)lose an object */
    closeobject(dir);
    break;
  case 'S':		/* (S)pike a door	(j)am a door */
    // This can take a multiplier in command_count.
    do {
      jamdoor(dir); // dungeon.c
      end_of_turn();
      if (command_count > 0) command_count--;
      start_of_turn();
    } while (command_count > 0);
    free_turn_flag = true; // 'cause we just did end_of and start_of.
    break;
  case 'o':		/* (o)pen something */
    // This can take a multiplier in command_count.
    do {
      openobject(dir); // moria3.c
      end_of_turn();
      if (command_count > 0) command_count--;
      start_of_turn();
    } while (command_count > 0);
    free_turn_flag = true; // 'cause we just did end_of and start_of.
    break;
  case 'd':		/* (o)pen something */
    // This can take a multiplier in command_count.
    do {
      tunnel(dir); // moria4.c
      end_of_turn();
      if (command_count > 0) command_count--;
      start_of_turn();
    } while (command_count > 0);
    free_turn_flag = true; // 'cause we just did end_of and start_of.
    break;
  case '?':		/* look or examine */
    look(dir);
    free_turn_flag = true;
    break;
  case '-':
    slide_dir(dir);
    free_turn_flag = true;
    break;

  case 'a':		/* (z)ap a wand		(a)im a wand */
    aim(XXX_item, dir);
    break;
  case 'z':		/* (Z)ap a staff	(u)se a staff */
    use(XXX_item);
    break;
  case 't':		/* (t)hrow something	(f)ire something */
    throw_object(XXX_item, dir);
    break;
  default:
    {
      Char buf[20];
      StrPrintF(buf, "Unknown command '%c'.", last_command);
      message(buf);
      free_turn_flag = true;
    }
  }
  command_count = 0; // seems like a good idea
  last_command = -1;
}

/* Check whether this command will accept a count.     -CJS-  */
static Boolean valid_countcommand(Char c)
{
  switch(c) {
    // Quit, wizard, exit, set options, inscribe,
    //  case 'Q':  case CTRL('W'):  case CTRL('X'):  case '=':  case '{':
    // whatsit, help, your-descrip, Eat, Fill-lamp
    //  case '/':  case '?':  case 'C':  case 'E':  case 'F':
    // View-scores, search-toggle, Peruse/browse, drop, examine/look
    //  case 'V':  case '#':  case 'P':  case 'd':  case 'x':
    // quaff, read-scroll, Take-off, version, wear-wield, Where-on-map
    //  case 'q':  case 'r':  case 'T':  case 'v':  case 'w':  case 'W':
    // wizard stuff....
    //  case CTRL('A'): case '\\': case CTRL('I'): case '*': case ':':
    //  case CTRL('T'): case CTRL('E'): case CTRL('F'): case CTRL('S'): //?
    //  case CTRL('Q'): //?

    /*
     * THESE THINGS DO NOT TAKE A COUNT:
     * ascend, descend, Gain-spell, zap-wand
     * close, equip-list, throw/fire, inv-list
     * magic, pray, Zap-staff, Xchange
     */
  case '<':  case '>':  case 'G':  case 'z':
  case 'c':  case 'e':  case 't':  case 'i':
  case 'm':  case 'p':  case 'Z':  case 'X':
    return false;

    // prev message, noop, old move-wo-pickup
    //  case CTRL('P'):  case ESCAPE:   case '-':
    // wizard foo
    //  case CTRL('D'):  case CTRL('G'):  case '+':

    /*
     * THESE THINGS TAKE A COUNT:
     * move-w/o-pickup, force, <movement>, stay-here,
     * disarm, rest, spike, open, search
     */
  case ' ':  case 'f':    // move-w/o-pickup, force
  case 'b': case 'n': case 'y': case 'u':     // these are all movement...
  case 'h': case 'j': case 'k': case 'l':
  case 'B': case 'N': case 'Y': case 'U':
  case 'H': case 'J': case 'K': case 'L':
  case CTRL('Y'): case CTRL('U'): case CTRL('B'): case CTRL('N'):
  case CTRL('H'): case CTRL('J'): case CTRL('K'): case CTRL('L'):
  case '.':     // stay in one place
  case '^':    // disarm trap was 'D', now '^'
  case 'r':    // rest was 'R', now 'r'
  case 'S':    // spike/jam a door
  case 'o':    // open door/chest
  case 's':    // search
    return true;

    /*
     * Anything I forgot does not take a count.
     * (Also, anything accessible SOLELY from menu, inv, & equip should not.)
     */
  default:
    return false;
  }
}


static void regenhp(Short percent)
{
  Long new_chp, new_chp_frac;
  Short old_chp;

  old_chp = pymisc.chp;
  new_chp = ((Long)pymisc.mhp) * percent + PLAYER_REGEN_HPBASE;
  pymisc.chp += new_chp >> 16;	/* div 65536 */
  /* check for overflow */
  if (pymisc.chp < 0 && old_chp > 0)
    pymisc.chp = MAX_SHORT;
  new_chp_frac = (new_chp & 0xFFFF) + pymisc.chp_frac; /* mod 65536 */
  if (new_chp_frac >= 0x10000L) {
    pymisc.chp_frac = new_chp_frac - 0x10000L;
    pymisc.chp++;
  } else
    pymisc.chp_frac = new_chp_frac;

  /* must set frac to zero even if equal */
  if (pymisc.chp >= pymisc.mhp) {
    pymisc.chp = pymisc.mhp;
    pymisc.chp_frac = 0;
  }
  if (old_chp != pymisc.chp)
  //    prt_chp();
    need_print_stats |= STATS_QUO;
}

static void regenmana(Short percent)
{
  Long new_mana, new_mana_frac;
  Short old_cmana;

  old_cmana = pymisc.cmana;
  new_mana = ((Long)pymisc.mana) * percent + PLAYER_REGEN_MNBASE;
  pymisc.cmana += new_mana >> 16;  /* div 65536 */
  /* check for overflow */
  if (pymisc.cmana < 0 && old_cmana > 0)
    pymisc.cmana = MAX_SHORT;
  new_mana_frac = (new_mana & 0xFFFF) + pymisc.cmana_frac; /* mod 65536 */
  if (new_mana_frac >= 0x10000L) {
    pymisc.cmana_frac = new_mana_frac - 0x10000L;
    pymisc.cmana++;
  } else
    pymisc.cmana_frac = new_mana_frac;

  /* must set frac to zero even if equal */
  if (pymisc.cmana >= pymisc.mana) {
    pymisc.cmana = pymisc.mana;
    pymisc.cmana_frac = 0;
  }
  if (old_cmana != pymisc.cmana)
  //    prt_cmana();
    need_print_stats |= STATS_QUO;
}


/* Is an item an enchanted weapon or armor and we don't know?  -CJS- */
/* only returns true if it is a good enchantment */
static Boolean enchanted (inven_type *t_ptr)
{
  if (t_ptr->tval < TV_MIN_ENCHANT ||
      t_ptr->tval > TV_MAX_ENCHANT ||
      t_ptr->flags & TR_CURSED)
    return false;
  if (known2_p(t_ptr))
    return false;
  if (t_ptr->ident & ID_MAGIK)
    return false;
  if (t_ptr->tohit > 0 || t_ptr->todam > 0 || t_ptr->toac > 0)
    return true;
  if ((0x4000107fL & t_ptr->flags) && t_ptr->p1 > 0)
    return true;
  if (0x07ffe980L & t_ptr->flags)
    return true;

  return false;
}

// Note that examine_book is complete and has been parceled out to form_blah fu


/* Go up one level					-RAK-	*/
static void go_up()
{
  UChar thing;

  thing = cave_tptr[char_row*MAX_WIDTH+char_col];
  if (thing != 0) {
    if (t_list[thing].tval == TV_UP_STAIR) {
      dun_level--;
      new_level_flag = true;
      message("You enter a maze of up staircases.");
      message("You pass through a one-way door.");
      // end_of_turn will call generate_cave and enter_new_level.
      free_turn_flag = false;
      return;
    }
  }
  message("I see no up staircase here.");
  free_turn_flag = true;
}

/* Go down one level					-RAK-	*/
static void go_down()
{
  UChar thing;

  thing = cave_tptr[char_row*MAX_WIDTH+char_col];
  if (thing != 0) {
    if (t_list[thing].tval == TV_DOWN_STAIR) {
      dun_level++;
      new_level_flag = true;
      message("You enter a maze of down staircases.");
      message("You pass through a one-way door.");
      free_turn_flag = false;
      return;
    }
  }
  message("I see no down staircase here.");
  free_turn_flag = true;
}

// missing: jamdoor()
// This involves destroying stuff from inventory so I will do it "later"
// i.e. "not tonight"
/* Jam a closed door					-RAK-	*/
// I now assume that we ALREADY HAVE A DIRECTION given.
static void jamdoor(Short dir)
{
  Short y, x, i, j, spikeness;
  inven_type *t_ptr, *i_ptr;
  Char tmp_str[80];
  UChar thing, critter;
  Boolean got_spikes;

  free_turn_flag = true;
  y = char_row;
  x = char_col;

  mmove(dir, &y, &x); // given direction calculate new row,col.

  // Make sure there is something there
  thing = cave_tptr[y*MAX_WIDTH+x];
  critter = cave_cptr[y*MAX_WIDTH+x];
  if (thing == 0) {
    message("That isn't a door!");
    return;
  }
  // Make sure the thing is a closed door
  t_ptr = &t_list[thing];
  if (t_ptr->tval != TV_CLOSED_DOOR) {
    if (t_ptr->tval == TV_OPEN_DOOR)
      message("The door must be closed first.");
    else
      message("That isn't a door!");
    return;
  }
  // Make sure no monsters in the way
  if (critter != 0) {
    free_turn_flag = false;
    StrPrintF(tmp_str, "The %s is in your way!",
	      c_names + c_list[m_list[critter].mptr].name_offset);
    message(tmp_str);
    return;
  }
  // Find your spikes.  this sets i which we use below.
  got_spikes = find_range(TV_SPIKE, TV_NEVER, &i, &j);
  if (!got_spikes) {
    message("But you have no spikes.");
    return;
  }
  // Make the door jammed.
  free_turn_flag = false;
  count_message("You jam the door with a spike."); // XXXXX not impl yet
  spikeness = t_ptr->p1;
  if (spikeness > 0) spikeness = -spikeness; /* Make door 'locked to' stuck. */
  /* Successive spikes have a progressively smaller effect.
     Series is: 0 20 30 37 43 48 52 56 60 64 67 70 ... */
  spikeness -= 1 + 190 / (10 - spikeness);
  invy_set_p1(t_list, thing, spikeness); // make the door that much spiked.
  // Lose a spike from inventory.
  i_ptr = &inventory[i];
  if (i_ptr->number > 1) { 
    invy_set_number(inventory, i, i_ptr->number - 1); //== "i_ptr->number--"
    inven_weight -= i_ptr->weight;
  } else
    inven_destroy(i);
}




// Note that refill_lamp already complete (?) in form_inv.c



// missing: (earlier)
//    do_command  - this will be near MainForm handler
//    valid_countcommand  - ditto



static void exchange_weap()
{
  Char prt1[80], prt2[160];
  if (inventory[INVEN_WIELD].tval == TV_NOTHING &&
      inventory[INVEN_AUX].tval == TV_NOTHING) {
    message("But you are wielding no weapons.");
  } else if (TR_CURSED & inventory[INVEN_WIELD].flags) {
    objdes(prt1, &inventory[INVEN_WIELD], false);
    StrPrintF(prt2, "The %s you are wielding appears to be cursed.", prt1);
    message(prt2);
  } else {
    free_turn_flag = false;
    invy_swap(INVEN_AUX, INVEN_WIELD);
    //    tmp_obj = inventory[INVEN_AUX];
    //    inventory[INVEN_AUX] = inventory[INVEN_WIELD];
    //    inventory[INVEN_WIELD] = tmp_obj;
    //    if (scr_state == EQUIP_SCR)
    //      scr_left = show_equip(show_weight_flag, scr_left);
    py_bonuses(&inventory[INVEN_AUX], -1);	 /* Subtract bonuses */
    py_bonuses(&inventory[INVEN_WIELD], 1);	   /* Add bonuses    */
    if (inventory[INVEN_WIELD].tval != TV_NOTHING) {
      StrCopy(prt1, "Primary weapon: ");
      objdes(prt2, &inventory[INVEN_WIELD], true);
      message(StrCat(prt1, prt2));
    } else
      message("No primary weapon.");
    /* this is a new weapon, so clear the heavy flag */
    weapon_heavy = false;
    check_strength();
  }
}


/* map rogue_like direction commands into numbers */
static Char map_roguedir(Char comval)
{
  if (comval < 'A' || comval > 'z') return 5; // catch most non-alphabetics
  if (comval <= 'Z') comval += ('a' - 'A');   // make it lowercase!
  switch(comval) {
  case 'h':
    comval = '4';
    break;
  case 'y':
    comval = '7';
    break;
  case 'k':
    comval = '8';
    break;
  case 'u':
    comval = '9';
    break;
  case 'l':
    comval = '6';
    break;
  case 'n':
    comval = '3';
    break;
  case 'j':
    comval = '2';
    break;
  case 'b':
    comval = '1';
    break;
  case '.':  default:
    comval = '5';
    break;
  }
  return(comval);
}

static void ident_char(Char command)
{
  switch(command) {
    /* every printing ASCII character is listed here, in the order in which
       they appear in the ASCII character set */
  case ' ': message("  - An open pit."); break;
  case '!': message("! - A potion."); break;
  case '"': message("\" - An amulet, periapt, or necklace."); break;
  case '#': message("# - A stone wall."); break;
  case '$': message("$ - Treasure."); break;
  case '%':
    if (highlight_seams == true)
      message("% - A magma or quartz vein.");
    else
      message("% - Not used.");
    break;
  case '&': message("& - Treasure chest."); break;
  case '\'': message("' - An open door."); break;
  case '(': message("( - Soft armor."); break;
  case ')': message(") - A shield."); break;
  case '*': message("* - Gems."); break;
  case '+': message("+ - A closed door."); break;
  case ',': message(", - Food or mushroom patch."); break;
  case '-': message("- - A wand"); break;
  case '.': message(". - Floor."); break;
  case '/': message("/ - A pole weapon."); break;
    /* case '0': message("0 - Not used."); break; */
  case '1': message("1 - Entrance to General Store."); break;
  case '2': message("2 - Entrance to Armory."); break;
  case '3': message("3 - Entrance to Weaponsmith."); break;
  case '4': message("4 - Entrance to Temple."); break;
  case '5': message("5 - Entrance to Alchemy shop."); break;
  case '6': message("6 - Entrance to Magic-Users store."); break;
    /* case '7': message("7 - Not used."); break; */
    /* case '8': message("8 - Not used."); break; */
    /* case '9': message("9 - Not used.");  break;*/
  case ':': message(": - Rubble."); break;
  case ';': message("; - A loose rock."); break;
  case '<': message("< - An up staircase."); break;
  case '=': message("= - A ring."); break;
  case '>': message("> - A down staircase."); break;
  case '?': message("? - A scroll."); break;
  case '@': message(pymisc.name); break;
  case 'A': message("A - Giant Ant Lion."); break;
  case 'B': message("B - The Balrog."); break;
  case 'C': message("C - Gelatinous Cube."); break;
  case 'D': message("D - An Ancient Dragon (Beware)."); break;
  case 'E': message("E - Elemental."); break;
  case 'F': message("F - Giant Fly."); break;
  case 'G': message("G - Ghost."); break;
  case 'H': message("H - Hobgoblin."); break;
    /* case 'I': message("I - Invisible Stalker."); break; */
  case 'J': message("J - Jelly."); break;
  case 'K': message("K - Killer Beetle."); break;
  case 'L': message("L - Lich."); break;
  case 'M': message("M - Mummy."); break;
    /* case 'N': message("N - Not used."); break; */
  case 'O': message("O - Ooze."); break;
  case 'P': message("P - Giant humanoid."); break;
  case 'Q': message("Q - Quylthulg (Pulsing Flesh Mound)."); break;
  case 'R': message("R - Reptile."); break;
  case 'S': message("S - Giant Scorpion."); break;
  case 'T': message("T - Troll."); break;
  case 'U': message("U - Umber Hulk."); break;
  case 'V': message("V - Vampire."); break;
  case 'W': message("W - Wight or Wraith."); break;
  case 'X': message("X - Xorn."); break;
  case 'Y': message("Y - Yeti."); break;
    /* case 'Z': message("Z - Not used."); break; */
  case '[': message("[ - Hard armor."); break;
  case '\\': message("\\ - A hafted weapon."); break;
  case ']': message("] - Misc. armor."); break;
  case '^': message("^ - A trap."); break;
  case '_': message("_ - A staff."); break;
    /* case '`': message("` - Not used."); break; */
  case 'a': message("a - Giant Ant."); break;
  case 'b': message("b - Giant Bat."); break;
  case 'c': message("c - Giant Centipede."); break;
  case 'd': message("d - Dragon."); break;
  case 'e': message("e - Floating Eye."); break;
  case 'f': message("f - Giant Frog."); break;
  case 'g': message("g - Golem."); break;
  case 'h': message("h - Harpy."); break;
  case 'i': message("i - Icky Thing."); break;
  case 'j': message("j - Jackal."); break;
  case 'k': message("k - Kobold."); break;
  case 'l': message("l - Giant Louse."); break;
  case 'm': message("m - Mold."); break;
  case 'n': message("n - Naga."); break;
  case 'o': message("o - Orc or Ogre."); break;
  case 'p': message("p - Person (Humanoid)."); break;
  case 'q': message("q - Quasit."); break;
  case 'r': message("r - Rodent."); break;
  case 's': message("s - Skeleton."); break;
  case 't': message("t - Giant Tick."); break;
    /* case 'u': message("u - Not used."); break; */
    /* case 'v': message("v - Not used."); break; */
  case 'w': message("w - Worm or Worm Mass."); break;
    /* case 'x': message("x - Not used."); break; */
  case 'y': message("y - Yeek."); break;
  case 'z': message("z - Zombie."); break;
  case '{': message("{ - Arrow, bolt, or bullet."); break;
  case '|': message("| - A sword or dagger."); break;
  case '}': message("} - Bow, crossbow, or sling."); break;
  case '~': message("~ - Miscellaneous item."); break;
  default:	message("Not Used."); break;
  }

  /* Allow access to monster memory. -CJS- */
  // nah. -sprite
}
