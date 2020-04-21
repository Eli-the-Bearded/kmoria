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

/* Object descriptor routines					*/

static void unsample(inven_type *i_ptr, Short index) SEC_1;

Boolean is_a_vowel(Char ch)
{
  switch(ch) {
  case 'a': case 'e': case 'i': case 'o': case 'u':
  case 'A': case 'E': case 'I': case 'O': case 'U':
    return true;
  default:
    return false;
  }
}

// magic_init
// I need to look this over and maybe put it in lock.c


Short object_offset(inven_type *t_ptr)
{
  switch (t_ptr->tval) {
  case TV_AMULET: return(0);
  case TV_RING: return(1);
  case TV_STAFF: return(2);
  case TV_WAND: return(3);
  case TV_SCROLL1: case TV_SCROLL2: return(4);
  case TV_POTION1: case TV_POTION2: return(5);
  case TV_FOOD:
    if ((t_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1)) < MAX_MUSH)
      return(6);
    return(-1);
  default:  return(-1);
  }
}


/* Remove "Secret" symbol for identity of object			*/
void known1(inven_type *i_ptr)
{
  Short offset;
  UChar indexx;

  if ((offset = object_offset(i_ptr)) < 0)
    return;
  offset <<= 6;
  indexx = i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1);
  object_ident_or(offset + indexx, OD_KNOWN1);
  // clear the tried flag, since it is now known
  object_ident_nand(offset + indexx, OD_TRIED);
}


Boolean known1_p(inven_type *i_ptr)
{
  Short offset;
  UChar indexx;

  /* Items which don't have a 'color' are always known1, so that they can
     be carried in order in the inventory.  */
  if ((offset = object_offset(i_ptr)) < 0)
    return OD_KNOWN1;
  if (store_bought_p(i_ptr))
    return OD_KNOWN1;
  offset <<= 6;
  indexx = i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1);
  return (object_ident[offset + indexx] & OD_KNOWN1);
}

/* Remove "Secret" symbol for identity of plusses			*/
void known2(inven_type *i_ptr, Short index)
{
  unsample(i_ptr, index);
  invy_set_ident(i_ptr, index, i_ptr[index].ident | ID_KNOWN2); // XXX
}

Boolean known2_p(inven_type *i_ptr)
{
  return (i_ptr->ident & ID_KNOWN2);
}

void clear_known2(inven_type *i_ptr, Short index)
{
  invy_set_ident(i_ptr, index, i_ptr[index].ident & ~ID_KNOWN2); // XXX
}

void clear_empty(inven_type *i_ptr, Short index)
{
  invy_set_ident(i_ptr, index, i_ptr[index].ident & ~ID_EMPTY); // XXX
}

void store_bought(inven_type *i_ptr, Short index)
{
  invy_set_ident(i_ptr, index, i_ptr[index].ident | ID_STOREBOUGHT); // XXX
  known2(i_ptr, index);
}


Boolean store_bought_p(inven_type *i_ptr)
{
  return (i_ptr->ident & ID_STOREBOUGHT);
}


/*	Remove an automatically generated inscription.	-CJS- */
static void unsample(inven_type *i, Short index)
{
  Short offset;
  UChar indexx;
  inven_type *i_ptr = &i[index];

  /* used to clear ID_DAMD flag, but I think it should remain set */
  invy_set_ident(i, index, i[index].ident &=~(ID_MAGIK|ID_EMPTY));

  if ((offset = object_offset(i_ptr)) < 0)
    return;
  offset <<= 6;
  indexx = i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1);
  object_ident_nand(offset + indexx, OD_TRIED);
}

/* Somethings been sampled -CJS- */
void sample (inven_type *i_ptr)
{
  Short offset;
  UChar indexx;

  if ((offset = object_offset(i_ptr)) < 0)
    return;
  offset <<= 6;
  indexx = i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1);
  object_ident_or(offset + indexx, OD_TRIED);
}



/* Somethings been identified					*/
/* extra complexity by CJS so that it can merge store/dungeon objects
   when appropriate */
void identify(Short *item)
{
  Short i, x1, x2;
  Short j;
  inven_type *i_ptr, *t_ptr;

  i_ptr = &inventory[*item];

  if (i_ptr->flags & TR_CURSED)
    add_inscribe(inventory, *item, ID_DAMD);

  if (!known1_p(i_ptr)) {
    known1(i_ptr);
    x1 = i_ptr->tval;
    x2 = i_ptr->subval;
    if (x2 < ITEM_SINGLE_STACK_MIN || x2 >= ITEM_GROUP_MIN)
      /* no merging possible */
      return;
    for (i = 0; i < inven_ctr; i++) {
      t_ptr = &inventory[i];
      if (t_ptr->tval == x1 && t_ptr->subval == x2 && i != *item
	  && ((Short)t_ptr->number + (Short)i_ptr->number < 256)) {
	/* make *item the smaller number */
	if (*item > i) { j = *item; *item = i; i = j; }
	message("You combine similar objects from the shop and dungeon.");

	invy_set_number(inventory, *item, 
			     inventory[*item].number + inventory[i].number);
	invy_delete(i);
      }
    }
  }
}



/* If an object has lost magical properties,
 * remove the appropriate portion of the name.	       -CJS-
 */
void unmagic_name(Short iptr)
{
  Short offset = sizeof(UShort);
  UChar noname = 0;
  // DANGER WILL ROBINSON
  DmWrite(inventory, iptr*sizeof(inven_type)+offset, &noname, sizeof(UChar));  // i_ptr->name2 = SN_NULL;
}


// Skipping objdes, I don't feel up to dealing with it tonight.
// ditto desc_charges and desc_remain.

/* defines for p1_use, determine how the p1 field is printed */
#define IGNORED  0
#define CHARGES  1
#define PLUSSES  2
#define LIGHT    3
#define FLAGS    4
#define Z_PLUSSES 5
/* Returns a description of item for inventory			*/
/* pref indicates that there should be an article added (prefix) */
/* note that since out_val can easily exceed 80 characters, objdes must
   always be called with a bigvtype as the first paramter */
void objdes(Char *out_val, inven_type *i_ptr, Boolean pref)
{
  /* base name, modifier string*/
  Char *basenm, *modstr;
  Char tmp_val[160];
  Char tmp_str[80], damstr[80];
  Short indexx, p1_use, modify, append_name, tmp;

  indexx = i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1);
  basenm = object_names + object_list[i_ptr->index].name_offset;
  modstr = CNIL;
  damstr[0] = '\0';
  p1_use = IGNORED;
  modify = (known1_p(i_ptr) ? false : true);
  append_name = false;
  switch(i_ptr->tval) {
  case  TV_MISC:
  case  TV_CHEST:
    break;
  case  TV_SLING_AMMO:
  case  TV_BOLT:
  case  TV_ARROW:
    StrPrintF(damstr, " (%dd%d)", i_ptr->damage[0], i_ptr->damage[1]);
    break;
  case  TV_LIGHT:
    p1_use = LIGHT;
    break;
  case  TV_SPIKE:
    break;
  case  TV_BOW:
    if (i_ptr->p1 == 1 || i_ptr->p1 == 2)
      tmp = 2;
    else if (i_ptr->p1 == 3 || i_ptr->p1 == 5)
      tmp = 3;
    else if (i_ptr->p1 == 4 || i_ptr->p1 == 6)
      tmp = 4;
    else
      tmp = -1;
    StrPrintF(damstr, " (x%d)", tmp);
    break;
  case  TV_HAFTED:
  case  TV_POLEARM:
  case  TV_SWORD:
    StrPrintF(damstr, " (%dd%d)", i_ptr->damage[0], i_ptr->damage[1]);
    p1_use = FLAGS;
    break;
  case  TV_DIGGING:
    p1_use = Z_PLUSSES;
    StrPrintF(damstr, " (%dd%d)", i_ptr->damage[0], i_ptr->damage[1]);
    break;
  case  TV_BOOTS:
  case  TV_GLOVES:
  case  TV_CLOAK:
  case  TV_HELM:
  case  TV_SHIELD:
  case  TV_HARD_ARMOR:
  case  TV_SOFT_ARMOR:
    break;
  case  TV_AMULET:
    if (modify) {
      basenm = "& %s Amulet";
      modstr = amulets[indexx];
    } else {
      basenm = "& Amulet";
      append_name = true;
    }
    p1_use = PLUSSES;
    break;
  case  TV_RING:
    if (modify) {
      basenm = "& %s Ring";
      modstr = rocks[indexx];
    } else {
      basenm = "& Ring";
      append_name = true;
    }
    p1_use = PLUSSES;
    break;
  case  TV_STAFF:
    if (modify) {
      basenm = "& %s Staff";
      modstr = woods[indexx];
    } else {
      basenm = "& Staff";
      append_name = true;
    }
    p1_use = CHARGES;
    break;
  case  TV_WAND:
    if (modify) {
      basenm = "& %s Wand";
      modstr = metals[indexx];
    } else {
      basenm = "& Wand";
      append_name = true;
    }
    p1_use = CHARGES;
    break;
  case  TV_SCROLL1:
  case  TV_SCROLL2:
    if (modify) {
      basenm =  "& Scroll~ titled \"%s\"";
      modstr = titles[indexx];
    } else {
      basenm = "& Scroll~";
      append_name = true;
    }
    break;
  case  TV_POTION1:
  case  TV_POTION2:
    if (modify) {
      basenm = "& %s Potion~";
      modstr = colors[indexx];
    } else {
      basenm = "& Potion~";
      append_name = true;
    }
    break;
  case  TV_FLASK:
    break;
  case  TV_FOOD:
    if (modify) {
      if (indexx <= 15)
	basenm = "& %s Mushroom~";
      else if (indexx <= 20)
	basenm = "& Hairy %s Mold~";
      if (indexx <= 20)
	modstr = mushrooms[indexx];
    } else {
      append_name = true;
      if (indexx <= 15)
	basenm = "& Mushroom~";
      else if (indexx <= 20)
	basenm = "& Hairy Mold~";
      else
	/* Ordinary food does not have a name appended.  */
	append_name = false;
    }
    break;
  case  TV_MAGIC_BOOK:
    modstr = basenm;
    basenm = "& Book~ of Magic Spells %s";
    break;
  case  TV_PRAYER_BOOK:
    modstr = basenm;
    basenm = "& Holy Book~ of Prayers %s";
    break;
  case TV_OPEN_DOOR:
  case TV_CLOSED_DOOR:
  case TV_SECRET_DOOR:
  case TV_RUBBLE:
    break;
  case TV_GOLD:
  case TV_INVIS_TRAP:
  case TV_VIS_TRAP:
  case TV_UP_STAIR:
  case TV_DOWN_STAIR:
    StrCopy(out_val, object_names + object_list[i_ptr->index].name_offset);
    StrCat(out_val, ".");
    return;
  case TV_STORE_DOOR:
    StrPrintF(out_val, "the entrance to the %s.",
	      object_names + object_list[i_ptr->index].name_offset);
    return;
  default:
    StrCopy(out_val, "Error in objdes()");
    return;
  }
  if (modstr != CNIL)
    StrPrintF(tmp_val, basenm, modstr);
  else
    StrCopy(tmp_val, basenm);
  if (append_name) {
    StrCat(tmp_val, " of ");
    StrCat(tmp_val, object_names + object_list[i_ptr->index].name_offset);
  }
  if (i_ptr->number != 1) {
    insert_str(tmp_val, "ch~", "ches");
    insert_str(tmp_val, "~", "s");
  } else
    insert_str(tmp_val, "~", CNIL);
  if (!pref) {
    if (!StrNCompare("some", tmp_val, 4))
      StrCopy(out_val, &tmp_val[5]);
    else if (tmp_val[0] == '&')
      /* eliminate the '& ' at the beginning */
      StrCopy(out_val, &tmp_val[2]);
    else
      StrCopy(out_val, tmp_val);
  } else {
    if (i_ptr->name2 != SN_NULL && known2_p(i_ptr)) {
      StrCat(tmp_val, " ");
      StrCat(tmp_val, special_names[i_ptr->name2]);
    }
    if (damstr[0] != '\0')
      StrCat(tmp_val, damstr);
    if (known2_p(i_ptr)) {
      /* originally used %+d, but several machines don't support it */
      if (i_ptr->ident & ID_SHOW_HITDAM)
	StrPrintF(tmp_str, " (%c%d,%c%d)",
		  (i_ptr->tohit < 0) ? '-' : '+', abs(i_ptr->tohit),
		  (i_ptr->todam < 0) ? '-' : '+', abs(i_ptr->todam));
      else if (i_ptr->tohit != 0)
	StrPrintF(tmp_str, " (%c%d)",
		  (i_ptr->tohit < 0) ? '-' : '+', abs(i_ptr->tohit));
      else if (i_ptr->todam != 0)
	StrPrintF(tmp_str, " (%c%d)",
		  (i_ptr->todam < 0) ? '-' : '+', abs(i_ptr->todam));
      else
	tmp_str[0] = '\0';
      StrCat(tmp_val, tmp_str);
    }
    /* Crowns have a zero base AC, so make a special test for them. */
    if (i_ptr->ac != 0 || (i_ptr->tval == TV_HELM)) {
      StrPrintF(tmp_str, " [%d", i_ptr->ac);
      StrCat(tmp_val, tmp_str);
      if (known2_p(i_ptr)) {
	/* originally used %+d, but several machines don't support it */
	StrPrintF(tmp_str, ",%c%d",
		  (i_ptr->toac < 0) ? '-' : '+', abs(i_ptr->toac));
	StrCat(tmp_val, tmp_str);
      }
      StrCat(tmp_val, "]");
    } else if ((i_ptr->toac != 0) && known2_p(i_ptr)) {
      /* originally used %+d, but several machines don't support it */
      StrPrintF(tmp_str, " [%c%d]",
		(i_ptr->toac < 0) ? '-' : '+', abs(i_ptr->toac));
      StrCat(tmp_val, tmp_str);
    }

    /* override defaults, check for p1 flags in the ident field */
    if (i_ptr->ident & ID_NOSHOW_P1)
      p1_use = IGNORED;
    else if (i_ptr->ident & ID_SHOW_P1)
      p1_use = Z_PLUSSES;
    tmp_str[0] = '\0';
    if (p1_use == LIGHT)
      StrPrintF(tmp_str, " with %d turns of light", i_ptr->p1);
    else if (p1_use == IGNORED)
	;
    else if (known2_p(i_ptr)) {
      if (p1_use == Z_PLUSSES)
	/* originally used %+d, but several machines don't support it */
	StrPrintF(tmp_str, " (%c%d)",
		  (i_ptr->p1 < 0) ? '-' : '+', abs(i_ptr->p1));
      else if (p1_use == CHARGES)
	StrPrintF(tmp_str, " (%d charges)", i_ptr->p1);
      else if (i_ptr->p1 != 0) {
	if (p1_use == PLUSSES)
	  StrPrintF(tmp_str, " (%c%d)",
		    (i_ptr->p1 < 0) ? '-' : '+', abs(i_ptr->p1));
	else if (p1_use == FLAGS) {
	  if (i_ptr->flags & TR_STR)
	    StrPrintF(tmp_str, " (%c%d to STR)",
		      (i_ptr->p1 < 0) ? '-' : '+',abs(i_ptr->p1));
	  else if (i_ptr->flags & TR_STEALTH)
	    StrPrintF(tmp_str, " (%c%d to stealth)",
		      (i_ptr->p1 < 0) ? '-' : '+',abs(i_ptr->p1));
	}
      }
    }
    StrCat(tmp_val, tmp_str);

    /* ampersand is always the first character */
    if (tmp_val[0] == '&') {
      /* use &tmp_val[1], so that & does not appear in output */
      if (i_ptr->number > 1)
	StrPrintF(out_val, "%d%s", (Short)i_ptr->number, &tmp_val[1]);
      else if (i_ptr->number < 1)
	StrPrintF(out_val, "%s%s", "no more", &tmp_val[1]);
      else if (is_a_vowel(tmp_val[2]))
	StrPrintF(out_val, "an%s", &tmp_val[1]);
      else
	StrPrintF(out_val, "a%s", &tmp_val[1]);
    } else if (i_ptr->number < 1) { /* handle 'no more' case specially */
      /* check for "some" at start */
      if (!StrNCompare("some", tmp_val, 4))
	StrPrintF(out_val, "no more %s", &tmp_val[5]);
      /* here if no article */
      else
	StrPrintF(out_val, "no more %s", tmp_val);
    } else
      StrCopy(out_val, tmp_val);

    tmp_str[0] = '\0';
    if ((indexx = object_offset(i_ptr)) >= 0) {
      indexx = (indexx <<= 6) +
	(i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1));
      /* don't print tried string for store bought items */
      if ((object_ident[indexx] & OD_TRIED) && !store_bought_p(i_ptr))
	StrCat(tmp_str, "tried ");
    }
    if (i_ptr->ident & (ID_MAGIK|ID_EMPTY|ID_DAMD)) {
      if (i_ptr->ident & ID_MAGIK)
	StrCat(tmp_str, "magik ");
      if (i_ptr->ident & ID_EMPTY)
	StrCat(tmp_str, "empty ");
      if (i_ptr->ident & ID_DAMD)
	StrCat(tmp_str, "damned ");
    }
    if (i_ptr->inscrip[0] != '\0')
      StrCat(tmp_str, i_ptr->inscrip);
    else if ((indexx = StrLen(tmp_str)) > 0)
      /* remove the extra blank at the end */
      tmp_str[indexx-1] = '\0';
    if (tmp_str[0]) {
      StrPrintF(tmp_val, " {%s}", tmp_str);
      StrCat(out_val, tmp_val);
    }
    StrCat(out_val, ".");
  }
}



// invcopy already lives somewhere, like lock.c



/* Describe number of remaining charges.		-RAK-	*/
void desc_charges(Short item_val)
{
  Short rem_num;
  Char out_val[80];

  if (known2_p(&inventory[item_val])) {
    rem_num = inventory[item_val].p1;
    StrPrintF(out_val, "You have %d charges remaining.", rem_num);
    message(out_val);
  }
}


/* Describe amount of item remaining.			-RAK-	*/
void desc_remain(Short item_val)
{
  Char out_val[160], tmp_str[160];
  inven_type *i_ptr;
  Short num;

  i_ptr = &inventory[item_val];
  num = i_ptr->number;
  // this is a bit freaky...
  invy_set_number(inventory, item_val, num - 1);
  objdes(tmp_str, i_ptr, true);
  invy_set_number(inventory, item_val, num);
  /* the string already has a dot at the end. */
  StrPrintF(out_val, "You have %s", tmp_str);
  message(out_val);
}
