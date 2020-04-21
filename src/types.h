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

/* source/types.h: global type declarations */

// In the PalmOS, 8 bits are Char, 16 bits are Short/Int, 32 bits are Long

//typedef unsigned long  Longu;
//typedef long	       Long;
//typedef unsigned short int16u;
//typedef short	       int16;
//typedef unsigned char  int8u;
/* some machines will not accept 'signed char' as a type, and some accept it
   but still treat it like an unsigned character, let's just avoid it,
   any variable which can ever hold a negative value must be 16 or 32 bits */

// Like, the palm os can DEAL... char = Char = SByte, == signed byte
// so I need to go through and change ALL unneccesary Shorts to Chars.

//#define VTYPESIZ	80
//#define BIGVTYPESIZ	160
//typedef char vtype[VTYPESIZ];
/* note that since its output can easily exceed 80 characters, objdes must
   always be called with a bigvtype as the first paramter */
//typedef char bigvtype[BIGVTYPESIZ];
//typedef char stat_type[7]; // this is not really used

/* Many of the character fields used to be fixed length, which greatly
   increased the size of the executable.  I have replaced many fixed
   length fields with variable length ones. */
// Sadly, I will have to change them back to fixed length
// if I am going to put them in database records. (or suffer pain.)
// Make sure that alignment is OK after I do this!

/* all fields are given the smallest possible type, and all fields are
   aligned within the structure to their natural size boundary, so that
   the structures contain no padding and are minimum size */

/* bit fields are only used where they would cause a large reduction in
   data size, they should not be used otherwise because their use
   results in larger and slower code */

typedef struct creature_type
{
  //  Char  *name;		/* Descrip of creature	*/ // 26 chars max
  Short name_offset;
  ULong  cmove;		/* Bit field		*/ // bit field
  ULong  spells; 	/* Creature spells	*/ // bit field
  UShort cdefense;	/* Bit field		*/ // bit field
  UShort mexp;		/* Exp value for kill	*/
  UChar  sleep;		/* Inactive counter/10	*/
  UChar  aaf;		/* Area affect radius	*/
  UChar  ac;		/* AC			*/
  UChar  speed;		/* Movement speed+10	*/
  UChar  cchar;		/* Character rep.	*/
  UChar  hd[2];		/* Creatures hit die	*/
  UChar  damage[4];	/* Type attack and damage*/
  UChar  level;		/* Level of creature	*/
} creature_type;

typedef struct m_attack_type	/* Monster attack and damage types */
{
  UChar attack_type;
  UChar attack_desc;
  UChar attack_dice;
  UChar attack_sides;
} m_attack_type;

typedef struct recall_type	/* Monster memories. -CJS- */
{
  ULong  r_cmove;    // bitfield
  ULong  r_spells;   // bitfield?  counter?  freaky
  UShort r_kills;    // counter
  UShort r_deaths;   // counter
  UShort r_cdefense; // bitfield
  UChar  r_wake;     // counter?
  UChar  r_ignore;   // counter?
  UChar  r_attacks[MAX_MON_NATTACK]; // 4 counters
} recall_type;

typedef struct monster_type
{
  Short hp;		/* Hit points		*/
  Short csleep;		/* Inactive counter	*/
  Short cspeed;		/* Movement speed	*/
  UShort mptr;		/* Pointer into creature*/
  /* NOTE: fy, fx, and cdis constrain dungeon size to less than 256 by 256 */
  UChar fy;		/* Y Pointer into map	*/
  UChar fx;		/* X Pointer into map	*/
  UChar cdis;		/* Cur dis from player	*/
  Boolean ml;           /* monster light - was UChar */
  UChar stunned;        /* counter */
  UChar confused;       /* counter */
} monster_type;

typedef struct treasure_type
{
  //  Char  name[32];	/* Object name		*/ // SIGH.
  Short name_offset;
  ULong flags;		/* Special flags	*/
  UChar tval;		/* Category number	*/
  UChar tchar;		/* Character representation*/
  Short p1;		/* Misc. use variable	*/ // must be at least a short
  Long cost;		/* Cost of item		*/
  UChar subval;		/* Sub-category number	*/
  UChar number;		/* Number of items	*/
  UShort weight;	/* Weight		*/
  Char tohit;		/* Plusses to hit	*/ // was a Short
  Char todam;		/* Plusses to damage	*/ // was a Short
  Char ac;		/* Normal AC		*/ // was a Short
  Char toac;		/* Plusses to AC	*/ // was a Short
  UChar damage[2];	/* Damage when hits	*/
  UChar level;		/* Level item first found */
} treasure_type;

/* only damage, ac, and tchar are constant; level could possibly be made
   constant by changing index instead; all are used rarely */
/* extra fields x and y for location in dungeon would simplify pusht() */
/* making inscrip a pointer and mallocing space does not work, there are
   too many places where inven_types are copied, which results in dangling
   pointers, so we use a char array for them instead */
#define INSCRIP_SIZE 13  /* notice alignment, must be 4*x + 1 */
typedef struct inven_type
{
  UShort index;		/* Index to object_list */
  UChar name2;		/* Object special name  */
  Char inscrip[INSCRIP_SIZE]; /* Object inscription   */
  ULong flags;		/* Special flags	*/
  UChar tval;		/* Category number	*/
  UChar tchar;		/* Character representation*/
  Short p1;		/* Misc. use variable	*/ // must be at least a short
  Long cost;		/* Cost of item		*/
  UChar subval;		/* Sub-category number	*/
  UChar number;		/* Number of items	*/
  UShort weight;	/* Weight		*/
  Char tohit;		/* Plusses to hit	*/ // was a Short
  Char todam;		/* Plusses to damage	*/ // was a Short
  Char ac;		/* Normal AC		*/ // was a Short
  Char toac;		/* Plusses to AC	*/ // was a Short
  UChar damage[2];	/* Damage when hits	*/
  UChar level;		/* Level item first found */
  UChar ident;		/* Identify information */
} inven_type;

#define PLAYER_NAME_SIZE 27

// I might be revising this.
// total size:
// bool   18           18
// char   27  5 18  1  51
// short  27  6 24    114
// long    3  1        16
//                    199 B
typedef struct player_type
{
  struct misc
  {
    // char  = PLAYER_NAME_SIZE + 5
    // short = 27 .. how many could be chars?  too lazy to find out.
    // long  = 3
    Char name[PLAYER_NAME_SIZE]; /* Name of character	*/ // 27; align ok
    Boolean male;	/* Sex of character	*/
    Long au;		/* Gold			*/
    Long max_exp;	/* Max experience	*/
    Long exp;	/* Cur experience	*/
    UShort exp_frac;	/* Cur exp fraction * 2^16 */
    UShort age;	/* Characters age	*/
    UShort ht;	/* Height		*/
    UShort wt;	/* Weight		*/
    UShort lev;	/* Level		*/
    UShort max_dlv;	/* Max level explored	*/
    Short srh;	/* Chance in search	*/
    Short fos;	/* Frenq of search	*/
    Short bth;	/* Base to hit		*/
    Short bthb;	/* BTH with bows	*/
    Short mana;	/* Mana points		*/
    Short mhp;	/* Max hit pts		*/
    Short ptohit;	/* Plusses to hit	*/
    Short ptodam;	/* Plusses to dam	*/
    Short pac;	/* Total AC		*/
    Short ptoac;	/* Magical AC		*/
    Short dis_th;	/* Display +ToHit	*/
    Short dis_td;	/* Display +ToDam	*/
    Short dis_ac;	/* Display +ToAC	*/
    Short dis_tac;	/* Display +ToTAC	*/
    Short disarm;	/* % to Disarm		*/
    Short save;	/* Saving throw		*/
    Short sc;		/* Social Class		*/
    Short stl;	/* Stealth factor	*/
    UChar pclass;	/* # of class		*/
    UChar prace;	/* # of race		*/
    UChar hitdie;	/* Char hit die		*/
    UChar expfact;	/* Experience factor	*/
    Short cmana;	/* Cur mana pts		*/
    UShort cmana_frac; /* Cur mana fraction * 2^16 */
    Short chp;	/* Cur hit pts		*/
    UShort chp_frac;	/* Cur hit fraction * 2^16 */
    //    Char history[4][60]; /* History record	*/ // UNNEEDED.
  } misc;
  /* Stats now kept in arrays, for more efficient access. -CJS- */
  struct stats
  {
    // 18 char, 6 short (which could be signed char.)
    UChar max_stat[6];	/* What is restored */
    UChar cur_stat[6];	/* What is natural */
    Short mod_stat[6];	/* What is modified, may be +/- */
    UChar use_stat[6];	/* What is used */
  } stats;
  struct flags
  {
    // long = 1
    // short = 24
    // char = 1
    // bool = 18
    ULong status;		/* Status of player    */
    Short rest;		/* Rest counter	       */
    Short blind;		/* Blindness counter   */
    Short paralysis;		/* Paralysis counter   */
    Short confused;		/* Confusion counter   */
    Short food;		/* Food counter	       */
    Short food_digested;	/* Food per round      */
    Short protection;		/* Protection fr. evil */
    Short speed;		/* Cur speed adjust    */
    Short fast;		/* Temp speed change   */
    Short slow;		/* Temp speed change   */
    Short afraid;		/* Fear		       */
    Short poisoned;		/* Poisoned	       */
    Short image;		/* Hallucinate	       */
    Short protevil;		/* Protect VS evil     */
    Short invuln;		/* Increases AC	       */
    Short hero;		/* Heroism	       */
    Short shero;		/* Super Heroism       */
    Short blessed;		/* Blessed	       */
    Short resist_heat;	/* Timed heat resist   */
    Short resist_cold;	/* Timed cold resist   */
    Short detect_inv;		/* Timed see invisible */
    Short word_recall;	/* Timed teleport level*/
    Short see_infra;		/* See warm creatures  */
    Short tim_infra;		/* Timed infra vision  */
    Boolean see_inv;		/* Can see invisible   */
    Boolean teleport;		/* Random teleportation*/
    Boolean free_act;		/* Never paralyzed     */
    Boolean slow_digest;	/* Lower food needs    */
    Boolean aggravate;		/* Aggravate monsters  */
    Boolean fire_resist;	/* Resistance to fire  */
    Boolean cold_resist;	/* Resistance to cold  */
    Boolean acid_resist;	/* Resistance to acid  */
    Boolean regenerate;		/* Regenerate hit pts  */
    Boolean lght_resist;	/* Resistance to light */
    Boolean ffall;		/* No damage falling   */
    Boolean sustain_str;	/* Keep strength       */
    Boolean sustain_int;	/* Keep intelligence   */
    Boolean sustain_wis;	/* Keep wisdom	       */
    Boolean sustain_con;	/* Keep constitution   */
    Boolean sustain_dex;	/* Keep dexterity      */
    Boolean sustain_chr;	/* Keep charisma       */
    Boolean confuse_monster;	/* Glowing hands.    */
    UChar new_spells;		/* Number of spells can learn. */
  } flags;
} player_type;

typedef struct spell_type
{  /* spell name is stored in spell_names[] array at index i, +31 if priest */
  UChar slevel;
  UChar smana;
  UChar sfail;
  UChar sexp;	/* 1/4 of exp gained for learning spell */
} spell_type;

typedef struct race_type
{
  Char	trace[12];	/* Type of race			*/ // Max 11
  Char str_adj;	        /* adjustments			*/
  Char int_adj;  // All the Chars here (except trace) were Shorts.
  Char wis_adj;
  Char dex_adj;
  Char con_adj;
  Char chr_adj;
  UChar b_age;	       /* Base age of character		*/
  UChar m_age;	       /* Maximum age of character	*/
  UChar m_b_ht;	      /* base height for males		*/
  UChar m_m_ht;	      /* mod height for males		*/
  UChar m_b_wt;	      /* base weight for males		*/
  UChar m_m_wt;	      /* mod weight for males		*/
  UChar f_b_ht;	      /* base height females		*/
  UChar f_m_ht;	      /* mod height for females	*/
  UChar f_b_wt;	      /* base weight for female	*/
  UChar f_m_wt;	      /* mod weight for females	*/
  Char b_dis;	       /* base chance to disarm		*/
  Char srh;	       /* base chance for search	*/
  Char stl;	       /* Stealth of character		*/
  Char fos;	       /* frequency of auto search	*/
  Char bth;	       /* adj base chance to hit	*/
  Char bthb;	       /* adj base to hit with bows	*/
  Char bsav;	       /* Race base for saving throw	*/
  UChar bhitdie;       /* Base hit points for race	*/
  UChar infra;	       /* See infra-red			*/
  UChar b_exp;	       /* Base experience factor	*/
  UChar rtclass;       /* Bit field for class types	*/
} race_type;

typedef struct class_type
{
  Char  title[8];	/* type of class	*/ //need 8; alignment ok?
  UChar adj_hd;		/* Adjust hit points		*/
  UChar mdis;		/* mod disarming traps		*/
  UChar msrh;		/* modifier to searching	*/
  UChar mstl;		/* modifier to stealth		*/
  UChar mfos;		/* modifier to freq-of-search	*/
  UChar mbth;		/* modifier to base to hit	*/
  UChar mbthb;		/* modifier to base to hit - bows*/
  UChar msav;		/* Class modifier to save	*/
  Short madj_str;	/* Class modifier for strength	*/
  Short madj_int;	/* Class modifier for intelligence*/
  Short madj_wis;	/* Class modifier for wisdom	*/
  Short madj_dex;	/* Class modifier for dexterity */
  Short madj_con;	/* Class modifier for constitution*/
  Short madj_chr;	/* Class modifier for charisma	*/
  UChar spell;		/* class use mage spells	*/
  UChar m_exp;		/* Class experience factor	*/
  UChar first_spell_lev;/* First level where class can use spells. */
} class_type;

// this is just used at character creation, really.
typedef struct background_type
{
  Char *info;		/* History information		*/ // Max 51!
  UChar roll;		/* Die roll needed for history	*/
  UChar chart;		/* Table number			*/
  UChar next;		/* Pointer to next table	*/
  UChar bonus;		/* Bonus to the Social Class+50	*/
} background_type;

// There are lots and lots of these.  (lots+lots) * 5 bytes?  4 bytes?
// anyway, it ought to be a database record if I can manage that.
#define LIGHT_lr 1
#define LIGHT_fm 2
#define LIGHT_pl 4
#define LIGHT_tl 8
#define CAVE_cptr 0
#define CAVE_tptr 1
#define CAVE_fval 2
#define CAVE_light 3
typedef struct cave_type
{
  UChar cptr; // index into creatures-on-this-level array 'm_list'
  UChar tptr; // index into treasures-on-this-level array 't_list'
  UChar fval; // floor type
  UChar light; // lr, fm, pl, tl
  // Ok, sdk-include files use ':' (with Word,==UInt) so this should work.
  //  UInt lr : 1;  /* room should be lit with perm light, walls with
  //			   this set should be perm lit after tunneled out */
  //  UInt fm : 1;	/* field mark, used for traps/doors/stairs, object is
  //			   hidden if fm is FALSE */
  //  UInt pl : 1;	/* permanent light, used for walls and lighted rooms */
  //  UInt tl : 1;	/* temporary light, used for player's lamp light,etc.*/
} cave_type;

typedef struct owner_type
{
  Char owner_name[36];  //  Char *owner_name; 22 if no race included
  Char store_name[14];
  Short max_cost;
  UChar max_inflate;
  UChar min_inflate;
  UChar haggle_per;
  UChar owner_race;
  UChar insult_max;
} owner_type;

typedef struct inven_record
{
  Long scost;
  inven_type sitem;
} inven_record;

typedef struct store_type
{
  Long store_open;
  Short insult_cur;
  UChar owner;
  UChar store_ctr;
  UShort good_buy;
  UShort bad_buy;
  //  inven_record store_inven[STORE_INVEN_MAX]; // 26
} store_type;

/* 64 bytes for this structure */
typedef struct high_scores
{
  Long points;
  Long birth_date;
  Short uid;
  Short mhp;
  Short chp;
  UChar dun_level;
  UChar lev;
  UChar max_dlv;
  UChar sex; // could be Boolean
  UChar race;
  UChar class;
  Char name[PLAYER_NAME_SIZE];
  Char died_from[25];
} high_scores;

/* Game preferences.  Saved between sessions. */
#define NAMELEN 20
struct MoriaPreferenceType_s {
  /* ... */
  Short run_walk_border;
  Short walk_center_border;
  // I may eliminate some of these.
  Boolean big_font;
  Boolean relative_move;
  Boolean overlay_circles;
  Boolean sound;
  Boolean run;
  Boolean run_cut_corners;
  Boolean run_examine_corners;
  Boolean run_stop_sector;
  Boolean run_thru_doors;
  Boolean prompt_pickup;
  Boolean show_weights;
  Boolean highlight_minerals;
  // Hardware buttons
  Boolean use_hardware;
  Short hardware[8];
  // Default character name
  Char name[NAMELEN];
};
typedef struct MoriaPreferenceType_s MoriaPreferenceType;
