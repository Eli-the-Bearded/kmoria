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
#include <Graffiti.h> /* just for GrfGetState */
#include <CharAttr.h> /* for IsDigit */ // why is this not working for 3.1??
#include <System/Globals.h> // for font foo

#include "kMoriaRsc.h"
#include "kMoria.h"
#include "lock-externs.h"

DmOpenRef MoriaDB = NULL;
DmOpenRef MoriaSaveDB = NULL;


MoriaPreferenceType my_prefs = {
  6, // run_walk_border
  2, // walk_center_border
  // I may eliminate some of these.
  true,  // big font
  false, // rel move
  false, // circles
  true,  // sound
  true,  // run
  true,  // cut
  true,  // examine
  false, // sector
  false, // doors
  false, // prompt
  false, // weights
  false, // minerals
  // Hardware buttons
  false,
  { 0,0,0,0,0,0,0,0 },
  // Default character name
  "my name"
};

/*****************************************************************************
 *                                                                           *
 *                  thrilling event handlers                                 *
 *                                                                           *
 *****************************************************************************/
Boolean do_pickup = true;
extern Char last_command;
//Short command_count = 0;
static void game_init()
{

#if (COST_ADJ != 100)
  //  price_adjust();
#endif

  /* Grab a random seed from the clock		*/
  //  init_seeds(seed);

  /* Init monster and treasure levels for allocate */
  //  init_m_level();
  //  init_t_level();

  /* Init the store inventories			*/
  store_init();
  // also should call magic_init

  // do this if the old character dies, too.
  last_command = -1;
}

//Boolean stats_toggler = true;
//Short ctr; // this is for testing foo_list
//static Boolean do_command(Char c);
extern Boolean came_from_generation;
Boolean Main_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Short tmp_x, tmp_y, dir;
  //  Char buf[40];

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /** place any form-initialization stuff here **/
    FrmDrawForm(frm);
    print_stats(STATS_STR_ETC);
    // Make new level if needed, display level, display stats, yadda.
    generate_cave();
    enter_new_level();
    start_of_turn();
    // just to be really sure:
    get_panel(char_row, char_col, true);
    recalc_screen();
    refresh(); // Hey!  what's wrong here?
    show_messages(); // in case you can learn a spell.  But it's not working..
    //recalc_screen();// now incorporated in enter_new_level
    command_count = 0;
    return true;

  case menuEvent:
    //    message_clear();
    MenuEraseStatus(NULL); /* clear menu status from display */
    switch(e->data.menu.itemID) {
    case menu_mainMap:
      FrmPopupForm(MapForm);
      free_turn_flag = true;
      break;
    case menu_mainSettings:
      FrmPopupForm(PrefsForm);
      free_turn_flag = true;
      break;
    case menu_mainMsgs:
      FrmPopupForm(MsgLogForm);
      free_turn_flag = true;
      break;
    case menu_mainFont:
      toggle_itsy();
      get_panel(char_row, char_col, true); // FORCE recalculation of panel
      recalc_screen();
      refresh();
      free_turn_flag = true;
      break;
    case menu_mainAbout:
      FrmPopupForm(AboutForm);
      free_turn_flag = true;
      break;
    case menu_mainStats:
      came_from_generation = false;
      FrmPopupForm(CharSummaryForm);
      free_turn_flag = true;
      break;
    case menu_mainRecall:
      // this should be same as '/' what-is-this- character
      //      FrmPopupForm(StoreForm); // just for testing
      do_command('/');
      free_turn_flag = true;
      break;
    default:
      // MAGIC & MORE MAGIC..
      tmp_x = e->data.menu.itemID - MAGIC_MENU_NUMBER;
      if (tmp_x > 0 && tmp_x < 128)
	handled = do_command((Char) tmp_x);
      if (handled && !free_turn_flag) command_count = 0; // XXX
      break;
    }
    handled = true;
    break;

    // let's have some fun...
  case keyDownEvent:
    // hardware button -- or else graffiti.
    if (true) {
      // under some circ.s you don't want to do this.
      // like casting a directional spell?  maybe?
      if (message_clear()) {
	show_messages();
	command_count = 0;
	return true;
      }
    }
    //      message_clear();
    handled = do_command(e->data.keyDown.chr); // XXXX This needs fixed!
    if (handled && !free_turn_flag) command_count = 0; // XXX
    else {
      free_turn_flag = true;
      // allow numbers for 'command_count'
      if ('0' <= e->data.keyDown.chr
	  && e->data.keyDown.chr <= '9') {
	Char buf[16];
	if (command_count <= 0) {
	  command_count = e->data.keyDown.chr - '0';
	} else if (command_count <= 9) {
	  command_count = command_count * 10 + e->data.keyDown.chr - '0';
	} else {
	  command_count = 99; /* maxed-out */
	} // xxx Hey, I need to make "backspace" work too.
	handled = true;
	StrPrintF(buf, "count: %d", command_count);
	message(buf);
      } // else it must be a typo...
    }
    break;

  case penDownEvent:
    if (true) {
      // under some circ.s you don't want to do this.
      // like casting a directional spell?  maybe?
      if (message_clear()) {
	show_messages();
	command_count = 0;
	return true;
      }
    }
    tmp_x = e->screenX - 80;
    tmp_y = e->screenY - 80;
    if (e->screenY > 160-6) {
      print_stats(STATS_TOGGLE);
      //      ctr = 0;
      handled = true;
      free_turn_flag = true;
      break;
    }
    /*
      1. Map the direction to 1,2,3,4,6,7,8,9.
      2. Decide whether last_command != -1
      3. Either do_dir_command(dir, NULL),
      4. Or move_char(dir, do_pickup)
      5. last_command = -1;do_pickup = true; count = 0; free_turn_flag = false;
     */
    if (last_command == -1) { // Move!...
      Boolean do_run = my_prefs.run;
      Short dist = tmp_x * tmp_x + tmp_y * tmp_y;
      if (my_prefs.relative_move) {
	// translate tap coordinates so that rogue's screen location = 0,0.
	relativize_move(&tmp_x, &tmp_y);
      }
      if (dist < my_prefs.walk_center_border * 10
	          * my_prefs.walk_center_border * 10) {
	; // search or something
      } else {
	dir = convert_to_dir(tmp_x, tmp_y, 10);
	// /* search or something */ }
	//if ((abs(tmp_x) < my_prefs.run_walk_border*10) &&
	//  (abs(tmp_y) < my_prefs.run_walk_border*10))
	//	if ((abs(tmp_x) < 6 * 10) && (abs(tmp_y) < 6 * 10))
	if (dist < my_prefs.run_walk_border * 10
	           * my_prefs.run_walk_border * 10)
	  do_run = false;
	if (!do_run)
	  move_char(dir, do_pickup);	// WALK
	else {
	  find_init(dir);	                // RUN
	  if (find_flag) find_run();
	  // how come this does not print hit-messages??
	}
      }
      do_pickup = true;
      command_count = 0; // not that it's, like, currently working or anything
    } else { // Last turn we got asked to do a directional command
      if (last_command == '/') {
	Short x, y;
	where_in_dungeon(e->screenX, e->screenY, &x, &y); // convert to cell
	do_xy_command(x, y);
      } else {
	undraw_directional();
	//	message_clear(); // in case some of these forget to clear it
	dir = convert_to_dir(tmp_x, tmp_y, 10);
	if (dir == 5 && last_command != '?') // 'aborted' unless 'look'
	  free_turn_flag = true;
	else
	  do_dir_command(dir, 0);
      }
      last_command = -1;
    }
    command_count = 0;
    handled = true;
    break;

  default:
    return true;
  } // end switch event type!
  // for anyone who has not returned,
  // call something to move the monsters.
  if (!free_turn_flag) {
    // so, like there's a fatal error in one of these puppies.
    end_of_turn();   // this is what moves the creatures and makes new levels
                     // (should I check for death here?  I wonder..)
    start_of_turn(); // this is what makes your own clock tick.
  }
  show_messages();
  refresh();
  return handled;
}

/*****************************************************************************/
/*
// if free_turn_flag needs to be set, do the setting here.
// caller will zero command_count, and deal with numeric input and '-'
static Boolean do_command(Char c)
{
  // decide whether to clear the message....
  switch(c) {
  case 'a':
    //message("aim()"); // xxx needs direction argument
    needed_direction_for = DIR_AIM; // also needs an object
    free_turn_flag = true;
    draw_directional();
    return true;
  case 'z':
    //use();
    return true;
  case 'c':
    needed_direction_for = DIR_CLOSE;
    free_turn_flag = true;
    draw_directional();
    return true;
  case 't':
    needed_direction_for = DIR_THROW; // also needs an object
    //throw_object(need item_val); // shouldn't this be an Inventory command??
    free_turn_flag = true;
    draw_directional();
    return true;
  case 'm':
    //cast()
    return true;
  case 'o':
    needed_direction_for = DIR_OPEN;
    free_turn_flag = true;
    draw_directional();
    return true;
  case 'p':
    //pray();
    return true;
  case 's':
    //search();
    return true;
  case 'x':
    // exchange weapons
    return true;
  case 'f':
    needed_direction_for = DIR_BASH;
    free_turn_flag = true;
    draw_directional();
    return true;
  case '.':
    //rest();
    return true;
  case 'd':
    needed_direction_for = DIR_TUNNEL;
    free_turn_flag = true;
    draw_directional();
    return true;
  case '^':
    needed_direction_for = DIR_DISARM;
    free_turn_flag = true;
    draw_directional();
    return true;
  case '<':
  case pageUpChr:
    //go_up();
    return true;
  case '>':
  case pageDownChr:
    //go_down();
    return true;
  case 'i':
    // go to the inventory form
    return true;
  case 'e':
    // go to the equipment form
    return true;
  case 'S':
    // toggle search mode
    free_turn_flag = true;
    return true;
  case 'w':
    needed_direction_for = DIR_WEDGE;
    free_turn_flag = true;
    draw_directional();
    return true;
  case '?':
    needed_direction_for = DIR_LOOK;
    free_turn_flag = true;
    draw_directional();
    return true;
  case 'g':
    //gain_spells();
    return true;
  }
  return false;
}
*/
/*****************************************************************************/

static Boolean buttonsHandleEvent(EventPtr e)
{
  Boolean handled = false;
  Boolean took_time = true;
  Short dispatch_type = 0;

  if (!(ChrIsHardKey(e->data.keyDown.chr))
      && (e->data.keyDown.chr != findChr)
      && (e->data.keyDown.chr != pageUpChr)
      && (e->data.keyDown.chr != pageDownChr))
    return false; // it's NOT a hardware button.. probably graffiti.

  // <incs>/UI/Chars.h is useful....   ChrIsHardKey(c)
  // hard[1-4]Chr, calcChr, findChr.
  switch (e->data.keyDown.chr) {
  case hard1Chr:      // datebook
    dispatch_type = my_prefs.hardware[0];
    break;
  case hard2Chr:      // address
    dispatch_type = my_prefs.hardware[1];
    break;
  case hard3Chr:      // todo
    dispatch_type = my_prefs.hardware[2];
    break;
  case hard4Chr:      // memos
    dispatch_type = my_prefs.hardware[3];
    break;
  case pageUpChr:
    dispatch_type = my_prefs.hardware[4];
    break;
  case pageDownChr:
    dispatch_type = my_prefs.hardware[5];
    break;
  case calcChr:
    dispatch_type = my_prefs.hardware[6];
    break;
  case findChr:
    dispatch_type = my_prefs.hardware[7];
    break;
  default:
    return false;
  }
  switch (dispatch_type) {
    /*
  case HWB_NOOP:
    return false;
  case HWB_N:
  case HWB_S:
  case HWB_E:
  case HWB_W:
    handled = true;
    break;
    */
  }
  // do that loop thing too.  yeah.
  if (handled && took_time) {
    ; //    start_of_Play_Level_loop();
  }
  return handled;
}


/*****************************************************************************
 *                                                                           *
 *                      OpenDatabase                                         *
 *                                                                           *
 *****************************************************************************/
static Boolean OpenDatabase(void)
{
  // The '0' arguments are all 'card number'.
  // Note I have two databases of the same type now, so
  //  DmOpenDatabaseByTypeCreator is not going to work.
  LocalID dbID;
  Boolean created = false;

  // We cannot run if we cannot find the MoriaDB, so return error
  if (0 == (dbID = DmFindDatabase(0, MoriaDBName))) return 1;
  if (0 == (MoriaDB = DmOpenDatabase(0, dbID, dmModeReadWrite))) return 1;

  // We can run if we cannot find the MoriaSaveDB.  Just create it.
  if (0 == (dbID = DmFindDatabase(0, MoriaSaveDBName))) {
    if (DmCreateDatabase(0, MoriaSaveDBName, MoriaAppID, MoriaSaveDBType,
			 false))
      return 1;
    created = true;
    if (0 == (dbID = DmFindDatabase(0, MoriaSaveDBName))) return 1;
  }
  if (0 == (MoriaSaveDB = DmOpenDatabase(0, dbID, dmModeReadWrite))) return 1;

  if (DmNumRecords(MoriaSaveDB) < 1) {
    data_record_create();
  }

  return 0;
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
	case AboutForm:
	    FrmSetEventHandler(frm, About_Form_HandleEvent);
	    break;
	case MapForm:
	    FrmSetEventHandler(frm, Map_Form_HandleEvent);
	    break;
	case PrefsForm:
	    FrmSetEventHandler(frm, Prefs_Form_HandleEvent);
	    break;
	case MsgLogForm:
	    FrmSetEventHandler(frm, MsgLog_Form_HandleEvent);
	    break;
	case InvForm:
	    FrmSetEventHandler(frm, Inv_Form_HandleEvent);
	    break;
	case EquForm:
	    FrmSetEventHandler(frm, Equ_Form_HandleEvent);
	    break;
	case RecallForm:
	    FrmSetEventHandler(frm, Recall_Form_HandleEvent);
	    break;
	case StoreForm:
	    FrmSetEventHandler(frm, Store_Form_HandleEvent);
	    break;
	case HaggleForm:
	    FrmSetEventHandler(frm, Haggle_Form_HandleEvent);
	    break;
	case BookForm:
	    FrmSetEventHandler(frm, Book_Form_HandleEvent);
	    break;
	case SpellForm:
	    FrmSetEventHandler(frm, Spell_Form_HandleEvent);
	    break;
	case GainSpForm:
	    FrmSetEventHandler(frm, GainSp_Form_HandleEvent);
	    break;
       // Generation stuff:
	case CharRaceForm:
	    FrmSetEventHandler(frm, CharRace_Form_HandleEvent);
	    break;
	case CharGenderForm:
	    FrmSetEventHandler(frm, CharGender_Form_HandleEvent);
	    break;
	case CharClassForm:
	    FrmSetEventHandler(frm, CharClass_Form_HandleEvent);
	    break;
	case CharSummaryForm:
	    FrmSetEventHandler(frm, CharSummary_Form_HandleEvent);
	    break;
	}
	handled = true;
    }

    return handled;
}

/*****************************************************************************
 *                      Preferences                                          *
 *****************************************************************************/

static void readMoriaPrefs()
{
  Word prefsSize;
  SWord prefsVersion;

  prefsSize = sizeof(MoriaPreferenceType);
  prefsVersion = PrefGetAppPreferences(MoriaAppID, MoriaAppPrefID, &my_prefs,
				       &prefsSize, true);
  if (!my_prefs.big_font)
    toggle_itsy();
}
void useMoriaPrefs()
{
  find_cut = my_prefs.run_cut_corners;
  find_examine = my_prefs.run_examine_corners;
  find_prself = true;
  find_bound = my_prefs.run_stop_sector;
  find_ignore_doors = my_prefs.run_thru_doors;
  prompt_carry_flag = my_prefs.prompt_pickup; // might wire this to false
  //  rogue_like_commands = true; // this is not used and also only quasitrue
  //  show_weight_flag = my_prefs.show_weights; // this is not used I guess
  highlight_seams = my_prefs.highlight_minerals;
  //  sound_beep_flag = false;
  //  display_counts = false;
}
void writeMoriaPrefs()
{
  Word prefsSize;

  prefsSize = sizeof(MoriaPreferenceType);
  PrefSetAppPreferences(MoriaAppID, MoriaAppPrefID, MoriaAppPrefVersion,
			&my_prefs, prefsSize, true);
}




/*****************************************************************************
 *                      StartApplication                                     *
 *****************************************************************************/
Boolean evil = false;
#ifdef I_AM_OS_2
FontPtr oldFontSix = 0;
#endif
static Word StartApplication(void)
{
  Word error = 0;
  //  Boolean found_my_character;
  DWord version;
#ifdef I_AM_OS_2
  void *font128 = 0;
#else
  VoidHand fontHandle;
  FontType *fontPtr;
#endif
  Boolean found_my_character;

  error = OpenDatabase(); /// I am not ready for this yet!
  //  error = false;

  if (error) {
    //    FrmGotoForm(SadForm);
    evil = true;
  } //else {

  data_record_lock(true);

  /* load game and/or do all the ONE-TIME initialization */
  //    found_my_character = init_game(); // allocates env et al
  readMoriaPrefs();
  useMoriaPrefs();
    
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
#ifdef I_AM_OS_2
  // evil font kludges!
  font128 = MemHandleLock(DmGetResource('NFNT', ItsyFont));
  if (version < 0x03000000L) {
    oldFontSix = ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6];
    ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6] = font128;
  }
#else
  if (version >= 0x03000000L) {
    fontHandle = DmGetResource('NFNT',ItsyFont);
    fontPtr = MemHandleLock(fontHandle);
    // "user defined fonts start from 129"
    FntDefineFont(129,fontPtr);
    // the custom font is uninstalled automatically when we leave iLarn;
    // however, fontHandle must remain locked until then.
  }  
#endif
  
  found_my_character = false;
    // found_my_character = true;
  /*
    if (generate) generate_cave()  -- this is sorta working
    then I loop until dead in MainForm.  which is like dungeon() was.
   */
  game_init();
  if (found_my_character)
    FrmGotoForm(MainForm);
  else
    FrmGotoForm(CharRaceForm);
  
  return 0;
}


/*****************************************************************************
 *                      StopApplication                                      *
 *****************************************************************************/
#define free_me(a)  h = MemPtrRecoverHandle((a)); if (h) MemHandleFree(h);
static void StopApplication(void)
{
  //  VoidHand h;
  DWord version;
#ifdef I_AM_OS_2
  //void *font128 = 0;
#else
  VoidHand fontHandle;
#endif

  if (evil) return;

  data_record_lock(false);

  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
#ifdef I_AM_OS_2
  if (version < 0x03000000L) {
    if (oldFontSix) {
      ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6] = oldFontSix;
      oldFontSix = 0;
      //      if ((font128 = ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6]))
      //	MemHandleUnlock(font128);
    }
  }
#else
  if (version >= 0x03000000L) {
    fontHandle = DmGetResource('NFNT',ItsyFont);
    MemHandleUnlock(fontHandle);
  }  
#endif

  // try to save!  woohoo.
  /*
  if (env->cdude[HP] > 0)
    save_all(env);

  free_env(env);
  free_me(env);
  */
  FrmSaveAllForms();
  FrmCloseAllForms();

  if (MoriaDB!=NULL) {
    DmCloseDatabase(MoriaDB);
    MoriaDB = NULL;
  }
  if (MoriaSaveDB!=NULL) {
    DmCloseDatabase(MoriaSaveDB);
    MoriaSaveDB = NULL;
  }
  // free stuff

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
	// first see if it's a hardware button thing!!!
	// don't ask me what the poweredOnKeyMask is, though; cargo cult.
	// Do special hardware button things only if:
	// it's a hardware button event, [you're alive,] and in the main form.
	if ( (e.eType != keyDownEvent)
	     || !my_prefs.use_hardware 
	     || (e.data.keyDown.modifiers & poweredOnKeyMask)
	     || (FrmGetActiveFormID() != MainForm)
	     || !buttonsHandleEvent(&e) )
	  // now proceed with usual handling
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
