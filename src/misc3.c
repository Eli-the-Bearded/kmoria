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

// needed: 
// tlist_set_cost

extern ULong spell_learned; /* bit mask of spells learned */
extern ULong spell_forgotten;        /* Bit field for spells forgotten -JEW- */
UChar spell_order[MAX_SPELLS+1];/* remember order that spells are learned in */

// This is only used here, and besides, it's only 42 bytes
// So no need for it to be a database record
/* used to calculate the number of blows the player gets in combat */
UChar blows_table[7][6] = {
/* STR/W:	   9  18  67 107 117 118   : DEX */
/* <2 */	{  1,  1,  1,  1,  1,  1 },
/* <3 */	{  1,  1,  1,  1,  2,  2 },
/* <4 */	{  1,  1,  1,  2,  2,  3 },
/* <5 */	{  1,  1,  2,  2,  3,  3 },
/* <7 */	{  1,  2,  2,  3,  3,  4 },
/* <9 */	{  1,  2,  2,  3,  4,  4 },
/* >9 */	{  2,  2,  3,  3,  4,  4 }
};

static void gain_level() SEC_4;


/* Places a particular trap at location y, x		-RAK-	*/
void place_trap(Short y, Short x, Short subval)
{
  Short cur_pos;

  cur_pos = popt();
  cave_set(y, x, CAVE_tptr, cur_pos);
  invcopy(t_list, cur_pos, OBJ_TRAP_LIST + subval);
}

/* Places rubble at location y, x			-RAK-	*/
void place_rubble(Short y, Short x)
{
  Short cur_pos;

  cur_pos = popt();
  cave_set(y, x, CAVE_tptr, cur_pos);
  cave_set(y, x, CAVE_fval, BLOCKED_FLOOR);
  invcopy(t_list, cur_pos, OBJ_RUBBLE);
}

/* Places a treasure (Gold or Gems) at given row, column -RAK-	*/
void place_gold(Short y, Short x)
{
  Short i, cur_pos;
  Long cost;
  //  inven_type *t_ptr;
  //  StrCopy(moron_caller, "place_gold");

  cur_pos = popt();
  i = ((randint(dun_level+2)+2) / 2) - 1;
  if (randint(OBJ_GREAT) == 1)
    i += randint(dun_level+1);
  if (i >= MAX_GOLD)
    i = MAX_GOLD - 1;
  //  cave[y][x].tptr = cur_pos;
  cave_set(y, x, CAVE_tptr, cur_pos);
  invcopy(t_list, cur_pos, OBJ_GOLD_LIST+i);
  cost = t_list[cur_pos].cost;
  cost += (8L * (Long)randint((Int)cost)) + randint(8);
  tlist_set_cost(t_list, cur_pos, cost);
  if (cave_cptr[y*MAX_WIDTH+x] == 1)
    message("You feel something roll beneath your feet.");
}

/* Returns the array number of a random object		-RAK-	*/
Short get_obj_num(Short level, Boolean must_be_small)
{
  Short i, j;

  if (level <= 0)
    i = randint(t_level[0]) - 1;
  else {
    if (level >= MAX_OBJ_LEVEL)
      level = MAX_OBJ_LEVEL;
    else if (randint(OBJ_GREAT) == 1) {
      level = level * MAX_OBJ_LEVEL / randint (MAX_OBJ_LEVEL) + 1;
      if (level > MAX_OBJ_LEVEL)
	level = MAX_OBJ_LEVEL;
    }

    /* This code has been added to make it slightly more likely to get the
       higher level objects.	Originally a uniform distribution over all
       objects less than or equal to the dungeon level.  This distribution
       makes a level n objects occur approx 2/n% of the time on level n,
       and 1/2n are 0th level. */
    do {
      if (randint(2) == 1)
	i = rund(t_level[level]);
      else { /* Choose three objects, pick the highest level. */
	i = rund(t_level[level]);
	j = rund(t_level[level]);
	if (i < j)
	  i = j;
	j = rund(t_level[level]);
	if (i < j)
	  i = j;
	j = object_list[sorted_objects[i]].level;
	if (j == 0)
	  i = rund(t_level[0]);
	else
	  i = rund(t_level[j]-t_level[j-1]) + t_level[j-1];
      }
    } while ( must_be_small 
	      && is_large(object_list+sorted_objects[i]) );
  }
  return i;
}

/* Places an object at given row, column co-ordinate	-RAK-	*/
void place_object(Short y, Short x, Boolean must_be_small)
{
  Short cur_pos, tmp;

  cur_pos = popt();
  cave_set(y, x, CAVE_tptr, cur_pos);
  /* split this line up to avoid a reported compiler bug */
  tmp = get_obj_num(dun_level, must_be_small);
  invcopy(t_list, cur_pos, sorted_objects[tmp]);
  magic_treasure(cur_pos, dun_level);
  if (cave_cptr[y*MAX_WIDTH+x] == 1)
    message("You feel something roll beneath your feet.");	/* -CJS- */
}

/* Allocates an object for tunnels and rooms		-RAK-	*/
// alloc_set may be: corr, room, floor.
void alloc_object(UChar alloc_set, Short typ, Short num)
{
  Short i=0, j=0, k;
  Boolean ok = false;
  UChar floor;

  for (k = 0; k < num; k++) {
    /* don't put an object beneath the player, this could cause problems
       if player is standing under rubble, or on a trap */
    while (!ok) {
      i = randint(cur_height) - 1;
      j = randint(cur_width) - 1;
      if (i == char_row && j == char_col) continue; // player is there
      if (cave_tptr[i*MAX_WIDTH+j] != 0) continue;  // object is there
      floor = cave_fval[i*MAX_WIDTH+j];
      switch(alloc_set) {
      case IS_CORR:  ok = is_corr(floor);   break;
      case IS_ROOM:  ok = is_room(floor);   break;
      case IS_FLOOR: ok = is_floor(floor);  break;
      default:       ok = true; // caller is a freak
      }
    } 
    
    if (typ < 4) {	/* typ == 2 not used, used to be visible traps */
      if (typ == 1) place_trap(i, j, randint(MAX_TRAP)-1); /* typ == 1 */
      else          place_rubble(i, j); /* typ == 3 */
    } else {
      if (typ == 4) place_gold(i, j); /* typ == 4 */
      else          place_object(i, j, false); /* typ == 5 */
    }
  }
}

/* Creates objects nearby the coordinates given		-RAK-	*/
void random_object(Short y, Short x, Short num)
{
  Short tries, j, k;

  while (num-- > 0) {
    // create 'num' random objects
    for (tries = 0 ; tries < 10 ; tries++) {
      j = y - 3 + randint(5);
      k = x - 4 + randint(7);
      if (in_bounds(j, k)
	  && (cave_fval[j*MAX_WIDTH+k] <= MAX_CAVE_FLOOR)
	  && (cave_tptr[j*MAX_WIDTH+k] == 0)) {
	if (randint(100) < 75)	  place_object(j, k, false);
	else                      place_gold(j, k);
	break;
      }
    }
  }
}


// cnv_stat converts a 0-18 or 18/01-100 UChar stat to a Char[].
// but that belongs in display.c

// prt_stat prints character stat in given row_column.
// prt_field prints character info string Char[13] in given row_column.
// prt_lnum prints 6-digit Long + header in given row_column.
// prt_7lnum prints 7-digit Long + header in given row_column.
// prt_num prints 6-digit Short + header in given row_column.
// prt_long prints 6-digit Long (no header) in given row_column.
// prt_int prints 6-digit Short (no header) in given row_column.
// but ALL OF THAT belongs in display.c, where we print stats!

/* Adjustment for wisdom/intelligence				-JWT-	*/
Short stat_adj(Short stat)
{
  Short value = pystats.use_stat[stat];
  if (value > 117)        return 7;
  else if (value > 107)   return 6;
  else if (value > 87)    return 5;
  else if (value > 67)    return 4;
  else if (value > 17)    return 3;
  else if (value > 14)    return 2;
  else if (value > 7)     return 1;
  else                    return 0;
}

/* Adjustment for charisma				-RAK-	*/
/* Percent decrease or increase in price of goods		 */
Short chr_adj()
{
  Short charisma = min(pystats.use_stat[A_CHR], 3);
  if (charisma > 117)         return(90);
  else if (charisma > 107)    return(92);
  else if (charisma > 87)     return(94);
  else if (charisma > 67)     return(96);
  else if (charisma > 18)     return(98);
  else if (charisma > 13)     return 100+(18-charisma);  // 14-18 -> 104-100
  else if (charisma > 4)      return 106+2*(13-charisma);//  5-13 -> 122-106
  else if (charisma > 3)      return 125;  // 4 = 125, 3, = 130
  return 130;
}

/* Returns a character's adjustment to hit points	 -JWT-	 */
Short con_adj()
{
  Short con = pystats.use_stat[A_CON];
  if (con < 7)          return (con - 7);
  else if (con <  17)   return 0;
  else if (con == 17)   return 1;
  else if (con <  94)   return 2;
  else if (con < 117)   return 3;
  else                  return 4;
}

// title_string returns the appropriate title str for the player's experience
// prt_title prints title_string() at a fixed location
// these should be in display.c

// prt_level prints experience level at a fixed location
// prt_cmana prints current mana at a fixed location
// prt_mhp prints max hp at a fixed location
// prt_chp prints current hp at a fixed location
// prt_pac prints current ac at a fixed location
// prt_gold prints current gold at a fixed location
// prt_depth prints current dungeon level depth at a fixed location
// these are subsumed by display.c

// prt_{hunger, blind, confused, afraid, poisoned}
//   prints WEAK/HUNGRY, BLIND, CONFUSED, FEAR, and POISONED stat at fixed loc.
// these should be in display.c

// prt_state prints searching, resting, paralysis, or 'count' status
// these should be in display.c or something

// prt_speed prints very slow / slow / / fast / very fast at a fixed location
// this should be in display.c

// prt_study prints Study at a fixed location.  what is the study flag for?
// this should be in display.c

// prt_winner prints winner, wizard, resurrected, etc status.
// Just skip this, I think.


UChar modify_stat (Short stat, Short amount)
{
  Short loop, i;
  UChar tmp_stat;

  tmp_stat = pystats.cur_stat[stat];

  loop = (amount < 0 ? -amount : amount); // Why not 'loop = abs(amount)' ?

  for (i = 0; i < loop; i++) {
    if (amount > 0) {
      if (tmp_stat < 18)	tmp_stat++;
      else if (tmp_stat < 108)	tmp_stat += 10;
      else                      tmp_stat = 118;
    } else {
      if (tmp_stat > 27)	tmp_stat -= 10;
      else if (tmp_stat > 18)	tmp_stat = 18;
      else if (tmp_stat > 3)	tmp_stat--;
    }
  }
  return tmp_stat;
}


/* Set the value of the stat which is actually used.	 -CJS- */
void set_use_stat(Short stat)
{
  pystats.use_stat[stat] = modify_stat(stat, pystats.mod_stat[stat]);

  // xxx A whole lot of not implemented yet
  if (stat == A_STR) {
    pyflags.status |= PY_STR_WGT;
    calc_bonuses();
  } else if (stat == A_DEX)
    calc_bonuses();
  else if (stat == A_INT && class[pymisc.pclass].spell == MAGE) {
    calc_spells(A_INT);
    calc_mana(A_INT);
  } else if (stat == A_WIS && class[pymisc.pclass].spell == PRIEST) {
    calc_spells(A_WIS);
    calc_mana(A_WIS);
  } else if (stat == A_CON)
    calc_hitpoints();
}

/* Increases a stat by one randomized level		-RAK-	*/
Boolean inc_stat(Short stat)
{
  Short tmp_stat, gain;

  tmp_stat = pystats.cur_stat[stat];
  if (tmp_stat < 118) {
    if (tmp_stat < 18)
      tmp_stat++;
    else if (tmp_stat < 116) {
      /* stat increases by 1/6 to 1/3 of difference from max */
      gain = ((118 - tmp_stat)/3 + 1) >> 1;
      tmp_stat += randint(gain) + gain;
    } else
      tmp_stat++;

    pystats.cur_stat[stat] = tmp_stat;
    // increase maximum (if exceeded)
    if (tmp_stat > pystats.max_stat[stat])
      pystats.max_stat[stat] = tmp_stat;
    set_use_stat(stat);
    print_stats(STATS_STR_ETC);    //      prt_stat(stat);
    return true;
  }
  return false;
}


/* Decreases a stat by one randomized level		-RAK-	*/
Boolean dec_stat(Short stat)
{
  Short tmp_stat, loss;

  tmp_stat = pystats.cur_stat[stat];
  if (tmp_stat > 3) {
    if (tmp_stat < 19)
      tmp_stat--;
    else if (tmp_stat < 117) {
      loss = (((118 - tmp_stat) >> 1) + 1) >> 1;
      tmp_stat += -randint(loss) - loss;
      if (tmp_stat < 18)
	tmp_stat = 18;
    } else
      tmp_stat--;

    pystats.cur_stat[stat] = tmp_stat;
    set_use_stat(stat);
    print_stats(STATS_STR_ETC);    //      prt_stat(stat);
    return true;
  }
  return false;
}


/* Restore a stat.  Return TRUE only if this actually makes a difference. */
Boolean res_stat (Short stat)
{
  Short i;

  i = pystats.max_stat[stat] - pystats.cur_stat[stat];
  if (i != 0) {
    pystats.cur_stat[stat] += i;
    set_use_stat (stat);
    print_stats(STATS_STR_ETC);    //      prt_stat(stat);
    return true;
  }
  return false;
}

/* Boost a stat artificially (by wearing something). If the display argument
   is TRUE, then increase is shown on the screen. */
void bst_stat (Short stat, Short amount)
{
  pystats.mod_stat[stat] += amount;

  set_use_stat (stat);
  /* can not call prt_stat() here, may be in store, may be in inven_command */
  pyflags.status |= (PY_STR << stat);
}

/* Returns a character's adjustment to hit.		 -JWT-	 */
Short tohit_adj()
{
  Short total, stat;

  stat = pystats.use_stat[A_DEX];
  if	  (stat <   4)	total = -3;
  else if (stat <   6)	total = -2;
  else if (stat <   8)	total = -1;
  else if (stat <  16)	total =	 0;
  else if (stat <  17)	total =	 1;
  else if (stat <  18)	total =	 2;
  else if (stat <  69)	total =	 3;
  else if (stat < 118)	total =	 4;
  else			total =	 5;
  stat = pystats.use_stat[A_STR];
  if	  (stat <   4)	total -= 3;
  else if (stat <   5)	total -= 2;
  else if (stat <   7)	total -= 1;
  else if (stat <  18)	total -= 0;
  else if (stat <  94)	total += 1;
  else if (stat < 109)	total += 2;
  else if (stat < 117)	total += 3;
  else			total += 4;
  return total;
}

/* Returns a character's adjustment to armor class	 -JWT-	 */
Short toac_adj()
{
  Short stat;

  stat = pystats.use_stat[A_DEX];
  if	  (stat <   4)	return(-4);
  else if (stat ==  4)	return(-3);
  else if (stat ==  5)	return(-2);
  else if (stat ==  6)	return(-1);
  else if (stat <  15)	return( 0);
  else if (stat <  18)	return( 1);
  else if (stat <  59)	return( 2);
  else if (stat <  94)	return( 3);
  else if (stat < 117)	return( 4);
  else			return( 5);
}


/* Returns a character's adjustment to disarm		 -RAK-	 */
Short todis_adj()
{
  Short stat;

  stat = pystats.use_stat[A_DEX];
  if	  (stat <   4)	return(-8);
  else if (stat ==  4)	return(-6);
  else if (stat ==  5)	return(-4);
  else if (stat ==  6)	return(-2);
  else if (stat ==  7)	return(-1);
  else if (stat <  13)	return( 0);
  else if (stat <  16)	return( 1);
  else if (stat <  18)	return( 2);
  else if (stat <  59)	return( 4);
  else if (stat <  94)	return( 5);
  else if (stat < 117)	return( 6);
  else			return( 8);
}


/* Returns a character's adjustment to damage		 -JWT-	 */
Short todam_adj()
{
  Short stat;

  stat = pystats.use_stat[A_STR];
  if	  (stat <   4)	return(-2);
  else if (stat <   5)	return(-1);
  else if (stat <  16)	return( 0);
  else if (stat <  17)	return( 1);
  else if (stat <  18)	return( 2);
  else if (stat <  94)	return( 3);
  else if (stat < 109)	return( 4);
  else if (stat < 117)	return( 5);
  else			return( 6);
}


// prt_stat_block: print all stats
// I am doing this in display.c instead


// draw_cave: draw dungeon and print stats (and depth)
// I am doing this in display.c instead
void draw_cave() {
  recalc_screen();
  refresh();
  print_stats(STATS_QUO); // probably unnecessary
}


// put_character: for character generation!
// put_stats: ditto
// likert: ditto
// put_misc1: age, height, weight, social class
// put_misc2: level, exp, max exp, exp to advance, gold, max/cur hp/man
// put_misc3: ability ratings
// I will have a form for displaying these...

// get_name
// change_name
// This should be in character generation and prefs forms.



// INVENTORY


/* Destroy an item in the inventory			-RAK-	*/
void inven_destroy(Short item_val)
{
  inven_type *i_ptr;

  i_ptr = &inventory[item_val];
  if ((i_ptr->number > 1) && (i_ptr->subval <= ITEM_SINGLE_STACK_MAX)) {
    invy_set_number(inventory, item_val, i_ptr->number-1);
    inven_weight -= i_ptr->weight;
  } else {
    inven_weight -= i_ptr->weight*i_ptr->number;
    invy_delete(item_val);
  }
  pyflags.status |= PY_STR_WGT;
}

/* Copies the object in the second argument over the first argument.
   However, the second always gets a number of one except for ammo etc. */
// NOTE! very important!  s_ptr is assumed not to be a database record
// but rather to be a global or stack or heap variable.
void take_one_item(inven_type *s_ptr, inven_type *i_ptr)
{
  *s_ptr = *i_ptr; // do not use DmWrite on s_ptr...
  if ((s_ptr->number > 1) && (s_ptr->subval >= ITEM_SINGLE_STACK_MIN)
      && (s_ptr->subval <= ITEM_SINGLE_STACK_MAX))
    s_ptr->number = 1; // do not use invy_set_number on s_ptr...
  // used only in store2 though.
}


/* Drops an item from inventory to given location	-RAK-	*/
void inven_drop(Short item_val, Boolean drop_all)
{
  Short i;
  inven_type *i_ptr;
  Char prt2[80];
  Char prt1[160];

  if (cave_tptr[char_row*MAX_WIDTH+char_col] != 0)
    delete_object(char_row, char_col);
  i = popt();
  i_ptr = &inventory[item_val];
  DmWrite(t_list, i*sizeof(inven_type), i_ptr, sizeof(inven_type));//t_list[i] = *i_ptr;
  cave_set(char_row, char_col, CAVE_tptr, i);

  if (item_val >= INVEN_WIELD)
    takeoff(item_val, -1);
  else {
    if (drop_all || i_ptr->number == 1) {
      inven_weight -= i_ptr->weight*i_ptr->number;
      invy_delete(item_val);
    } else {
      invy_set_number(t_list, i, 1);
      inven_weight -= i_ptr->weight;
      invy_set_number(inventory, item_val, i_ptr->number-1);
    }
    objdes(prt1, &t_list[i], true);
    StrPrintF(prt2, "Dropped %s", prt1);
    message(prt2);
  }
  pyflags.status |= PY_STR_WGT;
}


/* Destroys a type of item on a given percent chance	-RAK-	*/
Short inven_damage(Boolean (*typ)(), Short perc)
{
  Short i, j;

  j = 0;
  for (i = 0; i < inven_ctr; i++)
    if ((*typ)(&inventory[i]) && (randint(100) < perc)) {
      inven_destroy(i);
      j++;
    }
  return(j);
}

/*
#define CORRODES 1
#define FLAMMABLE 2
#define FROST_DESTROY 3
#define LIGHTNING_DESTROY 4
#define ACID_AFFECT 5
Short inven_damage(UChar typ, Short perc)
{
  Short i, j;
  Boolean affected;
  inven_type *i_ptr;
  if (typ == 0) return 0; // save time..
  j = 0;
  for (i = 0; i < inven_ctr; i++) {
    affected = false;
    i_ptr = &inventory[i];
    switch(typ) {
    case CORRODES:           affected = is_corrodes(i_ptr);          break;
    case FLAMMABLE:          affected = is_flammable(i_ptr);         break;
    case FROST_DESTROY:      affected = is_frost_destroy(i_ptr);     break;
    case LIGHTNING_DESTROY:  affected = is_lightning_destroy(i_ptr); break;
    case ACID_AFFECT:        affected = is_acid_affect(i_ptr);       break;
    }
    if (affected && (randint(100) < perc)) {
      inven_destroy(i);
      j++;
    }
  }
  return j;
}
*/

/* Computes current weight limit			-RAK-	*/
Short weight_limit()
{
  Short weight_cap;

  weight_cap = pystats.use_stat[A_STR] * PLAYER_WEIGHT_CAP + pymisc.wt;
  weight_cap = min(weight_cap, 3000);
  return weight_cap;
}


Boolean inven_similar(inven_type *a_ptr, inven_type *b_ptr)
{
  if (a_ptr->tval == b_ptr->tval &&
      a_ptr->subval == b_ptr->subval &&
      /* make sure the number field doesn't overflow */
      ((Short)a_ptr->number + (Short)b_ptr->number < 256) &&
      /* they always stack (subval < 192), or else they have same p1 */
      ((b_ptr->subval < ITEM_GROUP_MIN) || (a_ptr->p1 == b_ptr->p1))
      /* only stack if both or neither are identified */
      && (known1_p(a_ptr) == known1_p(b_ptr)))
    return true;
  return false;
}

/* this code must be identical to the inven_carry() code below */
Boolean inven_check_num (inven_type *t_ptr)
{
  register int i;

  if (inven_ctr < INVEN_WIELD)
    return true;
  else if (t_ptr->subval >= ITEM_SINGLE_STACK_MIN)
    for (i = 0; i < inven_ctr; i++)
      if (inven_similar(&inventory[i], t_ptr))
	return true;
  return false;
}


/* return FALSE if picking up an object would change the players speed */
Boolean inven_check_weight(inven_type *i_ptr)
{
  Short i, new_inven_weight;

  i = weight_limit();
  new_inven_weight = i_ptr->number*i_ptr->weight + inven_weight;
  if (i < new_inven_weight)
    i = new_inven_weight / (i + 1);
  else
    i = 0;

  if (pack_heavy != i)
    return false;
  else
    return true;
}

/* Are we strong enough for the current pack and weapon?  -CJS-	 */
void check_strength()
{
  Short i;
  inven_type *i_ptr;

  i_ptr = &inventory[INVEN_WIELD];
  if (i_ptr->tval != TV_NOTHING
      && (pystats.use_stat[A_STR]*15 < i_ptr->weight)) {
    if (weapon_heavy == false) {
      message("You have trouble wielding such a heavy weapon.");
      weapon_heavy = true;
      //      calc_bonuses();// not implemented yet
    }
  } else if (weapon_heavy == true) {
    weapon_heavy = false;
    if (i_ptr->tval != TV_NOTHING)
      message("You are strong enough to wield your weapon.");
    //    calc_bonuses(); // not implemented yet
  }
  i = weight_limit();
  if (i < inven_weight)
    i = inven_weight / (i+1);
  else
    i = 0;
  if (pack_heavy != i) {
    if (pack_heavy < i)
      message("Your pack is so heavy that it slows you down.");
    else
      message("You move more easily under the weight of your pack.");
    //    change_speed(i - pack_heavy); // not implemented yet
    pack_heavy = i;
  }
  pyflags.status &= ~PY_STR_WGT;
}


/* Add an item to players inventory.  Return the	*/
/* item position for a description if needed.	       -RAK-   */
/* this code must be identical to the inven_check_num() code above */
Short inven_carry(inven_type *i_ptr)
{
  Short locn;
  Short typ, subt;
  inven_type *t_ptr;
  Short known1p, always_known1p;
  //  Char buf[40];

  //  StrPrintF(buf,"attempting/%d", inven_ctr);
  //  message(buf);
  //  SysTaskDelay(SysTicksPerSecond());
  typ = i_ptr->tval;
  subt = i_ptr->subval;
  known1p = known1_p(i_ptr);
  always_known1p = (object_offset (i_ptr) == -1);
  /* Now, check to see if player can carry object  */
  for (locn = 0; ; locn++) {
    t_ptr = &inventory[locn];
    if ((typ == t_ptr->tval) && (subt == t_ptr->subval)
	&& (subt >= ITEM_SINGLE_STACK_MIN)
	&& ((Short)t_ptr->number + (Short)i_ptr->number < 256)
	&& ((subt < ITEM_GROUP_MIN) || (t_ptr->p1 == i_ptr->p1))
	/* only stack if both or neither are identified */
	&& (known1p == known1_p(t_ptr))) {
      //      StrPrintF(buf,"stacking %d/%d", locn, inven_ctr);
      //      message(buf);
      //      SysTaskDelay(SysTicksPerSecond());
      invy_set_number(inventory, locn, t_ptr->number + i_ptr->number);
      //      message("stacked");
      //      SysTaskDelay(SysTicksPerSecond());
      break;
    }
    /* For items which are always known1p, i.e. never have a 'color',
       insert them into the inventory in sorted order.  */
    else if ((typ == t_ptr->tval && subt < t_ptr->subval
	      && always_known1p)
	     || (typ > t_ptr->tval)) {
      //      StrPrintF(buf,"inserting %d/%d", locn, inven_ctr);
      //      message(buf);
      //      SysTaskDelay(SysTicksPerSecond());
      invy_insert(locn, i_ptr);
      //      message("not stacked");
      //      SysTaskDelay(SysTicksPerSecond());
      break;
    }
  }
  //  StrPrintF(buf, "at %d/%d", locn, inven_ctr);
  //  message(buf);
  //  SysTaskDelay(SysTicksPerSecond());
  inven_weight += i_ptr->number*i_ptr->weight;
  pyflags.status |= PY_STR_WGT;
  return locn;
}


// TEMPORARILY MISSING:
//
// spell_chance

/* Returns spell chance of failure for spell		-RAK-	*/
extern spell_type magic_spell[MAX_CLASS-1][MAX_SPELLS]; // magic.c
Short spell_chance(Short spell)
{
  spell_type *s_ptr;
  Short chance;
  Short stat;

  s_ptr = &magic_spell[pymisc.pclass - 1][spell];
  chance = s_ptr->sfail - 3 * (pymisc.lev - s_ptr->slevel);
  if (class[pymisc.pclass].spell == MAGE)
    stat = A_INT;
  else
    stat = A_WIS;
  chance -= 3 * (stat_adj(stat) - 1);
  if (s_ptr->smana > pymisc.cmana)
    chance += 5 * (s_ptr->smana - pymisc.cmana);
  if (chance > 95)
    chance = 95;
  else if (chance < 5)
    chance = 5;
  return chance;
}

// print_spells - now part of ui (form_spell)
// get_spell    - now part of ui (form_spell)
// calc_spells  - untested
// gain_spells  - incomplete! xxx
// calc_mana    - untested
// gain_level   - untested
// calc_hitpoints - untested
// insert_lnum  - works
//
// teleport
//

/* calculate number of spells player should have, and learn/forget spells
   until that number is met -JEW- */
void calc_spells(Short stat)
{
  Short i;
  ULong mask;
  ULong spell_flag;
  Short j, offset;
  Short num_allowed=0, new_spells, num_known, levels;
  Char tmp_str[80];
  Char *p;
  spell_type *msp_ptr;

  msp_ptr = &magic_spell[pymisc.pclass-1][0];
  if (stat == A_INT) {
    p = "spell";
    offset = SPELL_OFFSET;
  } else {
    p = "prayer";
    offset = PRAYER_OFFSET;
  }

  /* check to see if know any spells greater than level, eliminate them */
  for (i = MAX_SPELLS, mask = 0x80000000L; mask; mask >>= 1, i--) {
    if (mask & spell_learned) {
      if (msp_ptr[i].slevel > pymisc.lev) {
	spell_learned &= ~mask;
	spell_forgotten |= mask;
	//	StrPrintF(tmp_str, "You have forgotten the %s of %s.", p,
	//		  spell_names[i+offset]);
	// sorry dudes, that string is going to be too long.
	StrPrintF(tmp_str, "You forgot %s!", spell_names[i+offset]);
	message(tmp_str);
      } else
	break;
    }
  }

  /* calc number of spells allowed */
  levels = pymisc.lev - class[pymisc.pclass].first_spell_lev + 1;
  switch(stat_adj(stat)) {
  case 0:		    num_allowed = 0; break;
  case 1: case 2: case 3:   num_allowed = 1 * levels; break;
  case 4: case 5:	    num_allowed = 3 * levels / 2; break;
  case 6:		    num_allowed = 2 * levels; break;
  case 7:		    num_allowed = 5 * levels / 2; break;
  }

  num_known = 0;
  for (mask = 0x1; mask; mask <<= 1)
    if (mask & spell_learned)
      num_known++;
  new_spells = num_allowed - num_known;

  if (new_spells > 0) {
    /* remember forgotten spells while forgotten spells exist of new_spells
       positive, remember the spells in the order that they were learned */
    for (i = 0; (spell_forgotten && new_spells &&
		 (i < num_allowed) && (i < 32)); i++) {
      /* j is (i+1)th spell learned */
      j = spell_order[i];
      /* shifting by amounts greater than number of bits in long gives
	 an undefined result, so don't shift for unknown spells */
      if (j == 99)
	mask = 0x0;
      else
	mask = 1L << j;
      if (mask & spell_forgotten) {
	if (msp_ptr[j].slevel <= pymisc.lev) {
	  new_spells--;
	  spell_forgotten &= ~mask;
	  spell_learned |= mask;
	  //	  StrPrintF(tmp_str, "You have remembered the %s of %s.",
	  //		    p, spell_names[j+offset]);
	  StrPrintF(tmp_str, "You remember %s!", spell_names[j+offset]);
	  message(tmp_str);
	} else
	  num_allowed++;
      }
    }

    if (new_spells > 0) {
      /* determine which spells player can learn */
      /* must check all spells here, in gain_spell() we actually check
	 if the books are present */
      spell_flag = 0x7FFFFFFFL & ~spell_learned;

      mask = 0x1;
      i = 0;
      for (j = 0, mask = 0x1; spell_flag; mask <<= 1, j++) {
	if (spell_flag & mask) {
	  spell_flag &= ~mask;
	  if (msp_ptr[j].slevel <= pymisc.lev)
	    i++;
	}
      }

      if (new_spells > i)
	new_spells = i;
    }
  } else if (new_spells < 0) {
    /* forget spells until new_spells zero or no more spells know, spells
       are forgotten in the opposite order that they were learned */
    for (i = MAX_SPELLS; new_spells && spell_learned; i--) {
      /* j is the (i+1)th spell learned */
      j = spell_order[i];
      /* shifting by amounts greater than number of bits in long gives
	 an undefined result, so don't shift for unknown spells */
      if (j == 99)
	mask = 0x0;
      else
	mask = 1L << j;
      if (mask & spell_learned) {
	spell_learned &= ~mask;
	spell_forgotten |= mask;
	new_spells++;
	//	StrPrintF(tmp_str, "You have forgotten the %s of %s.", p,
	//		       spell_names[j+offset]);
	StrPrintF(tmp_str, "You forgot %s!", spell_names[j+offset]);
	message(tmp_str);
      }
    }

    new_spells = 0;
  }

  if (new_spells != pyflags.new_spells) {
    if (new_spells > 0 && pyflags.new_spells == 0) {
      StrPrintF(tmp_str, "You can learn some new %ss now.", p);
      message(tmp_str);
    }

    pyflags.new_spells = new_spells;
    pyflags.status |= PY_STUDY;
  }
}

/* gain spells when player wants to		- jw */
// XXXXXX It might be a real good idea to move this into form_spell.
// What of globals in this file does it use?
static Short diff_spells; // was local var, made global so helper can use it
Short new_spells, last_known;
extern Short spell_ctr; // form_spell.c - use for gaining mage spells
Short spells[MAX_SPELLS]; // form_spell.c - ditto
Boolean missing_a_book;
extern Boolean just_browsing_book;
void gain_spells()
{
  //  Char query;
  Short stat, offset;//,new_spells; diff_spells, // must be global for 'helper'
  //Short spells[MAX_SPELLS]; // need for form_spell
  // Short last_known;// need for learn_the_spell
  Short i, j;
  ULong spell_flag, mask;
  Char tmp_str[80];
  //struct misc *p_ptr = &py.misc;
  spell_type *msp_ptr;
  /* Priests don't need light because they get spells from their god,
     so only fail when can't see if player has MAGE spells.  This check
     is done below.  */
  if (pyflags.confused > 0) {
    message("You are too confused.");
    return;
  }
  new_spells = pyflags.new_spells;
  diff_spells = 0;
  msp_ptr = &magic_spell[pymisc.pclass-1][0];
  if (class[pymisc.pclass].spell == MAGE) {
    stat = A_INT;
    offset = SPELL_OFFSET;
    /* People with MAGE spells can't learn spells if they can't read their
       books.  */
    if (pyflags.blind > 0) {
      message("You can't see to read your spell book!");
      return;
    }
    else if (no_light()) {
      message("You have no light to read by.");
      return;
    }
  } else {
    stat = A_WIS;
    offset = PRAYER_OFFSET;
  }
  for (last_known = 0; last_known < 32; last_known++)
    if (spell_order[last_known] == 99)
      break;
  if (!new_spells) {
    StrPrintF(tmp_str, "You can't learn any new %ss!",
	      (stat == A_INT ? "spell" : "prayer"));
    message(tmp_str);
    free_turn_flag = true;
  } else {
    /* determine which spells player can learn */
    /* mages need the book to learn a spell, priests do not need the book */
    if (stat == A_INT) {
      spell_flag = 0;
      for (i = 0; i < inven_ctr; i++)
	if (inventory[i].tval == TV_MAGIC_BOOK)
	  spell_flag |= inventory[i].flags;
    } else
      spell_flag = 0x7FFFFFFF;
    /* clear bits for spells already learned */
    spell_flag &= ~spell_learned;

    mask = 0x1;
    i = 0;
    for (j = 0, mask = 0x1; spell_flag; mask <<= 1, j++) {
      if (spell_flag & mask) {
	spell_flag &= ~mask;
	if (msp_ptr[j].slevel <= pymisc.lev) {
	  spells[i] = j;
	  i++;
	}
      }
    }
    if (new_spells > i) {
      // danger will robinson - this message will not be visible for mages!
      // we'll remember this and print it in the comment space in that form.
      missing_a_book = true;
      message("You seem to be missing a book.");
      diff_spells = new_spells - i;
      new_spells = i;
    } else missing_a_book = false;
    if (new_spells > 0) {
      if (stat == A_INT) {
	// Cleverly we have already filled in spells[].
	// We also need to set spell_ctr, the number of things in spells[]
	spell_ctr = i;
	// Ok we are ready to pick spells (w/o reference to book)
	just_browsing_book = false;
	FrmPopupForm(GainSpForm);
	//	StrPrintF(tmp_str, "%d sorry mage, not implemented", i);
	//	message(tmp_str);
	return; // GainSpForm will have to call gain_spell_helper after this.
      } else {
	/* pick a prayer at random */
	while (new_spells) {
	  j = randint(i) - 1;
	  spell_learned |= 1L << spells[j];
	  spell_order[last_known++] = spells[j];
	  //	  StrPrintF(tmp_str, "You have learned the prayer of %s.",
	  StrPrintF(tmp_str, "You have learned %s.",
		    spell_names[spells[j]+offset]);
	  message(tmp_str);
	  for (; j <= i-1; j++)
	    spells[j] = spells[j+1];
	  i--;
	  new_spells--;
	}
      }
    }
    pyflags.new_spells = new_spells + diff_spells;
    if (pyflags.new_spells == 0)
      pyflags.status |= PY_STUDY;
    /* set the mana for first level characters when they learn their
       first spell */
    if (pymisc.mana == 0)
      calc_mana(stat);
  }
}
// GainSp form calls this when user has picked 0--n spells and is exiting.
void gain_spell_helper()
{
  Short stat = A_INT; // only a mage-type caster will call this
  // do the same endgame as above for priest
  pyflags.new_spells = new_spells + diff_spells;
  if (pyflags.new_spells == 0)
    pyflags.status |= PY_STUDY;
  /* set the mana for first level characters when they learn their
     first spell */
  if (pymisc.mana == 0)
    calc_mana(stat);
}

/* Gain some mana if you know at least one spell	-RAK-	*/
void calc_mana(Short stat)
{
  Short new_mana=0, levels;
  //struct misc *p_ptr;
  Long value;

  if (spell_learned != 0) {
    levels = pymisc.lev - class[pymisc.pclass].first_spell_lev + 1;
    switch(stat_adj(stat)) {
    case 0: new_mana = 0; break;
    case 1: case 2: new_mana = 1 * levels; break;
    case 3: new_mana = 3 * levels / 2; break;
    case 4: new_mana = 2 * levels; break;
    case 5: new_mana = 5 * levels / 2; break;
    case 6: new_mana = 3 * levels; break;
    case 7: new_mana = 4 * levels; break;
    }
    /* increment mana by one, so that first level chars have 2 mana */
    if (new_mana > 0)
      new_mana++;

    /* mana can be zero when creating character */
    if (pymisc.mana != new_mana) {
      if (pymisc.mana != 0) {
	/* change current mana proportionately to change of max mana,
	   divide first to avoid overflow, little loss of accuracy */
	value = (((Long)pymisc.cmana << 16) + pymisc.cmana_frac)
	  / pymisc.mana * new_mana;
	pymisc.cmana = value >> 16;
	pymisc.cmana_frac = value & 0xFFFF;
      } else {
	pymisc.cmana = new_mana;
	pymisc.cmana_frac = 0;
      }
      pymisc.mana = new_mana;
      /* can't print mana here, may be in store or inventory mode */
      pyflags.status |= PY_MANA;
    }
  } else if (pymisc.mana != 0) {
    pymisc.mana = 0;
    pymisc.cmana = 0;
    /* can't print mana here, may be in store or inventory mode */
    pyflags.status |= PY_MANA;
  }
}

/* Increases hit points and level			-RAK-	*/
static void gain_level()
{
  Long dif_exp, need_exp;
  Char out_val[80];
  //  register struct misc *p_ptr;
  class_type *c_ptr;

  pymisc.lev++;
  StrPrintF(out_val, "Welcome to level %d.", (Short) pymisc.lev);
  message(out_val);
  calc_hitpoints();

  need_exp = player_exp[pymisc.lev-1] * pymisc.expfact / 100;
  if (pymisc.exp > need_exp) {
    /* lose some of the 'extra' exp when gaining several levels at once */
    dif_exp = pymisc.exp - need_exp;
    pymisc.exp = need_exp + (dif_exp / 2);
  }
  //  prt_level(); // xxxx
  //  prt_title(); // xxxx
  c_ptr = &class[pymisc.pclass];
  if (c_ptr->spell == MAGE) {
    calc_spells(A_INT);
    calc_mana(A_INT);
  }
  else if (c_ptr->spell == PRIEST) {
    calc_spells(A_WIS);
    calc_mana(A_WIS);
  }
}

/* Calculate the players hit points */
void calc_hitpoints()
{
  Short hitpoints;
  //struct misc *p_ptr;
  Long value;

  hitpoints = player_hp[pymisc.lev-1] + (con_adj() * pymisc.lev);
  /* always give at least one point per level + 1 */
  if (hitpoints < (pymisc.lev + 1))
    hitpoints = pymisc.lev + 1;

  if (pyflags.status & PY_HERO)
    hitpoints += 10;
  if (pyflags.status & PY_SHERO)
    hitpoints += 20;

  /* mhp can equal zero while character is being created */
  if ((hitpoints != pymisc.mhp) && (pymisc.mhp != 0)) {
    /* change current hit points proportionately to change of mhp,
       divide first to avoid overflow, little loss of accuracy */
    value = (((Long)pymisc.chp << 16) + pymisc.chp_frac) / pymisc.mhp
      * hitpoints;
    pymisc.chp = value >> 16;
    pymisc.chp_frac = value & 0xFFFF;
    pymisc.mhp = hitpoints;

    /* can't print hit points here, may be in store or inventory mode */
    pyflags.status |= PY_HP;
  }
}



// object_str is a string with the code 'mtc_str' in it, maybe multiple times.
// which is to be replaced by the 'number'.  object_str is < 80 chars
// and of course better not be a database record
void insert_lnum(Char *object_str, const Char *tag_str,
		 Long number, Boolean signp)
{
  // xxxx This is patently not working.
  Short taglen, len;
  Char dupe[80];
  Char *p, *tagp;

  StrCopy(dupe, object_str); // work from dupe to modify object_str..
  object_str[0] = '\0';
  taglen = StrLen(tag_str);
  p = dupe;
  while (StrLen(p) > 0) {
    tagp = StrStr(p, tag_str);
    if (tagp == NULL) {
      // there are no more instances of the magic substring; we're done
      StrCat(object_str, p);
      return;
    }
    // found an instance of magic substring; replace it
    tagp[0] = '\0'; // terminate the string-before-substring, copy that much
    len = StrLen(object_str);
    StrPrintF(object_str+len, "%s%ld", p, number);
    p = tagp+taglen; // skip to just-past the substring
  }
}

/* Inserts a string into a string				*/
// XXX UNTESTED!
void insert_str(Char *object_str, Char *tag_str, Char *insert)
{
  Short obj_len;
  Char *bound, *pc;
  Short i, tag_len;
  Char *temp_obj, *temp_tag;
  Char out_val[80];

  tag_len = StrLen(tag_str);
  obj_len = StrLen(object_str);
  bound = object_str + obj_len - tag_len;
  for (pc = object_str; pc <= bound; pc++) {
    temp_obj = pc;
    temp_tag = tag_str;
    for (i = 0; i < tag_len; i++)
      if (*temp_obj++ != *temp_tag++)
	break;
    if (i == tag_len)
      break;
  }

  if (pc <= bound) {
      StrNCopy(out_val, object_str, (pc-object_str));
      /* Turbo C needs int for array index.  */
      out_val[(Short)(pc-object_str)] = '\0';
      if (insert)
	StrCat(out_val, insert);
      StrCat(out_val, (Char *)(pc+tag_len));
      StrCopy(object_str, out_val);
    }
}

/* Weapon weight VS strength and dexterity		-RAK-	*/
Short attack_blows(Short weight, Short *wtohit)
{
  Short adj_weight;
  Short str_index, dex_index, s, d;

  s = pystats.use_stat[A_STR];
  d = pystats.use_stat[A_DEX];
  if (s * 15 < weight) {
    *wtohit = s * 15 - weight;
    return 1;
  } else {
    *wtohit = 0;
    if      (d <  10)	 dex_index = 0;
    else if (d <  19)	 dex_index = 1;
    else if (d <  68)	 dex_index = 2;
    else if (d < 108)	 dex_index = 3;
    else if (d < 118)	 dex_index = 4;
    else		 dex_index = 5;
    adj_weight = (s * 10 / weight);
    if      (adj_weight < 2)	str_index = 0;
    else if (adj_weight < 3)	str_index = 1;
    else if (adj_weight < 4)	str_index = 2;
    else if (adj_weight < 5)	str_index = 3;
    else if (adj_weight < 7)	str_index = 4;
    else if (adj_weight < 9)	str_index = 5;
    else			str_index = 6;
    return (Short)blows_table[str_index][dex_index];
  }
}


/* Special damage due to magical abilities of object	-RAK-	*/
Short tot_dam(inven_type *i_ptr, Short tdam, Short monster)
{
  creature_type *m_ptr;

  if ((i_ptr->flags & TR_EGO_WEAPON)
      && (((i_ptr->tval >= TV_SLING_AMMO) && (i_ptr->tval <= TV_ARROW))
	  || ((i_ptr->tval >= TV_HAFTED) && (i_ptr->tval <= TV_SWORD))
	  || (i_ptr->tval == TV_FLASK))) {
    m_ptr = &c_list[monster];
    /* Slay Dragon  */
    if ((m_ptr->cdefense & CD_DRAGON) && (i_ptr->flags & TR_SLAY_DRAGON)) {
      tdam = tdam * 4;
      recall_cdefense_or(monster, CD_DRAGON);
    }
    /* Slay Undead  */
    else if ((m_ptr->cdefense & CD_UNDEAD) && (i_ptr->flags & TR_SLAY_UNDEAD)){
      tdam = tdam * 3;
      recall_cdefense_or(monster, CD_UNDEAD);
    }
    /* Slay Animal  */
    else if ((m_ptr->cdefense & CD_ANIMAL) && (i_ptr->flags & TR_SLAY_ANIMAL)){
      tdam = tdam * 2;
      recall_cdefense_or(monster, CD_ANIMAL);
    }
    /* Slay Evil     */
    else if ((m_ptr->cdefense & CD_EVIL) && (i_ptr->flags & TR_SLAY_EVIL)) {
      tdam = tdam * 2;
      recall_cdefense_or(monster, CD_EVIL);
    }
    /* Frost	       */
    else if ((m_ptr->cdefense & CD_FROST) && (i_ptr->flags & TR_FROST_BRAND)) {
      tdam = (tdam * 3) / 2;
      recall_cdefense_or(monster, CD_FROST);
    }
    /* Fire	      */
    else if ((m_ptr->cdefense & CD_FIRE) && (i_ptr->flags & TR_FLAME_TONGUE)) {
      tdam = (tdam * 3) / 2;
      recall_cdefense_or(monster, CD_FIRE);
    }
  }
  return tdam;
}


/* Critical hits, Nasty way to die.			-RAK-	*/
Short critical_blow(Short weight, Short plus, Short dam, Short attack_type)
{
  Short critical;

  critical = dam;
  /* Weight of weapon, plusses to hit, and character level all	    */
  /* contribute to the chance of a critical			   */
  if (randint(5000) <= (Short)(weight + 5 * plus
			       + (class_level_adj[pymisc.pclass][attack_type]
				  * pymisc.lev))) {
    weight += randint(650);
    if (weight < 400) {
      critical = 2*dam + 5;
      message("It was a good hit! (x2 damage)");
    } else if (weight < 700) {
      critical = 3*dam + 10;
      message("It was an excellent hit! (x3 damage)");
    } else if (weight < 900) {
      critical = 4*dam + 15;
      message("It was a superb hit! (x4 damage)");
    } else {
      critical = 5*dam + 20;
      message("It was a *GREAT* hit! (x5 damage)");
    }
  }
  return critical;
}


/* Given direction "dir", returns new row, column location -RAK- */
Boolean mmove(Short dir, Short *y, Short *x)
{
  Short new_row, new_col;
  Boolean bool = false;
  new_row = *y;
  new_col = *x;

  switch(dir) {
  case 1:
    new_row++; new_col--;
    break;
  case 2:
    new_row++;
    break;
  case 3:
    new_row++; new_col++;
    break;
  case 4:
    new_col--;
    break;
    //  case 5: no change
  case 6:
    new_col++;
    break;
  case 7:
    new_row--; new_col--;
    break;
  case 8:
    new_row--;
    break;
  case 9:
    new_row--; new_col++;
    break;
  }
  if ((new_row >= 0) && (new_row < cur_height)
      && (new_col >= 0) && (new_col < cur_width)) {
    *y = new_row;
    *x = new_col;
    bool = true;
  }
  return bool;
}


/* Saving throws for player character.		-RAK-	*/
Boolean player_saves()
{
  /* MPW C couldn't handle the expression, so split it into two parts */
  Short temp = class_level_adj[pymisc.pclass][CLA_SAVE];

  if (randint(100) <= (pymisc.save + stat_adj(A_WIS)
		       + (temp * pymisc.lev / 3)))
    return true;
  else
    return false;
}

/* Finds range of item in inventory list		-RAK-	*/
Boolean find_range(Short item1, Short item2, Short *j, Short *k)
{
  Short i;
  inven_type *i_ptr;
  Boolean flag;

  i = 0;
  *j = -1;
  *k = -1;
  flag = false;
  i_ptr = inventory;
  while (i < inven_ctr) {
    if (!flag) {
      if ((i_ptr->tval == item1) || (i_ptr->tval == item2)) {
	flag = true;
	*j = i;
      }
    } else {
      if ((i_ptr->tval != item1) && (i_ptr->tval != item2)) {
	  *k = i - 1;
	  break;
	}
    }
    i++;
    i_ptr++;
  }
  if (flag && (*k == -1))
    *k = inven_ctr - 1;
  return flag;
}

// Teleport
/* Teleport the player to a new location		-RAK-	*/
void teleport(Short dis)
{
  Short y, x, i, j;

  do {
    y = rund(cur_height);
    x = rund(cur_width);
    while (distance(y, x, char_row, char_col) > dis) {
      y += (char_row - y) / 2;
      x += (char_col - x) / 2;
    }
  } while ((cave_fval[y*MAX_WIDTH+x] >= MIN_CLOSED_SPACE) ||
	   (cave_cptr[y*MAX_WIDTH+x] >= 2));

  move_rec(char_row, char_col, y, x);
  for (i = char_row - 1 ; i <= char_row + 1 ; i++) {
    for (j = char_col - 1 ; j <= char_col + 1 ; j++) {
      cave_set(i, j, CAVE_light, cave_light[i*MAX_WIDTH+j] & ~LIGHT_tl);
      // (this implements      cave[i][j].tl = FALSE; )
      lite_spot(i, j);
    }
  }
  lite_spot(char_row, char_col);
  char_row = y;
  char_col = x;
  check_view();
  creatures(false);
  teleport_flag = false;
}
