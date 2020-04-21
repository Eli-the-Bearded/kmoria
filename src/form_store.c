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
#include <CharAttr.h> /* for IsDigit */ // why is this not working for 3.1??

extern owner_type owners[MAX_OWNERS]; // lock.c


Short in_store; // Need to know which store we are in!
Short store_item; // so that Haggle knows what we're bidding on
Short store_current_tens;
Boolean store_youbuying; // so Haggle knows why we're bidding
Short test = -1;
static Boolean isgold = false;
static void store_gold_owner(Boolean forcegold)
{
  Char buf[40];
  RectangleType r;
  RctSetRectangle(&r, 0, 128, 156, 11);
  WinEraseRectangle(&r, 0);
  if (forcegold || isgold) {
    StrPrintF(buf, "(You have: %ld gold pieces)", pymisc.au);
    isgold = false;
  } else {
    StrPrintF(buf, "%s", owners[store[in_store].owner].owner_name);
    isgold = true;
  }
  WinDrawChars(buf, StrLen(buf), 2, 128);
}

static void free_store_list(FormPtr frm);
#define STORE_NONE -1
extern Boolean in_a_store; 

Boolean Store_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  //  CharPtr label;
    
    // CALLBACK_PROLOGUE

  switch (e->eType) {

  case frmOpenEvent:
    // This is just for debugging purposes - it will let me
    // examine the various stores without moving the rogue.
    frm = FrmGetActiveForm();
    /* ...init form... */
    FrmDrawForm(frm);
    /* this must be AFTER draw form */
    // [draw stuff]
    store_list_tens(0);
    FrmCopyTitle(frm, owners[store[in_store].owner].store_name);
    isgold = false;
    store_gold_owner(false);
    //    store_btns(frm);
    // if there are <= 1 screen full of items in the store, DISABLE BUTTONS
    handled = true;
    break;

  case penDownEvent:
    if (e->screenY > 128 && e->screenY < 128+11) {
      store_gold_owner(false);
      handled = true;
      break;
    }
    break;

  case lstSelectEvent:
    handled = true;
    break;

  case keyDownEvent:
    // hardware button -- or else graffiti.
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      store_list_tens(-1);
      handled = true;
      break;
    case pageDownChr:
      store_list_tens(1);
      handled = true;
      break;
    }
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_st_buy:
      frm = FrmGetActiveForm();
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_st));
      store_item = LstGetSelection(lst);
      // make sure it's in range
      if (store_item >= 0 && store_item <= 9) {
	store_item += 10*store_current_tens;
	if (store_item < store[in_store].store_ctr) {
	  store_youbuying = true;
	  if (store_purchase_init(in_store, store_item))
	    FrmPopupForm(HaggleForm);
	}
      }
      handled = true;
      break;
    case btn_st_prev:
      store_list_tens(-1);
      handled = true;
      break;
    case btn_st_next:
      store_list_tens(1);
      handled = true;
      break;
    case btn_st_sell:
      //      frm = FrmGetActiveForm();
      //      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_st));
      //      store_item = LstGetSelection(lst);
      //      if (store_item >= 0 && store_item <= 9) {
      store_youbuying = false;
      FrmPopupForm(InvForm);
      // The rest will be in exiting the InvForm, I guess.
      //      }
      handled = true;
      break;
    case btn_st_exit:
      free_store_list(FrmGetActiveForm());
      LeaveForm();
      in_store = STORE_NONE;
      in_a_store = false;
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

    // CALLBACK_EPILOGUE

  return handled;
}



////////////////////////////////////////////////////////////////////////////
Char *store_display[10];
void store_list_tens(Short plus)
{
  Short i, iitem;
  Char tmp_val[160], buf[10]; //, c;
  Int width_str, length_str;
  Boolean fits;
  ListPtr lst;
  FormPtr frm = FrmGetActiveForm();
  inven_type *i_ptr;
  inven_record *storeinv;
  Long v;
  Short tmp, y = 15; 
  RectangleType r;

  storeinv = store_inven[in_store];

  if (plus != 0 && store[in_store].store_ctr <= 10)//store_ctr==#items in store
    return;// we're trying to scroll but nothing happens

  // erase the rectangle (126,15) to (156,15+110)
  RctSetRectangle(&r, 126,15, 30, 110);
  WinEraseRectangle(&r, 0);
  store_current_tens += plus;
  // if we overflow or underflow, wrap-around.
  if (store_current_tens*10 >= store[in_store].store_ctr) // "..."
    store_current_tens = 0;
  else if (store_current_tens < 0)
    store_current_tens = store[in_store].store_ctr / 10;// I think it will work
  StrPrintF(buf, "%d", store_current_tens);
  WinDrawChars(buf, 1, 88, 141); // print digit between < - and + > buttons

  for (i = 0 ; i < 10 ; i++) {
    //    c = i + 'a';
    //    StrPrintF(tmp_val, "%c) ", c);
    iitem = i+store_current_tens*10;
    i_ptr = &storeinv[iitem].sitem;
    if (iitem >= store[in_store].store_ctr) // yes, I really do mean >=
      StrPrintF(tmp_val, "..."); // because store_ctr is "first empty space"
    else {
      objdes(tmp_val, i_ptr, true); // was tmpval+3
      // Truncate if too wide for the screen..... we only have 124 pixels...
      // length_str gets set to the # of chars that will fit in 124
      // width_str gets set to the # of pixels that those chars will use
      width_str = 123; //124;
      length_str = StrLen(tmp_val); // max chars to allow
      FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
      tmp_val[length_str] = '\0';
    }
    // ok, now like put it somewhere.
    store_display[i] = (Char *) md_malloc(1 + length_str);
    StrCopy(store_display[i], tmp_val);
    // Also print the price.
    if (iitem < store[in_store].store_ctr) {
      v = storeinv[iitem].scost;
      if (v <= 0) {
	Long value = -v;
	value = value * chr_adj() / 100;
	if (value <= 0)
	  value = 1;
	StrPrintF(buf, "%ld", value);
	tmp = 6 - StrLen(buf); // lame attempt at right-justification...
	// it happens that all numbers are 5 pixels wide.
	WinDrawChars(buf, StrLen(buf), 126 + tmp*5, y);
      } else {
	StrPrintF(buf, "%ld", v);
	tmp = 6 - StrLen(buf); // lame attempt at right-justification...
	// it happens that all numbers are 5 pixels wide.
	WinDrawInvertedChars(buf, StrLen(buf), 126 + tmp*5, y);
      }
    }
    y += 11;
  }

  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_st));
  LstSetListChoices(lst, store_display, 10);
  LstSetSelection(lst, -1);
  LstDrawList(lst);
}

/*
 *  Free some space that we used in the inventory-selection form
 */
static void free_store_list(FormPtr frm)
{
  ListPtr lst;
  Short i;
  VoidHand h;
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_st));
  LstSetListChoices(lst, NULL, 0);
  for (i = 0 ; i < 10 ; i++) {
    h = MemPtrRecoverHandle(store_display[i]);
    if (h) 
      MemHandleFree(h);
  }
}


// store_item should never be -1 in haggle
static void haggle_print_obj()
{
  Char buf[160], *p;
  Word wwlen, wwpixels=156-2-2;
  Short x, y, lines = 0;
  inven_type sold_obj, *i_ptr;
  inven_record *storeinv;
  x = 2; y = 2;
  if (store_youbuying) {
    StrPrintF(buf, "Your offer:");
    WinDrawChars(buf, StrLen(buf), 25, 80);
    StrPrintF(buf, "Buying: ");
    //    WinDrawChars(buf, StrLen(buf), x, y);
    //    y += 11;
    //    StrPrintF(buf, "test %d", store_item);
    p = buf + StrLen(buf);
    storeinv = store_inven[in_store];
    i_ptr = &storeinv[store_item].sitem;
    objdes(p, i_ptr, true);
  } else {
    StrPrintF(buf, "Asking price:");
    WinDrawChars(buf, StrLen(buf), 25, 80);
    StrPrintF(buf, "Selling: ");
    //    WinDrawChars(buf, StrLen(buf), x, y);  y += 11;
    take_one_item(&sold_obj, &inventory[store_item]); // copy item, set count=1
    p = buf + StrLen(buf);
    objdes(p, &sold_obj, true);
  }
  // All the News that Fits, We Print.
  // Actually I don't expect there to be more than two lines.
  p = buf;
  wwlen = 0;
  while (lines < 3 && StrLen(p) > 0) {
    wwlen = FntWordWrap(p, wwpixels);
    WinDrawChars(p, wwlen, x, y);
    p += wwlen;
    y += 11;
    lines++;
  }
  StrPrintF(buf, "You have: %ld gold pieces", pymisc.au);
  WinDrawChars(buf, StrLen(buf), x, y);
  y += 11 + 2;
  WinDrawGrayLine(0, y, 160, y);
}

void haggle_form_enable_autoinc(Short incr)
{
  Char tmp[10];
  FormPtr frm;
  ControlPtr btn;
  frm = FrmGetActiveForm();
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_hag_incr));

  if (incr == 0) {
    CtlHideControl(btn);
    return;
  }

  if (incr > 0)
    StrPrintF(tmp, "[+%d]", incr);
  else
    StrPrintF(tmp, "[-%d]", 0 - incr);
  CtlSetLabel(btn, tmp);
  CtlShowControl(btn);
}

void store_form_boot()
{
  FormPtr frm;
  ControlPtr btn;
  frm = FrmGetActiveForm();
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_st_buy));
  CtlHideControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_st_sell));
  CtlHideControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_st_prev));
  CtlHideControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_st_next));
  CtlHideControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_st_exit));
  CtlSetLabel(btn, "OUT!!");
  CtlShowControl(btn);
}
void haggle_form_boot(Boolean kick_out, Char *done)
{
  FormPtr frm;
  ControlPtr btn;
  frm = FrmGetActiveForm();
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_hag_ok));
  CtlHideControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_hag_exit));
  if (kick_out) {
    CtlHideControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_hag_boot));
    CtlShowControl(btn);
    return;
  } else {
    CtlSetLabel(btn, done);
    CtlShowControl(btn);
  }
}

Boolean Haggle_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  FieldPtr fld;
  Word chr;
  VoidHand h;
  VoidPtr p;
  CharPtr textp;
  Char buf[20];
  Long gold, autobid;
  //  Boolean ok = true;
  Short outcome;
    
    // CALLBACK_PROLOGUE

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    // allocate memory for field, I hope.
    h = MemHandleNew(sizeof(Char) * 12);
    p = MemHandleLock(h);
    ((Char *)p)[0] = '\0';
    MemPtrUnlock(p);
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_hag));
    FldSetTextHandle(fld, (Handle) h);
    // also set title
    //    if (store_youbuying) FrmCopyTitle(frm, "Your offer?");
    //    else FrmCopyTitle(frm, "Asking price?");
    FrmDrawForm(frm);
    // also print the name of the thing being haggled
    haggle_print_obj();
    if (store_youbuying)
      purchase_haggle_init();
    else
      sell_haggle_init();
    // haggle_form_enable_autoinc(0); // it starts out unusable anyway.
    handled = true;
    break;

  case keyDownEvent:
    // filter out all non-handled, i.e. non-numeric, characters.
    handled = true;
    chr = e->data.keyDown.chr;
    if ( IsDigit(GetCharAttr(), chr) ||
	 (chr == backspaceChr)       ||
	 (chr == leftArrowChr)       ||
	 (chr == rightArrowChr) )
      handled = false;
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_hag));
    autobid = 0;
    switch(e->data.ctlSelect.controlID) {
    case btn_hag_incr:
      // increment the current offer
      autobid = haggle_get_incr();
      handled = true;
      // fall through
    case btn_hag_all:
      // match the current offer
      if (!handled)
	autobid = haggle_get_ask();
      StrPrintF(buf, "%ld", autobid);
      h = (VoidHand) FldGetTextHandle(fld);
      if (!h)
	h = MemHandleNew(12 * sizeof(Char));
      FldSetTextHandle(fld, (Handle) 0);
      p = MemHandleLock(h);
      StrNCopy(p, buf, 12);
      MemHandleUnlock(h);
      FldSetTextHandle(fld, (Handle) h);
      // I think that will do the right thing.
      //      FrmDrawForm(frm);
      FldDrawField(fld);
      handled = true;
      break;
    case btn_hag_ok:
      textp = FldGetTextPtr(fld); // if (textP) ...
      // Well, the only problem with StrAToI is that I want StrAToL !!
      // If the string has length greater than 4, I may have to get funky.
      gold = StrAToI(textp);
      if (store_youbuying)
	outcome = purchase_haggle_endturn(gold);
      else
	outcome = sell_haggle_endturn(gold);
      switch (outcome) {
      case HAG_ONGOING: break;
      case HAG_BOUGHT:
	if (store_youbuying)
	  store_purchase_end(true);
	else
	  store_sell_end(true);
	break;
      case HAG_REJECTED:
	haggle_form_boot(false, "Quit");
	break;
      case HAG_BOOTED:
	haggle_form_boot(true, "blah");
	break;
      }
      handled = true;
      break;
    case btn_hag_exit: // cancelled the haggle or succeeded in purchase
      FldReleaseFocus(fld);
      FldSetSelection(fld, 0, 0);
      FldFreeMemory(fld);
      LeaveForm();
      store_gold_owner(true); // reprint what gold you have
      // also reprint items!  only needed if you succeeded in buying something.
      store_list_tens(0);
      handled = true;
      break;
    case btn_hag_boot: // got kicked out
      FldReleaseFocus(fld);
      FldSetSelection(fld, 0, 0);
      FldFreeMemory(fld);
      LeaveForm();
      // now leave the store form too!  better test this, sometime.
      free_store_list(FrmGetActiveForm());
      LeaveForm();
      in_store = STORE_NONE;
      in_a_store = false;
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

    // CALLBACK_EPILOGUE

  return handled;
}


/*

1. Enter shop .....................................................

         Oglign Dragon-Slayer   (Dwarf)      Weaponsmith
   Item                                                     Asking Price
a) a Broadsword (2d5) (+0,+0).                                   588
b) a Small Sword (1d6) (+0,+0).                                  110
c) a Bastard Sword (3d4) (+0,+0).                                806
d) a Mace (2d4) (+0,+0).                                         298
e) a Mace (2d4) (+0,+1).                                         529
f) a Mace (2d4) (+0,+0).                                         298
g) a Morningstar (2d6) (+0,+0).                                  913
h) a War Hammer (3d3) (+0,+0).                                   518
i) a Mace (2d4) (+0,+0).                                         298
j) a Light Crossbow (x3) (+0,+0).                                322
k) a Light Crossbow (x3) (+0,+0).                                322
l) a Sling (x2) (+0,+0).                                           9
                                                            - cont. -
                 Gold Remaining : 721

You may:
 p) Purchase an item.           b) Browse store's inventory.
 s) Sell an item.               i/e/t/w/x) Inventory/Equipment Lists.
ESC) Exit from Building.        ^R) Redraw the screen.


2. Purchase .....................................................

What do you offer?
Asking :  9

What do you offer? 1
Asking :  9

Hah!  Try again.  -more-
Asking :  9

Thou knave!  No less than 8 gold pieces.  -more-
Asking :  8                            Your last offer : 1

What do you offer? 2
Asking :  9

2 for such a fine item?  HA!  No less than 7.  -more-
Asking :  7                            Your last offer : 2

May your chickens grow lips.  I want 6 in gold!  -more-
Asking :  6                            Your last offer : 3

4?!?  You would rob my poor starving children?  -more-
Asking :  5                            Your last offer : 4

5 is my final offer; take it or leave it.  -more-
Final Offer :  5                       Your last offer : 4

What do you offer? [+1] 
Final Offer :  5                       Your last offer : 4

Accepted!  -more-
Final Offer :  5                       Your last offer : 4

You have a Sling (x2) (+0,+0). (d)


... Note you can exit bargaining at any time [you press ESCAPE]
... Note if you piss off the shopkeeper, you get kicked out & locked out


3. Sell .....................................................


Selling a Sling (x2) (+0,+0). (d)  -more-
Offer :  1

What price do you ask? 
Offer :  1

... Basically like buying.  The item is added to (or removed from) store list.


*/
