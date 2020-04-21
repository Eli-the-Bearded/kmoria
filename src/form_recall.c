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
#include "kMoriaRsc.h"

// this will have to be an extern so we know which monster to look up:
Short recall_this_monster = 0;

/**********************************************************************
                       INIT_RECALL_FORM
 IN:
 fld = the text field
 assumes 'recall_this_monster' is in bounds
 PURPOSE:
 Initialize the recall form which will display information known about
 a particular monster type
 **********************************************************************/
VoidHand recall_handle = NULL;

void init_recall_form(FormPtr frm, FieldPtr fld) {
  CharPtr txtP;
  //  Char buf[40];
  /*  VoidHand recall_handle = NULL; */

  FrmCopyTitle(frm, c_names + c_list[recall_this_monster].name_offset);

  if (recall_handle != NULL) {
    FldSetTextHandle(fld, (Handle) 0);
    /* the following line is a fine way to crash the palmpilot,
       whenever someone re-visits this form */
    // MemHandleFree(recall_handle);
  }
  recall_handle = MemHandleNew(2700); // this SHOULD be enough, but who knows
  txtP = MemHandleLock(recall_handle);
  txtP[0] = '\0';
  roff_recall(recall_this_monster, txtP); // print info to this text buffer
  MemHandleUnlock(recall_handle);
  FldSetTextHandle(fld, (Handle) recall_handle);

  //  StrPrintF(buf, "%s:", c_names + c_list[recall_this_monster].name_offset);
  //  FrmCopyTitle(frm, buf);
}

/**********************************************************************
                       INIT_RECALL_FORM

 This form will display information (gathered by the current character
 and dead predecessors) about a particular monster type.
 Before you jump to this form, you should set recall_this_monster to
 the index of the desired monster type!
 The form is just a big boring text field that can be scrolled.
 Assumes 'recall_this_monster' is in bounds
 **********************************************************************/
Boolean Recall_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  FieldPtr fld;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_recall));
    init_recall_form(frm, fld);
    // There's a bug in this
    update_field_scrollers(frm, fld, repeat_recall_up, repeat_recall_down);
    FrmDrawForm(frm);
    handled = true;
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_recall_ok:
      LeaveForm();
      handled = true;
      break;
    }
    break;

  case ctlRepeatEvent:
    /*     "Repeating controls don't repeat if handled is set true." */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_recall));
    switch(e->data.ctlRepeat.controlID) {
    case repeat_recall_up:
      FldScrollField(fld, 1, up);
      update_field_scrollers(frm, fld, repeat_recall_up, repeat_recall_down);
      break;
    case repeat_recall_down:
      FldScrollField(fld, 1, down);
      update_field_scrollers(frm, fld, repeat_recall_up, repeat_recall_down);
      break;
    }
    break;

  case keyDownEvent:
    /* hardware button -- or else graffiti. */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_recall));
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      page_scroll_field(frm, fld, up);
      update_field_scrollers(frm, fld, repeat_recall_up, repeat_recall_down);
      handled = true;
      break;
    case pageDownChr:
      page_scroll_field(frm, fld, down);
      update_field_scrollers(frm, fld, repeat_recall_up, repeat_recall_down);
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}
