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

// I have not grokked the 'look'-related code.
// This might be necessary to see if there will be division problems.

static Char toupper(Char c) SEC_3;
//void tunnel(Short dir) SEC_3;
//void disarm_trap() SEC_3;
//void look() SEC_3;
static Short look_ray(Short y, Short from, Short to) SEC_3;
static Boolean look_see(Short x, Short y, Boolean *transparent) SEC_3;
static void facts(inven_type *i_ptr, Short *tbth, Short *tpth,
		  Short *tdam, Short *tdis) SEC_3;
static void drop_throw(Short y, Short x, inven_type *t_ptr) SEC_3;
//void throw_object(Short item_val) SEC_3;
static void inven_throw(Short item_val, inven_type *t_ptr) SEC_3;
static void py_bash(Short y, Short x) SEC_3;
//void bash() SEC_3;

static Char toupper(Char c)
{
  if (c < 'a' || c > 'z') return c;
  return ('A' + (c - 'a'));
}


/* Tunnels through rubble and walls			-RAK-	*/
/* Must take into account: secret doors,  special tools		  */
void tunnel(Short dir)
{
  Short i, tabil;
  //  register cave_type *c_ptr;
  inven_type *i_ptr;
  Short y, x;
  monster_type *m_ptr;
  Char out_val[80], m_name[80];
  UChar floor, thing, critter;

  if ((pyflags.confused > 0) &&    /* Confused?	     */
      (randint(4) > 1))		    /* 75% random movement   */
    dir = randint(9);
  y = char_row;
  x = char_col;
  mmove(dir, &y, &x);

  //  c_ptr = &cave[y][x];
  floor = cave_fval[y*MAX_WIDTH+x];
  thing = cave_tptr[y*MAX_WIDTH+x];
  critter = cave_cptr[y*MAX_WIDTH+x];
  /* Compute the digging ability of player; based on	   */
  /* strength, and type of tool used			   */
  tabil = pystats.use_stat[A_STR];
  i_ptr = &inventory[INVEN_WIELD];

  /* Don't let the player tunnel somewhere illegal, this is necessary to
     prevent the player from getting a free attack by trying to tunnel
     somewhere where it has no effect.  */
  if (floor < MIN_CAVE_WALL
      && (thing == 0 || (t_list[thing].tval != TV_RUBBLE
			 && t_list[thing].tval != TV_SECRET_DOOR))) {
    if (thing == 0) {
      message ("Tunnel through what?  Empty air?!?");
      free_turn_flag = true;
    } else {
      message("You can't tunnel through that.");
      free_turn_flag = true;
    }
    return;
  }

  if (critter > 1) {
    m_ptr = &m_list[critter];
    if (m_ptr->ml)
      StrPrintF (m_name, "The %s", c_names + c_list[m_ptr->mptr].name_offset);
    else
      StrCopy (m_name, "Something");
    StrPrintF(out_val, "%s is in your way!", m_name);
    message(out_val);

    /* let the player attack the creature */
    if (pyflags.afraid < 1)
      py_attack(y, x);
    else
      message("You are too afraid!");

  } else if (i_ptr->tval != TV_NOTHING) {
      if (TR_TUNNEL & i_ptr->flags)
	tabil += 25 + i_ptr->p1*50;
      else {
	tabil += (i_ptr->damage[0]*i_ptr->damage[1])+i_ptr->tohit+i_ptr->todam;
	/* divide by two so that digging without shovel isn't too easy */
	tabil >>= 1;
      }

    /* If this weapon is too heavy for the player to wield properly, then
       also make it harder to dig with it.  */

    if (weapon_heavy) {
      tabil += (pystats.use_stat[A_STR] * 15) - i_ptr->weight;
      if (tabil < 0)
	tabil = 0;
    }

    /* Regular walls; Granite, magma intrusion, quartz vein  */
    /* Don't forget the boundary walls, made of titanium (255)*/
    switch(floor) {
    case GRANITE_WALL:
      i = randint(1200) + 80;
      if (twall(y, x, tabil, i))
	message("You have finished the tunnel.");
      else
	count_message("You tunnel into the granite wall.");
      break;
    case MAGMA_WALL:
      i = randint(600) + 10;
      if (twall(y, x, tabil, i))
	message("You have finished the tunnel.");
      else
	count_message("You tunnel into the magma intrusion.");
      break;
    case QUARTZ_WALL:
      i = randint(400) + 10;
      if (twall(y, x, tabil, i))
	message("You have finished the tunnel.");
      else
	count_message("You tunnel into the quartz vein.");
      break;
    case BOUNDARY_WALL:
      message("This seems to be permanent rock.");
      break;
    default:
      /* Is there an object in the way?  (Rubble and secret doors)*/
      if (thing != 0) {
	if (t_list[thing].tval == TV_RUBBLE) {
	  /* Rubble.     */
	  if (tabil > randint(180)) {
	    delete_object(y, x);
	    message("You have removed the rubble.");
	    if (randint(10) == 1) {
	      place_object(y, x, false);
	      if (test_light(y, x))
		message("You have found something!");
	    }
	    lite_spot(y, x);
	  } else
	    count_message("You dig in the rubble.");
	} else if (t_list[thing].tval == TV_SECRET_DOOR) {
	  /* Secret doors.*/
	  count_message("You tunnel into the granite wall.");
	  search(char_row, char_col, pymisc.srh);
	} else {
	  //abort();// Oh please.  I don't have SIGABORT
	  command_count = 0; return; // maybe this will work instead
	}
      } else {
	//abort();// Oh please.  I don't have SIGABORT
	command_count = 0; return; // maybe this will work instead
      }
      break;
    }
  } else
    message("You dig with your hands, making no progress.");
}


/* Disarms a trap					-RAK-	*/
void disarm_trap(Short dir)
{
  Short y, x, level, tmp;
  Boolean no_disarm;
  Short tot, i;
  //  register cave_type *c_ptr;
  inven_type *i_ptr;
  monster_type *m_ptr;
  Char m_name[80], out_val[80];
  UChar critter, thing;

  y = char_row;
  x = char_col;
  mmove(dir, &y, &x);
  thing = cave_tptr[y*MAX_WIDTH+x];
  critter = cave_cptr[y*MAX_WIDTH+x];
  no_disarm = false;
  if (critter > 1 && thing != 0 &&
      (t_list[thing].tval == TV_VIS_TRAP
       || t_list[thing].tval == TV_CHEST)) {
    m_ptr = &m_list[critter];
    if (m_ptr->ml)
      StrPrintF (m_name, "The %s",c_names + c_list[m_ptr->mptr].name_offset);
    else
      StrCopy (m_name, "Something");
    StrPrintF(out_val, "%s is in your way!", m_name);
    message(out_val);
  } else if (thing != 0) {
    tot = pymisc.disarm + 2*todis_adj() + stat_adj(A_INT)
      + (class_level_adj[pymisc.pclass][CLA_DISARM] * pymisc.lev / 3);
    if ((pyflags.blind > 0) || (no_light()))
      tot = tot / 10;
    if (pyflags.confused > 0)
      tot = tot / 10;
    if (pyflags.image > 0)
      tot = tot / 10;
    i_ptr = &t_list[thing];
    i = i_ptr->tval;
    level = i_ptr->level;
    if (i == TV_VIS_TRAP) {
      /* Floor trap    */
      if ((tot + 100 - level) > randint(100)) {
	message("You have disarmed the trap.");
	pymisc.exp += i_ptr->p1;
	delete_object(y, x);
	/* make sure we move onto the trap even if confused */
	tmp = pyflags.confused;
	pyflags.confused = 0;
	move_char(dir, false);
	pyflags.confused = tmp;
	print_stats(STATS_QUO);//prt_experience();
      } else if ((tot > 5) && (randint(tot) > 5))
	/* avoid randint(0) call */
	count_message("You failed to disarm the trap.");
      else {
	message("You set the trap off!");
	/* make sure we move onto the trap even if confused */
	tmp = pyflags.confused;
	pyflags.confused = 0;
	move_char(dir, false);
	pyflags.confused += tmp;
      }
    } else if (i == TV_CHEST) {
      if (!known2_p(i_ptr)) {
	message("I don't see a trap.");
	free_turn_flag = true;
      } else if (CH_TRAPPED & i_ptr->flags) {
	if ((tot - level) > randint(100)) {
	  invy_set_flags(t_list, thing, i_ptr->flags & ~CH_TRAPPED);
	  if (CH_LOCKED & i_ptr->flags)
	    invy_set_name2(t_list, thing, SN_LOCKED);
	  else
	    invy_set_name2(t_list, thing, SN_DISARMED);
	  message("You have disarmed the chest.");
	  known2(t_list, thing);
	  pymisc.exp += level;
	  print_stats(STATS_QUO);//prt_experience();
	} else if ((tot > 5) && (randint(tot) > 5))
	  count_message("You failed to disarm the chest.");
	else {
	  message("You set a trap off!");
	  known2(t_list, thing);
	  chest_trap(y, x);
	}
      } else {
	message("The chest was not trapped.");
	free_turn_flag = true;
      }
    } else
      no_disarm = true;
  } else
    no_disarm = true;

  if (no_disarm) {
    message("I do not see anything to disarm there.");
    free_turn_flag = true;
  }
}


/* An enhanced look, with peripheral vision. Looking all 8	-CJS-
   directions will see everything which ought to be visible. Can
   specify direction 5, which looks in all directions.

   For the purpose of hindering vision, each place is regarded as
   a diamond just touching its four immediate neighbours. A
   diamond is opaque if it is a wall, or shut door, or something
   like that. A place is visible if any part of its diamond is
   visible: i.e. there is a line from the view point to part of
   the diamond which does not pass through any opaque diamonds.

   Consider the following situation:

     @....			    X	X   X	X   X
     .##..			   / \ / \ / \ / \ / \
     .....			  X @ X . X . X 1 X . X
				   \ / \ / \ / \ / \ /
				    X	X   X	X   X
	   Expanded view, with	   / \ / \ / \ / \ / \
	   diamonds inscribed	  X . X # X # X 2 X . X
	   about each point,	   \ / \ / \ / \ / \ /
	   and some locations	    X	X   X	X   X
	   numbered.		   / \ / \ / \ / \ / \
				  X . X . X . X 3 X 4 X
				   \ / \ / \ / \ / \ /
				    X	X   X	X   X
	- Location 1 is fully visible.
	- Location 2 is visible, even though partially obscured.
	- Location 3 is invisible, but if either # were
	  transparent, it would be visible.
	- Location 4 is completely obscured by a single #.

   The function which does the work is look_ray. It sets up its
   own co-ordinate frame (global variables map back to the
   dungeon frame) and looks for everything between two angles
   specified from a central line. It is recursive, and each call
   looks at stuff visible along a line parallel to the center
   line, and a set distance away from it. A diagonal look uses
   more extreme peripheral vision from the closest horizontal and
   vertical directions; horizontal or vertical looks take a call
   for each side of the central line. */

/* Globally accessed variables: gl_nseen counts the number of places where
   something is seen. gl_rock indicates a look for rock or objects.

   The others map co-ords in the ray frame to dungeon co-ords.

   dungeon y = char_row	 + gl_fyx * (ray x)  + gl_fyy * (ray y)
   dungeon x = char_col	 + gl_fxx * (ray x)  + gl_fxy * (ray y) */
static Short gl_fxx, gl_fxy, gl_fyx, gl_fyy;
static Short gl_nseen, gl_noquery;
static Short gl_rock;
/* Intended to be indexed by dir/2, since is only relevant to horizontal or
   vertical directions. */
static Short set_fxy[] = { 0,  1,	 0,  0, -1 };
static Short set_fxx[] = { 0,  0, -1,  1,	 0 };
static Short set_fyy[] = { 0,  0,	 1, -1,	 0 };
static Short set_fyx[] = { 0,  1,	 0,  0, -1 };
/* Map diagonal-dir/2 to a normal-dir/2. */
static Short map_diag1[] = { 1, 3, 0, 2, 4 };
static Short map_diag2[] = { 2, 1, 0, 4, 3 };

#define GRADF	10000	/* Any sufficiently big number will do */

/* Look at what we can see. This is a free move.

   Prompts for a direction, and then looks at every object in
   turn within a cone of vision in that direction. For each
   object, the cursor is moved over the object, a description is
   given, and we wait for the user to type something. Typing
   ESCAPE will abort the entire look.

   Looks first at real objects and monsters, and looks at rock
   types only after all other things have been seen.  Only looks at rock
   types if the highlight_seams option is set. */

void look(Short dir)
{
  Short i;
  Boolean dummy, abort;

  if (pyflags.blind > 0) {
    message("You can't see a damn thing!");
    return;
  }
  if (pyflags.image > 0) {
    message("You can't believe what you are seeing! It's like a dream!");
    return;
  }

  abort = false;
  gl_nseen = 0;
  gl_rock = 0;
  gl_noquery = false;	/* Have to set this up for the look_see */
  if (look_see(0, 0, &dummy))
    abort = true;
  else {
    do {
      abort = false;
      if (dir == 5) {
	for (i = 1; i <= 4; i++) {
	  gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
	  gl_fxy = set_fxy[i]; gl_fyy = set_fyy[i];
	  if (look_ray(0, 2*GRADF-1, 1)) {
	    abort = true;
	    break;
	  }
	  gl_fxy = -gl_fxy;
	  gl_fyy = -gl_fyy;
	  if (look_ray(0, 2*GRADF, 2)) {
	    abort = true;
	    break;
	  }
	}
      } else if ((dir & 1) == 0)	/* Straight directions */ {
	i = dir >> 1;
	gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
	gl_fxy = set_fxy[i]; gl_fyy = set_fyy[i];
	if (look_ray(0, GRADF, 1))
	  abort = true;
	else {
	  gl_fxy = -gl_fxy;
	  gl_fyy = -gl_fyy;
	  abort = look_ray(0, GRADF, 2);
	}
      } else {
	i = map_diag1[dir >> 1];
	gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
	gl_fxy = -set_fxy[i]; gl_fyy = -set_fyy[i];
	if (look_ray(1, 2*GRADF, GRADF))
	  abort = true;
	else {
	  i = map_diag2[dir >> 1];
	  gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
	  gl_fxy = set_fxy[i]; gl_fyy = set_fyy[i];
	  abort = look_ray(1, 2*GRADF-1, GRADF);
	}
      }
    } while (abort == false && highlight_seams && (++gl_rock < 2));
    if (abort)
      message("--Aborting look--");
    else {
      if (gl_nseen) {
	if (dir == 5)
	  message("That's all you see.");
	else
	  message("That's all you see in that direction.");
      }
      else if (dir == 5)
	message("You see nothing of interest.");
      else
	message("You see nothing of interest in that direction.");
    }
  }
}

/* Look at everything within a cone of vision between two ray
   lines emanating from the player, and y or more places away
   from the direct line of view. This is recursive.

   Rays are specified by gradients, y over x, multiplied by
   2*GRADF. This is ONLY called with gradients between 2*GRADF
   (45 degrees) and 1 (almost horizontal).

   (y axis)/ angle from
     ^	  /	    ___ angle to
     |	 /	 ___
  ...|../.....___.................... parameter y (look at things in the
     | /   ___			      cone, and on or above this line)
     |/ ___
     @-------------------->   direction in which you are looking. (x axis)
     |
     | */
static Short look_ray(Short y, Short from, Short to)
{
  Short max_x, x;
  Boolean transparent;

  /* from is the larger angle of the ray, since we scan towards the
     center line. If from is smaller, then the ray does not exist. */
  if (from <= to || y > MAX_SIGHT)
    return false;
  /* Find first visible location along this line. Minimum x such
     that (2x-1)/x < from/GRADF <=> x > GRADF(2x-1)/from. This may
     be called with y=0 whence x will be set to 0. Thus we need a
     special fix. */
  x = (Short) ((Long)GRADF * (2 * y - 1) / from + 1);
  if (x <= 0)
    x = 1;

  /* Find last visible location along this line.
     Maximum x such that (2x+1)/x > to/GRADF <=> x < GRADF(2x+1)/to */
  max_x = (Short) (((Long)GRADF * (2 * y + 1) - 1) / to);
  if (max_x > MAX_SIGHT)
    max_x = MAX_SIGHT;
  if (max_x < x)
    return false;

  /* gl_noquery is a HACK to prevent doubling up on direct lines of
     sight. If 'to' is	greater than 1, we do not really look at
     stuff along the direct line of sight, but we do have to see
     what is opaque for the purposes of obscuring other objects. */
  if ((y == 0 && to > 1) || (y == x && from < GRADF*2))
    gl_noquery = true;
  else
    gl_noquery = false;
  if (look_see(x, y, &transparent))
    return true;
  if (y == x)
    gl_noquery = false;
  if (transparent)
    goto init_transparent;

  for (;;) {
    /* Look down the window we've found. */
    if (look_ray(y+1, from, (Short) ((2 * y + 1) * (Long)GRADF / x)))
      return true;
    /* Find the start of next window. */
    do {
      if (x == max_x)
	return false;
      /* See if this seals off the scan. (If y is zero, then it will.) */
      from = (Short) ((2 * y - 1) * (Long)GRADF / x);
      if (from <= to)
	return false;
      x++;
      if (look_see(x, y, &transparent))
	return true;
    } while(!transparent);
  init_transparent:
    /* Find the end of this window of visibility. */
    do {
      if (x == max_x)
	/* The window is trimmed by an earlier limit. */
	return look_ray(y+1, from, to);
      x++;
      if (look_see(x, y, &transparent))
	return true;
    } while(transparent);
  }
}

// This will need some serious form-izing
// also, hello, a little use of MESSAGE()
static Boolean look_see(Short x, Short y, Boolean *transparent)
{
  Char *dstring, *string, query, *monname;
  //  register cave_type *c_ptr;
  Short j;
  Char out_val[160], tmp_str[160];
  UChar floor, critter, light, thing;

  if (x < 0 || y < 0 || y > x) {
    StrPrintF(tmp_str, "Illegal call to look_see(%d, %d)", x, y);
    message(tmp_str);
  }
  if (x == 0 && y == 0)
    dstring = "You are on";
  else
    dstring = "You see";
  j = char_col + gl_fxx * x + gl_fxy * y;
  y = char_row + gl_fyx * x + gl_fyy * y;
  x = j;
  if (!panel_contains(y, x)) {
    *transparent = false;
    return false;
  }
  //  c_ptr = &cave[y][x];
  floor = cave_fval[y*MAX_WIDTH+x];
  thing = cave_tptr[y*MAX_WIDTH+x];
  critter = cave_cptr[y*MAX_WIDTH+x];
  light = cave_light[y*MAX_WIDTH+x];
  *transparent = floor <= MAX_OPEN_SPACE;
  if (gl_noquery)
    return false; /* Don't look at a direct line of sight. A hack. */
  out_val[0] = 0;
  if (gl_rock == 0 && critter > 1 && m_list[critter].ml) {
    j = m_list[critter].mptr;
    monname = c_names + c_list[j].name_offset;
    //    StrPrintF(out_val, "%s %s %s. [(r)ecall]",
    StrPrintF(out_val, "%s %s %s.",
	      dstring,  is_a_vowel(monname[0]) ? "an" : "a",  monname);
    dstring = "It is on";
    message(out_val);
    //    prt(out_val, 0, 0); // xxx not implemented yet
    //    move_cursor_relative(y, x); // xxx not implemented yet
    //    query = inkey(); // xxx not implemented yet
    //    if (query == 'r' || query == 'R') {
      //      save_screen(); // xxx not implemented yet
      //      query = roff_recall(j); // XXX see form_recall.c !
      //      restore_screen(); // xxx not implemented yet
    //    }
  }
  if (light & (LIGHT_tl | LIGHT_pl | LIGHT_fm)) {
    if (thing != 0) {
      if (t_list[thing].tval == TV_SECRET_DOOR)
	goto granite;
      if (gl_rock ==0 && t_list[thing].tval != TV_INVIS_TRAP) {
	objdes(tmp_str, &t_list[thing], true);
	//	StrPrintF(out_val, "%s %s ---pause---", dstring, tmp_str);
	StrPrintF(out_val, "%s %s.", dstring, tmp_str);
	dstring = "It is in";
	message(out_val);
	//	prt(out_val, 0, 0); // xxx not implemented yet
	//	move_cursor_relative(y, x); // xxx not implemented yet
	//	query = inkey(); // xxx not implemented yet
      }
    }
    if ((gl_rock || out_val[0]) && floor >= MIN_CLOSED_SPACE) {
      switch(floor) {
      case BOUNDARY_WALL:
      case GRANITE_WALL:
 granite:
      /* Granite is only interesting if it contains something. */
      if (out_val[0])
	string = "a granite wall";
      else
	string = CNIL;	/* In case we jump here */
      break;
      case MAGMA_WALL: string = "some dark rock";
	break;
      case QUARTZ_WALL: string = "a quartz vein";
	break;
      default: string = CNIL;
	break;
      }
      if (string) {
	//	StrPrintF(out_val, "%s %s ---pause---", dstring, string);
	StrPrintF(out_val, "%s %s.", dstring, string);
	message(out_val);
	//	prt(out_val, 0, 0); // xxx not implemented yet
	//	move_cursor_relative(y, x); // xxx not implemented yet
	//	query = inkey(); // xxx not implemented yet
      }
    }
  }
  if (out_val[0]) {
    gl_nseen++;
    //    if (query == ESCAPE)
    //      return true;
  }

  return false;
}


static void inven_throw(Short item_val, inven_type *t_ptr)
{
  inven_type *i_ptr;

  i_ptr = &inventory[item_val];
  *t_ptr = *i_ptr; /* damn.  well, t_ptr is only used in one location,
		      and it is not a database record, so I *guess*
		      that I can safely leave this like it is... -bas */
  if (i_ptr->number > 1) {
    t_ptr->number = 1;
    invy_set_number(inventory, item_val, i_ptr->number - 1);
    inven_weight -= i_ptr->weight;
    pyflags.status |= PY_STR_WGT;
  } else
    inven_destroy(item_val);
}


/* Obtain the hit and damage bonuses and the maximum distance for a
   thrown missile. */
static void facts(inven_type *i_ptr, Short *tbth, Short *tpth, Short *tdam, Short *tdis)
{
  Short tmp_weight;

  if (i_ptr->weight < 1)
    tmp_weight = 1;
  else
    tmp_weight = i_ptr->weight;

  /* Throwing objects			*/
  *tdam = pdamroll(i_ptr->damage) + i_ptr->todam;
  *tbth = pymisc.bthb * 75 / 100;
  *tpth = pymisc.ptohit + i_ptr->tohit;

  /* Add this back later if the correct throwing device. -CJS- */
  if (inventory[INVEN_WIELD].tval != TV_NOTHING)
    *tpth -= inventory[INVEN_WIELD].tohit;

  *tdis = (((pystats.use_stat[A_STR]+20)*10)/tmp_weight);
  if (*tdis > 10)  *tdis = 10;

  /* multiply damage bonuses instead of adding, when have proper
     missile/weapon combo, this makes them much more useful */

  /* Using Bows,  slings,  or crossbows	*/
  if (inventory[INVEN_WIELD].tval == TV_BOW)
    switch(inventory[INVEN_WIELD].p1) {
    case 1:
      if (i_ptr->tval == TV_SLING_AMMO) {	/* Sling and ammo */
	*tbth = pymisc.bthb;
	*tpth += 2 * inventory[INVEN_WIELD].tohit;
	*tdam += inventory[INVEN_WIELD].todam;
	*tdam = *tdam * 2;
	*tdis = 20;
      }
      break;
    case 2:
      if (i_ptr->tval == TV_ARROW) {	/* Short Bow and Arrow	*/
	*tbth = pymisc.bthb;
	*tpth += 2 * inventory[INVEN_WIELD].tohit;
	*tdam += inventory[INVEN_WIELD].todam;
	*tdam = *tdam * 2;
	*tdis = 25;
      }
      break;
    case 3:
      if (i_ptr->tval == TV_ARROW) {      /* Long Bow and Arrow	*/
	*tbth = pymisc.bthb;
	*tpth += 2 * inventory[INVEN_WIELD].tohit;
	*tdam += inventory[INVEN_WIELD].todam;
	*tdam = *tdam * 3;
	*tdis = 30;
      }
      break;
    case 4:
      if (i_ptr->tval == TV_ARROW) {     /* Composite Bow and Arrow*/
	*tbth = pymisc.bthb;
	*tpth += 2 * inventory[INVEN_WIELD].tohit;
	*tdam += inventory[INVEN_WIELD].todam;
	*tdam = *tdam * 4;
	*tdis = 35;
      }
      break;
    case 5:
      if (i_ptr->tval == TV_BOLT) {     /* Light Crossbow and Bolt*/
	*tbth = pymisc.bthb;
	*tpth += 2 * inventory[INVEN_WIELD].tohit;
	*tdam += inventory[INVEN_WIELD].todam;
	*tdam = *tdam * 3;
	*tdis = 25;
      }
      break;
    case 6:
      if (i_ptr->tval == TV_BOLT) {     /* Heavy Crossbow and Bolt*/
	*tbth = pymisc.bthb;
	*tpth += 2 * inventory[INVEN_WIELD].tohit;
	*tdam += inventory[INVEN_WIELD].todam;
	*tdam = *tdam * 4;
	*tdis = 35;
      }
      break;
    }
}


static void drop_throw(Short y, Short x, inven_type *t_ptr)
{
  Short i, j, k, ij;
  Short flag, cur_pos;
  Char out_val[160], tmp_str[160];

  flag = false;
  i = y;
  j = x;
  k = 0;
  if (randint(10) > 1) {
    do {
      if (in_bounds(i, j)) {
	ij = i*MAX_WIDTH+j;
	if (cave_fval[ij] <= MAX_OPEN_SPACE && cave_tptr[ij] == 0)
	  flag = true;
      }
      if (!flag) {
	i = y + randint(3) - 2;
	j = x + randint(3) - 2;
	k++;
      }
    } while ((!flag) && (k <= 9));
  }
  if (flag) {
    cur_pos = popt();
    cave_set(i, j, CAVE_tptr, cur_pos);
    DmWrite(t_list, cur_pos*sizeof(inven_type), t_ptr, sizeof(inven_type));
    lite_spot(i, j);
  } else {
    objdes(tmp_str, t_ptr, false);
    StrPrintF(out_val, "The %s disappears.", tmp_str);
    message(out_val);
  }
}

/* Throw an object across the dungeon.		-RAK-	*/
/* Note: Flasks of oil do fire damage				 */
/* Note: Extra damage and chance of hitting when missiles are used*/
/*	 with correct weapon.  I.E.  wield bow and throw arrow.	 */
void throw_object(Short item_val, Short dir)
{
  Short tbth, tpth, tdam, tdis;
  Short y, x, oldy, oldx, cur_dis;
  Boolean flag, visible;
  Char out_val[160], tmp_str[160];
  inven_type throw_obj;
  //  register cave_type *c_ptr;
  monster_type *m_ptr;
  Short i;
  Char tchar;
  UChar floor, critter, light;

  //  if (inven_ctr == 0) {
  //    message("But you are not carrying anything.");
  //    free_turn_flag = true;
  //  } else if (get_item(&item_val, "Fire/Throw which one?", 0, inven_ctr-1,
  //		      CNIL, CNIL)) {

  //  if (!get_dir(CNIL, &dir)) return; // xxx not implemented yet
  desc_remain(item_val);
  if (pyflags.confused > 0) {
    message("You are confused.");
    do {
      dir = randint(9);
    } while (dir == 5);
  }
  inven_throw(item_val, &throw_obj);
  facts(&throw_obj, &tbth, &tpth, &tdam, &tdis);
  tchar = throw_obj.tchar;
  flag = false;
  y = char_row;
  x = char_col;
  oldy = char_row;
  oldx = char_col;
  cur_dis = 0;
  // it's ok up to here.//  return;
  while (!flag) {
    mmove(dir, &y, &x);
    cur_dis++;
    lite_spot(oldy, oldx);
    if (cur_dis > tdis)  flag = true;
    //	c_ptr = &cave[y][x];
    floor = cave_fval[y*MAX_WIDTH+x];
    critter = cave_cptr[y*MAX_WIDTH+x];
    light = cave_light[y*MAX_WIDTH+x];
    if ((floor <= MAX_OPEN_SPACE) && (!flag)) {
      if (critter > 1) {
	flag = true;
	m_ptr = &m_list[critter];
	tbth = tbth - cur_dis;
	/* if monster not lit, make it much more difficult to
	   hit, subtract off most bonuses, and reduce bthb
	   depending on distance */
	if (!m_ptr->ml) {
	  tbth = tbth / (cur_dis+2);
	  tbth -= pymisc.lev * class_level_adj[pymisc.pclass][CLA_BTHB]/2;
	  tbth -= tpth * (BTH_PLUS_ADJ-1);
	}
	if (test_hit(tbth, (Short) pymisc.lev, tpth,
		     (Short) c_list[m_ptr->mptr].ac, CLA_BTHB)) {
	  i = m_ptr->mptr;
	  objdes(tmp_str, &throw_obj, false);
	  /* Does the player know what he's fighting?	   */
	  if (!m_ptr->ml) {
	    StrPrintF(out_val,
		      "You hear a cry as the %s finds a mark.", tmp_str);
	    visible = false;
	  } else {
	    StrPrintF(out_val, "The %s hits the %s.",
		      tmp_str, c_names + c_list[i].name_offset);
	    visible = true;
	  }
	  message(out_val);
	  tdam = tot_dam(&throw_obj, tdam, i);
	  tdam = critical_blow((Short) throw_obj.weight,
			       tpth, tdam, CLA_BTHB);
	  if (tdam < 0) tdam = 0;
	  i = mon_take_hit((Short) critter, tdam);
	  if (i >= 0) {
	    if (!visible)
	      message("You have killed something!");
	    else {
	      StrPrintF(out_val,"You have killed the %s.",
			c_names + c_list[i].name_offset);
	      message(out_val);
	    }
	    print_stats(STATS_QUO);//prt_experience();
	  }
	} else
	  drop_throw(oldy, oldx, &throw_obj);
      } else {	/* do not test c_ptr->fm here */
	if (panel_contains(y, x) && (pyflags.blind < 1)
	    && (light & (LIGHT_tl | LIGHT_pl))) {
	  print(tchar, y, x);
	  //	  put_qio(); /* show object moving */ // xxx not implemented yet
	}
      }
    } else {
      flag = true;
      drop_throw(oldy, oldx, &throw_obj);
    }
    oldy = y;
    oldx = x;
  }
  //  }
}


/* Make a bash attack on someone.				-CJS-
   Used to be part of bash above. */
static void py_bash(Short y, Short x)
{
  Short monster, k, avg_max_hp, base_tohit;
  creature_type *r_ptr;
  monster_type *m_ptr;
  Char m_name[80], out_val[80];
  UChar stuntime;

  monster = cave_cptr[y+MAX_WIDTH+x];
  m_ptr = &m_list[monster];
  r_ptr = &c_list[m_ptr->mptr];
  mlist_set_short(monster, MLIST_csleep, 0);
  /* Does the player know what he's fighting?	   */
  if (!m_ptr->ml)
    StrCopy(m_name, "it");
  else
    StrPrintF(m_name, "the %s", c_names + r_ptr->name_offset);
  base_tohit = pystats.use_stat[A_STR] + inventory[INVEN_ARM].weight/2
    + pymisc.wt/10;
  if (!m_ptr->ml) {
    base_tohit = base_tohit / 2;
    base_tohit -= pystats.use_stat[A_DEX]*(BTH_PLUS_ADJ-1);
    base_tohit -= pymisc.lev * class_level_adj[pymisc.pclass][CLA_BTH] / 2;
  }
  if (test_hit(base_tohit, (Short) pymisc.lev,
	       (Short) pystats.use_stat[A_DEX], (Short) r_ptr->ac, CLA_BTH)) {
    StrPrintF(out_val, "You hit %s.", m_name);
    message(out_val);
    k = pdamroll(inventory[INVEN_ARM].damage);
    k = critical_blow((Short) (inventory[INVEN_ARM].weight / 4
			       + pystats.use_stat[A_STR]), 0, k, CLA_BTH);
    k += pymisc.wt/60 + 3;
    if (k < 0) k = 0;

    /* See if we done it in.				     */
    if (mon_take_hit(monster, k) >= 0) {
      StrPrintF(out_val, "You have slain %s.", m_name);
      message(out_val);
      print_stats(STATS_QUO);//prt_experience();
    } else {
      // note to self - I'm not sure toupper exists in this universe.
      m_name[0] = toupper((Short) m_name[0]); /* Capitalize */
      /* Can not stun Balrog */
      avg_max_hp = (r_ptr->cdefense & CD_MAX_HP ?
		    r_ptr->hd[0] * r_ptr->hd[1] :
		    (r_ptr->hd[0] * (r_ptr->hd[1] + 1)) >> 1);
      if ((100 + randint(400) + randint(400)) > (m_ptr->hp + avg_max_hp)) {
	stuntime = m_ptr->stunned + randint(3) + 1;
	if (stuntime > 24) stuntime = 24;
	mlist_set_uchar(monster, MLIST_stunned, stuntime);
	StrPrintF(out_val, "%s appears stunned!", m_name);
      } else
	StrPrintF(out_val, "%s ignores your bash!", m_name);
      message(out_val);
    }
  } else {
    StrPrintF(out_val, "You miss %s.", m_name);
    message(out_val);
  }
  if (randint(150) > pystats.use_stat[A_DEX]) {
    message("You are off balance.");
    pyflags.paralysis = 1 + randint(2);
  }
}


/* Bash open a door or chest				-RAK-	*/
/* Note: Affected by strength and weight of character

   For a closed door, p1 is positive if locked; negative if
   stuck. A disarm spell unlocks and unjams doors!

   For an open door, p1 is positive for a broken door.

   A closed door can be opened - harder if locked. Any door might
   be bashed open (and thereby broken). Bashing a door is
   (potentially) faster! You move into the door way. To open a
   stuck door, it must be bashed. A closed door can be jammed
   (which makes it stuck if previously locked).

   Creatures can also open doors. A creature with open door
   ability will (if not in the line of sight) move though a
   closed or secret door with no changes. If in the line of
   sight, closed door are openned, & secret door revealed.
   Whether in the line of sight or not, such a creature may
   unlock or unstick a door.

   A creature with no such ability will attempt to bash a
   non-secret door. */
void bash(Short dir)
{
  Short y, x, tmp;
  inven_type *t_ptr;
  UChar floor, critter, thing;

  y = char_row;
  x = char_col;
  //  if (!get_dir(CNIL, &dir)) return; // xxx not implemented yet
  if (pyflags.confused > 0) {
    message("You are confused.");
    do {
      dir = randint(9);
    } while (dir == 5);
  }
  mmove(dir, &y, &x);
  //    c_ptr = &cave[y][x];
  floor = cave_fval[y*MAX_WIDTH+x];
  thing = cave_tptr[y*MAX_WIDTH+x];
  critter = cave_cptr[y*MAX_WIDTH+x];
  if (critter > 1) {
    if (pyflags.afraid > 0)
      message("You are afraid!");
    else
      py_bash(y, x);
  } else if (thing != 0) {
    t_ptr = &t_list[thing];
    if (t_ptr->tval == TV_CLOSED_DOOR) {
      count_message("You smash into the door!");
      tmp = pystats.use_stat[A_STR] + pymisc.wt / 2;
      /* Use (roughly) similar method as for monsters. */
      if (randint(tmp*(20+abs(t_ptr->p1))) < 10*(tmp-abs(t_ptr->p1))) {
	message("The door crashes open!");
	invcopy(t_list, thing, OBJ_OPEN_DOOR);
	invy_set_p1(t_list, thing, 1 - randint(2)); /* 50% chance of breaking door */
	cave_set(y, x, CAVE_fval, CORR_FLOOR);
	if (pyflags.confused == 0)
	  move_char(dir, false);
	else
	  lite_spot(y, x);
      } else if (randint(150) > pystats.use_stat[A_DEX]) {
	message("You are off-balance.");
	pyflags.paralysis = 1 + randint(2);
      } else if (command_count == 0)
	message("The door holds firm.");
    } else if (t_ptr->tval == TV_CHEST) {
      if (randint(10) == 1) {
	message("You have destroyed the chest.");
	message("and its contents!");
	invy_set_index(t_list, thing, OBJ_RUINED_CHEST);
	invy_set_flags(t_list, thing, 0);
      } else if ((CH_LOCKED & t_ptr->flags) && (randint(10) == 1)) {
	message("The lock breaks open!");
	invy_set_flags(t_list, thing, t_ptr->flags & ~CH_LOCKED);
      } else
	count_message("The chest holds firm.");
    } else {
      /* Can't give free turn, or else player could try directions
	 until he found invisible creature */
      message("You bash it, but nothing interesting happens.");
    }
  } else {
    if (floor < MIN_CAVE_WALL)
      message("You bash at empty space.");
    else
      /* same message for wall as for secret door */
      message("You bash it, but nothing interesting happens.");
  }
}
