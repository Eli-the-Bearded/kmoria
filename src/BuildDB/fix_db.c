/*********************************************************************
 * iLarn - Ularn adapted for the PalmPilot.                          *
 * Copyright (C) 2000 Bridget Spitznagel                             *
 * iLarn is derived from Ularn by Phil Cordier                       *
 * which in turn was based on Larn by Noah Morgan                    *
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
//#include <Globals.h>
#include "iLarnFixDBRsc.h"
#include "fix_db.h"

void DrawSerialNumOrMessage(int x, int y, CharPtr noNumberMessage);
Boolean Main_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Char buf[30] = "no rom";
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);
    handled = true;
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_ok:
      if (0 == FrmAlert(NukeP))
	RebuildDatabase(); // if it returns false, pop up an error message
      handled = true;
      break;
    case btn_rom:
      DrawSerialNumOrMessage(5, 130, buf);
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}


/*****************************************************************************
 *                      ApplicationHandleEvent                               *
 *****************************************************************************/
static Boolean ApplicationHandleEvent(EventPtr e)
{
    FormPtr frm;
    Word    formId;
    Boolean handled = false;

    if (e->eType == frmLoadEvent) {
	formId = e->data.frmLoad.formID;
	frm = FrmInitForm(formId);
	FrmSetActiveForm(frm);

	switch(formId) {
	case MainForm:
	    FrmSetEventHandler(frm, Main_Form_HandleEvent);
	    break;
	}
	handled = true;
    }

    return handled;
}


/*****************************************************************************
 *                      StartApplication                                     *
 *****************************************************************************/
/* moved to the head of the class */
static Word StartApplication(void)
{
  Word error;
  //  error = OpenDatabase();
  //  if (error) return error;
  /*   error = checkAppInfo(); */
  /*   if (error) return error; */

  FrmGotoForm(MainForm);
  return 0;
}


/*****************************************************************************
 *                      StopApplication                                      *
 *****************************************************************************/
static void StopApplication(void)
{
  FrmSaveAllForms();
  FrmCloseAllForms();

}


/*****************************************************************************
 *                      EventLoop                                            *
 *****************************************************************************/

/* The main event loop */


static void EventLoop(void)
{
    Word err;
    EventType e;
     
    do {
	EvtGetEvent(&e, evtWaitForever);
	  if (! SysHandleEvent (&e))
	    if (! MenuHandleEvent (NULL, &e, &err))
	      if (! ApplicationHandleEvent (&e))
		FrmDispatchEvent (&e);
    } while (e.eType != appStopEvent);
}


/* Main entry point; it is unlikely you will need to change this except to
   handle other launch command codes */
/*****************************************************************************
 *                      PilotMain                                            *
 *****************************************************************************/
DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags)
{
    Word err;

    if (cmd == sysAppLaunchCmdNormalLaunch) {

      err = StartApplication();
      if (err) return err;

      EventLoop();
      StopApplication();

    } else {
      return sysErrParamErr;
    }

    return 0;
}

