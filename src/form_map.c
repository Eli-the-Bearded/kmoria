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
#include "kMoria.h"
#include "lock-externs.h"
#include "kMoriaRsc.h"

static void screen_map() SEC_5;

Boolean Map_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);
    /* this must be AFTER draw form */
    screen_map();
    handled = true;
    break;

  case penDownEvent:
    LeaveForm();
    handled = true;
    break;

  default:
    break;
  }

  return handled;
}


/*
  The dungeon is, at maximum, 66 high by 198 wide.

  The screen is 160 by 160.  In tiny font chars, it's 160/6 high by 160/5 wide.
  that's 26 high by 32 wide.  so if I use characters it's
  3 cells high per character and *7* cells wide.  gack.

  so, width-wise, we must represent at least 5 cells in each 4 pixels.
  if we try to keep the same proportions as in the dungeon,
  what do we get?  h:w = 5:4 or 6:5.  so, 3 cells per 2 pixels (6 in 4).

  in a 4 x 4 block of pixels,  we are representing 6 cells high by 5 cells wide
  that's 11 4x4 blocks of height or a mere 44 pixels, 1/4 of the screen. hmmm
  and 39.6 ~= 40 blocks of width or the total 160 pixels. hmmm

.... .... .... .... .... ....
***. ***. ..*. *... .... *...
***. *.*. .*.. .*.. .... *...
***. *.*. *... ..*. .... .*..
***. ***. .*.. .*.. .*.. ..*.



+------------------------------------------------------------------+
|>########4############                                            |
|#..#2###.###...###...#                                            |
|#..#####.###...###...#                                            |
|#..............5##..@#                                            |
|#..####..6##...#1#..p#                                            |
|#..####..###...###...#                                            |
|#..###3........###...#                                            |
|######################                                            |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
+----------------------Hit any key to continue---------------------+

             ##################################################################
Human        #................................................................#
Paladin      #>.........................4######...............................#
Gallant      #..........##2########.....#######...........#########...........#
             #..........###########.....#######...........#########...........#
STR :     18 #..........###########.....#######...........#########...........#
INT :      5 #..........###########.....#######...........#########...........#
WIS :     18 #..........................#######...........#########...........#
DEX :     11 #..........................#######...........#########...........#
CON :     13 #............................................5########...........#
CHR :     17 #.............................................................@..#
             #................................................................#
LEV :      1 #............................................#####1##........p...#
EXP :      0 #..........##########........6####...........########............#
MANA:      2 #..........##########........#####...........########............#
MHP :     16 #..........##########........#####...........########............#
CHP :     16 #..........##########........#####...........########............#
             #..........##########........#####...........########............#
AC  :      0 #..........########3#........................########............#
GOLD:    459 #................................................................#
             #................................................................#
             ##################################################################
                                                                 Town level

+------------------------------------------------------------------+
|                                                                  |
|              ###########                                         |
|              ####'######                                         |
|               ####    #######                                    |
|            #########  #########                                  |
|            #.......#     '....#######<                           |
|            ##s###+##     #############>##                  #     |
|                                     ######                 #     |
|                                     #....# ##          ######    |
|                                     #....# ###         '##~.#    |
|                                     #s########         ## ###    |
|                                    #########           ####@#    |
|                                    '...##              ##...#    |
|                                    ######           #########    |
|                                      #              ###:######   |
|                                      #        #####   ###'..'#   |
|                                      '#########...###### #..##   |
|                                               ########   ###'#   |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
+----------------------Hit any key to continue---------------------+



*/

/* definitions used by screen_map() */
/* index into border character array */
#define TL 0	/* top left */
#define TR 1
#define BL 2
#define BR 3
#define HE 4	/* horizontal edge */
#define VE 5
#   define CH(x)	(screen_border[x])
  /* Display highest priority object in the RATIO_W by RATIO_H area */
#define	RATIO_W 7
#define	RATIO_H 3
// maybe we can make RATIO_W 6 by squeezing all the win_draw_char's closer.
// (or at least n of the ones at the left and right border?)
// or I could draw the map in two sections.  blah.


static void screen_map()
{
  Short i, j, vc_h = 6, x=9, y; // set x so that the map is centered horizntly
  //  UChar screen_border[6] = {
  //    '+', '+', '+', '+', '-', '|'	/* screen border chars */
  //  };
  UChar map[MAX_WIDTH / RATIO_W + 1];
  UChar tmp;
  //  Short priority[256];
  Char priority[256];
  Short row, orow, col; // , myrow, mycol = 0;
  //  Short prntscrnbuf[80];

#ifdef I_AM_OS_2
  FntSetFont(ledFont);
#else
  FntSetFont(129);
#endif

  for (i = 0; i < 256; i++)
    priority[i] = 0;
  priority[' '] = -15;
  priority['.'] = -10;
  priority['\''] = -3;
  priority['#'] = -5;
  priority['<'] = 5;
  priority['>'] = 5;
  priority['@'] = 10;

  orow = -1;
  map[MAX_WIDTH / RATIO_W] = '\0';
  for (i = 0; i < MAX_HEIGHT; i++) {
    row = i / RATIO_H;
    if (row != orow) {
      if (orow >= 0) {
	y = (orow + 1) * vc_h;
	WinDrawChars(map, StrLen(map), x, y);
	//	StrPrintF(prntscrnbuf,"%s", map);
	//	mvaddstr(orow+1, 0, prntscrnbuf);
      }
      for (j = 0 ; j < MAX_WIDTH / RATIO_W ; j++)
	map[j] = ' ';
      orow = row;
    }
    for (j = 0; j < MAX_WIDTH; j++) {
      col = j / RATIO_W;
      tmp = loc_symbol(i, j);
      if (priority[map[col]] < priority[tmp])
	map[col] = tmp;
      if (map[col] == '@') {
	//	mycol = col + 1; /* account for border */
	//	myrow = row + 1;
      }
    }
  }
  if (orow >= 0) {
    y = (orow + 1) * vc_h;
    WinDrawChars(map, StrLen(map), x, y);
    //    StrPrintF(prntscrnbuf,"%s", map);
    //    mvaddstr(orow+1, 0, prntscrnbuf);
  }

#ifdef I_AM_OS_2
  FntSetFont(stdFont);
#else
  FntSetFont(stdFont);
#endif

}

