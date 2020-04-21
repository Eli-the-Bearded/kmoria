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

static void show_equip(FormPtr frm);
static void free_equipment_select(FormPtr frm);
static void dwimify_buttons(FormPtr frm, Short item);
static void equ_takeoff(Short item);
extern Short wear_low, wear_high; // these live in form_inv.c
extern Boolean in_a_store; // ditto
extern Short in_store; // Need to know which store we are in!
extern Short store_item; // lives in form_store.c
Short *equip_indices;
Boolean Equ_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Short s, e_item;
  ListPtr lst;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    show_equip(frm);
    FrmDrawForm(frm);
    dwimify_buttons(frm, -1);
    handled = true;
    break;

  case lstSelectEvent:
    frm = FrmGetActiveForm();
    s = e->data.lstSelect.selection;
    if (s != -1) {
      e_item = equip_indices[s];
      dwimify_buttons(frm, e_item);      //    specialize_labels(frm, e_item);
    }
    handled = true;
    break;

  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_eq));
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

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_eq));
    s = LstGetSelection(lst);
    if (s == -1) e_item = -1;
    else e_item = equip_indices[s];
    switch(e->data.ctlSelect.controlID) {
    case btn_eq_off:
      if (e_item != -1) {
	free_equipment_select(frm);
	equ_takeoff(e_item);
	show_equip(frm);
	FrmDrawForm(frm);
	dwimify_buttons(frm, -1);
      }
      handled = true;
      break;
    case btn_eq_drop:
      if (e_item != -1) {
	free_equipment_select(frm);
	if (!in_a_store) {
	  invOrEqu_drop(e_item);
	  show_equip(frm);
	  FrmDrawForm(frm);
	  dwimify_buttons(frm, -1);
	} else {
	  LeaveForm();
	  store_item = e_item;
	  if (store_sell_init(in_store, store_item))
	    FrmPopupForm(HaggleForm);
	  // else:  may have been thrown out... xxxx need to deal with that
	}
      }
      handled = true;
      break;
    case btn_eq_cancel:
      free_equipment_select(frm);
      LeaveForm();
      handled = true;
      break;
    case btn_eq_if:
      free_equipment_select(frm);
      LeaveForm();
      FrmPopupForm(InvForm);
      handled = true;
      break;
    }

  default:
    break;
  }
  return handled;
}

void init_equ(FormPtr frm)
{
  // Set up the equipment foo
  // Select the first item so we never have to make all buttons invisible?
  // (for useless items, the first button will be invisible)
}


// Unlike inventory, the buttons DON'T really change meaning here.
static void dwimify_buttons(FormPtr frm, Short item)
{
  ControlPtr btn;

  // based on type of item,
  // make button 1 usable or not, set its text, and set dwim_action.
  // actually dwim_action may be unneeded.
  // it could be a function pointer if I felt evil, though,

  // also, hide the '[I]' button if !inven_ctr.
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_eq_if));
  if (inven_ctr > 0)
    CtlShowControl(btn);
  else
    CtlHideControl(btn);
  // hide takeoff/drop if nothing selected
  if (item == -1) {
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_eq_off));
    CtlHideControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_eq_drop));
    CtlHideControl(btn);
    return;
  }
  // also, disable the 'take off' button if there's no room in the pack.
  // xxx remember to test this someday when I have a lot of inventory.
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_eq_off));
  if (inven_check_num(&inventory[item])) 
    CtlShowControl(btn);
  else
    CtlHideControl(btn);
  // also, disable the 'drop' button if there's already stuff on the ground
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_eq_drop));
  if (in_a_store) {
    // only allow the sell button if you are in the right KNID of store.
    Boolean flag;
    flag = is_store_buy(in_store, inventory[item].tval);
    if (flag) {
      CtlSetLabel(btn, "sell");
      CtlShowControl(btn);
    } else {
      CtlHideControl(btn);
    }
  } else if (cave_tptr[char_row*MAX_WIDTH+char_col] == 0)
    CtlShowControl(btn);
  else
    CtlHideControl(btn);
}

static void dwim_dispatch(Short item)
{
  // remove it or something.
  message("take off");
}

Char **equipment_display;
Short equips_used;
Short listitems_used;
// I have left off all of the original arguments, and return value...
static void show_equip(FormPtr frm)
{
  Short i, j;
  Char tmp_val[160], c, *prt1;
  Int width_str, length_str;
  Boolean fits;
  ListPtr lst;
  //  Char buf[80];

  equips_used = 0;
  listitems_used = 0;
  for (i = INVEN_WIELD ; i < INVEN_ARRAY_SIZE ; i++)
    if (inventory[i].tval != TV_NOTHING)
      equips_used++;

  equipment_display = (Char **) md_malloc(sizeof(Char *) * 2*equips_used);
  equip_indices = (Short *) md_malloc(sizeof(Short) * 2*equips_used);

  c = 'a';
  j = 0;
  for (i = INVEN_WIELD ; i < INVEN_ARRAY_SIZE && j < 2*equips_used ; i++) {
    if (inventory[i].tval != TV_NOTHING) {
      equip_indices[j] = equip_indices[j+1] = i;
      switch(i) {
      case INVEN_WIELD:
	if (pystats.use_stat[A_STR]*15 < inventory[i].weight)
	  prt1 = "Just lifting";
	else
	  prt1 = "Wielding";
	break;
      case INVEN_HEAD:	prt1 = "On head";       break;
      case INVEN_NECK:	prt1 = "Around neck";   break;
      case INVEN_BODY:	prt1 = "On body";       break;
      case INVEN_ARM:	prt1 = "On arm";        break;
      case INVEN_HANDS:	prt1 = "On hands";      break;
      case INVEN_RIGHT:	prt1 = "On right hand"; break;
      case INVEN_LEFT:	prt1 = "On left hand";  break;
      case INVEN_FEET:	prt1 = "On feet";       break;
      case INVEN_OUTER:	prt1 = "About body";    break;
      case INVEN_LIGHT:	prt1 = "Light source";  break;
      case INVEN_AUX:	prt1 = "Spare weapon";  break;
      default:  	prt1 = "Unknown value"; break;
      }
      equipment_display[j] = (Char *) md_malloc(3 + StrLen(prt1) + 1);
      StrPrintF(equipment_display[j], "%c) %s", c, prt1);
      j++;
      objdes(tmp_val, &inventory[i], true);
      width_str = 156-3; // allow room for 1 space
      length_str = StrLen(tmp_val);
      FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
      tmp_val[length_str] = '\0'; // truncate at screen width
      equipment_display[j] = (Char *) md_malloc(1 + StrLen(tmp_val) + 1);
      StrPrintF(equipment_display[j], " %s", tmp_val);
      //      message(tmp_val);
      //      SysTaskDelay(2*SysTicksPerSecond());
      /*
      //
      StrPrintF(tmp_val, "fnord");
      // Truncate if too wide for the screen..... we only have 156 pixels...
      StrPrintF(buf, "%d %c) %s %s", equips_used, c, prt1, tmp_val);
    equipment_display[j] = (Char *) md_malloc(1 + StrLen(tmp_val));
    StrCopy(equipment_display[j], tmp_val);
    //  return;
      */
      j++;
      c++;
    }
  }
  
  listitems_used = j;
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_eq));
  LstSetListChoices(lst, equipment_display, listitems_used);
  LstSetSelection(lst, -1);
}

/*
 *  Free some space that we used in the inventory-selection form
 */
static void free_equipment_select(FormPtr frm)
{
  ListPtr lst;
  Short i;
  VoidHand h;
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_eq));
  LstSetListChoices(lst, NULL, 0);
  if (!listitems_used) return;
  for (i = 0 ; i < listitems_used ; i++) {
    h = MemPtrRecoverHandle(equipment_display[i]);
    if (h) 
      MemHandleFree(h);
  }
  h = MemPtrRecoverHandle(equipment_display);
  if (h) 
    MemHandleFree(h);
  h = MemPtrRecoverHandle(equip_indices);
  if (h) 
    MemHandleFree(h);
}

// both Inventory and Equipment form use this.
void invOrEqu_drop(Short item)
{
  if (item < 0) return;
  if (TR_CURSED & inventory[item].flags) {
    message("Hmmm, it seems to be cursed.");
    return;
  }
  inven_drop(item, true);
  // As a safety measure, set the player's inven weight to 0, 
  // when the last object is dropped
  if (inven_ctr == 0 && equip_ctr == 0)
    inven_weight = 0;
  check_strength();
  free_turn_flag = false;
}

// only Equipment form uses this.
static void equ_takeoff(Short item)
{
  Short slot;
  if (item < 0) return;
  if (TR_CURSED & inventory[item].flags) {
    message("Hmmm, it seems to be cursed.");
    return;
  }
  if (!inven_check_num(&inventory[item])) {
    message("You can't carry it.");
    return;
  }
  slot = inven_carry(&inventory[item]);
  takeoff(item, slot);
  check_strength();
  free_turn_flag = false;
}
