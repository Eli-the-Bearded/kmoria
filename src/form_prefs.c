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

extern MoriaPreferenceType my_prefs;

/***********************************************************************
 * This form lets you view and modify your preferences.
 * There are a bunch of checkboxes and things in it,
 * and an ok and a cancel button, as you might expect.
 ***********************************************************************/

Boolean Prefs_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  ControlPtr checkbox;
  Boolean dirty = false, val;
  static Short rw = 0;
  static Short ws = 0;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    rw = max(1, my_prefs.run_walk_border - my_prefs.walk_center_border);
    ws = max(1, my_prefs.walk_center_border);
    /* and checkboxes */
    // Big font.  Relative move.  Sound.  Run.
    // Cut.  Examine.  Stop.  Doors.  Pickup.  Weights.  Minerals.
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_1));
    CtlSetValue(checkbox, (my_prefs.big_font ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_2));
    CtlSetValue(checkbox, (my_prefs.relative_move ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_3));
    CtlSetValue(checkbox, (my_prefs.overlay_circles ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_4));
    CtlSetValue(checkbox, (my_prefs.sound ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_5));
    CtlSetValue(checkbox, (my_prefs.run ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_6));
    CtlSetValue(checkbox, (my_prefs.run_cut_corners ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_7));
    CtlSetValue(checkbox, (my_prefs.run_examine_corners ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_8));
    CtlSetValue(checkbox, (my_prefs.run_stop_sector ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_9));
    CtlSetValue(checkbox, (my_prefs.run_thru_doors ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_10));
    CtlSetValue(checkbox, (my_prefs.prompt_pickup ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_11));
    CtlSetValue(checkbox, (my_prefs.highlight_minerals ? 1 : 0));
    //    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_12));
    //    CtlSetValue(checkbox, (my_prefs.show_weights ? 1 : 0));

    FrmDrawForm(frm);
    /* set initial settings for lists */
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_prf_1));
    LstSetSelection(lst, ws-1);
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_prf_2));
    LstSetSelection(lst, rw-1);
    handled = true;
    break;

  case lstSelectEvent:
    switch (e->data.lstSelect.listID) {
    case list_prf_1:
      ws = max(1, 1 + e->data.lstSelect.selection);      /* "hole" */
      break;
    case list_prf_2:
      rw = max(1, 1 + e->data.lstSelect.selection);      /* "donut" */
      break;
    }
    handled = true;
    break;
  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_prf_ok:
      if (my_prefs.overlay_circles &&
	  (my_prefs.run_walk_border != rw+ws ||
	   my_prefs.walk_center_border != ws))
	dirty = true; // need to erase old circles
      my_prefs.run_walk_border = rw+ws;
      my_prefs.walk_center_border = ws;

      // Big font.  Relative move.  Sound.  Run.
      // Cut.  Examine.  Stop.  Doors.  Pickup.  Weights.  Minerals.

      // if I decide to actually toggle itsy, then I should set dirty=true.
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_1));
      my_prefs.big_font = (CtlGetValue(checkbox) != 0);

      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_2));
      val = (CtlGetValue(checkbox) != 0); // decide to erase old circles
      if (my_prefs.overlay_circles && val && !my_prefs.relative_move)
	dirty = true;
      my_prefs.relative_move = val;

      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_5));
      val = (CtlGetValue(checkbox) != 0); // decide to erase old 'run' circle
      if (my_prefs.overlay_circles && !val && my_prefs.run) dirty = true;
      my_prefs.run = val;

      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_3));
      val = (CtlGetValue(checkbox) != 0); // decide to draw or erase circles
      if (my_prefs.overlay_circles != val) dirty=true;
      my_prefs.overlay_circles = val;

      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_4));
      my_prefs.sound = (CtlGetValue(checkbox) != 0);

      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_6));
      my_prefs.run_cut_corners = (CtlGetValue(checkbox) != 0);
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_7));
      my_prefs.run_examine_corners = (CtlGetValue(checkbox) != 0);
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_8));
      my_prefs.run_stop_sector = (CtlGetValue(checkbox) != 0);
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_9));
      my_prefs.run_thru_doors = (CtlGetValue(checkbox) != 0);
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_10));
      my_prefs.prompt_pickup = (CtlGetValue(checkbox) != 0);

      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_11));
      val = (CtlGetValue(checkbox) != 0); // decide to draw or erase minerals?
      if (my_prefs.highlight_minerals != val) dirty = true;
      my_prefs.highlight_minerals = val;

      // checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_12));
      // my_prefs.show_weights = (CtlGetValue(checkbox) != 0);

      useMoriaPrefs(); // map most of these back to the right moria globals
      writeMoriaPrefs();
      LeaveForm();
      if (dirty) {
	get_panel(char_row, char_col, true); // FORCE recalculation of panel
	recalc_screen();
	refresh();
      }
      handled = true;
      break;
    case btn_prf_cancel:
      LeaveForm();
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}

