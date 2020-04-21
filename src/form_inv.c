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

/* Ok.  I am going to try to design inventory and equipment forms
   so that (1) you can stay in them after some actions
   and (2) you can use them in a store for a subset of those actions.
   This means that for those stay-in-inv actions, I will need
   a different message routine, and I had better not print
   stats until I exit.
   That, or message and print_stats will have to figure out that
   I am in the inventory form.  That might be easier.
   ...Have the monster-printing functions figure that out too, so
   that I can move monsters while the player makes actions.

   What are the store actions: wear/wield, takeoff. (and exchange)
   What are the stay-in-inv actions: those plus drop.
   Make sure that these don't use message or print_stats.
   Hey, I think I will make 'drop' into 'sell' in the store!

   wear/wield: wear_wield_etc
       inven_destroy
       inven_carry
       takeoff
       py_bonuses
       check_strength
   drop:       invOrEqu_drop
       inven_drop
       check_strength
   takeof:     equ_takeoff
       inven_carry
       takeoff
       check_strength

  What about making time pass while doing inventory actions?
  I can't move monsters if it draws them to the screen.
  I guess screen-drawing things will have to secretly check
  whether they are in the main form or not.
*/

static void dwimify_buttons(FormPtr frm, Short item) SEC_5;
static void dwim_dispatch(Short item) SEC_5;
static void show_inven(FormPtr frm) SEC_5;
static void free_inventory_select(FormPtr frm) SEC_5;
static void wear_wield_etc(Short item) SEC_5;
static void examine_book(Short item_val, Short spellcaster_class) SEC_5;
Short wear_low, wear_high;
extern Short in_store; // Need to know which store we are in!

#define ACT_NONE   0
#define ACT_EAT    1
#define ACT_REFILL 2
#define ACT_WIELD  3
#define ACT_WEAR   4
#define ACT_SCROLL 5
#define ACT_SPELLB 6
#define ACT_PRAYRB 7
#define ACT_QUAFF  8
#define ACT_AIM    9
#define ACT_ZAP   10
#define ACT_DROP  11
#define ACT_THROW 12
Short dwim_action = ACT_NONE; // probably don't even need this

Short inventory_item;
Boolean in_a_store;
extern Short store_item;
Boolean Inv_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  CharPtr label;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    inventory_item = -1;
    show_inven(frm);
    FrmDrawForm(frm);
    dwimify_buttons(frm, inventory_item);
    handled = true;
    break;

  case lstSelectEvent:
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_if));
    label = LstGetSelectionText(lst, e->data.lstSelect.selection);
    if (label) inventory_item = label[0] - 'a';
    else inventory_item = -1; // bug if this happens
    dwimify_buttons(frm, inventory_item); //    specialize_labels(frm, inventory_item);
    handled = true;
    break;

  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_if));
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      if (LstScrollList(lst, up, 10)) {// there are 10 items visible at a time
	LstSetSelection(lst, -1);
	inventory_item = -1;
      }
      handled = true;
      break;
    case pageDownChr:
      if (LstScrollList(lst, down, 10)) {
	LstSetSelection(lst, -1);
	inventory_item = -1;
      }
      handled = true;
      break;
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_if_frob:
      free_inventory_select(frm);
      //      LeaveForm();
      if (inventory_item != -1)
	dwim_dispatch(inventory_item);
      //      else unmessage();
      handled = true;
      break;
    case btn_if_drop:
      //      LeaveForm();
      if (inventory_item != -1) {
	free_inventory_select(frm);
	if (!in_a_store) {
	  invOrEqu_drop(inventory_item);
	  show_inven(frm);
	  FrmDrawForm(frm);
	  //	inventory_item = -1;
	  dwimify_buttons(frm, inventory_item);
	} else {
	  LeaveForm();
	  store_item = inventory_item;
	  if (store_sell_init(in_store, store_item))
	    FrmPopupForm(HaggleForm);
	  // else:  may have been thrown out... xxxx need to deal with that
	}
      }
      //      else unmessage();
      handled = true;
      break;
    case btn_if_throw:
      free_inventory_select(frm);
      LeaveForm();
      if (inventory_item != -1) {
	needed_direction_for = DIR_THROW;
	needed_direction_item = inventory_item;
	draw_directional();
      }
      //      else unmessage();
      handled = true;
      break;
    case btn_if_cancel:
      free_inventory_select(frm);
      LeaveForm();
      //      unmessage();
      handled = true;
      break;
    case btn_if_eq:
      free_inventory_select(frm);
      LeaveForm();
      FrmPopupForm(EquForm);
      handled = true;
      break;
    }

  default:
    break;
  }
  return handled;
}



// This determins what string is printed on the frob button.
static Short do_what(Short item_type)
{
  if (item_type >= TV_MIN_WEAR && item_type <= TV_MAX_WEAR) {
    if (item_type < TV_MID_WEAR)
      return ACT_WIELD;
    return ACT_WEAR;
  }
  switch(item_type) {
  case TV_STAFF:
    return ACT_ZAP;
  case TV_WAND:
    return ACT_AIM;
  case TV_FLASK:
    return ACT_REFILL;
  case TV_SCROLL1: case TV_SCROLL2:
    return ACT_SCROLL;
  case TV_POTION1: case TV_POTION2:
    return ACT_QUAFF;
  case TV_FOOD:
    return ACT_EAT;
  case TV_PRAYER_BOOK:
    return ACT_PRAYRB;
  case TV_MAGIC_BOOK:
    return ACT_SPELLB;
  }
  return ACT_NONE;
}
const Char if_button_labels[10][7] = {
  "eat", "refill", "wield", "wear", "read",
  "read", "read", "quaff", "aim", "zap"
};
static void dwimify_buttons(FormPtr frm, Short item)
{
  Short i;
  ControlPtr btn;

  // hide the '[E]' button if !equip_ctr.
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_eq));
  CtlHideControl(btn);
  for (i = INVEN_WIELD ; i < INVEN_ARRAY_SIZE ; i++)
    if (inventory[i].tval != TV_NOTHING) {
      CtlShowControl(btn);
      break;
    }
  // hide drop/throw if nothing selected
  if (item == -1 || in_a_store) {
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_frob));
    CtlHideControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_throw));
    CtlHideControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_drop));
    if (item == -1) {
      CtlHideControl(btn);
      return;
    } else {
    // only allow the sell button if you are in the right KNID of store.
      Boolean flag;
      flag = is_store_buy(in_store, inventory[item].tval);
      if (flag) {
	CtlSetLabel(btn, "sell");
	CtlShowControl(btn);
      } else {
	CtlHideControl(btn);
      }
    }
  } else {
    // also, disable the 'drop' button if cave_tptr[char_row][char_col]!=0
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_drop));
    if (cave_tptr[char_row*MAX_WIDTH+char_col] == 0)
      CtlShowControl(btn);
    else
      CtlHideControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_throw));
    CtlShowControl(btn);
  }
  // based on type of item,
  // make button 1 usable or not, set its text, and set dwim_action.
  // actually dwim_action may be unneeded.
  // it could be a function pointer if I felt evil, though,
  i = do_what(inventory[item].tval);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_frob));
  if (i == ACT_NONE || (in_a_store && i != ACT_WIELD && i != ACT_WEAR))
    CtlHideControl(btn);
  else {
    CtlSetLabel(btn, if_button_labels[i-1]);
    CtlShowControl(btn);    
  }
  // In store, you are allowed to wear/wield (inv), and to take off (equ).
  // And also to exchange weapons.  Disable anything else.
}

// 'item' should be an index into inventory[].
static void dwim_dispatch(Short item)
{
  Short i;
  FormPtr frm;
  if (item == -1) return;
  i = do_what(inventory[item].tval);
  // Actions that let you stay in inventory:
  switch(i) {
  case ACT_WIELD: case ACT_WEAR:
    wear_wield_etc(item);
    frm = FrmGetActiveForm();
    show_inven(frm);
    FrmDrawForm(frm);
    //    inventory_item = -1;
    dwimify_buttons(frm, inventory_item);
    return;
  }
  // Actions that require you to leave inventory:
  LeaveForm();
  switch(i) {
  case ACT_QUAFF:
    quaff(item); break;
  case ACT_EAT:
    eat(item); break;
  case ACT_REFILL: // this ignores 'item' and can be called stand-alone.
    refill_lamp(item); break;
  case ACT_AIM:
    needed_direction_for = DIR_AIM;
    needed_direction_item = item;
    draw_directional();
    break;
  case ACT_ZAP:
    use(item); break;
  case ACT_SCROLL:
  //  read_scroll(item); // note this could fail if dark, blind, or confused
    message("read scroll"); break;
  case ACT_PRAYRB:
    examine_book(item, PRIEST);
    break;
  case ACT_SPELLB:
    // This button should only have been enabled if you're the right class.
    // also consider disabling if:
    // pyflags.blind > 0; no_light(); pyflags.confused > 0.
    examine_book(item, MAGE);
    break;
    // This should be like Browse
    //    pray(item); // note this could fail if dark, blind, or confused;
    //                  and it should pop up a "which prayer?" form, itself.
    //    message("read prayer book"); break;
    // This should be like Browse
    //    cast(item); // note this could fail if dark, blind, or confused;
    //                 and it should pop up a "which spell?" form, itself.
    //                 I should combine cast/pray and examine_book
    //                 probably.
    //    message("read spell book"); break;
  default:
    message("not implemented yet");
  }
}


Char **inventory_display;
// I have left off all of the original arguments, and return value...
static void show_inven(FormPtr frm)
{
  Short i;
  Char tmp_val[160], c;
  Int width_str, length_str;
  Boolean fits;
  ListPtr lst;

  inventory_display = (Char **) md_malloc(sizeof(Char *) * inven_ctr); // ?

  for (i = 0, c = 'a' ; i < inven_ctr ; i++, c++) {
    /* Print the items	  */
    //    if (mask == CNIL || mask[i]) {

    StrPrintF(tmp_val, "%c) ", c);
    objdes(tmp_val+3, &inventory[i], true); // XXXXxxx fix this.

    // Truncate if too wide for the screen..... we only have 156 pixels...
    // length_str gets set to the # of chars that will fit in 156
    // width_str gets set to the # of pixels that those chars will use (<=156)
    width_str = 156;
    length_str = StrLen(tmp_val); // max chars to allow
    FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
    tmp_val[length_str] = '\0';
    // ok, now like put it somewhere.
    inventory_display[i] = (Char *) md_malloc(1 + length_str);
    StrCopy(inventory_display[i], tmp_val);
  }

  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_if));
  LstSetListChoices(lst, inventory_display, inven_ctr);
  if (inventory_item < 0)
    LstSetSelection(lst, -1);
  else {
    if (inventory_item >= inven_ctr)
      inventory_item = inven_ctr - 1;
    LstSetSelection(lst, inventory_item);
  }
}

/*
 *  Free some space that we used in the inventory-selection form
 */
static void free_inventory_select(FormPtr frm)
{
  ListPtr lst;
  Short i;
  VoidHand h;
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_if));
  LstSetListChoices(lst, NULL, 0);
  for (i = 0 ; i < inven_ctr ; i++) {
    h = MemPtrRecoverHandle(inventory_display[i]);
    if (h) 
      MemHandleFree(h);
  }
  h = MemPtrRecoverHandle(inventory_display);
  if (h) 
    MemHandleFree(h);
}




/* Refill the players lamp				-RAK-	*/
void refill_lamp(Short ignored)
{
  Short i, j;
  Short k, qty;
  inven_type *i_ptr;

  free_turn_flag = true;
  k = inventory[INVEN_LIGHT].subval;
  if (k != 0)
    message("But you are not using a lamp.");
  else if (!find_range(TV_FLASK, TV_NEVER, &i, &j))
    message("You have no oil.");
  else {
    free_turn_flag = false;
    i_ptr = &inventory[INVEN_LIGHT];
    qty = i_ptr->p1 + inventory[i].p1;
    if (qty > OBJ_LAMP_MAX) {
      qty = OBJ_LAMP_MAX;
      message ("Your lamp overflows, spilling oil on the ground.");
      message("Your lamp is full.");
    } else if (qty > OBJ_LAMP_MAX/2)
      message ("Your lamp is more than half full.");
    else if (qty == OBJ_LAMP_MAX/2)
      message ("Your lamp is half full.");
    else
      message ("Your lamp is less than half full.");
    invy_set_p1(inventory, INVEN_LIGHT, qty);
    desc_remain(i);
    inven_destroy(i);
  }
}


static void wear_wield_etc(Short item)
{
  Short slot, tmp2, tmp;
  Char prt1[160], prt2[160];
  inven_type tmp_obj, *i_ptr, tmp_obj2;
  Char *string;

  /* Wearing. Go to a bit of trouble over replacing
     existing equipment. */
  //  if (isupper((int)which) && !verify(prompt, item))
  //    item = -1;
  //  else
  slot = 0;
  switch(inventory[item].tval) {
    /* Slot for equipment	   */
  case TV_SLING_AMMO: case TV_BOLT: case TV_ARROW:
  case TV_BOW: case TV_HAFTED: case TV_POLEARM:
  case TV_SWORD: case TV_DIGGING: case TV_SPIKE:
    slot = INVEN_WIELD; break;
  case TV_LIGHT: slot = INVEN_LIGHT; break;
  case TV_BOOTS: slot = INVEN_FEET; break;
  case TV_GLOVES: slot = INVEN_HANDS; break;
  case TV_CLOAK: slot = INVEN_OUTER; break;
  case TV_HELM: slot = INVEN_HEAD; break;
  case TV_SHIELD: slot = INVEN_ARM; break;
  case TV_HARD_ARMOR: case TV_SOFT_ARMOR:
    slot = INVEN_BODY; break;
  case TV_AMULET: slot = INVEN_NECK; break;
  case TV_RING:
    if (inventory[INVEN_RIGHT].tval == TV_NOTHING)
      slot = INVEN_RIGHT;
    else if (inventory[INVEN_LEFT].tval == TV_NOTHING)
      slot = INVEN_LEFT;
    /*
    else {
      slot = 0;
      // Rings. Give choice over where they go.
      do {
	if (!get_com(
		     "Put ring on which hand (l/r/L/R)?", &query)) {
	  item = -1;
	  slot = -1;
	}
	else if (query == 'l')
	  slot = INVEN_LEFT;
	else if (query == 'r')
	  slot = INVEN_RIGHT;
	else {
	  if (query == 'L')
	    slot = INVEN_LEFT;
	  else if (query == 'R')
	    slot = INVEN_RIGHT;
	  else
	    bell();
	  if (slot && !verify("Replace", slot))
	    slot = 0;
	}
      } while(slot == 0);
    }
    */
    break;
  default:
    message("I don't see how you can use that.");
    item = -1;
    break;
  }
  if (item >= 0 && inventory[slot].tval != TV_NOTHING) {
    if (TR_CURSED & inventory[slot].flags) {
      objdes(prt1, &inventory[slot], false);
      StrPrintF(prt2, "The %s you are ", prt1);
      if (slot == INVEN_HEAD) // hello???
	StrCat(prt2, "wielding ");
      else
	StrCat(prt2, "wearing ");
      message(StrCat(prt2, "appears to be cursed."));
      item = -1;
    } else if (inventory[item].subval == ITEM_GROUP_MIN &&
	       inventory[item].number > 1 &&
	       !inven_check_num(&inventory[slot])) {
      /* this can happen if try to wield a torch, 
	 and have more than one in inventory */
      message("You'll have to drop something first.");
      item = -1;
    }
  }

  if (item >= 0) {
    /* OK. Wear it. */
    free_turn_flag = false;
			      
    /* first remove new item from inventory */
    tmp_obj = inventory[item];
    i_ptr = &tmp_obj;
			      
    wear_high--;
    /* Fix for torches	   */
    if (i_ptr->number > 1
	&& i_ptr->subval <= ITEM_SINGLE_STACK_MAX) {
      i_ptr->number = 1;
      wear_high++;
    }
    //      message("freak");
    //      SysTaskDelay(2*SysTicksPerSecond());
    inven_weight += i_ptr->weight*i_ptr->number;
    inven_destroy(item);	/* Subtracts weight */ // Problem for Torch!!!
    //      message("torch");
    //      SysTaskDelay(2*SysTicksPerSecond());
			      
    /* second, add old item to inv and remove from
       equipment list, if necessary */
    i_ptr = &inventory[slot];
    if (i_ptr->tval != TV_NOTHING) {
      tmp2 = inven_ctr;
      tmp_obj2 = *i_ptr;
      tmp = inven_carry(&tmp_obj2);//      tmp = inven_carry(i_ptr);
      /* if item removed did not stack 
	 with anything in inventory, 
	 then increment wear_high */
      if (inven_ctr != tmp2)
	wear_high++;
      takeoff(slot, tmp); // xxx not implemented yet
    }
    //

    /* third, wear new item */
    //    *i_ptr = tmp_obj; // uh, this will not work.  instead,
    DmWrite(inventory, slot*sizeof(inven_type), &tmp_obj, sizeof(inven_type));
    equip_ctr++;
    py_bonuses(i_ptr, 1);
    if (slot == INVEN_WIELD)
      string = "Wielding:";
    else if (slot == INVEN_LIGHT)
      string = "Lighted:";
    else
      string = "Wearing:";
    objdes(prt2, i_ptr, true);
    /* Get the right equipment letter. */
    tmp = INVEN_WIELD;
    item = 0;
    while (tmp != slot)
      if (inventory[tmp++].tval != TV_NOTHING)
	item++;
			      
    //    StrPrintF(prt1, "%s %s (%c)", string, prt2, 'a'+item);
    StrPrintF(prt1, "%s %s", string, prt2);
    message(prt1);
    /* this is a new weapon, so clear heavy flag */
    if (slot == INVEN_WIELD)
      weapon_heavy = false;
    check_strength();
    if (i_ptr->flags & TR_CURSED) {
      message("Oops! It feels deathly cold!");
      add_inscribe(inventory, slot, ID_DAMD);
      /* To force a cost of 0, 
	 even if unidentified. */
      i_ptr->cost = -1;
    }
  }
}

/*
  'context sensitive' inventory buttons: (you can also inscribe anything)
  food: eat, throw, drop
  oil: refill, throw, drop
  weapon: wield, throw, drop
  armor: wear, throw, drop
  scroll: read, throw, drop
  potion: quaff, throw, drop
  book: read, throw, drop
  wand: aim, throw, drop
  staff: zap, throw, drop
  
  equipment: take off, exchange, drop .. ALSO wear/wield from inventory...
  you can also {t, x, d, w} in stores


  inventory:  see inven_command for {w,d}
  browse/peruse a book: examine_book()
  drop any item
  quaff a potion: quaff()
  read a scroll: read_scroll()
  wear/wield an item
  eat food: eat()
  fill lamp with oil: refill_lamp()
  (maybe) inscribe an object: scribe_object()
  
  equipment:  see inven_command for {d,t}
  drop any item
  take off / unwield any item
*/


extern Boolean just_browsing_book; 
extern spell_type magic_spell[MAX_CLASS-1][MAX_SPELLS]; // magic.c
extern Short spells[MAX_SPELLS]; // form_spell.c
extern Short spell_ctr; // ditto 
extern Short selected_book;
static void examine_book(Short item_val, Short spellcaster_class)
{
  ULong j;
  Short i, k, my_class = pymisc.pclass - 1;

  if (pyflags.blind > 0)
    message("You can't see to read your spell book!");
  else if (no_light())
    message("You have no light to read by.");
  else if (pyflags.confused > 0)
    message("You are too confused.");
  else if (class[pymisc.pclass].spell != spellcaster_class)
    message("You do not understand the language.");
  else {
    just_browsing_book = true;
    i = 0;
    j = inventory[item_val].flags;
    while (j) {
      k = bit_pos(&j);
      if (magic_spell[my_class][k].slevel < 99) {
	spells[i] = k;
	i++;
      } // else it's a spell your class never gets; sorry dude
    }
    spell_ctr = i;
    selected_book = item_val;
    //    save_screen();
    //    print_spells(spell_index, i, TRUE, -1);
    //    pause_line(0);
    //    restore_screen();
    FrmPopupForm(GainSpForm); // ok, I lied we're using THIS form
  }
}
