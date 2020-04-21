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

// Form to select a spell book, then select a spell.
static void show_books(FormPtr frm) SEC_4;
static void free_book_select(FormPtr frm) SEC_4;
static void count_spells(Short selected_book) SEC_4;
static void show_spells(ListPtr lst, Short first_sp, Boolean comment) SEC_4;
static void print_spell_ss(Short start, Short rows, Boolean browse) SEC_4;
static void print_spell_status(Short i, Short j) SEC_4;
static void print_spell_stats(Short i, Short j) SEC_4;
static void free_spell_select(ListPtr lst) SEC_4;
static Boolean confirmed_spell() SEC_4;
static void learn_the_spell(Short j, ListPtr lst) SEC_4;

Short selected_book = -1;
Short selected_spell = -1;
Char **book_display; // used for books or spells
Char **spell_display; // used for books or spells
Short book_ctr; // number of strings in book_display
Short spell_ctr; // number of strings in book_display
extern spell_type magic_spell[MAX_CLASS-1][MAX_SPELLS]; // magic.c
extern Short spells[MAX_SPELLS]; // spell_ctr of them will be filled in.
extern ULong spell_learned;
extern ULong spell_worked;
extern ULong spell_forgotten;

// why we're in the spell form
Boolean just_browsing_book = false;

Boolean Book_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  CharPtr label;
  Short s;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    selected_book = -1;
    show_books(frm);
    FrmDrawForm(frm);
    // make the right buttons visible - they are by default.
    handled = true;
    break;

    // No need to scroll - there are only 8 distinct books.
    /*
  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_book));
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      if (LstScrollList(lst, up, 10)) // there are 10 items visible at a time
	LstSetSelection(lst, -1);
      handled = true;
      break;
    case pageDownChr:
      if (LstScrollList(lst, down, 10))
	LstSetSelection(lst, -1);
      handled = true;
      break;
    }
    break;
    */

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_book_ok:
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_book));
      s = LstGetSelection(lst);
      if (s != -1) {
	label = LstGetSelectionText(lst, s);
	if (label) {
	  selected_book = label[0] - 'a';
	  count_spells(selected_book);
	  if (spell_ctr > 0) {
	    just_browsing_book = false;
	    FrmPopupForm(SpellForm);
	  } else
	    WinDrawChars("You don't know any in that book.", 32, 2, 128);
	}
      }
      handled = true;
      break;
    case btn_book_cancel:
      free_book_select(frm);
      LeaveForm();
      handled = true;
      break;
    }

  default:
    break;
  }
  return handled;
}

// I should really let the books have 2 lines each.
static void show_books(FormPtr frm)
{
  Short i, j;
  Char tmp_val[160], c;
  Int width_str, length_str;
  Boolean fits;
  ListPtr lst;
  UChar tval;

  if (class[pymisc.pclass].spell == PRIEST)
    tval = TV_PRAYER_BOOK;
  else if (class[pymisc.pclass].spell == MAGE)
    tval = TV_MAGIC_BOOK;
  else
    return;// should not happen!

  book_ctr = 0;
  for (i = 0 ; i < inven_ctr ; i++)
    if (inventory[i].tval == tval)
      book_ctr++;

  if (book_ctr == 0)
    return;// caller should have checked!

  // there must be at least one book.
  book_display = (Char **) md_malloc(sizeof(Char *) * book_ctr);

  for (i = 0, j = 0, c = 'a' ; i < inven_ctr ; i++, c++) {
    /* Print the items	  */
    //    if (mask == CNIL || mask[i]) {
    if (inventory[i].tval == tval) {
      //      objdes(tmp_val+3, &inventory[i], true); // XXXXxxx fix this.
      StrPrintF(tmp_val, "%c) %s", c,
		object_names + object_list[inventory[i].index].name_offset);

      // Truncate if too wide for the screen..... we only have 156 pixels...
      // length_str gets set to the # of chars that will fit in 156
      // width_str gets set to the # of pixels that those chars will use(<=156)
      width_str = 156;
      length_str = StrLen(tmp_val); // max chars to allow
      FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
      tmp_val[length_str] = '\0';
      // ok, now like put it somewhere.
      book_display[j] = (Char *) md_malloc(1 + length_str);
      StrCopy(book_display[j], tmp_val);
      j++;
    }
  }

  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_book));
  LstSetListChoices(lst, book_display, book_ctr);
  selected_book = -1;
  LstSetSelection(lst, -1);
}

/*
 *  Free the strings in the list
 */
static void free_book_select(FormPtr frm)
{
  ListPtr lst;
  Short i;
  VoidHand h;
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_book));
  LstSetListChoices(lst, NULL, 0);
  for (i = 0 ; i < book_ctr ; i++) {
    h = MemPtrRecoverHandle(book_display[i]);
    if (h) 
      MemHandleFree(h);
  }
  h = MemPtrRecoverHandle(book_display);
  if (h) 
    MemHandleFree(h);
}

////////////////////////////////////////////////////////////////////////

Boolean Spell_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  CharPtr label;
  Short s, first_spell;
  ULong j;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    FrmDrawForm(frm);
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_spell));
    j = inventory[selected_book].flags;
    first_spell = bit_pos(&j); // this modifies j
    show_spells(lst, first_spell, just_browsing_book);
    print_spell_ss(0, 10, just_browsing_book);
    // also fix the buttons:
    // if browse: hide "ok", change cancel's buttonlabel to "done"
    // if learn: hide "ok", *show "learn"*
    // create a new "learn" button, initially disabled,
    // and make its handler cause you to learn the selected spell.
    handled = true;
    break;

    // No need to scroll - the books each have 10 or fewer spells.  I _think_.
    // I will make Learning a separate form.
    /*
  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_spell));
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      if (LstScrollList(lst, up, 10)) // there are 10 items visible at a time
	LstSetSelection(lst, -1);
      handled = true;
      break;
    case pageDownChr:
      if (LstScrollList(lst, down, 10))
	LstSetSelection(lst, -1);
      handled = true;
      break;
    }
    break;
    */

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_spell_ok:
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_spell));
      s = LstGetSelection(lst);
      if (s != -1) {
	label = LstGetSelectionText(lst, s);
	if (label) {
	  selected_spell = label[0] - 'a';
	  if (confirmed_spell()) {
	    free_spell_select(lst);
	    LeaveForm();
	    LeaveForm(); // yes, TWICE.
	    // also CALL THE SPELL CASTING HELPER now.
	    if (class[pymisc.pclass].spell == MAGE)
	      cast_helper(1, selected_spell, spell_chance(selected_spell));
	    else
	      pray_helper(1, selected_spell, spell_chance(selected_spell));
	  }
	}
      }
      handled = true;
      break;
    case btn_spell_cancel:
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_spell));
      free_spell_select(lst); // hm, I get a Nil Pointer here. // fixed
      // (don't forget to NOT free something you allocated *0* bytes for)
      LeaveForm();
      handled = true;
      break;
    }

  default:
    break;
  }
  return handled;
}
static void count_spells(Short selected_book)
{
  Short k;
  ULong j;
  spell_type *s_ptr;

  // set spell_ctr to the number of known SPELLS from the given book
  //  j = inventory[selected_book].flags;
  //  first_spell = bit_pos(&j); // this modifies j
  j = inventory[selected_book].flags & spell_learned;
  s_ptr = magic_spell[pymisc.pclass-1];
  spell_ctr = 0;
  while (j) {
    k = bit_pos(&j);
    if (s_ptr[k].slevel <= pymisc.lev) {
      spells[spell_ctr] = k;
      spell_ctr++;
    }
  }
}

/* Note - may also want to use this form for:
   browsing a book (list all n<10 spell names, and status instead of #s)
   [selecting a spell to learn (there might be >10, rats.  how likely??)] */
// so, caller can have a different spell_ctr and spells[]
// also need a boolean argument to indicate "print numbers or other stuff"
static void show_spells(ListPtr lst, Short first_spell, Boolean comment)
{
  Short i, offset, j;
  //  ULong j;
  Char tmp_val[160], c;
  Int width_str, length_str;
  Boolean fits;

  // spell_ctr, spells[] have ALREADY BEEN SET by caller.

  // malloc spell_display - there must be at least 1 spell.
  spell_display = (Char **) md_malloc(sizeof(Char *) * spell_ctr);
  // print strings
  // this was in:      print_spells (spell, num, FALSE, first_spell);
  offset = (class[pymisc.pclass].spell==MAGE ? SPELL_OFFSET : PRAYER_OFFSET);
  for (i = 0 ; i < spell_ctr ; i++) {
    j = spells[i];
    // comment == false, so no "forgotten/unknown/untried" here
    c = 'a' + j - first_spell;
    StrPrintF(tmp_val, "%c) %s", c, spell_names[j+offset]);
    width_str = 156;
    length_str = StrLen(tmp_val); // max chars to allow
    FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
    tmp_val[length_str] = '\0';
    // ok, now like put it somewhere.
    spell_display[i] = (Char *) md_malloc(1 + length_str);
    StrCopy(spell_display[i], tmp_val);
  }
  // set ui foo
  //  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_spell));
  LstSetListChoices(lst, spell_display, spell_ctr);
  selected_spell = -1;
  LstSetSelection(lst, -1);
  LstDrawList(lst);
}
// Print something to the right side of all the spells.
static void print_spell_ss(Short start, Short rows, Boolean browse)
{
  RectangleType r;
  Short i, j;
  RctSetRectangle(&r, 113, 15, 160-113, 10*11);
  WinEraseRectangle(&r, 0);

  if (rows > spell_ctr) rows = spell_ctr;
  if (!browse) {
    WinDrawChars("Lv-Mn-Fail", 10,   111, 15);
    for (i = start ; i < rows ; i++) {
      j = spells[i];
      print_spell_stats(i-start, j); // where to draw, what to draw
    }
  } else {
    WinDrawChars("Status", 6,   111, 15);
    for (i = start ; i < rows ; i++) {
      j = spells[i];
      print_spell_status(i-start, j); // words instead of numbers
    }
  }
}

// show for browsing
static void print_spell_status(Short i, Short j)
{
  Short x = 115, y = 26;
  Char *p;
  if ((spell_forgotten & (1L << j)) != 0)
    p = "forgotten";
  else if ((spell_learned & (1L << j)) == 0)
    p = "unknown";
  else if ((spell_worked & (1L << j)) == 0)
    p = "untried";
  else
    return;
  WinDrawChars(p, StrLen(p), x, y + 11*i);
}
// show this if you are Casting [or Learning] a spell (not browsing)
static void print_spell_stats(Short i, Short j)
{
  //    StrPrintF(tmp_val, "%d %d %d%%", // each %d is 1- or 2-digit
  //	      s_ptr->slevel, s_ptr->smana, spell_chance(j));
  Short x = 113, y = 26;
  spell_type *s_ptr;
  Char tmp_val[10];
  s_ptr = &magic_spell[pymisc.pclass-1][j];
  StrPrintF(tmp_val, "%d", s_ptr->slevel);
  if (s_ptr->slevel > 9) WinDrawChars(tmp_val, 2, x, y + 11*i);
  else                   WinDrawChars(tmp_val, 1, x+5, y + 11*i);
  x += 13; // "%2d "
  StrPrintF(tmp_val, "%d", s_ptr->smana);
  if (s_ptr->smana > 9) WinDrawChars(tmp_val, 2, x, y + 11*i);
  else                  WinDrawChars(tmp_val, 1, x+5, y + 11*i);
  x += 13; // "%2d "
  StrPrintF(tmp_val, "%d%%", spell_chance(j));
  x += 5 * (3 - StrLen(tmp_val));
  WinDrawChars(tmp_val, StrLen(tmp_val), x, y + 11*i);
}

static void free_spell_select(ListPtr lst)
{
  //  ListPtr lst;
  Short i;
  VoidHand h;
  LstSetListChoices(lst, NULL, 0);
  for (i = 0 ; i < spell_ctr ; i++) {
    h = MemPtrRecoverHandle(spell_display[i]);
    if (h) 
      MemHandleFree(h);
  }
  h = MemPtrRecoverHandle(spell_display);
  if (h) 
    MemHandleFree(h);
}

static Boolean confirmed_spell()
{
  if (magic_spell[pymisc.pclass-1][selected_spell].smana <= pymisc.cmana)
    return true;
  if (class[pymisc.pclass].spell == MAGE)
    return (0 == FrmAlert(LowMageP));
  else
    return (0 == FrmAlert(LowPriestP));
}




////////////////////////////////////////////////////////////////////////

// This form will have only mage spells in it
// Unlike the book form and the browse/cast form, it might need scrolled.
// (damn)

// Hey - news flash.  This isn't an ok->exit form
// This is a loop until out of new spells form.
// SOOOO we need a "new spells left" display line,
// annnnd we need to be able to subtract lines from the list!!
// so, er, actually this is also the form to double-up for browsing.
extern Boolean missing_a_book;
extern ULong spell_learned;
extern UChar spell_order[MAX_SPELLS+1];
extern Short new_spells, last_known;
static Boolean stats_toggler;
Boolean GainSp_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  ControlPtr btn;
  Short start;
  Char buf[40];

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    // This form will ONLY be entered if we have > 0 new_spells possible.
    // (And make sure there are still spells left to learn, eh)
    FrmDrawForm(frm);
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_gain));
    show_spells(lst, 0, false);
    print_spell_ss(0, 9, just_browsing_book);
    if (!just_browsing_book) {
      if (missing_a_book)
	StrPrintF(buf, "You seem to be missing a book.");
      else {
	StrPrintF(buf, "You can learn %d %s", new_spells,
		  ((new_spells == 1) ? "spell" : "spells"));
      }
    } else {
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_gain_ok));
      CtlHideControl(btn);
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_gain_tog));
      CtlShowControl(btn);
      StrPrintF(buf, "Browsing: %s", object_names +
		object_list[inventory[selected_book].index].name_offset);
      // also it would be a good plan to change the title bar.
      FrmCopyTitle(frm, "Browsing...");
      stats_toggler = false; // what it should display next time
    }
    WinDrawChars(buf, StrLen(buf),  2, 128);
    handled = true;
    break;

    // need to be able to scroll - a mage of "average" ability might
    // have 12 or so learnable spells at late-teen levels.
    // so, I don't know how to catch use of the little arrows in the list.
    // this is a pain.  might have to remember previous topItem
    // and check every penDownEvent,
    // or mess with LstSetDrawFunction which sets drawItemsCallback.
    // scrolling below HAS NOT BEEN TESTED and above HAS NOT BEEN IMPLEMENTED
  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_gain));
    start = lst->topItem;
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      if (LstScrollList(lst, up, 9)) // there are 9 items visible at a time
	LstSetSelection(lst, -1);
      // xxxxx ALSO need to reprint the stats alongside.  plan how to deal..
      // heck, could just allocate an array of shorts to reprint from.
      // it's not like we use this form very often and would hate the overhead
      print_spell_ss(start, min(start+9, spell_ctr), false);
      handled = true;
      break;
    case pageDownChr:
      if (LstScrollList(lst, down, 9))
	LstSetSelection(lst, -1);
      // xxxxx ALSO need to reprint the stats alongside.
      print_spell_ss(start, min(start+9, spell_ctr), false);
      handled = true;
      break;
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_gain_tog:
      print_spell_ss(0, 9, stats_toggler);
      stats_toggler = !stats_toggler;
      break;
    case btn_gain_ok:
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_gain));
      selected_spell = LstGetSelection(lst);
      if (selected_spell != -1) {
	learn_the_spell(selected_spell, lst);
	// DO NOT LEAVE FORM, just unselect spell + remove it from the list.
	if (new_spells <= 0) {
	  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_gain_ok));
	  CtlHideControl(btn);
	}
      }
      StrPrintF(buf, "You can learn %d %s", new_spells,
		((new_spells == 1) ? "spell" : "spells"));
      WinDrawChars(buf, StrLen(buf),  2, 128);
      handled = true;
      break;
    case btn_gain_cancel:
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_gain));
      free_spell_select(lst);
      LeaveForm();
      // It should be safe to call this whether we picked 0 or n spells.
      // the original moria did in any event.
      if (!just_browsing_book)
	gain_spell_helper();
      handled = true;
      break;
    }

  default:
    break;
  }
  return handled;
}


// hey, this should be in misc3.c and avoid 'extern', or else the stuff there
// should be here.
//extern ULong spell_learned;
//extern UChar spell_order[MAX_SPELLS+1];
//extern Short new_spells, last_known;
//
// XXX Note:  I have a memory leak here because I am not freeing
// the orphaned spell_display[0].
// if I do, the free_spell_select gets an invalid chunk ptr.
// same thing if I rearrange the strings.  I don't understand why.
// HYPOTHESIS: people cannot play enough moria on one set of batteries
// in one session to make this a serious leak, so just leave it in.
static void learn_the_spell(Short j, ListPtr lst)
{
  //  VoidHand h;
  //  Char *tmp;
  new_spells--; // decrement number of spells avail
  spell_learned |= 1L << spells[j]; // learn it
  spell_order[last_known++] = spells[j]; // remember order it was learned in
  //  return;
  LstSetListChoices(lst, NULL, 0);
  //  tmp = spell_display[0];
  //  h = MemPtrRecoverHandle(spell_display[0]);
  //  if (h) MemHandleFree(h); // delete the unneeded string
  for (; j < spell_ctr-1; j++) {
    spells[j] = spells[j+1]; // delete this spell from array
    spell_display[j] = spell_display[j+1]; // delete this spell from List
  }
  //  spell_display[spell_ctr-1] = tmp; // will this work?  NO.
  spell_ctr--;
  // also, redraw the freaking list,
  // and the numbers next to it.
  LstSetListChoices(lst, spell_display, spell_ctr);
  selected_spell = -1;
  LstSetSelection(lst, -1);
  LstDrawList(lst);
  print_spell_ss(0, 9, false);
}
// Note - there's a fatal error in there somewhere.  No real surprise
// it's just bubble gum and baling wire.

