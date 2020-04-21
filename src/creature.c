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

static Short movement_rate(Short) SEC_4;
static Boolean check_mon_lite(Short, Short) SEC_4;
static void get_moves(Short, Short *) SEC_4;
static void make_attack(Short) SEC_4;
static void make_move(Short, Short *, ULong *) SEC_4;
static void mon_cast_spell(Short, Boolean *) SEC_4;
static void mon_move(Short, ULong *) SEC_4;

// XXXX Move monster_attacks out of recall.c ... it's read-only anyway.
extern struct m_attack_type monster_attacks[N_MONS_ATTS];

/* Updates screen when monsters move about		-RAK-	*/
void update_mon(Short monptr)
{
  Boolean flag; // we'll set this to 'true' if we should draw the monster.
  monster_type *m_ptr;
  creature_type *r_ptr;

  m_ptr = &m_list[monptr];
  flag = false;
  // If the monster is close enough to see, and the player isn't blind,
  // and the monster is currently "on screen"...
  //   if you're in wizard mode you always see it,
  //   otherwise it has to be in your line-of-sight, lit, non-invisible, etc.
  if ((m_ptr->cdis <= MAX_SIGHT) &&
      !(pyflags.status & PY_BLIND) &&
      (panel_contains((Short)m_ptr->fy, (Short)m_ptr->fx))) {
    //    if (wizard)
    //      /* Wizard sight.	     */
    //      flag = true;
    //    else
    if (los(char_row, char_col, (Short)m_ptr->fy, (Short)m_ptr->fx)) {
      /* Normal sight.	     */
      r_ptr = &c_list[m_ptr->mptr];
      if ((cave_light[m_ptr->fy*MAX_WIDTH+m_ptr->fx] & (LIGHT_pl | LIGHT_tl))
	  || (find_flag && m_ptr->cdis < 2 && player_light)) {
	if ((CM_INVISIBLE & r_ptr->cmove) == 0)
	  flag = true;
	else if (pyflags.see_inv) {
	  flag = true;
	  recall_cmove_or(m_ptr->mptr, CM_INVISIBLE);
	}
      } else if ((pyflags.see_infra > 0) &&
		 (m_ptr->cdis <= pyflags.see_infra) &&
		 (CD_INFRA & r_ptr->cdefense)) {
	/* Infra vision.	 */
	flag = true;
	recall_cdefense_or(m_ptr->mptr, CD_INFRA);
      }
    }
  }
  // Now... draw the monster, if appropriate
  if (flag) {
    if (!m_ptr->ml) {
      // the monster has just come into view.. shake up the player.
      disturb(true, false);
      mlist_set_ml(monptr, true);
      lite_spot((Short)m_ptr->fy, (Short)m_ptr->fx);
      //screen_change = true; /* notify inven_command */ // Hey, do I use this?
    }
  } else if (m_ptr->ml) {
    // ho hum, old news.. just draw the monster again.
    mlist_set_ml(monptr, false);
    lite_spot((Short)m_ptr->fy, (Short)m_ptr->fx);
    //screen_change = true; /* notify inven_command */
  }
}


/* Given speed,	 returns number of moves this turn.	-RAK-	*/
/* NOTE: Player must always move at least once per iteration,	  */
/*	 a slowed player is handled by moving monsters faster	 */
static Short movement_rate(Short speed)
{
  if (speed > 0) {
    if (pyflags.rest != 0)
      return 1;
    else
      return speed;
  } else {
    /* speed must be negative here */
    return ((turn % (2 - speed)) == 0); // hello, what the heck does this do?
  }
}


/* Makes sure a new creature gets lit up.			-CJS- */
static Boolean check_mon_lite(Short y, Short x)
{
  Short monptr;

  monptr = cave_cptr[y*MAX_WIDTH+x];
  if (monptr <= 1)
    return false;
  else {
    update_mon(monptr);
    return m_list[monptr].ml;
  }
}


/* Choose correct directions for monster movement	-RAK-	*/
/* The argument 'mm' is an array of 9 shorts for the 9 possible moves,
 * although only the first 5 are ever set here or used in make_move.
 * Here they get set to the 5 best moves from best to worst. */
static void get_moves(Short monptr, Short *mm)
{
  Short y, x;    // diff between monster position and player pos.
  Short ay, ax;  // absolute value of x and y, a measure of distance.
  Short move_val; /* use to figure the direction the monster most wants to go:
		     0=ne 1,9=e 8=se 10,11=s 12=sw 5,13=w 4=nw 2,6=n. */

  y = m_list[monptr].fy - char_row;
  x = m_list[monptr].fx - char_col;
  if (y < 0) {
    move_val = 8; // player is southerly.  unless I have that backwards.
    ay = -y;
  } else {
    move_val = 0;
    ay = y;
  }
  if (x > 0) {
    move_val += 4; // player is westerly.
    ax = x;
  } else
    ax = -x;

  /* this has the advantage of preventing the diamond maneuvre, also faster */
  if (ay > (ax << 1))
    move_val += 2;
  else if (ax > (ay << 1))
    move_val++;
  switch(move_val) {
  case 0:
    mm[0] = 9;
    if (ay > ax) {
      mm[1] = 8;
      mm[2] = 6;
      mm[3] = 7;
      mm[4] = 3;
    } else {
      mm[1] = 6;
      mm[2] = 8;
      mm[3] = 3;
      mm[4] = 7;
    }
    break;
  case 1: case 9:
    mm[0] = 6;
    if (y < 0) {
      mm[1] = 3;
      mm[2] = 9;
      mm[3] = 2;
      mm[4] = 8;
    } else {
      mm[1] = 9;
      mm[2] = 3;
      mm[3] = 8;
      mm[4] = 2;
    }
    break;
  case 2: case 6:
    mm[0] = 8;
    if (x < 0) {
      mm[1] = 9;
      mm[2] = 7;
      mm[3] = 6;
      mm[4] = 4;
    } else {
      mm[1] = 7;
      mm[2] = 9;
      mm[3] = 4;
      mm[4] = 6;
    }
    break;
  case 4:
    mm[0] = 7;
    if (ay > ax) {
      mm[1] = 8;
      mm[2] = 4;
      mm[3] = 9;
      mm[4] = 1;
    } else {
      mm[1] = 4;
      mm[2] = 8;
      mm[3] = 1;
      mm[4] = 9;
    }
    break;
  case 5: case 13:
    mm[0] = 4;
    if (y < 0) {
      mm[1] = 1;
      mm[2] = 7;
      mm[3] = 2;
      mm[4] = 8;
    } else {
      mm[1] = 7;
      mm[2] = 1;
      mm[3] = 8;
      mm[4] = 2;
    }
    break;
  case 8:
    mm[0] = 3;
    if (ay > ax) {
      mm[1] = 2;
      mm[2] = 6;
      mm[3] = 1;
      mm[4] = 9;
    } else {
      mm[1] = 6;
      mm[2] = 2;
      mm[3] = 9;
      mm[4] = 1;
    }
    break;
  case 10: case 14:
    mm[0] = 2;
    if (x < 0) {
      mm[1] = 3;
      mm[2] = 1;
      mm[3] = 6;
      mm[4] = 4;
    } else {
      mm[1] = 1;
      mm[2] = 3;
      mm[3] = 4;
      mm[4] = 6;
    }
    break;
  case 12:
    mm[0] = 1;
    if (ay > ax) {
      mm[1] = 2;
      mm[2] = 4;
      mm[3] = 3;
      mm[4] = 7;
    } else {
      mm[1] = 4;
      mm[2] = 2;
      mm[3] = 7;
      mm[4] = 3;
    }
    break;
  }
}


/* Make an attack on the player (chuckle.)		-RAK-	*/
static void make_attack(Short monptr)
{
  Short attype, adesc, adice, asides;
  Short i, j, damage, attackn, notice, visible;
  Boolean flag;
  Long gold;
  UChar *attstr;
  Char cdesc[80], tmp_str[80], ddesc[80];
  creature_type *r_ptr;
  monster_type *m_ptr;
  inven_type *i_ptr;
  Char *monname;

  if (death)  /* don't beat a dead body! */
    return;

  m_ptr = &m_list[monptr];
  r_ptr = &c_list[m_ptr->mptr];
  monname = c_names + r_ptr->name_offset;
  if (!m_ptr->ml)
    StrCopy(cdesc, "It ");
  else
    StrPrintF(cdesc, "The %s ", monname);
  /* For "DIED_FROM" string	   */
  if (CM_WIN & r_ptr->cmove)
    StrPrintF(ddesc, "The %s", monname);  // (THE Balrog)
  else if (is_a_vowel (monname[0]))
    StrPrintF(ddesc, "an %s", monname);
  else
    StrPrintF (ddesc, "a %s", monname);
  /* End DIED_FROM		   */

  attackn = 0;
  attstr = r_ptr->damage;
  while ((*attstr != 0) && !death) {
    attype = monster_attacks[*attstr].attack_type;
    adesc = monster_attacks[*attstr].attack_desc;
    adice = monster_attacks[*attstr].attack_dice;
    asides = monster_attacks[*attstr].attack_sides;
    attstr++;
    flag = false;
    if ((pyflags.protevil > 0) && (r_ptr->cdefense & CD_EVIL) &&
	((pymisc.lev + 1) > r_ptr->level)) {
      if (m_ptr->ml)
	recall_cdefense_or(m_ptr->mptr, CD_EVIL);
      attype = 99;
      adesc = 99;
    }
    switch(attype) {
    case 1:	      /*Normal attack  */
      if (test_hit(60, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 2:	      /*Lose Strength*/
      if (test_hit(-3, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 3:	      /*Confusion attack*/
      if (test_hit(10, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 4:	      /*Fear attack    */
      if (test_hit(10, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 5:	      /*Fire attack    */
      if (test_hit(10, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 6:	      /*Acid attack    */
      if (test_hit(0, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 7:	      /*Cold attack    */
      if (test_hit(10, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 8:	      /*Lightning attack*/
      if (test_hit(10, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 9:	      /*Corrosion attack*/
      if (test_hit(0, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 10:      /*Blindness attack*/
      if (test_hit(2, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 11:      /*Paralysis attack*/
      if (test_hit(2, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 12:      /*Steal Money    */
      if ((test_hit(5, (Short)r_ptr->level, 0, (Short)pymisc.lev,CLA_MISC_HIT))
	  && (pymisc.au > 0))
	flag = true;
      break;
    case 13:      /*Steal Object   */
      if ((test_hit(2, (Short)r_ptr->level, 0, (Short)pymisc.lev,CLA_MISC_HIT))
	  && (inven_ctr > 0))
	flag = true;
      break;
    case 14:      /*Poison	       */
      if (test_hit(5, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 15:      /*Lose dexterity*/
      if (test_hit(0, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 16:      /*Lose constitution*/
      if (test_hit(0, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 17:      /*Lose intelligence*/
      if (test_hit(2, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 18:      /*Lose wisdom*/
      if (test_hit(2, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 19:      /*Lose experience*/
      if (test_hit(5, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 20:      /*Aggravate monsters*/
      flag = true;
      break;
    case 21:      /*Disenchant	  */
      if (test_hit(20, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 22:      /*Eat food	  */
      if (test_hit(5, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 23:      /*Eat light	  */
      if (test_hit(5, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		   CLA_MISC_HIT))
	flag = true;
      break;
    case 24:      /*Eat charges	  */
      if ((test_hit(15, (Short)r_ptr->level, 0, pymisc.pac+pymisc.ptoac,
		    CLA_MISC_HIT)) &&
	  (inven_ctr > 0))	/* check to make sure an object exists */
	flag = true;
      break;
    case 99:
      flag = true;
      break;
    default:
      break;
    }
    if (flag) {
      /* can not StrCat to cdesc because the creature may have
	 multiple attacks */
      disturb(true, false);
      StrCopy(tmp_str, cdesc);
      switch(adesc) {
      case 1: message(StrCat(tmp_str, "hits you.")); break;
      case 2: message(StrCat(tmp_str, "bites you.")); break;
      case 3: message(StrCat(tmp_str, "claws you.")); break;
      case 4: message(StrCat(tmp_str, "stings you.")); break;
      case 5: message(StrCat(tmp_str, "touches you.")); break;
#if 0
      case 6: message(StrCat(tmp_str, "kicks you.")); break;
#endif
      case 7: message(StrCat(tmp_str, "gazes at you.")); break;
      case 8: message(StrCat(tmp_str, "breathes on you.")); break;
      case 9: message(StrCat(tmp_str, "spits on you.")); break;
      case 10: message(StrCat(tmp_str,"makes a horrible wail."));break;
#if 0
      case 11: message(StrCat(tmp_str, "embraces you.")); break;
#endif
      case 12: message(StrCat(tmp_str, "crawls on you.")); break;
      case 13:
	message(StrCat(tmp_str, "releases a cloud of spores.")); break;
      case 14: message(StrCat(tmp_str, "begs you for money.")); break;
      case 15: message("You've been slimed!"); break;
      case 16: message(StrCat(tmp_str, "crushes you.")); break;
      case 17: message(StrCat(tmp_str, "tramples you.")); break;
      case 18: message(StrCat(tmp_str, "drools on you.")); break;
      case 19:
	switch(randint(9)) {
	case 1: message(StrCat(tmp_str, "insults you!")); break;
	case 2:
	  message(StrCat(tmp_str, "insults your mother!")); break;
	case 3:
	  message(StrCat(tmp_str, "gives you the finger!")); break;
	case 4: message(StrCat(tmp_str, "humiliates you!")); break;
	case 5: message(StrCat(tmp_str, "wets on your leg!")); break;
	case 6: message(StrCat(tmp_str, "defiles you!")); break;
	case 7: message(StrCat(tmp_str, "dances around you!"));break;
	case 8:
	  message(StrCat(tmp_str, "makes obscene gestures!")); break;
	case 9: message(StrCat(tmp_str, "moons you!!!")); break;
	}
	break;
      case 99: message(StrCat(tmp_str, "is repelled.")); break;
      default: break;
      }

      notice = true;
      /* always fail to notice attack if creature invisible, set notice
	 and visible here since creature may be visible when attacking
	 and then teleport afterwards (becoming effectively invisible) */
      if (!m_ptr->ml) {
	visible = false;
	notice = false;
      } else
	visible = true;

      damage = damroll (adice, asides);
      switch(attype) {
      case 1:    /*Normal attack	*/
	/* round half-way case down */
	damage -= ((pymisc.pac+pymisc.ptoac) * damage) / 200;
	take_hit(damage, ddesc);
	break;
      case 2:    /*Lose Strength*/
	take_hit(damage, ddesc);
	if (pyflags.sustain_str)
	  message("You feel weaker for a moment, but it passes.");
	else if (randint(2) == 1) {
	  message("You feel weaker.");
	  dec_stat (A_STR);
	} else
	  notice = false;
	break;
      case 3:    /*Confusion attack*/
	take_hit(damage, ddesc);
	if (randint(2) == 1) {
	  if (pyflags.confused < 1) {
	    message("You feel confused.");
	    pyflags.confused += randint((Short)r_ptr->level);
	  } else
	    notice = false;
	  pyflags.confused += 3;
	} else
	  notice = false;
	break;
      case 4:    /*Fear attack	*/
	take_hit(damage, ddesc);
	if (player_saves())
	  message("You resist the effects!");
	else if (pyflags.afraid < 1) {
	  message("You are suddenly afraid!");
	  pyflags.afraid += 3 + randint((Short)r_ptr->level);
	} else {
	  pyflags.afraid += 3;
	  notice = false;
	}
	break;
      case 5:    /*Fire attack	*/
	message("You are enveloped in flames!");
	fire_dam(damage, ddesc);
	break;
      case 6:    /*Acid attack	*/
	message("You are covered in acid!");
	acid_dam(damage, ddesc);
	break;
      case 7:    /*Cold attack	*/
	message("You are covered with frost!");
	cold_dam(damage, ddesc);
	break;
      case 8:    /*Lightning attack*/
	message("Lightning strikes you!");
	light_dam(damage, ddesc);
	break;
      case 9:    /*Corrosion attack*/
	message("A stinging red gas swirls about you.");
	corrode_gas(ddesc);
	take_hit(damage, ddesc);
	break;
      case 10:	/*Blindness attack*/
	take_hit(damage, ddesc);
	if (pyflags.blind < 1) {
	  pyflags.blind += 10 + randint((Short)r_ptr->level);
	  message("Your eyes begin to sting.");
	} else {
	  pyflags.blind += 5;
	  notice = false;
	}
	break;
      case 11:	/*Paralysis attack*/
	take_hit(damage, ddesc);
	if (player_saves())
	  message("You resist the effects!");
	else if (pyflags.paralysis < 1) {
	  if (pyflags.free_act)
	    message("You are unaffected.");
	  else {
	    pyflags.paralysis = randint((Short)r_ptr->level) + 3;
	    message("You are paralyzed.");
	  }
	} else
	  notice = false;
	break;
      case 12:	/*Steal Money	  */
	if ((pyflags.paralysis < 1) &&
	    (randint(124) < pystats.use_stat[A_DEX]))
	  message("You quickly protect your money pouch!");
	else {
	  gold = (pymisc.au/10) + randint(25);
	  if (gold > pymisc.au)
	    pymisc.au = 0;
	  else
	    pymisc.au -= gold;
	  message("Your purse feels lighter.");
	  print_stats(STATS_GOLD_AC); //prt_gold();
	}
	if (randint(2) == 1) {
	  message("There is a puff of smoke!");
	  teleport_away(monptr, MAX_SIGHT);
	}
	break;
      case 13:	/*Steal Object	 */
	if ((pyflags.paralysis < 1) &&
	    (randint(124) < pystats.use_stat[A_DEX]))
	  message("You grab hold of your backpack!");
	else {
	  i = randint(inven_ctr) - 1;
	  inven_destroy(i);
	  message("Your backpack feels lighter.");
	}
	if (randint(2) == 1) {
	  message("There is a puff of smoke!");
	  teleport_away(monptr, MAX_SIGHT);
	}
	break;
      case 14:	/*Poison	 */
	take_hit(damage, ddesc);
	message("You feel very sick.");
	pyflags.poisoned += randint((Short)r_ptr->level)+5;
	break;
      case 15:	/*Lose dexterity */
	take_hit(damage, ddesc);
	if (pyflags.sustain_dex)
	  message("You feel clumsy for a moment, but it passes.");
	else {
	  message("You feel more clumsy.");
	  dec_stat (A_DEX);
	}
	break;
      case 16:	/*Lose constitution */
	take_hit(damage, ddesc);
	if (pyflags.sustain_con)
	  message("Your body resists the effects of the disease.");
	else {
	  message("Your health is damaged!");
	  dec_stat (A_CON);
	}
	break;
      case 17:	/*Lose intelligence */
	take_hit(damage, ddesc);
	message("You have trouble thinking clearly.");
	if (pyflags.sustain_int)
	  message("But your mind quickly clears.");
	else
	  dec_stat (A_INT);
	break;
      case 18:	/*Lose wisdom	   */
	take_hit(damage, ddesc);
	if (pyflags.sustain_wis)
	  message("Your wisdom is sustained.");
	else {
	  message("Your wisdom is drained.");
	  dec_stat (A_WIS);
	}
	break;
      case 19:	/*Lose experience  */
	message("You feel your life draining away!");
	lose_exp(damage + (pymisc.exp / 100)*MON_DRAIN_LIFE);
	break;
      case 20:	/*Aggravate monster*/
	aggravate_monster(20);
	break;
      case 21:	/*Disenchant	   */
	  flag = false;
	  switch(randint(7)) {
	  case 1: i = INVEN_WIELD; break;
	  case 2: i = INVEN_BODY;	 break;
	  case 3: i = INVEN_ARM;	 break;
	  case 4: i = INVEN_OUTER; break;
	  case 5: i = INVEN_HANDS; break;
	  case 6: i = INVEN_HEAD;	 break;
	  case 7: i = INVEN_FEET;	 break;
	  }
	  i_ptr = &inventory[i];
	  {
	    Char tohit, todam, toac;
	    tohit = i_ptr->tohit;
	    todam = i_ptr->todam;
	    toac = i_ptr->toac;
	    if (tohit > 0) {
	      tohit -= randint(2);
	      if (tohit < 0) tohit = 0;
	      flag = true;
	    }
	    if (todam > 0) {
	      todam -= randint(2);
	      if (todam < 0) todam = 0;
	      flag = true;
	    }
	    if (toac > 0) {
	      toac  -= randint(2);
	      if (toac < 0) toac = 0;
	      flag = true;
	    }
	    if (flag) {
	      invy_set_hitdamac(inventory, i, tohit, todam, toac);//Writes it.
	      message("There is a static feeling in the air.");
	      calc_bonuses ();
	    } else
	      notice = false;
	  }
	  break;
      case 22:	/*Eat food	   */
	if (find_range(TV_FOOD, TV_NEVER, &i, &j)) {
	  inven_destroy(i);
	  message ("It got at your rations!");
	}
	else
	  notice = false;
	break;
      case 23:	/*Eat light	   */
	i_ptr = &inventory[INVEN_LIGHT];
	{
	  Short p1 = i_ptr->p1;
	  if (p1 > 0) {
	    p1 -= (250 + randint(250));
	    if (p1 < 1)  p1 = 1;
	    invy_set_p1(inventory, i, p1); // this does the actual DmWrite.
	    if (pyflags.blind < 1)
	      message("Your light dims.");
	    else
	      notice = false;
	  } else
	    notice = false;
	}
	break;
      case 24:	/*Eat charges	  */
	i = randint(inven_ctr) - 1;
	j = r_ptr->level;
	i_ptr = &inventory[i];
	if (((i_ptr->tval == TV_STAFF) || (i_ptr->tval == TV_WAND))
	    && (i_ptr->p1 > 0)) {
	  //	  m_ptr->hp += j*i_ptr->p1;
	  mlist_set_short(monptr, MLIST_hp, m_ptr->hp + j*i_ptr->p1);
	  invy_set_p1(inventory, i, 0); // this sets "i_ptr->p1 = 0;"
	  if (! known2_p (i_ptr))
	    add_inscribe (inventory, i, ID_EMPTY);
	  message("Energy drains from your pack!");
	}
	else
	  notice = false;
	break;
      case 99:
	notice = false;
	break;
      default:
	notice = false;
	break;
      }

      /* Moved here from mon_move, so that monster only confused if it
	 actually hits.  A monster that has been repelled has not hit
	 the player, so it should not be confused.  */
      if (pyflags.confuse_monster && adesc != 99) {
	message("Your hands stop glowing.");
	pyflags.confuse_monster = false;
	if ((randint(MAX_MONS_LEVEL) < r_ptr->level) ||
	    (CD_NO_SLEEP & r_ptr->cdefense))
	  StrPrintF(tmp_str, "%sis unaffected.", cdesc);
	else {
	  StrPrintF(tmp_str, "%sappears confused.", cdesc);
	  if (m_ptr->confused)
	    //	    m_ptr->confused += 3;
	    mlist_set_uchar(monptr, MLIST_confused, m_ptr->confused + 3);
	  else
	    //	    m_ptr->confused = 2 + randint(16);
	    mlist_set_uchar(monptr, MLIST_confused,
			    m_ptr->confused + 2 + randint(16));
	}
	message(tmp_str);
	if (visible && !death && randint(4) == 1) {
	  // don't ask me what NO_SLEEP has to do with confusion resistance
	  recall_cdefense_or( m_ptr->mptr, (r_ptr->cdefense & CD_NO_SLEEP) );
	}
      }

      /* increase number of attacks if notice true, or if visible and had
	 previously noticed the attack (in which case all this does is
	 help player learn damage), note that in the second case do
	 not increase attacks if creature repelled (no damage done) */
      if ((notice ||
	   (visible &&
	    c_recall[m_ptr->mptr].r_attacks[attackn] != 0 &&
	    attype != 99)) &&
	  c_recall[m_ptr->mptr].r_attacks[attackn] < MAX_UCHAR) {
	// this does "c_recall[m_ptr->mptr].r_attacks[attackn]++;"
	recall_increment_attacks( m_ptr->mptr, attackn );
      }
      if (death && c_recall[m_ptr->mptr].r_deaths < MAX_SHORT) {
	// this does c_recall[m_ptr->mptr].r_deaths++;
	recall_increment_deaths(m_ptr->mptr);
      }
    } else { // i.e. NOT if(flag)
      if ((adesc >= 1 && adesc <= 3) || (adesc == 6)) {
	disturb(true, false);
	StrCopy(tmp_str, cdesc);
	message(StrCat(tmp_str, "misses you."));
      }
    }
    if (attackn < MAX_MON_NATTACK-1)
      attackn++;
    else
      break;
  }
}


/* Make the move if possible, five choices		-RAK-	*/
// mm is an array of 9 shorts, though we only use 5 and they
// are the five best moves in order of goodness.
static void make_move(Short monptr, Short *mm, ULong *rcmove)
{
  Short i, newy, newx, do_turn, do_move, stuck_door;
  ULong movebits;
  //  cave_type *c_ptr;
  Short c_ptr;
  monster_type *m_ptr;
  inven_type *t_ptr;
  UChar thing, critter;

  i = 0;
  do_turn = false;
  do_move = false;
  m_ptr = &m_list[monptr];
  movebits = c_list[m_ptr->mptr].cmove;
  for (i = 0 ; (!do_turn && (i < 5)) ; i++) {
    /* Get new position		*/
    newy = m_ptr->fy;
    newx = m_ptr->fx;
    mmove(mm[i], &newy, &newx);
    c_ptr = newy * MAX_WIDTH + newx; //      c_ptr = &cave[newy][newx];
    thing = cave_tptr[c_ptr];
    critter = cave_cptr[c_ptr];
    if (cave_fval[c_ptr] != BOUNDARY_WALL) {
      /* Floor is open?		   */
      if (cave_fval[c_ptr] <= MAX_OPEN_SPACE)
	do_move = true;
      /* Creature moves through walls? */
      else if (movebits & CM_PHASE) {
	do_move = true;
	*rcmove |= CM_PHASE;
      }
      /* Creature can open doors?	   */
      else if (thing != 0) {
	t_ptr = &t_list[thing];
	if (movebits & CM_OPEN_DOOR) {
	  /* Creature can open doors.		     */
	  stuck_door = false;
	  if (t_ptr->tval == TV_CLOSED_DOOR) {
	    do_turn = true;
	    if (t_ptr->p1 == 0)		/* Closed doors	 */
	      do_move = true;
	    else if (t_ptr->p1 > 0) {	       /* Locked doors	*/
	      if (randint((m_ptr->hp+1)*(50+t_ptr->p1)) <
		  40*(m_ptr->hp-10-t_ptr->p1))
		invy_set_p1(t_list, thing, 0);//t_ptr->p1 = 0;
	    }
	    else if (t_ptr->p1 < 0)	{      /* Stuck doors	*/
	      if (randint((m_ptr->hp+1)*(50-t_ptr->p1)) <
		  40*(m_ptr->hp-10+t_ptr->p1)) {
		message("You hear a door burst open!");
		disturb(true, false);
		stuck_door = true;
		do_move = true;
	      }
	    }
	  }
	  else if (t_ptr->tval == TV_SECRET_DOOR) {
	    do_turn = true;
	    do_move = true;
	  }
	  if (do_move) {
	    invcopy(t_list, thing, OBJ_OPEN_DOOR);
	    if (stuck_door) /* 50% chance of breaking door */
	      invy_set_p1(t_list, thing, 1 - randint(2)); // t_ptr->p1 = ;
	    cave_set(newy, newx, CAVE_fval, CORR_FLOOR);
	    lite_spot(newy, newx);
	    *rcmove |= CM_OPEN_DOOR;
	    do_move = false;
	  }
	}
	else {     /* Creature can not open doors, must bash them   */
	  if (t_ptr->tval == TV_CLOSED_DOOR) {
	    do_turn = true;
	    if (randint((m_ptr->hp+1)*(80+abs(t_ptr->p1))) <
		40*(m_ptr->hp-20-abs(t_ptr->p1)))
	      {
		invcopy(t_list, thing, OBJ_OPEN_DOOR);
		/* 50% chance of breaking door */
		invy_set_p1(t_list, thing, 1 - randint(2)); // t_ptr->p1 =;
		cave_set(newy, newx, CAVE_fval, CORR_FLOOR);
		lite_spot(newy, newx);
		message ("You hear a door burst open!");
		disturb(true, false);
	      }
	  }
	}
      }
      /* Glyph of warding present?	   */
      if (do_move && (thing != 0) &&
	  (t_list[thing].tval == TV_VIS_TRAP) &&
	  (t_list[thing].subval == 99)) {
	if (randint(OBJ_RUNE_PROT) < c_list[m_ptr->mptr].level) {
	  if ((newy==char_row) && (newx==char_col))
	    message("The rune of protection is broken!");
	  delete_object(newy, newx);
	}
	else {
	  do_move = false;
	  /* If the creature moves only to attack, */
	  /* don't let it move if the glyph prevents */
	  /* it from attacking */
	  if (movebits & CM_ATTACK_ONLY)
	    do_turn = true;
	}
      }
      /* Creature has attempted to move on player?	   */
      if (do_move) {
	if (critter == 1) {
	  /* if the monster is not lit, must call update_mon, it may
	     be faster than character, and hence could have just
	     moved next to character this same turn */
	  if (!m_ptr->ml)
	    update_mon(monptr);
	  make_attack(monptr);
	  do_move = false;
	  do_turn = true;
	}
      /* Creature is attempting to move on other creature?	   */
	else if ((critter > 1) &&
		 ((newy != m_ptr->fy) ||
		  (newx != m_ptr->fx))) {
	  /* Creature eats other creatures?	 */
	  if ((movebits & CM_EATS_OTHER) &&
	      (c_list[m_ptr->mptr].mexp >=
	       c_list[m_list[critter].mptr].mexp))
	    {
	      if (m_list[critter].ml)
		*rcmove |= CM_EATS_OTHER;
	      /* It ate an already processed monster. Handle normally. */
	      if (monptr < critter)
		delete_monster((Short) critter);
	      /* If it eats this monster, an already processed monster
		 will take its place, causing all kinds of havoc.  Delay
		 the kill a bit. */
	      else
		fix1_delete_monster((Short) critter);
	    }
	  else
	    do_move = false;
	}
      }
      /* Creature has been allowed move.	 */
      if (do_move) {
	/* Pick up or eat an object	       */
	if (movebits & CM_PICKS_UP) {
	  //	      c_ptr = &cave[newy][newx];
	  if ((thing != 0)
	      && (t_list[thing].tval <= TV_MAX_OBJECT)) {
	    *rcmove |= CM_PICKS_UP;
	    delete_object(newy, newx);
	  }
	}
	/* Move creature record		       */
	move_rec((Short)m_ptr->fy, (Short)m_ptr->fx, newy, newx);
	if (m_ptr->ml) {
	  //	  m_ptr->ml = false;
	  mlist_set_ml(monptr, false);
	  lite_spot ((Short)m_ptr->fy, (Short)m_ptr->fx);
	}
	mlist_set_uchar(monptr, MLIST_fy, newy);
	mlist_set_uchar(monptr, MLIST_fx, newx);
	mlist_set_uchar(monptr, MLIST_cdis,
			distance(char_row, char_col, newy, newx));
	do_turn = true;
      }
    }
  } // well, either do_turn==true, or we ran out of our 5 tries.

}



/* Creatures can cast spells too.  (Dragon Breath)	-RAK-	*/
/* cast_spell = true if creature changes position	*/
/* took_turn  = true if creature casts a spell		*/
static void mon_cast_spell(Short monptr, Boolean *took_turn)
{
  ULong i;
  Short y, x, chance, thrown_spell, r1;
  Short k;
  Short spell_choice[30];
  Char cdesc[80], outval[80], ddesc[80];
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char *monname;

  if (death)
    return;

  m_ptr = &m_list[monptr];
  r_ptr = &c_list[m_ptr->mptr];
  monname = c_names + r_ptr->name_offset;
  chance = (Short)(r_ptr->spells & CS_FREQ);
  /* 1 in x chance of casting spell		   */
  if (randint(chance) != 1)
    *took_turn	= false;
  /* Must be within certain range		   */
  else if (m_ptr->cdis > MAX_SPELL_DIS)
    *took_turn	= false;
  /* Must have unobstructed Line-Of-Sight	   */
  else if (!los(char_row, char_col, (Short)m_ptr->fy, (Short)m_ptr->fx))
    *took_turn	= false;
  else	/* Creature is going to cast a spell	 */ {
    *took_turn  = true;
    /* Check to see if monster should be lit. */
    update_mon (monptr);
    /* Describe the attack			       */
    if (m_ptr->ml)
      StrPrintF(cdesc, "The %s ", monname);
    else
      StrCopy(cdesc, "It ");
    /* For "DIED_FROM" string	 */
    if (CM_WIN & r_ptr->cmove)
      StrPrintF(ddesc, "The %s", monname);
    else if (is_a_vowel (monname[0]))
      StrPrintF (ddesc, "an %s", monname);
    else
      StrPrintF(ddesc, "a %s", monname);
    /* End DIED_FROM		       */

    /* Extract all possible spells into spell_choice */
    i = (r_ptr->spells & ~CS_FREQ);
    k = 0;
    // xxx What the heck is this doing?
    while (i != 0) {
      spell_choice[k] = bit_pos(&i);
      k++;
    }
    /* Choose a spell to cast			       */
    thrown_spell = spell_choice[randint(k) - 1];
    thrown_spell++;
    /* all except teleport_away() and drain mana spells always disturb */
    if (thrown_spell > 6 && thrown_spell != 17)
      disturb(true, false);
    /* save some code/data space here, with a small time penalty */
    if ((thrown_spell < 14 && thrown_spell > 6) || (thrown_spell == 16)) {
      StrCat(cdesc, "casts a spell.");
      message(cdesc);
    }
    /* Cast the spell.			     */
    switch(thrown_spell) {
    case 5:	 /*Teleport Short*/
      teleport_away(monptr, 5);
      break;
    case 6:	 /*Teleport Long */
      teleport_away(monptr, MAX_SIGHT);
      break;
    case 7:	 /*Teleport To	 */
      teleport_to((Short)m_ptr->fy, (Short)m_ptr->fx);
      break;
    case 8:	 /*Light Wound	 */
      if (player_saves())
	message("You resist the effects of the spell.");
      else
	take_hit(damroll(3, 8), ddesc);
      break;
    case 9:	 /*Serious Wound */
      if (player_saves())
	message("You resist the effects of the spell.");
      else
	take_hit(damroll(8, 8), ddesc);
      break;
    case 10:  /*Hold Person	  */
      if (pyflags.free_act)
	message("You are unaffected.");
      else if (player_saves())
	message("You resist the effects of the spell.");
      else if (pyflags.paralysis > 0)
	pyflags.paralysis += 2;
      else
	pyflags.paralysis = randint(5)+4;
      break;
    case 11:  /*Cause Blindness*/
      if (player_saves())
	message("You resist the effects of the spell.");
      else if (pyflags.blind > 0)
	pyflags.blind += 6;
      else
	pyflags.blind += 12 + randint(3);
      break;
    case 12:  /*Cause Confuse */
      if (player_saves())
	message("You resist the effects of the spell.");
      else if (pyflags.confused > 0)
	pyflags.confused += 2;
      else
	pyflags.confused = randint(5) + 3;
      break;
    case 13:  /*Cause Fear	  */
      if (player_saves())
	message("You resist the effects of the spell.");
      else if (pyflags.afraid > 0)
	pyflags.afraid += 2;
      else
	pyflags.afraid = randint(5) + 3;
      break;
    case 14:  /*Summon Monster*/
      StrCat(cdesc, "magically summons a monster!");
      message(cdesc);
      y = char_row;
      x = char_col;
      /* in case compact_monster() is called,it needs monptr */
      hack_monptr = monptr;
      summon_monster(&y, &x, false);
      hack_monptr = -1;
      update_mon ((Short)cave_cptr[y*MAX_WIDTH+x]);
      break;
    case 15:  /*Summon Undead*/
      StrCat(cdesc, "magically summons an undead!");
      message(cdesc);
      y = char_row;
      x = char_col;
      /* in case compact_monster() is called,it needs monptr */
      hack_monptr = monptr;
      summon_undead(&y, &x);
      hack_monptr = -1;
      update_mon ((Short)cave_cptr[y*MAX_WIDTH+x]);
      break;
    case 16:  /*Slow Person	 */
      if (pyflags.free_act)
	message("You are unaffected.");
      else if (player_saves())
	message("You resist the effects of the spell.");
      else if (pyflags.slow > 0)
	pyflags.slow += 2;
      else
	pyflags.slow = randint(5) + 3;
      break;
    case 17:  /*Drain Mana	 */
      if (pymisc.cmana > 0) {
	disturb(true, false);
	StrPrintF(outval, "%sdraws psychic energy from you!",cdesc);
	message(outval);
	if (m_ptr->ml) {
	  StrPrintF(outval, "%sappears healthier.", cdesc);
	  message(outval);
	}
	r1 = (randint((Short)r_ptr->level) >> 1) + 1;
	if (r1 > pymisc.cmana) {
	  r1 = pymisc.cmana;
	  pymisc.cmana = 0;
	  pymisc.cmana_frac = 0;
	}
	else
	  pymisc.cmana -= r1;
	print_stats(STATS_QUO);//prt_cmana();
	//	m_ptr->hp += 6*(r1);
	mlist_set_short(monptr, MLIST_hp, m_ptr->hp + 6*(r1));
      }
      break;
    case 20:  /*Breath Light */
      StrCat(cdesc, "breathes lightning.");
      message(cdesc);
      breath(GF_LIGHTNING, char_row, char_col, (m_ptr->hp / 4), ddesc,
	     monptr);
      break;
    case 21:  /*Breath Gas	 */
      StrCat(cdesc, "breathes gas.");
      message(cdesc);
      breath(GF_POISON_GAS, char_row, char_col, (m_ptr->hp / 3), ddesc,
	     monptr);
      break;
    case 22:  /*Breath Acid	 */
      StrCat(cdesc, "breathes acid.");
      message(cdesc);
      breath(GF_ACID, char_row, char_col, (m_ptr->hp / 3), ddesc, monptr);
      break;
    case 23:  /*Breath Frost */
      StrCat(cdesc, "breathes frost.");
      message(cdesc);
      breath(GF_FROST, char_row, char_col, (m_ptr->hp / 3), ddesc, monptr);
      break;
    case 24:  /*Breath Fire	 */
      StrCat(cdesc, "breathes fire.");
      message(cdesc);
      breath(GF_FIRE, char_row, char_col, (m_ptr->hp / 3), ddesc, monptr);
      break;
    default:
      StrCat (cdesc, "cast unknown spell.");
      message(cdesc);
    }
    /* End of spells				       */
    if (m_ptr->ml) {
      Short mi = m_ptr->mptr;
      c_recall[mi].r_spells |= 1L << (thrown_spell-1);
      if ((c_recall[mi].r_spells & CS_FREQ) != CS_FREQ)
	//	c_recall[mi].r_spells++;
	recall_set_spells(mi, c_recall[mi].r_spells + 1);
      if (death && c_recall[mi].r_deaths < MAX_SHORT)
	//	c_recall[mi].r_deaths++;
	recall_increment_deaths(mi);
    }
  }
}


/* Places creature adjacent to given location		-RAK-	*/
/* Rats and Flys are fun!					 */
Boolean multiply_monster(Short y, Short x, Short cr_index, Short monptr)
{
  Short i, j, k;
  //cave_type *c_ptr;
  Short c_ptr;
  Short result;
  UChar critter;

  i = 0;
  for (i = 0 ; i <= 18 ; i++) {
    j = y - 2 + randint(3);
    k = x - 2 + randint(3);
    /* don't create a new creature on top of the old one, that causes
       invincible/invisible creatures to appear */
    if (in_bounds(j, k) && (j != y || k != x)) {
      c_ptr = j * MAX_WIDTH + k; //      c_ptr = &cave[j][k];
      critter = cave_cptr[c_ptr];
      if ((cave_fval[c_ptr] <= MAX_OPEN_SPACE) &&
	  (cave_tptr[c_ptr] == 0) &&
	  (critter != 1)) {
	if (critter > 1)     /* Creature there already?	*/ {
	  /* Some critters are cannibalistic!	    */
	  if ((c_list[cr_index].cmove & CM_EATS_OTHER) &&
	      /* Check the experience level -CJS- */
	      c_list[cr_index].mexp >= c_list[m_list[critter].mptr].mexp) {
	    /* It ate an already processed monster.Handle normally.*/
	    if (monptr < critter)
	      delete_monster((Short) critter);
	    /* If it eats this monster, an already processed mosnter
	       will take its place, causing all kinds of havoc.
	       Delay the kill a bit. */
	    else
	      fix1_delete_monster((Short) critter);
	      
	    /* in case compact_monster() is called,it needs monptr */
	    hack_monptr = monptr;
	    /* Place_monster() may fail if monster list full.  */
	    result = place_monster(j, k, cr_index, false);
	    hack_monptr = -1;
	    if (! result)
	      return false;
	    mon_tot_mult++;
	    return check_mon_lite(j, k);
	  }
	}
	else
	  /* All clear,  place a monster	  */ {
	  /* in case compact_monster() is called,it needs monptr */
	  hack_monptr = monptr;
	  /* Place_monster() may fail if monster list full.  */
	  result = place_monster(j, k, cr_index, false);
	  hack_monptr = -1;
	  if (! result)
	    return false;
	  mon_tot_mult++;
	  return check_mon_lite(j, k);
	}
      }
    }
  }
  return false;
}


/* Move the critters about the dungeon			-RAK-	*/
static void mon_move(Short monptr, ULong *rcmove)
{
  Short i, j;
  Short k, dir;
  Boolean move_test;
  creature_type *r_ptr;
  monster_type *m_ptr;
  Short mm[9];
  Short rest_val;

  m_ptr = &m_list[monptr];
  r_ptr = &c_list[m_ptr->mptr];
  /* Does the critter multiply?				   */
  /* rest could be negative, to be safe, only use mod with positive values. */
  rest_val = abs (pyflags.rest);
  if ((r_ptr->cmove & CM_MULTIPLY) && (MAX_MON_MULT >= mon_tot_mult) &&
      ((rest_val % MON_MULT_ADJ) == 0)) {
    k = 0;
    for (i = m_ptr->fy-1; i <= m_ptr->fy+1; i++)
      for (j = m_ptr->fx-1; j <= m_ptr->fx+1; j++)
	if (in_bounds(i, j) && (cave_cptr[i*MAX_WIDTH+j] > 1))
	  k++;
    /* can't call randint with a value of zero, increment counter
       to allow creature multiplication */
    if (k == 0)
      k++;
    if ((k < 4) && (randint(k*MON_MULT_ADJ) == 1))
      if (multiply_monster((Short)m_ptr->fy, (Short)m_ptr->fx,
			   (Short)m_ptr->mptr, monptr))
	*rcmove |= CM_MULTIPLY;
  }
  move_test = false;

  /* if in wall, must immediately escape to a clear area */
  if (!(r_ptr->cmove & CM_PHASE) &&
      (cave_fval[m_ptr->fy*MAX_WIDTH+m_ptr->fx] >= MIN_CAVE_WALL)) {
    /* If the monster is already dead, don't kill it again!
       This can happen for monsters moving faster than the player.  They
       will get multiple moves, but should not if they die on the first
       move.  This is only a problem for monsters stuck in rock.  */
    if (m_ptr->hp < 0)
      return;

    k = 0;
    dir = 1;
    /* note direction of for loops matches direction of keypad from 1 to 9*/
    /* do not allow attack against the player */
    /* Must cast fy-1 to signed int, so that a nagative value of i will
       fail the comparison.  */
    for (i = m_ptr->fy+1; i >= (Short)(m_ptr->fy-1); i--)
      for (j = m_ptr->fx-1; j <= m_ptr->fx+1; j++) {
	Short ij = i*MAX_WIDTH+j;
	if ((dir != 5) &&
	    (cave_fval[ij] <= MAX_OPEN_SPACE) &&
	    (cave_cptr[ij] != 1))
	  mm[k++] = dir;
	dir++;
      }
    if (k != 0) {
      /* put a random direction first */
      dir = randint (k) - 1;
      i = mm[0];
      mm[0] = mm[dir];
      mm[dir] = i;
      make_move (monptr, mm, rcmove);
      /* this can only fail if mm[0] has a rune of protection */
    }
    /* if still in a wall, let it dig itself out, but also apply some
       more damage */
    if (cave_fval[m_ptr->fy*MAX_WIDTH+m_ptr->fx] >= MIN_CAVE_WALL) {
      /* in case the monster dies, may need to call fix1_delete_monster()
	 instead of delete_monsters() */
      hack_monptr = monptr;
      i = mon_take_hit(monptr, damroll (8, 8));
      hack_monptr = -1;
      if (i >= 0) {
	message("You hear a scream muffled by rock!");
	print_stats(STATS_QUO);//prt_experience();
      } else {
	message ("A creature digs itself out from the rock!");
	twall ((Short)m_ptr->fy, (Short)m_ptr->fx, 1, 0);
      }
    }
    return;  /* monster movement finished */
  }
  /* Creature is confused or undead turned? */
  else if (m_ptr->confused) {
    if (r_ptr->cdefense & CD_UNDEAD) /* Undead only get confused from
					turn undead, so they should flee */ {
      get_moves(monptr,mm);
      mm[0] = 10-mm[0];
      mm[1] = 10-mm[1];
      mm[2] = 10-mm[2];
      mm[3] = randint(9); /* May attack only if cornered */
      mm[4] = randint(9);
    }
    else {
      mm[0] = randint(9);
      mm[1] = randint(9);
      mm[2] = randint(9);
      mm[3] = randint(9);
      mm[4] = randint(9);
    }
    /* don't move him if he is not supposed to move! */
    if (!(r_ptr->cmove & CM_ATTACK_ONLY))
      make_move(monptr, mm, rcmove);
    mlist_set_uchar(monptr, MLIST_confused, m_ptr->confused - 1); // "--"
    move_test = true;
  }
  /* Creature may cast a spell */
  else if (r_ptr->spells & CS_FREQ)
    mon_cast_spell(monptr, &move_test);
  if (!move_test) {
    /* 75% random movement */
    if ((r_ptr->cmove & CM_75_RANDOM) && (randint(100) < 75)) {
      mm[0] = randint(9);
      mm[1] = randint(9);
      mm[2] = randint(9);
      mm[3] = randint(9);
      mm[4] = randint(9);
      *rcmove |= CM_75_RANDOM;
      make_move(monptr, mm, rcmove);
    }
    /* 40% random movement */
    else if ((r_ptr->cmove & CM_40_RANDOM) && (randint(100) < 40)) {
      mm[0] = randint(9);
      mm[1] = randint(9);
      mm[2] = randint(9);
      mm[3] = randint(9);
      mm[4] = randint(9);
      *rcmove |= CM_40_RANDOM;
      make_move(monptr, mm, rcmove);
    }
    /* 20% random movement */
    else if ((r_ptr->cmove & CM_20_RANDOM) && (randint(100) < 20)) {
      mm[0] = randint(9);
      mm[1] = randint(9);
      mm[2] = randint(9);
      mm[3] = randint(9);
      mm[4] = randint(9);
      *rcmove |= CM_20_RANDOM;
      make_move(monptr, mm, rcmove);
    }
    /* Normal movement */
    else if (r_ptr->cmove & CM_MOVE_NORMAL) {
      if (randint(200) == 1) {
	mm[0] = randint(9);
	mm[1] = randint(9);
	mm[2] = randint(9);
	mm[3] = randint(9);
	mm[4] = randint(9);
      }
      else
	get_moves(monptr, mm);
      *rcmove |= CM_MOVE_NORMAL;
      make_move(monptr, mm, rcmove);
    }
    /* Attack, but don't move */
    else if (r_ptr->cmove & CM_ATTACK_ONLY) {
      if (m_ptr->cdis < 2) {
	get_moves(monptr, mm);
	make_move(monptr, mm, rcmove);
      }
      else
	/* Learn that the monster does does not move when it should have
	   moved, but didn't.  */
	*rcmove |= CM_ATTACK_ONLY;
    }
    else if ((r_ptr->cmove & CM_ONLY_MAGIC) && (m_ptr->cdis < 2)) {
      /* A little hack for Quylthulgs, so that one will eventually notice
	 that they have no physical attacks.  */
      if (c_recall[m_ptr->mptr].r_attacks[0] < MAX_UCHAR)
	//	c_recall[m_ptr->mptr].r_attacks[0]++;
	recall_increment_attacks( m_ptr->mptr, 0 );
      /* Another little hack for Quylthulgs, so that one can eventually
	 learn their speed.  */
      if (c_recall[m_ptr->mptr].r_attacks[0] > 20)
	// c_recall[m_ptr->mptr].r_cmove |= CM_ONLY_MAGIC;
	recall_cmove_or( m_ptr->mptr, CM_ONLY_MAGIC );
    }
  }
}


/* Creatures movement and attacking are done from here	-RAK-	*/
// There is a fatal error in here - watch out for writing to m_ptr, a record.
// There is still a fatal error when the creatures move.  Maybe.
void creatures(Short attack)
{
  Short i, k;
  monster_type *m_ptr;
  //  recall_type *r_ptr;
  ULong notice, rcmove;
  Short wake, ignore;
  Char cdesc[80];

  /* Process the monsters  */
  for (i = mfptr - 1; i >= MIN_MONIX && !death; i--) {
    m_ptr = &m_list[i];
    /* Get rid of an eaten/breathed on monster.  Note: Be sure not to
       process this monster. This is necessary because we can't delete
       monsters while scanning the m_list here. */
    if (m_ptr->hp < 0) {
      fix2_delete_monster(i);
      continue;
    }

    mlist_set_uchar(i, MLIST_cdis, distance(char_row, char_col,
					    (Short) m_ptr->fy,
					    (Short) m_ptr->fx));
    if (attack)   /* Attack is argument passed to CREATURE*/ {
      k = movement_rate(m_ptr->cspeed);
      if (k <= 0)
	update_mon(i);
      else {
	while (k > 0) {
	  k--;
	  wake = false;
	  ignore = false;
	  rcmove = 0;
	  if (m_ptr->ml || (m_ptr->cdis <= c_list[m_ptr->mptr].aaf) ||
	      /* Monsters trapped in rock must be given a turn also,
		 so that they will die/dig out immediately.  */
	      ((! (c_list[m_ptr->mptr].cmove & CM_PHASE)) &&
	       cave_fval[m_ptr->fy*MAX_WIDTH+m_ptr->fx] >= MIN_CAVE_WALL)) {
	    if (m_ptr->csleep > 0) {
	      if (pyflags.aggravate)
		mlist_set_short(i, MLIST_csleep, 0); // m_ptr->csleep = 0;
	      else if ((pyflags.rest == 0 &&
			pyflags.paralysis < 1) ||
		       (randint(50) == 1)) {
		notice = randint(1024);
		if (notice*notice*notice <= (1L << (29 - pymisc.stl))) {
		  Short new_csleep = m_ptr->csleep - (100 / m_ptr->cdis);
		  if (m_ptr->csleep > 0)
		    ignore = true;
		  else {
		    wake = true; /* force it to be exactly zero */
		    new_csleep = 0;
		  }
		  mlist_set_short(i, MLIST_csleep, new_csleep);
		} // fi notice^3
	      }
	    } // fi csleep > 0
	    if (m_ptr->stunned != 0) {
	      /* NOTE: Balrog = 100*100 = 10000, it always
		 recovers instantly */
	      if (randint(5000) <
		  c_list[m_ptr->mptr].level * c_list[m_ptr->mptr].level)
		mlist_set_uchar(i, MLIST_stunned, 0); // m_ptr->stunned = 0;
	      else
		mlist_set_uchar(i, MLIST_stunned, m_ptr->stunned - 1); // "--"
	      if (m_ptr->stunned == 0) {
		if (m_ptr->ml) {
		  StrPrintF(cdesc, "The %s ",
			    c_names + c_list[m_ptr->mptr].name_offset);
		  message(StrCat(cdesc,
				 "recovers and glares at you."));
		}
	      }
	    } // fi stunned != 0
	    if ((m_ptr->csleep == 0) && (m_ptr->stunned == 0))
	      mon_move (i, &rcmove);
	  } // fi monster gets a tick
	    
	  update_mon(i);
	  if (m_ptr->ml) {
	    Short mi = m_ptr->mptr;
	    // r_ptr = &c_recall[m_ptr->mptr];
	    if (wake) {
	      if (c_recall[m_ptr->mptr].r_wake < MAX_UCHAR)
		//r_ptr->r_wake++;
		recall_set_wake(mi, c_recall[mi].r_wake + 1);
	    } else if (ignore) {
	      if (c_recall[m_ptr->mptr].r_ignore < MAX_UCHAR)
		//r_ptr->r_ignore++;
		recall_set_ignore(mi, c_recall[mi].r_ignore + 1);
	    }
	    //r_ptr->r_cmove |= rcmove;
	    recall_cmove_or(mi, rcmove);
	  } // fi m_ptr->ml
	} // elihw (k > 0)
      }
    } // fi (attack)
    else {
      update_mon(i);
    }

    /* Get rid of an eaten/breathed on monster.  This is necessary because
       we can't delete monsters while scanning the m_list here.  This
       monster may have been killed during mon_move(). */
    if (m_ptr->hp < 0) {
      fix2_delete_monster(i);
      continue;
    }
  }
  /* End processing monsters	   */
}

// XXXXXXXXXX I have PalmOS'ified everything above here XXXXXXXXXXXXXXXXXX
// I did it pretty late at night though
//
// Yes, i can tell.  "m_ptr->foo = blah;"  indeed.... m_ptr is a *record*.
