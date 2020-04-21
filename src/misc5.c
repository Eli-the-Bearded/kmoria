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

// This is stuff related to obtaining a DIRECTION from the player.

Short needed_direction_for = DIR_NONE;
Short needed_direction_item;

extern Boolean stats_dirty;
void draw_directional()
{
  //  XOR some lines onto the screen
  // centered around 80,80 if navigation is screen-relative,
  // or around the @'s position if navigation is position-relative

  // This is from iLarn.. assumes screen-relative
  WinInvertLine(0,  40, 60, 70);
  WinInvertLine(100,90,160,120);
  WinInvertLine(0,  120,   60, 90);
  WinInvertLine(100, 70,  160, 40);
  WinInvertLine(40,  0, 70, 60);
  WinInvertLine(90,100,120,159);
  WinInvertLine(120,  0,90, 60);
  WinInvertLine(70, 100,40,159);
  stats_dirty = false;
}
void undraw_directional()
{
  draw_directional(); //  XOR the lines again.. hopefully nothing has changed!
  stats_dirty = true;
}

//#define abs(a)                                  (((a) >= 0) ? (a) : (-(a)))
// ignore_center is the 'radius' of a central square that is 'ignored'
Short convert_to_dir(Short x, Short y, Short ignore_center)
{
  //    x = e->screenX - 80;
  //    y = e->screenY - 80;
  // x and y each range from -80 to +80
  // -80,-80 is the top left corner of the screen

  // almost in the center?  heck, let's make it a real (circle) radius
  if (x*x + y*y < ignore_center*ignore_center)
    return 5; // the NOOP direction
  // mostly x?
  if ((abs(x)) > 2 * (abs(y)))
    return ( (x > 0) ? 6 : 4 ); // east, west
  // mostly y?
  if ((abs(y)) > 2 * (abs(x)))
    return ( (y > 0) ? 2 : 8 ); // south, north
  // diagonal?  (x ==approx== y)
  if (y > 0)      // southish
    return ( (x > 0) ? 3 : 1 ); // se, sw
  else            // northish
    return ( (x > 0) ? 9 : 7 ); // ne, nw
}

/*
extern Char last_command;
void dispatch_directional(Short dir)
{
  if (dir == 5 && last_command != '?') return;
  switch(needed_direction_for) {
  case DIR_NONE: return;
  case DIR_AIM:
    aim(needed_direction_item, dir);
    break;
  case DIR_BASH:
    bash(dir);
    break;
  case DIR_CLOSE:
    closeobject(dir);
    break;
  case DIR_DISARM:
    disarm_trap(dir);
    break;
  case DIR_LOOK:
    look(dir);
    free_turn_flag = true;
    break;
  case DIR_NOPICK: // move w/o picking up objects
    // xxx not implemented yet
    break;
  case DIR_OPEN:
    openobject(dir);
    break;
  case DIR_THROW:
    throw_object(needed_direction_item, dir);
    break;
  case DIR_TUNNEL:
    tunnel(dir);
    break;
  case DIR_WEDGE:
    // jamdoor(dir); // xxx not implemented yet
    break;
  case DIR_SPELLS:
    // xxx not distinguished from one another yet
    break;
  }
  needed_direction_for = DIR_NONE;
}
*/
