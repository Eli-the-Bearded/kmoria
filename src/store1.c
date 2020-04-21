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

/* Buying and selling adjustments for character race VS store	*/
/* owner race							 */
// this is 64 bytes?
UChar rgold_adj[MAX_RACES][MAX_RACES] = {
			/*  Hum,  HfE,	Elf,  Hal,  Gno,  Dwa,	HfO,  HfT*/
/*Human		 */	  {  100,  105,	 105,  110,  113,  115,	 120,  125},
/*Half-Elf	 */	  {  110,  100,	 100,  105,  110,  120,	 125,  130},
/*Elf		 */	  {  110,  105,	 100,  105,  110,  120,	 125,  130},
/*Halfling	 */	  {  115,  110,	 105,	95,  105,  110,	 115,  130},
/*Gnome		 */	  {  115,  115,	 110,  105,   95,  110,	 115,  130},
/*Dwarf		 */	  {  115,  120,	 120,  110,  110,   95,	 125,  135},
/*Half-Orc	 */	  {  115,  120,	 125,  115,  115,  130,	 110,  115},
/*Half-Troll	 */	  {  110,  115,	 115,  110,  110,  130,	 110,  110}
};
owner_type owners[MAX_OWNERS] = {
{"Erick the Honest      (Human)",   "General Store", 250, 175, 108, 4, 0, 12},
{"Mauglin the Grumpy    (Dwarf)",   "Armory",      32000, 200, 112, 4, 5,  5},
{"Arndal Beast-Slayer   (Half-Elf)","Weaponsmith", 10000, 185, 110, 5, 1,  8},
{"Hardblow the Humble   (Human)",   "Temple",       3500, 175, 109, 6, 0, 15},
{"Ga-nat the Greedy     (Gnome)",   "Alchemist",   12000, 220, 115, 4, 4,  9},
{"Valeria Starshine     (Elf)",     "Magic Shop",  32000, 175, 110, 5, 2, 11},
{"Andy the Friendly     (Halfling)","General Store", 200, 170, 108, 5, 3, 15},
{"Darg-Low the Grim     (Human)",   "Armory",      10000, 190, 111, 4, 0,  9},
{"Oglign Dragon-Slayer  (Dwarf)",   "Weaponsmith", 32000, 195, 112, 4, 5,  8},
{"Gunnar the Paladin    (Human)",   "Temple",       5000, 185, 110, 5, 0, 23},
{"Mauser the Chemist    (Half-Elf)","Alchemist",   10000, 190, 111, 5, 1,  8},
{"Gopher the Great!     (Gnome)",   "Magic Shop",  20000, 215, 113, 6, 4, 10},
{"Lyar-el the Comely    (Elf)",     "General Store", 300, 165, 107, 6, 2, 18},
{"Mauglim the Horrible  (Half-Orc)","Armory",       3000, 200, 113, 5, 6,  9},
{"Ithyl-Mak the Beastly (Half-Troll)","Weaponsmith",3000, 210, 115, 6, 7,  8},
{"Delilah the Pure      (Half-Elf)","Temple",      25000, 180, 107, 6, 1, 20},
{"Wizzle the Chaotic    (Halfling)","Alchemist",   10000, 190, 110, 6, 3,  8},
{"Inglorian the Mage    (Human?)",  "Magic Shop",  32000, 200, 110, 7, 0, 10}
};
/* object_list[] index of objects that may appear in the store */
// 156 Bytes
UShort store_choice[MAX_STORES][STORE_CHOICES] = {
	/* General Store */
{366,365,364,84,84,365,123,366,365,350,349,348,347,346,346,345,345,345,
	344,344,344,344,344,344,344,344},
	/* Armory	 */
{94,95,96,109,103,104,105,106,110,111,112,114,116,124,125,126,127,129,103,
	104,124,125,91,92,95,96},
	/* Weaponsmith	 */
{29,30,34,37,45,49,57,58,59,65,67,68,73,74,75,77,79,80,81,83,29,30,80,83,
	80,83},
	/* Temple	 */
{322,323,324,325,180,180,233,237,240,241,361,362,57,58,59,260,358,359,265,
	237,237,240,240,241,323,359},
	/* Alchemy shop	 */
{173,174,175,351,351,352,353,354,355,356,357,206,227,230,236,252,253,352,
	353,354,355,356,359,363,359,359},
	/* Magic-User store*/
{318,141,142,153,164,167,168,140,319,320,320,321,269,270,282,286,287,292,
	293,294,295,308,269,290,319,282}
};

static void insert_store(Short store_num, Short pos, Long icost,
			 inven_type *i_ptr) SEC_4;
static void store_create(Short store_num) SEC_4;


/* Returns the value for any given object		-RAK-	*/
Long item_value(inven_type *i_ptr)
{
  Long value;

  value = i_ptr->cost;

  // Items known to be cursed
  /* don't purchase known cursed items */
  if (i_ptr->ident & ID_DAMD)
    return 0;

  // Weapons and armor
  // BOW to SWORD, inclusive, and BOOTS to SOFT_ARMOR, inclusive
  if (((i_ptr->tval >= TV_BOW) && (i_ptr->tval <= TV_SWORD)) ||
      ((i_ptr->tval >= TV_BOOTS) && (i_ptr->tval <= TV_SOFT_ARMOR))) {
    if (!known2_p(i_ptr))
      // unknown item?
      value = object_list[i_ptr->index].cost;
    else if ((i_ptr->tval >= TV_BOW) && (i_ptr->tval <= TV_SWORD)) {
      // weapon...
      if (i_ptr->tohit < 0 || i_ptr->todam < 0 || i_ptr->toac < 0)
	value = 0; // weapon with minusses
      else
	value = i_ptr->cost+(i_ptr->tohit+i_ptr->todam+i_ptr->toac)*100;
    } else {
      // armor...
      value = (i_ptr->toac < 0) ? 0 : i_ptr->cost+i_ptr->toac*100;
    }
    goto groupstack;
  }

  // Ammunition of various sorts
  if ((i_ptr->tval >= TV_SLING_AMMO) && (i_ptr->tval <= TV_SPIKE)) {
    if (!known2_p(i_ptr))
      value = object_list[i_ptr->index].cost;
    else {
      if (i_ptr->tohit < 0 || i_ptr->todam < 0 || i_ptr->toac < 0)
	value = 0;
      else
	/* use 5, because missiles generally appear in groups of 20,
	   so 20 * 5 == 100, which is comparable to weapon bonus above */
	value = i_ptr->cost+(i_ptr->tohit+i_ptr->todam+i_ptr->toac)*5;
    }
    goto groupstack;
  }

  // Potions, scrolls
  if ((i_ptr->tval == TV_SCROLL1) || (i_ptr->tval == TV_SCROLL2) ||
      (i_ptr->tval == TV_POTION1) || (i_ptr->tval == TV_POTION2)) {
    if (!known1_p(i_ptr))
      value = 20;
    goto groupstack;
  }

  // Food
  if (i_ptr->tval == TV_FOOD) {
    if ((i_ptr->subval < (ITEM_SINGLE_STACK_MIN + MAX_MUSH))
	&& !known1_p(i_ptr))
      value = 1;
    goto groupstack;
  }

  // Rings, amulets
  if ((i_ptr->tval == TV_AMULET) || (i_ptr->tval == TV_RING)) {
    if (!known1_p(i_ptr))
      /* player does not know what type of ring/amulet this is */
      value = 45;
    else if (!known2_p(i_ptr))
      /* player knows what type of ring, but does not know whether it is
	 cursed or not, if refuse to buy cursed objects here, then
	 player can use this to 'identify' cursed objects */
      value = object_list[i_ptr->index].cost;
    goto groupstack;
  }

  // Wands, staffs
  if ((i_ptr->tval == TV_STAFF) || (i_ptr->tval == TV_WAND)) {
    if (!known1_p(i_ptr)) {
      value = (i_ptr->tval == TV_WAND) ? 50 : 70;
    } else if (known2_p(i_ptr))
      value = i_ptr->cost + (i_ptr->cost / 20) * i_ptr->p1;
    goto groupstack;
  }

  /* picks and shovels */
  if (i_ptr->tval == TV_DIGGING) {
    if (!known2_p(i_ptr))
      value = object_list[i_ptr->index].cost;
    else {
      if (i_ptr->p1 < 0)
	value = 0;
      else {
	/* some digging tools start with non-zero p1 values, so only
	   multiply the plusses by 100, make sure result is positive */
	value = i_ptr->cost + (i_ptr->p1 - object_list[i_ptr->index].p1) * 100;
	if (value < 0)
	  value = 0;
      }
    }
  }
  
 groupstack:
  /* multiply value by number of items if it is a group stack item */
  if (i_ptr->subval > ITEM_GROUP_MIN) /* do not include torches here */
    value = value * i_ptr->number;
  return value;
}


/* Asking price for an item				-RAK-	*/
Long sell_price(Short snum, Long *max_sell, Long *min_sell, inven_type *item)
{
  Long i;
  UChar adj;
  store_type *s_ptr;
  owner_type *own;

  s_ptr = &store[snum];
  i = item_value(item);
  /* check item->cost in case it is cursed, check i in case it is damaged */
  if ((item->cost <= 0) || (i <= 0))
    return 0;

  own = &owners[s_ptr->owner];
  adj = rgold_adj[own->owner_race][pymisc.prace];
  i = i * adj / 100; // xxx hmm.. will I have truncation problems?
  if (i < 1)
    i = 1;
  *max_sell = i * own->max_inflate / 100;
  *min_sell = i * own->min_inflate / 100;
  if (*min_sell > *max_sell)
    *min_sell = *max_sell;
  return i;
}


/* Check to see if he will be carrying too many objects	-RAK-	*/
Boolean store_check_num(inven_type *t_ptr, Short store_num)
{
  Boolean store_check;
  Short i;
  store_type *s_ptr;
  inven_type *i_ptr;
  inven_record *storeinv;

  s_ptr = &store[store_num];
  storeinv = store_inven[store_num];

  if (s_ptr->store_ctr < STORE_INVEN_MAX)
    return true;

  store_check = false;
  if (t_ptr->subval >= ITEM_SINGLE_STACK_MIN) {
    for (i = 0; i < s_ptr->store_ctr; i++) {
      i_ptr = &storeinv[i].sitem;
      /* note: items with subval of gte ITEM_SINGLE_STACK_MAX only stack
	 if their subvals match */
      if (i_ptr->tval == t_ptr->tval && i_ptr->subval == t_ptr->subval
	  && ((Short)i_ptr->number + (Short)t_ptr->number < 256)
	  && (t_ptr->subval < ITEM_GROUP_MIN
	      || (i_ptr->p1 == t_ptr->p1))) {
	store_check = true;
      }
    }
  }
  return store_check;
}


/* Insert INVEN_MAX at given location	*/
// xxx This is quite a nice mess now, I hope it works
static void insert_store(Short store_num, Short pos, Long icost,
			 inven_type *i_ptr)
{
  Short i;
  store_type *s_ptr;
  inven_record *storeinv, tmp;
  Short offset;
  UChar ctr;

  s_ptr = &store[store_num];
  storeinv = store_inven[store_num];
  for (i = s_ptr->store_ctr-1; i >= pos; i--) {
    // storeinv[i+1] = storeinv[i];   ... equivalent is
    tmp = storeinv[i];
    offset = (i+1) * sizeof(inven_record);
    DmWrite(storeinv, offset, &tmp, sizeof(inven_record));
  }
  // storeinv[pos].sitem = *i_ptr;   ... equivalent is
  offset = pos * sizeof(inven_record);
  DmWrite(storeinv, offset+sizeof(Long), i_ptr, sizeof(inven_type));
  // storeinv[pos].scost = -icost;  ... equivalent is
  icost *= -1;
  DmWrite(storeinv, offset, &icost, sizeof(Long));
  //  s_ptr->store_ctr++;   ... equivalent is
  offset = store_num * sizeof(store_type);
  offset += +sizeof(Long)+sizeof(Short)+sizeof(UChar); // open, insult, owner
  ctr = s_ptr->store_ctr + 1;
  DmWrite(store, offset, &ctr, sizeof(UChar));
}


/* Add the item in INVEN_MAX to stores inventory.	-RAK-	*/
// xxx This needs revised see comments
void store_carry(Short store_num, Short *ipos, inven_type *t_ptr)
{
  Short item_num, item_val;
  Boolean flag;
  Short typ, subt;
  Long icost, dummy;
  inven_type *i_ptr;
  store_type *s_ptr;
  inven_record *storeinv;
  UChar num;
  Short offset;
  Long scost;

  *ipos = -1;
  if (sell_price(store_num, &icost, &dummy, t_ptr) > 0) {
    s_ptr = &store[store_num];
    storeinv = store_inven[store_num];
    item_val = 0;
    item_num = t_ptr->number;
    flag = false;
    typ  = t_ptr->tval;
    subt = t_ptr->subval;
    do {
      i_ptr = &storeinv[item_val].sitem;
      if (typ == i_ptr->tval) {
	if (subt == i_ptr->subval && /* Adds to other item	*/
	    subt >= ITEM_SINGLE_STACK_MIN
	    && (subt < ITEM_GROUP_MIN || i_ptr->p1 == t_ptr->p1)) {
	  *ipos = item_val;
	  // 	  i_ptr->number += item_num; ... equivalent is:
	  num = i_ptr->number + item_num;
	  offset = item_val * sizeof(inven_record) + sizeof(Long);// skip scost
	  DmWrite(storeinv, offset + I_number, &num, sizeof(UChar));
	  /* must set new scost for group items, do this only for items
	     strictly greater than group_min, not for torches, this
	     must be recalculated for entire group */
	  if (subt > ITEM_GROUP_MIN) {
	    sell_price (store_num, &icost, &dummy, i_ptr);
	    //	    storeinv[item_val].scost = -icost;  ... equivalent is:
	    scost = -icost;
	    offset = item_val * sizeof(inven_record);
	    DmWrite(storeinv, offset, &scost, sizeof(Long));
	  }
	  /* must let group objects (except torches) stack over 24
	     since there may be more than 24 in the group */
	  else if (i_ptr->number > 24) {
	    // 	    i_ptr->number = 24; ... equivalent is:
	    num = 24;
	    offset = item_val * sizeof(inven_record) + sizeof(Long);
	    DmWrite(storeinv, offset + I_number, &num, sizeof(UChar));
	  }
	  flag = true;
	}
      } else if (typ > i_ptr->tval) {
	/* Insert into list		*/
	insert_store(store_num, item_val, icost, t_ptr);
	flag = true;
	*ipos = item_val;
      }
      item_val++;
    } while ((item_val < s_ptr->store_ctr) && (!flag));

    if (!flag) {
      /* Becomes last item in list */
      insert_store(store_num, (Short) s_ptr->store_ctr, icost, t_ptr);
      *ipos = s_ptr->store_ctr - 1;
    }
  }
}

/* Destroy an item in the stores inventory.  Note that if	*/
/* "one_of" is false, an entire slot is destroyed	-RAK-	*/
// xxx This has been revised hope the DmWrites work
void store_destroy(Short store_num, Short item_val, Boolean one_of)
{
  Short j, number;
  store_type *s_ptr;
  inven_type *i_ptr;
  inven_record *storeinv, tmp;
  UChar num, ctr;
  Short offset;
  Long scost;

  s_ptr = &store[store_num];
  storeinv = store_inven[store_num];
  i_ptr = &storeinv[item_val].sitem;

  /* for single stackable objects, only destroy one half on average,
     this will help ensure that general store and alchemist have
     reasonable selection of objects */
  if ((i_ptr->subval >= ITEM_SINGLE_STACK_MIN) &&
      (i_ptr->subval <= ITEM_SINGLE_STACK_MAX))
    number = (one_of) ? 1 : randint((Short)i_ptr->number);
  else
    number = i_ptr->number;

  if (number != i_ptr->number) {
    // i_ptr->number -= number; ... equivalent is
    num = i_ptr->number - number;
    offset = item_val * sizeof(inven_record) + sizeof(Long);
    DmWrite(storeinv, offset + I_number, &num, sizeof(UChar));
  } else {
    for (j = item_val; j < s_ptr->store_ctr-1; j++) {
      //      storeinv[j] = storeinv[j+1];   ... equivalent is
      tmp = storeinv[j+1];
      offset = j * sizeof(inven_record);
      DmWrite(storeinv, offset, &tmp, sizeof(inven_record));
    }
    storeinvcopy(storeinv, s_ptr->store_ctr-1, OBJ_NOTHING); // xxx test me!
    // storeinv[s_ptr->store_ctr-1].scost = 0;   ... equivalent is
    scost = 0;
    offset = (s_ptr->store_ctr-1) * sizeof(inven_record);
    DmWrite(storeinv, offset, &scost, sizeof(Long));
    //  s_ptr->store_ctr--;   ... equivalent is
    offset = store_num * sizeof(store_type);
    offset += +sizeof(Long)+sizeof(Short)+sizeof(UChar); // open, insult, owner
    ctr = s_ptr->store_ctr - 1;
    DmWrite(store, offset, &ctr, sizeof(UChar));
  }
}


/* Initializes the stores with owners			-RAK-	*/
void store_init()
{
  Short i, j, k;
  store_type tmp; // , *s_ptr
  inven_record *storeinv;
  Short offset;
  Long scost = 0;

  i = MAX_OWNERS / MAX_STORES;
 // xxx store_init is not implemented yet
  for (j = 0; j < MAX_STORES; j++) {
    //    s_ptr = &store[j];
    storeinv = store_inven[j];
    tmp.store_open = 0;
    tmp.insult_cur = 0;
    tmp.owner = MAX_STORES*(randint(i)-1) + j;
    tmp.store_ctr = 0;
    tmp.good_buy = 0;
    tmp.bad_buy = 0;
    // copy tmp to s_ptr
    DmWrite(store, j*sizeof(store_type), &tmp, sizeof(store_type));
    for (k = 0; k < STORE_INVEN_MAX; k++) {
      storeinvcopy(storeinv, k, OBJ_NOTHING); // xxx test me!
      // set storeinv[k].scost to 0;
      offset = k * sizeof(inven_record);
      DmWrite(storeinv, offset, &scost, sizeof(Long));
    }
  }
}


/* Creates an item and inserts it into store's inven	-RAK-	*/
static void store_create(Short store_num)
{
  Short i, tries;
  Short cur_pos, dummy;
  store_type *s_ptr;
  inven_type *t_ptr;

  tries = 0;
  cur_pos = popt();
  s_ptr = &store[store_num];
 // xxx store_create is not implemented yet
  do {
    i = store_choice[store_num][randint(STORE_CHOICES)-1];
    invcopy(t_list, cur_pos, i);
    magic_treasure(cur_pos, OBJ_TOWN_LEVEL);
    t_ptr = &t_list[cur_pos];
    if (store_check_num(t_ptr, store_num)) {
      if ((t_ptr->cost > 0) &&	// * Item must be good	*
	  (t_ptr->cost < owners[s_ptr->owner].max_cost)) {
	// * equivalent to calling ident_spell(), except will not
	//   change the object_ident array *
	//	store_bought(t_list, cur_pos); // xxx fatal exception
	invy_set_ident(t_list, cur_pos,
		       (t_list[cur_pos].ident | ID_KNOWN2 | ID_STOREBOUGHT));
	store_carry(store_num, &dummy, t_ptr);
	tries = 10;
      }
    }
    tries++;
  } while (tries <= 3);
  pusht((UChar)cur_pos);
}


/* Initialize and up-keep the store's inventory.		-RAK-	*/
void store_maint()
{
  Short i, j, ic = 0;
  store_type *s_ptr;

 // xxx This is not implemented yet
  for (i = 0; i < MAX_STORES; i++) {
    s_ptr = &store[i];
    //    s_ptr->insult_cur = 0;  .. equivalent is
    DmWrite(store, i*sizeof(store_type)+sizeof(Long), &ic, sizeof(Short));
    
    if (s_ptr->store_ctr >= STORE_MIN_INVEN) {
      j = randint(STORE_TURN_AROUND);
      if (s_ptr->store_ctr >= STORE_MAX_INVEN)
	j += 1 + s_ptr->store_ctr - STORE_MAX_INVEN;
      while (--j >= 0)
	store_destroy(i, randint((Short)s_ptr->store_ctr)-1, false);
    }
    // no crashes up to here..
    if (s_ptr->store_ctr <= STORE_MAX_INVEN) {
      j = randint(STORE_TURN_AROUND);
      if (s_ptr->store_ctr < STORE_MIN_INVEN)
	j += STORE_MIN_INVEN - s_ptr->store_ctr;
      while (--j >= 0)
	store_create(i);
    }
  }
}

/* eliminate need to bargain if player has haggled well in the past   -DJB- */
Boolean noneedtobargain(Short store_num, Long minprice)
{
  Short bargain_record;
  Long br2;
  store_type *s_ptr;

  s_ptr = &store[store_num];
  if (s_ptr->good_buy == MAX_SHORT)
    return true;
  bargain_record = (s_ptr->good_buy - 3 * s_ptr->bad_buy - 5);
  br2 = (Long)bargain_record * (Long)bargain_record;
  return ((bargain_record > 0) && (br2 > minprice/50));
}


/* update the bargin info					-DJB- */
void updatebargain(Short store_num, Long price, Long minprice)
{
  store_type *s_ptr;
  Short offset;
  UShort val;

  s_ptr = &store[store_num];
  offset = store_num*sizeof(store_type);
  offset += sizeof(Long) + sizeof(Short) + 2*sizeof(UChar);
  if (minprice > 9) {
    if (price == minprice) {
      if (s_ptr->good_buy < MAX_SHORT) {
	//	s_ptr->good_buy++; // xxx This is not implemented yet
	val = s_ptr->good_buy + 1;
	DmWrite(store, offset, &val, sizeof(UShort));
      }
    } else {
      if (s_ptr->bad_buy < MAX_SHORT) {
	offset += sizeof(UShort);
	//	s_ptr->bad_buy++; // xxx This is not implemented yet
	val = s_ptr->bad_buy + 1;
	DmWrite(store, offset, &val, sizeof(UShort));
      }
    }
  }
}
