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


/***********************************************************************
 * This form allows player to view the last N message that went by..
 * useful if he/she acted too fast and missed one.
 * Note that the old_messages array is probably going to be combined with
 * the splash array and a position-in-array int to enable "--more--"
 ***********************************************************************/

extern Char *old_messages[SAVED_MSGS];
VoidHand msglog_TextHandle = NULL;
static void init_messagelog_view(FormPtr frm) SEC_5;
Boolean MsgLog_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  FieldPtr fld;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    init_messagelog_view(frm);
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_ml));
    update_field_scrollers(frm, fld, repeat_ml_up, repeat_ml_down);
    FrmDrawForm(frm);
    handled = true;
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_ml_ok:
      LeaveForm(); // LeaveForm(true);
      handled = true;
      break;
    }
    break;

  case ctlRepeatEvent:
    /*     "Repeating controls don't repeat if handled is set true." */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_ml));
    switch(e->data.ctlRepeat.controlID) {
    case repeat_ml_up:
      FldScrollField(fld, 1, up);
      update_field_scrollers(frm, fld, repeat_ml_up, repeat_ml_down);
      break;
    case repeat_ml_down:
      FldScrollField(fld, 1, down);
      update_field_scrollers(frm, fld, repeat_ml_up, repeat_ml_down);
      break;
    }
    break;

  case keyDownEvent:
    /* hardware button -- or else graffiti. */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_ml));
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      page_scroll_field(frm, fld, up);
      update_field_scrollers(frm, fld, repeat_ml_up, repeat_ml_down);
      handled = true;
      break;
    case pageDownChr:
      page_scroll_field(frm, fld, down);
      update_field_scrollers(frm, fld, repeat_ml_up, repeat_ml_down);
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}



/**********************************************************************
                       INIT_MESSAGELOG_VIEW
 IN: frm = a form that has a text-view widget
 PURPOSE:
 Initializes the displayed form 'frm' with a list of the last
 SAVED_MSGS messages.  Called when frm is being initialized.
 **********************************************************************/
static void init_messagelog_view(FormPtr frm)
{
  FieldPtr fld;
  CharPtr txtP;
  Short i;
  // Get the text field
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_ml));
  // Create a mem. handle and lock it
  msglog_TextHandle = MemHandleNew(BIG_NUMBER);
  txtP = MemHandleLock(msglog_TextHandle);
  // Use MemMove and/or MemSet to copy text to the mem. handle
  for (i = 0 ; i < SAVED_MSGS ; i++) {
    MemMove(txtP, old_messages[i], StrLen(old_messages[i])+1);
    txtP += StrLen(old_messages[i]);
  }
  // Unlock the handle.  Set the field to display the handle's text.
  MemHandleUnlock(msglog_TextHandle);
  FldSetTextHandle(fld, (Handle) msglog_TextHandle);
}
