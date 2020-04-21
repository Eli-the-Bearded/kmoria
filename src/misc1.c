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

// Things I need:
// a real randnor
// compact_monsters
// compact_objects
//Boolean total_winner; // should be an extern and live with the others.

static void compact_objects() SEC_1;

ULong old_seed;
void seed_random()
{
  old_seed = TimGetSeconds();
  SysRandom(old_seed);
}

void set_seed(ULong seed)
{
  SysRandom(seed); // sneaky; this allows reproducible behavior
}
void reset_seed()
{
  //  SysRandom(old_seed);  
  seed_random();
}

/* Check the day-time strings to see if open		-RAK-	*/
/*
Boolean check_time()
{
  return true;
}
*/

/***************************************************************
                   RANDINT
 IN:
 N = upper bound (inclusive)
 OUT:
 a random number between 1 and N inclusive: [1,N]
 PURPOSE:
 duh
****************************************************************/
Int randint(Int y)
{
  Int r;
  if (y <= 1) return 1;
  r = SysRandom(0);
  r = (r % y) + 1; // 0 to y-1... add 1... 1 to y.
  return r;
}
/***************************************************************
                   RANDINT
 IN:
 N = upper bound (exclusive)
 OUT:
 a random number between 0 and N-1 inclusive: [0,N)
 PURPOSE:
 duh
****************************************************************/
Int rund(Int y)
{
  Int r;
  if (y <= 1) return 0;
  r = SysRandom(0);
  r = r % y; // 0 to y-1...
  return r;
}


/* Generates a random integer number of NORMAL distribution -RAK-*/
// NOTE TO SELF - fix this later.
Short randnor(Short mean, Short stand)
{
  Short tmp;
  //  Short offset, low, iindex, high;

  //#if 0
  /* alternate randnor code, slower but much smaller since no table */
  /* 2 per 1,000,000 will be > 4*SD, max is 5*SD */
  tmp = damroll(8, 99);	 /* mean 400, SD 81 */
  tmp = (tmp - 400) * stand / 81;
  return tmp + mean;
  //#endif
}



/* Returns position of first set bit			-RAK-	*/
/*     and clears that bit */
// hey, don't call this on a bitfield stored in database record, ok?
Short bit_pos(ULong *test)
{
  Short i;
  ULong mask = 0x1;

  for (i = 0; i < sizeof(*test)*8; i++) {
    if (*test & mask) {
      *test &= ~mask;
      return(i);
    }
    mask <<= 1;
  }
  /* all bits are 0 */
  return(-1);
}


/* Checks a co-ordinate for in bounds status		-RAK-	*/
// This is temporarily in generate.c
Boolean in_bounds(Short y, Short x)
{
  return (y > 0 && y < cur_height-1 && x > 0 && x < cur_width-1);
}

/* Calculates current boundaries				-RAK-	*/
// This is display-related and should probably be in display.c
// So should get_ panel!

void panel_bounds()
{
  panel_row_min = panel_row*(SCREEN_HEIGHT/2);
  panel_row_max = panel_row_min + SCREEN_HEIGHT - 1;
  //  panel_row_prt = panel_row_min - 1; // should not be used
  panel_col_min = panel_col*(SCREEN_WIDTH_REAL/2);
  panel_col_max = panel_col_min + SCREEN_WIDTH_REAL - 1;
  //  panel_col_prt = panel_col_min - 13; // should not be used
}


/* Given an row (y) and col (x), this routine detects  -RAK-	*/
/* when a move off the screen has occurred and figures new borders.
   Force forcses the panel bounds to be recalculated, useful for 'W'here. */
// This should be in display.c

/*
Boolean old_get_panel(Short y, Short x, Boolean force)
{
  Short prow, pcol;
  Boolean panel;

  prow = panel_row;
  pcol = panel_col;
  if (force || (y < panel_row_min + 2) || (y > panel_row_max - 2)) {
    prow = ((y - SCREEN_HEIGHT/4)/(SCREEN_HEIGHT/2));
    if (prow > max_panel_rows)
      prow = max_panel_rows;
    else if (prow < 0)
      prow = 0;
  }
  if (force || (x < panel_col_min + 3) || (x > panel_col_max - 3)) {
    pcol = ((x - SCREEN_WIDTH_REAL/4)/(SCREEN_WIDTH_REAL/2));
    if (pcol > max_panel_cols)
      pcol = max_panel_cols;
    else if (pcol < 0)
      pcol = 0;
  }
  if ((prow != panel_row) || (pcol != panel_col)) {
    panel_row = prow;
    panel_col = pcol;
    panel_bounds();
    panel = true;
    // stop movement if any
    if (find_bound)
      end_find(); // turn off run flag, and correct the light
    change_visible_xy(panel_row_min, panel_col_min); // XXX will this DTRT?
  }
  else
    panel = false;
  return(panel);
}
*/

/* Tests a given point to see if it is within the screen -RAK-	*/
/* boundaries.							  */
// This should be in display.c

Boolean panel_contains(Short y, Short x)
{
  if ((y >= panel_row_min) && (y <= panel_row_max) &&
      (x >= panel_col_min) && (x <= panel_col_max))
    return true;
  else
    return false;
}


/* Distance between two points				-RAK-	*/
Short distance(Short y1, Short x1, Short y2, Short x2)
{
  Short dy, dx;

  dy = y1 - y2;
  if (dy < 0)   dy = -dy;
  dx = x1 - x2;
  if (dx < 0)   dx = -dx;

  return ((((dy + dx) << 1) - (dy > dx ? dx : dy)) >> 1);
}

// next_to_walls is used only in generate.c and has been moved there.
// next_to_corr  is used only in generate.c and has been moved there.


/* generates damage for 2d6 style dice rolls */
Short damroll(Short num, Short sides)
{
  Short i, sum = 0;

  for (i = 0; i < num; i++)
    sum += randint(sides);
  return sum;
}

Short pdamroll(UChar *array)
{
  return damroll((Short)array[0], (Short)array[1]);
}

/* A simple, fast, integer-based line-of-sight algorithm.  By Joseph Hall,
   4116 Brewster Drive, Raleigh NC 27606.  Email to jnh@ecemwl.ncsu.edu.

   Returns TRUE if a line of sight can be traced from x0, y0 to x1, y1.

   The LOS begins at the center of the tile [x0, y0] and ends at
   the center of the tile [x1, y1].  If los() is to return TRUE, all of
   the tiles this line passes through must be transparent, WITH THE
   EXCEPTIONS of the starting and ending tiles.

   We don't consider the line to be "passing through" a tile if
   it only passes across one corner of that tile. */

/* Because this function uses (short) ints for all calculations, overflow
   may occur if deltaX and deltaY exceed 90. */

Boolean los(Short fromY, Short fromX, Short toY, Short toX)
{
  Short tmp, deltaX, deltaY;
  Short p_y, px; 	/* x or y position -- loop variable	*/

  deltaX = toX - fromX;
  deltaY = toY - fromY;

  /* Adjacent? */
  if ((deltaX < 2) && (deltaX > -2) && (deltaY < 2) && (deltaY > -2))
    return true;

  /* Handle the cases where deltaX or deltaY == 0. */
  if (deltaX == 0) {
    if (deltaY < 0) {
      tmp = fromY; fromY = toY; toY = tmp;
    }
    for (p_y = fromY + 1; p_y < toY; p_y++)
      if (cave_fval[p_y*MAX_WIDTH+fromX] >= MIN_CLOSED_SPACE)
	return false;
    return true;
  }

  if (deltaY == 0) {
    if (deltaX < 0) {
      tmp = fromX; fromX = toX; toX = tmp;
    }
    for (px = fromX + 1; px < toX; px++)
      if (cave_fval[fromY*MAX_WIDTH+px] >= MIN_CLOSED_SPACE)
	return false;
    return true;
  }

  /* Now, we've eliminated all the degenerate cases.
     In the computations below, dy (or dx) and m are multiplied by a
     scale factor, scale = abs(deltaX * deltaY * 2), so that we can use
     integer arithmetic. */

  {
    Short scale2;	/* above scale factor / 2		*/
    Short scale,	/* above scale factor			*/
      xSign,		/* sign of deltaX			*/
      ySign,		/* sign of deltaY			*/
      m;			/* slope or 1/slope of LOS		*/

    scale2 = abs(deltaX * deltaY);
    scale = scale2 << 1;
    xSign = (deltaX < 0) ? -1 : 1;
    ySign = (deltaY < 0) ? -1 : 1;

    /* Travel from one end of the line to the other, oriented along
       the longer axis. */

    if (abs(deltaX) >= abs(deltaY)) {
      Short dy;		/* "fractional" y position	*/
      /* We start at the border between the first and second tiles,
	 where the y offset = .5 * slope.  Remember the scale
	 factor.  We have:

	 m = deltaY / deltaX * 2 * (deltaY * deltaX)
	 = 2 * deltaY * deltaY. */

      dy = deltaY * deltaY;
      m = dy << 1;
      px = fromX + xSign;

      /* Consider the special case where slope == 1. */
      if (dy == scale2) {
	p_y = fromY + ySign;
	dy -= scale;
      }
      else
	p_y = fromY;

      while (toX - px) {
	if (cave_fval[p_y*MAX_WIDTH+px] >= MIN_CLOSED_SPACE)
	  return false;

	dy += m;
	if (dy < scale2)
	  px += xSign;
	else if (dy > scale2) {
	  p_y += ySign;
	  if (cave_fval[p_y*MAX_WIDTH+px] >= MIN_CLOSED_SPACE)
	    return false;
	  px += xSign;
	  dy -= scale;
	} else {
	  /* This is the case, dy == scale2, where the LOS
	     exactly meets the corner of a tile. */
	  px += xSign;
	  p_y += ySign;
	  dy -= scale;
	}
      }
      return true;
    } else {
      Short dx;		/* "fractional" x position	*/
      dx = deltaX * deltaX;
      m = dx << 1;

      p_y = fromY + ySign;
      if (dx == scale2) {
	px = fromX + xSign;
	dx -= scale;
      }
      else
	px = fromX;

      while (toY - p_y) {
	if (cave_fval[p_y*MAX_WIDTH+px] >= MIN_CLOSED_SPACE)
	  return false;
	dx += m;
	if (dx < scale2)
	  p_y += ySign;
	else if (dx > scale2) {
	  px += xSign;
	  if (cave_fval[p_y*MAX_WIDTH+px] >= MIN_CLOSED_SPACE)
	    return false;
	  p_y += ySign;
	  dx -= scale;
	} else {
	  px += xSign;
	  p_y += ySign;
	  dx -= scale;
	}
      }
      return true;
    }
  }
}



//#define WIZARD 1
/* Returns symbol for given row, column			-RAK-	*/
UChar loc_symbol(Short y, Short x)
{
  UChar critter, thing, light, floor;
  Short c_ptr = y * MAX_WIDTH + x;

  //  if (cave_cptr[c_ptr] == 1) // and if we want to show self.
  if ((cave_cptr[c_ptr] == 1))
    //  if ((cave_cptr[c_ptr] == 1) && (!find_flag || find_prself))
    return '@';
  // if blind, return ' '
  if (pyflags.status & PY_BLIND)
    return ' ';
  // beats me
  if ((pyflags.image > 0) && (randint(12) == 1))
    return randint(95) + 31;

  critter = cave_cptr[c_ptr];
  if ((critter > 1) && (m_list[critter].ml)) // monster exists AND is lighted
    return c_list[m_list[critter].mptr].cchar; // look up its representation

  light = cave_light[c_ptr];
  //  if (!WIZARD && !(light & (LIGHT_pl | LIGHT_tl | LIGHT_fm)))
  if ( !(light & (LIGHT_pl | LIGHT_tl | LIGHT_fm)) )
    return ' '; // no light
  
  thing = cave_tptr[c_ptr];
  if ((thing != 0) && (t_list[thing].tval != TV_INVIS_TRAP))
    return t_list[thing].tchar;
  
  floor = cave_fval[c_ptr];
  if (floor <= MAX_CAVE_FLOOR)
    return '.'; // lighted open floor
  if (floor == GRANITE_WALL || floor == BOUNDARY_WALL
      || highlight_seams == false)
    return '#';
  else return '%'; // for highlighting interesting mineral seams

}

/* Tests a spot for light or field mark status		-RAK-	*/
Boolean test_light(Short y, Short x)
{
  UChar light;

  light = cave_light[y*MAX_WIDTH+x];
  if (light & (LIGHT_pl | LIGHT_tl | LIGHT_fm))
    return true;
  return false;
}

/* Prints the map of the dungeon			-RAK-	*/
// What it should do is to clear the screen, then redraw the dungeon.
// Something like this already exists in display.c, so use that instead
/*
void prt_map()
{
}
*/

/* Compact monsters					-RAK-	*/
/* Return TRUE if any monsters were deleted, FALSE if could not delete any
   monsters.  */
// You know this otter be in lock.c
Boolean compact_monsters()
{
  // used in misc1.c and dungeon.c, or indirectly by anything that calls popm!
  return false;
}


/* Add to the players food time				-RAK-	*/
// This must change if player_type is stored in the database!!
void add_food(Short num)
{
  Short extra, penalty;
  pyflags.food = max(pyflags.food, 0);
  pyflags.food += num;
  if (pyflags.food > PLAYER_FOOD_MAX) {
      message("You are bloated from overeating.");
      /* Calculate how much of num is responsible for the bloating.
	 Give the player food credit for 1/50, and slow him for that many
	 turns also.  */
      extra = pyflags.food - PLAYER_FOOD_MAX;
      extra = min(extra, num);
      penalty = extra / 50; // Maybe should add something to extra, to round up

      pyflags.slow += penalty;
      if (extra == num)
	pyflags.food = pyflags.food - num + penalty;
      else
	pyflags.food = PLAYER_FOOD_MAX + penalty;
    }
  else if (pyflags.food > PLAYER_FOOD_FULL)
    message("You are full.");
}


/* Returns a pointer to next free space			-RAK-	*/
/* Returns -1 if could not allocate a monster.  */
Short popm()
{
  if (mfptr == MAX_MALLOC)
    tcptr = 0;  //    if (! compact_monsters()) return -1;
  return (mfptr++);
}

/* Gives Max hit points					-RAK-	*/
Short max_hp(UChar *array)
{
  return(array[0] * array[1]);
}

/* Places a monster at given location			-RAK-	*/
monster_type straw_monster = {0,0,0,0,0,0,0,false,0,0};
Boolean place_monster(Short y, Short x, Short z, Boolean slp)
{
  Short cur_pos;
  monster_type *mon_ptr = &straw_monster;

  cur_pos = popm();
  if (cur_pos == -1)
    return false;
  //  mon_ptr = m_list + cur_pos;
  mon_ptr->fy = y;
  mon_ptr->fx = x;
  mon_ptr->mptr = z;
  if (c_list[z].cdefense & CD_MAX_HP)
    mon_ptr->hp = max_hp(c_list[z].hd);
  else
    mon_ptr->hp = pdamroll(c_list[z].hd);
  /* the c_list speed value is 10 greater, so that it can be a int8u */
  mon_ptr->cspeed = c_list[z].speed - 10 + pyflags.speed;
  mon_ptr->stunned = 0;
  mon_ptr->cdis = distance(char_row, char_col,y,x);
  mon_ptr->ml = false;
  cave_set(y, x, CAVE_cptr, cur_pos);
  if (slp) {
    if (c_list[z].sleep == 0)
      mon_ptr->csleep = 0;
    else
      mon_ptr->csleep = c_list[z].sleep * 2 + randint((Int)c_list[z].sleep*10);
  } else
    mon_ptr->csleep = 0;
  // now copy it.  I hope this works.
  DmWrite(m_list, cur_pos*sizeof(monster_type), mon_ptr, sizeof(monster_type));
  return true;
}


/* Places a monster at given location			-RAK-	*/
void place_win_monster()
{
  Short y, x, cur_pos, c_ptr;
  monster_type *mon_ptr = &straw_monster;

  if (!total_winner) {
    cur_pos = popm();
    /* Check for case where could not allocate space for the win monster,
       this should never happen.  */
    if (cur_pos == -1)
      return;      //      abort(); // Oh please.  I don't have SIGABORT
    //    mon_ptr = &m_list[cur_pos];
    do {
      y = randint(cur_height-2);
      x = randint(cur_width-2);
      c_ptr = y*MAX_WIDTH+x;
    } while (cave_fval[c_ptr] >= MIN_CLOSED_SPACE
	     || (cave_cptr[c_ptr] != 0)
	     || (cave_tptr[c_ptr] != 0)
	     || (distance(y,x,char_row, char_col) <= MAX_SIGHT));
    mon_ptr->fy = y;
    mon_ptr->fx = x;
    mon_ptr->mptr = rund(WIN_MON_TOT) + m_level[MAX_MONS_LEVEL];
    if (c_list[mon_ptr->mptr].cdefense & CD_MAX_HP)
      mon_ptr->hp = max_hp(c_list[mon_ptr->mptr].hd);
    else
      mon_ptr->hp = pdamroll(c_list[mon_ptr->mptr].hd);
    /* the c_list speed value is 10 greater, so that it can be a int8u */
    mon_ptr->cspeed = c_list[mon_ptr->mptr].speed - 10 + pyflags.speed;
    mon_ptr->stunned = 0;
    mon_ptr->cdis = distance(char_row, char_col,y,x);
    cave_set(y, x, CAVE_cptr, cur_pos);
    mon_ptr->csleep = 0;
    // now copy it.  I hope this works.
    DmWrite(m_list, cur_pos*sizeof(monster_type),mon_ptr,sizeof(monster_type));
  } // else total_winner
}



/* Return a monster suitable to be placed at a given level.  This makes
   high level monsters (up to the given level) slightly more common than
   low level monsters at any given level.   -CJS- */
Short get_mons_num(Short level)
{
  Short i, j, num;

  if (level <= 0)
    i = rund(m_level[0]);
  else {
    level = min(level, MAX_MONS_LEVEL);
    if (randint (MON_NASTY) == 1) {
      i = randnor (0, 4);
      level = level + abs(i) + 1;
      level = min(level, MAX_MONS_LEVEL);
    } else {
      /* This code has been added to make it slightly more likely to
	 get the higher level monsters. Originally a uniform
	 distribution over all monsters of level less than or equal to the
	 dungeon level. This distribution makes a level n monster occur
	 approx 2/n% of the time on level n, and 1/n*n% are 1st level. */
      num = m_level[level] - m_level[0];
      i = rund (num);
      j = rund (num);
      if (j > i)
	i = j;
      level = c_list[i + m_level[0]].level;
    }
    i = rund(m_level[level]-m_level[level-1]) + m_level[level-1];
  }
  return i;
}

/* Allocates a random monster				-RAK-	*/
void alloc_monster(Short num, Short dist, Boolean sleep)
{
  Short y, x, i;
  Short m;

  for (i = 0; i < num; i++) {
    do {
      y = randint(cur_height-2);
      x = randint(cur_width-2);
    } while (cave_fval[y*MAX_WIDTH+x] >= MIN_CLOSED_SPACE
	     || (cave_cptr[y*MAX_WIDTH+x] != 0)
	     || (distance(y, x, char_row, char_col) <= dist));

    m = get_mons_num (dun_level);
    /* Dragons are always created sleeping here, so as to give the player a
       sporting chance.  */
    if (c_list[m].cchar == 'd' || c_list[m].cchar == 'D')
      sleep = true;
    /* Place_monster() should always return TRUE here.  It does not
       matter if it fails though.  */
    place_monster(y, x, m, sleep);
  }
}

/* Places creature adjacent to given location		-RAK-	*/
Boolean summon_monster(Short *y, Short *x, Boolean slp)
{
  Short i, j, k;
  Short m, summon, c_ptr;
  //  register cave_type *cave_ptr;
  m = get_mons_num(dun_level + MON_SUMMON_ADJ);
  i = 0;
  summon = false;
  for (i = 0 ; i < 10 ; i++) {
    j = *y - 2 + randint(3);
    k = *x - 2 + randint(3);
    if (in_bounds(j, k)) {
      c_ptr = j*MAX_WIDTH+k;
      if (cave_fval[c_ptr] <= MAX_OPEN_SPACE && (cave_cptr[c_ptr] == 0)) {
	/* Place_monster() "should" always return TRUE here.  */
	if (!place_monster(j, k, m, slp)) return false;
	*y = j;
	*x = k;
	return true; //	summon = true; break;
      }
    }
  }
  return summon;
}

/* Places undead adjacent to given location		-RAK-	*/
Boolean summon_undead(Short *y, Short *x)
{
  Short i, j, k, c_ptr;
  Short maxlev, m, tries, summon;
  //  register cave_type *cave_ptr;

  i = m = 0;
  summon = false;
  maxlev = m_level[MAX_MONS_LEVEL];
  while (maxlev > 0) {
    m = rund(maxlev);
    for (tries = 0 ; tries < 20 ; tries++) {
      if (c_list[m].cdefense & CD_UNDEAD) {
	maxlev = 0;
	break;
      }
      m++;
      if (m > maxlev)
	break;
    }
  }
  for (tries = 0 ; tries < 10 ; tries++) {
    j = *y - 2 + randint(3);
    k = *x - 2 + randint(3);
    if (!in_bounds(j, k)) continue;
    c_ptr = j*MAX_WIDTH+k;
    if (cave_fval[c_ptr] <= MAX_OPEN_SPACE && (cave_cptr[c_ptr] == 0)) {
      /* Place_monster() should always return true here.  */
      if (!place_monster(j, k, m, false)) return false;
      summon = true;
      *y = j;
      *x = k;
      break;
    }
  }
  return summon;
}

/* If too many objects on floor level, delete some of them-RAK-	*/
// You know this otter be in lock.c
static void compact_objects()
{
  // used in misc1.c, or indirectly by anything that calls popt!
  return;
}


/* Gives pointer to next free space                     -RAK-   */
Short popt()
{
  if (tcptr == MAX_TALLOC)
    tcptr = 0;  //    compact_objects();
  return (tcptr++);
  //  return 0; // not implemented
}

/* Pushs a record back onto free space list		-RAK-	*/
/* Delete_object() should always be called instead, unless the object in
   question is not in the dungeon, e.g. in store1.c and files.c */
void pusht(UChar x)
{
  Short i, j;

  if (x != tcptr - 1) {
    tlist_mv(tcptr - 1, x);

    /* must change the tptr in the cave of the object just moved */
    for (i = 0; i < cur_height; i++)
      for (j = 0; j < cur_width; j++)
	if (cave_tptr[i*MAX_WIDTH+j] == tcptr - 1)
	  cave_set(i, j, CAVE_tptr, x);
  }
  tcptr--;
  invcopy(t_list, tcptr, OBJ_NOTHING);
}

/* Boolean : is object enchanted          -RAK- */
Boolean magik(Short chance)
{
  return (randint(100) <= chance);
}




/* Enchant a bonus based on degree desired -RAK- */
Short m_bonus(Short base, Short max_std, Short level)
{
  Short x, stand_dev, tmp;

  stand_dev = (OBJ_STD_ADJ * level / 100) + OBJ_STD_MIN;
  /* Check for level > max_std since that may have generated an overflow.  */
  if (stand_dev > max_std || level > max_std)
    stand_dev = max_std;
  /* abs may be a macro, don't call it with randnor as a parameter */
  tmp = randnor(0, stand_dev);
  x = (abs(tmp) / 10) + base;
  if (x < base)
    return(base);
  else
    return(x);
}
