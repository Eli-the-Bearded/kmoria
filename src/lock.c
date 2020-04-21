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

// Note.. anything with DmWrite of a STRUCT should live here if it can.
// E.g. anything marked 'danger will robinson'

//player_type py;
struct flags pyflags;
struct stats pystats;
struct misc pymisc;

// externy globals, not used here but they needed a home.
Long turn = 0;	/* Cur trun of game    */
Short cur_height, cur_width;	/* Cur dungeon size    */
/*  Following are calculated from max dungeon sizes		*/
Short max_panel_rows, max_panel_cols;
Short panel_row, panel_col;
Short panel_row_min = 0, panel_row_max = 0;
Short panel_col_min = 0, panel_col_max = 0;
Short char_row;
Short char_col;
Short dun_level = 0;

ULong town_seed;

Short inven_weight = 0;
Boolean weapon_heavy = false;
Short pack_heavy = 0;
Boolean free_turn_flag, new_level_flag;
/* command_count is a very interesting global variable.
   it is the number of times to repeat a command (if the command is
   susceptible to repetition.)  it is reset to 0 by disturb() and message().*/
Short command_count = 0;
Short equip_ctr = 0;
Boolean light_flag=false, find_flag, find_prself=true, player_light, death;
Boolean find_bound = false; // "Running: stop when map sector changes"
Boolean total_winner, find_ignore_doors, find_examine, find_cut;
Boolean teleport_flag, prompt_carry_flag, highlight_seams;
Char died_from[25];
Short mon_tot_mult, hack_monptr;
ULong spell_learned=0, spell_worked; // bit field for spells learned/tried
ULong spell_forgotten;

// Create and LOCK some database records.
static void create_cave_rec() SEC_4;
static void cave_record_lock(Boolean unlocked) SEC_4;
static void create_tlist_rec() SEC_4;
static void tlist_record_lock(Boolean unlocked) SEC_4;
static void objnames_record_lock(Boolean unlocked) SEC_4;
static void objectlist_record_lock(Boolean unlocked) SEC_4;
static void cnames_record_lock(Boolean unlocked) SEC_4;
static void creaturelist_record_lock(Boolean unlocked) SEC_4;
static void cnames_record_lock(Boolean unlocked) SEC_4;
static void create_mlist_rec() SEC_4;
static void mlist_record_lock(Boolean unlocked) SEC_4;
static void tlevel_record_lock(Boolean unlocked) SEC_4;
static void sortobj_record_lock(Boolean unlocked) SEC_4;
static void mlevel_record_lock(Boolean unlocked) SEC_4;
static void create_inventory_rec() SEC_4;
static void inventory_record_lock(Boolean unlocked) SEC_4;
static void create_objident_rec() SEC_4;
static void objident_record_lock(Boolean unlocked) SEC_4;
static void create_recall_rec() SEC_4;
static void recall_record_lock(Boolean unlocked) SEC_4;
static void create_store_rec() SEC_4; // also calls inventories
static void store_record_lock(Boolean unlocked) SEC_4;// also calls inventories
//static void owner_record_lock(Boolean unlocked);
static void haggle_record_lock(Boolean unlocked) SEC_4;

void data_record_create() {
  create_cave_rec();
  create_tlist_rec();
  create_mlist_rec();
  create_inventory_rec();
  create_objident_rec();
  create_recall_rec();
  create_store_rec();
}

void data_record_lock(Boolean b) {
  // also take care of random seeds
  if (b)     seed_random();
  // These are the highly variable records:
  cave_record_lock(b);
  tlist_record_lock(b);
  mlist_record_lock(b);
  inventory_record_lock(b);
  objident_record_lock(b);
  recall_record_lock(b);
  store_record_lock(b);
  // these are const-like records:
  objnames_record_lock(b);
  objectlist_record_lock(b);  
  creaturelist_record_lock(b);  
  cnames_record_lock(b);
  tlevel_record_lock(b);
  sortobj_record_lock(b);
  mlevel_record_lock(b);
  haggle_record_lock(b);
  //  owner_record_lock(b);
  // also take care of random seeds
  if (b)   town_seed = TimGetSeconds();
  alloc_message_log();
}

/**********************************************************************
 *                                                                    *
 *         "the dungeon" ... stuff that was cave_type                 *
 *                                                                    *
 **********************************************************************/

// The layout is: ---row---  ---row--- ---row--- ...
//cave_type *cave_rec = NULL;
UChar *cave_rec[4] = {NULL,NULL,NULL,NULL};
UChar *cave_cptr = NULL;
UChar *cave_tptr = NULL;
UChar *cave_fval = NULL;
UChar *cave_light = NULL;

// Create the cave storage record (will delete it if it already exists)
static void create_cave_rec()
{
  ULong uniqueID;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  ULong size;
  // Delete existing cave record
  for (uniqueID = REC_CAVE_CPTR ; uniqueID <= REC_CAVE_LIGHT ; uniqueID++) {
    if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
      DmRemoveRecord(MoriaSaveDB, index);      // EX TERM IN ATE !
    
    index = DmNumRecords(MoriaSaveDB);
    size = MAX_WIDTH * MAX_HEIGHT * sizeof(UChar); // this is 13K, honking big.
    vh = DmNewRecord(MoriaSaveDB, &index, size);
    p = MemHandleLock(vh);
    if (p)
      DmSet(p, 0, size, 0);
    MemPtrUnlock(p);
    DmReleaseRecord(MoriaSaveDB, index, true);
    DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  }
  return;
}

// Set the cave record to 0's.  Assumes it has been locked.
void blank_cave()
{
  Short i;
  ULong size = sizeof(UChar) * MAX_WIDTH * MAX_HEIGHT;
  VoidPtr p;
  //  if (cave_rec[0] == NULL) message("foo!");
  //  p = cave_rec[0];
  //    DmSet(p, 0, 4, 0);
  //  return;
  // Delete existing cave record
  for (i = 0 ; i < 4 ; i++) {
    p = cave_rec[i];
    DmSet(p, 0, size, 0);
  }
}

// Lock the cave record so that we can write to it throughout the session
// If unlocked=false, UNLOCk the cave record instead.
static void cave_record_lock(Boolean unlocked)
{
  ULong uniqueID;
  static UInt index[4];
  static VoidHand vh[4];
  VoidPtr p;
  Short i = 0;
  if (unlocked) {
    // lock record
    for (uniqueID = REC_CAVE_CPTR, i = 0 ; i < 4 ; i++, uniqueID++) {
      if (0!=DmFindRecordByID(MoriaSaveDB, uniqueID, index+i)) return;
      vh[i] = DmGetRecord(MoriaSaveDB, index[i]);
      p = MemHandleLock(vh[i]);
      cave_rec[i] = (UChar *) p;
    }
    cave_cptr = cave_rec[CAVE_cptr];
    cave_tptr = cave_rec[CAVE_tptr];
    cave_fval = cave_rec[CAVE_fval];
    cave_light = cave_rec[CAVE_light];
  } else {
    // unlock record
    // I get a fatal error.  SIGH.
    for (i = 0 ; i < 4 ; i++) {
      if (cave_rec[i] != NULL) {
	cave_rec[i] = NULL;
	MemHandleUnlock(vh[i]);
	DmReleaseRecord(MoriaSaveDB, index[i], true);
      }
    }
  }
}
// Use this to write the struct elements
// CAVE_{cptr, tptr, fval, light}.
void cave_set(Short row, Short col, Short element, UChar val)
{
  Short i = row * MAX_WIDTH + col;
  if (row >= MAX_HEIGHT || col >= MAX_WIDTH || row < 0 || col < 0) {
    // caller is a mo-ron
    //    message(moron_caller); // "eek"
    message("eek");
    return;
  }
  DmWrite(cave_rec[element], i*sizeof(UChar), &val, sizeof(UChar));
}


/**********************************************************************
 *                                                                    *
 *  "objects on the level" ... t_list which was inven_type            *
 *                                                                    *
 **********************************************************************/

// MAX_TALLOC = 175
// sizeof(inven_type) = 40 B (saved 700 B by making 4 shorts into chars:
// tohit, todam, ac, toac.  keep an eye on them though in case of overflow.)

inven_type *t_list = NULL;
Short tcptr = MIN_TRIX; // index of the next free slot in t_list

// Create the cave storage record (will delete it if it already exists)
static void create_tlist_rec()
{
  ULong uniqueID;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  ULong size = sizeof(inven_type) * MAX_TALLOC;
  // Delete existing cave record
  uniqueID = REC_TLIST;
  if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
    DmRemoveRecord(MoriaSaveDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaSaveDB);
  vh = DmNewRecord(MoriaSaveDB, &index, size);
  p = MemHandleLock(vh);
  if (p)
    DmSet(p, 0, size, 0);
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaSaveDB, index, true);
  DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  tcptr = MIN_TRIX;
  return;
}

// Set the tlist record to 0's.  Assumes it has been locked!!
void blank_tlist()
{
  ULong size = sizeof(inven_type) * MAX_TALLOC;
  VoidPtr p;
  p = t_list;
  DmSet(p, 0, size, 0);
  tcptr = MIN_TRIX;
}

// Lock the cave record so that we can write to it throughout the session
// If unlocked=false, UNLOCk the cave record instead.
static void tlist_record_lock(Boolean unlocked)
{
  ULong uniqueID;
  static UInt index;
  static VoidHand vh;
  VoidPtr p;
  if (unlocked) {
    // lock record
    uniqueID = REC_TLIST;
    if (0!=DmFindRecordByID(MoriaSaveDB, uniqueID, &index)) return;
    vh = DmGetRecord(MoriaSaveDB, index);
    p = MemHandleLock(vh);
    t_list = (inven_type *) p;
  } else {
    // unlock record
    if (t_list != NULL) {
      t_list = NULL;
      MemHandleUnlock(vh);
      DmReleaseRecord(MoriaSaveDB, index, true);
    }
  }
}
// Use this to write the struct elements. somehow.
/*
void tlist_set(...)
{
  DmWrite(t_list, offset, &val, sizeof(UChar));
}
*/

/*
void tlist_set_p1(Short index, Short p1)
{
  DmWrite(t_list, index * sizeof(inven_type) + I_p1, &p1, sizeof(Short));
}

void tlist_set_flags(Short index, ULong flags)
{
  DmWrite(t_list, index * sizeof(inven_type) + I_flags, &flags, sizeof(ULong));
}
*/

void tlist_mv(Short from, Short to)
{
  inven_type tmp;
  // copy what's at 'from' index to the 'to' index.
  tmp = t_list[from];
  DmWrite(t_list, to*sizeof(inven_type), &tmp, sizeof(inven_type));
}

/**********************************************************************
 *                                                                    *
 *  "canonical objects" ... object_list which was treasure_type       *
 *                                                                    *
 **********************************************************************/
// this has been created for us in the constant-data database.  yay.
// make sure this code is kept in sync with its creation code in BuildDB

treasure_type *object_list = NULL;
Char *object_names = NULL;

// This record is not locked in the same sense the others are..
// It is not being written to.  Also note this is a different database!
static void objectlist_record_lock(Boolean unlocked)
{
  ULong uniqueID = REC_OBJECTLIST;
  UInt index;
  static VoidHand vh;
  VoidPtr p = NULL;
  if (unlocked) {
    // lock record
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh = DmQueryRecord(MoriaDB, index);
    p = MemHandleLock(vh);
    object_list = (treasure_type *) p;
  } else {
    // unlock record
    if (object_list != NULL) {
      object_list = NULL;
      MemHandleUnlock(vh);
    }
  }
}
// Similar. The 1st field in treasure_type indexes the obj's name in this array
static void objnames_record_lock(Boolean unlocked)
{
  ULong uniqueID = REC_OBJLIST_NAMES;
  UInt index;
  static VoidHand vh;
  VoidPtr p = NULL;
  if (unlocked) {
    // lock record
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh = DmQueryRecord(MoriaDB, index);
    p = MemHandleLock(vh);
    object_names = (Char *) p;
  } else {
    // unlock record
    if (object_names != NULL) {
      object_names = NULL;
      MemHandleUnlock(vh);
    }
  }
}

// this was in desc.c.
//  it copies from object_list (a treasure_type) to a inven_type.
// it needs to be revised cause sometimes we need to copy from not object_list.
inven_type to; // I reckon I don't want this on the call stack
static void invcopy_helper(VoidPtr *to_rec, Short to_index,
			   Short from_index, Short offset)
{
  treasure_type *from;
  //  return;
  if (to_rec == NULL || object_list == NULL) return; // error!
  if (from_index >= MAX_OBJECTS || from_index < 0) return; // error!
  // I should also check to_index, I suppose.  Depends on what to_rec is..
  from = object_list + from_index;

  to.index     = from_index;
  to.name2     = SN_NULL;
  to.inscrip[0]= '\0';
  to.flags     = from->flags;
  to.tval      = from->tval;
  to.tchar     = from->tchar;
  to.p1        = from->p1;
  to.cost      = from->cost;
  to.subval    = from->subval;
  to.number    = from->number;
  to.weight    = from->weight;
  to.tohit     = from->tohit;
  to.todam     = from->todam;
  to.ac        = from->ac;
  to.toac      = from->toac;
  to.damage[0] = from->damage[0];
  to.damage[1] = from->damage[1];
  to.level     = from->level;
  to.ident     = 0;

  // now... write the record!
  offset += to_index*sizeof(inven_type);
  DmWrite(to_rec, offset, &to, sizeof(inven_type));
  // hooray!
}
void invcopy(inven_type *to_rec, Short to_index,Short from_index)
{
  invcopy_helper((VoidPtr *) to_rec, to_index, from_index, 0);
}
// life's a bitch, then you fail DmWriteCheck
// this should copy to the 'sitem' of 'a_store_inventory[to_index]'
void storeinvcopy(inven_record *to_rec, Short to_index, Short from_index)
{
  invcopy_helper((VoidPtr *) to_rec, to_index, from_index, sizeof(Long));
}


/**********************************************************************
 *                                                                    *
 *  "canonical monsters" ... c_list which was creature_type           *
 *                                                                    *
 **********************************************************************/
// this has been created for us in the constant-data database.  yay.
// make sure this code is kept in sync with its creation code in BuildDB/

creature_type *c_list = NULL;
Char *c_names = NULL;

// This record is not locked in the same sense the others are..
// It is not being written to.  Also note this is not the Save database!
static void creaturelist_record_lock(Boolean unlocked)
{
  ULong uniqueID = REC_CREATURELIST;
  UInt index;
  static VoidHand vh;
  VoidPtr p = NULL;
  if (unlocked) {
    // lock record
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh = DmQueryRecord(MoriaDB, index);
    p = MemHandleLock(vh);
    c_list = (creature_type *) p;
  } else {
    // unlock record
    if (c_list != NULL) {
      c_list = NULL;
      MemHandleUnlock(vh);
    }
  }
}
// Similar. The 1st field in creature_type indexes the mon's name in this array
static void cnames_record_lock(Boolean unlocked)
{
  ULong uniqueID = REC_CLIST_NAMES;
  UInt index;
  static VoidHand vh;
  VoidPtr p = NULL;
  if (unlocked) {
    // lock record
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh = DmQueryRecord(MoriaDB, index);
    p = MemHandleLock(vh);
    c_names = (Char *) p;
  } else {
    // unlock record
    if (c_names != NULL) {
      c_names = NULL;
      MemHandleUnlock(vh);
    }
  }
}

/**********************************************************************
 *                                                                    *
 *  "monsters on the level" ... m_list which was monster_type         *
 *                                                                    *
 **********************************************************************/

// MAX_MALLOC = 125
// sizeof(monster_type) = 14 B.  so, not so bad.

monster_type *m_list = NULL;
Short mfptr = MIN_MONIX; // index of the next free slot in m_list
//monster_type blank_monster = {0,0,0,0,0,0,0,false,0,0};

// Create the storage record (will delete it if it already exists)
static void create_mlist_rec()
{
  ULong uniqueID;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  ULong size = sizeof(monster_type) * MAX_MALLOC;
  // Delete existing record
  uniqueID = REC_MLIST;
  if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
    DmRemoveRecord(MoriaSaveDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaSaveDB);
  vh = DmNewRecord(MoriaSaveDB, &index, size);
  p = MemHandleLock(vh);
  if (p)
    DmSet(p, 0, size, 0);
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaSaveDB, index, true);
  DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  mfptr = MIN_MONIX;
  return;
}

// Set the mlist record to 0's.  Assumes it has been locked!!
void blank_mlist()
{
  ULong size = sizeof(monster_type) * MAX_MALLOC;
  VoidPtr p;
  p = m_list;
  DmSet(p, 0, size, 0);
  mfptr = MIN_MONIX;
}

// Lock the storage record so that we can write to it throughout the session
// If unlocked=false, UNLOCk the record instead.
static void mlist_record_lock(Boolean unlocked)
{
  ULong uniqueID;
  static UInt index;
  static VoidHand vh;
  VoidPtr p;
  if (unlocked) {
    // lock record
    uniqueID = REC_MLIST;
    if (0!=DmFindRecordByID(MoriaSaveDB, uniqueID, &index)) return;
    vh = DmGetRecord(MoriaSaveDB, index);
    p = MemHandleLock(vh);
    m_list = (monster_type *) p;
  } else {
    // unlock record
    if (m_list != NULL) {
      m_list = NULL;
      MemHandleUnlock(vh);
      DmReleaseRecord(MoriaSaveDB, index, true);
    }
  }
}

// allow you to set hp, csleep, cspeed
// see record.h for appropriate offset #define's
void mlist_set_short(Short mptr, Short offset, Short val)
{
  DmWrite(m_list, mptr*sizeof(monster_type)+offset, &val, sizeof(Short));
}
// allow you to set fy, fx, cdis, stunned, confused
// see record.h for appropriate offset #define's
void mlist_set_uchar(Short mptr, Short offset, UChar val)
{
  DmWrite(m_list, mptr*sizeof(monster_type)+offset, &val, sizeof(UChar));
}
// allow you to set ml
void mlist_set_ml(Short mptr, Boolean ml)
{
  Short offset = MLIST_ml;
  DmWrite(m_list, mptr*sizeof(monster_type)+offset, &ml, sizeof(Boolean));
}

void mlist_mv(Short from, Short to)
{
  monster_type tmp;
  // copy what's at 'from' index to the 'to' index.
  // I'll be sorry if I do a DmWrite from one part of the record to the other:
  tmp = m_list[from];
  DmWrite(m_list, to*sizeof(monster_type), &tmp, sizeof(monster_type));
}

/**********************************************************************
 *                                                                    *
 *  "?" ... t_level which was Short                                   *
 *                                                                    *
 **********************************************************************/
Short *t_level = NULL;
// another read-only
static void tlevel_record_lock(Boolean unlocked)
{
  ULong uniqueID = REC_TLEVEL;
  UInt index;
  static VoidHand vh;
  VoidPtr p = NULL;
  if (unlocked) {
    // lock record
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh = DmQueryRecord(MoriaDB, index);
    p = MemHandleLock(vh);
    t_level = (Short *) p;
  } else {
    // unlock record
    if (t_level != NULL) {
      t_level = NULL;
      MemHandleUnlock(vh);
    }
  }
}
/**********************************************************************
 *                                                                    *
 *  "?" ... sorted_objects which was Short                            *
 *                                                                    *
 **********************************************************************/
Short *sorted_objects = NULL;
// another read-only
static void sortobj_record_lock(Boolean unlocked)
{
  ULong uniqueID = REC_SORTEDOBJ;
  UInt index;
  static VoidHand vh;
  VoidPtr p = NULL;
  if (unlocked) {
    // lock record
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh = DmQueryRecord(MoriaDB, index);
    p = MemHandleLock(vh);
    sorted_objects = (Short *) p;
  } else {
    // unlock record
    if (sorted_objects != NULL) {
      sorted_objects = NULL;
      MemHandleUnlock(vh);
    }
  }
}
/**********************************************************************
 *                                                                    *
 *  "?" ... m_level which was Short                                   *
 *                                                                    *
 **********************************************************************/
Short *m_level = NULL;
// another read-only
static void mlevel_record_lock(Boolean unlocked)
{
  ULong uniqueID = REC_MLEVEL;
  UInt index;
  static VoidHand vh;
  VoidPtr p = NULL;
  if (unlocked) {
    // lock record
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh = DmQueryRecord(MoriaDB, index);
    p = MemHandleLock(vh);
    m_level = (Short *) p;
  } else {
    // unlock record
    if (m_level != NULL) {
      m_level = NULL;
      MemHandleUnlock(vh);
    }
  }
}


/**********************************************************************
 *                                                                    *
 *  "your pack" ... inventory which was inven_type                    *
 *                                                                    *
 **********************************************************************/

// INVEN_ARRAY_SIZE = 34
// sizeof(inven_type) = 40 B (saved 700 B by making 4 shorts into chars:
// tohit, todam, ac, toac.  keep an eye on them though in case of overflow.)

inven_type *inventory = NULL;
Short inven_ctr = 0; // index of the next free slot in inventory

// Create the cave storage record (will delete it if it already exists)
static void create_inventory_rec()
{
  ULong uniqueID;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  ULong size = sizeof(inven_type) * INVEN_ARRAY_SIZE;
  // Delete existing record
  uniqueID = REC_INVENTORY;
  if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
    DmRemoveRecord(MoriaSaveDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaSaveDB);
  vh = DmNewRecord(MoriaSaveDB, &index, size);
  p = MemHandleLock(vh);
  if (p)
    DmSet(p, 0, size, 0);
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaSaveDB, index, true);
  DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  inven_ctr = 0;
  return;
}

// Set the tlist record to 0's.  Assumes it has been locked!!
// But this is not the same as setting it to OBJ_NOTHING.  Argh.
void blank_inventory()
{
  ULong size = sizeof(inven_type) * INVEN_ARRAY_SIZE;
  VoidPtr p;
  p = inventory;
  DmSet(p, 0, size, 0);
  inven_ctr = 0;
}

// Lock the record so that we can write to it throughout the session
// If unlocked=false, UNLOCk the record instead.
static void inventory_record_lock(Boolean unlocked)
{
  ULong uniqueID;
  static UInt index;
  static VoidHand vh;
  VoidPtr p;
  if (unlocked) {
    // lock record
    uniqueID = REC_INVENTORY;
    if (0!=DmFindRecordByID(MoriaSaveDB, uniqueID, &index)) return;
    vh = DmGetRecord(MoriaSaveDB, index);
    p = MemHandleLock(vh);
    inventory = (inven_type *) p;
  } else {
    // unlock record
    if (inventory != NULL) {
      inventory = NULL;
      MemHandleUnlock(vh);
      DmReleaseRecord(MoriaSaveDB, index, true);
    }
  }
}

void invy_set_number(inven_type *iptr, Short index, UChar n)
{
  // set the inventory[i].number to n
  Short offset = I_number;
  // offset = 2*sizeof(Short) + (4+INSCRIP_SIZE)*sizeof(Char) + 2*sizeof(Long);
  DmWrite(iptr, index*sizeof(inven_type)+offset, &n, sizeof(UChar));
}

void invy_set_flags(inven_type *iptr, Short index, ULong n)
{
  Short offset = I_flags;
  //offset = sizeof(Short) + INSCRIP_SIZE*sizeof(Char) + sizeof(UChar);
  DmWrite(iptr, index*sizeof(inven_type)+offset, &n, sizeof(ULong));
}

void invy_set_p1(inven_type *iptr, Short index, Short n)
{
  Short offset = I_p1;
  //  offset = sizeof(Short) + (3+INSCRIP_SIZE)*sizeof(Char) + sizeof(ULong);
  DmWrite(iptr, index*sizeof(inven_type)+offset, &n, sizeof(Short));
}

void invy_set_cost(inven_type *iptr, Short index, Long cost)
{
  Short offset = I_cost;
  DmWrite(iptr, index*sizeof(inven_type)+offset, &cost, sizeof(Long));
}


void invy_set_hitdamac(inven_type *iptr, Short index,
			    Char tohit, Char todam, Char toac)
{
  Short offset;
  //offset = 3*sizeof(Short) + (5+INSCRIP_SIZE)*sizeof(Char) + 2*sizeof(Long);
  offset = I_tohit;
  DmWrite(iptr, index*sizeof(inven_type)+offset, &tohit, sizeof(Char));
  offset = I_todam;
  DmWrite(iptr, index*sizeof(inven_type)+offset, &todam, sizeof(Char));
  offset = I_toac;
  DmWrite(iptr, index*sizeof(inven_type)+offset, &toac, sizeof(Char));
}

void invy_set_ident(inven_type *iptr, Short index, UChar ident)
{
  // set the inventory[i].ident to ident
  Short offset = I_ident;
  // it's at the end... way irritating.  skip 3s, 2L, 12+13c
  //offset = 3*sizeof(Short) + (12+INSCRIP_SIZE)*sizeof(Char) + 2*sizeof(Long);
  DmWrite(iptr, index*sizeof(inven_type)+offset, &ident, sizeof(UChar));
}

void invy_set_uchar(inven_type *iptr, Short index, Short offset, UChar val)
{
  DmWrite(iptr, index*sizeof(inven_type)+offset, &val, sizeof(UChar));
}

void invy_set_name2(inven_type *iptr, Short index, UChar val)
{
  Short offset = I_name2;
  DmWrite(iptr, index*sizeof(inven_type)+offset, &val, sizeof(UChar));
}

void invy_set_index(inven_type *iptr, Short index, UShort val)
{
  Short offset = I_index;
  DmWrite(iptr, index*sizeof(inven_type)+offset, &val, sizeof(UShort));
}


// Delete index item_val and shift item_val+1, etc, down one.
void invy_delete(Short item_val)
{
  Short j;
  for (j = item_val; j < inven_ctr-1; j++)
    // this will probably die in some evil way
    DmWrite(inventory,j*sizeof(inven_type),&inventory[j+1],sizeof(inven_type));
  invcopy(inventory, inven_ctr-1, OBJ_NOTHING);
  inven_ctr--;
}

// Shift locn to locn+1, etc, leaving a space at locn
void invy_insert(Short locn, inven_type *copyme)
{
  Short j;
  inven_type tmp;
  if (locn<0) return; // caller is moron
  for (j = inven_ctr-1; j >= locn; j--) {
    // this will probably die in some evil way
    // yep it did
    // DmWrite(inventory+j+1, 0, inventory+j, sizeof(inven_type));
    tmp = inventory[j];
    //    message("hello?");
    //    SysTaskDelay(SysTicksPerSecond());
    DmWrite(inventory, (j+1)*sizeof(inven_type), &tmp, sizeof(inven_type));
  }
  //  message("(bye)");
  //  SysTaskDelay(SysTicksPerSecond());
  DmWrite(inventory, locn*sizeof(inven_type), copyme, sizeof(inven_type));
  inven_ctr++;
}

// Swap two items.
void invy_swap(Short item_a, Short item_b)
{
  inven_type tmp;
  // keep a copy of A.  I think this will work.  Maybe not though.
  tmp = inventory[item_a]; 
  // write B where A used to be
  DmWrite(inventory, item_a*sizeof(inven_type),
	  &(inventory[item_b]), sizeof(inven_type));
  // write A where B used to be
  DmWrite(inventory, item_b*sizeof(inven_type), &tmp, sizeof(inven_type));
}

/**********************************************************************
 *                                                                    *
 *  "identification status" ... object_ident which was UChar          *
 *                                                                    *
 **********************************************************************/

// OBJECT_IDENT_SIZE = 448 !  wow !

UChar *object_ident = NULL;
//Short inven_ctr = 0; // index of the next free slot in t_list

// Create the record (will delete it if it already exists)
static void create_objident_rec()
{
  ULong uniqueID;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  ULong size = sizeof(UChar) * OBJECT_IDENT_SIZE;
  // Delete existing record
  uniqueID = REC_OBJIDENT;
  if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
    DmRemoveRecord(MoriaSaveDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaSaveDB);
  vh = DmNewRecord(MoriaSaveDB, &index, size);
  p = MemHandleLock(vh);
  if (p)
    DmSet(p, 0, size, 0);
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaSaveDB, index, true);
  DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  //  inven_ctr = 0;
  return;
}

// Set the tlist record to 0's.  Assumes it has been locked!!
void blank_objident()
{
  ULong size = sizeof(UChar) * OBJECT_IDENT_SIZE;
  VoidPtr p;
  p = object_ident;
  DmSet(p, 0, size, 0);
  //  inven_ctr = 0;
}

// Lock the record so that we can write to it throughout the session
// If unlocked=false, UNLOCk the record instead.
static void objident_record_lock(Boolean unlocked)
{
  ULong uniqueID;
  static UInt index;
  static VoidHand vh;
  VoidPtr p;
  if (unlocked) {
    // lock record
    uniqueID = REC_OBJIDENT;
    if (0!=DmFindRecordByID(MoriaSaveDB, uniqueID, &index)) return;
    vh = DmGetRecord(MoriaSaveDB, index);
    p = MemHandleLock(vh);
    object_ident = (UChar *) p;
  } else {
    // unlock record
    if (object_ident != NULL) {
      object_ident = NULL;
      MemHandleUnlock(vh);
      DmReleaseRecord(MoriaSaveDB, index, true);
    }
  }
}

void object_ident_or(Short index, UChar or_me)
{
  or_me |= object_ident[index];
  DmWrite(object_ident, index*sizeof(UChar), &or_me, sizeof(UChar));
}

void object_ident_nand(Short index, UChar nand_me)
{
  nand_me = object_ident[index] & ~nand_me;
  DmWrite(object_ident, index*sizeof(UChar), &nand_me, sizeof(UChar));
}


/**********************************************************************
 *                                                                    *
 *  "monster recall" ... c_recall which was recall_type               *
 *                                                                    *
 **********************************************************************/

// OBJECT_IDENT_SIZE = 448 !  wow !

recall_type *c_recall = NULL;
//Short inven_ctr = 0; // index of the next free slot in t_list

// Create the record (will delete it if it already exists)
static void create_recall_rec()
{
  ULong uniqueID;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  ULong size = sizeof(recall_type) * MAX_CREATURES;
  // Delete existing record
  uniqueID = REC_RECALL;
  if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
    DmRemoveRecord(MoriaSaveDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaSaveDB);
  vh = DmNewRecord(MoriaSaveDB, &index, size);
  p = MemHandleLock(vh);
  if (p)
    DmSet(p, 0, size, 0);
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaSaveDB, index, true);
  DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  //  inven_ctr = 0;
  return;
}

// Set the record to 0's.  Assumes it has been locked!!
// Note: This should probably never be called!!!
void blank_recall()
{
  ULong size = sizeof(recall_type) * MAX_CREATURES;
  VoidPtr p;
  p = c_recall;
  DmSet(p, 0, size, 0);
  //  inven_ctr = 0;
}

// Lock the record so that we can write to it throughout the session
// If unlocked=false, UNLOCk the record instead.
static void recall_record_lock(Boolean unlocked)
{
  ULong uniqueID;
  static UInt index;
  static VoidHand vh;
  VoidPtr p;
  if (unlocked) {
    // lock record
    uniqueID = REC_RECALL;
    if (0!=DmFindRecordByID(MoriaSaveDB, uniqueID, &index)) return;
    vh = DmGetRecord(MoriaSaveDB, index);
    p = MemHandleLock(vh);
    c_recall = (recall_type *) p;
  } else {
    // unlock record
    if (c_recall != NULL) {
      c_recall = NULL;
      MemHandleUnlock(vh);
      DmReleaseRecord(MoriaSaveDB, index, true);
    }
  }
}

void recall_cmove_or(Short index, ULong or_me)
{
  Short offset = 0;
  or_me |= c_recall[index].r_cmove;
  DmWrite(c_recall, offset+index*sizeof(recall_type), &or_me, sizeof(ULong));
}
void recall_set_cmove(Short index, ULong val)
{
  Short offset = 0;
  DmWrite(c_recall, offset+index*sizeof(recall_type), &val, sizeof(ULong));
}
void recall_spells_or(Short index, ULong or_me)
{
  Short offset = sizeof(ULong);
  or_me |= c_recall[index].r_spells;
  DmWrite(c_recall, offset+index*sizeof(recall_type), &or_me, sizeof(ULong));
}
void recall_set_spells(Short index, ULong val)
{
  Short offset = sizeof(ULong);
  DmWrite(c_recall, offset+index*sizeof(recall_type), &val, sizeof(ULong));
}
void recall_increment_kills(Short index)
{
  Short offset = 2*sizeof(ULong);
  UShort k = c_recall[index].r_kills + 1;
  DmWrite(c_recall, offset+index*sizeof(recall_type), &k, sizeof(UShort));
}
void recall_increment_deaths(Short index)
{
  Short offset = 2*sizeof(ULong) + sizeof(UShort);
  UShort k = c_recall[index].r_deaths + 1;
  DmWrite(c_recall, offset+index*sizeof(recall_type), &k, sizeof(UShort));
}
void recall_cdefense_or(Short index, UShort or_me)
{
  Short offset = 2*sizeof(ULong) + 2*sizeof(UShort);
  or_me |= c_recall[index].r_cdefense;
  DmWrite(c_recall, offset+index*sizeof(recall_type), &or_me, sizeof(UShort));
}
void recall_set_wake(Short index, UChar val)
{
  Short offset = 2*sizeof(ULong) + 3*sizeof(UShort);
  DmWrite(c_recall, offset+index*sizeof(recall_type), &val, sizeof(UChar));
}
void recall_set_ignore(Short index, UChar val)
{
  Short offset = 2*sizeof(ULong) + 3*sizeof(UShort) + sizeof(UChar);
  DmWrite(c_recall, offset+index*sizeof(recall_type), &val, sizeof(UChar));
}
void recall_increment_attacks(Short index, Short attindex)
{
  Short offset = 2*sizeof(ULong) + 3*sizeof(UShort) + 2*sizeof(UChar);
  UShort k = c_recall[index].r_attacks[attindex] + 1;
  DmWrite(c_recall, offset+index*sizeof(recall_type), &k, sizeof(UChar));
}


/**********************************************************************
 *                                                                    *
 *  "names of stuff" ...                                              *
 *                                                                    *
 **********************************************************************/
Char **amulets;
Char **rocks;
Char **woods;
Char **metals;
Char **titles;
Char **colors;
Char **mushrooms;
Char **special_names;

// This is where magic_init will go
// It needs to be called again when you die?

/**********************************************************************
 *                                                                    *
 *         "the stores" ... stuff that was store_type                 *
 *                                                                    *
 **********************************************************************/

// There are STORE_MAX of each of these.
// The store inventories can have 26 items apiece
//owner_type *owners; // read-only
store_type *store;  // write
inven_record *store_inven[MAX_STORES]; // write

// Create the cave storage record (will delete it if it already exists)
static void create_storeinv_rec()
{
  ULong uniqueID;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  ULong size = STORE_INVEN_MAX * sizeof(inven_record);
  // Delete existing storeinv records
  for (uniqueID = REC_STOREINV_A ; uniqueID <= REC_STOREINV_Z ; uniqueID++) {
    if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
      DmRemoveRecord(MoriaSaveDB, index);      // EX TERM IN ATE !
    
    index = DmNumRecords(MoriaSaveDB);
    vh = DmNewRecord(MoriaSaveDB, &index, size);
    p = MemHandleLock(vh);
    if (p)
      DmSet(p, 0, size, 0);
    MemPtrUnlock(p);
    DmReleaseRecord(MoriaSaveDB, index, true);
    DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  }
  return;
}

// Create the record (will delete it if it already exists)
static void create_store_rec()
{
  ULong uniqueID;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  ULong size = sizeof(store_type) * MAX_STORES;
  // Delete existing record
  uniqueID = REC_STORES;
  if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
    DmRemoveRecord(MoriaSaveDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaSaveDB);
  vh = DmNewRecord(MoriaSaveDB, &index, size);
  p = MemHandleLock(vh);
  if (p)
    DmSet(p, 0, size, 0);
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaSaveDB, index, true);
  DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  //  inven_ctr = 0;
  create_storeinv_rec();
  return;
}

// Lock store inventories.  called by store_record_lock
static void storeinv_record_lock(Boolean unlocked)
{
  ULong uniqueID;
  static UInt index[MAX_STORES];
  static VoidHand vh[MAX_STORES];
  VoidPtr p;
  Short i = 0;
  if (unlocked) {
    // lock record
    for (uniqueID = REC_STOREINV_A, i = 0 ; i < MAX_STORES ; i++, uniqueID++) {
      if (0!=DmFindRecordByID(MoriaSaveDB, uniqueID, index+i)) return;
      vh[i] = DmGetRecord(MoriaSaveDB, index[i]);
      p = MemHandleLock(vh[i]);
      store_inven[i] = (inven_record *) p;
    }
  } else {
    // unlock record
    // I get a fatal error.  SIGH.
    for (i = 0 ; i < MAX_STORES ; i++) {
      if (store_inven[i] != NULL) {
	store_inven[i] = NULL;
	MemHandleUnlock(vh[i]);
	DmReleaseRecord(MoriaSaveDB, index[i], true);
      }
    }
  }
}
static void store_record_lock(Boolean unlocked)
{
  ULong uniqueID;
  static UInt index;
  static VoidHand vh;
  VoidPtr p;
  if (unlocked) {
    // lock record
    uniqueID = REC_STORES;
    if (0!=DmFindRecordByID(MoriaSaveDB, uniqueID, &index)) return;
    vh = DmGetRecord(MoriaSaveDB, index);
    p = MemHandleLock(vh);
    store = (store_type *) p;
  } else {
    // unlock record
    if (store != NULL) {
      store = NULL;
      MemHandleUnlock(vh);
      DmReleaseRecord(MoriaSaveDB, index, true);
    }
  }
  storeinv_record_lock(unlocked);
}

/**********************************************************************
 *                                                                    *
 *         haggle strings                                             *
 *                                                                    *
 **********************************************************************/
Char *haggle_strings = NULL;
Short *haggle_offsets = NULL;
// another read-only
static void haggle_record_lock(Boolean unlocked)
{
  ULong uniqueID = REC_HAGGLE;
  UInt index;
  static VoidHand vh, vh2;
  VoidPtr p = NULL;
  if (unlocked) {
    // lock record
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh = DmQueryRecord(MoriaDB, index);
    p = MemHandleLock(vh);
    haggle_offsets = (Short *) p;
    uniqueID = REC_HAGGLE_TEXT;
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh2 = DmQueryRecord(MoriaDB, index);
    p = MemHandleLock(vh2);
    haggle_strings = (Char *) p;
  } else {
    // unlock record
    if (haggle_offsets != NULL) {
      haggle_offsets = NULL;
      MemHandleUnlock(vh);
    }
    if (haggle_strings != NULL) {
      haggle_strings = NULL;
      MemHandleUnlock(vh2);
    }
  }
}
#define HAGGLE_QTY 76
Char *get_haggle_string(Short i)
{
  Short j;
  Char *str;
  if (i < 0 || i >= HAGGLE_QTY) return NULL;
  j = haggle_offsets[i];
  str = haggle_strings + j;
  return str;
}














/*
// just once to bootstrap
static void create_owner_rec()
{
  ULong uniqueID;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  ULong size = sizeof(owner_type) * MAX_STORES;
  Short i;
  uniqueID = REC_OWNERS;
  if (0 == DmFindRecordByID(MoriaDB, uniqueID, &index))
    DmRemoveRecord(MoriaDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaDB);
  vh = DmNewRecord(MoriaDB, &index, size);
  p = MemHandleLock(vh);
  if (p)
    for (i = 0 ; i < MAX_STORES ; i++)
      DmWrite(p, i*sizeof(owner_type), &owners_tmp[i],sizeof(owner_type));
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaDB, index, true);
  DmSetRecordInfo(MoriaDB, index, NULL, &uniqueID); // set its uid!
  return;
}
static void owner_record_lock(Boolean unlocked)
{
  ULong uniqueID;
  static UInt index;
  static VoidHand vh;
  VoidPtr p;
  if (unlocked) {
    // create record...
    create_owner_rec();
    // lock record
    uniqueID = REC_OWNERS;
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh = DmGetRecord(MoriaDB, index);
    p = MemHandleLock(vh);
    owners = (owner_type *) p;
  } else {
    // unlock record
    if (owners != NULL) {
      owners = NULL;
      MemHandleUnlock(vh);
      DmReleaseRecord(MoriaDB, index, true);
    }
  }
}
*/

