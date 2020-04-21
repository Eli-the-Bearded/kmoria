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

// This is temporary.  I should put it in a read-only record.

/* ERROR: attack #35 is no longer used */
struct m_attack_type monster_attacks[N_MONS_ATTS] = {
/* 0 */	{0, 0, 0, 0},	{1, 1, 1, 2},	{1, 1, 1, 3},	{1, 1, 1, 4},
	{1, 1, 1, 5},	{1, 1, 1, 6},	{1, 1, 1, 7},	{1, 1, 1, 8},
	{1, 1, 1, 9},	{1, 1, 1, 10},	{1, 1, 1, 12},	{1, 1, 2, 2},
	{1, 1, 2, 3},	{1, 1, 2, 4},	{1, 1, 2, 5},	{1, 1, 2, 6},
	{1, 1, 2, 8},	{1, 1, 3, 4},	{1, 1, 3, 5},	{1, 1, 3, 6},
/* 20 */{1, 1, 3, 8},	{1, 1, 4, 3},	{1, 1, 4, 6},	{1, 1, 5, 5},
	{1, 2, 1, 1},	{1, 2, 1, 2},	{1, 2, 1, 3},	{1, 2, 1, 4},
	{1, 2, 1, 5},	{1, 2, 1, 6},	{1, 2, 1, 7},	{1, 2, 1, 8},
	{1, 2, 1, 10},	{1, 2, 2, 3},	{1, 2, 2, 4},	{1, 2, 2, 5},
	{1, 2, 2, 6},	{1, 2, 2, 8},	{1, 2, 2, 10},	{1, 2, 2, 12},
/* 40 */{1, 2, 2, 14},	{1, 2, 3, 4},	{1, 2, 3, 12},	{1, 2, 4, 4},
	{1, 2, 4, 5},	{1, 2, 4, 6},	{1, 2, 4, 8},	{1, 2, 5, 4},
	{1, 2, 5, 8},	{1, 3, 1, 1},	{1, 3, 1, 2},	{1, 3, 1, 3},
	{1, 3, 1, 4},	{1, 3, 1, 5},	{1, 3, 1, 8},	{1, 3, 1, 9},
	{1, 3, 1, 10},	{1, 3, 1, 12},	{1, 3, 3, 3},	{1, 4, 1, 2},
/* 60 */{1, 4, 1, 3},	{1, 4, 1, 4},	{1, 4, 2, 4},	{1, 5, 1, 2},
	{1, 5, 1, 3},	{1, 5, 1, 4},	{1, 5, 1, 5},	{1, 10, 5, 6},
	{1, 12, 1, 1},	{1, 12, 1, 2},	{1, 13, 1, 1},	{1, 13, 1, 3},
	{1, 14, 0, 0},	{1, 16, 1, 4},	{1, 16, 1, 6},	{1, 16, 1, 8},
	{1, 16, 1, 10},	{1, 16, 2, 8},	{1, 17, 8, 12},	{1, 18, 0, 0},
/* 80 */{2, 1, 3, 4},	{2, 1, 4, 6},	{2, 2, 1, 4},	{2, 2, 2, 4},
	{2, 2, 4, 4},	{2, 4, 1, 4},	{2, 4, 1, 7},	{2, 5, 1, 5},
	{2, 7, 1, 6},	{3, 1, 1, 4},	{3, 5, 1, 8},	{3, 13, 1, 4},
	{3, 7, 0, 0},	{4, 1, 1, 1},	{4, 1, 1, 4},	{4, 2, 1, 2},
	{4, 2, 1, 6},	{4, 5, 0, 0},	{4, 7, 0, 0},	{4, 10, 0, 0},
/*100 */{4, 13, 1, 6},	{5, 1, 2, 6},	{5, 1, 3, 7},	{5, 1, 4, 6},
	{5, 1, 10, 12},	{5, 2, 1, 3},	{5, 2, 3, 6},	{5, 2, 3, 12},
	{5, 5, 4, 4},	{5, 9, 3, 7},	{5, 9, 4, 5},	{5, 12, 1, 6},
	{6, 2, 1, 3},	{6, 2, 2, 8},	{6, 2, 4, 4},	{6, 5, 1, 10},
	{6, 5, 2, 3},	{6, 8, 1, 5},	{6, 9, 2, 6},	{6, 9, 3, 6},
/*120 */{7, 1, 3, 6},	{7, 2, 1, 3},	{7, 2, 1, 6},	{7, 2, 3, 6},
	{7, 2, 3, 10},	{7, 5, 1, 6},	{7, 5, 2, 3},	{7, 5, 2, 6},
	{7, 5, 4, 4},	{7, 12, 1, 4},	{8, 1, 3, 8},	{8, 2, 1, 3},
	{8, 2, 2, 6},	{8, 2, 3, 8},	{8, 2, 5, 5},	{8, 5, 5, 4},
	{9, 5, 1, 2},	{9, 5, 2, 5},	{9, 5, 2, 6},	{9, 8, 2, 4},
/*140 */{9, 12, 1, 3},	{10, 2, 1, 6},	{10, 4, 1, 1},	{10, 7, 2, 6},
	{10, 9, 1, 2},	{11, 1, 1, 2},	{11, 7, 0, 0},	{11, 13, 2, 4},
	{12, 5, 0, 0},	{13, 5, 0, 0},	{13, 19, 0, 0},	{14, 1, 1, 3},
	{14, 1, 3, 4},	{14, 2, 1, 3},	{14, 2, 1, 4},	{14, 2, 1, 5},
	{14, 2, 1, 6},	{14, 2, 1, 10},	{14, 2, 2, 4},	{14, 2, 2, 5},
/*160 */{14, 2, 2, 6},	{14, 2, 3, 4},	{14, 2, 3, 9},	{14, 2, 4, 4},
	{14, 4, 1, 2},	{14, 4, 1, 4},	{14, 4, 1, 8},	{14, 4, 2, 5},
	{14, 5, 1, 2},	{14, 5, 1, 3},	{14, 5, 2, 4},	{14, 5, 2, 6},
	{14, 5, 3, 5},	{14, 12, 1, 2},	{14, 12, 1, 4},	{14, 13, 2, 4},
	{15, 2, 1, 6},	{15, 2, 3, 6},	{15, 5, 1, 8},	{15, 5, 2, 8},
/*180 */{15, 5, 2, 10},	{15, 5, 2, 12},	{15, 12, 1, 3},	{16, 13, 1, 2},
	{17, 3, 1, 10},	{18, 5, 0, 0},	{19, 5, 5, 8},	{19, 5, 12, 8},
	{19, 5, 14, 8},	{19, 5, 15, 8},	{19, 5, 18, 8},	{19, 5, 20, 8},
	{19, 5, 22, 8},	{19, 5, 26, 8},	{19, 5, 30, 8},	{19, 5, 32, 8},
	{19, 5, 34, 8},	{19, 5, 36, 8},	{19, 5, 38, 8},	{19, 5, 42, 8},
/*200 */{19, 5, 44, 8},	{19, 5, 46, 8},	{19, 5, 52, 8},	{20, 10, 0, 0},
	{21, 1, 0, 0},	{21, 5, 0, 0},	{21, 5, 1, 6},	{21, 7, 0, 0},
	{21, 12, 1, 4},	{22, 5, 2, 3},	{22, 12, 0, 0},	{22, 15, 1, 1},
/*212 */{23, 1, 1, 1},	{23, 5, 1, 3},	{24, 5, 0, 0}
};


// How long are these, anyway?  I should put them in a read-only record.

static Char *desc_atype[] = {
  "do something undefined",
  "attack",
  "weaken",
  "confuse",
  "terrify",
  "shoot flames",
  "shoot acid",
  "freeze",
  "shoot lightning",
  "corrode",
  "blind",
  "paralyse",
  "steal money",
  "steal things",
  "poison",
  "reduce dexterity",
  "reduce constitution",
  "drain intelligence",
  "drain wisdom",
  "lower experience",
  "call for help",
  "disenchant",
  "eat your food",
  "absorb light",
  "absorb charges" };
static Char *desc_amethod[] = {
  "make an undefined advance",
  "hit",
  "bite",
  "claw",
  "sting",
  "touch",
  "kick",
  "gaze",
  "breathe",
  "spit",
  "wail",
  "embrace",
  "crawl on you",
  "release spores",
  "beg",
  "slime you",
  "crush",
  "trample",
  "drool",
  "insult" };
static Char *desc_howmuch[] = {
  " not at all",
  " a bit",
  "",
  " quite",
  " very",
  " most",
  " highly",
  " extremely" };

static Char *desc_move[] = {
  "move invisibly",
  "open doors",
  "pass through walls",
  "kill weaker creatures",
  "pick up objects",
  "breed explosively" };
static Char *desc_spell[] = {
  "teleport short distances",
  "teleport long distances",
  "teleport its prey",
  "cause light wounds",
  "cause serious wounds",
  "paralyse its prey",
  "induce blindness",
  "confuse",
  "terrify",
  "summon a monster",
  "summon the undead",
  "slow its prey",
  "drain mana",
  "unknown 1",
  "unknown 2" };
static Char *desc_breath[] = {
  "lightning",
  "poison gases",
  "acid",
  "frost",
  "fire" };
static Char *desc_weakness[] = {
  "frost",
  "fire",
  "poison",
  "acid",
  "bright light",
  "rock remover" };

//static Char roffbuf[80];		/* Line buffer. */
//static Char *roffp;		/* Pointer into line buffer. */
//static Short roffpline;	/* Place to print line now being loaded. */

#define plural(c, ss, sp)	((c) == 1 ? ss : sp)

/* Number of kills needed for information. */

/* the higher the level of the monster, the fewer the kills you need */
#define knowarmor(l,d)		((d) > 304 / (4 + (l)))
/* the higher the level of the monster, the fewer the attacks you need,
   the more damage an attack does, the more attacks you need */
#define knowdamage(l,a,d)	((4 + (l))*(a) > 80 * (d))

/* Do we know anything about this monster? */
Boolean bool_roff_recall(Short mon_num)
{
  recall_type *mp;
  Short i;

  //  if (wizard)
  //    return true;
  mp = &c_recall[mon_num];
  if (mp->r_cmove || mp->r_cdefense || mp->r_kills || mp->r_spells
      || mp->r_deaths)
    return true;
  for (i = 0; i < 4; i++)
    if (mp->r_attacks[i])
      return true;
  return false;
}

/*
  What is the maximum length of a recall string?  Taking max branches,
  6 + strlen(cp->name)max26     (the name:)
  digits(mp->r_deaths) + 83     (x have been killed, and)
  digits(mp->r_kills) + 47      (x have been exterminated)
  digits(cp->level*50) + 40     (it is found at blah)
  11 + 37                       (, and moves err. and unb. qu.)
  39                            (, but does not deign intr.)
  46                            (, but blah magic blah.)
  22                            (kill natural)
  digits(>64K) + 26             (worth xp)
  digits(pymisc.lev) + 27       (to nth level char)
  20 + 46                       (It is resistant to x, y, a, b, and z)
  44 + 232 (desc_spell)         (and is also magical.. foo, bar, or qux)
  13 + digits(cp->spels&CS_FREQ) (1 time in..)
  61 + digits(ac,cdefense,hd0&1) (rating)
  desc_move + 8 + ", and"
  desc_weakness + 22 + ", and"
  52                            (warm, charmed slept.)
  76 + digits(aaf)              (attention)
  52                            (a small object or treasures)
... so far, that's 1084 not counting desc_move and desc_weakness.
(+ 6 26 3 83 3 47 5 40 11 37 39 46 22 6 26 3 27 20 46 44 232 13 3 61
   (+ 3 3 3 3) 8 5 22 5 52 76 3 52)
it does count more desc_spell than is likely, but still it's over 852K.

the missing part is "it can VERB-PHRASE to VERB-PHRASE with damage XXdXX, ..."
where it's desc_amethod then desc_atype.
24 atypes * 19 maxlen, 19 amethods * 14 maxlen.
say 24 * (19 + 14 + "it can "." to "." with damage xxdxx, ") + " and ."
that's 1536 + 6 = 1542.

add the previous lot and it's 1542 + 1084 = 2626.
(well anyway, it seems like it couldn't possibly be over 3K.
so just make the Mondo Buffer that big, I guess.)

*/

/* Print out what we have discovered about this monster. */
void roff_recall(Short mon_num, Char *buf)
{
  Char *p, *q;
  UChar *pu;
  Char temp[80];
  recall_type *mp;
  creature_type *cp;
  Short i;
  Boolean k;
  ULong j;
  Long templong;
  Short mspeed;
  ULong rcmove, rspells;
  UShort rcdefense;
  //  recall_type save_mem;

  mp = &c_recall[mon_num];
  cp = &c_list[mon_num];
  /* if(wizard) has been DELETED. -bas. */
  //  roffpline = 0;
  //  roffp = roffbuf;
  rspells = mp->r_spells & cp->spells & ~CS_FREQ;
  /* the CM_WIN property is always known, set it if a win monster */
  rcmove = mp->r_cmove | (CM_WIN & cp->cmove);
  rcdefense = mp->r_cdefense & cp->cdefense;
  StrPrintF(temp, "The %s:\n", c_names + cp->name_offset);
  StrCat(buf, temp);

  /* Conflict history. */
  if (mp->r_deaths) {
    StrPrintF(temp, "%d of the contributors to your monster memory %s",
	      mp->r_deaths, plural(mp->r_deaths, "has", "have") );
    StrCat(buf, temp);
    StrCat(buf, " been killed by this creature, and ");
    if (mp->r_kills == 0)
      StrCat(buf, "it is not ever known to have been defeated.");
    else {
      StrPrintF(temp,
		"at least %d of the beasts %s been exterminated.",
		mp->r_kills, plural(mp->r_kills, "has", "have") );
      StrCat(buf, temp);
    }
  } else if (mp->r_kills) {
    StrPrintF(temp, "At least %d of these creatures %s",
	      mp->r_kills, plural(mp->r_kills, "has", "have") );
    StrCat(buf, temp);
    StrCat(buf, " been killed by contributors to your monster memory.");
  } else
    StrCat(buf, "No known battles to the death are recalled.");

  /* Immediately obvious. */
  k = false;
  if (cp->level == 0) {
    StrCat(buf, " It lives in the town");
    k = true;
  } else if (mp->r_kills) {
    /* The Balrog is a level 100 monster, but appears at 50 feet.  */
    i = cp->level;
    if (i > WIN_MON_APPEAR)
      i = WIN_MON_APPEAR;
    StrPrintF(temp, " It is normally found at depths of %d feet",
	      i * 50);
    StrCat(buf, temp);
    k = true;
  }

  /* the c_list speed value is 10 greater, so that it can be a int8u */
  mspeed = cp->speed - 10;
  if (rcmove & CM_ALL_MV_FLAGS) {
    if (k)
      StrCat(buf, ", and");
    else {
      StrCat(buf, " It");
      k = true;
    }
    StrCat(buf, " moves");
    if (rcmove & CM_RANDOM_MOVE) {
      /* Turbo C needs a 16 bit int for the array index.  */
      StrCat(buf, desc_howmuch[(Short) ((rcmove & CM_RANDOM_MOVE) >> 3)]);
      StrCat(buf, " erratically");
    }
    if (mspeed == 1)
      StrCat(buf, " at normal speed");
    else {
      if (rcmove & CM_RANDOM_MOVE)
	StrCat(buf, ", and");
      if (mspeed <= 0) {
	if (mspeed == -1)
	  StrCat(buf, " very");
	else if (mspeed < -1)
	  StrCat(buf, " incredibly");
	StrCat(buf, " slowly");
      } else {
	if (mspeed == 3)
	  StrCat(buf, " very");
	else if (mspeed > 3)
	  StrCat(buf, " unbelievably");
	StrCat(buf, " quickly");
      }
    }
  }
  if (rcmove & CM_ATTACK_ONLY) {
    if (k)
      StrCat(buf, ", but");
    else {
      StrCat(buf, " It");
      k = true;
    }
    StrCat(buf, " does not deign to chase intruders");
  }
  if (rcmove & CM_ONLY_MAGIC) {
    if (k)
      StrCat(buf, ", but");
    else {
      StrCat(buf, " It");
      k = true;
    }
    StrCat(buf, " always moves and attacks by using magic");
  }
  // townies are OK up to here.
  if (k)
    StrCat(buf, ".");
  /* Kill it once to know experience, and quality (evil, undead, monsterous).
     The quality of being a dragon is obvious. */
  if (mp->r_kills) {
    StrCat(buf, " A kill of this");
    if (cp->cdefense & CD_ANIMAL)
      StrCat(buf, " natural");
    if (cp->cdefense & CD_EVIL)
      StrCat(buf, " evil");
    if (cp->cdefense & CD_UNDEAD)
      StrCat(buf, " undead");

    /* calculate the integer exp part, can be larger than 64K when first
       level character looks at Balrog info, so must store in long */
    templong = (Long) cp->mexp * cp->level / pymisc.lev;
    /* calculate the fractional exp part scaled by 100,
       must use long arithmetic to avoid overflow */
    j = (((Long) cp->mexp * cp->level % pymisc.lev) * (Long) 1000 /
	 pymisc.lev+5) / 10;

    //    StrPrintF(temp, " creature is worth %ld.%02ld %s", templong,
    //	      j, (templong == 1 && j == 0 ? "point" : "points"));
    StrPrintF(temp, " creature is worth %ld.", templong);
    StrCat(buf, temp);
    StrPrintF(temp, ((j < 10) ? "0%ld" : "%ld"), j);
    StrNCat(buf, temp, 2); // since we CAN'T SAY %02ld ... *sigh*
    StrCat(buf, ((templong == 1 && j == 0) ? " point" : " points"));
    // Ok, that part isn't quite working right, but at least doesn't CRASH.

    if (pymisc.lev / 10 == 1) p = "th";
    else {
      i = pymisc.lev % 10;
      if (i == 1)		p = "st";
      else if (i == 2)  	p = "nd";
      else if (i == 3)          p = "rd";
      else			p = "th";
    }
    i = pymisc.lev;
    if (i == 8 || i == 11 || i == 18)   q = "an";
    else				q = "a";
    StrPrintF(temp, " for %s %d%s level character.", q, i, p);
    StrCat(buf, temp);
  }

  /* Spells known, if have been used against us.
     Breath weapons or resistance might be known only because we cast spells 
     at it. */
  k = true;
  j = rspells;
  for (i = 0; j & CS_BREATHE; i++) {
    if (j & (CS_BR_LIGHT << i)) {
      j &= ~(CS_BR_LIGHT << i);
      if (k) {
	if (mp->r_spells & CS_FREQ)
	  StrCat(buf, " It can breathe ");
	else
	  StrCat(buf, " It is resistant to ");
	k = false;
      } else if (j & CS_BREATHE)
	StrCat(buf, ", ");
      else
	StrCat(buf, " and ");
      StrCat(buf, desc_breath[i]);
    }
  }
  k = true;
  for (i = 0; j & CS_SPELLS; i++) {
    if (j & (CS_TEL_SHORT << i)) {
      j &= ~(CS_TEL_SHORT << i);
      if (k) {
	if (rspells & CS_BREATHE)
	  StrCat(buf, ", and is also");
	else
	  StrCat(buf, " It is");
	StrCat(buf, " magical, casting spells which ");
	k = false;
      }
      else if (j & CS_SPELLS)
	StrCat(buf, ", ");
      else
	StrCat(buf, " or ");
      StrCat(buf, desc_spell[i]);
    }
  }
  if (rspells & (CS_BREATHE|CS_SPELLS)) {
    if ((mp->r_spells & CS_FREQ) > 5) {	/* Could offset by level */
      StrPrintF(temp, "; 1 time in %ld", cp->spells & CS_FREQ);
      StrCat(buf, temp);
    }
    StrCat(buf, ".");
  }
  /* Do we know how hard they are to kill? Armor class, hit die. */
  if (knowarmor(cp->level, mp->r_kills)) {
    StrPrintF(temp, " It has an armor rating of %d", cp->ac);
    StrCat(buf, temp);
    StrPrintF(temp, " and a%slife rating of %dd%d.",
	      ((cp->cdefense & CD_MAX_HP) ? " maximized " : " "),
	      cp->hd[0], cp->hd[1]);
    StrCat(buf, temp);
  }
  /* Do we know how clever they are? Special abilities. */
  k = true;
  j = rcmove;
  for (i = 0; j & CM_SPECIAL; i++) {
    if (j & (CM_INVISIBLE << i)) {
      j &= ~(CM_INVISIBLE << i);
      if (k) {
	StrCat(buf, " It can ");
	k = false;
      } else if (j & CM_SPECIAL)
	StrCat(buf, ", ");
      else
	StrCat(buf, " and ");
      StrCat(buf, desc_move[i]);
    }
  }
  if (!k)
    StrCat(buf, ".");
  /* Do we know its special weaknesses? Most cdefense flags. */
  k = true;
  j = rcdefense;
  for (i = 0; j & CD_WEAKNESS; i++) {
    if (j & (CD_FROST << i)) {
      j &= ~(CD_FROST << i);
      if (k) {
	StrCat(buf, " It is susceptible to ");
	k = false;
      } else if (j & CD_WEAKNESS)
	StrCat(buf, ", ");
      else
	StrCat(buf, " and ");
      StrCat(buf, desc_weakness[i]);
    }
  }
  if (!k)
    StrCat(buf, ".");
  if (rcdefense & CD_INFRA)
    StrCat(buf, " It is warm blooded");
  if (rcdefense & CD_NO_SLEEP) {
    if (rcdefense & CD_INFRA)
      StrCat(buf, ", and");
    else
      StrCat(buf, " It");
    StrCat(buf, " cannot be charmed or slept");
  }
  if (rcdefense & (CD_NO_SLEEP|CD_INFRA))
    StrCat(buf, ".");
  /* Do we know how aware it is? */
  if (((mp->r_wake * mp->r_wake) > cp->sleep) || mp->r_ignore == MAX_UCHAR ||
      (cp->sleep == 0 && mp->r_kills >= 10)) {
    StrCat(buf, " It ");
    if(cp->sleep > 200)
      StrCat(buf, "prefers to ignore");
    else if(cp->sleep > 95)
      StrCat(buf, "pays very little attention to");
    else if(cp->sleep > 75)
      StrCat(buf, "pays little attention to");
    else if(cp->sleep > 45)
      StrCat(buf, "tends to overlook");
    else if(cp->sleep > 25)
      StrCat(buf, "takes quite a while to see");
    else if(cp->sleep > 10)
      StrCat(buf, "takes a while to see");
    else if(cp->sleep > 5)
      StrCat(buf, "is fairly observant of");
    else if(cp->sleep > 3)
      StrCat(buf, "is observant of");
    else if(cp->sleep > 1)
      StrCat(buf, "is very observant of");
    else if(cp->sleep != 0)
      StrCat(buf, "is vigilant for");
    else
      StrCat(buf, "is ever vigilant for");
    StrPrintF(temp, " intruders, which it may notice from %d feet.",
	      10 * cp->aaf);
    StrCat(buf, temp);
  }
  /* Do we know what it might carry? */
  if (rcmove & (CM_CARRY_OBJ|CM_CARRY_GOLD)) {
    StrCat(buf, " It may");
    j = (rcmove & CM_TREASURE) >> CM_TR_SHIFT;
    if (j == 1) {
      if ((cp->cmove & CM_TREASURE) == CM_60_RANDOM)
	StrCat(buf, " sometimes");
      else
	StrCat(buf, " often");
    } else if ((j == 2) && ((cp->cmove & CM_TREASURE) ==
			    (CM_60_RANDOM|CM_90_RANDOM)))
      StrCat(buf, " often");
    StrCat(buf, " carry");
    if (rcmove & CM_SMALL_OBJ)
      p = " small objects";
    else
      p = " objects";
    if (j == 1) {
      if (rcmove & CM_SMALL_OBJ)
	p = " a small object";
      else
	p = " an object";
    } else if (j == 2)
      StrCat(buf, " one or two");
    else {
      StrPrintF(temp, " up to %ld", j);
      StrCat(buf, temp);
    }
    if (rcmove & CM_CARRY_OBJ) {
      StrCat(buf, p);
      if (rcmove & CM_CARRY_GOLD) {
	StrCat(buf, " or treasure");
	if (j > 1)
	  StrCat(buf, "s");
      }
      StrCat(buf, ".");
    } else if (j != 1)
      StrCat(buf, " treasures.");
    else
      StrCat(buf, " treasure.");
  }

  /* We know about attacks it has used on us, and maybe the damage they do. */
  /* k is the total number of known attacks, used for punctuation */
  k = 0;
  for (j = 0; j < 4; j++)
    /* Turbo C needs a 16 bit int for the array index.  */
    if (mp->r_attacks[(Short) j])
      k++;
  pu = cp->damage;
  /* j counts the attacks as printed, used for punctuation */
  j = 0;
  for (i = 0; *pu != 0 && i < 4; pu++, i++) {
    Short att_type, att_how, d1, d2;

    /* don't print out unknown attacks */
    if (!mp->r_attacks[i])
      continue;

    att_type = monster_attacks[*pu].attack_type;
    att_how = monster_attacks[*pu].attack_desc;
    d1 = monster_attacks[*pu].attack_dice;
    d2 = monster_attacks[*pu].attack_sides;

    j++;
    if (j == 1)
      StrCat(buf, " It can ");
    else if (j == k)
      StrCat(buf, ", and ");
    else
      StrCat(buf, ", ");

    if (att_how > 19)
      att_how = 0;
    StrCat(buf, desc_amethod[att_how]);
    if (att_type != 1 || (d1 > 0 && d2 > 0)) {
      StrCat(buf, " to ");
      if (att_type > 24)
	att_type = 0;
      StrCat(buf, desc_atype[att_type]);
      if (d1 && d2) {
	if (knowdamage(cp->level, mp->r_attacks[i], d1*d2)) {
	  if (att_type == 19)	/* Loss of experience */
	    StrCat(buf, " by");
	  else
	    StrCat(buf, " with damage");
	  StrPrintF(temp, " %dd%d", d1, d2 );
	  StrCat(buf, temp);
	}
      }
    }
  }
  if (j)
    StrCat(buf, ".");
  else if (k > 0 && mp->r_attacks[0] >= 10)
    StrCat(buf, " It has no physical attacks.");
  else
    StrCat(buf, " Nothing is known about its attack.");
  /* Always know the win creature. */
  if (cp->cmove & CM_WIN)
    StrCat(buf, " Killing one of these wins the game!");
  StrCat(buf, "\n");
  //  prt("--pause--", roffpline, 0);
  //  if (wizard)
  //    *mp = save_mem;

  return;//  return inkey();
}

// 'roff' is no longer needed (it has been rplaced with StrCat,
// concatenating each string onto the monster text handle of RecallForm.)
