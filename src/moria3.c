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

// testing functions from moria3.c

// MISSING PIECES:  not including "xxx not implemented yet"s
// move_char: get_panel(), prt_map()
// openobject: count_message()

static void hit_trap(Short y, Short x) SEC_3;
static void carry(Short y, Short x, Boolean pickup) SEC_3;
static Short summon_object(Short y, Short x, Short num, Short typ) SEC_3;



/* Player hit a trap.	(Chuckle)			-RAK-	*/
static void hit_trap(Short y, Short x)
{
  Short i, ty, tx, num, dam;
  inven_type *t_ptr;
  Char tmp[160];

  end_find();
  change_trap(y, x);
  t_ptr = &t_list[cave_tptr[y*MAX_WIDTH+x]];
  dam = pdamroll(t_ptr->damage);
  switch(t_ptr->subval) {
  case 1:  /* Open pit*/
    message("You fell into a pit!");
    if (pyflags.ffall)
      message("You gently float down.");
    else {
      objdes(tmp, t_ptr, true);
      take_hit(dam, tmp);
    }
    break;
  case 2: /* Arrow trap*/
    if (test_hit(125, 0, 0, pymisc.pac+pymisc.ptoac, CLA_MISC_HIT)) {
      objdes(tmp, t_ptr, true);
      take_hit(dam, tmp);
      message("An arrow hits you.");
    } else
      message("An arrow barely misses you.");
    break;
  case 3: /* Covered pit*/
    message("You fell into a covered pit.");
    if (pyflags.ffall)
      message("You gently float down.");
    else {
      objdes(tmp, t_ptr, true);
      take_hit(dam, tmp);
    }
    place_trap(y, x, 0);
    break;
  case 4: /* Trap door*/
    message("You fell through a trap door!");
    new_level_flag = true;
    dun_level++;
    if (pyflags.ffall)
      message("You gently float down.");
    else {
      objdes(tmp, t_ptr, true);
      take_hit(dam, tmp);
    }
    /* Force the messages to display before starting to generate the
       next level.  */
    show_messages(); // maybe this will work. maybe not.
    break;
  case 5: /* Sleep gas*/
    if (pyflags.paralysis == 0) {
      message("A strange white mist surrounds you!");
      if (pyflags.free_act)
	message("You are unaffected.");
      else {
	message("You fall asleep.");
	pyflags.paralysis += randint(10) + 4;
      }
    }
    break;
  case 6: /* Hid Obj*/
    delete_object(y, x);
    place_object(y, x, false);
    message("Hmmm, there was something under this rock.");
    break;
  case 7:  /* STR Dart*/
    if (test_hit(125, 0, 0, pymisc.pac+pymisc.ptoac, CLA_MISC_HIT)) {
      if (!pyflags.sustain_str) {
	dec_stat(A_STR);
	objdes(tmp, t_ptr, true);
	take_hit(dam, tmp);
	message("A small dart weakens you!");
      } else
	message("A small dart hits you.");
    } else
      message("A small dart barely misses you.");
    break;
  case 8: /* Teleport*/
    teleport_flag = true;
    message("You hit a teleport trap!");
    /* Light up the teleport trap, before we teleport away.  */
    move_light (y, x, y, x);
    break;
  case 9: /* Rockfall*/
    take_hit(dam, "a falling rock");
    delete_object(y, x);
    place_rubble(y, x);
    message("You are hit by falling rock.");
    break;
  case 10: /* Corrode gas*/
    /* Makes more sense to print the message first, then damage an
       object.  */
    message("A strange red gas surrounds you.");
    corrode_gas("corrosion gas");
    break;
  case 11: /* Summon mon*/
    delete_object(y, x);	/* Rune disappears.    */
    num = 2 + randint (3);
    for (i = 0; i < num; i++) {
      ty = y;
      tx = x;
      summon_monster(&ty, &tx, false);
    }
    break;
  case 12: /* Fire trap*/
    message("You are enveloped in flames!");
    fire_dam(dam, "a fire trap");
    break;
  case 13: /* Acid trap*/
    message("You are splashed with acid!");
    acid_dam(dam, "an acid trap");
    break;
  case 14: /* Poison gas*/
    message("A pungent green gas surrounds you!");
    poison_gas(dam, "a poison gas trap");
    break;
  case 15: /* Blind Gas */
    message("A black gas surrounds you!");
    pyflags.blind += randint(50) + 50;
    break;
  case 16: /* Confuse Gas*/
    message("A gas of scintillating colors surrounds you!");
    pyflags.confused += randint(15) + 15;
    break;
  case 17: /* Slow Dart*/
    if (test_hit(125, 0, 0, pymisc.pac+pymisc.ptoac, CLA_MISC_HIT)) {
      objdes(tmp, t_ptr, true);
      take_hit(dam, tmp);
      message("A small dart hits you!");
      if (pyflags.free_act)
	message("You are unaffected.");
      else
	pyflags.slow += randint(20) + 10;
    } else
      message("A small dart barely misses you.");
    break;
  case 18: /* CON Dart*/
    if (test_hit(125, 0, 0, pymisc.pac+pymisc.ptoac, CLA_MISC_HIT)) {
      if (!pyflags.sustain_con) {
	dec_stat(A_CON);
	objdes(tmp, t_ptr, true);
	take_hit(dam, tmp);
	message("A small dart saps your health!");
      } else
	message("A small dart hits you.");
    } else
      message("A small dart barely misses you.");
    break;
  case 19: /*Secret Door*/
    break;
  case 99: /* Scare Mon*/
    break;

    /* Town level traps are special,	the stores.	*/
  case 101: /* General    */
    enter_store(0);
    break;
  case 102: /* Armory	    */
    enter_store(1);
    break;
  case 103: /* Weaponsmith*/
    enter_store(2);
    break;
  case 104: /* Temple	    */
    enter_store(3);
    break;
  case 105: /* Alchemy    */
    enter_store(4);
    break;
  case 106: /* Magic-User */
    enter_store(5);
    break;

  default:
    message("Unknown trap value.");
    break;
  }
}

//Short cast_spell(Char *prompt, Short item_val, Short *sn, Short *sc)
//{ return 0; } // This is done with forms now.


/* Player is on an object.  Many things can happen based -RAK-	*/
/* on the TVAL of the object.  Traps are set off, money and most */
/* objects are picked up.  Some objects, such as open doors, just*/
/* sit there.						       */
static void carry(Short y, Short x, Boolean pickup)
{
  Short locn, i;
  Char out_val[160], tmp_str[160];
  inven_type *i_ptr;

  i_ptr = &t_list[cave_tptr[y*MAX_WIDTH+x]];
  i = t_list[cave_tptr[y*MAX_WIDTH+x]].tval;
  if (i <= TV_MAX_PICK_UP) {
    end_find();
    /* There's GOLD in them thar hills!      */
    if (i == TV_GOLD) {
      pymisc.au += i_ptr->cost;
      objdes(tmp_str, i_ptr, true);
      StrPrintF(out_val,
		"You have found %ld gold pieces worth of %s",
		i_ptr->cost, tmp_str);
      print_stats(STATS_GOLD_AC); //prt_gold();
      delete_object(y, x);
      message(out_val);
    } else {
      /* Too many objects?	    */
      if (inven_check_num(i_ptr)) {
	/* Okay,  pick it up      */
	if (pickup && prompt_carry_flag) {
	  objdes(tmp_str, i_ptr, true);
	  /* change the period to a question mark */
	  tmp_str[StrLen(tmp_str)-1] = '?';
	  StrPrintF(out_val, "Pick up %s", tmp_str);
	  pickup = true; //  pickup = get_check(out_val); // xxx not implemented yet
	}
	/* Check to see if it will change the players speed. */
	if (pickup && !inven_check_weight(i_ptr)) {
	  objdes(tmp_str, i_ptr, true);
	  /* change the period to a question mark */
	  tmp_str[StrLen(tmp_str)-1] = '?';
	  StrPrintF(out_val,
		    "Exceed your weight limit to pick up %s",
		    tmp_str);
	  pickup = true; //  pickup = get_check(out_val); // xxx not implemented yet
	}
	/* Attempt to pick up an object.	       */
	if (pickup) {
	  locn = inven_carry(i_ptr);
	  objdes(tmp_str, &inventory[locn], true);
	  StrPrintF(out_val, "You have %s (%c)",tmp_str,locn+'a');
	  message(out_val);
	  delete_object(y, x);
	}
      } else {
	objdes(tmp_str, i_ptr, true);
	StrPrintF(out_val, "You can't carry %s", tmp_str);
	message(out_val);
      }
    }
  }
  /* OPPS!				   */
  else if (i == TV_INVIS_TRAP || i == TV_VIS_TRAP || i == TV_STORE_DOOR)
    hit_trap(y, x);
}



/* Deletes a monster entry from the level		-RAK-	*/
void delete_monster(Short j)
{
  monster_type *m_ptr;

  m_ptr = &m_list[j];
  cave_set(m_ptr->fy, m_ptr->fx, CAVE_cptr, 0);
  if (m_ptr->ml)
    lite_spot((Short) m_ptr->fy, (Short) m_ptr->fx);
  if (j != mfptr - 1) {
    m_ptr = &m_list[mfptr - 1];
    cave_set(m_ptr->fy, m_ptr->fx, CAVE_cptr, j);
    mlist_mv(mfptr - 1, j);
  }
  mfptr--;
  DmSet(m_list, mfptr*sizeof(monster_type), sizeof(monster_type),0);//m_list[mfptr]=blank_monster;
  if (mon_tot_mult > 0)
    mon_tot_mult--;
}

/* The following two procedures implement the same function as delete monster.
   However, they are used within creatures(), because deleting a monster
   while scanning the m_list causes two problems, monsters might get two
   turns, and m_ptr/monptr might be invalid after the delete_monster.
   Hence the delete is done in two steps. */
/* fix1_delete_monster does everything delete_monster does except delete
   the monster record and reduce mfptr, this is called in breathe, and
   a couple of places in creatures.c */
void fix1_delete_monster(Short j)
{
  monster_type *m_ptr;

  m_ptr = &m_list[j];
  /* force the hp negative to ensure that the monster is dead, for example,
     if the monster was just eaten by another, it will still have positive
     hit points */
  mlist_set_short(j, MLIST_hp, -1);
  cave_set(m_ptr->fy, m_ptr->fx, CAVE_cptr, 0);
  if (m_ptr->ml)
    lite_spot((Short) m_ptr->fy, (Short) m_ptr->fx);
  if (mon_tot_mult > 0)
    mon_tot_mult--;
}

/* fix2_delete_monster does everything in delete_monster that wasn't done
   by fix1_monster_delete above, this is only called in creatures() */
void fix2_delete_monster(Short j)
{
  monster_type *m_ptr;

  if (j != mfptr - 1) {
    m_ptr = &m_list[mfptr - 1];
    cave_set(m_ptr->fy, m_ptr->fx, CAVE_cptr, j);
    mlist_mv(mfptr - 1, j);
  }
  mfptr--;
  DmSet(m_list, mfptr*sizeof(monster_type), sizeof(monster_type),0);//m_list[mfptr]=blank_monster;
}


/* Creates objects nearby the coordinates given		-RAK-	  */
static Short summon_object(Short y, Short x, Short num, Short typ)
{
  Short tries, j, k;
  UChar floor, thing;
  Short real_typ, res;

  if ((typ == 1) || (typ == 5))
    real_typ = 1; /* typ == 1 -> objects */
  else
    real_typ = 256; /* typ == 2 -> gold */
  for (res = 0 ; num > 0 ; num--) {
    for (tries = 0 ; tries <= 20 ; tries++) {
      j = y - 3 + randint(5);
      k = x - 3 + randint(5);
      if (in_bounds(j, k) && los(y, x, j, k)) {
	floor = cave_fval[j*MAX_WIDTH+k];
	thing = cave_tptr[j*MAX_WIDTH+k];
	if (floor <= MAX_OPEN_SPACE && (thing == 0)) {
	  if ((typ == 3) || (typ == 7)) {
	    /* typ == 3 -> 50% objects, 50% gold */
	    if (randint(100) < 50)
	      real_typ = 1;
	    else
	      real_typ = 256;
	  }
	  if (real_typ == 1)
	    place_object(j, k, (typ >= 4));
	  else
	    place_gold(j, k);
	  lite_spot(j, k);
	  if (test_light(j, k))
	    res += real_typ;
	  break;
	}
      }
    }
  }
  return res;
}


/* Deletes object from given location			-RAK-	*/
Boolean delete_object(Short y, Short x)
{
  UChar light;

  //  c_ptr = &cave[y][x];
  if (cave_fval[y*MAX_WIDTH+x] == BLOCKED_FLOOR)
    cave_set(y, x, CAVE_fval, CORR_FLOOR);
  pusht(cave_tptr[y*MAX_WIDTH+x]);
  cave_set(y, x, CAVE_tptr, 0);
  light = cave_light[y*MAX_WIDTH+x] & ~LIGHT_fm;
  cave_set(y, x, CAVE_light, light); // turn off fm

  lite_spot(y, x);// not implemented yet
  if (test_light(y, x))// not implemented yet
    return true;

  return false;
}


/* Allocates objects upon a creatures death		-RAK-	*/
/* Oh well,  another creature bites the dust.  Reward the victor*/
/* based on flags set in the main creature record		 */
/* Returns a mask of bits from the given flags which indicates what the
   monster is seen to have dropped.  This may be added to monster memory. */
ULong monster_death(Short y, Short x, ULong flags)
{
  Short i, number;
  ULong dump, res;

  if (flags & CM_CARRY_OBJ)
    i = 1;
  else
    i = 0;
  if (flags & CM_CARRY_GOLD)
    i += 2;
  if (flags & CM_SMALL_OBJ)
    i += 4;

  number = 0;
  if ((flags & CM_60_RANDOM) && (randint(100) < 60))
    number++;
  if ((flags & CM_90_RANDOM) && (randint(100) < 90))
    number++;
  if (flags & CM_1D2_OBJ)
    number += randint(2);
  if (flags & CM_2D2_OBJ)
    number += damroll(2, 2);
  if (flags & CM_4D2_OBJ)
    number += damroll(4, 2);
  if (number > 0)
    dump = summon_object(y, x, number, i);
  else
    dump = 0;

  if (flags & CM_WIN)
    if (!death) /* maybe the player died in mid-turn */ {
      total_winner = true;
      //      prt_winner(); // xxx not implemented yet
      message("*** CONGRATULATIONS *** You have won the game.");
      message("You cannot save this game, but you may retire when ready.");
    }

  if (dump) {
    res = 0;
    if (dump & 255) {
      res |= CM_CARRY_OBJ;
      if (i & 0x04)
	res |= CM_SMALL_OBJ;
    }
    if (dump >= 256)
      res |= CM_CARRY_GOLD;
    dump = (dump % 256) + (dump / 256);  /* number of items */
    res |= dump << CM_TR_SHIFT;
  } else
    res = 0;

  return res;
}



/* Decreases monsters hit points and deletes monster if needed.	*/
/* (Picking on my babies.)			       -RAK-   */
Short mon_take_hit(Short monptr, Short dam)
{
  ULong i;
  Long new_exp, new_exp_frac;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Short m_take_hit;
  ULong tmp;

  m_ptr = &m_list[monptr];
  mlist_set_short(monptr, MLIST_hp, m_ptr->hp - dam);
  mlist_set_short(monptr, MLIST_csleep, 0);
  if (m_ptr->hp < 0) {
    i = monster_death((Short) m_ptr->fy, (Short) m_ptr->fx,
		      c_list[m_ptr->mptr].cmove);
    if ((pyflags.blind < 1 && m_ptr->ml)
	|| (c_list[m_ptr->mptr].cmove & CM_WIN)) {
      tmp = (c_recall[m_ptr->mptr].r_cmove & CM_TREASURE) >> CM_TR_SHIFT;
      if (tmp > ((i & CM_TREASURE) >> CM_TR_SHIFT))
	i = (i & ~CM_TREASURE) | (tmp << CM_TR_SHIFT);
      recall_set_cmove(m_ptr->mptr,
		       (c_recall[m_ptr->mptr].r_cmove & ~CM_TREASURE) | i);
      if (c_recall[m_ptr->mptr].r_kills < MAX_SHORT)
	recall_increment_kills(m_ptr->mptr);
    }
    r_ptr = &c_list[m_ptr->mptr];

    new_exp = ((Long) r_ptr->mexp * r_ptr->level) / pymisc.lev;
    new_exp_frac = ((((Long) r_ptr->mexp * r_ptr->level) % pymisc.lev)
		    * 0x10000L / pymisc.lev) + pymisc.exp_frac;
    if (new_exp_frac >= 0x10000L) {
      new_exp++;
      pymisc.exp_frac = new_exp_frac - 0x10000L;
    } else
      pymisc.exp_frac = new_exp_frac;

    pymisc.exp += new_exp;
    /* can't call prt_experience() here, as that would result in "new level"
       message appearing before "monster dies" message */
    m_take_hit = m_ptr->mptr;
    /* in case this is called from within creatures(), this is a
       horrible hack, the m_list/creatures() code needs to be
       rewritten */
    if (hack_monptr < monptr)
      delete_monster(monptr);
    else
      fix1_delete_monster(monptr);
  } else
    m_take_hit = -1;
  return m_take_hit;
}


/* Player attacks a (poor, defenseless) creature	-RAK-	*/
void py_attack(Short y, Short x)
{
  Short k, blows;
  Short crptr, monptr, tot_tohit, base_tohit;
  Char m_name[80], out_val[80];
  inven_type *i_ptr;
  UChar conf;

  crptr = cave_cptr[y*MAX_WIDTH+x];
  monptr = m_list[crptr].mptr;
  mlist_set_short(crptr, MLIST_csleep, 0);
  i_ptr = &inventory[INVEN_WIELD];
  /* Does the player know what he's fighting?	   */
  if (!m_list[crptr].ml)
    StrCopy(m_name, "it");
  else
    StrPrintF(m_name, "the %s", c_names + c_list[monptr].name_offset);
  if (i_ptr->tval != TV_NOTHING)		/* Proper weapon */
    blows = attack_blows((Short) i_ptr->weight, &tot_tohit);
  else					  /* Bare hands?   */ {
    blows = 2;
    tot_tohit = -3;
  }
  if ((i_ptr->tval >= TV_SLING_AMMO) && (i_ptr->tval <= TV_SPIKE))
    /* Fix for arrows */
    blows = 1;
  tot_tohit += pymisc.ptohit;
  /* if creature not lit, make it more difficult to hit */
  if (m_list[crptr].ml)
    base_tohit = pymisc.bth;
  else
    base_tohit = (pymisc.bth / 2) - (tot_tohit * (BTH_PLUS_ADJ-1))
      - (pymisc.lev * class_level_adj[pymisc.pclass][CLA_BTH] / 2);

  /* Loop for number of blows,	trying to hit the critter.	  */
  while (blows-- > 0) {
    if (test_hit(base_tohit, (Short) pymisc.lev, tot_tohit,
		 (Short) c_list[monptr].ac, CLA_BTH)) {
      StrPrintF(out_val, "You hit %s.", m_name);
      message(out_val);
      if (i_ptr->tval != TV_NOTHING) {
	k = pdamroll(i_ptr->damage);
	k = tot_dam(i_ptr, k, monptr);
	k = critical_blow((Short) i_ptr->weight, tot_tohit, k, CLA_BTH);
      } else			      /* Bare hands!?  */ {
	k = damroll(1, 1);
	k = critical_blow(1, 0, k, CLA_BTH);
      }
      k += pymisc.ptodam;
      if (k < 0)  k = 0;

      if (pyflags.confuse_monster) {
	pyflags.confuse_monster = false;
	message("Your hands stop glowing.");
	if ((c_list[monptr].cdefense & CD_NO_SLEEP)
	    || (randint(MAX_MONS_LEVEL) < c_list[monptr].level))
	  StrPrintF(out_val, "%s is unaffected.", m_name);
	else {
	  StrPrintF(out_val, "%s appears confused.", m_name);
	  conf = m_list[crptr].confused;
	  if (conf)
	    mlist_set_uchar(crptr, MLIST_confused, conf + 3);
	  else
	    mlist_set_uchar(crptr, MLIST_confused, 2 + randint(16));
	}
	message(out_val);
	if (m_list[crptr].ml && randint(4) == 1)
	  recall_cdefense_or(monptr, c_list[monptr].cdefense & CD_NO_SLEEP);
      }

      /* See if we done it in.				 */
      if (mon_take_hit(crptr, k) >= 0) {
	StrPrintF(out_val, "You have slain %s.", m_name);
	message(out_val);
	print_stats(STATS_QUO); //prt_experience();
	blows = 0;
      }

      if ((i_ptr->tval >= TV_SLING_AMMO)
	  && (i_ptr->tval <= TV_SPIKE)) /* Use missiles up*/ {
	invy_set_number(inventory, INVEN_WIELD, i_ptr->number - 1);
	inven_weight -= i_ptr->weight;
	pyflags.status |= PY_STR_WGT;
	if (i_ptr->number == 0) {
	  equip_ctr--;
	  py_bonuses(i_ptr, -1);
	  //  since i_ptr = &inventory[INVEN_WIELD];
	  invcopy(inventory, INVEN_WIELD, OBJ_NOTHING);
	  calc_bonuses();
	}
      }
    } else {
      StrPrintF(out_val, "You miss %s.", m_name);
      message(out_val);
    }
  }
}


/* Moves player from one space to another.		-RAK-	*/
/* Note: This routine has been pre-declared; see that for argument*/
void move_char(Short dir, Boolean do_pickup)
{
  Short old_row, old_col, old_find_flag;
  Short y, x;
  Short i, j;
  UChar floor, light, thing, critter, thing2;

  if ((pyflags.confused > 0) &&    /* Confused?	     */
      (randint(4) > 1) &&	    /* 75% random movement   */
      (dir != 5))		    /* Never random if sitting*/ {
    dir = randint(9);
    end_find();
  }
  y = char_row;
  x = char_col;
  if (mmove(dir, &y, &x))    /* Legal move?	      */ {
    critter = cave_cptr[y*MAX_WIDTH+x];
    thing = cave_tptr[y*MAX_WIDTH+x];
    floor = cave_fval[y*MAX_WIDTH+x];
    light = cave_light[y*MAX_WIDTH+x];
    /* if there is no creature, or an unlit creature in the walls then... */
    /* disallow attacks against unlit creatures in walls because moving into
       a wall is a free turn normally, hence don't give player free turns
       attacking each wall in an attempt to locate the invisible creature,
       instead force player to tunnel into walls which always takes a turn */
    if ((critter < 2) || (!m_list[critter].ml && floor >= MIN_CLOSED_SPACE)) {
      if (floor <= MAX_OPEN_SPACE) /* Open floor spot	*/ {
	/* Make final assignments of char co-ords */
	old_row = char_row;
	old_col = char_col;
	char_row = y;
	char_col = x;
	/* Move character record (-1)	       */
	move_rec(old_row, old_col, char_row, char_col);

	/* Check for new panel		       */
	// basically at this point check and see if we walked off the edge
	if (get_panel(char_row, char_col, false))
	  recalc_screen(); // prt_map();

	/* Check to see if he should stop	       */
	if (find_flag)
	  area_affect(dir, char_row, char_col);
	/* Check to see if he notices something  */
	/* fos may be negative if have good rings of searching */
	if ((pymisc.fos <= 1) || (randint(pymisc.fos) == 1) ||
	    (pyflags.status & PY_SEARCH))
	  search(char_row, char_col, pymisc.srh);
	/* A room of light should be lit.	     */
	if (floor == LIGHT_FLOOR) {
	  if (!(light & LIGHT_pl) && !pyflags.blind)
	    light_room(char_row, char_col);
	} else if ((light & LIGHT_lr) && (pyflags.blind < 1))
	  /* In doorway of light-room?	       */
	  for (i = (char_row - 1); i <= (char_row + 1); i++)
	    for (j = (char_col - 1); j <= (char_col + 1); j++) {
	      UChar floor2, light2;
	      floor2 = cave_fval[i*MAX_WIDTH+j];
	      light2 = cave_light[i*MAX_WIDTH+j];
	      if ((floor2 == LIGHT_FLOOR) && (!(light2 & LIGHT_pl)))
		light_room(i, j);
	    }
	/* Move the light source		       */
	move_light(old_row, old_col, char_row, char_col);
	/* An object is beneath him.	     */
	if (thing != 0) {
	  carry(char_row, char_col, do_pickup);
	  /* if stepped on falling rock trap, and space contains
	     rubble, then step back into a clear area */
	  if (t_list[thing].tval == TV_RUBBLE) {
	    move_rec(char_row, char_col, old_row, old_col);
	    move_light(char_row, char_col, old_row, old_col);
	    char_row = old_row;
	    char_col = old_col;
	    /* check to see if we have stepped back onto another
	       trap, if so, set it off */
	    thing2 = cave_tptr[char_row*MAX_WIDTH+char_col];
	    if (thing2 != 0) {
	      i = t_list[thing2].tval;
	      if (i == TV_INVIS_TRAP || i == TV_VIS_TRAP
		  || i == TV_STORE_DOOR)
		hit_trap(char_row, char_col);
	    }
	  }
	}
      } else {
	/*Can't move onto floor space*/
	if (!find_flag && (thing != 0)) {
	  if (t_list[thing].tval == TV_RUBBLE)
	    message("There is rubble blocking your way.");
	  else if (t_list[thing].tval == TV_CLOSED_DOOR)
	    message("There is a closed door blocking your way.");
	} else
	  end_find();
	free_turn_flag = true;
      }
    } else {
      /* Attacking a creature! */
      old_find_flag = find_flag;
      end_find();
      /* if player can see monster, and was in find mode, then nothing */
      if (m_list[critter].ml && old_find_flag) {
	/* did not do anything this turn */
	free_turn_flag = true;
      } else {
	if (pyflags.afraid < 1)		/* Coward?	*/
	  py_attack(y, x);
	else				/* Coward!	*/
	  message("You are too afraid!");
      }
    }
  }
}


/* Chests have traps too.				-RAK-	*/
/* Note: Chest traps are based on the FLAGS value		 */
void chest_trap(Short y, Short x)
{
  Short i;
  Short j, k;
  inven_type *t_ptr;

  t_ptr = &t_list[cave_tptr[y*MAX_WIDTH+x]];
  if (CH_LOSE_STR & t_ptr->flags) {
    message("A small needle has pricked you!");
    if (!pyflags.sustain_str) {
      dec_stat(A_STR);
      take_hit(damroll(1, 4), "a poison needle");
      message("You feel weakened!");
    } else
      message("You are unaffected.");
  }
  if (CH_POISON & t_ptr->flags) {
    message("A small needle has pricked you!");
    take_hit(damroll(1, 6), "a poison needle");
    pyflags.poisoned += 10 + randint(20);
  }
  if (CH_PARALYSED & t_ptr->flags) {
    message("A puff of yellow gas surrounds you!");
    if (pyflags.free_act)
      message("You are unaffected.");
    else {
      message("You choke and pass out.");
      pyflags.paralysis = 10 + randint(20);
    }
  }
  if (CH_SUMMON & t_ptr->flags) {
    for (i = 0; i < 3; i++) {
      j = y;
      k = x;
      summon_monster(&j, &k, false);
    }
  }
  if (CH_EXPLODE & t_ptr->flags) {
    message("There is a sudden explosion!");
    delete_object(y, x);
    take_hit(damroll(5, 8), "an exploding chest");
  }
}


/* Opens a closed door or closed chest.		-RAK-	*/
void openobject(Short dir)
{
  Short y, x, i;
  Boolean flag, no_object;
  inven_type *t_ptr;
  monster_type *m_ptr;
  Char m_name[80], out_val[80];
  UChar thing, critter, light;

  y = char_row;
  x = char_col;
  //  if (!get_dir(CNIL, &dir)) return; // xxx not implemented yet
  mmove(dir, &y, &x);
  thing = cave_tptr[y*MAX_WIDTH+x];
  critter = cave_cptr[y*MAX_WIDTH+x];
  light = cave_light[y*MAX_WIDTH+x];
  no_object = false;
  if (critter > 1 && thing != 0 &&
      (t_list[thing].tval == TV_CLOSED_DOOR
       || t_list[thing].tval == TV_CHEST)) {
    m_ptr = &m_list[critter];
    if (m_ptr->ml)
      StrPrintF (m_name, "The %s",c_names + c_list[m_ptr->mptr].name_offset);
    else
      StrCopy (m_name, "Something");
    StrPrintF(out_val, "%s is in your way!", m_name);
    message(out_val);
  } else if (thing != 0)
    /* Closed door		 */
    if (t_list[thing].tval == TV_CLOSED_DOOR) {
      t_ptr = &t_list[thing];
      if (t_ptr->p1 > 0)	 /* It's locked.	*/ {
	i = pymisc.disarm + 2*todis_adj() + stat_adj(A_INT)
	  + (class_level_adj[pymisc.pclass][CLA_DISARM]
	     * pymisc.lev / 3);
	if (pyflags.confused > 0)
	  message("You are too confused to pick the lock.");
	else if ((i-t_ptr->p1) > randint(100)) {
	  message("You have picked the lock.");
	  pymisc.exp++;
	  print_stats(STATS_QUO);//	    prt_experience();
	  tlist_set_p1(t_list, thing, 0);
	} else
	  count_message("You failed to pick the lock.");
      } else if (t_ptr->p1 < 0)	 /* It's stuck	  */
	message("It appears to be stuck.");
      if (t_ptr->p1 == 0) {
	invcopy(t_list, thing, OBJ_OPEN_DOOR);
	cave_set(y, x, CAVE_fval, CORR_FLOOR);
	lite_spot(y, x);
	command_count = 0; // stop trying to open it
      }
    }
  /* Open a closed chest.		     */
    else if (t_list[thing].tval == TV_CHEST) {
      i = pymisc.disarm + 2*todis_adj() + stat_adj(A_INT)
	+ (class_level_adj[pymisc.pclass][CLA_DISARM] * pymisc.lev / 3);
      t_ptr = &t_list[thing];
      flag = false;
      if (CH_LOCKED & t_ptr->flags)
	if (pyflags.confused > 0)
	  message("You are too confused to pick the lock.");
	else if ((i - (Short)t_ptr->level) > randint(100)) {
	  message("You have picked the lock.");
	  flag = true;
	  pymisc.exp += t_ptr->level;
	  print_stats(STATS_QUO); // prt_experience();
	} else
	  count_message("You failed to pick the lock.");
      else
	flag = true;
      if (flag) {
	tlist_set_flags(t_list, thing, t_ptr->flags & ~CH_LOCKED);
	tlist_set_name2(t_list, thing, SN_EMPTY);
	known2(t_list, thing);
	tlist_set_cost(t_list, thing, 0);
      }
      flag = false;
      /* Was chest still trapped?	 (Snicker)   */
      if ((CH_LOCKED & t_ptr->flags) == 0) {
	chest_trap(y, x);
	if (thing != 0)
	  flag = true;
      }
      /* Chest treasure is allocated as if a creature   */
      /* had been killed.				   */
      if (flag) {
	/* clear the cursed chest/monster win flag, so that people
	   can not win by opening a cursed chest */
	  
	tlist_set_flags(t_list, thing,
			t_list[thing].flags & ~TR_CURSED);
	monster_death(y, x, t_list[thing].flags);
	tlist_set_flags(t_list, thing, 0);
      }
    } else
      no_object = true;
  else
    no_object = true;

  if (no_object) {
    message("I do not see anything you can open there.");
    free_turn_flag = true;
  }
}


/* Closes an open door.				-RAK-	*/
void closeobject(Short dir)
{
  Short y, x;
  Boolean no_object;
  Char out_val[80], m_name[80];
  monster_type *m_ptr;
  UChar thing, critter;

  y = char_row;
  x = char_col;
  //  if (!get_dir(CNIL, &dir)) return; // xxx not implemented yet
  mmove(dir, &y, &x);
  thing = cave_tptr[y*MAX_WIDTH+x];
  critter = cave_cptr[y*MAX_WIDTH+x];
  no_object = false;
  if (thing != 0)
    if (t_list[thing].tval == TV_OPEN_DOOR)
      if (critter == 0)
	if (t_list[thing].p1 == 0) {
	  invcopy(t_list, thing, OBJ_CLOSED_DOOR);
	  cave_set(y, x, CAVE_fval, BLOCKED_FLOOR);
	  lite_spot(y, x);
	} else
	  message("The door appears to be broken.");
      else {
	m_ptr = &m_list[critter];
	if (m_ptr->ml)
	  StrPrintF (m_name, "The %s",
		     c_names + c_list[m_ptr->mptr].name_offset);
	else
	  StrCopy (m_name, "Something");
	StrPrintF(out_val, "%s is in your way!", m_name);
	message(out_val);
      } else
	no_object = true;
  else
    no_object = true;

  if (no_object) {
    message("I do not see anything you can close there.");
    free_turn_flag = true;
  }
}

/* Tunneling through real wall: 10, 11, 12		-RAK-	*/
/* Used by TUNNEL and WALL_TO_MUD				 */
Boolean twall(Short y, Short x, Short t1, Short t2)
{
  Short i, j;
  Boolean res, found;
  UChar light, floor, thing;

  res = false;
  if (t1 > t2) {
    light = cave_light[y*MAX_WIDTH+x];
    thing = cave_tptr[y*MAX_WIDTH+x];
    if (light & LIGHT_lr) {
      /* should become a room space, check to see whether it should be
	 LIGHT_FLOOR or DARK_FLOOR */
      found = false;
      for (i = y-1; i <= y+1; i++)
	for (j = x-1; j <= x+1; j++) {
	  floor = cave_fval[i*MAX_WIDTH+j];
	  if (floor <= MAX_CAVE_ROOM) {
	    cave_set(y, x, CAVE_fval, floor);
	    light |= (LIGHT_pl & cave_light[i*MAX_WIDTH+j]);// make pl same:
	    //cave_set(y, x, CAVE_light, light);//  c_ptr->pl = cave[i][j].pl;
	    found = true;
	    break;
	  }
	}
      if (!found) {
	cave_set(y, x, CAVE_fval, CORR_FLOOR);
	light &= ~LIGHT_pl;
	//cave_set(y, x, CAVE_light, light);//postponed
      }
    } else {
      /* should become a corridor space */
      cave_set(y, x, CAVE_fval, CORR_FLOOR);
      light &= ~LIGHT_pl;
      //cave_set(y, x, CAVE_light, light);// postponed
    }
    light &= ~LIGHT_fm;// c_ptr->fm = false;
    cave_set(y, x, CAVE_light, light); // de-postponed
    if (panel_contains(y, x))
      if ((light & (LIGHT_tl | LIGHT_pl)) && thing != 0)
	message("You have found something!");
    lite_spot(y, x);
    res = true;
  }
  return res;
}
