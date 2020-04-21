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


// Working on first pass through this... !

/* Add a comment to an object description.		-CJS- */
// This should be handled in a form!
void scribe_object()
{
  Short item_val, j;
  Char out_val[80], tmp_str[80];

  if (inven_ctr > 0 || equip_ctr > 0) {
    if (get_item(&item_val, "Which one? ", 0, INVEN_ARRAY_SIZE, CNIL, CNIL)) {
      objdes(tmp_str, &inventory[item_val], true);
      StrPrintF(out_val, "Inscribing %s", tmp_str);
      message(out_val);
      if (inventory[item_val].inscrip[0] != '\0')
	StrPrintF(out_val, "Replace %s New inscription:",
		  inventory[item_val].inscrip);
      else
	StrCopy(out_val, "Inscription: ");
      j = 78 - StrLen(tmp_str);
      if (j > 12)
	j = 12;
      ;      //      prt(out_val, 0, 0); // xxx not implemented yet
      if(0)//      if (get_string(out_val, 0, (Short) StrLen(out_val), j)) // xxx not implemented yet
	inscribe(item_val, out_val);
    }
  }
  else
    message("You are not carrying anything to inscribe.");
}

/* Append an additional comment to an object description.	-CJS- */
void add_inscribe(inven_type *i_ptr, Short index, UChar type)
{
  invy_set_ident(i_ptr, index, i_ptr[index].ident | type);
}

/* Replace any existing comment in an object description with a new one. CJS*/
void inscribe(Short iptr, Char *str)
{
  if (StrLen(str) >= INSCRIP_SIZE)
    str[INSCRIP_SIZE-1] = '\0'; // truncate!
  DmStrCopy(inventory, iptr*sizeof(inven_type)+I_inscrip, str);
}


/* We need to reset the view of things.			-CJS- */
void check_view()
{
  Short i, j, c_ptr, d_ptr;;

  c_ptr = char_row*MAX_WIDTH+char_col;
  /* Check for new panel		   */
  if (get_panel(char_row, char_col, false)) // so does this, like, work?? xxxx
    recalc_screen();//    prt_map(); // xxx not implemented yet
    //    refresh();//    prt_map(); // xxx not implemented yet
  /* Move the light source		   */
  move_light(char_row, char_col, char_row, char_col);
  /* A room of light should be lit.	 */
  if (cave_fval[c_ptr] == LIGHT_FLOOR) {
    if ((pyflags.blind < 1) && !(cave_light[c_ptr] & LIGHT_pl))
      light_room(char_row, char_col);
  }
  /* In doorway of light-room?		   */
  else if ((cave_light[c_ptr] & LIGHT_lr) && (pyflags.blind < 1)) {
    for (i = (char_row - 1); i <= (char_row + 1); i++)
      for (j = (char_col - 1); j <= (char_col + 1); j++) {
	d_ptr = i*MAX_WIDTH+j;
	if ((cave_fval[d_ptr] == LIGHT_FLOOR)
	    && !(cave_light[d_ptr] & LIGHT_pl))
	  light_room(i, j);
      }
  }
}
