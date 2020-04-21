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
#define REC_MSGLOG      100
#define REC_PLAYERTITLE 101
#define REC_BG          102
#define REC_BG_STR      103
#define REC_PLAYEREXP   104
#define REC_PLAYERHP    105
#define REC_RGOLDADJ    106
#define REC_SPELLNAME   107
#define REC_SPELLORDER  108
#define REC_STOREOWN    109

#define REC_OBJECTLIST    150
#define REC_OBJLIST_NAMES 151
#define REC_CREATURELIST  160
#define REC_CLIST_NAMES   161

#define REC_STORES      170
#define REC_STOREINV_A  171
// 171, 172, 173, 174, 175, 176: these are store inventories, 26 structs each
#define REC_STOREINV_Z  176
#define REC_OWNERS      177
#define REC_HAGGLE      180
#define REC_HAGGLE_TEXT 181

#define REC_CAVE_CPTR   200
#define REC_CAVE_TPTR   201
#define REC_CAVE_FVAL   202
#define REC_CAVE_LIGHT  203
#define REC_TLIST       204
#define REC_MLIST       205
#define REC_TLEVEL      206
#define REC_SORTEDOBJ   207
#define REC_MLEVEL      208
#define REC_INVENTORY   209
#define REC_OBJIDENT    210
// Note: the recall record should *NOT* be erased when a character dies!!!
#define REC_RECALL      211



#define MAXLEN_MSG 80
#define MAX_MSGLOG 10
#define MAXLEN_PLAYERTITLE 14
#define MAXLEN_SPELLNAME 24
#define MAX_SPELLNAME 62
#define MAX_SPELLORDER 32
#define NO_SPELL 99
#define MAXLEN_STOREOWNERNAME 22
#define MAXLEN_SPECNAME 19
#define MAXLEN_COLORS 16
#define MAXLEN_MUSH 14
#define MAXLEN_WOOD 11
#define MAXLEN_METALS 16
#define MAXLEN_ROCKS 13
#define MAXLEN_AMULETS 15
#define MAXLEN_SYLLABLES 5



// These could be used for offsets in the 'recall' record
#define R_CMOVE    0
#define R_SPELLS   (sizeof(ULong))
#define R_KILLS    (2*sizeof(ULong))
#define R_DEATHS   (R_KILLS + sizeof(UShort))
#define R_CDEFENSE (R_KILLS + 2*sizeof(UShort))
#define R_WAKE     (R_KILLS + 3*sizeof(UShort))
#define R_IGNORE   (R_WAKE + sizeof(UChar))
#define R_ATTACKS  (R_WAKE + 2*sizeof(UChar))

// These could be used for offsets in the 'm_list' record
#define MLIST_hp 0
#define MLIST_csleep sizeof(Short)
#define MLIST_cspeed 2*sizeof(Short)
#define MLIST_fy        4*sizeof(Short)
#define MLIST_fx       (4*sizeof(Short) +   sizeof(Char))
#define MLIST_cdis     (4*sizeof(Short) + 2*sizeof(Char))
#define MLIST_ml       (4*sizeof(Short) + 3*sizeof(Char))
#define MLIST_stunned  (4*sizeof(Short) + 3*sizeof(Char) + sizeof(Boolean))
#define MLIST_confused (4*sizeof(Short) + 4*sizeof(Char) + sizeof(Boolean))

// These could be used for offsets in an 'inven_type' record
#define I_index    0
#define I_name2    sizeof(Short)
#define I_inscrip (sizeof(Short) + sizeof(Char))
#define I_flags   (sizeof(Short) + (1+INSCRIP_SIZE)*sizeof(Char))
#define I_tval  (sizeof(Long) +sizeof(Short) +(1+INSCRIP_SIZE)*sizeof(Char))
#define I_tchar (sizeof(Long) +sizeof(Short) +(2+INSCRIP_SIZE)*sizeof(Char))
#define I_p1    (sizeof(Long) +sizeof(Short) +(3+INSCRIP_SIZE)*sizeof(Char))
#define I_cost  (sizeof(Long)+2*sizeof(Short)+(3+INSCRIP_SIZE)*sizeof(Char))
#define I_subval (2*sizeof(Long)+2*sizeof(Short)+(3+INSCRIP_SIZE)*sizeof(Char))
#define I_number (2*sizeof(Long)+2*sizeof(Short)+(4+INSCRIP_SIZE)*sizeof(Char))
#define I_weight (2*sizeof(Long)+2*sizeof(Short)+(5+INSCRIP_SIZE)*sizeof(Char))
#define I_tohit (2*sizeof(Long)+3*sizeof(Short)+(5+INSCRIP_SIZE)*sizeof(Char))
#define I_todam (2*sizeof(Long)+3*sizeof(Short)+(6+INSCRIP_SIZE)*sizeof(Char))
#define I_ac    (2*sizeof(Long)+3*sizeof(Short)+(7+INSCRIP_SIZE)*sizeof(Char))
#define I_toac  (2*sizeof(Long)+3*sizeof(Short)+(8+INSCRIP_SIZE)*sizeof(Char))
#define I_toac  (2*sizeof(Long)+3*sizeof(Short)+(8+INSCRIP_SIZE)*sizeof(Char))
#define I_damage (2*sizeof(Long)+3*sizeof(Short)+(9+INSCRIP_SIZE)*sizeof(Char))
#define I_level (2*sizeof(Long)+3*sizeof(Short)+(11+INSCRIP_SIZE)*sizeof(Char))
#define I_ident (2*sizeof(Long)+3*sizeof(Short)+(12+INSCRIP_SIZE)*sizeof(Char))

