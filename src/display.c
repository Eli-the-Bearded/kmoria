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
#include <System/Globals.h> // for font foo
#include "kMoriaRsc.h"
#include "kMoria.h"
#include "lock-externs.h"
// these live in lock.c
extern Short cur_height, cur_width;

extern MoriaPreferenceType my_prefs;

static void print_stats_large(Boolean siwdcc);
static void print_stats_itsy(Boolean siwdcc);
static void print_stats_real(UChar which_stats, Boolean large) SEC_5;
static void draw_circle(Short radius, Short x0, Short y0) SEC_5;

// Ok.  I have how much screen?
// Top line sometimes taken by messages
// Bottom two lines taken

/* The screen can have 15 rows, but I took 2 for stats */
//#define visible_h 11
#define visible_h 13
/* You can put about 19.9 m's on a memo line.. so.. we'll have 20 cols */
#define visible_w 20
/* Height in pixels of one character ... in this case an 'M' */
#define visible_char_h 10
/* Width in pixels of one character ... 160 pixels / 20 col = 8 pix/col */
#define visible_char_w 8

// itsy is a smaller font
//#define visible_h_itsy 17
#define visible_h_itsy 22
#define visible_w_itsy 32
#define visible_char_h_itsy 6
#define visible_char_w_itsy 5

Boolean itsy_on = false;

/* This is the offset to convert character-positions in
   the "visible" part of the screen represented by 'terminal'
   to the larger "real" dungeon represented by 'buffer'. */
Short visible_x = 0, visible_y = 0;

// From Ularn's constant.h, these are 6*11 down by 6*33 across.
#define DROWS MAX_HEIGHT
#define DCOLS MAX_WIDTH

Char terminal[visible_h_itsy][visible_w_itsy];/*relative: model of the screen*/
Char buffer[DROWS][DCOLS]; /* absolute - a model of the dungeon */
Boolean screen_dirty, stats_dirty = true;
Boolean lines_dirty[DROWS]; /* absolute */


Boolean was_siwdcc = true;
Boolean pending_messages;
Char *old_messages[SAVED_MSGS];
Short last_old_message_shown;
extern UChar need_print_stats;

/*
 * Switch between the small font and the regular font.
 * (Do NOT allow the small font if the SDK that I compiled with
 * is inappropriate for your operating system version.)
 */
Boolean toggle_itsy()
{
  DWord version;
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
#ifdef I_AM_OS_2
  if (version < 0x03000000l) {
    itsy_on = !itsy_on;
    return true;
  } else itsy_on = false;
#else
  if (version >= 0x03000000l) {
    itsy_on = !itsy_on;
    return true;
  } else itsy_on = false;
#endif
  return false;
}


// I also need a routine to translate a tap on the screen
// into absolute-dungeon coordinates.
// This has not been debugged.
void where_in_dungeon(Short scr_x, Short scr_y, Short *dun_x, Short *dun_y)
{
  Short vc_w = itsy_on ? visible_char_w_itsy : visible_char_w;
  Short vc_h = itsy_on ? visible_char_h_itsy : visible_char_h;
  *dun_x = scr_x / vc_w;
  *dun_y = scr_y / vc_h;
  *dun_x += visible_x;
  *dun_y += visible_y;
  return;
}


/* 
 *  This will clear the part of the display that belongs to
 *  "the dungeon".  It also clears 'terminal' since that is
 *  a representation of the display.
 *  This should be called before the offset for 'terminal' is
 *  moved to show a different part of 'buffer'.
 */
// Actually, the Moria equivalent of this clears the WHOLE screen.
void clear_screen() 
{
  Short i, j;
  Short v_h = itsy_on ? visible_h_itsy : visible_h;
  Short v_w = itsy_on ? visible_w_itsy : visible_w;
  RectangleType r;
  if (FrmGetActiveFormID() != MainForm) return;

  /* Clear the physical screen. */
  RctSetRectangle(&r, 0, 0, 160, 134-1);
  WinEraseRectangle(&r, 0);

  /* Update 'terminal' to represent the cleared screen. */
  screen_dirty = true;
  for (i = 0 ; i < v_h; i++) {
    lines_dirty[i+visible_y] = true;
    for (j = 0; j < v_w; j++) {
      terminal[i][j] = ' ';
    }
  }
}

void print(Short ch, Short row, Short col) {
  buffer[row][col] = ch;
  lines_dirty[row] = true;
  screen_dirty = true;
}



/*
 * Draw the whole screen, as the player knows it.
 * On second thought, maybe update but DON'T draw.
 */
void recalc_screen()
{
  Short row, col;// absolute
  if (FrmGetActiveFormID() != MainForm) return;
  clear_screen();
  for (row = 0 ; row < cur_height ; row++)
    for (col = 0 ; col < cur_width ; col++)
      buffer[row][col] = loc_symbol(row, col);
  //      buffer[row][col] = '^';
  //  for (row = visible_y, vr = 0 ; vr < visible_h_itsy ; row++, vr++)
  //    for (col = visible_x, vc = 0 ; vc < visible_w_itsy ; col++, vc++)
  //      terminal[vr][vc] = buffer[row][col];
  /*refresh();*/
}



/*
 * The funtion to actually draw a character ch at a col,row position
 * (row, col are RELATIVE i.e. this is used in model-of-screen)
 * note the row+1 offset is so that we have a line for msgs.
 * The character will be drawn in either the small or normal font.
 */
static void put_char_at(Short row, Short col, Char ch, Boolean bold)
{
  //  Short cheat, vcheat = itsy_on ? 14 : 5; // center the dungeon vertically
  Short cheat, vcheat = itsy_on ? 0 : 0; // center the dungeon vertically
  /* the space has already been cleared with clear_line */
  /* on second thought, no it hasn't. */
  RectangleType r;
  Short vc_w = itsy_on ? visible_char_w_itsy : visible_char_w;
  Short vc_h = itsy_on ? visible_char_h_itsy : visible_char_h;

  RctSetRectangle(&r, col * vc_w, row*vc_h+vcheat,
		  vc_w, vc_h); /* Left, Top, W, H */
  WinEraseRectangle(&r, 0); /* 0 for square corners */
    
  // calculate pixel position of "row, col" and put char there
  cheat = vc_w - FntCharWidth(ch); // center the variable width characters

  if (cheat <= 1)   cheat = 0;
  else              cheat /= 2;
  if (ch != ' ') {
    if (!itsy_on && (ch== 'g' || ch== 'j' || ch== 'p' ||ch == 'q' ||ch == 'y'))
      // unfortunately, letters with dangling bits are a pain.
      WinDrawChars(&ch, 1, col * vc_w + cheat, row * vc_h+vcheat-1);
    else
      WinDrawChars(&ch, 1, col * vc_w + cheat, row * vc_h+vcheat);
    if (bold)  WinInvertRectangle(&r, 0); /* 0 for square corners */
  }
  terminal[row][col] = ch;

}

/*
 *  Actually redraw the screen.  (if it is 'dirty'.)
 */
void refresh()
{
  Short col, line, absline;
  Short v_h = itsy_on ? visible_h_itsy : visible_h;
  Short v_w = itsy_on ? visible_w_itsy : visible_w;
  if (FrmGetActiveFormID() != MainForm) return;
  if (!screen_dirty) return;
  screen_dirty = false;

#ifdef I_AM_OS_2
  if (itsy_on)
    FntSetFont(ledFont);
#else
  if (itsy_on)
    FntSetFont(129);
#endif
  v_w = min(v_w, cur_width  - visible_x);
  v_h = min(v_h, cur_height - visible_y);

  // I forget why I drew it bottom-to-top... 
//    for (line = v_h - 1; line >= 0; line--) {
  for (line = 0 ; line < v_h ; line++) {
    /* line is RELATIVE, line+visible_y is ABSOLUTE. */
    absline = line + visible_y;
    if (!lines_dirty[absline]) continue;
    lines_dirty[absline] = false;
    for (col = 0; col < v_w; col++) {
      /* col is RELATIVE, col+visible_x is ABSOLUTE. */
      if (col+visible_x >= cur_width) break;
      if (buffer[absline][col+visible_x] != terminal[line][col]) {
	put_char_at(line, col, buffer[absline][col+visible_x], false);
      }
    }
  }
  //  WinDrawLine(0,12,160,12);
  //  spell_stats(env); // don't forget the spell-status on right hand side.
#ifdef I_AM_OS_2
  if (itsy_on)
    FntSetFont(stdFont);
#else
  if (itsy_on)
    FntSetFont(stdFont);
#endif
  /* I might eventually have a pref to draw run/walk circles. This code works*/
  if (my_prefs.overlay_circles && !my_prefs.relative_move) {
    Short center_x = 80, center_y = 80;
    /*
    if (my_prefs.relative_move) {
      // eh, probably this will be right.  Center it around the '@'
      center_x = (char_col - visible_x) * (itsy_on ? visible_char_w_itsy :
					   visible_char_w);
      center_y = (char_row - visible_y) * (itsy_on ? visible_char_h_itsy :
					   visible_char_h);
      // Come to think of it this will make the screen look like utter crap.
      // Ok - I will not allow circle overlay && relative_move until I
      // figure out how to resolve this sad little issue.
      // Also rewrite this to use relativize_move.
    }
    */
    draw_circle(my_prefs.walk_center_border * 10, center_x, center_y);
    if (my_prefs.run)
      draw_circle(my_prefs.run_walk_border * 10, center_x, center_y);
  }
}

/*
 * Given input SCREEN coordinates x,y,
 * Translate x,y such that the onscreen location of '@' is mapped to 0,0
 */
void relativize_move(Short *x, Short *y)
{
  Short center_x, center_y;
  center_x = (char_col - visible_x) * (itsy_on ? visible_char_w_itsy :
				       visible_char_w);
  center_y = (char_row - visible_y) * (itsy_on ? visible_char_h_itsy :
				       visible_char_h);
  *x -= center_x;
  *y -= center_y;
}


// y,x are ABSOLUTE row,col.
void change_visible_xy(Short x, Short y)
{
  y = max(y, 0);
  x = max(x, 0);
  y = min(y, cur_height - (itsy_on ? visible_h_itsy : visible_h));
  x = min(x, cur_width - (itsy_on ? visible_w_itsy : visible_w));
  if (visible_y == y && visible_x == x) return;  // no change!
  visible_y = y;
  visible_x = x;
  recalc_screen();
  refresh();

  // this is like panel_bounds()
  panel_row_min = visible_y;
  panel_row_max = visible_y + (itsy_on ? visible_h_itsy : visible_h) - 1;
  panel_col_min = visible_x;
  panel_col_max = visible_x + (itsy_on ? visible_w_itsy : visible_w) - 1;

}
// x and y can be -1, 0, 1.
static Short dirx[9] = { -1,  0,  1, -1,  0,  1, -1,  0,  1 };
static Short diry[9] = {  1,  1,  1,  0,  0,  0, -1, -1, -1 };
void slide_dir(Short dir)
{
  Short x, y;
  dir = max(min(dir,9),1); // keep in bounds [1,9]
  x = dirx[dir-1];
  y = diry[dir-1];
  y *= (itsy_on ? visible_h_itsy : visible_h);
  x *= (itsy_on ? visible_w_itsy : visible_w);
  change_visible_xy(x+visible_x,y+visible_y);
}


// XXX Odd behavior, when you enter a room from the left, only the
// part in the left half of the screen is lit.  is this due to wackiness
// in "what panel am I in"??
Boolean get_panel(Short y, Short x, Boolean force)
{
  Short screen_width = itsy_on ? visible_w_itsy : visible_w; // unit = cells
  Short screen_height = itsy_on ? visible_h_itsy : visible_h; // unit = cells
  Short epsilon_x = 3, epsilon_y = 2; // # of border cells you "can't get to"
  Short new_visible_x, new_visible_y;
  Boolean in_new_panel = true;
  // cur_height and cur_width are the # of cells in CURRENT floor's dimensions
  if (!(force ||
	(x < visible_x + epsilon_x) ||
	(x >= visible_x + screen_width - epsilon_x) ||
	(y < visible_y + epsilon_y) ||
	(y >= visible_y + screen_height - epsilon_y)))
    return false; // no need to change panels

  // CALCULATE X   of top left corner of the panel to move to
  // This is a little ugly in order to deal with possibly odd screen_width.
  new_visible_x = ((x - epsilon_x)/screen_width) * screen_width;
  panel_col = ((x - epsilon_x)/screen_width) * 2; // panels overlap by width/2
  if ((x - epsilon_x) % screen_width >= screen_width/2) {
    new_visible_x += screen_width/2;
    panel_col++;
  }
  // Boundary conditions.
  if (new_visible_x > cur_width - screen_width)
    new_visible_x = cur_width - screen_width;
  if (new_visible_x < 0)
    new_visible_x = 0;
  // CALCULATE Y   of top left corner of the panel to move to
  // This is a little ugly in order to deal with possibly odd screen_width.
  new_visible_y = ((y - epsilon_y)/screen_height) * screen_height;
  panel_row = ((y - epsilon_y)/screen_height) * 2; // panels overlap by h/2
  if ((y - epsilon_y) % screen_height >= screen_height/2) {
    new_visible_y += screen_height/2;
    panel_row++;
  }
  // Boundary conditions.
  if (new_visible_y > cur_height - screen_height)
    new_visible_y = cur_height - screen_height;
  if (new_visible_y < 0)
    new_visible_y = 0;
  
  if (visible_x == new_visible_x && visible_y == new_visible_y)
    in_new_panel = false; // Ha!  There was no change.  Silly rabbit.

  visible_x = new_visible_x;
  visible_y = new_visible_y;
  panel_row_min = visible_y;
  panel_row_max = visible_y + screen_height - 1;
  panel_col_min = visible_x;
  panel_col_max = visible_x + screen_width - 1;

  if (in_new_panel)
    if (find_bound)
      end_find(); // stop running, etc.

  // Caller decides whether to recalc_screen & refresh.

  return in_new_panel;
}


/*
 * Set the buffer value to what it "should" be "now"
 * but it will get actually drawn when the screen is refreshed
 */
//void update_screen_cell(Short j, Short i, struct everything *env)


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Here is the code formerly known as stats.c.
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/*
  S00 I00 W00 D00 C00 Ch00  0000 feet
  L 00 xp 0123456 mana 000 hp 000/000
  AC 00 Gold ...
*/

// HEY - revise this so that it just queues up the messages,
// and have another routine print them at the end of the turn
// (with --more-- as necessary)
void count_message(Char *buf)
{
  Short i;
  i = command_count;
  message(buf);
  command_count = i;
}
void message(Char *buf)
{
  Word curfrm = FrmGetActiveFormID();
  Short i;
  Char *tmp;
  if (curfrm == MainForm ||
      curfrm == CharSummaryForm) {    // Log a message for later display
    // Rats, I wonder why the "you can learn spells" is absent
    if (!buf) return;
    tmp = old_messages[0];
    for (i = 0 ; i < SAVED_MSGS-1 ; i++)
      old_messages[i] = old_messages[i+1];
    StrNCopy(tmp, buf, SAVED_MSG_LEN-2); /* leave room for \n.. */
    tmp[SAVED_MSG_LEN-2] = '\0'; /* making sure it's terminated.. */
    StrCat(tmp, "\n");
    old_messages[SAVED_MSGS-1] = tmp;
    // this part is for "--more--" ability:
    last_old_message_shown--;
    if (last_old_message_shown < 0) // "none of these msgs have been shown"
      last_old_message_shown = -1;
    // Remember that we have something to display
    pending_messages = true;
    command_count = 0;  // (What a hack!)
  } else if (curfrm == InvForm || curfrm == EquForm) {
    // These messages should be printed straightaway.
    RectangleType r;
    RctSetRectangle(&r,0,128,156,11); /* left,top, width and height */
    WinEraseRectangle(&r, 0); /* 0 for square corners */
    if (buf)
      WinDrawChars(buf, StrLen(buf), 1, 128);
  }
}
// a mainform message that we require to be displayed in "real time"
void alert_message(Char *buf)
{
  RectangleType r;
  if (FrmGetActiveFormID() != MainForm || !buf) return;
  RctSetRectangle(&r,0,134,160,160-134); /* left,top, width and height */
  WinEraseRectangle(&r, 0); /* 0 for square corners */
  if (buf)
    WinDrawChars(buf, StrLen(buf), 1, 134);
}
// Clear the message area
Boolean message_clear()
{
  RectangleType r;
  RctSetRectangle(&r,0,134,160,160-134); /* left,top, width and height */
  if (false)
    WinEraseRectangle(&r, 0);
  WinDrawLine(0,133,160,133);
  //print_stats(STATS_QUO);
  if (last_old_message_shown < SAVED_MSGS-1)  return true;
  else return false;
}

/*
 * Blow away the message queue
 * (Use this when you die)
 */
void preempt_messages()
{
  pending_messages = false;
  last_old_message_shown = SAVED_MSGS - 1;  
}

// Print up to 2 lines of messages.  make sure you word-wrap.
// 1. print first message
// 2. if necessary, word wrap
// 3. else if second message exists + does not need word wrap, print it too.
// 4. else if second message does not exist, print teensy stats.
// 5. print "--more--" if any more messages are in the queue
void show_messages()
{
  Short lines_avail = 2, len_line, len_str, lines_used = 0;
  Char *msg;

  RectangleType r;
  RctSetRectangle(&r,0,134,160,160-134); /* left,top, width and height */

  // if there's nothing to print, just print stats.
  if (last_old_message_shown >= SAVED_MSGS-1) {
    print_stats_real(need_print_stats, true);
  } else {
    WinEraseRectangle(&r, 0);
    WinDrawLine(0,133,160,133);
    while (lines_avail > 0 && last_old_message_shown < SAVED_MSGS-1) {
      msg = old_messages[last_old_message_shown+1];
      len_str = StrLen(msg);
      if (lines_avail <= 1 && FntCharsWidth(msg, len_str-1) > SCR_WIDTH)
	break; // 1 line avail., message is 2 lines.  Do not print partial msg.
      while (lines_avail > 0) {
	len_str = StrLen(msg);
	len_line = FntWordWrap(msg, SCR_WIDTH);
	if (len_line >= len_str) { // we have finished the string!
	  WinDrawChars(msg, len_str-1, 0, 134 + 11*lines_used);
	  lines_avail--; lines_used++;
	  break;
	}
	WinDrawChars(msg, len_line, 0, 134 + 11*lines_used);
	lines_avail--; lines_used++;
	msg += len_line;
      }    // we finished the string, or we ran out of space.
      last_old_message_shown++;
    } // finished all messages or ran out of space.
    if (last_old_message_shown < SAVED_MSGS-1) {
      DWord version;
      Boolean large = false;
      FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
#ifdef I_AM_OS_2
      if (version < 0x03000000l) FntSetFont(ledFont);
      else large = true;
#else
      if (version >= 0x03000000l) FntSetFont(129);
      else large = true;
#endif
      //    WinDrawChars("--more--", 8, 120, 134 + 11*lines_used);
      WinDrawChars("MORE", 8, (large ? 130 : 140), 134 + 11*lines_used - 3);
#ifdef I_AM_OS_2
      FntSetFont(stdFont);
#else
      FntSetFont(stdFont);
#endif
    }
    if (lines_used < 2)
      print_stats_real(need_print_stats, false);
    else stats_dirty = true;
  }
  pending_messages = false;
  need_print_stats = STATS_QUO;
}

// call alloc_message_log initially when all stuff is being allocated..
// it has to be dynamic so we can play games with pointers in log_message.
void alloc_message_log()
{
  Short i;
  for (i = 0 ; i < SAVED_MSGS ; i++) {
    old_messages[i] = (Char *) md_malloc(sizeof(Char) * SAVED_MSG_LEN);
    old_messages[i][0] = '\0';
  }
  // the log is all null strings.. so (we pretend) all of it has been shown
  last_old_message_shown = SAVED_MSGS - 1;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


// See prt_stat_block() in misc3.c .. there are other things I should print
void print_stats(UChar which_stats)
{
  need_print_stats = which_stats;
  stats_dirty = true;
  return; // let's see if this sorta works.
}
static void print_stats_real(UChar which_stats, Boolean large)
{

  if (FrmGetActiveFormID() != MainForm) return;
  // decide what to show for first line of stats

  switch (which_stats) {
  case STATS_QUO: break;
  case STATS_TOGGLE:
    was_siwdcc = !was_siwdcc;
    break;
  case STATS_GOLD_AC:
    was_siwdcc = false;
    break;
  case STATS_STR_ETC:
    was_siwdcc = true;
    break;
  }
  if (large) {
    print_stats_large(was_siwdcc);
  } else
    print_stats_itsy(was_siwdcc);
}
/*
 * Print two lines of status at the bottom of screen
 */
Short statwidth_large[6] = { 5,7,8,6,5,10 };// also, width of "%2d " is 12
static void print_stats_large(Boolean siwdcc)
{
  Char buf[80];
  Short hi, x, i, tmp, w, depth;
  RectangleType r;

  if (FrmGetActiveFormID() != MainForm) return;
  if (!stats_dirty) return;
  RctSetRectangle(&r,0,134,160,160-134); /* left,top, width and height */
  WinEraseRectangle(&r, 0); /* 0 for square corners */
  WinDrawLine(0,133,160,133);

  // Tap bottom line of screen to toggle what the first line says
  
  hi = 138;

  x = 1;
  if (siwdcc) { // I need to do this differently.
    // need to print each number at a fixed position, also
    // subtract 18 if > 18 (and print '00' if 100) and invert the display.
    StrPrintF(buf, "S00 In00 W00 D00 C00 Ch00");
    WinDrawChars(buf, StrLen(buf), x, hi);
    for (i = 0, w = x ; i < 6 ; i++, w+=12) {
      w += statwidth_large[i];
      tmp = pystats.use_stat[i];
      if (tmp > 18) {
	tmp -= 18;
	if (tmp > 99) tmp = 0;
	StrPrintF(buf, (tmp > 9 ? "%d" : "0%d"), tmp);
	WinDrawInvertedChars(buf, StrLen(buf), w, hi);
      } else {
	StrPrintF(buf, "%d", tmp);
	WinDrawChars(buf, StrLen(buf), (tmp > 9) ? w : w+5, hi);
      }
    }
  } else {
    StrPrintF(buf, "AC %d  $ %ld", pymisc.dis_ac, pymisc.au);
    WinDrawChars(buf, StrLen(buf), x, hi);
  }

  x += 115;
  depth = dun_level * 50;
  if (depth > 0)
    StrPrintF(buf, "%d feet", depth);
  else 
    StrPrintF(buf, "Town");
  WinDrawChars(buf, StrLen(buf), x, hi);
  // done with first line
  x = 1;
  hi += 11;

  // player level 12
  StrPrintF(buf, "L %d", (Short) pymisc.lev);
  WinDrawChars(buf, StrLen(buf), x, hi);
  x += 20;
  // player experience 1234567
  StrPrintF(buf, "x %ld", pymisc.exp);
  WinDrawChars(buf, StrLen(buf), x, hi);
  x += 45;
  // mana points 123
  StrPrintF(buf, "mana %d", pymisc.cmana);
  WinDrawChars(buf, StrLen(buf), x, hi);
  x += 43;
  // hit points 123/123
  StrPrintF(buf, "hp %d/%d", pymisc.mhp, pymisc.chp);
  WinDrawChars(buf, StrLen(buf), x, hi);

  // xxxxxxxxxxx also need FLAGS: hungry/weak, blind, confused, fear, poisoned,
  // and also search/rest/speed/study!
  stats_dirty = false;
  return;
}


static void print_stats_itsy(Boolean siwdcc)
{
  Char buf[80];
  Short hi, x, depth;

  if (FrmGetActiveFormID() != MainForm) return;

#ifdef I_AM_OS_2
  FntSetFont(ledFont);
#else
  FntSetFont(129);
#endif
  
  hi = 160-12;

  x = 1;
  if (siwdcc) {
    // Dude!  Fix this!  Do that same inverse-video stats thing too.
    StrPrintF(buf, "S%d In%d W%d D%d C%d Ch%d", 
	      pystats.max_stat[0],
	      pystats.max_stat[1],
	      pystats.max_stat[2],
	      pystats.max_stat[3],
	      pystats.max_stat[4],
	      pystats.max_stat[5]);
    WinDrawChars(buf, StrLen(buf), x, hi);
  } else {
    StrPrintF(buf, "AC %d  $ %ld", pymisc.dis_ac, pymisc.au);
    WinDrawChars(buf, StrLen(buf), x, hi);
  }

  x += 115;
  depth = dun_level * 50;
  if (depth > 0)
    StrPrintF(buf, "%d feet", depth);
  else 
    StrPrintF(buf, "Town");
  WinDrawChars(buf, StrLen(buf), x, hi);
  // done with first line
  x = 1;
  hi = 160-6;

  // player level 12
  StrPrintF(buf, "L %d", (Short) pymisc.lev);
  WinDrawChars(buf, StrLen(buf), x, hi);
  x += 20;
  // player experience 1234567
  StrPrintF(buf, "/ %ld", pymisc.exp);
  WinDrawChars(buf, StrLen(buf), x, hi);
  x += 45;
  // mana points 123
  StrPrintF(buf, "Mana %d", pymisc.cmana);
  WinDrawChars(buf, StrLen(buf), x, hi);
  x += 43;
  // hit points 123/123
  StrPrintF(buf, "HP %d/%d", pymisc.mhp, pymisc.chp);
  if (pymisc.chp < pymisc.mhp / 4) // might want to make this tweakable
    WinDrawInvertedChars(buf, StrLen(buf), x, hi);
  else
    WinDrawChars(buf, StrLen(buf), x, hi);

#ifdef I_AM_OS_2
  FntSetFont(stdFont);
#else
  FntSetFont(stdFont);
#endif

  stats_dirty = true;
  return;
}


// Draw a circle of radius radius (pixels) without using "sqrt".
// this is a sad sad little function.  but it doesn't look all that bad really.
static void draw_circle(Short radius, Short x0, Short y0)
{
  Short x, y, gap=2;
  if (radius >= 80) return;

  for (x = 0, y = radius ; x <= y ; x += gap) {
    while (x*x + y*y > radius*radius)
      y--;
    // draw horizontalish parts
    WinDrawLine( x + x0, y + y0, x + x0, y + y0);
    WinDrawLine( x + x0,-y + y0, x + x0,-y + y0);
    WinDrawLine(-x + x0, y + y0,-x + x0, y + y0);
    WinDrawLine(-x + x0,-y + y0,-x + x0,-y + y0);
    // draw verticalish parts
    WinDrawLine( y + x0, x + y0, y + x0, x + y0);
    WinDrawLine( y + x0,-x + y0, y + x0,-x + y0);
    WinDrawLine(-y + x0, x + y0,-y + x0, x + y0);
    WinDrawLine(-y + x0,-x + y0,-y + x0,-x + y0);
  }
  // really I shouldn't draw the part that falls into the message area oh well.
}
