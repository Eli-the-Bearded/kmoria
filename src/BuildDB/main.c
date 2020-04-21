#include <Pilot.h>
//#include <Globals.h>
#include "kMoriaBuildRsc.h"
#include "kMoria.h"

DmOpenRef MoriaDB;
static Boolean OpenDatabase();
/*****************************************************************************
 *                                                                           *
 *                      RebuildDatabase                                      *
 *                                                                           *
 *****************************************************************************/
// returns false on error, true otherwise.
void newrec_objectlist();
void newrec_creaturelist();
void newrec_sortedobjects();
void newrec_tlevel();
void newrec_mlevel();
void newrec_haggle_texts();
static Boolean RebuildDatabase()
{
  // open the database
  if (!OpenDatabase())
    return false;

  // CHANGE THIS PART WHEN YOU CHANGE WHAT OBJECTS ARE COMPILED IN.
  // 1.
  //  newrec_objectlist(); // create the object_list
  // 2.
  //  newrec_creaturelist(); // create the c_list
  // 3.
  //  newrec_tlevel();
  //  newrec_sortedobjects();
  //  newrec_mlevel(); // create the t_level, sorted_objects, and m_level. one shot
  // 4.
  newrec_haggle_texts();
  // END OF CHANGE THIS PART

  if (MoriaDB != NULL) {
    DmCloseDatabase(MoriaDB);
    MoriaDB = NULL;
  }
  return true;
}
/*****************************************************************************
 *                     little event handler                                  *
 *****************************************************************************/

Boolean Main_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
    
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
    }
    break;

  default:
    break;
  }

  return handled;
}
/*****************************************************************************
 *                                                                           *
 *                      OpenDatabase                                         *
 *                                                                           *
 *****************************************************************************/
// returns false on error, true otherwise.
static Boolean OpenDatabase()
{
  LocalID dbID;

  dbID = DmFindDatabase(0, MoriaDBName); // card#, name
  if (!dbID) {
    // create database IFF it does not exist
    if (DmCreateDatabase(0, MoriaDBName, MoriaAppID, MoriaDBType, false))
      return false;
    dbID = DmFindDatabase(0, MoriaDBName); // card#, name
    if (!dbID) return false;
  }
  MoriaDB = DmOpenDatabase(0, dbID, dmModeReadWrite);
  return true;
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

