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
/*
  S00 I00 W00 D00 C00 Ch00  0000 feet
  L 00 xp 0123456 mana 000 hp 000/000
  AC 00 Gold ...
*/
#include <Pilot.h>

/*
 * Print a one-line message at top of screen
 */
void message(Char *buf)
{
  RectangleType r;

  RctSetRectangle(&r,0,0,160,13); /* left,top, width and height */
  WinEraseRectangle(&r, 0); /* 0 for square corners */
  if (buf) {
    WinDrawLine(0,12,160,12);
    WinDrawChars(buf, StrLen(buf), 1, 0);
  }
  // else... have the screen-drawer draw the top line!!!  yeah.
}


/*
 * Print two lines of status at the bottom of screen
 */
void print_stats(Boolean siwdcc)
{
  RectangleType r;
  //  Char buf[20];
  Char buf[80];
  // Short w, hi, x;
  Short hi, x;

  if (false) return; // check whether we're in the main screen

  // Tap bottom line of screen to toggle what the first line says
  
  RctSetRectangle(&r,0,138,160,160-138); /* left,top, width and height */
  WinEraseRectangle(&r, 0); /* 0 for square corners */

  WinDrawLine(0,137,160,137);
  hi = 138;

  x = 1;
  if (siwdcc) {
    StrPrintF(buf, "S%d In%d W%d D%d C%d Ch%d", 18, 17, 16, 15, 14, 13);
    WinDrawChars(buf, StrLen(buf), x, hi);
  } else {
    StrPrintF(buf, "AC %d  $ %d", 18, 17);
    WinDrawChars(buf, StrLen(buf), x, hi);
  }

  x += 115;
  StrPrintF(buf, "%d feet", 1000); // 50 * dun_level
  WinDrawChars(buf, StrLen(buf), x, hi);
  // done with first line
  x = 1;
  hi += 11;

  // player level 12
  StrPrintF(buf, "L %d", 20);
  WinDrawChars(buf, StrLen(buf), x, hi);
  x += 20;
  // player experience 1234567
  StrPrintF(buf, "x 0123456");
  WinDrawChars(buf, StrLen(buf), x, hi);
  x += 45;
  // mana points 123
  StrPrintF(buf, "mana 012");
  WinDrawChars(buf, StrLen(buf), x, hi);
  x += 43;
  // hit points 123/123
  StrPrintF(buf, "hp 123/123");
  WinDrawChars(buf, StrLen(buf), x, hi);
  return;
}


