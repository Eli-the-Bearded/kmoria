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

// all of spells.c with some unimplemented-function calls commented out.


// This could be in a read-only database record!
// It is 40*sizeof(ULong) = 160 bytes.
ULong player_exp[MAX_PLAYER_LEVEL] = {
      10,      25,	45,	 70,	  100,	    140,      200,	280,
     380,     500,     650,	850,	 1100,	   1400,     1800,     2300,
    2900,    3600,    4400,    5400,	 6800,	   8400,    10200,    12500,
   17500,   25000,  35000L,  50000L,   75000L,	100000L,  150000L,  200000L,
 300000L, 400000L, 500000L, 750000L, 1500000L, 2500000L, 5000000L, 10000000L
};
// This could be in a read-only database record!
// It is ... mumble... 6*sizeof(class_type) = 6*31? = 186 bytes
class_type class[MAX_CLASS] = {
/*        HP Dis Src Stl Fos bth btb sve S  I  W  D Co Ch  Spell Exp  spl */
{"Warrior",9, 25, 14, 1, 38, 70, 55, 18, 5,-2,-2, 2, 2,-1, NONE,    0, 0},
{"Mage",   0, 30, 16, 2, 20, 34, 20, 36,-5, 3, 0, 1,-2, 1, MAGE,   30, 1},
{"Priest", 2, 25, 16, 2, 32, 48, 35, 30,-3,-3, 3,-1, 0, 2, PRIEST, 20, 1},
{"Rogue",  6, 45, 32, 5, 16, 60, 66, 30, 2, 1,-2, 3, 1,-1, MAGE,    0, 5},
{"Ranger", 4, 30, 24, 3, 24, 56, 72, 30, 2, 2, 0, 1, 1, 1, MAGE,   40, 3},
{"Paladin",6, 20, 12, 1, 38, 68, 40, 24, 3,-3, 1, 0, 2, 2, PRIEST, 35, 1}
};

//static void get_flags(Short typ, ULong *weapon_type, Short *harm_type, 
//		      Boolean (**destroy)() ) SEC_2;
static void replace_spot(Short y, Short x, Short typ) SEC_2;

/* Following are spell procedure/functions			-RAK-	*/
/* These routines are commonly used in the scroll, potion, wands, and	 */
/* staves routines, and are occasionally called from other areas.	  */
/* Now included are creature spells also.		       -RAK    */

void monster_name(Char *m_name, monster_type *m_ptr, creature_type *r_ptr)
{
  if (!m_ptr->ml)
    StrCopy (m_name, "It");
  else
    StrPrintF (m_name, "The %s", c_names + r_ptr->name_offset);
}

void lower_monster_name(Char *m_name, monster_type *m_ptr,creature_type *r_ptr)
{
  if (!m_ptr->ml)
    StrCopy (m_name, "it");
  else
    StrPrintF (m_name, "the %s", c_names + r_ptr->name_offset);
}


/* Sleep creatures adjacent to player			-RAK-	*/
Boolean sleep_monsters1(Short y, Short x)
{
  Short i, j;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Boolean sleep;
  Char out_val[80], m_name[80];
  UChar critter;

  sleep = false;
  for (i = y-1; i <= y+1; i++)
    for (j = x-1; j <= x+1; j++) {
      critter = cave_cptr[i*MAX_WIDTH+j];
      if (critter > 1) {
	m_ptr = &m_list[critter];
	r_ptr = &c_list[m_ptr->mptr];

	monster_name(m_name, m_ptr, r_ptr);
	if ((randint(MAX_MONS_LEVEL) < r_ptr->level)
	    || (CD_NO_SLEEP & r_ptr->cdefense)) {
	  if (m_ptr->ml && (r_ptr->cdefense & CD_NO_SLEEP))
	    recall_cdefense_or(m_ptr->mptr, CD_NO_SLEEP);
	  StrPrintF(out_val, "%s is unaffected.", m_name);
	  message(out_val);
	} else {
	  sleep = true;
	  mlist_set_short(critter, MLIST_csleep, 500);
	  StrPrintF(out_val, "%s falls asleep.", m_name);
	  message(out_val);
	}
      }
    }
  return sleep;
}

/* Detect any treasure on the current panel		-RAK-	*/
Boolean detect_treasure()
{
  Short i, j;
  Boolean detect;
  UChar thing;

  detect = false;
  for (i = panel_row_min; i <= panel_row_max; i++)
    for (j = panel_col_min; j <= panel_col_max; j++) {
      thing = cave_tptr[i*MAX_WIDTH+j];
      if ((thing != 0) && (t_list[thing].tval == TV_GOLD)
	  && !test_light(i, j)) {
	cave_set(i, j, CAVE_light, cave_light[i*MAX_WIDTH+j] | LIGHT_fm);
	lite_spot(i, j);
	detect = true;
      }
    }
  return detect;
}


/* Detect all objects on the current panel		-RAK-	*/
Boolean detect_object()
{
  Short i, j;
  Boolean detect;
  UChar thing;

  detect = false;
  for (i = panel_row_min; i <= panel_row_max; i++)
    for (j = panel_col_min; j <= panel_col_max; j++) {
      thing = cave_tptr[i*MAX_WIDTH+j];
      if ((thing != 0) && (t_list[thing].tval < TV_MAX_OBJECT)
	  && !test_light(i, j))	{
	cave_set(i, j, CAVE_light, cave_light[i*MAX_WIDTH+j] | LIGHT_fm);
	lite_spot(i, j);
	detect = true;
      }
    }
  return detect;
}


/* Locates and displays traps on current panel		-RAK-	*/
Boolean detect_trap()
{
  Short i, j;
  Boolean detect;
  //  inven_type *t_ptr;
  UChar thing;

  detect = false;
  for (i = panel_row_min; i <= panel_row_max; i++)
    for (j = panel_col_min; j <= panel_col_max; j++) {
      thing = cave_tptr[i*MAX_WIDTH+j];
      if (thing != 0) {
	if (t_list[thing].tval == TV_INVIS_TRAP) {
	  cave_set(i, j, CAVE_light, cave_light[i*MAX_WIDTH+j] | LIGHT_fm);
	  change_trap(i, j);
	  detect = true;
	} else if (t_list[thing].tval == TV_CHEST) {
	  known2(t_list, thing);
	}
      }
    }
  return detect;
}

/* Locates and displays all secret doors on current panel -RAK-	*/
Boolean detect_sdoor()
{
  Short i, j;
  Boolean detect;
  UChar thing, light;

  detect = false;
  for (i = panel_row_min; i <= panel_row_max; i++)
    for (j = panel_col_min; j <= panel_col_max; j++) {
      thing = cave_tptr[i*MAX_WIDTH+j];
      if (thing != 0) {
	light = cave_light[i*MAX_WIDTH+j];
	if (t_list[thing].tval == TV_SECRET_DOOR) {
	  /* Secret doors  */
	  //	  c_ptr->fm = true;
	  cave_set(i, j, CAVE_light, light | LIGHT_fm);
	  change_trap(i, j);
	  detect = true;
	} else if (((t_list[thing].tval == TV_UP_STAIR) ||
		    (t_list[thing].tval == TV_DOWN_STAIR)) &&
		   !(light & LIGHT_fm)) {
	  /* Staircases	 */
	  cave_set(i, j, CAVE_light, light | LIGHT_fm);
	  lite_spot(i, j);
	  detect = true;
	}
      }
    }
  return detect;
}


/* Locates and displays all invisible creatures on current panel -RAK-*/
Boolean detect_invisible()
{
  Short i;
  Boolean flag;
  monster_type *m_ptr;

  flag = false;
  for (i = mfptr - 1; i >= MIN_MONIX; i--) {
    m_ptr = &m_list[i];
    if (panel_contains((Short) m_ptr->fy, (Short) m_ptr->fx) &&
	(CM_INVISIBLE & c_list[m_ptr->mptr].cmove)) {
      mlist_set_ml(i, true);
      /* works correctly even if hallucinating */
      print((Char) c_list[m_ptr->mptr].cchar, (Short) m_ptr->fy,
	    (Short) m_ptr->fx);
      flag = true;
    }
  }
  if (flag) {
    message("You sense the presence of invisible creatures!");
    //      msg_print(CNIL);
    // PROMPT --more--
    /* must unlight every monster just lighted */
    creatures(false);
  }
  return flag;
}


/* Light an area: 1.  If corridor  light immediate area -RAK-*/
/*		  2.  If room  light entire room plus immediate area.     */
Boolean light_area(Short y, Short x)
{
  Short i, j;
  Boolean light;

  if (pyflags.blind < 1)
    message("You are surrounded by a white light.");
  light = true;
  if ((cave_light[y*MAX_WIDTH+x] & LIGHT_lr) && (dun_level > 0))
    light_room(y, x);
  /* Must always light immediate area, because one might be standing on
     the edge of a room, or next to a destroyed area, etc.  */
  for (i = y-1; i <= y+1; i++)
    for (j = x-1; j <=  x+1; j++) {
      cave_set(i, j, CAVE_light, cave_light[i*MAX_WIDTH+j] | LIGHT_pl);
      lite_spot(i, j);
    }
  return light;
}


/* Darken an area, opposite of light area		-RAK-	*/
Boolean unlight_area(Short y, Short x)
{
  Short i, j;
  Short tmp1, tmp2;
  Boolean unlight;
  Short start_row, start_col, end_row, end_col;
  Short c_ptr;

  unlight = false;
  if ((cave_light[y*MAX_WIDTH+x] & LIGHT_lr) && (dun_level > 0)) {
    tmp1 = (SCREEN_HEIGHT/2);
    tmp2 = (SCREEN_WIDTH_REAL /2);
    start_row = (y/tmp1)*tmp1 + 1;
    start_col = (x/tmp2)*tmp2 + 1;
    end_row = start_row + tmp1 - 1;
    end_col = start_col + tmp2 - 1;
    for (i = start_row; i <= end_row; i++) {
      for (j = start_col; j <= end_col; j++) {
	c_ptr = i*MAX_WIDTH+j;
	if ((cave_light[c_ptr] & LIGHT_lr)
	    && cave_fval[c_ptr] <= MAX_CAVE_FLOOR) {
	  cave_set(i, j, CAVE_light, cave_light[i*MAX_WIDTH+j] & ~LIGHT_pl);
	  cave_set(i, j, CAVE_fval, DARK_FLOOR);
	  lite_spot (i, j);
	  if (!test_light(i, j))
	    unlight = true;
	}
      }
    }
  } else
    for (i = y-1; i <= y+1; i++)
      for (j = x-1; j <= x+1; j++) {
	c_ptr = i*MAX_WIDTH+j;
	if (cave_fval[c_ptr] == CORR_FLOOR && (cave_light[c_ptr] & LIGHT_pl)) {
	  /* pl could have been set by star-lite wand, etc */
	  cave_set(i, j, CAVE_light, cave_light[i*MAX_WIDTH+j] & ~LIGHT_pl);
	  unlight = true;
	}
      }

  if (unlight && pyflags.blind <= 0)
    message("Darkness surrounds you.");

  return unlight;
}


/* Map the current area plus some			-RAK-	*/
void map_area()
{
  Short i7, i8, n, m;
  Short i, j, k, l;
  UChar thing;

  i = panel_row_min - randint(10);
  j = panel_row_max + randint(10);
  k = panel_col_min - randint(20);
  l = panel_col_max + randint(20);
  for (m = i; m <= j; m++)
    for (n = k; n <= l; n++)
      if (in_bounds(m, n) && (cave_fval[m*MAX_WIDTH+n] <= MAX_CAVE_FLOOR))
	for (i7 = m-1; i7 <= m+1; i7++)
	  for (i8 = n-1; i8 <= n+1; i8++) {
	    thing = cave_tptr[i7*MAX_WIDTH+i8];
	    if (cave_fval[i7*MAX_WIDTH+i8] >= MIN_CAVE_WALL)
	      cave_set(i7,i8,CAVE_light,cave_light[i7*MAX_WIDTH+i8] |LIGHT_pl);
	    else if ((thing != 0) &&
		     (t_list[thing].tval >= TV_MIN_VISIBLE) &&
		     (t_list[thing].tval <= TV_MAX_VISIBLE))
	      cave_set(i7,i8,CAVE_light,cave_light[i7*MAX_WIDTH+i8] |LIGHT_fm);
	  }
  //  prt_map(); // xxx not implemented yet
}


/* Identify an object					-RAK-	*/
Short ident_spell()
{
  Short item_val;
  Char out_val[160], tmp_str[160];
  Short ident;
  inven_type *i_ptr;

  ident = false;
  if (get_item(&item_val, "Item you wish identified?", 0, INVEN_ARRAY_SIZE,
	       CNIL, CNIL)) {
    ident = true;
    identify(&item_val);
    i_ptr = &inventory[item_val];
    known2(inventory, item_val);
    objdes(tmp_str, i_ptr, true);
    if (item_val >= INVEN_WIELD) {
      calc_bonuses();
      StrPrintF (out_val, "%s: %s", describe_use(item_val), tmp_str);
    } else
      StrPrintF(out_val, "%c %s", item_val+97, tmp_str);
    message(out_val);
  }
  return ident;
}


/* Get all the monsters on the level pissed off.	-RAK-	*/
Boolean aggravate_monster(Short dis_affect)
{
  Short i;
  Boolean aggravate;
  monster_type *m_ptr;

  aggravate = false;
  for (i = mfptr - 1; i >= MIN_MONIX; i--) {
    m_ptr = &m_list[i];
    mlist_set_short(i, MLIST_csleep, 0);
    if ((m_ptr->cdis <= dis_affect) && (m_ptr->cspeed < 2)) {
      mlist_set_short(i, MLIST_cspeed, m_ptr->cspeed + 1);
      aggravate = true;
    }
  }
  if (aggravate)
    message ("You hear a sudden stirring in the distance!");
  return aggravate;
}


/* Surround the fool with traps (chuckle)		-RAK-	*/
Boolean trap_creation()
{
  Short i, j;
  Boolean trap;
  UChar thing;

  trap = true;
  for (i = char_row-1; i <= char_row+1; i++)
    for (j = char_col-1; j <= char_col+1; j++) {
      /* Don't put a trap under the player, since this can lead to
	 strange situations, e.g. falling through a trap door while
	 trying to rest, setting off a falling rock trap and ending
	 up under the rock.  */
      if (i == char_row && j == char_col)
	continue;

      if (cave_fval[i*MAX_WIDTH+j] <= MAX_CAVE_FLOOR) {
	thing = cave_tptr[i*MAX_WIDTH+j];
	if (thing != 0)
	  delete_object(i, j);
	place_trap(i, j, randint(MAX_TRAP)-1);

	/* don't let player gain exp from the newly created traps */
	tlist_set_p1(t_list, thing, 0);
	/* open pits are immediately visible, so call lite_spot */
	lite_spot(i, j);
      }
    }
  return trap;
}


/* Surround the player with doors.			-RAK-	*/
Boolean door_creation()
{
  Short i, j;
  Boolean door;
  Short k;
  UChar thing;

  door = false;
  for (i = char_row-1; i <= char_row+1; i++)
    for (j = char_col-1; j <=  char_col+1; j++)
      if ((i != char_row) || (j != char_col)) {
	if (cave_fval[i*MAX_WIDTH+j] <= MAX_CAVE_FLOOR) {
	  door = true;
	  thing = cave_tptr[i*MAX_WIDTH+j];
	  if (thing != 0)
	    delete_object(i, j);
	  k = popt();
	  cave_set(i, j, CAVE_fval, BLOCKED_FLOOR);
	  cave_set(i, j, CAVE_tptr, k);
	  invcopy(t_list, k, OBJ_CLOSED_DOOR);
	  lite_spot(i, j);
	}
      }
  return door;
}


/* Destroys any adjacent door(s)/trap(s)		-RAK-	*/
Short td_destroy()
{
  Short i, j;
  Boolean destroy;
  inven_type *t_ptr;
  UChar thing;

  destroy = false;
  for (i = char_row-1; i <= char_row+1; i++)
    for (j = char_col-1; j <= char_col+1; j++) {
      thing = cave_tptr[i*MAX_WIDTH+j];
      if (thing != 0) {
	t_ptr = &t_list[thing];
	if (((t_ptr->tval >= TV_INVIS_TRAP) && (t_ptr->tval <= TV_CLOSED_DOOR)
	     && (t_ptr->tval != TV_RUBBLE))
	    || (t_ptr->tval == TV_SECRET_DOOR)) {
	  if (delete_object(i, j))
	    destroy = true;
	} else if ((t_ptr->tval == TV_CHEST) && (t_ptr->flags != 0)) {
	  /* destroy traps on chest and unlock */
	  tlist_set_flags(t_list,thing,t_ptr->flags & ~(CH_TRAPPED|CH_LOCKED));
	  tlist_set_name2(t_list, thing, SN_UNLOCKED);
	  message ("You have disarmed the chest.");
	  known2(t_list, thing);
	  destroy = true;
	}
      }
    }
  return destroy;
}


/* Display all creatures on the current panel		-RAK-	*/
Boolean detect_monsters()
{
  Short i;
  Boolean detect;
  monster_type *m_ptr;

  detect = false;
  for (i = mfptr - 1; i >= MIN_MONIX; i--) {
    m_ptr = &m_list[i];
    if (panel_contains((Short) m_ptr->fy, (Short) m_ptr->fx) &&
	((CM_INVISIBLE & c_list[m_ptr->mptr].cmove) == 0)) {
      mlist_set_ml(i, true);
      /* works correctly even if hallucinating */
      print((Char) c_list[m_ptr->mptr].cchar, (Short) m_ptr->fy,
	    (Short) m_ptr->fx);
      detect = true;
    }
  }
  if (detect) {
    message("You sense the presence of monsters!");
    //    message(CNIL);
    /* must unlight every monster just lighted */
    creatures(false);
  }
  return detect;
}


/* Leave a line of light in given dir, blue light can sometimes	*/
/* hurt creatures.				       -RAK-   */
void light_line(Short dir, Short y, Short x)
{
  Short i;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Short dist;
  Boolean flag;
  Char out_val[80], m_name[80];
  UChar light, floor, critter, thing;

  dist = -1;
  flag = false;
  while (!flag) {
  /* put mmove at end because want to light up current spot */
    dist++;
    floor = cave_fval[y*MAX_WIDTH+x];
    light = cave_light[y*MAX_WIDTH+x];
    critter = cave_cptr[y*MAX_WIDTH+x];
    thing = cave_tptr[y*MAX_WIDTH+x];
    if (dist > OBJ_BOLT_RANGE || floor >= MIN_CLOSED_SPACE)
      flag = true;
    else {
      if (!(light & (LIGHT_pl | LIGHT_tl))) {
	/* set pl so that lite_spot will work */
	cave_set(y, x, CAVE_light, light | LIGHT_pl);
	if (floor == LIGHT_FLOOR) {
	  if (panel_contains(y, x))
	    light_room(y, x);
	} else
	  lite_spot(y, x);
      }
      /* set pl in case tl was true above */
      cave_set(y, x, CAVE_light, light | LIGHT_pl);
      if (critter > 1) {
	m_ptr = &m_list[critter];
	r_ptr = &c_list[m_ptr->mptr];
	/* light up and draw monster */
	update_mon ((Short) critter);
	monster_name (m_name, m_ptr, r_ptr);
	if (CD_LIGHT & r_ptr->cdefense) {
	  if (m_ptr->ml)
	    recall_cdefense_or(m_ptr->mptr, CD_LIGHT);
	  i = mon_take_hit((Short) critter, damroll(2, 8));
	  if (i >= 0) {
	    StrPrintF(out_val,
		      "%s shrivels away in the light!", m_name);
	    message(out_val);
	    print_stats(STATS_QUO);//prt_experience();
	  } else {
	    StrPrintF(out_val, "%s cringes from the light!",
		      m_name);
	    message (out_val);
	  }
	}
      }
    }
    mmove(dir, &y, &x);
  } 
}



/* Light line in all directions				-RAK-	*/
void starlite(Short y, Short x)
{
  Short i;

  if (pyflags.blind < 1)
    message("The end of the staff bursts into a blue shimmering light.");
  for (i = 1; i <= 9; i++)
    if (i != 5)
      light_line(i, y, x);
}


/* Disarms all traps/chests in a given direction	-RAK-	*/
Boolean disarm_all(Short dir, Short y, Short x)
{
  inven_type *t_ptr;
  Short dist;
  Boolean disarm;
  UChar thing;

  disarm = false;
  dist = -1;
  do {
    /* put mmove at end, in case standing on a trap */
    dist++;
    /* note, must continue upto and including the first non open space,
       because secret doors have fval greater than MAX_OPEN_SPACE */
    thing = cave_tptr[y*MAX_WIDTH+x];
    if (thing != 0) {
      t_ptr = &t_list[thing];
      if ((t_ptr->tval == TV_INVIS_TRAP) || (t_ptr->tval == TV_VIS_TRAP)) {
	if (delete_object(y, x))
	  disarm = true;
      } else if (t_ptr->tval == TV_CLOSED_DOOR)
	/* Locked/jammed doors become merely closed. */
	tlist_set_p1(t_list, thing, 0);
      else if (t_ptr->tval == TV_SECRET_DOOR) {
	cave_set(y, x, CAVE_light, cave_light[y*MAX_WIDTH+x] | LIGHT_fm);
	change_trap(y, x);
	disarm = true;
      } else if ((t_ptr->tval == TV_CHEST) && (t_ptr->flags != 0)) {
	message("Click!");
	tlist_set_flags(t_list, thing, t_ptr->flags & ~(CH_TRAPPED|CH_LOCKED));
	disarm = true;
	tlist_set_name2(t_list, thing, SN_UNLOCKED);
	known2(t_list, thing);
      }
    }
    mmove(dir, &y, &x);
  } while ((dist <= OBJ_BOLT_RANGE)
	   && cave_fval[y*MAX_WIDTH+x] <= MAX_OPEN_SPACE);
  return disarm;
}


/* Return flags for given type area affect		-RAK-	*/
static void get_flags(Short typ, ULong *weapon_type, Short *harm_type, 
		      Boolean (**destroy)() )
{
  // Hm.  I would rather not be passing function pointers around.
  // Mostly because it confuses me.
  switch(typ) {
  case GF_MAGIC_MISSILE:
    *weapon_type = 0;
    *harm_type   = 0;
    *destroy	   = is_null;
    break;
  case GF_LIGHTNING:
    *weapon_type = CS_BR_LIGHT;
    *harm_type   = CD_LIGHT;
    *destroy	   = is_lightning_destroy;
    break;
  case GF_POISON_GAS:
    *weapon_type = CS_BR_GAS;
    *harm_type   = CD_POISON;
    *destroy	   = is_null;
    break;
  case GF_ACID:
    *weapon_type = CS_BR_ACID;
    *harm_type   = CD_ACID;
    *destroy	   = is_acid_destroy;
    break;
  case GF_FROST:
    *weapon_type = CS_BR_FROST;
    *harm_type   = CD_FROST;
    *destroy	   = is_frost_destroy;
    break;
  case GF_FIRE:
    *weapon_type = CS_BR_FIRE;
    *harm_type   = CD_FIRE;
    *destroy	   = is_fire_destroy;
    break;
  case GF_HOLY_ORB:
    *weapon_type = 0;
    *harm_type   = CD_EVIL;
    *destroy	   = is_null;
    break;
  default:
    message("ERROR in get_flags()\n");
  }
}


/* Shoot a bolt in a given direction			-RAK-	*/
void fire_bolt(Short typ, Short dir, Short y, Short x, Short dam,
	       Char *bolt_typ)
{
  Short i, oldy, oldx, dist;
  Boolean flag;
  ULong weapon_type;
  Short harm_type;
  Boolean (*dummy)(); // hm.. I would rather not mess with function pointers
  Short c_ptr;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80], m_name[80];
  UChar light, critter;

  flag = false;
  get_flags(typ, &weapon_type, &harm_type, &dummy); // xxx change function ptrs
  oldy = y;
  oldx = x;
  dist = 0;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = y*MAX_WIDTH+x;
    lite_spot(oldy, oldx);
    if ((dist > OBJ_BOLT_RANGE) || cave_fval[c_ptr] >= MIN_CLOSED_SPACE)
      flag = true;
    else {
      critter = cave_cptr[c_ptr];
      if (critter > 1) {
	flag = true;
	m_ptr = &m_list[critter];
	r_ptr = &c_list[m_ptr->mptr];

	/* light up monster and draw monster, temporarily set
	   pl so that update_mon() will work */
	light = cave_light[c_ptr];
	cave_set(y, x, CAVE_light, light | LIGHT_pl);
	update_mon ((Short) critter);
	cave_set(y, x, CAVE_light, light);
	/* draw monster and clear previous bolt */
	//	put_qio(); // xxx not implemented yet

	lower_monster_name(m_name, m_ptr, r_ptr);
	StrPrintF(out_val, "The %s strikes %s.", bolt_typ, m_name);
	message(out_val);
	if (harm_type & r_ptr->cdefense) {
	  dam = dam*2;
	  if (m_ptr->ml)
	    recall_cdefense_or(m_ptr->mptr, harm_type);
	} else if (weapon_type & r_ptr->spells) {
	  dam = dam / 4;
	  if (m_ptr->ml)
	    recall_spells_or(m_ptr->mptr, weapon_type);
	}
	monster_name(m_name, m_ptr, r_ptr);
	i = mon_take_hit((Short) critter, dam);
	if (i >= 0) {
	  StrPrintF(out_val, "%s dies in a fit of agony.",
		    m_name);
	  message(out_val);
	  print_stats(STATS_QUO);//	  prt_experience();
	} else if (dam > 0) {
	  StrPrintF (out_val, "%s screams in agony.", m_name);
	  message (out_val);
	}
      } else if (panel_contains(y, x) && (pyflags.blind < 1)) {
	print('*', y, x);
	/* show the bolt */
	//	put_qio(); // xxx not implemented yet
      }
    }
    oldy = y;
    oldx = x;
  }
}


/* Shoot a ball in a given direction.  Note that balls have an	*/
/* area affect.					      -RAK-   */
void fire_ball(Short typ, Short dir, Short y, Short x, Short dam_hp,
	       Char *descrip)
{
  Short i, j;
  Short dam, max_dis, thit, tkill, k;
  Short oldy, oldx, dist, harm_type;
  Boolean flag;
  ULong weapon_type;
  Boolean (*destroy)(); // xxx
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80];
  UChar floor, thing, critter, light;

  thit	 = 0;
  tkill	 = 0;
  max_dis = 2;
  get_flags(typ, &weapon_type, &harm_type, &destroy);
  flag = false;
  oldy = y;
  oldx = x;
  dist = 0;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    lite_spot(oldy, oldx);
    if (dist > OBJ_BOLT_RANGE)
      flag = true;
    else {
      floor = cave_fval[y*MAX_WIDTH+x];
      critter = cave_cptr[y*MAX_WIDTH+x];
      if ((floor >= MIN_CLOSED_SPACE) || (critter > 1)) {
	flag = true;
	if (floor >= MIN_CLOSED_SPACE) {
	  y = oldy;
	  x = oldx;
	}
	/* The ball hits and explodes.		     */
	/* The explosion.			     */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis)
		&& los(y, x, i, j)) {
	      floor = cave_fval[i*MAX_WIDTH+j];
	      thing = cave_tptr[i*MAX_WIDTH+j];
	      critter = cave_cptr[i*MAX_WIDTH+j];
	      light = cave_light[i*MAX_WIDTH+j];
	      if ((thing != 0) &&
		  (*destroy)(&t_list[thing]))
		delete_object(i, j);
	      if (floor <= MAX_OPEN_SPACE) {
		if (critter > 1) {
		  m_ptr = &m_list[critter];
		  r_ptr = &c_list[m_ptr->mptr];

		  /* lite up creature if visible, temp
		     set pl so that update_mon works */
		  // tmp = c_ptr->pl; c_ptr->pl = true;
		  cave_set(i, j, CAVE_light, light | LIGHT_pl);
		  update_mon((Short) critter);

		  thit++;
		  dam = dam_hp;
		  if (harm_type & r_ptr->cdefense) {
		    dam = dam*2;
		    if (m_ptr->ml)
		      recall_cdefense_or(m_ptr->mptr, harm_type);
		  } else if (weapon_type & r_ptr->spells) {
		    dam = dam / 4;
		    if (m_ptr->ml)
		      recall_spells_or(m_ptr->mptr, weapon_type);
		  }
		  dam = (dam/(distance(i, j, y, x)+1));
		  k = mon_take_hit((Short) critter, dam);
		  if (k >= 0)
		    tkill++;
		  cave_set(i, j, CAVE_light, light); //  c_ptr->pl = tmp;
		} else if (panel_contains(i, j) &&(pyflags.blind < 1))
		  print('*', i, j);
	      }
	    }
	/* show ball of whatever */
	//	put_qio(); // xxx not implemented yet

	for (i = (y - 2); i <= (y + 2); i++)
	  for (j = (x - 2); j <= (x + 2); j++)
	    if (in_bounds(i, j) && panel_contains(i, j) &&
		(distance(y, x, i, j) <= max_dis))
	      lite_spot(i, j);

	/* End  explosion.		     */
	if (thit == 1) {
	  StrPrintF(out_val,
		    "The %s envelops a creature!",
		    descrip);
	  message(out_val);
	} else if (thit > 1) {
	  StrPrintF(out_val,
		    "The %s envelops several creatures!",
		    descrip);
	  message(out_val);
	}
	if (tkill == 1)
	  message("There is a scream of agony!");
	else if (tkill > 1)
	  message("There are several screams of agony!");
	if (tkill >= 0)
	  print_stats(STATS_QUO);//prt_experience();
	/* End ball hitting.		     */
      } else if (panel_contains(y, x) && (pyflags.blind < 1)) {
	print('*', y, x);
	/* show bolt */
	//	put_qio(); // xxx not implemented yet
      }
      oldy = y;
      oldx = x;
    }
  }
}


/* Breath weapon works like a fire_ball, but affects the player. */
/* Note the area affect.			      -RAK-   */
void breath(Short typ, Short y, Short x, Short dam_hp, Char *ddesc,
	    Short monptr)
{
  Short i, j;
  Short dam, max_dis, harm_type;
  ULong weapon_type;
  ULong tmp, treas;
  Boolean (*destroy)(); // xxx
  monster_type *m_ptr;
  creature_type *r_ptr;
  UChar thing, critter;

  max_dis = 2;
  get_flags(typ, &weapon_type, &harm_type, &destroy);
  for (i = y-2; i <= y+2; i++)
    for (j = x-2; j <= x+2; j++)
      if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis)
	  && los(y, x, i, j)) {
	thing = cave_tptr[i*MAX_WIDTH+j];
	critter = cave_cptr[i*MAX_WIDTH+j];
	if ((thing != 0) &&
	    (*destroy)(&t_list[thing]))
	  delete_object(i, j);
	if (cave_fval[i*MAX_WIDTH+j] <= MAX_OPEN_SPACE) {
	  /* must test status bit, not pyflags.blind here, flag could have
	     been set by a previous monster, but the breath should still
	     be visible until the blindness takes effect */
	  if (panel_contains(i, j) && !(pyflags.status & PY_BLIND))
	    print('*', i, j);
	  if (critter > 1) {
	    m_ptr = &m_list[critter];
	    r_ptr = &c_list[m_ptr->mptr];
	    dam = dam_hp;
	    if (harm_type & r_ptr->cdefense)
	      dam = dam*2;
	    else if (weapon_type & r_ptr->spells)
	      dam = (dam / 4);
	    dam = (dam/(distance(i, j, y, x)+1));
	    /* can not call mon_take_hit here, since player does not
	       get experience for kill */
	    mlist_set_short(critter, MLIST_hp, m_ptr->hp - dam);
	    mlist_set_short(critter, MLIST_csleep, 0);
	    if (m_ptr->hp < 0) {
	      treas = monster_death((Short) m_ptr->fy, (Short) m_ptr->fx,
				    r_ptr->cmove);
	      if (m_ptr->ml) {
		tmp = (c_recall[m_ptr->mptr].r_cmove & CM_TREASURE)
		  >> CM_TR_SHIFT;
		if (tmp > ((treas & CM_TREASURE) >> CM_TR_SHIFT))
		  treas = (treas & ~CM_TREASURE)|(tmp<<CM_TR_SHIFT);
		recall_set_cmove(m_ptr->mptr,
		     treas | (c_recall[m_ptr->mptr].r_cmove & ~CM_TREASURE));
	      }

	      /* It ate an already processed monster.Handle normally.*/
	      if (monptr < critter)
		delete_monster((Short)  critter);
	      /* If it eats this monster, an already processed monster
		 will take its place, causing all kinds of havoc.
		 Delay the kill a bit. */
	      else
		fix1_delete_monster((Short)  critter);
	    }
	  } else if (critter == 1) {
	    dam = (dam_hp/(distance(i, j, y, x)+1));
	    /* let's do at least one point of damage */
	    /* prevents randint(0) problem with poison_gas, also */
	    if (dam == 0)
	      dam = 1;
	    switch(typ) {
	      //	    case GF_LIGHTNING: light_dam(dam, ddesc); break; // xxx not implemented yet
	      //	    case GF_POISON_GAS: poison_gas(dam, ddesc); break; // xxx not implemented yet
	      //	    case GF_ACID: acid_dam(dam, ddesc); break; // xxx not implemented yet
	      //	    case GF_FROST: cold_dam(dam, ddesc); break; // xxx not implemented yet
	      //	    case GF_FIRE: fire_dam(dam, ddesc); break; // xxx not implemented yet
	    }
	  }
	}
      }
  /* show the ball of gas */
  //  put_qio(); // xxx not implemented yet

  for (i = (y - 2); i <= (y + 2); i++)
    for (j = (x - 2); j <= (x + 2); j++)
      if (in_bounds(i, j) && panel_contains(i, j) &&
	  (distance(y, x, i, j) <= max_dis))
	lite_spot(i, j);
}


/* Recharge a wand, staff, or rod.  Sometimes the item breaks. -RAK-*/
Boolean recharge(Short num)
{
  Short i, j, item_val;
  Boolean res;
  inven_type *i_ptr;

  res = false;
  if (!find_range(TV_STAFF, TV_WAND, &i, &j))
    message("You have nothing to recharge.");
  else if (get_item(&item_val, "Recharge which item?", i, j, CNIL, CNIL)) {
    i_ptr = &inventory[item_val];
    res = true;
    /* recharge I = recharge(20) = 1/6 failure for empty 10th level wand */
    /* recharge II = recharge(60) = 1/10 failure for empty 10th level wand*/
    /* make it harder to recharge high level, and highly charged wands, note
       that i can be negative, so check its value before trying to call
       randint().  */
    i = num + 50 - (Short) i_ptr->level - i_ptr->p1;
    if (i < 19)
      i = 1;	/* Automatic failure.  */
    else
      i = randint (i/10);

    if (i == 1) {
      message("There is a bright flash of light.");
      inven_destroy(item_val);
    } else {
      num = (num/(i_ptr->level+2)) + 1;
      invy_set_p1(inventory, item_val, i_ptr->p1 + 2 + randint(num));
      if (known2_p(i_ptr))
	clear_known2(inventory, item_val);
      clear_empty(inventory, item_val);
    }
  }
  return res;
}


/* Increase or decrease a creatures hit points		-RAK-	*/
Boolean hp_monster(Short dir, Short y, Short x, Short dam)
{
  Short i, dist;
  Boolean flag, monster;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80], m_name[80];
  UChar critter;

  monster = false;
  flag = false;
  dist = 0;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    critter = cave_cptr[y*MAX_WIDTH+x];
    if (dist > OBJ_BOLT_RANGE || cave_fval[y*MAX_WIDTH+x] >= MIN_CLOSED_SPACE)
      flag = true;
    else if (critter > 1) {
      flag = true;
      m_ptr = &m_list[critter];
      r_ptr = &c_list[m_ptr->mptr];
      monster_name (m_name, m_ptr, r_ptr);
      monster = true;
      i = mon_take_hit((Short) critter, dam);
      if (i >= 0) {
	StrPrintF(out_val, "%s dies in a fit of agony.", m_name);
	message(out_val);
	print_stats(STATS_QUO);//prt_experience();
      } else if (dam > 0) {
	StrPrintF(out_val, "%s screams in agony.", m_name);
	message(out_val);
      }
    }
  }
  return monster;
}


/* Drains life; note it must be living.		-RAK-	*/
Boolean drain_life(Short dir, Short y, Short x)
{
  Short i, dist;
  Boolean flag, drain;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80], m_name[80];
  UChar critter;

  drain = false;
  flag = false;
  dist = 0;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    critter = cave_cptr[y*MAX_WIDTH+x];
    if (dist > OBJ_BOLT_RANGE || cave_fval[y*MAX_WIDTH+x] >= MIN_CLOSED_SPACE)
      flag = true;
    else if (critter > 1) {
      flag = true;
      m_ptr = &m_list[critter];
      r_ptr = &c_list[m_ptr->mptr];
      if ((r_ptr->cdefense & CD_UNDEAD) == 0) {
	drain = true;
	monster_name (m_name, m_ptr, r_ptr);
	i = mon_take_hit((Short) critter, 75);
	if (i >= 0) {
	  StrPrintF(out_val, "%s dies in a fit of agony.",m_name);
	  message(out_val);
	  print_stats(STATS_QUO);//prt_experience();
	} else {
	  StrPrintF(out_val, "%s screams in agony.", m_name);
	  message(out_val);
	}
      } else
	recall_cdefense_or(m_ptr->mptr, CD_UNDEAD);
    }
  }
  return drain;
}


/* Increase or decrease a creatures speed		-RAK-	*/
/* NOTE: cannot slow a winning creature (BALROG)		 */
Boolean speed_monster(Short dir, Short y, Short x, Short spd)
{
  Boolean flag, speed;
  Short dist;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80], m_name[80];
  UChar critter;

  speed = false;
  flag = false;
  dist = 0;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    critter = cave_cptr[y*MAX_WIDTH+x];
    if (dist > OBJ_BOLT_RANGE || cave_fval[y*MAX_WIDTH+x] >= MIN_CLOSED_SPACE)
      flag = true;
    else if (critter > 1) {
      flag = true;
      m_ptr = &m_list[critter];
      r_ptr = &c_list[m_ptr->mptr];
      monster_name (m_name, m_ptr, r_ptr);
      if (spd > 0) {
	mlist_set_short(critter, MLIST_cspeed, m_ptr->cspeed + spd);
	mlist_set_short(critter, MLIST_csleep, 0);
	StrPrintF (out_val, "%s starts moving faster.", m_name);
	message (out_val);
	speed = true;
      } else if (randint(MAX_MONS_LEVEL) > r_ptr->level) {
	mlist_set_short(critter, MLIST_cspeed, m_ptr->cspeed + spd);
	mlist_set_short(critter, MLIST_csleep, 0);
	StrPrintF (out_val, "%s starts moving slower.", m_name);
	message (out_val);
	speed = true;
      } else {
	mlist_set_short(critter, MLIST_csleep, 0);
	StrPrintF(out_val, "%s is unaffected.", m_name);
	message(out_val);
      }
    }
  }
  return speed;
}


/* Confuse a creature					-RAK-	*/
Boolean confuse_monster(Short dir, Short y, Short x)
{
  Boolean flag, confuse;
  Short dist;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80], m_name[80];
  UChar critter;

  confuse = false;
  flag = false;
  dist = 0;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    critter = cave_cptr[y*MAX_WIDTH+x];
    if (dist > OBJ_BOLT_RANGE || cave_fval[y*MAX_WIDTH+x] >= MIN_CLOSED_SPACE)
      flag = true;
    else if (critter > 1) {
      m_ptr = &m_list[critter];
      r_ptr = &c_list[m_ptr->mptr];
      monster_name (m_name, m_ptr, r_ptr);
      flag = true;
      if ((randint(MAX_MONS_LEVEL) < r_ptr->level) ||
	  (CD_NO_SLEEP & r_ptr->cdefense)) {
	if (m_ptr->ml && (r_ptr->cdefense & CD_NO_SLEEP))
	  recall_cdefense_or(m_ptr->mptr, CD_NO_SLEEP);
	/* Monsters which resisted the attack should wake up.
	   Monsters with innate resistence ignore the attack.  */
	if (! (CD_NO_SLEEP & r_ptr->cdefense))
	  mlist_set_short(critter, MLIST_csleep, 0);
	StrPrintF(out_val, "%s is unaffected.", m_name);
	message(out_val);
      } else {
	if (m_ptr->confused)
	  mlist_set_uchar(critter, MLIST_confused, m_ptr->confused + 3);
	else
	  mlist_set_uchar(critter, MLIST_confused, 2 + randint(16));
	confuse = true;
	mlist_set_short(critter, MLIST_csleep, 0);
	StrPrintF(out_val, "%s appears confused.", m_name);
	message(out_val);
      }
    }
  }
  return confuse;
}


/* Sleep a creature.					-RAK-	*/
Boolean sleep_monster(Short dir, Short y, Short x)
{
  Boolean flag, sleep;
  Short dist;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80], m_name[80];
  UChar critter;

  sleep = false;
  flag = false;
  dist = 0;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    critter = cave_cptr[y*MAX_WIDTH+x];
    if (dist > OBJ_BOLT_RANGE || cave_fval[y*MAX_WIDTH+x] >= MIN_CLOSED_SPACE)
      flag = true;
    else if (critter > 1) {
      m_ptr = &m_list[critter];
      r_ptr = &c_list[m_ptr->mptr];
      flag = true;
      monster_name (m_name, m_ptr, r_ptr);
      if ((randint(MAX_MONS_LEVEL) < r_ptr->level) ||
	  (CD_NO_SLEEP & r_ptr->cdefense)) {
	if (m_ptr->ml && (r_ptr->cdefense & CD_NO_SLEEP))
	  recall_cdefense_or(m_ptr->mptr, CD_NO_SLEEP);
	StrPrintF(out_val, "%s is unaffected.", m_name);
	message(out_val);
      } else {
	mlist_set_short(critter, MLIST_csleep, 500);
	sleep = true;
	StrPrintF(out_val, "%s falls asleep.", m_name);
	message(out_val);
      }
    }
  }
  return sleep;
}


/* Turn stone to mud, delete wall.			-RAK-	*/
Boolean wall_to_mud(Short dir, Short y, Short x)
{
  Short i, dist;
  Char out_val[160], tmp_str[160];
  Boolean wall, flag;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char m_name[80];
  UChar critter, floor, thing;

  wall = false;
  flag = false;
  dist = 0;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    critter = cave_cptr[y*MAX_WIDTH+x];
    floor = cave_fval[y*MAX_WIDTH+x];
    thing = cave_tptr[y*MAX_WIDTH+x];
    /* note, this ray can move through walls as it turns them to mud */
    if (dist == OBJ_BOLT_RANGE)
      flag = true;
    if ((floor >= MIN_CAVE_WALL) && (floor != BOUNDARY_WALL)) {
      flag = true;
      twall(y, x, 1, 0);
      if (test_light(y, x)) {
	message("The wall turns into mud.");
	wall = true;
      }
    } else if ((thing != 0) && (floor >= MIN_CLOSED_SPACE)) {
      flag = true;
      if (panel_contains(y, x) && test_light(y, x)) {
	objdes(tmp_str, &t_list[thing], false);
	StrPrintF(out_val, "The %s turns into mud.", tmp_str);
	message(out_val);
	wall = true;
      }
      if (t_list[thing].tval == TV_RUBBLE) {
	delete_object(y, x);
	if (randint(10) == 1) {
	  place_object(y, x, false);
	  if (test_light(y, x))
	    message("You have found something!");
	}
	lite_spot(y, x);
      }
      else delete_object(y, x);
    }
    if (critter > 1) {
      m_ptr = &m_list[critter];
      r_ptr = &c_list[m_ptr->mptr];
      if (CD_STONE & r_ptr->cdefense) {
	monster_name (m_name, m_ptr, r_ptr);
	i = mon_take_hit((Short) critter, 100);
	/* Should get these messages even if the monster is not
	   visible.  */
	if (i >= 0) {
	  recall_cdefense_or(i, CD_STONE);
	  StrPrintF(out_val, "%s dissolves!", m_name);
	  message(out_val);
	  print_stats(STATS_QUO);//prt_experience(); /* print msg before calling prt_exp */
	} else {
	  recall_cdefense_or(i, CD_STONE);
	  StrPrintF(out_val, "%s grunts in pain!",m_name);
	  message(out_val);
	}
	flag = true;
      }
    }
  }
  return wall;
}


/* Destroy all traps and doors in a given direction	-RAK-	*/
Boolean td_destroy2(Short dir, Short y, Short x)
{
  Boolean destroy2;
  Short dist;
  inven_type *t_ptr;
  UChar thing;

  destroy2 = false;
  dist= 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    thing = cave_tptr[y*MAX_WIDTH+x];
    /* must move into first closed spot, as it might be a secret door */
    if (thing != 0) {
      t_ptr = &t_list[thing];
      if ((t_ptr->tval == TV_INVIS_TRAP) || (t_ptr->tval == TV_CLOSED_DOOR)
	  || (t_ptr->tval == TV_VIS_TRAP) || (t_ptr->tval == TV_OPEN_DOOR)
	  || (t_ptr->tval == TV_SECRET_DOOR)) {
	if (delete_object(y, x)) {
	  message("There is a bright flash of light!");
	  destroy2 = true;
	}
      } else if ((t_ptr->tval == TV_CHEST) && (t_ptr->flags != 0)) {
	message("Click!");
	tlist_set_flags(t_list, thing, t_ptr->flags & ~(CH_TRAPPED|CH_LOCKED));
	destroy2 = true;
	tlist_set_name2(t_list, thing, SN_UNLOCKED);
	known2(t_list, thing);
      }
    }
  } while ((dist <= OBJ_BOLT_RANGE)
	   || cave_fval[y*MAX_WIDTH+x] <= MAX_OPEN_SPACE);

  return destroy2;
}


/* Polymorph a monster					-RAK-	*/
/* NOTE: cannot polymorph a winning creature (BALROG)		 */
Boolean poly_monster(Short dir, Short y, Short x)
{
  Short dist, flag, poly;
  creature_type *r_ptr;
  monster_type *m_ptr;
  Char out_val[80], m_name[80];
  UChar critter, light;

  poly = false;
  flag = false;
  dist = 0;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    critter = cave_cptr[y*MAX_WIDTH+x];
    light = cave_cptr[y*MAX_WIDTH+x];
    if (dist > OBJ_BOLT_RANGE || cave_fval[y*MAX_WIDTH+x] >= MIN_CLOSED_SPACE)
      flag = true;
    else if (critter > 1)	{
      m_ptr = &m_list[critter];
      r_ptr = &c_list[m_ptr->mptr];
      if (randint(MAX_MONS_LEVEL) > r_ptr->level) {
	flag = true;
	delete_monster((Short) critter);
	/* Place_monster() should always return true here.  */
	poly = place_monster(y, x,
			     randint(m_level[MAX_MONS_LEVEL]-m_level[0])
			     - 1 + m_level[0], false);
	/* don't test c_ptr->fm here, only pl/tl */
	if (poly && panel_contains(y, x) && (light & (LIGHT_tl | LIGHT_pl)))
	  poly = true;
      }
      else {
	monster_name (m_name, m_ptr, r_ptr);
	StrPrintF(out_val, "%s is unaffected.", m_name);
	message(out_val);
      }
    }
  }
  return poly;
}


/* Create a wall.					-RAK-	*/
Boolean build_wall(Short dir, Short y, Short x)
{
  Short i;
  Short damage, dist;
  Boolean build, flag;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char m_name[80], out_val[80];
  UChar critter, light;

  build = false;
  dist = 0;
  flag = false;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    critter = cave_cptr[y*MAX_WIDTH+x];
    light = cave_light[y*MAX_WIDTH+x];
    if (dist > OBJ_BOLT_RANGE || cave_fval[y*MAX_WIDTH+x] >= MIN_CLOSED_SPACE)
      flag = true;
    else {
      if (cave_tptr[y*MAX_WIDTH+x] != 0)
	delete_object(y, x);
      if (critter > 1) {
	/* stop the wall building */
	flag = true;
	m_ptr = &m_list[critter];
	r_ptr = &c_list[m_ptr->mptr];

	if (!(r_ptr->cmove & CM_PHASE)) {
	  /* monster does not move, can't escape the wall */
	  if (r_ptr->cmove & CM_ATTACK_ONLY)
	    damage = 3000; /* this will kill everything */
	  else
	    damage = damroll (4, 8);

	  monster_name (m_name, m_ptr, r_ptr);
	  StrPrintF (out_val, "%s wails out in pain!", m_name);
	  message (out_val);
	  i = mon_take_hit((Short) critter, damage);
	  if (i >= 0) {
	    StrPrintF (out_val, "%s is embedded in the rock.",
		       m_name);
	    message (out_val);
	    print_stats(STATS_QUO);//prt_experience();
	  }
	}
	else if (r_ptr->cchar == 'E' || r_ptr->cchar == 'X') {
	  /* must be an earth elemental or an earth spirit, or a Xorn
	     increase its hit points */
	  mlist_set_short(critter, MLIST_hp, m_ptr->hp + damroll(4, 8));
	}
      }
      cave_set(y, x, CAVE_fval, MAGMA_WALL);
      light &= ~LIGHT_fm; //  c_ptr->fm = false;
      /* Permanently light this wall if it is lit by player's lamp.  */
      if (light & LIGHT_tl)
	light |= LIGHT_pl; // c_ptr->pl = (c_ptr->tl || c_ptr->pl);
      lite_spot(y, x);
      i++;
      build = true;
    }
  }
  return build;
}


/* Replicate a creature					-RAK-	*/
Boolean clone_monster(Short dir, Short y, Short x)
{
  Short dist;
  Boolean flag;
  UChar critter;

  dist = 0;
  flag = false;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    critter = cave_cptr[y*MAX_WIDTH+x];
    if (dist > OBJ_BOLT_RANGE || cave_fval[y*MAX_WIDTH+x] >= MIN_CLOSED_SPACE)
      flag = true;
    else if (critter > 1)	{
      mlist_set_short(critter, MLIST_csleep, 0);
      /* monptr of 0 is safe here, since can't reach here from creatures */
      //return false;
      return multiply_monster(y, x, (Short) m_list[critter].mptr, 0);
    }
  }
  return false;
}


/* Move the creature record to a new location		-RAK-	*/
void teleport_away(Short monptr, Short dis)
{
  Short yn, xn, ctr;
  monster_type *m_ptr;
  UChar floor, critter;

  m_ptr = &m_list[monptr];
  ctr = 0;
  do {
    do {
      yn = m_ptr->fy + (randint(2*dis+1) - (dis + 1));
      xn = m_ptr->fx + (randint(2*dis+1) - (dis + 1));
    } while (!in_bounds(yn, xn));
    ctr++;
    if (ctr > 9) {
      ctr = 0;
      dis += 5;
    }
    floor = cave_fval[yn*MAX_WIDTH+xn];
    critter = cave_cptr[yn*MAX_WIDTH+xn];
  } while ((floor >= MIN_CLOSED_SPACE) || (critter != 0));

  move_rec((Short) m_ptr->fy, (Short) m_ptr->fx, yn, xn);
  lite_spot((Short) m_ptr->fy, (Short) m_ptr->fx);
  mlist_set_uchar(monptr, MLIST_fy, yn);
  mlist_set_uchar(monptr, MLIST_fx, xn);
  /* this is necessary, because the creature is not currently visible
     in its new position */
  mlist_set_ml(monptr, false);
  mlist_set_uchar(monptr, MLIST_cdis, distance(char_row, char_col, yn, xn));
  update_mon (monptr);
}


/* Teleport player to spell casting creature		-RAK-	*/
void teleport_to(Short ny, Short nx)
{
  Short dis, ctr, y, x;
  Short i, j;
  UChar floor, critter;

  dis = 1;
  ctr = 0;
  do {
    y = ny + (randint(2*dis+1) - (dis + 1));
    x = nx + (randint(2*dis+1) - (dis + 1));
    ctr++;
    if (ctr > 9) {
      ctr = 0;
      dis++;
    }
    floor = cave_fval[y*MAX_WIDTH+x];
    critter = cave_cptr[y*MAX_WIDTH+x];
  } while (!in_bounds(y, x) || (floor >= MIN_CLOSED_SPACE) || (critter >= 2));

  move_rec(char_row, char_col, y, x);
  for (i = char_row-1; i <= char_row+1; i++)
    for (j = char_col-1; j <= char_col+1; j++) {
      cave_set(i, j, CAVE_light, cave_light[i*MAX_WIDTH+j] & ~LIGHT_tl);
      lite_spot(i, j);
    }
  lite_spot(char_row, char_col);
  char_row = y;
  char_col = x;
  check_view();
  /* light creatures */
  creatures(false);
}


/* Teleport all creatures in a given direction away	-RAK-	*/
Boolean teleport_monster(Short dir, Short y, Short x)
{
  Boolean flag, result;
  Short dist;
  UChar critter;

  flag = false;
  result = false;
  dist = 0;
  while (!flag) {
    mmove(dir, &y, &x);
    dist++;
    critter = cave_cptr[y*MAX_WIDTH+x];
    if (dist > OBJ_BOLT_RANGE || cave_fval[y*MAX_WIDTH+x] >= MIN_CLOSED_SPACE)
      flag = true;
    else if (critter > 1) {
      mlist_set_short(critter, MLIST_csleep, 0); /* wake it up */
      teleport_away((Short) critter, MAX_SIGHT);
      result = true;
    }
  }
  return result;
}


/* Delete all creatures within max_sight distance	-RAK-	*/
/* NOTE : Winning creatures cannot be genocided			 */
Boolean mass_genocide()
{
  Short i;
  Boolean result;
  monster_type *m_ptr;
  creature_type *r_ptr;

  result = false;
  for (i = mfptr - 1; i >= MIN_MONIX; i--) {
    m_ptr = &m_list[i];
    r_ptr = &c_list[m_ptr->mptr];
    if ((m_ptr->cdis <= MAX_SIGHT) && ((r_ptr->cmove & CM_WIN) == 0)) {
      delete_monster(i);
      result = true;
    }
  }
  return result;
}

/* Delete all creatures of a given type from level.	-RAK-	*/
/* This does not keep creatures of type from appearing later.	 */
/* NOTE : Winning creatures can not be genocided. */
Boolean genocide()
{
  Short i;
  Boolean killed;
  Char typ;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80];

  killed = false;
  if (0) //   if (get_com("Which type of creature do you wish exterminated?", &typ)) // xxx not implemented yet
    for (i = mfptr - 1; i >= MIN_MONIX; i--) {
      m_ptr = &m_list[i];
      r_ptr = &c_list[m_ptr->mptr];
      if (typ == c_list[m_ptr->mptr].cchar) {
	if ((r_ptr->cmove & CM_WIN) == 0) {
	  delete_monster(i);
	  killed = true;
	} else {
	  /* genocide is a powerful spell, so we will let the player
	     know the names of the creatures he did not destroy,
	     this message makes no sense otherwise */
	  StrPrintF(out_val, "The %s is unaffected.",
		    c_names + r_ptr->name_offset);
	  message(out_val);
	}
      }
    }
  return killed;
}


/* Change speed of any creature .			-RAK-	*/
/* NOTE: cannot slow a winning creature (BALROG)		 */
Boolean speed_monsters(Short spd)
{
  Short i, speed;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80], m_name[80];

  speed = false;
  for (i = mfptr - 1; i >= MIN_MONIX; i--) {
    m_ptr = &m_list[i];
    r_ptr = &c_list[m_ptr->mptr];
    monster_name (m_name, m_ptr, r_ptr);

    if ((m_ptr->cdis > MAX_SIGHT) ||
	!los(char_row, char_col, (Short) m_ptr->fy, (Short) m_ptr->fx))
      /* do nothing */
      ;
    else if (spd > 0)	{
      mlist_set_short(i, MLIST_cspeed, m_ptr->cspeed + spd);
      mlist_set_short(i, MLIST_csleep, 0);
      if (m_ptr->ml) {
	speed = true;
	StrPrintF (out_val, "%s starts moving faster.", m_name);
	message (out_val);
      }
    }
    else if (randint(MAX_MONS_LEVEL) > r_ptr->level) {
      mlist_set_short(i, MLIST_cspeed, m_ptr->cspeed + spd);
      mlist_set_short(i, MLIST_csleep, 0);
      if (m_ptr->ml) {
	StrPrintF (out_val, "%s starts moving slower.", m_name);
	message (out_val);
	speed = true;
      }
    }
    else if (m_ptr->ml) {
      mlist_set_short(i, MLIST_csleep, 0);
      StrPrintF(out_val, "%s is unaffected.", m_name);
      message(out_val);
    }
  }
  return speed;
}


/* Sleep any creature .		-RAK-	*/
Boolean sleep_monsters2()
{
  Short i;
  Boolean sleep;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80], m_name[80];

  sleep = false;
  for (i = mfptr - 1; i >= MIN_MONIX; i--) {
    m_ptr = &m_list[i];
    r_ptr = &c_list[m_ptr->mptr];
    monster_name (m_name, m_ptr, r_ptr);
    if ((m_ptr->cdis > MAX_SIGHT) || 
	!los(char_row, char_col, (Short) m_ptr->fy, (Short) m_ptr->fx))
      /* do nothing */
      ;
    else if ((randint(MAX_MONS_LEVEL) < r_ptr->level) ||
	     (CD_NO_SLEEP & r_ptr->cdefense)) {
      if (m_ptr->ml) {
	if (r_ptr->cdefense & CD_NO_SLEEP)
	  recall_cdefense_or(m_ptr->mptr, CD_NO_SLEEP);
	StrPrintF(out_val, "%s is unaffected.", m_name);
	message(out_val);
      }
    }
    else {
      mlist_set_short(i, MLIST_csleep, 500);
      if (m_ptr->ml) {
	StrPrintF(out_val, "%s falls asleep.", m_name);
	message(out_val);
	sleep = true;
      }
    }
  }
  return sleep;
}


/* Polymorph any creature that player can see.	-RAK-	*/
/* NOTE: cannot polymorph a winning creature (BALROG)		 */
Boolean mass_poly()
{
  Short i;
  Short y, x;
  Boolean mass;
  monster_type *m_ptr;
  creature_type *r_ptr;

  mass = false;
  for (i = mfptr - 1; i >= MIN_MONIX; i--) {
    m_ptr = &m_list[i];
    if (m_ptr->cdis <= MAX_SIGHT) {
      r_ptr = &c_list[m_ptr->mptr];
      if ((r_ptr->cmove & CM_WIN) == 0)	{
	y = m_ptr->fy;
	x = m_ptr->fx;
	delete_monster(i);
	/* Place_monster() should always return true here.  */
	mass = place_monster(y, x,
			     randint(m_level[MAX_MONS_LEVEL]-m_level[0])
			     - 1 + m_level[0], false);
      }
    }
  }
  return mass;
}


/* Display evil creatures on current panel		-RAK-	*/
Boolean detect_evil()
{
  Short i;
  Boolean flag;
  monster_type *m_ptr;

  flag = false;
  for (i = mfptr - 1; i >= MIN_MONIX; i--) {
    m_ptr = &m_list[i];
    if (panel_contains((Short) m_ptr->fy, (Short) m_ptr->fx) &&
	(CD_EVIL & c_list[m_ptr->mptr].cdefense)) {
      mlist_set_ml(i, true);
      /* works correctly even if hallucinating */
      print((Char) c_list[m_ptr->mptr].cchar, (Short) m_ptr->fy,
	    (Short) m_ptr->fx);
      flag = true;
    }
  }
  if (flag) {
    message("You sense the presence of evil!");
    //      message(CNIL);
    /* must unlight every monster just lighted */
    creatures(false);
  } else message("test: Your ears itch"); // xxxx just testing
  return flag;
}


/* Change players hit points in some manner		-RAK-	*/
Boolean hp_player(Short num)
{
  Boolean res;

  res = false;
  if (pymisc.chp < pymisc.mhp) {
    pymisc.chp += num;
    if (pymisc.chp > pymisc.mhp) {
      pymisc.chp = pymisc.mhp;
      pymisc.chp_frac = 0;
    }
    print_stats(STATS_QUO); //    prt_chp();

    num = num / 5;
    if (num < 3) {
      if (num == 0) message("You feel a little better.");
      else	      message("You feel better.");
    } else {
      if (num < 7) message("You feel much better.");
      else	     message("You feel very good.");
    }
    res = true;
  }
  return res;
}


/* Cure players confusion				-RAK-	*/
Boolean cure_confusion()
{
  Boolean cure;

  cure = false;
  if (pyflags.confused > 1) {
    pyflags.confused = 1;
    cure = true;
  }
  return cure;
}


/* Cure players blindness				-RAK-	*/
Boolean cure_blindness()
{
  Boolean cure;

  cure = false;
  if (pyflags.blind > 1) {
    pyflags.blind = 1;
    cure = true;
  }
  return cure;
}


/* Cure poisoning					-RAK-	*/
Boolean cure_poison()
{
  Boolean cure;

  cure = false;
  if (pyflags.poisoned > 1) {
    pyflags.poisoned = 1;
    cure = true;
  }
  return cure;
}


/* Cure the players fear				-RAK-	*/
Boolean remove_fear()
{
  Boolean result;

  result = false;
  if (pyflags.afraid > 1) {
    pyflags.afraid = 1;
    result = true;
  }
  return result;
}


/* This is a fun one.  In a given block, pick some walls and	*/
/* turn them into open spots.  Pick some open spots and turn	 */
/* them into walls.  An "Earthquake" effect.	       -RAK-   */
void earthquake()
{
  Short i, j;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Short damage, tmp;
  Char out_val[80], m_name[80];
  UChar critter, floor, light, thing;

  for (i = char_row-8; i <= char_row+8; i++)
    for (j = char_col-8; j <= char_col+8; j++)
      if (((i != char_row) || (j != char_col)) &&
	  in_bounds(i, j) && (randint(8) == 1))	{
	critter = cave_cptr[i*MAX_WIDTH+j];
	thing = cave_tptr[i*MAX_WIDTH+j];
	if (thing != 0)
	  delete_object(i, j);
	if (critter > 1) {
	  m_ptr = &m_list[critter];
	  r_ptr = &c_list[m_ptr->mptr];

	  if (!(r_ptr->cmove & CM_PHASE)) {
	    if(r_ptr->cmove & CM_ATTACK_ONLY)
	      damage = 3000; /* this will kill everything */
	    else
	      damage = damroll (4, 8);

	    monster_name (m_name, m_ptr, r_ptr);
	    StrPrintF (out_val, "%s wails out in pain!", m_name);
	    message (out_val);
	    i = mon_take_hit((Short) critter, damage);
	    if (i >= 0) {
	      StrPrintF (out_val, "%s is embedded in the rock.",
			 m_name);
	      message (out_val);
	      print_stats(STATS_QUO); //prt_experience();
	    }
	  } else if (r_ptr->cchar == 'E' || r_ptr->cchar == 'X') {
	    /* must be an earth elemental or an earth spirit, or a Xorn
	       increase its hit points */
	    mlist_set_short(critter, MLIST_hp, m_ptr->hp + damroll(4, 8));
	  }
	}

	floor = cave_fval[i*MAX_WIDTH+j];
	light = cave_light[i*MAX_WIDTH+j];
	if ((floor >= MIN_CAVE_WALL) && (floor != BOUNDARY_WALL)) {
	  cave_set(i, j, CAVE_fval, CORR_FLOOR);
	  light &= ~(LIGHT_pl | LIGHT_fm);
	  cave_set(i, j, CAVE_light, light);
	} else if (floor <= MAX_CAVE_FLOOR) {
	  tmp = randint(10);
	  if (tmp < 6)
	    cave_set(i, j, CAVE_fval, QUARTZ_WALL);
	  else if (tmp < 9)
	    cave_set(i, j, CAVE_fval, MAGMA_WALL);
	  else
	    cave_set(i, j, CAVE_fval, GRANITE_WALL);

	  cave_set(i, j, CAVE_light, light & ~LIGHT_fm);
	}
	lite_spot(i, j);
      }
}


/* Evil creatures don't like this.		       -RAK-   */
Boolean protect_evil()
{
  Boolean res;

  if (pyflags.protevil == 0)
    res = true;
  else
    res = false;
  pyflags.protevil += randint(25) + 3*pymisc.lev;
  return res;
}


/* Create some high quality mush for the player.	-RAK-	*/
void create_food()
{
  UChar thing;

  thing = cave_tptr[char_row*MAX_WIDTH+char_col];
  if (thing != 0) {
    /* take no action here, don't want to destroy object under player */
    message ("There is already an object under you.");
    /* set free_turn_flag so that scroll/spell points won't be used */
    free_turn_flag = true;
  }
  else {
    place_object(char_row, char_col, false);
    invcopy(t_list, thing, OBJ_MUSH);
  }
}


/* Attempts to destroy a type of creature.  Success depends on	*/
/* the creatures level VS. the player's level		 -RAK-	 */
Boolean dispel_creature(Short cflag, Short damage)
{
  Short i;
  Short k, dispel;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80], m_name[80];

  dispel = false;
  for (i = mfptr - 1; i >= MIN_MONIX; i--) {
    m_ptr = &m_list[i];
    if ((m_ptr->cdis <= MAX_SIGHT) &&
	(cflag & c_list[m_ptr->mptr].cdefense) &&
	los(char_row, char_col, (Short) m_ptr->fy, (Short) m_ptr->fx)) {
      r_ptr = &c_list[m_ptr->mptr];
      recall_cdefense_or(m_ptr->mptr, cflag);
      monster_name (m_name, m_ptr, r_ptr);
      k = mon_take_hit (i, randint(damage));
      /* Should get these messages even if the monster is not
	 visible.  */
      if (k >= 0)
	StrPrintF(out_val, "%s dissolves!", m_name);
      else
	StrPrintF(out_val, "%s shudders.", m_name);
      message(out_val);
      dispel = true;
      if (k >= 0)
	print_stats(STATS_QUO);//prt_experience();
    }
  }
  return dispel;
}


/* Attempt to turn (confuse) undead creatures.	-RAK-	*/
Boolean turn_undead()
{
  Short i;
  Boolean turn_und;
  monster_type *m_ptr;
  creature_type *r_ptr;
  Char out_val[80], m_name[80];

  turn_und = false;
  for (i = mfptr - 1; i >= MIN_MONIX; i--) {
    m_ptr = &m_list[i];
    r_ptr = &c_list[m_ptr->mptr];
    if ((m_ptr->cdis <= MAX_SIGHT) &&
	(CD_UNDEAD & r_ptr->cdefense) &&
	(los(char_row, char_col, (Short) m_ptr->fy, (Short) m_ptr->fx))) {
      monster_name (m_name, m_ptr, r_ptr);
      if (((pymisc.lev+1) > r_ptr->level) ||
	  (randint(5) == 1)) {
	if (m_ptr->ml) {
	  StrPrintF(out_val, "%s runs frantically!", m_name);
	  message(out_val);
	  turn_und = true;
	  recall_cdefense_or(m_ptr->mptr, CD_UNDEAD);
	}
	mlist_set_uchar(i, MLIST_confused, pymisc.lev);
      } else if (m_ptr->ml) {
	StrPrintF(out_val, "%s is unaffected.", m_name);
	message(out_val);
      }
    }
  }
  return turn_und;
}


/* Leave a glyph of warding. Creatures will not pass over! -RAK-*/
void warding_glyph()
{
  Short i;
  UChar thing;

  thing = cave_tptr[char_row*MAX_WIDTH+char_col];
  if (thing == 0) {
    i = popt();
    thing = i;
    invcopy(t_list, i, OBJ_SCARE_MON);
  }
}


/* Lose a strength point.				-RAK-	*/
void lose_str()
{
  if (!pyflags.sustain_str) {
    dec_stat (A_STR);
    message("You feel very sick.");
  }
  else
    message("You feel sick for a moment,  it passes.");
}


/* Lose an intelligence point.				-RAK-	*/
void lose_int()
{
  if (!pyflags.sustain_int) {
    dec_stat(A_INT);
    message("You become very dizzy.");
  }
  else
    message("You become dizzy for a moment,  it passes.");
}


/* Lose a wisdom point.					-RAK-	*/
void lose_wis()
{
  if (!pyflags.sustain_wis) {
    dec_stat(A_WIS);
    message("You feel very naive.");
  }
  else
    message("You feel naive for a moment,  it passes.");
}


/* Lose a dexterity point.				-RAK-	*/
void lose_dex()
{
  if (!pyflags.sustain_dex) {
    dec_stat(A_DEX);
    message("You feel very sore.");
  }
  else
    message("You feel sore for a moment,  it passes.");
}


/* Lose a constitution point.				-RAK-	*/
void lose_con()
{
  if (!pyflags.sustain_con) {
    dec_stat(A_CON);
    message("You feel very sick.");
  }
  else
    message("You feel sick for a moment,  it passes.");
}


/* Lose a charisma point.				-RAK-	*/
void lose_chr()
{
  if (!pyflags.sustain_chr) {
    dec_stat(A_CHR);
    message("Your skin starts to itch.");
  }
  else
    message("Your skin itches briefly.");
}


/* Lose experience					-RAK-	*/
void lose_exp(Long amount)
{
  Short i;
  class_type *c_ptr;

  if (amount > pymisc.exp)
    pymisc.exp = 0;
  else
    pymisc.exp -= amount;
  print_stats(STATS_QUO);//prt_experience();

  i = 0;
  while ((player_exp[i] * pymisc.expfact / 100) <= pymisc.exp)
    i++;
  /* increment i once more, because level 1 exp is stored in player_exp[0] */
  i++;

  if (pymisc.lev != i) {
    pymisc.lev = i;

    calc_hitpoints();
    c_ptr = &class[pymisc.pclass];
    if (c_ptr->spell == MAGE) {
      calc_spells(A_INT);
      calc_mana(A_INT);
    } else if (c_ptr->spell == PRIEST) {
      calc_spells(A_WIS);
      calc_mana(A_WIS);
    }
    //    prt_level(); // xx not implemented yet
    //    prt_title(); // xxx not implemented yet
  }
}


/* Slow Poison						-RAK-	*/
Boolean slow_poison()
{
  Boolean slow;

  slow = false;
  if (pyflags.poisoned > 0) {
    pyflags.poisoned = pyflags.poisoned / 2;
    if (pyflags.poisoned < 1)	pyflags.poisoned = 1;
    slow = true;
    message("The effect of the poison has been reduced.");
  }
  return slow;
}


/* Bless						-RAK-	*/
void bless(Short amount)
{
  pyflags.blessed += amount;
}


/* Detect Invisible for period of time			-RAK-	*/
void detect_inv2(Short amount)
{
  pyflags.detect_inv += amount;
}


static void replace_spot(Short y, Short x, Short typ)
{
  UChar light, thing, critter;

  light = cave_light[y*MAX_WIDTH+x];
  switch(typ) {
  case 1: case 2: case 3:
    cave_set(y, x, CAVE_fval, CORR_FLOOR);
    break;
  case 4: case 7: case 10:
    cave_set(y, x, CAVE_fval, GRANITE_WALL);
    break;
  case 5: case 8: case 11:
    cave_set(y, x, CAVE_fval, MAGMA_WALL);
    break;
  case 6: case 9: case 12:
    cave_set(y, x, CAVE_fval, QUARTZ_WALL);
    break;
  }
  light &= ~(LIGHT_pl | LIGHT_fm | LIGHT_lr);
  cave_set(y, x, CAVE_light, light); /* this is no longer part of a room */
  thing = cave_tptr[y*MAX_WIDTH+x];
  if (thing != 0)
    delete_object(y, x);
  critter = cave_cptr[y*MAX_WIDTH+x];
  if (critter > 1)
    delete_monster((Short) critter);
}


/* The spell of destruction.				-RAK-	*/
/* NOTE : Winning creatures that are deleted will be considered	 */
/*	  as teleporting to another level.  This will NOT win the*/
/*	  game.						       */
void destroy_area(Short y, Short x)
{
  Short i, j, k;

  if (dun_level > 0) {
    for (i = (y-15); i <= (y+15); i++)
      for (j = (x-15); j <= (x+15); j++)
	if (in_bounds(i, j) && (cave_fval[i*MAX_WIDTH+j] != BOUNDARY_WALL)) {
	  k = distance(i, j, y, x);
	  if (k == 0) /* clear player's spot, but don't put wall there */
	    replace_spot(i, j, 1);
	  else if (k < 13)
	    replace_spot(i, j, randint(6));
	  else if (k < 16)
	    replace_spot(i, j, randint(9));
	}
  }
  message("There is a searing blast of light!");
  pyflags.blind += 10 + randint(10);
}


/* Enchants a plus onto an item.			-RAK-	*/
/* limit = maximum bonus allowed; usually 10, but weapon's maximum damage
   when enchanting melee weapons to damage */
Boolean enchant(Short *plusses, Short limit)
{
  Short chance;
  Boolean res;
  
  if (limit <= 0) /* avoid randint(0) call */
    return false;
  chance = 0;
  res = false;
  if (*plusses > 0) {
    chance = *plusses;
    if (randint(100) == 1) /* very rarely allow enchantment over limit */
      chance = randint(chance) - 1;
  }
  if (randint(limit) > chance) {
    *plusses += 1;
    res = true;
  }
  return res;
}


/* Removes curses from items in inventory		-RAK-	*/
Boolean remove_curse()
{
  Short i;
  Boolean result;
  inven_type *i_ptr;

  result = false;
  for (i = INVEN_WIELD; i <= INVEN_OUTER; i++) {
    i_ptr = &inventory[i];
    if (TR_CURSED & i_ptr->flags) {
      invy_set_flags(inventory, i, i_ptr->flags & ~TR_CURSED);
      calc_bonuses();
      result = true;
    }
  }
  return result;
}


/* Restores any drained experience			-RAK-	*/
Boolean restore_level()
{
  Boolean restore;

  restore = false;
  if (pymisc.max_exp > pymisc.exp) {
    restore = true;
    message("You feel your life energies returning.");
    /* this While loop is not redundant, prt_exp may reduce the exp level */
    while (pymisc.exp < pymisc.max_exp) {
      pymisc.exp = pymisc.max_exp;
      print_stats(STATS_QUO);//prt_experience();
    }
  }
  return restore;
}
