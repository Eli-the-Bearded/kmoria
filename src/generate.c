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




// prototypes
//void generate_cave();
static void town_gen() SEC_1;
static void build_store(Short store_num, Short y, Short x) SEC_1;
static void fill_cave(Short fval) SEC_1;
static void place_boundary() SEC_1;
static void place_stairs(Short typ, Short num_stairs, Short walls) SEC_1;
static void place_up_stairs(Short y, Short x) SEC_1;
static void place_down_stairs(Short y, Short x) SEC_1;
static Short next_to_walls(Short y, Short x) SEC_1;
static void new_spot(Short *y, Short *x) SEC_1;
static void cave_gen() SEC_1;
static void vault_trap(Short y, Short x, Short yd, Short xd, Short num) SEC_1;
static void vault_monster(Short y, Short x, Short num) SEC_1;
static void build_room(Short yval, Short xval) SEC_1;
static void build_type1(Short yval, Short xval) SEC_1;
static void build_type2(Short yval, Short xval) SEC_1;
static void build_type3(Short yval, Short xval) SEC_1;
static void correct_dir(Short *rdir, Short *cdir,
			Short y1, Short x1, Short y2, Short x2) SEC_1;
static void rand_dir(Short *rdir, Short *cdir) SEC_1;
static void build_tunnel(Short row1, Short col1, Short row2, Short col2) SEC_1;
static void place_streamer(Short fval, Short treas_chance) SEC_1;
static Short next_to_corr(Short y, Short x) SEC_1;
static Boolean next_to(Short y, Short x) SEC_1;
static void try_door(Short y, Short x) SEC_1;
static void place_open_door(Short y, Short x) SEC_1;
static void place_broken_door(Short y, Short x) SEC_1;
static void place_door(Short y, Short x) SEC_1;
static void place_closed_door(Short y, Short x) SEC_1;
static void place_locked_door(Short y, Short x) SEC_1;
static void place_stuck_door(Short y, Short x) SEC_1;
static void place_secret_door(Short y, Short x) SEC_1;




//Char moron_caller[40];


// Dunno what to do about tlink and mlink.
void generate_cave()
{
  // hmmm.. I think in my world these belong in display.c
  panel_row_min	= 0;
  panel_row_max	= 0;
  panel_col_min	= 0;
  panel_col_max	= 0;
  char_row = -1;
  char_col = -1;

  // tlink, mlink.
  blank_cave();
  blank_tlist(); // replaced tlink
  blank_mlist(); // replaced mlink

  // I might be changing screen_height and screen_width.
  if (dun_level == 0) {
    cur_height = SCREEN_HEIGHT;
    cur_width  = SCREEN_WIDTH;
    max_panel_rows = (cur_height/SCREEN_HEIGHT)*2 - 2; // 1*2 - 2 = 0
    max_panel_cols = (cur_width /SCREEN_WIDTH )*2 - 2; // 2*2 - 2 = 2
    panel_row = max_panel_rows; // 0
    panel_col = max_panel_cols; // 2
    town_gen();
  } else {
    cur_height = MAX_HEIGHT;
    cur_width  = MAX_WIDTH;
    max_panel_rows = (cur_height/SCREEN_HEIGHT)*2 - 2; // 3*2 - 2 = 4
    max_panel_cols = (cur_width /SCREEN_WIDTH )*2 - 2; // 6*2 - 2 = 10
    panel_row = max_panel_rows; // 4
    panel_col = max_panel_cols; // 10
    cave_gen();
  }
}

////////////// town gen stuff/////////////////////////////////////

/* Town logic flow for generation of new town		*/
static void town_gen()
{
  Short i, j, row, col, k, m;
  Short c_ptr;
  Short rooms[6], r;
  Boolean is_night;
  UChar floorval, lightval;
  //  StrCopy(moron_caller, "town_gen");

  set_seed(town_seed);
  //  set_seed((ULong) 42);
  for (i = 0; i < 6; i++)
    rooms[i] = i;
  for (i = 0, k = 6 ; i < 2 ; i++)
    for (j = 0      ; j < 3 ; j++, k--) {
      r = rund(k);
      build_store(rooms[r], i, j); // bug here somewhere.
      for (m = r ; m < k-1 ; m++)
	rooms[m] = rooms[m+1];
    }
  fill_cave(DARK_FLOOR);
  /* make stairs before reset_seed, so that they don't move around */
  place_boundary();
  place_stairs(2, 1, 0);
  reset_seed();
  /* Set up the character co-ords, used by alloc_monster below */
  new_spot(&row, &col); // xxx will I be sad that these are ptrs?
  char_row = row; char_col = col;
  is_night = (0x1 & (turn / 5000));
  for (row = 0; row < cur_height; row++) {
    for (col = 0; col < cur_width; col++) {
      c_ptr = row * MAX_WIDTH + col;
      floorval = cave_fval[c_ptr];
      if (!is_night || (floorval != DARK_FLOOR)) {
	lightval = cave_light[c_ptr] | LIGHT_pl; // turn on 'pl'
	cave_set(row, col, CAVE_light, lightval);
      }
    }
  }
  alloc_monster( (is_night ? MIN_MALLOC_TN : MIN_MALLOC_TD), 3, true);

  store_maint(); // xxx test me
}



/* Builds a store at a row, column coordinate			*/
static void build_store(Short store_num, Short y, Short x)
{
  Short yval, y_height, y_depth;
  Short xval, x_left, x_right;
  Short i, j;
  Short cur_pos, tmp;
  //  StrCopy(moron_caller, "build_store");

  yval	   = y*10 + 5;
  xval	   = x*16 + 16;
  y_height = yval - randint(3);
  y_depth  = yval + randint(4);
  x_left   = xval - randint(6);
  x_right  = xval + randint(6);
  for (i = y_height; i <= y_depth; i++)
    for (j = x_left; j <= x_right; j++)
      cave_set(i, j, CAVE_fval, BOUNDARY_WALL);
  tmp = randint(4);
  if (tmp < 3) {
    i = randint(y_depth-y_height) + y_height - 1;
    if (tmp == 1) j = x_left;
    else	  j = x_right;
  } else {
    j = randint(x_right-x_left) + x_left - 1;
    if (tmp == 3) i = y_depth;
    else	  i = y_height;
  }
  cave_set(i, j, CAVE_fval, CORR_FLOOR);
  cur_pos = popt();
  cave_set(i, j, CAVE_tptr, cur_pos);
  invcopy(t_list, cur_pos, OBJ_STORE_DOOR + store_num);
}



/* Fills in empty spots with desired rock		-RAK-	*/
/* Note: 9 is a temporary value.				*/
static void fill_cave(Short fval)
{
  Short i, j; // row, col.
  UChar floorval;
  //  StrCopy(moron_caller, "fill_cave");

  /* no need to check the border of the cave */
  for (i = cur_height - 2; i > 0; i--) {
    for (j = cur_width - 2; j > 0; j--) {
      floorval = cave_fval[i*MAX_WIDTH+j];
      if ((floorval == NULL_WALL) || (floorval == TMP1_WALL) ||
	  (floorval == TMP2_WALL))
	cave_set(i, j, CAVE_fval, fval); // write cave record
    }
  }
}



/* Places indestructible rock around edges of dungeon	-RAK-	*/
static void place_boundary()
{
  Short i;
  //  StrCopy(moron_caller, "place_bound");
  // There was some weird ass pointer arithmetic here before.
  // I imagine it speeds things up, but I would rather keep it simple looking.

  /*                            row                col                */
  /* put permanent wall on cave [0]                [0 .. cur_width-1] */
  /* put permanent wall on cave [cur_height-1]     [0 .. cur_width-1] */
  for (i = 0 ; i < cur_width ; i++) {
    cave_set(0,            i, CAVE_fval, BOUNDARY_WALL);
    cave_set(cur_height-1, i, CAVE_fval, BOUNDARY_WALL);
  }
  /*                            row                col                */
  /* put permanent wall on cave [0 .. cur_height-1][0]                */
  /* put permanent wall on cave [0 .. cur_height-1][cur_width-1]      */
  for (i = 0 ; i < cur_height ; i++) {
    cave_set(i, 0,            CAVE_fval, BOUNDARY_WALL);
    cave_set(i, cur_width-1, CAVE_fval, BOUNDARY_WALL);
  }
}




/* Places a staircase 1=up, 2=down			-RAK-	*/
#define UP_STAIRS 1
#define DOWN_STAIRS 2
static void place_stairs(Short typ, Short num_stairs, Short walls)
{
  // Hello, what is num?  Number of stairs to place, I think?
  // walls is the number of walls you want to be next-to your stairs
  // (i.e. 3 = walls on 3/4 of n,s,e,w sides.)
  Short c_ptr;
  Short stairs_placed, tries;
  Boolean flag; // tell us when to stop
  Short y, x;
  Short initial_x, initial_y, final_x, final_y;

  for (stairs_placed = 0; stairs_placed < num_stairs; stairs_placed++) {
    flag = false;
    while (!flag) {
      tries = 0;
      while (!flag && tries <= 30) {
	/* Note: don't let y or x be zero, and don't let them reach
	   cur_height-1/cur_width-1, these values are always
	   BOUNDARY_ROCK. */
	initial_y = randint(cur_height - 14);
	initial_x = randint(cur_width  - 14);
	final_y = initial_y + 12;
	final_x = initial_x + 12;
	for (y = initial_y ; !flag && (y < final_y) ; y++) {
	  for (x = initial_x ; !flag && (x < final_x) ; x++) {
	    c_ptr =  y * MAX_WIDTH + x;
	    if (cave_fval[c_ptr] <= MAX_OPEN_SPACE
		&& (cave_tptr[c_ptr] == 0)
		&& (next_to_walls(y, x) >= walls)) {
	      // the floor is open space, there is no object, place a stair!
	      flag = true;
	      if (typ == UP_STAIRS)
		place_up_stairs(y, x);
	      else
		place_down_stairs(y, x);
	    }
	  } // end for x (and not flag)
	} // end for y (and not flag)
	tries++;
      } // end while not flag, and tries<=30
      walls--;
    } // end while not flag
  } // end for i
}

/* Place an up staircase at given y, x			-RAK-	*/
static void place_up_stairs(Short y, Short x)
{
  Short cur_pos;
  Short c_ptr;
  //  StrCopy(moron_caller, "place_up_st");

  c_ptr = y * MAX_WIDTH + x;
  if (cave_tptr[c_ptr] != 0)
    delete_object(y, x);
  cur_pos = popt();
  cave_set(y, x, CAVE_tptr, cur_pos);
  invcopy(t_list, cur_pos, OBJ_UP_STAIR);
}

/* Place a down staircase at given y, x			-RAK-	*/
// This is nearly identical to place_up_stairs
// Surely I could combine the two fskers.
static void place_down_stairs(Short y, Short x)
{
  Short cur_pos;
  Short c_ptr;
  //  StrCopy(moron_caller, "place_down_st");

  c_ptr = y * MAX_WIDTH + x;
  if (cave_tptr[c_ptr] != 0)
    delete_object(y, x);
  cur_pos = popt();
  cave_set(y, x, CAVE_tptr, cur_pos);
  invcopy(t_list, cur_pos, OBJ_DOWN_STAIR);
}


/* Checks points north, south, east, and west for a wall -RAK-	*/
/* note that y,x is always in_bounds(), i.e. 0 < y < cur_height-1, and
   0 < x < cur_width-1	*/
// formerly in misc1.c
static Short next_to_walls(Short y, Short x)
{
  Short i;
  Short c_ptr;

  i = 0;
  c_ptr = (y-1)*MAX_WIDTH + x;
  if (cave_fval[c_ptr] >= MIN_CAVE_WALL)          i++;
  c_ptr = (y+1)*MAX_WIDTH + x;
  if (cave_fval[c_ptr] >= MIN_CAVE_WALL)          i++;
  c_ptr = y*MAX_WIDTH + x-1;
  if (cave_fval[c_ptr] >= MIN_CAVE_WALL)          i++;
  c_ptr = y*MAX_WIDTH + x+1;
  if (cave_fval[c_ptr] >= MIN_CAVE_WALL)          i++;

  return i;
}




/* Returns random co-ordinates				-RAK-	*/
static void new_spot(Short *y, Short *x)
{
  Short i, j;
  Short c_ptr;

  do {
    i = randint(cur_height - 2);
    j = randint(cur_width - 2);
    c_ptr = i*MAX_WIDTH + j;
  } while (cave_fval[c_ptr] >= MIN_CLOSED_SPACE || (cave_cptr[c_ptr] != 0)
	   || (cave_tptr[c_ptr] != 0));
  *y = i;
  *x = j;
}




////////////// cave gen stuff/////////////////////////////////////

/* Cave logic flow for generation of new dungeon		*/
Boolean room_map[20][20]; // it's best not to have much stuff on the stack
Short yloc[400], xloc[400]; // because the palmos stack is not very big
// dude.. room_map SHOULD be a bit vector!  save 8x space.
typedef struct coords {
  int x, y;
} coords;
coords doorstk[100];
int doorindex;

static void cave_gen()
{
  Short i, j, k;
  Short y1, x1, y2, x2, pick1, pick2, tmp;
  Short row_rooms, col_rooms, alloc_level;

  //  Char biteme_buf[40];
  alert_message("digging..");

  for (i = 0 ; i < 400 ; i++)
    yloc[i] = xloc[i] = 0;

  row_rooms = 2 * (cur_height/SCREEN_HEIGHT); // 2*3
  col_rooms = 2 * (cur_width /SCREEN_WIDTH);  // 2*6
  for (i = 0; i < row_rooms; i++)
    for (j = 0; j < col_rooms; j++)
      room_map[i][j] = false;

  k = randnor(DUN_ROO_MEA, 2);
  for (i = 0; i < k; i++)
    room_map[ rund(row_rooms) ][ rund(col_rooms) ] = true;

  k = 0;
  for (i = 0; i < row_rooms; i++)
    for (j = 0; j < col_rooms; j++)
      if (room_map[i][j]) {
	// Note to self - I changed SCREEN_ and QUART_ so this might suck.
	yloc[k] = i * (SCREEN_HEIGHT >> 1) + QUART_HEIGHT;
	xloc[k] = j * (SCREEN_WIDTH >> 1) + QUART_WIDTH;

	//	StrPrintF(biteme_buf, "cg %d/%d %d/%d  %d",
	//		  yloc[k], MAX_HEIGHT, xloc[k], MAX_WIDTH, k);
	//	message(biteme_buf);
	//	SysTaskDelay(100);

	if (dun_level > randint(DUN_UNUSUAL)) { // TESTING, added 0&&
	  // some rooms are 'unusual'
	  tmp = randint(3);
	  if (tmp == 1)
	    build_type1(yloc[k], xloc[k]);
	  else if (tmp == 2)
	    build_type2(yloc[k], xloc[k]);
	  else
	    build_type3(yloc[k], xloc[k]);
	} else
	  build_room(yloc[k], xloc[k]);
	k++;
      }
  for (i = 0; i < k; i++) {
    // some gratuitous shuffling here
    pick1 = rund(k);
    pick2 = rund(k);
    y1 = yloc[pick1];
    x1 = xloc[pick1];
    yloc[pick1] = yloc[pick2];
    xloc[pick1] = xloc[pick2];
    yloc[pick2] = y1;
    xloc[pick2] = x1;
  }
  doorindex = 0;
  /* move zero entry to k, so that can call build_tunnel all k times */
  yloc[k] = yloc[0];
  xloc[k] = xloc[0];
  alert_message("tunnelling..");
  for (i = 0; i < k; i++) {
    // build tunnels between k pairs of rooms
    y1 = yloc[i];
    x1 = xloc[i];
    y2 = yloc[i+1];
    x2 = xloc[i+1];
    build_tunnel(y2, x2, y1, x1);
  }
  fill_cave(GRANITE_WALL);
  for (i = 0; i < DUN_STR_MAG; i++)
    place_streamer(MAGMA_WALL, DUN_STR_MC);
  for (i = 0; i < DUN_STR_QUA; i++)
    place_streamer(QUARTZ_WALL, DUN_STR_QC);
  place_boundary(); // DmWriteCheck fails now!
  /* Place intersection doors	*/
  alert_message("sweeping the floor..");
  for (i = 0; i < doorindex; i++) {
    try_door(doorstk[i].y, doorstk[i].x-1);
    try_door(doorstk[i].y, doorstk[i].x+1);
    try_door(doorstk[i].y-1, doorstk[i].x);
    try_door(doorstk[i].y+1, doorstk[i].x);
  }
  alloc_level = dun_level / 3;
  if (alloc_level < 2)
    alloc_level = 2;
  else if (alloc_level > 10)
    alloc_level = 10;
  place_stairs(2, randint(2)+2, 3);
  place_stairs(1, randint(2), 3);
  /* Set up the character co-ords, used by alloc_monster, place_win_monster */
  new_spot(&i, &j); //  new_spot(&char_row, &char_col);
  char_row = i; char_col = j;
  alloc_monster((randint(8)+MIN_MALLOC_LEVEL+alloc_level), 0, true);
  alloc_object(IS_CORR, 3, randint(alloc_level));
  alloc_object(IS_ROOM, 5, randnor(TREAS_ROOM_ALLOC, 3));
  alloc_object(IS_FLOOR, 5, randnor(TREAS_ANY_ALLOC, 3));
  alloc_object(IS_FLOOR, 4, randnor(TREAS_GOLD_ALLOC, 3));
  alloc_object(IS_FLOOR, 1, randint(alloc_level));
  if (dun_level >= WIN_MON_APPEAR)
    place_win_monster();
}


/* Place a trap with a given displacement of point	-RAK-	*/
static void vault_trap(Short y, Short x, Short yd, Short xd, Short num)
{
  Short tries, y1, x1;
  Short i;
  UChar floor;

  for (i = 0; i < num; i++) {
    for (tries = 0 ; tries < 5 ; tries++) {
      y1 = y - yd - 1 + randint(2*yd+1);
      x1 = x - xd - 1 + randint(2*xd+1);
      floor = cave_fval[y1*MAX_WIDTH+x1];
      if ((floor != NULL_WALL) && (floor <= MAX_CAVE_FLOOR)
	  && (cave_tptr[y1*MAX_WIDTH+x1] == 0)) {
	place_trap(y1, x1, randint(MAX_TRAP)-1);
	break;
      }
    }
  }
}


/* Place a trap with a given displacement of point	-RAK-	*/
static void vault_monster(Short y, Short x, Short num)
{
  Short i, y1, x1;

  for (i = 0; i < num; i++) {
    y1 = y;
    x1 = x;
    summon_monster(&y1, &x1, true);
  }
}




/* Builds a room at a row, column coordinate		-RAK-	*/
static void build_room(Short yval, Short xval)
{
  Short i, j, y_depth, x_right;
  Short y_height, x_left;
  UChar floor, light;
  //  StrPrintF(moron_caller, "build_room %d %d", yval, xval);

  if (dun_level <= randint(25))
    floor = LIGHT_FLOOR;	/* Floor with light	*/
  else
    floor = DARK_FLOOR;		/* Dark floor		*/

  //                             v-- this appears to be necessary! --v
  y_height = yval - randint(4);  y_height = max(y_height, 1);
  y_depth  = yval + randint(3);  y_depth  = min(y_depth, MAX_HEIGHT-2);
  x_left   = xval - randint(11); x_left  = max(x_left, 1);
  x_right  = xval + randint(11); x_right = min(x_right, MAX_WIDTH-2);

  // floor-space
  for (i = y_height; i <= y_depth; i++) {
    for (j = x_left; j <= x_right; j++) {
      cave_set(i, j, CAVE_fval, floor);
      light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr; // turn on 'lr'
      cave_set(i, j, CAVE_light, light);
    }
  }

  // two parallel walls
  for (i = (y_height - 1); i <= (y_depth + 1); i++) {
    j = x_left-1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
    j = x_right+1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
  }

  // other two parallel walls
  for (j = x_left; j <= x_right; j++) {
    i = y_height - 1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
    i = y_depth + 1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
  }
}

/* Builds a room at a row, column coordinate		-RAK-	*/
/* Type 1 unusual rooms are several overlapping rectangular ones	*/
static void build_type1(Short yval, Short xval)
{
  Short y_height, y_depth;
  Short x_left, x_right, limit;
  Short i0, i, j;
  UChar floor, light;
  //  StrCopy(moron_caller, "build_type1");

  if (dun_level <= randint(25))
    floor = LIGHT_FLOOR;	/* Floor with light	*/
  else
    floor = DARK_FLOOR;		/* Dark floor		*/
  limit = 1 + randint(2);
  for (i0 = 0; i0 < limit; i0++) {
    //                             v-- probably still necessary --v
    y_height = yval - randint(4);  y_height = max(y_height, 1);
    y_depth  = yval + randint(3);  y_depth  = min(y_depth, MAX_HEIGHT-2);
    x_left   = xval - randint(11); x_left  = max(x_left, 1);
    x_right  = xval + randint(11); x_right = min(x_right, MAX_WIDTH-2);

    /* the x dim of rooms tends to be much larger than the y dim, so don't
       bother rewriting the y loop */

    // floor-space
    for (i = y_height; i <= y_depth; i++) {
      for (j = x_left; j <= x_right; j++) {
	cave_set(i, j, CAVE_fval, floor);
	light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr; // turn on 'lr'
	cave_set(i, j, CAVE_light, light);
      }
    }

    // two parallel walls.. don't overwrite floor
    for (i = (y_height - 1); i <= (y_depth + 1); i++) {
      j = x_left-1;
      if (cave_fval[i*MAX_WIDTH+j] != floor) {
	cave_set(i, j, CAVE_fval, GRANITE_WALL);
	light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
	cave_set(i, j, CAVE_light, light);
      }
      j = x_right+1;
      if (cave_fval[i*MAX_WIDTH+j] != floor) {
	cave_set(i, j, CAVE_fval, GRANITE_WALL);
	light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
	cave_set(i, j, CAVE_light, light);
      }
    }

    // other two parallel walls.. don't overwrite floor
    for (j = x_left; j <= x_right; j++) {
      i = y_height - 1;
      if (cave_fval[i*MAX_WIDTH+j] != floor) {
	cave_set(i, j, CAVE_fval, GRANITE_WALL);
	light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
	cave_set(i, j, CAVE_light, light);
      }
      i = y_depth + 1;
      if (cave_fval[i*MAX_WIDTH+j] != floor) {
	cave_set(i, j, CAVE_fval, GRANITE_WALL);
	light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
	cave_set(i, j, CAVE_light, light);
      }
    }
  } // end for i0
}


/* Builds an unusual room at a row, column coordinate	-RAK-	*/
/* Type 2 unusual rooms all have an inner room:			*/
/*   1 - Just an inner room with one door			*/
/*   2 - An inner room within an inner room			*/
/*   3 - An inner room with pillar(s)				*/
/*   4 - Inner room has a maze					*/
/*   5 - A set of four inner rooms				*/
static void build_type2(Short yval, Short xval)
{
  Short i, j, y_height, x_left;
  Short y_depth, x_right, tmp;
  UChar floor, light;
  //  cave_type *c_ptr, *d_ptr;
  //  StrCopy(moron_caller, "build_type2");

  if (dun_level <= randint(25))
    floor = LIGHT_FLOOR;	/* Floor with light	*/
  else
    floor = DARK_FLOOR;		/* Dark floor		*/
  //                             v-- might as well do this here too --v
  y_height = yval - 4;        y_height = max(y_height, 1);           
  y_depth  = yval + 4;	      y_depth  = min(y_depth, MAX_HEIGHT-2); 
  x_left   = xval - 11;	      x_left  = max(x_left, 1);              
  x_right  = xval + 11;	      x_right = min(x_right, MAX_WIDTH-2);   

  // floor-space
  for (i = y_height; i <= y_depth; i++) {
    for (j = x_left; j <= x_right; j++) {
      cave_set(i, j, CAVE_fval, floor);
      light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr; // turn on 'lr'
      cave_set(i, j, CAVE_light, light);
    }
  }

  // two parallel walls
  for (i = (y_height - 1); i <= (y_depth + 1); i++) {
    j = x_left-1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
    j = x_right+1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
  }

  // other two parallel walls
  for (j = x_left; j <= x_right; j++) {
    i = y_height - 1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
    i = y_depth + 1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
  }

  /* The inner room		*/
  y_height = y_height + 2;
  y_depth  = y_depth  - 2;
  x_left   = x_left   + 2;
  x_right  = x_right  - 2;
  for (i = (y_height - 1); i <= (y_depth + 1); i++) {
    cave_set(i, x_left-1,  CAVE_fval, TMP1_WALL);
    cave_set(i, x_right+1, CAVE_fval, TMP1_WALL);
  }
  for (i = x_left; i <= x_right; i++) {
    cave_set(y_height-1, i, CAVE_fval, TMP1_WALL);
    cave_set(y_depth+1,  i, CAVE_fval, TMP1_WALL);
  }
  /* Inner room variations		*/
  switch(randint(5)) {
  case 1:	/* Just an inner room.	*/
    tmp = randint(4);
    if (tmp < 3) {	/* Place a door	*/
      if (tmp == 1) place_secret_door(y_height-1, xval);
      else          place_secret_door(y_depth+1,  xval);
    } else {
      if (tmp == 3) place_secret_door(yval, x_left-1);
      else          place_secret_door(yval, x_right+1);
    }
    vault_monster(yval, xval, 1);
    break;

  case 2:	/* Treasure Vault	*/
    tmp = randint(4);
    if (tmp < 3) {	/* Place a door	*/
      if (tmp == 1) place_secret_door(y_height-1, xval);
      else          place_secret_door(y_depth+1,  xval);
    } else {
      if (tmp == 3) place_secret_door(yval, x_left-1);
      else          place_secret_door(yval, x_right+1);
    }

    for (i = yval-1; i <= yval+1; i++) {
      cave_set(i, xval-1, CAVE_fval, TMP1_WALL);
      cave_set(i, xval+1, CAVE_fval, TMP1_WALL);
    }
    cave_set(yval-1, xval, CAVE_fval, TMP1_WALL);
    cave_set(yval+1, xval, CAVE_fval, TMP1_WALL);

    tmp = randint(4);	/* Place a door	*/
    if (tmp < 3)
      place_locked_door(yval-3+(tmp<<1), xval); /* 1 -> yval-1; 2 -> yval+1*/
    else
      place_locked_door(yval, xval-7+(tmp<<1));

    /* Place an object in the treasure vault	*/
    tmp = randint(10);
    if (tmp > 2)
      place_object(yval, xval, false);
    else if (tmp == 2)
      place_down_stairs(yval, xval);
    else
      place_up_stairs(yval, xval);

    /* Guard the treasure well		*/
    vault_monster(yval, xval, 2+randint(3));
    /* If the monsters don't get 'em.	*/
    vault_trap(yval, xval, 4, 10, 2+randint(3));
    break;

  case 3:	/* Inner pillar(s).	*/
    tmp = randint(4);
    if (tmp < 3) {	/* Place a door	*/
      if (tmp == 1) place_secret_door(y_height-1, xval);
      else          place_secret_door(y_depth+1,  xval);
    } else {
      if (tmp == 3) place_secret_door(yval, x_left-1);
      else          place_secret_door(yval, x_right+1);
    }

    for (i = yval-1; i <= yval+1; i++)
      for (j = xval-1; j <= xval+1; j++)
	cave_set(i, j, CAVE_fval, TMP1_WALL);

    if (randint(2) == 1) {
      tmp = randint(2);
      for (i = yval-1; i <= yval+1; i++)
	for (j = xval-5-tmp; j <= xval-3-tmp; j++)
	  cave_set(i, j, CAVE_fval, TMP1_WALL);
      for (i = yval-1; i <= yval+1; i++)
	for (j = xval+3+tmp; j <= xval+5+tmp; j++)
	  cave_set(i, j, CAVE_fval, TMP1_WALL);
    }


    if (randint(3) == 1) {	/* Inner rooms	*/
      for (i = xval-5; i <= xval+5; i++) {
	cave_set(yval-1, i, CAVE_fval, TMP1_WALL);
	cave_set(yval+1, i, CAVE_fval, TMP1_WALL);
      }
      cave_set(yval, xval-5, CAVE_fval, TMP1_WALL);
      cave_set(yval, xval+5, CAVE_fval, TMP1_WALL);
      place_secret_door(yval-3+(randint(2)<<1), xval-3);
      place_secret_door(yval-3+(randint(2)<<1), xval+3);
      if (randint(3) == 1)	place_object(yval, xval-2, false);
      if (randint(3) == 1)	place_object(yval, xval+2, false);
      vault_monster(yval, xval-2, randint(2));
      vault_monster(yval, xval+2, randint(2));
    }
    break;

  case 4:	/* Maze inside.	*/
    tmp = randint(4);
    if (tmp < 3) {	/* Place a door	*/
      if (tmp == 1) place_secret_door(y_height-1, xval);
      else          place_secret_door(y_depth+1,  xval);
    } else {
      if (tmp == 3) place_secret_door(yval, x_left-1);
      else          place_secret_door(yval, x_right+1);
    }

    for (i = y_height; i <= y_depth; i++)
      for (j = x_left; j <= x_right; j++)
	if (0x1 & (j+i))
	  cave_set(i, j, CAVE_fval, TMP1_WALL);

    /* Monsters just love mazes.		*/
    vault_monster(yval, xval-5, randint(3));
    vault_monster(yval, xval+5, randint(3));
    /* Traps make them entertaining.	*/
    vault_trap(yval, xval-3, 2, 8, randint(3));
    vault_trap(yval, xval+3, 2, 8, randint(3));
    /* Mazes should have some treasure too..	*/
    for (i = 0; i < 3; i++)
      random_object(yval, xval, 1);
    break;

  case 5:	/* Four small rooms.	*/
    for (i = y_height; i <= y_depth; i++)
      cave_set(i, xval, CAVE_fval, TMP1_WALL);

    for (i = x_left; i <= x_right; i++)
      cave_set(yval, i, CAVE_fval, TMP1_WALL);

    if (randint(2) == 1) {
      i = randint(10);
      place_secret_door(y_height-1, xval-i);
      place_secret_door(y_height-1, xval+i);
      place_secret_door(y_depth+1, xval-i);
      place_secret_door(y_depth+1, xval+i);
    } else {
      i = randint(3);
      place_secret_door(yval+i, x_left-1);
      place_secret_door(yval-i, x_left-1);
      place_secret_door(yval+i, x_right+1);
      place_secret_door(yval-i, x_right+1);
    }

    /* Treasure in each one.		*/
    random_object(yval, xval, 2+randint(2));
    /* Gotta have some monsters.		*/
    vault_monster(yval+2, xval-4, randint(2));
    vault_monster(yval+2, xval+4, randint(2));
    vault_monster(yval-2, xval-4, randint(2));
    vault_monster(yval-2, xval+4, randint(2));
    break;
  }
}


/* Builds a room at a row, column coordinate		-RAK-	*/
/* Type 3 unusual rooms are cross shaped				*/
static void build_type3(Short yval, Short xval)
{
  Short y_height, y_depth;
  Short x_left, x_right;
  Short tmp, i, j;
  UChar floor, light;
  //  register cave_type *c_ptr;
  //  StrCopy(moron_caller, "build_type3");

  if (dun_level <= randint(25))
    floor = LIGHT_FLOOR;	/* Floor with light	*/
  else
    floor = DARK_FLOOR;		/* Dark floor		*/
  tmp = 2 + randint(2);
  y_height = yval - tmp;
  y_depth  = yval + tmp;
  x_left   = xval - 1;
  x_right  = xval + 1;
  // floor-space
  for (i = y_height; i <= y_depth; i++) {
    for (j = x_left; j <= x_right; j++) {
      cave_set(i, j, CAVE_fval, floor);
      light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr; // turn on 'lr'
      cave_set(i, j, CAVE_light, light);
    }
  }
  // two parallel walls
  for (i = (y_height - 1); i <= (y_depth + 1); i++) {
    j = x_left-1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
    j = x_right+1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
  }

  // other two parallel walls
  for (j = x_left; j <= x_right; j++) {
    i = y_height - 1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
    i = y_depth + 1;
    cave_set(i, j, CAVE_fval, GRANITE_WALL);
    light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
    cave_set(i, j, CAVE_light, light);
  }

  tmp = 2 + randint(9);
  y_height = yval - 1;
  y_depth  = yval + 1;
  x_left   = xval - tmp;
  x_right  = xval + tmp;
  for (i = y_height; i <= y_depth; i++)
    for (j = x_left; j <= x_right; j++) {
      cave_set(i, j, CAVE_fval, floor);
      light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
      cave_set(i, j, CAVE_light, light);
    }

  for (i = (y_height - 1); i <= (y_depth + 1); i++) {
    j = x_left-1;
    if (cave_fval[i*MAX_WIDTH+j] != floor) {
      cave_set(i, j, CAVE_fval, GRANITE_WALL);
      light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
      cave_set(i, j, CAVE_light, light);
    }
    j = x_right+1;
    if (cave_fval[i*MAX_WIDTH+j] != floor) {
      cave_set(i, j, CAVE_fval, GRANITE_WALL);
      light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
      cave_set(i, j, CAVE_light, light);
    }
  }
  for (j = x_left; j <= x_right; j++) {
    i = y_height-1;
    if (cave_fval[i*MAX_WIDTH+j] != floor) {
      cave_set(i, j, CAVE_fval, GRANITE_WALL);
      light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
      cave_set(i, j, CAVE_light, light);
    }
    i = y_depth+1;
    if (cave_fval[i*MAX_WIDTH+j] != floor) {
      cave_set(i, j, CAVE_fval, GRANITE_WALL);
      light = cave_light[i*MAX_WIDTH+j] | LIGHT_lr;
      cave_set(i, j, CAVE_light, light);
    }
  }
  /* Special features.			*/
  switch(randint(4)) {
  case 1:	/* Large middle pillar		*/
    for (i = yval-1; i <= yval+1; i++)
      for (j = xval-1; j <= xval+1; j++)
	cave_set(i, j, CAVE_fval, TMP1_WALL);
    break;

  case 2:	/* Inner treasure vault		*/
    for (i = yval-1; i <= yval+1; i++) {
      cave_set(i, xval-1, CAVE_fval, TMP1_WALL);
      cave_set(i, xval+1, CAVE_fval, TMP1_WALL);
    }
    cave_set(yval-1, xval, CAVE_fval, TMP1_WALL);
    cave_set(yval+1, xval, CAVE_fval, TMP1_WALL);

    tmp = randint(4);	/* Place a door	*/
    if (tmp < 3)
      place_secret_door(yval-3+(tmp<<1), xval);
    else
      place_secret_door(yval, xval-7+(tmp<<1));

    /* Place a treasure in the vault		*/
    place_object(yval, xval, false);
    /* Let's guard the treasure well.	*/
    vault_monster(yval, xval, 2+randint(2));
    /* Traps naturally			*/
    vault_trap(yval, xval, 4, 4, 1+randint(3));
    break;

  case 3:
    if (randint(3) == 1) {
      cave_set(yval-1, xval-2, CAVE_fval, TMP1_WALL);
      cave_set(yval+1, xval-2, CAVE_fval, TMP1_WALL);
      cave_set(yval-1, xval+2, CAVE_fval, TMP1_WALL);
      cave_set(yval+1, xval+2, CAVE_fval, TMP1_WALL);
      cave_set(yval-2, xval-1, CAVE_fval, TMP1_WALL);
      cave_set(yval-2, xval+1, CAVE_fval, TMP1_WALL);
      cave_set(yval+2, xval-1, CAVE_fval, TMP1_WALL);
      cave_set(yval+2, xval+1, CAVE_fval, TMP1_WALL);
      if (randint(3) == 1) {
	place_secret_door(yval, xval-2);
	place_secret_door(yval, xval+2);
	place_secret_door(yval-2, xval);
	place_secret_door(yval+2, xval);
      }
    } else if (randint(3) == 1) {
      cave_set(yval,   xval,   CAVE_fval, TMP1_WALL);
      cave_set(yval-1, xval,   CAVE_fval, TMP1_WALL);
      cave_set(yval+1, xval,   CAVE_fval, TMP1_WALL);
      cave_set(yval,   xval-1, CAVE_fval, TMP1_WALL);
      cave_set(yval,   xval+1, CAVE_fval, TMP1_WALL);
    } else if (randint(3) == 1)
      cave_set(yval,   xval,   CAVE_fval, TMP1_WALL);
    break;

  case 4:
    break;
  }
}




/* Always picks a correct direction		*/
// Whatever that means.
static void correct_dir(Short *rdir, Short *cdir,
			Short y1, Short x1, Short y2, Short x2)
{
  if (y1 < y2)        *rdir =  1;
  else if (y1 == y2)  *rdir =  0;
  else                *rdir = -1;

  if (x1 < x2)        *cdir =  1;
  else if (x1 == x2)  *cdir =  0;
  else                *cdir = -1;

  if ((*rdir != 0) && (*cdir != 0)) {
    if (randint(2) == 1)    *rdir = 0;
    else                    *cdir = 0;
  }
}


/* Chance of wandering direction			*/
static void rand_dir(Short *rdir, Short *cdir)
{
  Short tmp;

  tmp = randint(4);
  if (tmp < 3) {
    *cdir = 0;
    *rdir = -3 + (tmp << 1); /* tmp=1 -> *rdir=-1; tmp=2 -> *rdir=1 */
  } else {
    *rdir = 0;
    *cdir = -7 + (tmp << 1); /* tmp=3 -> *cdir=-1; tmp=4 -> *cdir=1 */
  }
}



/* Constructs a tunnel between two points		*/
coords tunstk[1000], wallstk[1000]; // better not put those on the stack!!
static void build_tunnel(Short row1, Short col1, Short row2, Short col2)
{
  Short tmp_row, tmp_col, i, j;
  //  cave_type *c_ptr;
  //  cave_type *d_ptr;
  UChar floor_c, floor_d;
  coords *tun_ptr;
  Short row_dir, col_dir, tunindex, wallindex, main_loop_count;
  Boolean stop_flag, door_flag;
  Short  start_row, start_col;
  //  StrCopy(moron_caller, "build_tunnel");

  /* Main procedure for Tunnel			*/
  /* Note: 9 is a temporary value		*/
  stop_flag = false;
  door_flag = false;
  tunindex    = 0;
  wallindex   = 0;
  main_loop_count = 0;
  start_row = row1;
  start_col = col1;
  correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

  while (!stop_flag && (row1 != row2 || col1 != col2)) {
    /* prevent infinite loops, just in case */
    main_loop_count++;
    if (main_loop_count > 2000)
      break;

    if (randint(100) > DUN_TUN_CHG) {
      if (randint(DUN_TUN_RND) == 1)
	rand_dir(&row_dir, &col_dir);
      else
	correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);
    }
    tmp_row = row1 + row_dir;
    tmp_col = col1 + col_dir;
    while (!in_bounds(tmp_row, tmp_col)) {
      if (randint(DUN_TUN_RND) == 1)
	rand_dir(&row_dir, &col_dir);
      else
	correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);
      tmp_row = row1 + row_dir;
      tmp_col = col1 + col_dir;
    }
    // c_ptr = &cave[tmp_row][tmp_col];
    floor_c = cave_fval[tmp_row*MAX_WIDTH + tmp_col];
    if (floor_c == NULL_WALL) {
      row1 = tmp_row;
      col1 = tmp_col;
      if (tunindex < 1000) {
	tunstk[tunindex].y = row1;
	tunstk[tunindex].x = col1;
	tunindex++;
      }
      door_flag = false;
    }
    else if (floor_c == TMP2_WALL)
      /* do nothing */
	;
    else if (floor_c == GRANITE_WALL) {
      row1 = tmp_row;
      col1 = tmp_col;
      if (wallindex < 1000) {
	wallstk[wallindex].y = row1;
	wallstk[wallindex].x = col1;
	wallindex++;
      }
      for (i = row1-1; i <= row1+1; i++)
	for (j = col1-1; j <= col1+1; j++)
	  if (in_bounds(i, j)) {
	    //	    d_ptr = &cave[i][j];
	    floor_d = cave_fval[i*MAX_WIDTH + j];
	    /* values 11 and 12 are impossible here, place_streamer
	       is never run before build_tunnel */
	    if (floor_d == GRANITE_WALL)
	      //	      d_ptr->fval = TMP2_WALL;
	      cave_set(i, j, CAVE_fval, TMP2_WALL);

	  }
    }
    else if (floor_c == CORR_FLOOR || floor_c == BLOCKED_FLOOR) {
      row1 = tmp_row;
      col1 = tmp_col;
      if (!door_flag) {
	if (doorindex < 100) {
	  doorstk[doorindex].y = row1;
	  doorstk[doorindex].x = col1;
	  doorindex++;
	}
	door_flag = true;
      }
      if (randint(100) > DUN_TUN_CON) {
	/* make sure that tunnel has gone a reasonable distance
	   before stopping it, this helps prevent isolated rooms */
	tmp_row = row1 - start_row;
	if (tmp_row < 0) tmp_row = -tmp_row;
	tmp_col = col1 - start_col;
	if (tmp_col < 0) tmp_col = -tmp_col;
	if (tmp_row > 10 || tmp_col > 10)
	  stop_flag = true;
      }
    }
    else  /* floor_c != NULL, TMP2, GRANITE, CORR */
      {
	row1 = tmp_row;
	col1 = tmp_col;
      }
  } // end WHILE.


  tun_ptr = &tunstk[0];
  for (i = 0; i < tunindex; i++) {
    //    d_ptr = &cave[tun_ptr->y][tun_ptr->x];
    //    floor_d = cave_fval[tun_ptr->y*MAX_WIDTH + tun_ptr->x]; // unneeded
    //    d_ptr->fval  = CORR_FLOOR;
    cave_set(tun_ptr->y, tun_ptr->x, CAVE_fval, CORR_FLOOR);
    tun_ptr++;
  }
  for (i = 0; i < wallindex; i++) {
    //    c_ptr = &cave[wallstk[i].y][wallstk[i].x];
    tmp_row = wallstk[i].y;
    tmp_col = wallstk[i].x;
    floor_c = cave_fval[tmp_row*MAX_WIDTH + tmp_col];
    if (floor_c == TMP2_WALL) {
      if (randint(100) < DUN_TUN_PEN)
	place_door(wallstk[i].y, wallstk[i].x);
      else {
	/* these have to be doorways to rooms */
	cave_set(tmp_row, tmp_col, CAVE_fval, CORR_FLOOR);
      }
    }
  }
}




/* Places "streamers" of rock through dungeon		-RAK-	*/
static void place_streamer(Short fval, Short treas_chance)
{
  Short i, tx, ty;
  Short y, x, t1, t2, dir;
  //  cave_type *c_ptr;
  UChar floor;
  //  StrCopy(moron_caller, "place_streamer");

  /* Choose starting point and direction		*/
  y = (cur_height / 2) + 11 - randint(23);
  x = (cur_width / 2)  + 16 - randint(33);

  dir = randint(8);	/* Number 1-4, 6-9	*/
  if (dir > 4) dir++;

  /* Place streamer into dungeon			*/
  t1 = 2*DUN_STR_RNG + 1;	/* Constants	*/
  t2 =	 DUN_STR_RNG + 1;
  do {
    for (i = 0; i < DUN_STR_DEN; i++) {
      ty = y + randint(t1) - t2;
      tx = x + randint(t1) - t2;
      if (in_bounds(ty, tx)) {
	floor = cave_fval[ty*MAX_WIDTH+tx];
	if (floor == GRANITE_WALL) {
	  cave_set(ty, tx, CAVE_fval, fval);
	  if (randint(treas_chance) == 1)
	    place_gold(ty, tx);
	}
      }
    }
  } while (mmove(dir, &y, &x));
}




// this was in misc1.c, but it could be here and be static.
/* Checks all adjacent spots for corridors		-RAK-	*/
/* note that y, x is always in_bounds(), hence no need to check that
   j, k are in_bounds(), even if they are 0 or cur_x-1 is still works */
static Short next_to_corr(Short y, Short x)
{
  Short k, j, i;
  Short c_ptr;

  i = 0;
  for (j = y - 1; j <= (y + 1); j++)
    for (k = x - 1; k <= (x + 1); k++) {
      c_ptr = j*MAX_WIDTH+k;
      /* should fail if there is already a door present */
      if (cave_fval[c_ptr] == CORR_FLOOR
	  && (cave_tptr[c_ptr] == 0
	      || t_list[cave_tptr[c_ptr]].tval < TV_MIN_DOORS ))
	i++;
    }
  return i;
}


static Boolean next_to(Short y, Short x)
{
  if (next_to_corr(y, x) > 2)
    if ((cave_fval[(y-1)*MAX_WIDTH+x] >= MIN_CAVE_WALL)
	&& (cave_fval[(y+1)*MAX_WIDTH+x] >= MIN_CAVE_WALL))
      return true;
    else if ((cave_fval[y*MAX_WIDTH+x-1] >= MIN_CAVE_WALL)
	     && (cave_fval[y*MAX_WIDTH+x+1] >= MIN_CAVE_WALL))
      return true;
    else
      return false;
  else
    return false;
}

/* Places door at y, x position if at least 2 walls found	*/
static void try_door(Short y, Short x)
{
  if ((cave_fval[y*MAX_WIDTH+x] == CORR_FLOOR) && (randint(100) > DUN_TUN_JCT)
      && next_to(y, x))
    place_door(y, x);
}



static void place_open_door(Short y, Short x)
{
  Short cur_pos;

  cur_pos = popt();
  cave_set(y, x, CAVE_tptr, cur_pos);
  invcopy(t_list, cur_pos, OBJ_OPEN_DOOR);
  cave_set(y, x, CAVE_fval, CORR_FLOOR);
}


static void place_broken_door(Short y, Short x)
{
  Short cur_pos;

  cur_pos = popt();
  cave_set(y, x, CAVE_tptr, cur_pos);
  invcopy(t_list, cur_pos, OBJ_OPEN_DOOR);
  cave_set(y, x, CAVE_fval, CORR_FLOOR);
  tlist_set_p1(t_list, cur_pos, 1);
}


static void place_closed_door(Short y, Short x)
{
  Short cur_pos;

  cur_pos = popt();
  cave_set(y, x, CAVE_tptr, cur_pos);
  invcopy(t_list, cur_pos, OBJ_CLOSED_DOOR);
  cave_set(y, x, CAVE_fval, BLOCKED_FLOOR);
}


static void place_locked_door(Short y, Short x)
{
  Short cur_pos;

  cur_pos = popt();
  cave_set(y, x, CAVE_tptr, cur_pos);
  invcopy(t_list, cur_pos, OBJ_CLOSED_DOOR);
  cave_set(y, x, CAVE_fval, BLOCKED_FLOOR);
  tlist_set_p1(t_list, cur_pos, randint(10) + 10);
}


static void place_stuck_door(Short y, Short x)
{
  Short cur_pos;

  cur_pos = popt();
  cave_set(y, x, CAVE_tptr, cur_pos);
  invcopy(t_list, cur_pos, OBJ_CLOSED_DOOR);
  cave_set(y, x, CAVE_fval, BLOCKED_FLOOR);
  tlist_set_p1(t_list, cur_pos, 0-(randint(10) + 10));
}


static void place_secret_door(Short y, Short x)
{
  Short cur_pos;

  cur_pos = popt();
  cave_set(y, x, CAVE_tptr, cur_pos);
  invcopy(t_list, cur_pos, OBJ_SECRET_DOOR);
  cave_set(y, x, CAVE_fval, BLOCKED_FLOOR);
}


static void place_door(Short y, Short x)
{
  Short tmp;
  //  StrCopy(moron_caller, "place_*_door");

  tmp = randint(3);
  if (tmp == 1) {
    if (randint(4) == 1)
      place_broken_door(y, x);
    else
      place_open_door(y, x);
  } else if (tmp == 2) {
    tmp = randint(12);
    if (tmp > 3)
      place_closed_door(y, x);
    else if (tmp == 3)
      place_stuck_door(y, x);
    else
      place_locked_door(y, x);
  } else
    place_secret_door(y, x);
}
