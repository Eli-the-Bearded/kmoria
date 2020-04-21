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

/*
static void inven_screen(int);
static char map_roguedir(char);
static void sub1_move_light(int, int, int, int);
static void sub3_move_light(int, int, int, int);
*/
static void sub1_move_light(Short y1, Short x1, Short y2, Short x2) SEC_1;
static void sub3_move_light(Short y1, Short x1, Short y2, Short x2) SEC_1;


// This could be temporarily and go into a record instead.  Read-only.
Short class_level_adj[MAX_CLASS][MAX_LEV_ADJ] = {
/*             bth    bthb   device  disarm   save/misc hit  */
/* Warrior */ { 4,      4,      2,      2,      3 },
/* Mage    */ { 2,      2,      4,      3,      3 },
/* Priest  */ { 2,      2,      4,      3,      3 },
/* Rogue   */ { 3,      4,      3,      4,      3 },
/* Ranger  */ { 3,      4,      3,      3,      3 },
/* Paladin */ { 3,      3,      3,      2,      3 }
};



/* Changes speed of monsters relative to player		-RAK-	*/
/* Note: When the player is sped up or slowed down, I simply	 */
/*	 change the speed of all the monsters.	This greatly	 */
/*	 simplified the logic.				       */
void change_speed(Short num)
{
  Short i, cspeed;

  pyflags.speed += num;
  pyflags.status |= PY_SPEED;
  for (i = mfptr - 1; i >= MIN_MONIX; i--) {
    cspeed = m_list[i].cspeed + num;
    // With luck this will work.  Note to self - TEST it sometime...
    mlist_set_short(i, MLIST_cspeed, cspeed);
  }
}


/* Player bonuses					-RAK-	*/
/* When an item is worn or taken off, this re-adjusts the player */
/* bonuses.  Factor=1 : wear; Factor=-1 : removed		 */
/* Only calculates properties with cumulative effect.  Properties that
   depend on everything being worn are recalculated by calc_bonuses() -CJS- */
void py_bonuses(inven_type *t_ptr, Short factor)
{
  Short i, amount;

  amount = t_ptr->p1 * factor;
  if (t_ptr->flags & TR_STATS) {
    for (i = 0; i < 6; i++)
      if ((1 << i) & t_ptr->flags)
	bst_stat(i, amount);
  }
  if (TR_SEARCH & t_ptr->flags) {
    pymisc.srh += amount;
    pymisc.fos -= amount;
  }
  if (TR_STEALTH & t_ptr->flags)
    pymisc.stl += amount;
  if (TR_SPEED & t_ptr->flags)
    change_speed(-amount);
  if ((TR_BLIND & t_ptr->flags) && (factor > 0))
    pyflags.blind += 1000;
  if ((TR_TIMID & t_ptr->flags) && (factor > 0))
    pyflags.afraid += 50;
  if (TR_INFRA & t_ptr->flags)
    pyflags.see_infra += amount;
}

/* Recalculate the effect of all the stuff we use.		  -CJS- */
void calc_bonuses()
{
  ULong item_flags;
  Short old_dis_ac;
  inven_type *i_ptr;
  Short i;

  if (pyflags.slow_digest)
    pyflags.food_digested++;
  if (pyflags.regenerate)
    pyflags.food_digested -= 3;
  pyflags.see_inv     = false;
  pyflags.teleport    = false;
  pyflags.free_act    = false;
  pyflags.slow_digest = false;
  pyflags.aggravate   = false;
  pyflags.sustain_str = false;
  pyflags.sustain_int = false;
  pyflags.sustain_wis = false;
  pyflags.sustain_con = false;
  pyflags.sustain_dex = false;
  pyflags.sustain_chr = false;
  pyflags.fire_resist = false;
  pyflags.acid_resist = false;
  pyflags.cold_resist = false;
  pyflags.regenerate  = false;
  pyflags.lght_resist = false;
  pyflags.ffall	      = false;

  old_dis_ac = pymisc.dis_ac;
  pymisc.ptohit	 = tohit_adj();	      // Real To Hit
  pymisc.ptodam	 = todam_adj();	      // Real To Dam
  pymisc.ptoac	 = toac_adj();	      // Real To AC
  pymisc.pac	 = 0;		    // Real AC
  pymisc.dis_th	 = pymisc.ptohit;  // Display To Hit
  pymisc.dis_td	 = pymisc.ptodam;  // Display To Dam
  pymisc.dis_ac	 = 0;		// Display AC
  pymisc.dis_tac = pymisc.ptoac;   // Display To AC

  for (i = INVEN_WIELD; i < INVEN_LIGHT; i++) {
    i_ptr = &inventory[i];
    if (i_ptr->tval != TV_NOTHING) {
      pymisc.ptohit += i_ptr->tohit;
      if (i_ptr->tval != TV_BOW)		// Bows can't damage. -CJS-
	pymisc.ptodam += i_ptr->todam;
      pymisc.ptoac    += i_ptr->toac;
      pymisc.pac += i_ptr->ac;
      if (known2_p(i_ptr)) {
	pymisc.dis_th  += i_ptr->tohit;
	if (i_ptr->tval != TV_BOW)
	  pymisc.dis_td  += i_ptr->todam;	// Bows can't damage. -CJS-
	pymisc.dis_tac += i_ptr->toac;
	pymisc.dis_ac += i_ptr->ac;
      } else if (! (TR_CURSED & i_ptr->flags))
	// Base AC values should always be visible, as long as the item
	// is not cursed.
	pymisc.dis_ac += i_ptr->ac;
    }
  }
  pymisc.dis_ac += pymisc.dis_tac;

  if (weapon_heavy)
    pymisc.dis_th += (pystats.use_stat[A_STR] * 15 -
		      inventory[INVEN_WIELD].weight);

  // Add in temporary spell increases
  if (pyflags.invuln > 0) {
    pymisc.pac += 100;
    pymisc.dis_ac += 100;
  }
  if (pyflags.blessed > 0) {
    pymisc.pac    += 2;
    pymisc.dis_ac += 2;
  }
  if (pyflags.detect_inv > 0)
    pyflags.see_inv = true;

  // can't print AC here because might be in a store
  if (old_dis_ac != pymisc.dis_ac)
    pyflags.status |= PY_ARMOR;

  item_flags = 0;
  i_ptr = &inventory[INVEN_WIELD];
  for (i = INVEN_WIELD; i < INVEN_LIGHT; i++) {
    item_flags |= i_ptr->flags;
    i_ptr++;
  }
  if (TR_SLOW_DIGEST & item_flags)
    pyflags.slow_digest = true;
  if (TR_AGGRAVATE & item_flags)
    pyflags.aggravate = true;
  if (TR_TELEPORT & item_flags)
    pyflags.teleport = true;
  if (TR_REGEN & item_flags)
    pyflags.regenerate = true;
  if (TR_RES_FIRE & item_flags)
    pyflags.fire_resist = true;
  if (TR_RES_ACID & item_flags)
    pyflags.acid_resist = true;
  if (TR_RES_COLD & item_flags)
    pyflags.cold_resist = true;
  if (TR_FREE_ACT & item_flags)
    pyflags.free_act = true;
  if (TR_SEE_INVIS & item_flags)
    pyflags.see_inv = true;
  if (TR_RES_LIGHT & item_flags)
    pyflags.lght_resist = true;
  if (TR_FFALL & item_flags)
    pyflags.ffall = true;

  i_ptr = &inventory[INVEN_WIELD];
  for (i = INVEN_WIELD; i < INVEN_LIGHT; i++) {
    if (TR_SUST_STAT & i_ptr->flags)
      switch(i_ptr->p1) {
      case 1: pyflags.sustain_str = true; break;
      case 2: pyflags.sustain_int = true; break;
      case 3: pyflags.sustain_wis = true; break;
      case 4: pyflags.sustain_con = true; break;
      case 5: pyflags.sustain_dex = true; break;
      case 6: pyflags.sustain_chr = true; break;
      default: break;
      }
    i_ptr++;
  }

  if (pyflags.slow_digest)
    pyflags.food_digested--;
  if (pyflags.regenerate)
    pyflags.food_digested += 3;
}


// void show_inven(...)
// There will be a form to do this!


/* Return a string describing how a given equipment item is carried. -CJS- */
Char *describe_use(Short i)
{
  Char *p;

  switch(i) {
  case INVEN_WIELD:
    p = "wielding"; break;
  case INVEN_HEAD:
    p = "wearing on your head"; break;
  case INVEN_NECK:
    p = "wearing around your neck"; break;
  case INVEN_BODY:
    p = "wearing on your body"; break;
  case INVEN_ARM:
    p = "wearing on your arm"; break;
  case INVEN_HANDS:
    p = "wearing on your hands"; break;
  case INVEN_RIGHT:
    p = "wearing on your right hand"; break;
  case INVEN_LEFT:
    p = "wearing on your left hand"; break;
  case INVEN_FEET:
    p = "wearing on your feet"; break;
  case INVEN_OUTER:
    p = "wearing about your body"; break;
  case INVEN_LIGHT:
    p = "using to light the way"; break;
  case INVEN_AUX:
    p = "holding ready by your side"; break;
  default:
    p = "carrying in your pack"; break;
  }
  return p;
}


// Short show_equip(..)
// There will be a form to do this!


/* Remove item from equipment list		-RAK-	*/
void takeoff(Short item_val, Short posn)
{
  Char *p;
  Char out_val[160], prt2[160];
  inven_type *t_ptr;

  equip_ctr--;
  t_ptr = &inventory[item_val];
  inven_weight -= t_ptr->weight*t_ptr->number;
  pyflags.status |= PY_STR_WGT;

  if (item_val == INVEN_WIELD || item_val == INVEN_AUX)
    p = "Was wielding ";
  else if (item_val == INVEN_LIGHT)
    p = "Light source was ";
  else
    p = "Was wearing ";

  objdes(prt2, t_ptr, true);
  if (posn >= 0)
    StrPrintF(out_val, "%s%s (%c)", p, prt2, 'a'+posn);
  else
    StrPrintF(out_val, "%s%s", p, prt2);
  message(out_val);
  if (item_val != INVEN_AUX)	  /* For secondary weapon  */
    py_bonuses(t_ptr, -1);
  invcopy(inventory, item_val, OBJ_NOTHING);
}



// Boolean verify(...)
// There will be a form to do this!

// void inven_screen(...)
// void inven_command(...)
// There will be a form to do this!

// Need: get_item  (I need inventory before I can do that.)
// Oh, this is something that will be a form I think.

/* I may have written the town level code, but I'm not exactly	 */
/* proud of it.	 Adding the stores required some real slucky	 */
/* hooks which I have not had time to re-think.		 -RAK-	 */

/* Returns true if player has no light			-RAK-	*/
Boolean no_light()
{
  UChar light = cave_light[char_row*MAX_WIDTH+char_col];

  if (light & (LIGHT_tl | LIGHT_pl))
    return false; // player has light
  return true; // player has no light
}


// Char map_roguedir(..)
// This might be useful in main.c


// Short get_dir(..)
// Short get_alldir(..)
// There will be a form, or something, for this!



/* Moves creature record from one space to another	-RAK-	*/
void move_rec(Short y1, Short x1, Short y2, Short x2)
{
  Short tmp;

  /* this always works correctly, even if y1==y2 and x1==x2 */
  // (still true :) //
  tmp = cave_cptr[y1*MAX_WIDTH+x1];
  cave_set(y1, x1, CAVE_cptr, 0);
  cave_set(y2, x2, CAVE_cptr, tmp);
}



/* Room is lit, make it appear				-RAK-	*/
// I might need to change this in light of what I do in display.c
void light_room(Short y, Short x)
{
  Short i, j, start_col, end_col;
  Short tmp1, tmp2, start_row, end_row;
  Short c_ptr;
  Short tval;
  UChar light, thing;

  tmp1 = (SCREEN_HEIGHT/2);
  tmp2 = (SCREEN_WIDTH_REAL /2);
  start_row = (y/tmp1)*tmp1;
  start_col = (x/tmp2)*tmp2;
  end_row = start_row + tmp1 - 1;
  end_col = start_col + tmp2 - 1;
  for (i = start_row; i <= end_row; i++)
    for (j = start_col; j <= end_col; j++) {
      c_ptr = i*MAX_WIDTH+j;
      light = cave_light[c_ptr];
      thing = cave_tptr[c_ptr];
      if ((light & LIGHT_lr) && !(light & LIGHT_pl)) {
	light |= LIGHT_pl;
	cave_set(i, j, CAVE_light, light);
	if (cave_fval[c_ptr] == DARK_FLOOR)
	  cave_set(i, j, CAVE_fval, LIGHT_FLOOR);
	if (!(light & LIGHT_fm) && thing != 0) {
	  tval = t_list[thing].tval;
	  if (tval >= TV_MIN_VISIBLE && tval <= TV_MAX_VISIBLE) {
	    light |= LIGHT_fm;
	    cave_set(i, j, CAVE_light, light);
	  }
	}
	print(loc_symbol(i, j), i, j);
      }
    }
}


/* Lights up given location				-RAK-	*/
void lite_spot(Short y, Short x)
{
  if (panel_contains(y, x)) {
    print(loc_symbol(y, x), y, x);
  }
}



/* Normal movement					*/
/* When FIND_FLAG,  light only permanent features	*/
static void sub1_move_light(Short y1, Short x1, Short y2, Short x2)
{
  Short i, j;
  Short tval, top, left, bottom, right;
  UChar light, thing;

  if (light_flag) {
    for (i = y1-1; i <= y1+1; i++)	   /* Turn off lamp light	*/
      for (j = x1-1; j <= x1+1; j++) {
	light = cave_light[i*MAX_WIDTH+j] & ~LIGHT_tl;
	cave_set(i, j, CAVE_light, light);
      }
    if (find_flag && !find_prself)
      light_flag = false;
  } else if (!find_flag || find_prself)
    light_flag = true;

  for (i = y2-1; i <= y2+1; i++)
    for (j = x2-1; j <= x2+1; j++) {
      //      c_ptr = &cave[i][j];
      light = cave_light[i*MAX_WIDTH+j];
      thing = cave_tptr[i*MAX_WIDTH+j];
      /* only light up if normal movement */
      if (light_flag)
	light |= LIGHT_tl;
      if (cave_fval[i*MAX_WIDTH+j] >= MIN_CAVE_WALL)
	light |= LIGHT_pl;
      else if (!(light & LIGHT_fm) && thing != 0) {
	tval = t_list[thing].tval;
	if ((tval >= TV_MIN_VISIBLE) && (tval <= TV_MAX_VISIBLE))
	  light |= LIGHT_fm;
      }
      cave_set(i, j, CAVE_light, light);
    }

  /* From uppermost to bottom most lines player was on.	 */
  if (y1 < y2) {
    top = y1 - 1;
    bottom = y2 + 1;
  } else {
    top = y2 - 1;
    bottom = y1 + 1;
  }
  if (x1 < x2) {
    left = x1 - 1;
    right = x2 + 1;
  } else {
    left = x2 - 1;
    right = x1 + 1;
  }
  for (i = top; i <= bottom; i++)
    for (j = left; j <= right; j++)   /* Leftmost to rightmost do*/
      print(loc_symbol(i, j), i, j);
}

/* When blinded,  move only the player symbol.		*/
/* With no light,  movement becomes involved.		*/
static void sub3_move_light(Short y1, Short x1, Short y2, Short x2)
{
  Short i, j;
  UChar light;

  if (light_flag) {
    for (i = y1-1; i <= y1+1; i++)
      for (j = x1-1; j <= x1+1; j++) {
	light = cave_light[i*MAX_WIDTH+j] & ~LIGHT_tl;
	cave_set(i, j, CAVE_light, light);
	print(loc_symbol(i, j), i, j);
      }
    light_flag = false;
  } else if (!find_flag || find_prself)
    print(loc_symbol(y1, x1), y1, x1);

  // if not running, or if we like to print ourselves during a run,
  if (!find_flag || find_prself)
    print('@', y2, x2);
}


/* Package for moving the character's light about the screen	 */
/* Four cases : Normal, Finding, Blind, and Nolight	 -RAK-	 */
void move_light(Short y1, Short x1, Short y2, Short x2)
{
  if (pyflags.blind > 0 || !player_light)
    sub3_move_light(y1, x1, y2, x2);
  else
    sub1_move_light(y1, x1, y2, x2);
}


/* Something happens to disturb the player.		-CJS-
   The first arg indicates a major disturbance, which affects search.
   The second arg indicates a light change. */
void disturb(Boolean se, Boolean li)
{
  command_count = 0;
  if (se && (pyflags.status & PY_SEARCH))
    search_off();
  if (pyflags.rest != 0)
    rest_off();
  if (li || find_flag) {
    find_flag = false;
    check_view();
  }
  //  flush();
}



/* Search Mode enhancement				-RAK-	*/
void search_on()
{
  change_speed(1);
  pyflags.status |= PY_SEARCH;
  print_stats(STATS_QUO);//  prt_state();  //  prt_speed();
  pyflags.food_digested++;
}

void search_off()
{
  check_view();
  change_speed(-1);
  pyflags.status &= ~PY_SEARCH;
  print_stats(STATS_QUO);//  prt_state();  //  prt_speed();
  pyflags.food_digested--;
}


// void rest()
// This will require a form or something, to detemine count.
void rest()
{
}


void rest_off()
{
  pyflags.rest = 0;
  pyflags.status &= ~PY_REST;
  print_stats(STATS_QUO);//  prt_state();
  //  msg_print(CNIL); /* flush last message, or delete "press any key" message */
  message_clear();
  pyflags.food_digested++;
}



/* Attacker's level and plusses,  defender's AC		-RAK-	*/
Boolean test_hit(Short bth, Short level, Short pth, Short ac,Short attack_type)
{
  Short i, die;

  disturb(true, false);
  i = bth + pth * BTH_PLUS_ADJ
    + (level * class_level_adj[pymisc.pclass][attack_type]);
  /* pth could be less than 0 if player wielding weapon too heavy for him */
  /* always miss 1 out of 20, always hit 1 out of 20 */
  die = randint(20);
  if ((die != 1) && ((die == 20)
		     || ((i > 0) && (randint (i) > ac))))  /* normal hit */
    return true;
  else
    return false;
}


/* Decreases players hit points and sets death flag if necessary*/
/*							 -RAK-	 */
void take_hit(Short damage, Char *hit_from)
{
  if (pyflags.invuln > 0)  damage = 0;
  pymisc.chp -= damage;
  if (pymisc.chp < 0) {
    if (!death) {
      death = true;
      StrCopy(died_from, hit_from);
      total_winner = false;
    }
    new_level_flag = true;
  }
  else
    print_stats(STATS_QUO); //prt_chp();
}
