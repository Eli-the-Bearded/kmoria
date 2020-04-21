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
static int see_wall(int, int, int);
static int see_nothing(int, int, int);
*/
static Boolean see_wall(Short dir, Short y, Short x) SEC_2;
static Boolean see_nothing(Short dir, Short y, Short x) SEC_2;



/* Change a trap from invisible to visible		-RAK-	*/
/* Note: Secret doors are handled here				 */
void change_trap(Short y, Short x)
{
  inven_type *t_ptr;
  Short offset;
  UChar new_tval, new_tchar, thing;
  UShort new_index;

  thing = cave_tptr[y*MAX_WIDTH+x];
  t_ptr = t_list + thing;
  // tval offset must skip: UShort, UChar, Char[13], ULong.
  //  offset = sizeof(UShort) + sizeof(UChar) +
  //    INSCRIP_SIZE*sizeof(Char) + sizeof(ULong);
  offset = I_tval;
  if (t_ptr->tval == TV_INVIS_TRAP) {
    // DANGER WILL ROBINSON
    new_tval = TV_VIS_TRAP;
    // use tval offset
    DmWrite(t_list, thing*sizeof(inven_type)+offset, &new_tval, sizeof(UChar));
    lite_spot(y, x);
  } else if (t_ptr->tval == TV_SECRET_DOOR) {
    /* change secret door to closed door */
    // index offset is 0
    new_index = OBJ_CLOSED_DOOR;
    DmWrite(t_list,thing*sizeof(inven_type)+offset, &new_index,sizeof(UShort));
    // use tval offset
    new_tval = object_list[OBJ_CLOSED_DOOR].tval;
    DmWrite(t_list, thing*sizeof(inven_type)+offset, &new_tval, sizeof(UChar));
    // tchar offset must skip: UShort, UChar, Char[13], ULong, PLUS UChar
    offset += sizeof(UChar);
    new_tchar = object_list[OBJ_CLOSED_DOOR].tchar;
    DmWrite(t_list,thing*sizeof(inven_type)+offset, &new_tchar, sizeof(UChar));
    lite_spot(y, x);
  }
}


/* Searches for hidden things.			-RAK-	*/
void search(Short y, Short x, Short chance)
{
  Short i, j;
  inven_type *t_ptr;
  Short c_ptr;
  Char tmp_str[160], tmp_str2[160];

  if (pyflags.confused > 0)
    chance = chance / 10;
  if ((pyflags.blind > 0) || no_light())
    chance = chance / 10;
  if (pyflags.image > 0)
    chance = chance / 10;

  for (i = (y - 1); i <= (y + 1); i++) {
    for (j = (x - 1); j <= (x + 1); j++) {
      if (randint(100) < chance) { /* always in_bounds here */
	c_ptr = i*MAX_WIDTH+j;
	/* Search for hidden objects		   */
	if (cave_tptr[c_ptr] != 0) {
	  t_ptr = t_list + cave_tptr[c_ptr];
	  /* Trap on floor?		       */
	  if (t_ptr->tval == TV_INVIS_TRAP) {
	    objdes(tmp_str2, t_ptr, true);
	    StrPrintF(tmp_str, "You have found %s", tmp_str2);
	    message(tmp_str);
	    change_trap(i, j);
	    end_find();
	  }
	  /* Secret door?		       */
	  else if (t_ptr->tval == TV_SECRET_DOOR) {
	    message("You have found a secret door.");
	    change_trap(i, j);
	    end_find();
	  }
	  /* Chest is trapped?	       */
	  else if (t_ptr->tval == TV_CHEST) {
	    /* mask out the treasure bits */
	    if ((t_ptr->flags & CH_TRAPPED) > 1) {
	      if (!known2_p(t_ptr)) {
		known2(t_list, cave_tptr[c_ptr]);
		message("You have discovered a trap on the chest!");
	      }
	      else
		message("The chest is trapped!");
	    }
	  }
	}
      }
    }
  }
}


/* The running algorithm:			-CJS-

   Overview: You keep moving until something interesting happens.
   If you are in an enclosed space, you follow corners. This is
   the usual corridor scheme. If you are in an open space, you go
   straight, but stop before entering enclosed space. This is
   analogous to reaching doorways. If you have enclosed space on
   one side only (that is, running along side a wall) stop if
   your wall opens out, or your open space closes in. Either case
   corresponds to a doorway.

   What happens depends on what you can really SEE. (i.e. if you
   have no light, then running along a dark corridor is JUST like
   running in a dark room.) The algorithm works equally well in
   corridors, rooms, mine tailings, earthquake rubble, etc, etc.

   These conditions are kept in static memory:
	find_openarea	 You are in the open on at least one
			 side.
	find_breakleft	 You have a wall on the left, and will
			 stop if it opens
	find_breakright	 You have a wall on the right, and will
			 stop if it opens

   To initialize these conditions is the task of find_init. If
   moving from the square marked @ to the square marked . (in the
   two diagrams below), then two adjacent sqares on the left and
   the right (L and R) are considered. If either one is seen to
   be closed, then that side is considered to be closed. If both
   sides are closed, then it is an enclosed (corridor) run.

	 LL		L
	@.	       L.R
	 RR	       @R

   Looking at more than just the immediate squares is
   significant. Consider the following case. A run along the
   corridor will stop just before entering the center point,
   because a choice is clearly established. Running in any of
   three available directions will be defined as a corridor run.
   Note that a minor hack is inserted to make the angled corridor
   entry (with one side blocked near and the other side blocked
   further away from the runner) work correctly. The runner moves
   diagonally, but then saves the previous direction as being
   straight into the gap. Otherwise, the tail end of the other
   entry would be perceived as an alternative on the next move.

	   #.#
	  ##.##
	  .@...
	  ##.##
	   #.#

   Likewise, a run along a wall, and then into a doorway (two
   runs) will work correctly. A single run rightwards from @ will
   stop at 1. Another run right and down will enter the corridor
   and make the corner, stopping at the 2.

	#@	  1
	########### ######
	2	    #
	#############
	#

   After any move, the function area_affect is called to
   determine the new surroundings, and the direction of
   subsequent moves. It takes a location (at which the runner has
   just arrived) and the previous direction (from which the
   runner is considered to have come). Moving one square in some
   direction places you adjacent to three or five new squares
   (for straight and diagonal moves) to which you were not
   previously adjacent.

       ...!	  ...	       EG Moving from 1 to 2.
       .12!	  .1.!		  . means previously adjacent
       ...!	  ..2!		  ! means newly adjacent
		   !!!

   You STOP if you can't even make the move in the chosen
   direction. You STOP if any of the new squares are interesting
   in any way: usually containing monsters or treasure. You STOP
   if any of the newly adjacent squares seem to be open, and you
   are also looking for a break on that side. (i.e. find_openarea
   AND find_break) You STOP if any of the newly adjacent squares
   do NOT seem to be open and you are in an open area, and that
   side was previously entirely open.

   Corners: If you are not in the open (i.e. you are in a
   corridor) and there is only one way to go in the new squares,
   then turn in that direction. If there are more than two new
   ways to go, STOP. If there are two ways to go, and those ways
   are separated by a square which does not seem to be open, then
   STOP.

   Otherwise, we have a potential corner. There are two new open
   squares, which are also adjacent. One of the new squares is
   diagonally located, the other is straight on (as in the
   diagram). We consider two more squares further out (marked
   below as ?).
	  .X
	 @.?
	  #?
   If they are both seen to be closed, then it is seen that no
   benefit is gained from moving straight. It is a known corner.
   To cut the corner, go diagonally, otherwise go straight, but
   pretend you stepped diagonally into that next location for a
   full view next time. Conversely, if one of the ? squares is
   not seen to be closed, then there is a potential choice. We check
   to see whether it is a potential corner or an intersection/room entrance.
   If the square two spaces straight ahead, and the space marked with 'X'
   are both blank, then it is a potential corner and enter if find_examine
   is set, otherwise must stop because it is not a corner. */

/* The cycle lists the directions in anticlockwise order, for	-CJS-
   over two complete cycles. The chome array maps a direction on
   to its position in the cycle.
*/
static Char cycle[] = { 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };
static Char chome[] = { -1, 8, 9, 10, 7, -1, 11, 6, 5, 4 };
static Char find_openarea, find_breakright, find_breakleft, find_prevdir;
static Char find_direction; /* Keep a record of which way we are going. */

// Hm, I wonder if this will work.  Let's try it shall we.  (Oct 27)
void find_init(Short dir)
{
  Short row, col, deepleft, deepright;
  Short i, shortleft, shortright;

  row = char_row;
  col = char_col;
  if (!mmove(dir, &row, &col))
    find_flag = false;
  else {
    find_direction = dir;
    find_flag = true;
    find_breakright = find_breakleft = false;
    find_prevdir = dir;
    if (pyflags.blind < 1) {
      i = chome[dir];
      deepleft = deepright = false;
      shortright = shortleft = false;
      if (see_wall(cycle[i+1], char_row, char_col)) {
	find_breakleft = true;
	shortleft = true;
      } else if (see_wall(cycle[i+1], row, col)) {
	find_breakleft = true;
	deepleft = true;
      }
      if (see_wall(cycle[i-1], char_row, char_col)) {
	find_breakright = true;
	shortright = true;
      } else if (see_wall(cycle[i-1], row, col)) {
	find_breakright = true;
	deepright = true;
      }
      if (find_breakleft && find_breakright) {
	find_openarea = false;
	if (dir & 1) {		/* a hack to allow angled corridor entry */
	  if (deepleft && !deepright)
	    find_prevdir = cycle[i-1];
	  else if (deepright && !deepleft)
	    find_prevdir = cycle[i+1];
	}
	/* else if there is a wall two spaces ahead and seem to be in a
	   corridor, then force a turn into the side corridor, must
	   be moving straight into a corridor here */
	else if (see_wall(cycle[i], row, col)) {
	  if (shortleft && !shortright)
	    find_prevdir = cycle[i-2];
	  else if (shortright && !shortleft)
	    find_prevdir = cycle[i+2];
	}
      } else // find_breakleft and find_breakright aren't both true
	find_openarea = true;
    }
  }

  /* We must erase the player symbol '@' here, because sub3_move_light()
     does not erase the previous location of the player when in find mode
     and when find_prself is false.  The player symbol is not drawn at all
     in this case while moving, so the only problem is on the first turn
     of find mode, when the initial position of the character must be erased.
     Hence we must do the erasure here.  */
  // so, if !find_prself, it does not seem like this does the right thing
  // so I have initialized find_prself to true.  so thre.
  if (! light_flag && ! find_prself)
    print(loc_symbol(char_row, char_col), char_row, char_col);

  move_char(dir, true);
  if (find_flag == false)
    command_count = 0;
}

void old_find_run()
{
  /* prevent infinite loops in find mode, will stop after moving 100 times */
  if (find_flag++ > 100) {
    message("You stop running to catch your breath.");
    end_find();
  } else
    move_char(find_direction, true);
}

void find_run()
{
  /* prevent infinite loops in find mode, will stop after moving 100 times */
  while (find_flag && find_flag < 100) {
    find_flag++;
    end_of_turn(); // (find_init already moved once so we start with end_of)
    start_of_turn();
    move_char(find_direction, true);
  }
  if (find_flag)
    message("You stop running to catch your breath.");
  end_find();
  // it might be a good idea to refresh?
}

/* Switch off the run flag - and get the light correct. -CJS- */
void end_find()
{
  if (find_flag) {
    find_flag = false;
    move_light(char_row, char_col, char_row, char_col);
  }
}

/* Do we see a wall? Used in running.		-CJS- */
static Boolean see_wall(Short dir, Short y, Short x)
{
  Char c;

  if (!mmove(dir, &y, &x))	/* check to see if movement there possible */
    return true;
  else if ((c = loc_symbol(y, x)) == '#' || c == '%')
    return true;
  else
    return false;
}

/* Do we see anything? Used in running.		-CJS- */
static Boolean see_nothing(Short dir, Short y, Short x)
{
  if (!mmove(dir, &y, &x))	/* check to see if movement there possible */
    return false;
  else if (loc_symbol(y, x) == ' ')
    return true;
  else
    return false;
}


/* Determine the next direction for a run, or if we should stop.  -CJS- */
void area_affect(Short dir, Short y, Short x)
{
  Short newdir, t, inv, check_dir=0, row, col;
  Short i, max, option, option2;
  Short c_ptr;
  UChar light, thing, critter, floor;

  if (pyflags.blind < 1) {
    option = 0;
    option2 = 0;
    dir = find_prevdir;
    max = (dir & 1) + 1;
    /* Look at every newly adjacent square. */
    for (i = -max; i <= max; i++) {
      newdir = cycle[chome[dir]+i];
      row = y;
      col = x;
      if (mmove(newdir, &row, &col)){
	/* Objects player can see (Including doors?) cause a stop. */
	//	c_ptr = &cave[row][col];
	c_ptr = row*MAX_WIDTH+col;
	light = cave_light[c_ptr];
	if (player_light || (light & (LIGHT_tl | LIGHT_pl | LIGHT_fm))) {
	  thing = cave_tptr[c_ptr];
	  critter = cave_cptr[c_ptr];
	  if (thing != 0) {
	    t = t_list[thing].tval;
	    if (t != TV_INVIS_TRAP && t != TV_SECRET_DOOR
		&& (t != TV_OPEN_DOOR || !find_ignore_doors)) {
	      end_find();
	      return;
	    }
	  }
	  /* Also Creatures		*/
	  /* the monster should be visible since update_mon() checks
	     for the special case of being in find mode */
	  if (critter > 1 && m_list[critter].ml) {
	    end_find();
	    return;
	  }
	  inv = false;
	} else
	  inv = true;	/* Square unseen. Treat as open. */

	floor = cave_fval[c_ptr];
	if (floor <= MAX_OPEN_SPACE || inv) {
	  if (find_openarea) {
	    /* Have we found a break? */
	    if (i < 0) {
	      if (find_breakright) {
		end_find();
		return;
	      }
	    } else if (i > 0) {
	      if (find_breakleft) {
		end_find();
		return;
	      }
	    } // else i==0.
	  } else if (option == 0)
	    option = newdir;	/* The first new direction. */
	  else if (option2 != 0) {
	    end_find();	/* Three new directions. STOP. */
	    return;
	  } else if (option != cycle[chome[dir]+i-1]) {
	    end_find();	/* If not adjacent to prev, STOP */
	    return;
	  } else {
	    /* Two adjacent choices. Make option2 the diagonal,
	       and remember the other diagonal adjacent to the first
	       option. */
	    if ((newdir & 1) == 1) {
	      check_dir = cycle[chome[dir]+i-2];
	      option2 = newdir;
	    } else {
	      check_dir = cycle[chome[dir]+i+1];
	      option2 = option;
	      option = newdir;
	    }
	  }
	} else if (find_openarea) {
	  /* We see an obstacle. In open area, STOP if on a side
	     previously open. */
	  if (i < 0) {
	    if (find_breakleft) {
	      end_find();
	      return;
	    }
	    find_breakright = true;
	  } else if (i > 0) {
	    if (find_breakright) {
	      end_find();
	      return;
	    }
	    find_breakleft = true;
	  }
	}
      }
    }

    if (find_openarea == false) {	/* choose a direction. */
      if (option2 == 0 || (find_examine && !find_cut)) {
	/* There is only one option, or if two, then we always examine
	   potential corners and never cur known corners, so you step
	   into the straight option. */
	if (option != 0)
	  find_direction = option;
	if (option2 == 0)
	  find_prevdir = option;
	else
	  find_prevdir = option2;
      } else {
	/* Two options! */
	row = y;
	col = x;
	(void) mmove(option, &row, &col);
	if (!see_wall(option, row, col)
	    || !see_wall(check_dir, row, col)) {
	  /* Don't see that it is closed off.  This could be a
	     potential corner or an intersection. */
	  if (find_examine && see_nothing(option, row, col)
	      && see_nothing(option2, row, col)) {
	    /* Can not see anything ahead and in the direction we are
	       turning, assume that it is a potential corner. */
	    find_direction = option;
	    find_prevdir = option2;
	  } else
	    /* STOP: we are next to an intersection or a room */
	    end_find();
	} else if (find_cut) {
	  /* This corner is seen to be enclosed; we cut the corner. */
	  find_direction = option2;
	  find_prevdir = option2;
	} else {
	  /* This corner is seen to be enclosed, and we deliberately
	     go the long way. */
	  find_direction = option;
	  find_prevdir = option2;
	}
      }
    }
  }
}


/* AC gets worse					-RAK-	*/
/* Note: This routine affects magical AC bonuses so that stores	  */
/*	 can detect the damage.					 */
Boolean minus_ac(ULong typ_dam)
{
  Short i, j;
  Short tmp[6], minus;
  inven_type *i_ptr;
  Char out_val[160], tmp_str[160];

  i = 0;
  if (inventory[INVEN_BODY].tval != TV_NOTHING) {
    tmp[i] = INVEN_BODY;
    i++;
  }
  if (inventory[INVEN_ARM].tval != TV_NOTHING) {
    tmp[i] = INVEN_ARM;
    i++;
  }
  if (inventory[INVEN_OUTER].tval != TV_NOTHING) {
    tmp[i] = INVEN_OUTER;
    i++;
  }
  if (inventory[INVEN_HANDS].tval != TV_NOTHING) {
    tmp[i] = INVEN_HANDS;
    i++;
  }
  if (inventory[INVEN_HEAD].tval != TV_NOTHING) {
    tmp[i] = INVEN_HEAD;
    i++;
  }
  /* also affect boots */
  if (inventory[INVEN_FEET].tval != TV_NOTHING) {
    tmp[i] = INVEN_FEET;
    i++;
  }
  minus = false;
  if (i > 0) {
    j = tmp[randint(i) - 1];
    i_ptr = &inventory[j];
    if (i_ptr->flags & typ_dam) {
      objdes(tmp_str, &inventory[j], false);
      StrPrintF(out_val, "Your %s resists damage!", tmp_str);
      message(out_val);
      minus = true;
    } else if ((i_ptr->ac+i_ptr->toac) > 0) {
      objdes(tmp_str, &inventory[j], false);
      StrPrintF(out_val, "Your %s is damaged!", tmp_str);
      message(out_val);
      i_ptr->toac--;
      calc_bonuses();
      minus = true;
    }
  }
  return minus;
}


/* Corrode the unsuspecting person's armor		 -RAK-	 */
void corrode_gas(Char *kb_str)
{
  if (!minus_ac((ULong) TR_RES_ACID))
    take_hit(randint(8), kb_str);
  if (inven_damage(is_corrodes, 5) > 0)
    message("There is an acrid smell coming from your pack.");
}


/* Poison gas the idiot.				-RAK-	*/
void poison_gas(Short dam, Char *kb_str)
{
  take_hit(dam, kb_str);
  pyflags.poisoned += 12 + randint(dam);
}


/* Burn the fool up.					-RAK-	*/
void fire_dam(Short dam, Char *kb_str)
{
  if (pyflags.fire_resist)
    dam = dam / 3;
  if (pyflags.resist_heat > 0)
    dam = dam / 3;
  take_hit(dam, kb_str);
  if (inven_damage(is_flammable, 3) > 0)
    message("There is smoke coming from your pack!");
}


/* Freeze him to death.				-RAK-	*/
void cold_dam(Short dam, Char *kb_str)
{
  if (pyflags.cold_resist)
    dam = dam / 3;
  if (pyflags.resist_cold > 0)
    dam = dam / 3;
  take_hit(dam, kb_str);
  if (inven_damage(is_frost_destroy, 5) > 0)
    message("Something shatters inside your pack!");
}


/* Lightning bolt the sucker away.			-RAK-	*/
void light_dam(Short dam, Char *kb_str)
{
  if (pyflags.lght_resist)
    take_hit((dam / 3), kb_str);
  else
    take_hit(dam, kb_str);
  if (inven_damage(is_lightning_destroy, 3) > 0)
    message("There are sparks coming from your pack!");
}


/* Throw acid on the hapless victim			-RAK-	*/
void acid_dam(Short dam, Char *kb_str)
{
  Short flag;

  flag = 0;
  if (minus_ac((ULong) TR_RES_ACID))
    flag = 1;
  if (pyflags.acid_resist)
    flag += 2;
  take_hit (dam / (flag + 1), kb_str);
  if (inven_damage(is_acid_affect, 3) > 0)
    message("There is an acrid smell coming from your pack!");
}
