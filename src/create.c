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
#include "lock-externs.h"
//#include "kMoriaRsc.h"


Short player_hp[MAX_PLAYER_LEVEL];

race_type race[MAX_RACES] = {
  {"Human",	 0,  0,	 0,  0,	 0,  0,
   14,  6, 72,  6,180, 25, 66,  4,150, 20,
   0,  0,  0,  0,  0,  0,  0, 10,  0, 100, 0x3F,
  },
  {"Half-Elf", -1,  1,	 0,  1, -1,  1,
   24, 16, 66,  6,130, 15, 62,  6,100, 10,
   2,  6,  1, -1, -1,  5,  3,  9,  2, 110, 0x3F,
  },
  {"Elf",	-1,  2,	 1,  1, -2,  1,
   75, 75, 60,  4,100,  6, 54,  4, 80,  6,
   5,  8,  1, -2, -5, 15,  6,  8,  3, 120, 0x1F,
  },
  {"Halfling", -2,  2,	 1,  3,	 1,  1,
   21, 12, 36,  3, 60,  3, 33,  3, 50,  3,
   15, 12,  4, -5,-10, 20, 18,  6,  4, 110, 0x0B,
  },
  {"Gnome",	-1,  2,	 0,  2,	 1, -2,
   50, 40, 42,  3, 90,  6, 39,  3, 75,  3,
   10,  6,  3, -3, -8, 12, 12,  7,  4, 125, 0x0F,
  },
  {"Dwarf",	 2, -3,	 1, -2,	 2, -3,
   35, 15, 48,  3,150, 10, 46,  3,120, 10,
   2,  7,  -1,  0, 15,  0,  9,  9,  5, 120, 0x05,
  },
  {"Half-Orc",	 2, -1,	 0,  0,	 1, -4,
   11,  4, 66,  1,150,  5, 62,  1,120,  5,
   -3,  0, -1,  3, 12, -5, -3, 10,  3, 110, 0x0D,
  },
  {"Half-Troll",4, -4, -2, -4,	 3, -6,
   20, 10, 96, 10,255, 50, 84,  8,225, 40,
   -5, -1, -2,  5, 20,-10, -8, 12,  3, 120, 0x05,
  }
};


background_type background[MAX_BACKGROUND] = {
{"You are the illegitimate and unacknowledged child ",		 10, 1, 2, 25},
{"You are the illegitimate but acknowledged child ",		 20, 1, 2, 35},
{"You are one of several children ",				 95, 1, 2, 45},
{"You are the first child ",					100, 1, 2, 50},
{"of a Serf.  ",						 40, 2, 3, 65},
{"of a Yeoman.  ",						 65, 2, 3, 80},
{"of a Townsman.  ",						 80, 2, 3, 90},
{"of a Guildsman.  ",						 90, 2, 3,105},
{"of a Landed Knight.  ",					 96, 2, 3,120},
{"of a Titled Noble.  ",					 99, 2, 3,130},
{"of a Royal Blood Line.  ",					100, 2, 3,140},
{"You are the black sheep of the family.  ",			 20, 3,50, 20},
{"You are a credit to the family.  ",				 80, 3,50, 55},
{"You are a well liked child.  ",				100, 3,50, 60},
{"Your mother was a Green-Elf.  ",				 40, 4, 1, 50},
{"Your father was a Green-Elf.  ",				 75, 4, 1, 55},
{"Your mother was a Grey-Elf.  ",				 90, 4, 1, 55},
{"Your father was a Grey-Elf.  ",				 95, 4, 1, 60},
{"Your mother was a High-Elf.  ",				 98, 4, 1, 65},
{"Your father was a High-Elf.  ",				100, 4, 1, 70},
{"You are one of several children ",				 60, 7, 8, 50},
{"You are the only child ",					100, 7, 8, 55},
{"of a Green-Elf ",						 75, 8, 9, 50},
{"of a Grey-Elf ",						 95, 8, 9, 55},
{"of a High-Elf ",						100, 8, 9, 60},
{"Ranger.  ",							 40, 9,54, 80},
{"Archer.  ",							 70, 9,54, 90},
{"Warrior.  ",							 87, 9,54,110},
{"Mage.  ",							 95, 9,54,125},
{"Prince.  ",							 99, 9,54,140},
{"King.  ",							100, 9,54,145},
{"You are one of several children of a Halfling ",		 85,10,11, 45},
{"You are the only child of a Halfling ",			100,10,11, 55},
{"Bum.  ",							 20,11, 3, 55},
{"Tavern Owner.  ",						 30,11, 3, 80},
{"Miller.  ",							 40,11, 3, 90},
{"Home Owner.  ",						 50,11, 3,100},
{"Burglar.  ",							 80,11, 3,110},
{"Warrior.  ",							 95,11, 3,115},
{"Mage.  ",							 99,11, 3,125},
{"Clan Elder.  ",						100,11, 3,140},
{"You are one of several children of a Gnome ",			 85,13,14, 45},
{"You are the only child of a Gnome ",				100,13,14, 55},
{"Beggar.  ",							 20,14, 3, 55},
{"Braggart.  ",							 50,14, 3, 70},
{"Prankster.  ",						 75,14, 3, 85},
{"Warrior.  ",							 95,14, 3,100},
{"Mage.  ",							100,14, 3,125},
{"You are one of two children of a Dwarven ",			 25,16,17, 40},
{"You are the only child of a Dwarven ",			100,16,17, 50},
{"Thief.  ",							 10,17,18, 60},
{"Prison Guard.  ",						 25,17,18, 75},
{"Miner.  ",							 75,17,18, 90},
{"Warrior.  ",							 90,17,18,110},
{"Priest.  ",							 99,17,18,130},
{"King.  ",							100,17,18,150},
{"You are the black sheep of the family.  ",			 15,18,57, 10},
{"You are a credit to the family.  ",				 85,18,57, 50},
{"You are a well liked child.  ",				100,18,57, 55},
{"Your mother was an Orc, but it is unacknowledged.  ",		 25,19,20, 25},
{"Your father was an Orc, but it is unacknowledged.  ",		100,19,20, 25},
{"You are the adopted child ",					100,20, 2, 50},
{"Your mother was a Cave-Troll ",				 30,22,23, 20},
{"Your father was a Cave-Troll ",				 60,22,23, 25},
{"Your mother was a Hill-Troll ",				 75,22,23, 30},
{"Your father was a Hill-Troll ",				 90,22,23, 35},
{"Your mother was a Water-Troll ",				 95,22,23, 40},
{"Your father was a Water-Troll ",				100,22,23, 45},
{"Cook.  ",							  5,23,62, 60},
{"Warrior.  ",							 95,23,62, 55},
{"Shaman.  ",							 99,23,62, 65},
{"Clan Chief.  ",						100,23,62, 80},
{"You have dark brown eyes, ",					 20,50,51, 50},
{"You have brown eyes, ",					 60,50,51, 50},
{"You have hazel eyes, ",					 70,50,51, 50},
{"You have green eyes, ",					 80,50,51, 50},
{"You have blue eyes, ",					 90,50,51, 50},
{"You have blue-gray eyes, ",					100,50,51, 50},
{"straight ",							 70,51,52, 50},
{"wavy ",							 90,51,52, 50},
{"curly ",							100,51,52, 50},
{"black hair, ",						 30,52,53, 50},
{"brown hair, ",						 70,52,53, 50},
{"auburn hair, ",						 80,52,53, 50},
{"red hair, ",							 90,52,53, 50},
{"blond hair, ",						100,52,53, 50},
{"and a very dark complexion.",					 10,53, 0, 50},
{"and a dark complexion.",					 30,53, 0, 50},
{"and an average complexion.",					 80,53, 0, 50},
{"and a fair complexion.",					 90,53, 0, 50},
{"and a very fair complexion.",					100,53, 0, 50},
{"You have light grey eyes, ",					 85,54,55, 50},
{"You have light blue eyes, ",					 95,54,55, 50},
{"You have light green eyes, ",					100,54,55, 50},
{"straight ",							 75,55,56, 50},
{"wavy ",							100,55,56, 50},
{"black hair, and a fair complexion.",				 75,56, 0, 50},
{"brown hair, and a fair complexion.",				 85,56, 0, 50},
{"blond hair, and a fair complexion.",				 95,56, 0, 50},
{"silver hair, and a fair complexion.",				100,56, 0, 50},
{"You have dark brown eyes, ",					 99,57,58, 50},
{"You have glowing red eyes, ",					100,57,58, 60},
{"straight ",							 90,58,59, 50},
{"wavy ",							100,58,59, 50},
{"black hair, ",						 75,59,60, 50},
{"brown hair, ",						100,59,60, 50},
{"a one foot beard, ",						 25,60,61, 50},
{"a two foot beard, ",						 60,60,61, 51},
{"a three foot beard, ",					 90,60,61, 53},
{"a four foot beard, ",						100,60,61, 55},
{"and a dark complexion.",					100,61, 0, 50},
{"You have slime green eyes, ",					 60,62,63, 50},
{"You have puke yellow eyes, ",					 85,62,63, 50},
{"You have blue-bloodshot eyes, ",				 99,62,63, 50},
{"You have glowing red eyes, ",					100,62,63, 55},
{"dirty ",							 33,63,64, 50},
{"mangy ",							 66,63,64, 50},
{"oily ",							100,63,64, 50},
{"sea-weed green hair, ",					 33,64,65, 50},
{"bright red hair, ",						 66,64,65, 50},
{"dark purple hair, ",						100,64,65, 50},
{"and green ",							 25,65,66, 50},
{"and blue ",							 50,65,66, 50},
{"and white ",							 75,65,66, 50},
{"and black ",							100,65,66, 50},
{"ulcerous skin.",						 33,66, 0, 50},
{"scabby skin.",						 66,66, 0, 50},
{"leprous skin.",						100,66, 0, 50}
};



/* Each type of character starts out with a few provisions.	*/
/* Note that the entries refer to elements of the object_list[] array*/
/* 344 = Food Ration, 365 = Wooden Torch, 123 = Cloak, 318 = Beginners-Majik,
   103 = Soft Leather Armor, 30 = Stiletto, 322 = Beginners Handbook */
UShort player_init[MAX_CLASS][5] = {
  { 344, 365, 123,  30, 103},	/* Warrior	 */
  { 344, 365, 123,  30, 318},	/* Mage		 */
  { 344, 365, 123,  30, 322},	/* Priest	 */
  { 344, 365, 123,  30, 318},	/* Rogue	 */
  { 344, 365, 123,  30, 318},	/* Ranger	 */
  { 344, 365, 123,  30, 322}	/* Paladin	 */
};


/* Generates character's stats				-JWT-	*/
static void get_stats()
{
  Short i, tot;
  Short dice[18];

  do {
    tot = 0;
    for (i = 0; i < 18; i++) {
      dice[i] = randint (3 + i % 3);  /* Roll 3,4,5 sided dice once each */
      tot += dice[i];
    }
  } while (tot <= 42 || tot >= 54);

  for (i = 0; i < 6; i++)
    pystats.max_stat[i] = 5 + dice[3*i] + dice[3*i+1] + dice[3*i+2];
}


/* Changes stats by given amount				-JWT-	*/
static void change_stat(Short stat, Short amount)
{
  Short i, tmp_stat;

  tmp_stat = pystats.max_stat[stat];
  if (amount < 0) {
    for (i = 0; i > amount; i--) {
      if (tmp_stat > 108)
	tmp_stat--;
      else if (tmp_stat > 88)
	tmp_stat += -randint(6) - 2;
      else if (tmp_stat > 18) {
	tmp_stat += -randint(15) - 5;
	if (tmp_stat < 18)
	  tmp_stat = 18;
      } else if (tmp_stat > 3)
	tmp_stat--;
    }
  } else {
    for (i = 0; i < amount; i++) {
      if (tmp_stat < 18)
	tmp_stat++;
      else if (tmp_stat < 88)
	tmp_stat += randint(15) + 5;
      else if (tmp_stat < 108)
	tmp_stat += randint(6) + 2;
      else if (tmp_stat < 118)
	tmp_stat++;
    }
  }
  pystats.max_stat[stat] = tmp_stat;
}


/* generate all stats and modify for race. needed in a separate module so
   looping of character selection would be allowed     -RGM- */
static void get_all_stats ()
{
  race_type *r_ptr;
  Short j;

  r_ptr = &race[pymisc.prace];
  get_stats ();
  change_stat (A_STR, r_ptr->str_adj);
  change_stat (A_INT, r_ptr->int_adj);
  change_stat (A_WIS, r_ptr->wis_adj);
  change_stat (A_DEX, r_ptr->dex_adj);
  change_stat (A_CON, r_ptr->con_adj);
  change_stat (A_CHR, r_ptr->chr_adj);
  pymisc.lev = 1;
  for (j = 0; j < 6; j++) {
    pystats.cur_stat[j] = pystats.max_stat[j];
    set_use_stat (j);
  }

  pymisc.srh    = r_ptr->srh;
  pymisc.bth    = r_ptr->bth;
  pymisc.bthb   = r_ptr->bthb;
  pymisc.fos    = r_ptr->fos;
  pymisc.stl    = r_ptr->stl;
  pymisc.save   = r_ptr->bsav;
  pymisc.hitdie = r_ptr->bhitdie;
  pymisc.ptodam = todam_adj();
  pymisc.ptohit = tohit_adj();
  pymisc.ptoac  = 0;
  pymisc.pac    = toac_adj();
  pymisc.expfact = r_ptr->b_exp;
  pyflags.see_infra = r_ptr->infra;
}


/* Allows player to select a race			-JWT-	*/
// static void choose_race() ---> replaced by a form!
//  pymisc.prace  = j;
//  race[pymisc.prace].trace;


/* Will print the history of a character			-JWT-	*/
static void print_history()
{
  /*
  register int i;

  put_buffer("Character Background", 14, 27);
  for (i = 0; i < 4; i++)
    prt(pymisc.history[i], i+15, 10);
  */
}


/* Get the racial history, determines social class	-RAK-	*/
/* Assumptions:	Each race has init history beginning at		*/
/*		(race-1)*3+1					*/
/*		All history parts are in ascending order	*/
static void get_history()
{
  Short hist_ptr, cur_ptr, test_roll;
  Boolean flag;
  //  Short start_pos, end_pos, cur_len, line_ctr, new_start;
  Short social_class;
  Char history_block[240];
  background_type *b_ptr;
  Short x, y, lines=0, plen;
  Word wwlen, maxwid=160;
  Char *p;
  // see: foo_class_bg in form_chargen.c
  x = 0; // COL_1;
  y = 91-11;// ROW_4-3-11;

  /* Get a block of history text				*/
  hist_ptr = pymisc.prace*3 + 1;
  history_block[0] = '\0';
  social_class = randint(4);
  cur_ptr = 0;

  do {
    flag = false;
    while (!flag) {
      if (background[cur_ptr].chart == hist_ptr) {
	test_roll = randint(100);
	while (test_roll > background[cur_ptr].roll)
	  cur_ptr++;
	b_ptr = &background[cur_ptr];
	StrCat(history_block, b_ptr->info);
	social_class += b_ptr->bonus - 50;
	if (hist_ptr > b_ptr->next)
	  cur_ptr = 0;
	hist_ptr = b_ptr->next;
	flag = true;
      } else
	cur_ptr++;
    } 
  } while (hist_ptr >= 1);
  
  p = history_block;
  plen = StrLen(p);
  while (lines < 6) {
    // All the News that Fits, We Print.
    // I will edit the history strings as necessary to make fitting more likely
    wwlen = FntWordWrap(p, maxwid);
    WinDrawChars(p, wwlen, x, y);
    if (wwlen >= StrLen(p)) break;
    p += wwlen;
    y += 11;
    lines++;
  }
  

  /* clear the previous history strings */
  //  for (hist_ptr = 0; hist_ptr < 4; hist_ptr++)
  //    pymisc.history[hist_ptr][0] = '\0';

  /* Process block of history text for pretty output	*/
  /*
  start_pos = 0;
  end_pos   = StrLen(history_block) - 1;
  line_ctr  = 0;
  flag = false;
  while (history_block[end_pos] == ' ')
    end_pos--;
  while (!flag) {
    while (history_block[start_pos] == ' ')
      start_pos++;
    cur_len = end_pos - start_pos + 1;
    if (cur_len > 60) {
      cur_len = 60;
      while (history_block[start_pos+cur_len-1] != ' ')
	cur_len--;
      new_start = start_pos + cur_len;
      while (history_block[start_pos+cur_len-1] == ' ')
	cur_len--;
    } else
      flag = true;
    Strncpy(pymisc.history[line_ctr], &history_block[start_pos],
		   cur_len);
    pymisc.history[line_ctr][cur_len] = '\0';
    line_ctr++;
    start_pos = new_start;
  }
  */

  /* Compute social class for player			*/
  if (social_class > 100)
    social_class = 100;
  else if (social_class < 1)
    social_class = 1;
  pymisc.sc = social_class;
}


/* Gets the character's sex				-JWT-	*/
//static void get_sex() ---> There is a form for this


/* Computes character's age, height, and weight		-JWT-	*/
static void get_ahw()
{
  Short i;

  i = pymisc.prace;
  pymisc.age = race[i].b_age + randint((Short)race[i].m_age);
  if (pymisc.male) {
    pymisc.ht = randnor((Short)race[i].m_b_ht, (Short)race[i].m_m_ht);
    pymisc.wt = randnor((Short)race[i].m_b_wt, (Short)race[i].m_m_wt);
  } else {
    pymisc.ht = randnor((Short)race[i].f_b_ht, (Short)race[i].f_m_ht);
    pymisc.wt = randnor((Short)race[i].f_b_wt, (Short)race[i].f_m_wt);
  }
  pymisc.disarm = race[i].b_dis + todis_adj();
}


/* Gets a character class				-JWT-	*/
static void get_class()
{
  Short i, min_value, max_value;
  //  Short cl[MAX_CLASS], exit_flag;
  //  register struct misc *m_ptr;
  //  register player_type *p_ptr;
  class_type *c_ptr;

  c_ptr = &class[pymisc.pclass];
  //  clear_from (20);
  //  put_buffer(c_ptr->title, 5, 15);

  /* Adjust the stats for the class adjustment		-RAK-	*/
  //  p_ptr = &py;
  change_stat (A_STR, c_ptr->madj_str);
  change_stat (A_INT, c_ptr->madj_int);
  change_stat (A_WIS, c_ptr->madj_wis);
  change_stat (A_DEX, c_ptr->madj_dex);
  change_stat (A_CON, c_ptr->madj_con);
  change_stat (A_CHR, c_ptr->madj_chr);
  for (i = 0; i < 6; i++) {
    pystats.cur_stat[i] = pystats.max_stat[i];
    set_use_stat(i);
  }

  pymisc.ptodam = todam_adj();	/* Real values		*/
  pymisc.ptohit = tohit_adj();
  pymisc.ptoac  = toac_adj();
  pymisc.pac    = 0;
  pymisc.dis_td = pymisc.ptodam; /* Displayed values	*/
  pymisc.dis_th = pymisc.ptohit;
  pymisc.dis_tac= pymisc.ptoac;
  pymisc.dis_ac = pymisc.pac + pymisc.dis_tac;

  /* now set misc stats, do this after setting stats because
     of con_adj() for hitpoints */
  //  m_ptr = &pymisc;
  pymisc.hitdie += c_ptr->adj_hd;
  pymisc.mhp = con_adj() + pymisc.hitdie;
  pymisc.chp = pymisc.mhp;
  pymisc.chp_frac = 0;

  /* initialize hit_points array */
  /* put bounds on total possible hp, only succeed if it is within
     1/8 of average value */
  min_value = (MAX_PLAYER_LEVEL*3/8 * (pymisc.hitdie-1)) +
    MAX_PLAYER_LEVEL;
  max_value = (MAX_PLAYER_LEVEL*5/8 * (pymisc.hitdie-1)) +
    MAX_PLAYER_LEVEL;
  player_hp[0] = pymisc.hitdie;
  do {
    for (i = 1; i < MAX_PLAYER_LEVEL; i++) {
      player_hp[i] = randint((Short)pymisc.hitdie);
      player_hp[i] += player_hp[i-1];
    }
  } while ((player_hp[MAX_PLAYER_LEVEL-1] < min_value)
	   || (player_hp[MAX_PLAYER_LEVEL-1] > max_value));

  pymisc.bth += c_ptr->mbth;
  pymisc.bthb += c_ptr->mbthb;	/*RAK*/
  pymisc.srh += c_ptr->msrh;
  pymisc.disarm += c_ptr->mdis;
  pymisc.fos += c_ptr->mfos;
  pymisc.stl += c_ptr->mstl;
  pymisc.save += c_ptr->msav;
  pymisc.expfact += c_ptr->m_exp;
}


/* Given a stat value, return a monetary value, which affects the amount
   of gold a player has. */
static int monval (UChar i)
{
  return 5 * ((Short)i - 10);
}

static void get_money()
{
  Short tmp, gold;
  UChar *a_ptr;

  a_ptr = pystats.max_stat;
  tmp = monval (a_ptr[A_STR]) + monval (a_ptr[A_INT])
      + monval (a_ptr[A_WIS]) + monval (a_ptr[A_CON])
      + monval (a_ptr[A_DEX]);

  gold = pymisc.sc*6 + randint (25) + 325;	/* Social Class adj */
  gold -= tmp;					/* Stat adj */
  gold += monval (a_ptr[A_CHR]);		/* Charisma adj	*/
  if (!pymisc.male)
    gold += 50;			/* She charmed the banker into it! -CJS- */
  if (gold < 80)
    gold = 80;			/* Minimum */
  pymisc.au = gold;
}

// call this an arbitrary number of times, after selecting gender
// but before selecting class
void reroll_char()
{
  /* here we start a loop giving a player a choice of characters -RGM- */
  get_all_stats ();
  get_history();
  get_ahw();
  //  print_history();
  //  put_misc1();
  //  put_stats();
}
void post_reroll()
{
  get_class();
  get_money();
  //  put_stats();
  //  put_misc2();
  //  put_misc3();
  //  get_name();
}

///////////////////////////// stuff that was in umoria/main.c///////////

ULong birth_date;
static void char_inven_init();
void other_stuff()
{
  
  birth_date = TimGetSeconds();
  char_inven_init();
  pyflags.food = 7500;
  pyflags.food_digested = 2;
  // XXXXXX
  if (class[pymisc.pclass].spell == MAGE)
    {	  /* Magic realm   */
      //  clear_screen(); /* makes spell list easier to read */
      calc_spells(A_INT);
      calc_mana(A_INT);
    }
  else if (class[pymisc.pclass].spell == PRIEST)
    {	  /* Clerical realm*/
      calc_spells(A_WIS);
      //   clear_screen(); /* force out the 'learn prayer' message */
      calc_mana(A_WIS);
    }
  /* prevent ^c quit from entering score into scoreboard,
     and prevent signal from creating panic save until this point,
     all info needed for save file is now valid */
  //  character_generated = true;
  //  generate = true;
  
}
/* Init players with some belongings			-RAK-	*/
// This must be revised.. the functions called assume database record
// which inven_init is not!  where do I store player inventory anyway.
static void char_inven_init()
{
  Short i, j;
  inven_type inven_init;
  treasure_type *from;

  //  * this is needed for bash to work right, it can't hurt anyway *
  for (i = 0; i < INVEN_ARRAY_SIZE; i++)
    invcopy(inventory, i, OBJ_NOTHING); // maybe unneeded

  for (i = 0; i < 5; i++) {
    j = player_init[pymisc.pclass][i];
    from = object_list + j;
    //    invcopy(&inven_init, 0, j);
    // This is the same as invcopy, but the target is not a database record
    inven_init.index     = j;
    inven_init.name2     = SN_NULL;
    inven_init.inscrip[0]= '\0';
    inven_init.flags     = from->flags;
    inven_init.tval      = from->tval;
    inven_init.tchar     = from->tchar;
    inven_init.p1        = from->p1;
    inven_init.cost      = from->cost;
    inven_init.subval    = from->subval;
    inven_init.number    = from->number;
    inven_init.weight    = from->weight;
    inven_init.tohit     = from->tohit;
    inven_init.todam     = from->todam;
    inven_init.ac        = from->ac;
    inven_init.toac      = from->toac;
    inven_init.damage[0] = from->damage[0];
    inven_init.damage[1] = from->damage[1];
    inven_init.level     = from->level;
    inven_init.ident     = 0;
    //    * this makes it known2 and known1 *
    //    store_bought(&inven_init);
    // This is the same as store_bought, but the target is not a record
    inven_init.ident |= (ID_STOREBOUGHT | ID_KNOWN2);
    // (store_bought also calls unsample.  I don't think it's necessary here.)

    //    * must set this bit to display tohit/todam for stiletto *
    if (inven_init.tval == TV_SWORD)
      inven_init.ident |= ID_SHOW_HITDAM;
    inven_carry(&inven_init); // gosh, I hope this works.
  }

  //  * weird place for it, but why not? *
  //    for (i = 0; i < 32; i++)
  //      spell_order[i] = 99; // xxx not implemented yet
}
