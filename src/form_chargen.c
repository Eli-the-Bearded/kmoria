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

// Not yet done:
// Background
// Recording the selected name
// Access to the name-change form from the game
// Aborting character creation

// in create.c:
void reroll_char();
void post_reroll();
void other_stuff();

/*
 *  This is the form for Character Generation.
 *  I would like to keep the 'feel' similar to the original.
 *  At the bottom is a transcript of original generation.
 */


// this is squished a bit to make room for 18/00's.
// name, race, sex, class, social class, to hit, to ac
#define COL_1    0
// to damage, total ac
#define COL_2   50
// age, height, weight
#define COL_3   66
#define COL_3a  98
// Max HP...  Searching...
#define COL_31  88
// str, int, wis, dex, con, chr (strings)
#define COL_4  116
// str, int, wis, dex, con, chr (values)
#define COL_5  136
// name
#define ROW_1   15
// race, age, str
#define ROW_2   26
// to hit, to damage
#define ROW_3   70
//#define ROW_4   92
#define ROW_4   94

static void print_dnd_stat(Char *buf, Short stat)
{
  if (stat < 19)
    StrPrintF(buf, "%d", stat);
  else {
    stat -= 18;
    if (stat > 100) stat = 0; // 18/00 == 18/100 in MY world.
    if (stat < 10)
      StrPrintF(buf, "18/0%d", stat);
    else 
      StrPrintF(buf, "18/%d", stat);
  }
}

static void foo_race()
{
  Char buf[40];
  Short x, y;
  x = COL_1;
  y = ROW_1;
  StrPrintF(buf, "Name:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11;
  StrPrintF(buf, "Race:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11;
  StrPrintF(buf, "Sex:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11;
  StrPrintF(buf, "Class:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11;
  WinDrawLine(0, y, 160, y);
  y += 2;  
  StrPrintF(buf, "Please select a race:");
  WinDrawChars(buf, StrLen(buf), x, y);
}

/* STEP 1: Select a race. */
Boolean CharRace_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ControlPtr pushbtn;
  Short i, tentative_race;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);
    pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_race_0));
    CtlSetValue(pushbtn, true);
    foo_race();
    handled = true;
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_race_ok:
      frm = FrmGetActiveForm();
      tentative_race = -1;
      for (i = pbtn_race_0 ; i <= pbtn_race_7 ; i++) {
	pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
	if (CtlGetValue(pushbtn))
	  tentative_race = i - pbtn_race_0;
      }
      if (tentative_race != -1) {
	pymisc.prace = tentative_race;
	FrmGotoForm(CharGenderForm);
      }
      // else beep or something.
      handled = true;
      break;
    case pbtn_race_0:
    case pbtn_race_1:
    case pbtn_race_2:
    case pbtn_race_3:
    case pbtn_race_4:
    case pbtn_race_5:
    case pbtn_race_6:
    case pbtn_race_7:
      handled = true;
      break;
    }
    break; // end of ctlSelectEvent

  default:
    break;
  }

  return handled;
}

static void foo_gender()
{
  Char buf[40];
  Short x, y;
  x = COL_1;
  y = ROW_1;
  StrPrintF(buf, "Name:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11;
  StrPrintF(buf, "Race: %s", race[pymisc.prace].trace);
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11;
  StrPrintF(buf, "Sex:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11;
  StrPrintF(buf, "Class:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11;
  WinDrawLine(0, y, 160, y);
  y += 2;  
  StrPrintF(buf, "Please select a gender:");
  WinDrawChars(buf, StrLen(buf), x, y);
}


/* STEP 2: Select a gender. */
Boolean CharGender_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ControlPtr pushbtn;
  Short i, tentative_gender;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);
    pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_gender_0));
    CtlSetValue(pushbtn, true);
    foo_gender();
    handled = true;
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_gender_no:
      FrmGotoForm(CharRaceForm);
      handled = true;
      break;
    case btn_gender_ok:
      frm = FrmGetActiveForm();
      tentative_gender = -1;
      for (i = pbtn_gender_0 ; i <= pbtn_gender_1 ; i++) {
	pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
	if (CtlGetValue(pushbtn))
	  tentative_gender = i - pbtn_gender_0;
      }
      if (tentative_gender != -1) {
	pymisc.male = (tentative_gender==0);
	FrmGotoForm(CharClassForm);
      }
      // else beep or something.
      handled = true;
      break;
    case pbtn_gender_0:
    case pbtn_gender_1:
      handled = true;
      break;
    }
    break; // end of ctlSelectEvent

  default:
    break;
  }

  return handled;
}

static void foo_class(Boolean all, Boolean squeeze)
{
  Char buf[40];
  Short x, y, i;
  RectangleType r;
  x = COL_1;
  y = ROW_1;
  if (all) {
    if (!squeeze) {
      StrPrintF(buf, "Name:");
      WinDrawChars(buf, StrLen(buf), x, y);
      y += 11;
    }
    StrPrintF(buf, "Race: %s", race[pymisc.prace].trace);
    WinDrawChars(buf, StrLen(buf), x, y);
    y += 11;
    StrPrintF(buf, "Sex: %s", pymisc.male?"Male":"Female");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += 11;
    StrPrintF(buf, "Class:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += 11;
  } else {
    y += squeeze ? 33 : 44;
    RctSetRectangle(&r, x, y, COL_4-x, 33);
    WinEraseRectangle(&r, 0);
  }
  StrPrintF(buf, "Social Class: %d", pymisc.sc);
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11;
  StrPrintF(buf, "+ To Hit: %d", pymisc.dis_th);
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11;
  StrPrintF(buf, "+ To AC: %d", pymisc.dis_tac);
  WinDrawChars(buf, StrLen(buf), x, y);
  // end of first column
  x = COL_2;
  y = squeeze ? ROW_3 - 11 : ROW_3;
  StrPrintF(buf, "To Damage: %d", pymisc.dis_td);
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11;
  StrPrintF(buf, "Total AC: %d", pymisc.dis_ac);
  WinDrawChars(buf, StrLen(buf), x, y);
  // end of second column
  if (all) {
    x = COL_3;
    y = squeeze ? ROW_1 : ROW_2;
    StrPrintF(buf, "Age:", pymisc.age);
    WinDrawChars(buf, StrLen(buf), x, y);
    y += 11;
    StrPrintF(buf, "Height:", pymisc.ht);
    WinDrawChars(buf, StrLen(buf), x, y); // Hey!  I should have a METRIC pref!
    y += 11;
    StrPrintF(buf, "Weight:", pymisc.wt);
    WinDrawChars(buf, StrLen(buf), x, y);
  }
  x = COL_3a;
  y = squeeze ? ROW_1 : ROW_2;
  if (!all) {
    RctSetRectangle(&r, x, y, COL_4-x, 33);
    WinEraseRectangle(&r, 0);
  }
  StrPrintF(buf, "%d", pymisc.age);
  WinDrawChars(buf, StrLen(buf), pymisc.age<100?x+5:x, y);
  y += 11;
  StrPrintF(buf, "%d", pymisc.ht); // Hey!  I should have a UNITS preference!
  WinDrawChars(buf, StrLen(buf), pymisc.ht<100?x+5:x, y);
  y += 11;
  StrPrintF(buf, "%d", pymisc.wt);
  WinDrawChars(buf, StrLen(buf), pymisc.wt<100?x+5:x, y);

  // end of third column
  if (all) {
    x = COL_4;
    y = squeeze ? ROW_1 : ROW_2;
    StrPrintF(buf, "Str:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += 11;
    StrPrintF(buf, "Int:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += 11;
    StrPrintF(buf, "Wis:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += 11;
    StrPrintF(buf, "Dex:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += 11;
    StrPrintF(buf, "Con:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += 11;
    StrPrintF(buf, "Chr:");
    WinDrawChars(buf, StrLen(buf), x, y);
  }
  //
  x = COL_5;
  y = squeeze ? ROW_1 : ROW_2;
  if (!all) {
    RctSetRectangle(&r, x, y, 160-x, 66);
    WinEraseRectangle(&r, 0);
  }
  for (i = 0 ; i < 6 ; i++) {
    // hey what about 18/00
    print_dnd_stat(buf, pystats.max_stat[i]);
    WinDrawChars(buf, StrLen(buf), pystats.max_stat[i]<10?x+5:x, y);
    y += 11;
  }
  //  WinDrawLine(0, y, 160, y);
}

static void foo_class_bg(Boolean bg_only)
{
  // [Calculate and print] the background information
  Short x, y;
  RectangleType r;
  if (bg_only) {
    x = COL_1;
    y = ROW_4-3-11;
  } else {
    x = COL_1;
    y = ROW_1;
  }
  RctSetRectangle(&r, x, y, 160-x, 146-y);
  WinEraseRectangle(&r, 0);
}

/* STEP 3: Select a class. */
Boolean CharClass_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ControlPtr pushbtn, btn;
  Short i, tentative_class;
  Boolean foo=false;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);
    // Also, print their race, gender, and rolled stats:
    reroll_char();
    foo_class(true, true);
    handled = true;
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_class_no:
      FrmGotoForm(CharRaceForm);
      handled = true;
      break;
    case btn_class_reroll:
      // call again whatever routine we called to roll & print stats:
      foo_class_bg(true);
      reroll_char();
      foo_class(false, true);
      handled = true;
      break;
    case btn_class_ok:
      frm = FrmGetActiveForm();
      foo_class_bg(false);
      foo_class(true, false);
      WinDrawLine(0, ROW_4-2, 160, ROW_4-2);
      // enable class buttons based on character race
      for (i = 0 ; i < MAX_CLASS ; i++) {
	if (1<<i & race[pymisc.prace].rtclass) {
	  pushbtn = FrmGetObjectPtr(frm,
				    FrmGetObjectIndex(frm, pbtn_class_0+i));
	  CtlShowControl(pushbtn);
	  if (!foo) CtlSetValue(pushbtn, true);
	  foo = true;
	}
      }
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_class_ok));
      CtlHideControl(btn);
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_class_reroll));
      CtlHideControl(btn);
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_class_ok2));
      CtlShowControl(btn);
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_class_no));
      CtlShowControl(btn);
      FrmCopyTitle(frm, "Now pick a class");
      handled = true;
      break;
    case btn_class_ok2:
      frm = FrmGetActiveForm();
      tentative_class = -1;
      for (i = pbtn_class_0 ; i <= pbtn_class_5 ; i++) {
	pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
	if (CtlGetValue(pushbtn))
	  tentative_class = i - pbtn_class_0;
      }
      if (tentative_class != -1) {
	pymisc.pclass = tentative_class;
	post_reroll();
	FrmGotoForm(CharSummaryForm);
      }
      // else beep or something.
      handled = true;
      break;
    case pbtn_class_0:
    case pbtn_class_1:
    case pbtn_class_2:
    case pbtn_class_3:
    case pbtn_class_4:
    case pbtn_class_5:
      handled = true;
      break;
    }
    break; // end of ctlSelectEvent

  default:
    break;
  }

  return handled;
}

// Note: I assume a maximum length of 7 digits (decimal) for experience,gold.
// These two magic numbers happen to be the same:
#define FNORD 47
#define QUUX 47
static void foo_summary_a()
{
  Char buf[40];
  Short x, y;

  x = COL_1;
  y = ROW_4;
  StrPrintF(buf, "Level:");
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%d", pymisc.lev);
  WinDrawChars(buf, StrLen(buf), x+FNORD+5*(7-StrLen(buf)), y);
  y += 11;
  StrPrintF(buf, "Experience:");
  WinDrawChars(buf, StrLen(buf), x, y);
  //  pymisc.exp = 1234567L;
  StrPrintF(buf, "%ld", pymisc.exp);
  WinDrawChars(buf, StrLen(buf), x+FNORD+5*(7-StrLen(buf)), y);
  y += 11;
  StrPrintF(buf, "Max Exp:");
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%ld", pymisc.max_exp);
  WinDrawChars(buf, StrLen(buf), x+FNORD+5*(7-StrLen(buf)), y);
  y += 11;
  if (pymisc.lev < MAX_PLAYER_LEVEL && pymisc.lev > 0) {
    StrPrintF(buf, "Exp to Adv.:");
    WinDrawChars(buf, StrLen(buf), x, y);
    // xxx this might be bogus, check for division truncation problems
    StrPrintF(buf, "%ld", (Long) (player_exp[pymisc.lev-1]
				  * pymisc.expfact / 100));
    WinDrawChars(buf, StrLen(buf), x+FNORD+5*(7-StrLen(buf)), y);
  }
  // end of first column
  x = COL_31;
  y = ROW_4;
  StrPrintF(buf, "Max HP:", pymisc.mhp);
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%d", pymisc.mhp);
  WinDrawChars(buf, StrLen(buf), x+QUUX+5*(4-StrLen(buf)), y);
  y += 11;
  StrPrintF(buf, "Cur HP:", pymisc.chp);
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%d", pymisc.chp);
  WinDrawChars(buf, StrLen(buf), x+QUUX+5*(4-StrLen(buf)), y);
  y += 11;
  StrPrintF(buf, "Max Mana:", pymisc.mana);
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%d", pymisc.mana);
  WinDrawChars(buf, StrLen(buf), x+QUUX+5*(4-StrLen(buf)), y);
  y += 11;
  StrPrintF(buf, "Cur Mana:", pymisc.cmana);
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%d", pymisc.cmana);
  WinDrawChars(buf, StrLen(buf), x+QUUX+5*(4-StrLen(buf)), y);
  y += 11;
  StrPrintF(buf, "Gold:");
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%ld", pymisc.au);
  WinDrawChars(buf, StrLen(buf), x+(QUUX-15)+5*(7-StrLen(buf)), y);
}
#undef FNORD
#undef QUUX
// was in misc3
static Char *likert(Short x, Short y)
{
  // I hope this still DTRT
  switch((x/y)) {
  case -3: case -2: case -1: return("v.bad");
  case 0: case 1:            return("bad");
  case 2:                    return("poor");
  case 3: case 4:            return("fair");
  case  5:                   return("good");
  case 6:                    return("v.good");
  case 7: case 8:            return("excell.");
  default:                   return("superb");
  }
}

static void foo_summary_b()
{
  Char buf[40], *val;
  Short x, y, x2;
  Short xbth, xbthb, xfos, xsrh, xstl, xdis, xsave, xdev;

  // Calculate the miscellaneous abilities;
  // this replaces calc_misc_abilities()    in misc3.c
  xbth  = pymisc.bth + pymisc.ptohit*BTH_PLUS_ADJ
    + (class_level_adj[pymisc.pclass][CLA_BTH] * pymisc.lev);
  xbthb = pymisc.bthb + pymisc.ptohit*BTH_PLUS_ADJ
    + (class_level_adj[pymisc.pclass][CLA_BTHB] * pymisc.lev);
  /* this results in a range from 0 to 29 */
  xfos  = 40 - pymisc.fos;
  if (xfos < 0)  xfos = 0;
  xsrh  = pymisc.srh;
  /* this results in a range from 0 to 9 */
  xstl  = pymisc.stl + 1;
  xdis  = pymisc.disarm + 2*todis_adj() + stat_adj(A_INT)
    + (class_level_adj[pymisc.pclass][CLA_DISARM] * pymisc.lev / 3);
  xsave = pymisc.save + stat_adj(A_WIS)
    + (class_level_adj[pymisc.pclass][CLA_SAVE] * pymisc.lev / 3);
  xdev  = pymisc.save + stat_adj(A_INT)
    + (class_level_adj[pymisc.pclass][CLA_DEVICE] * pymisc.lev / 3);
  // done calculating, now print stuff

  x = COL_1;  x2 = x + 56;
  y = ROW_4;
  StrPrintF(buf, "Fighting");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xbth, 12); // fighting
  WinDrawChars(val, StrLen(val), x2, y);
  y += 11;
  StrPrintF(buf, "Bows/Throw");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xbthb,12); // bows/throw
  WinDrawChars(val, StrLen(val), x2, y);
  y += 11;
  StrPrintF(buf, "Magic Device");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xdev, 6); // magic device
  WinDrawChars(val, StrLen(val), x2, y);
  y += 11;
  StrPrintF(buf, "Saving Throw");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xsave, 6); // saving throw
  WinDrawChars(val, StrLen(val), x2, y);
  x = COL_31;  x2 = x + 45;
  y = ROW_4;
  StrPrintF(buf, "Disarming");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xdis, 8); // disarming
  WinDrawChars(val, StrLen(val), x2, y);
  y += 11;  
  StrPrintF(buf, "Stealth");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xstl, 1); // stealth
  WinDrawChars(val, StrLen(val), x2, y);
  y += 11;  
  StrPrintF(buf, "Searching");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xsrh, 6); // searching
  WinDrawChars(val, StrLen(val), x2, y);
  y += 11;  
  StrPrintF(buf, "Perception");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xfos, 3); // perception
  WinDrawChars(val, StrLen(val), x2, y);
  y += 11;  
  StrPrintF(buf, "Infravision");
  WinDrawChars(buf, StrLen(buf), x, y);
  // xxx Hey, when I have a metric preference option, don't forget infravision
  StrPrintF(buf, "%d'", pyflags.see_infra*10); // infravision
  WinDrawChars(buf, StrLen(buf), x2, y);
}
Boolean charsum_is_a;
static void foo_summary()
{
  Char buf[40];
  Short x, y;

  foo_class(true, false);
  WinDrawLine(0, ROW_4-2, 160, ROW_4-2);

  x = COL_1;
  y = ROW_1 + 33;
  StrPrintF(buf, "Class: %s", class[pymisc.pclass].title);
  WinDrawChars(buf, StrLen(buf), x, y);
  foo_summary_a();
  charsum_is_a = true;
}
static void foo_summary_swapbot()
{
  RectangleType r;
  Short x, y;
  x = 0; y = ROW_4;
  RctSetRectangle(&r, x, y, COL_31-x, 146-y);
  WinEraseRectangle(&r, 0);
  x = COL_31;
  RctSetRectangle(&r, x, y, 160-x, 160-y);
  WinEraseRectangle(&r, 0);
  if (charsum_is_a)
    foo_summary_b();
  else 
    foo_summary_a();
  charsum_is_a = !charsum_is_a;
}

/* STEP 4: Pick a name. */
/* megaNOTE: This form must be re-visitable! somehow! */
static void charsum_init_name(FormPtr frm)
{
  FieldPtr fld;
  CharPtr p;
  VoidHand vh;
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_charsum));
  vh = (VoidHand) FldGetTextHandle(fld);
  if (!vh) {
    vh = MemHandleNew(PLAYER_NAME_SIZE * sizeof(Char));
  }
  FldSetTextHandle(fld, (Handle) 0);
  p = MemHandleLock(vh);
  StrNCopy(p, pymisc.name, PLAYER_NAME_SIZE);
  p[PLAYER_NAME_SIZE-1] = '\0';
  MemHandleUnlock(vh);
  FldSetTextHandle(fld, (Handle) vh);
}
static void charsum_commit_name(FormPtr frm)
{
  FieldPtr fld;
  CharPtr p;
  VoidHand vh;
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_charsum));
  vh = (VoidHand) FldGetTextHandle(fld);
  if (vh) {
    p = MemHandleLock(vh);
    StrNCopy(pymisc.name, p, PLAYER_NAME_SIZE);
    MemHandleUnlock(vh);
    pymisc.name[PLAYER_NAME_SIZE-1] = '\0';
  }
}
Boolean came_from_generation = true;
Boolean CharSummary_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  //  FieldPtr fld;
  ControlPtr btn;
  // see Trans_Form_HandleEvent in ilarn/src/main.c
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    charsum_init_name(frm);
    FrmDrawForm(frm);
    // Also, print all kinds of stuff!
    foo_summary();
    if (came_from_generation) {
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_charsum_no));
      CtlShowControl(btn);
    }
    handled = true;
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_charsum_no:
      FrmGotoForm(CharRaceForm);
      handled = true;
      break;
    case btn_charsum_ok:
      // Hey!  do something with field_charsum, which is the character name.
      charsum_commit_name(frm);
      if (came_from_generation) {
	other_stuff();
	FrmGotoForm(MainForm);
      } else
	LeaveForm(); // reckon we are a popup..
      came_from_generation = false; // popup caller should also have set it!!
      handled = true;
      break;
    }
    break; // end of ctlSelectEvent


  case penDownEvent:
    if (e->screenY > ROW_4 && (e->screenY < 146 ||
			       (e->screenX > COL_2 && e->screenX < COL_4))) {
      foo_summary_swapbot();
      if (came_from_generation) {
	frm = FrmGetActiveForm();
	btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_charsum_no));
	CtlShowControl(btn);
      }
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}




/*

Step 1:

 Name        :
 Race        :
 Sex         :
 Class       :
  Choose a race (? for Help):
  a) Human       b) Half-Elf    c) Elf         d) Halfling    e) Gnome
  f) Dwarf       g) Half-Orc    h) Half-Troll

Step 2:

 Name        :
 Race        : Human
 Sex         :
 Class       :
  Choose a sex (? for Help): 
  m) Male       f) Female

Step 3:

 Name        :                        Age          :     20  STR :    11
 Race        : Human                  Height       :     68  INT :    15
 Sex         : Female                 Weight       :    149  WIS :    12
 Class       :                        Social Class :     18  DEX :    15
                                                             CON :    14
                                                             CHR :    13

 + To Hit    :      0
 + To Damage :      0
 + To AC     :      1
   Total AC  :      1

                           Character Background
          You are one of several children of a Serf.  You are a
          credit to the family.  You have brown eyes, straight brown
          hair, and a dark complexion.


  Hit space to reroll or ESC to accept characteristics: 

Step 4:

 Name        :                        Age          :     20  STR :    11
 Race        : Human                  Height       :     68  INT :    15
 Sex         : Female                 Weight       :    149  WIS :    12
 Class       :                        Social Class :     18  DEX :    15
                                                             CON :    14
                                                             CHR :    13

 + To Hit    :      0
 + To Damage :      0
 + To AC     :      1
   Total AC  :      1

                           Character Background
          You are one of several children of a Serf.  You are a
          credit to the family.  You have brown eyes, straight brown
          hair, and a dark complexion.


  Choose a class (? for Help):
  a) Warrior     b) Mage        c) Priest      d) Rogue       e) Ranger
  f) Paladin

Step 5:

You can learn some new prayers now.

 Name        :                        Age          :     20  STR :    14
 Race        : Human                  Height       :     68  INT :    12
 Sex         : Female                 Weight       :    149  WIS :    13
 Class       : Paladin                Social Class :     18  DEX :    15
                                                             CON :    16
                                                             CHR :    15

 + To Hit    :      0       Level      :       1    Max Hit Points :     16
 + To Damage :      0       Experience :       0    Cur Hit Points :     16
 + To AC     :      1       Max Exp    :       0    Max Mana       :      0
   Total AC  :      1       Exp to Adv.:      13    Cur Mana       :      0
                            Gold       :     433

                         (Miscellaneous Abilities)
 Fighting    : Good         Stealth     : Poor         Perception  : Bad
 Bows/Throw  : Fair         Disarming   : Fair         Searching   : Poor
 Saving Throw: Fair         Magic Device: Fair         Infra-Vision: 0 feet

  
  Enter your player's name  [press <RETURN> when finished]

Step 6:

You can learn some new prayers now.

 Name        : Helga                  Age          :     20  STR :    14
 Race        : Human                  Height       :     68  INT :    12
 Sex         : Female                 Weight       :    149  WIS :    13
 Class       : Paladin                Social Class :     18  DEX :    15
                                                             CON :    16
                                                             CHR :    15

 + To Hit    :      0       Level      :       1    Max Hit Points :     16
 + To Damage :      0       Experience :       0    Cur Hit Points :     16
 + To AC     :      1       Max Exp    :       0    Max Mana       :      0
   Total AC  :      1       Exp to Adv.:      13    Cur Mana       :      0
                            Gold       :     433

                         (Miscellaneous Abilities)
 Fighting    : Good         Stealth     : Poor         Perception  : Bad
 Bows/Throw  : Fair         Disarming   : Fair         Searching   : Poor
 Saving Throw: Fair         Magic Device: Fair         Infra-Vision: 0 feet

  
  

          [Press any key to continue, or Q to exit.]

Step 7:

You can learn some new prayers now.  -more-

 Name        : Helga                  Age          :     20  STR :    14
 Race        : Human                  Height       :     68  INT :    12
 Sex         : Female                 Weight       :    149  WIS :    13
 Class       : Paladin                Social Class :     18  DEX :    15
                                                             CON :    16
                                                             CHR :    15

 + To Hit    :      0       Level      :       1    Max Hit Points :     16
 + To Damage :      0       Experience :       0    Cur Hit Points :     16
 + To AC     :      1       Max Exp    :       0    Max Mana       :      0
   Total AC  :      1       Exp to Adv.:      13    Cur Mana       :      0
                            Gold       :     433

                         (Miscellaneous Abilities)
 Fighting    : Good         Stealth     : Poor         Perception  : Bad
 Bows/Throw  : Fair         Disarming   : Fair         Searching   : Poor
 Saving Throw: Fair         Magic Device: Fair         Infra-Vision: 0 feet

Step 8:


             ##################################################################
Human        #................................................................#
Paladin      #..............#####.......###########...........................#
Gallant      #..............####5.......###########.........#######...........#
             #..............#####.......6##########.........#######...........#
STR :     14 #..............#####.......###########.........#######>..........#
INT :     12 #..............#####.......###########.........######1...........#
WIS :     13 #..........................###########.........#######...........#
DEX :     15 #..........................###########.........#######...........#
CON :     16 #..............................................#######...........#
CHR :     15 #................................................................#
             #..........................................p.....................#
LEV :      1 #............#######3#..................................@........#
EXP :      0 #............#########...........................................#
MANA:      0 #............#########.........######........###4#...............#
MHP :     16 #............#########.........2#####........#####...............#
CHP :     16 #............#########.........######........#####...............#
             #..............................######........#####...............#
AC  :      1 #................................................................#
GOLD:    433 #................................................................#
             #................................................................#
             ##################################################################
                                                           Study Town level

*/
