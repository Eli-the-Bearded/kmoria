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

// Needed:
// data: missile_ctr

Short missile_ctr; // this wants to be an extern and stuff

inven_type straw_object;
void magic_treasure(Short x, Short level)
{
  inven_type *t_ptr;
  Short chance, special, cursed, i;
  Short tmp;

  return;

  chance = OBJ_BASE_MAGIC + level;
  if (chance > OBJ_BASE_MAX)
    chance = OBJ_BASE_MAX;
  special = chance / OBJ_DIV_SPECIAL;
  cursed  = (10 * chance) / OBJ_DIV_CURSED;
  //   t_ptr = t_list[x];
  // NOOOOO.  We will DmWrite t_ptr back to t_list at offset x when done.
  t_ptr = &straw_object;

  /* some objects appear multiple times in the object_list with different
     levels, this is to make the object occur more often, however, for
     consistency, must set the level of these duplicates to be the same
     as the object with the lowest level */

  /* Depending on treasure type, it can have certain magical properties*/
  switch (t_ptr->tval) {
  case TV_SHIELD: case TV_HARD_ARMOR: case TV_SOFT_ARMOR:
    if (magik(chance)) {
      t_ptr->toac += m_bonus(1, 30, level);
      if (magik(special))
	switch(randint(9)) {
	case 1:
	  t_ptr->flags |= (TR_RES_LIGHT|TR_RES_COLD|TR_RES_ACID|
			   TR_RES_FIRE);
	  t_ptr->name2 = SN_R;
	  t_ptr->toac += 5;
	  t_ptr->cost += 2500;
	  break;
	case 2:	 /* Resist Acid	  */
	  t_ptr->flags |= TR_RES_ACID;
	  t_ptr->name2 = SN_RA;
	  t_ptr->cost += 1000;
	  break;
	case 3: case 4:	 /* Resist Fire	  */
	  t_ptr->flags |= TR_RES_FIRE;
	  t_ptr->name2 = SN_RF;
	  t_ptr->cost += 600;
	  break;
	case 5: case 6:	/* Resist Cold	 */
	  t_ptr->flags |= TR_RES_COLD;
	  t_ptr->name2 = SN_RC;
	  t_ptr->cost += 600;
	  break;
	case 7: case 8: case 9:  /* Resist Lightning*/
	  t_ptr->flags |= TR_RES_LIGHT;
	  t_ptr->name2 = SN_RL;
	  t_ptr->cost += 500;
	  break;
	}
    } else if (magik(cursed)) {
      t_ptr->toac -= m_bonus(1, 40, level);
      t_ptr->cost = 0;
      t_ptr->flags |= TR_CURSED;
    }
    break;

  case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
    /* always show tohit/todam values if identified */
    t_ptr->ident |= ID_SHOW_HITDAM;
    if (magik(chance)) {
      t_ptr->tohit += m_bonus(0, 40, level);
      /* Magical damage bonus now proportional to weapon base damage */
      tmp = t_ptr->damage[0] * t_ptr->damage[1];
      t_ptr->todam += m_bonus(0, 4*tmp, tmp*level/10);
      /* the 3*special/2 is needed because weapons are not as common as
	 before change to treasure distribution, this helps keep same
	 number of ego weapons same as before, see also missiles */
      if (magik(3*special/2))
	switch(randint(16)) {
	case 1:	/* Holy Avenger	 */
	  t_ptr->flags |= (TR_SEE_INVIS|TR_SUST_STAT|TR_SLAY_UNDEAD|
			   TR_SLAY_EVIL|TR_STR);
	  t_ptr->tohit += 5;
	  t_ptr->todam += 5;
	  t_ptr->toac  += randint(4);
	  /* the value in p1 is used for strength increase */
	  /* p1 is also used for sustain stat */
	  t_ptr->p1    = randint(4);
	  t_ptr->name2 = SN_HA;
	  t_ptr->cost += t_ptr->p1*500;
	  t_ptr->cost += 10000;
	  break;
	case 2:	/* Defender	 */
	  t_ptr->flags |= (TR_FFALL|TR_RES_LIGHT|TR_SEE_INVIS|TR_FREE_ACT
			   |TR_RES_COLD|TR_RES_ACID|TR_RES_FIRE|
			   TR_REGEN|TR_STEALTH);
	  t_ptr->tohit += 3;
	  t_ptr->todam += 3;
	  t_ptr->toac  += 5 + randint(5);
	  t_ptr->name2 = SN_DF;
	  /* the value in p1 is used for stealth */
	  t_ptr->p1    = randint(3);
	  t_ptr->cost += t_ptr->p1*500;
	  t_ptr->cost += 7500;
	  break;
	case 3: case 4:	 /* Slay Animal  */
	  t_ptr->flags |= TR_SLAY_ANIMAL;
	  t_ptr->tohit += 2;
	  t_ptr->todam += 2;
	  t_ptr->name2 = SN_SA;
	  t_ptr->cost += 3000;
	  break;
	case 5: case 6:	/* Slay Dragon	 */
	  t_ptr->flags |= TR_SLAY_DRAGON;
	  t_ptr->tohit += 3;
	  t_ptr->todam += 3;
	  t_ptr->name2 = SN_SD;
	  t_ptr->cost += 4000;
	  break;
	case 7: case 8:	  /* Slay Evil	   */
	  t_ptr->flags |= TR_SLAY_EVIL;
	  t_ptr->tohit += 3;
	  t_ptr->todam += 3;
	  t_ptr->name2 = SN_SE;
	  t_ptr->cost += 4000;
	  break;
	case 9: case 10:	 /* Slay Undead	  */
	  t_ptr->flags |= (TR_SEE_INVIS|TR_SLAY_UNDEAD);
	  t_ptr->tohit += 3;
	  t_ptr->todam += 3;
	  t_ptr->name2 = SN_SU;
	  t_ptr->cost += 5000;
	  break;
	case 11: case 12: case 13:   /* Flame Tongue  */
	  t_ptr->flags |= TR_FLAME_TONGUE;
	  t_ptr->tohit++;
	  t_ptr->todam += 3;
	  t_ptr->name2 = SN_FT;
	  t_ptr->cost += 2000;
	  break;
	case 14: case 15: case 16:   /* Frost Brand   */
	  t_ptr->flags |= TR_FROST_BRAND;
	  t_ptr->tohit++;
	  t_ptr->todam++;
	  t_ptr->name2 = SN_FB;
	  t_ptr->cost += 1200;
	  break;
	}
    } else if (magik(cursed)) {
      t_ptr->tohit -= m_bonus(1, 55, level);
      /* Magical damage bonus now proportional to weapon base damage */
      tmp = t_ptr->damage[0] * t_ptr->damage[1];
      t_ptr->todam -= m_bonus(1, 11*tmp/2, tmp*level/10);
      t_ptr->flags |= TR_CURSED;
      t_ptr->cost = 0;
    }
    break;

  case TV_BOW:
    /* always show tohit/todam values if identified */
    t_ptr->ident |= ID_SHOW_HITDAM;
    if (magik(chance)) {
      t_ptr->tohit += m_bonus(1, 30, level);
      t_ptr->todam += m_bonus(1, 20, level); /* add damage. -CJS- */
    } else if (magik(cursed)) {
      t_ptr->tohit -= m_bonus(1, 50, level);
      t_ptr->todam -= m_bonus(1, 30, level); /* add damage. -CJS- */
      t_ptr->flags |= TR_CURSED;
      t_ptr->cost = 0;
    }
    break;

  case TV_DIGGING:
    /* always show tohit/todam values if identified */
    t_ptr->ident |= ID_SHOW_HITDAM;
    if (magik(chance)) {
      tmp = randint(3);
      if (tmp < 3)
	t_ptr->p1 += m_bonus(0, 25, level);
      else {
	/* a cursed digging tool */
	t_ptr->p1 = -m_bonus(1, 30, level);
	t_ptr->cost = 0;
	t_ptr->flags |= TR_CURSED;
      }
    }
    break;

  case TV_GLOVES:
    if (magik(chance)) {
      t_ptr->toac += m_bonus(1, 20, level);
      if (magik(special)) {
	if (randint(2) == 1) {
	  t_ptr->flags |= TR_FREE_ACT;
	  t_ptr->name2 = SN_FREE_ACTION;
	  t_ptr->cost += 1000;
	} else {
	  t_ptr->ident |= ID_SHOW_HITDAM;
	  t_ptr->tohit += 1 + randint(3);
	  t_ptr->todam += 1 + randint(3);
	  t_ptr->name2 = SN_SLAYING;
	  t_ptr->cost += (t_ptr->tohit+t_ptr->todam)*250;
	}
      }
    } else if (magik(cursed)) {
      if (magik(special)) {
	if (randint(2) == 1) {
	  t_ptr->flags |= TR_DEX;
	  t_ptr->name2 = SN_CLUMSINESS;
	} else {
	  t_ptr->flags |= TR_STR;
	  t_ptr->name2 = SN_WEAKNESS;
	}
	t_ptr->ident |= ID_SHOW_P1;
	t_ptr->p1   = -m_bonus(1, 10, level);
      }
      t_ptr->toac -= m_bonus(1, 40, level);
      t_ptr->flags |= TR_CURSED;
      t_ptr->cost = 0;
    }
    break;

  case TV_BOOTS:
    if (magik(chance)) {
	t_ptr->toac += m_bonus(1, 20, level);
	if (magik(special)) {
	    tmp = randint(12);
	    if (tmp > 5) {
		t_ptr->flags |= TR_FFALL;
		t_ptr->name2 = SN_SLOW_DESCENT;
		t_ptr->cost += 250;
	      }
	    else if (tmp == 1) {
		t_ptr->flags |= TR_SPEED;
		t_ptr->name2 = SN_SPEED;
		t_ptr->ident |= ID_SHOW_P1;
		t_ptr->p1 = 1;
		t_ptr->cost += 5000;
	      }
	    else /* 2 - 5 */ {
		t_ptr->flags |= TR_STEALTH;
		t_ptr->ident |= ID_SHOW_P1;
		t_ptr->p1 = randint(3);
		t_ptr->name2 = SN_STEALTH;
		t_ptr->cost += 500;
	      }
	  }
      }
    else if (magik(cursed)) {
	tmp = randint(3);
	if (tmp == 1) {
	    t_ptr->flags |= TR_SPEED;
	    t_ptr->name2 = SN_SLOWNESS;
	    t_ptr->ident |= ID_SHOW_P1;
	    t_ptr->p1 = -1;
	  }
	else if (tmp == 2) {
	    t_ptr->flags |= TR_AGGRAVATE;
	    t_ptr->name2 = SN_NOISE;
	  }
	else {
	    t_ptr->name2 = SN_GREAT_MASS;
	    t_ptr->weight = t_ptr->weight * 5;
	  }
	t_ptr->cost = 0;
	t_ptr->toac -= m_bonus(2, 45, level);
	t_ptr->flags |= TR_CURSED;
      }
    break;

  case TV_HELM:  /* Helms */
    if ((t_ptr->subval >= 6) && (t_ptr->subval <= 8)) {
	/* give crowns a higher chance for magic */
	chance += (int) (t_ptr->cost / 100);
	special += special;
      }
    if (magik(chance)) {
	t_ptr->toac += m_bonus(1, 20, level);
	if (magik(special)) {
	    if (t_ptr->subval < 6) {
		tmp = randint(3);
		t_ptr->ident |= ID_SHOW_P1;
		if (tmp == 1)
		  {
		    t_ptr->p1 = randint(2);
		    t_ptr->flags |= TR_INT;
		    t_ptr->name2 = SN_INTELLIGENCE;
		    t_ptr->cost += t_ptr->p1*500;
		  }
		else if (tmp == 2)
		  {
		    t_ptr->p1 = randint(2);
		    t_ptr->flags |= TR_WIS;
		    t_ptr->name2 = SN_WISDOM;
		    t_ptr->cost += t_ptr->p1*500;
		  }
		else
		  {
		    t_ptr->p1 = 1 + randint(4);
		    t_ptr->flags |= TR_INFRA;
		    t_ptr->name2 = SN_INFRAVISION;
		    t_ptr->cost += t_ptr->p1*250;
		  }
	      }
	    else {
		switch(randint(6))
		  {
		  case 1:
		    t_ptr->ident |= ID_SHOW_P1;
		    t_ptr->p1 = randint(3);
		    t_ptr->flags |= (TR_FREE_ACT|TR_CON|TR_DEX|TR_STR);
		    t_ptr->name2 = SN_MIGHT;
		    t_ptr->cost += 1000 + t_ptr->p1*500;
		    break;
		  case 2:
		    t_ptr->ident |= ID_SHOW_P1;
		    t_ptr->p1 = randint(3);
		    t_ptr->flags |= (TR_CHR|TR_WIS);
		    t_ptr->name2 = SN_LORDLINESS;
		    t_ptr->cost += 1000 + t_ptr->p1*500;
		    break;
		  case 3:
		    t_ptr->ident |= ID_SHOW_P1;
		    t_ptr->p1 = randint(3);
		    t_ptr->flags |= (TR_RES_LIGHT|TR_RES_COLD|TR_RES_ACID|
				     TR_RES_FIRE|TR_INT);
		    t_ptr->name2 = SN_MAGI;
		    t_ptr->cost += 3000 + t_ptr->p1*500;
		    break;
		  case 4:
		    t_ptr->ident |= ID_SHOW_P1;
		    t_ptr->p1 = randint(3);
		    t_ptr->flags |= TR_CHR;
		    t_ptr->name2 = SN_BEAUTY;
		    t_ptr->cost += 750;
		    break;
		  case 5:
		    t_ptr->ident |= ID_SHOW_P1;
		    t_ptr->p1 = 5*(1 + randint(4));
		    t_ptr->flags |= (TR_SEE_INVIS|TR_SEARCH);
		    t_ptr->name2 = SN_SEEING;
		    t_ptr->cost += 1000 + t_ptr->p1*100;
		    break;
		  case 6:
		    t_ptr->flags |= TR_REGEN;
		    t_ptr->name2 = SN_REGENERATION;
		    t_ptr->cost += 1500;
		    break;
		  }
	      }
	  }
      }
    else if (magik(cursed)) {
	t_ptr->toac -= m_bonus(1, 45, level);
	t_ptr->flags |= TR_CURSED;
	t_ptr->cost = 0;
	if (magik(special))
	  switch(randint(7))
	    {
	    case 1:
	      t_ptr->ident |= ID_SHOW_P1;
	      t_ptr->p1 = -randint (5);
	      t_ptr->flags |= TR_INT;
	      t_ptr->name2 = SN_STUPIDITY;
	      break;
	    case 2:
	      t_ptr->ident |= ID_SHOW_P1;
	      t_ptr->p1 = -randint (5);
	      t_ptr->flags |= TR_WIS;
	      t_ptr->name2 = SN_DULLNESS;
	      break;
	    case 3:
	      t_ptr->flags |= TR_BLIND;
	      t_ptr->name2 = SN_BLINDNESS;
	      break;
	    case 4:
	      t_ptr->flags |= TR_TIMID;
	      t_ptr->name2 = SN_TIMIDNESS;
	      break;
	    case 5:
	      t_ptr->ident |= ID_SHOW_P1;
	      t_ptr->p1 = -randint (5);
	      t_ptr->flags |= TR_STR;
	      t_ptr->name2 = SN_WEAKNESS;
	      break;
	    case 6:
	      t_ptr->flags |= TR_TELEPORT;
	      t_ptr->name2 = SN_TELEPORTATION;
	      break;
	    case 7:
	      t_ptr->ident |= ID_SHOW_P1;
	      t_ptr->p1 = -randint (5);
	      t_ptr->flags |= TR_CHR;
	      t_ptr->name2 = SN_UGLINESS;
	      break;
	    }
      }
    break;

  case TV_RING: /* Rings	      */
    switch(t_ptr->subval) {
      case 0: case 1: case 2: case 3:
	if (magik(cursed)) {
	    t_ptr->p1 = -m_bonus(1, 20, level);
	    t_ptr->flags |= TR_CURSED;
	    t_ptr->cost = -t_ptr->cost;
	  }
	else {
	    t_ptr->p1 = m_bonus(1, 10, level);
	    t_ptr->cost += t_ptr->p1*100;
	  }
	break;
      case 4:
	if (magik(cursed)) {
	    t_ptr->p1 = -randint(3);
	    t_ptr->flags |= TR_CURSED;
	    t_ptr->cost = -t_ptr->cost;
	  }
	else
	  t_ptr->p1 = 1;
	break;
      case 5:
	t_ptr->p1 = 5 * m_bonus(1, 20, level);
	t_ptr->cost += t_ptr->p1*50;
	if (magik (cursed)) {
	    t_ptr->p1 = -t_ptr->p1;
	    t_ptr->flags |= TR_CURSED;
	    t_ptr->cost = -t_ptr->cost;
	  }
	break;
      case 19:     /* Increase damage	      */
	t_ptr->todam += m_bonus(1, 20, level);
	t_ptr->cost += t_ptr->todam*100;
	if (magik(cursed)) {
	    t_ptr->todam = -t_ptr->todam;
	    t_ptr->flags |= TR_CURSED;
	    t_ptr->cost = -t_ptr->cost;
	  }
	break;
      case 20:     /* Increase To-Hit	      */
	t_ptr->tohit += m_bonus(1, 20, level);
	t_ptr->cost += t_ptr->tohit*100;
	if (magik(cursed)) {
	    t_ptr->tohit = -t_ptr->tohit;
	    t_ptr->flags |= TR_CURSED;
	    t_ptr->cost = -t_ptr->cost;
	  }
	break;
      case 21:     /* Protection	      */
	t_ptr->toac += m_bonus(1, 20, level);
	t_ptr->cost += t_ptr->toac*100;
	if (magik(cursed)) {
	    t_ptr->toac = -t_ptr->toac;
	    t_ptr->flags |= TR_CURSED;
	    t_ptr->cost = -t_ptr->cost;
	  }
	break;
      case 24: case 25: case 26:
      case 27: case 28: case 29:
	t_ptr->ident |= ID_NOSHOW_P1;
	break;
      case 30:     /* Slaying	      */
	t_ptr->ident |= ID_SHOW_HITDAM;
	t_ptr->todam += m_bonus(1, 25, level);
	t_ptr->tohit += m_bonus(1, 25, level);
	t_ptr->cost += (t_ptr->tohit+t_ptr->todam)*100;
	if (magik(cursed)) {
	    t_ptr->tohit = -t_ptr->tohit;
	    t_ptr->todam = -t_ptr->todam;
	    t_ptr->flags |= TR_CURSED;
	    t_ptr->cost = -t_ptr->cost;
	  }
	break;
      default:
	break;
      }
    break;

  case TV_AMULET: /* Amulets	      */
    if (t_ptr->subval < 2) {
	if (magik(cursed)) {
	    t_ptr->p1 = -m_bonus(1, 20, level);
	    t_ptr->flags |= TR_CURSED;
	    t_ptr->cost = -t_ptr->cost;
	  }
	else {
	    t_ptr->p1 = m_bonus(1, 10, level);
	    t_ptr->cost += t_ptr->p1*100;
	  }
      }
    else if (t_ptr->subval == 2) {
	t_ptr->p1 = 5 * m_bonus(1, 25, level);
	if (magik(cursed)) {
	    t_ptr->p1 = -t_ptr->p1;
	    t_ptr->cost = -t_ptr->cost;
	    t_ptr->flags |= TR_CURSED;
	  }
	else
	  t_ptr->cost += 50*t_ptr->p1;
      }
    else if (t_ptr->subval == 8) {
	/* amulet of the magi is never cursed */
	t_ptr->p1 = 5 * m_bonus(1, 25, level);
	t_ptr->cost += 20*t_ptr->p1;
      }
    break;

    /* Subval should be even for store, odd for dungeon*/
    /* Dungeon found ones will be partially charged	 */
  case TV_LIGHT:
    if ((t_ptr->subval % 2) == 1) {
	t_ptr->p1 = randint(t_ptr->p1);
	t_ptr->subval -= 1;
      }
    break;

  case TV_WAND:
    switch(t_ptr->subval) {
      case 0:	  t_ptr->p1 = randint(10) +	 6; break;
      case 1:	  t_ptr->p1 = randint(8)  +	 6; break;
      case 2:	  t_ptr->p1 = randint(5)  +	 6; break;
      case 3:	  t_ptr->p1 = randint(8)  +	 6; break;
      case 4:	  t_ptr->p1 = randint(4)  +	 3; break;
      case 5:	  t_ptr->p1 = randint(8)  +	 6; break;
      case 6:	  t_ptr->p1 = randint(20) +	 12; break;
      case 7:	  t_ptr->p1 = randint(20) +	 12; break;
      case 8:	  t_ptr->p1 = randint(10) +	 6; break;
      case 9:	  t_ptr->p1 = randint(12) +	 6; break;
      case 10:   t_ptr->p1 = randint(10) +	 12; break;
      case 11:   t_ptr->p1 = randint(3)  +	 3; break;
      case 12:   t_ptr->p1 = randint(8)  +	 6; break;
      case 13:   t_ptr->p1 = randint(10) +	 6; break;
      case 14:   t_ptr->p1 = randint(5)  +	 3; break;
      case 15:   t_ptr->p1 = randint(5)  +	 3; break;
      case 16:   t_ptr->p1 = randint(5)  +	 6; break;
      case 17:   t_ptr->p1 = randint(5)  +	 4; break;
      case 18:   t_ptr->p1 = randint(8)  +	 4; break;
      case 19:   t_ptr->p1 = randint(6)  +	 2; break;
      case 20:   t_ptr->p1 = randint(4)  +	 2; break;
      case 21:   t_ptr->p1 = randint(8)  +	 6; break;
      case 22:   t_ptr->p1 = randint(5)  +	 2; break;
      case 23:   t_ptr->p1 = randint(12) + 12; break;
      default:
	break;
      }
    break;

  case TV_STAFF:
    switch(t_ptr->subval) {
      case 0:	  t_ptr->p1 = randint(20) +	 12; break;
      case 1:	  t_ptr->p1 = randint(8)  +	 6; break;
      case 2:	  t_ptr->p1 = randint(5)  +	 6; break;
      case 3:	  t_ptr->p1 = randint(20) +	 12; break;
      case 4:	  t_ptr->p1 = randint(15) +	 6; break;
      case 5:	  t_ptr->p1 = randint(4)  +	 5; break;
      case 6:	  t_ptr->p1 = randint(5)  +	 3; break;
      case 7:	  t_ptr->p1 = randint(3)  +	 1;
	t_ptr->level = 10;
	break;
      case 8:	  t_ptr->p1 = randint(3)  +	 1; break;
      case 9:	  t_ptr->p1 = randint(5)  +	 6; break;
      case 10:   t_ptr->p1 = randint(10) +	 12; break;
      case 11:   t_ptr->p1 = randint(5)  +	 6; break;
      case 12:   t_ptr->p1 = randint(5)  +	 6; break;
      case 13:   t_ptr->p1 = randint(5)  +	 6; break;
      case 14:   t_ptr->p1 = randint(10) +	 12; break;
      case 15:   t_ptr->p1 = randint(3)  +	 4; break;
      case 16:   t_ptr->p1 = randint(5)  +	 6; break;
      case 17:   t_ptr->p1 = randint(5)  +	 6; break;
      case 18:   t_ptr->p1 = randint(3)  +	 4; break;
      case 19:   t_ptr->p1 = randint(10) +	 12; break;
      case 20:   t_ptr->p1 = randint(3)  +	 4; break;
      case 21:   t_ptr->p1 = randint(3)  +	 4; break;
      case 22:   t_ptr->p1 = randint(10) + 6;
	t_ptr->level = 5;
	break;
      default:
	break;
      }
    break;

  case TV_CLOAK:
    if (magik(chance)) {
	if (magik(special)) {
	    if (randint(2) == 1) {
		t_ptr->name2 = SN_PROTECTION;
		t_ptr->toac += m_bonus(2, 40, level);
		t_ptr->cost += 250;
	      }
	    else {
		t_ptr->toac += m_bonus(1, 20, level);
		t_ptr->ident |= ID_SHOW_P1;
		t_ptr->p1 = randint(3);
		t_ptr->flags |= TR_STEALTH;
		t_ptr->name2 = SN_STEALTH;
		t_ptr->cost += 500;
	      }
	  }
	else
	  t_ptr->toac += m_bonus(1, 20, level);
      }
    else if (magik(cursed)) {
	tmp = randint(3);
	if (tmp == 1) {
	    t_ptr->flags |= TR_AGGRAVATE;
	    t_ptr->name2 = SN_IRRITATION;
	    t_ptr->toac  -= m_bonus(1, 10, level);
	    t_ptr->ident |= ID_SHOW_HITDAM;
	    t_ptr->tohit -= m_bonus(1, 10, level);
	    t_ptr->todam -= m_bonus(1, 10, level);
	    t_ptr->cost =  0;
	  }
	else if (tmp == 2) {
	    t_ptr->name2 = SN_VULNERABILITY;
	    t_ptr->toac -= m_bonus(10, 100, level+50);
	    t_ptr->cost = 0;
	  }
	else {
	    t_ptr->name2 = SN_ENVELOPING;
	    t_ptr->toac  -= m_bonus(1, 10, level);
	    t_ptr->ident |= ID_SHOW_HITDAM;
	    t_ptr->tohit -= m_bonus(2, 40, level+10);
	    t_ptr->todam -= m_bonus(2, 40, level+10);
	    t_ptr->cost = 0;
	  }
	t_ptr->flags |= TR_CURSED;
      }
    break;

  case TV_CHEST:
    switch(randint(level+4)) {
      case 1:
	t_ptr->flags = 0;
	t_ptr->name2 = SN_EMPTY;
	break;
      case 2:
	t_ptr->flags |= CH_LOCKED;
	t_ptr->name2 = SN_LOCKED;
	break;
      case 3: case 4:
	t_ptr->flags |= (CH_LOSE_STR|CH_LOCKED);
	t_ptr->name2 = SN_POISON_NEEDLE;
	break;
      case 5: case 6:
	t_ptr->flags |= (CH_POISON|CH_LOCKED);
	t_ptr->name2 = SN_POISON_NEEDLE;
	break;
      case 7: case 8: case 9:
	t_ptr->flags |= (CH_PARALYSED|CH_LOCKED);
	t_ptr->name2 = SN_GAS_TRAP;
	break;
      case 10: case 11:
	t_ptr->flags |= (CH_EXPLODE|CH_LOCKED);
	t_ptr->name2 = SN_EXPLOSION_DEVICE;
	break;
      case 12: case 13: case 14:
	t_ptr->flags |= (CH_SUMMON|CH_LOCKED);
	t_ptr->name2 = SN_SUMMONING_RUNES;
	break;
      case 15: case 16: case 17:
	t_ptr->flags |= (CH_PARALYSED|CH_POISON|CH_LOSE_STR|CH_LOCKED);
	t_ptr->name2 = SN_MULTIPLE_TRAPS;
	break;
      default:
	t_ptr->flags |= (CH_SUMMON|CH_EXPLODE|CH_LOCKED);
	t_ptr->name2 = SN_MULTIPLE_TRAPS;
	break;
      }
    break;

  case TV_SLING_AMMO: case TV_SPIKE:
  case TV_BOLT: case TV_ARROW:
    if (t_ptr->tval == TV_SLING_AMMO || t_ptr->tval == TV_BOLT
	|| t_ptr->tval == TV_ARROW) {
	/* always show tohit/todam values if identified */
	t_ptr->ident |= ID_SHOW_HITDAM;

	if (magik(chance)) {
	    t_ptr->tohit += m_bonus(1, 35, level);
	    t_ptr->todam += m_bonus(1, 35, level);
	    /* see comment for weapons */
	    if (magik(3*special/2))
	      switch(randint(10))
		{
		case 1: case 2: case 3:
		  t_ptr->name2 = SN_SLAYING;
		  t_ptr->tohit += 5;
		  t_ptr->todam += 5;
		  t_ptr->cost += 20;
		  break;
		case 4: case 5:
		  t_ptr->flags |= TR_FLAME_TONGUE;
		  t_ptr->tohit += 2;
		  t_ptr->todam += 4;
		  t_ptr->name2 = SN_FIRE;
		  t_ptr->cost += 25;
		  break;
		case 6: case 7:
		  t_ptr->flags |= TR_SLAY_EVIL;
		  t_ptr->tohit += 3;
		  t_ptr->todam += 3;
		  t_ptr->name2 = SN_SLAY_EVIL;
		  t_ptr->cost += 25;
		  break;
		case 8: case 9:
		  t_ptr->flags |= TR_SLAY_ANIMAL;
		  t_ptr->tohit += 2;
		  t_ptr->todam += 2;
		  t_ptr->name2 = SN_SLAY_ANIMAL;
		  t_ptr->cost += 30;
		  break;
		case 10:
		  t_ptr->flags |= TR_SLAY_DRAGON;
		  t_ptr->tohit += 3;
		  t_ptr->todam += 3;
		  t_ptr->name2 = SN_DRAGON_SLAYING;
		  t_ptr->cost += 35;
		  break;
		}
	  }
	else if (magik(cursed)) {
	    t_ptr->tohit -= m_bonus(5, 55, level);
	    t_ptr->todam -= m_bonus(5, 55, level);
	    t_ptr->flags |= TR_CURSED;
	    t_ptr->cost = 0;
	  }
      }

    t_ptr->number = 0;
    for (i = 0; i < 7; i++)
      t_ptr->number += randint(6);
    if (missile_ctr == MAX_SHORT)
      missile_ctr = -MAX_SHORT - 1;
    else
      missile_ctr++;
    t_ptr->p1 = missile_ctr;
    break;

  case TV_FOOD:
    /* make sure all food rations have the same level */
    if (t_ptr->subval == 90)
      t_ptr->level = 0;
    /* give all elvish waybread the same level */
    else if (t_ptr->subval == 92)
      t_ptr->level = 6;
    break;

  case TV_SCROLL1:
    /* give all identify scrolls the same level */
    if (t_ptr->subval == 67)
      t_ptr->level = 1;
    /* scroll of light */
    else if (t_ptr->subval == 69)
      t_ptr->level = 0;
    /* scroll of trap detection */
    else if (t_ptr->subval == 80)
      t_ptr->level = 5;
    /* scroll of door/stair location */
    else if (t_ptr->subval == 81)
      t_ptr->level = 5;
    break;

  case TV_POTION1:  /* potions */
    /* cure light */
    if (t_ptr->subval == 76)
      t_ptr->level = 0;
    break;

  default:
    break;
  }

  // now copy it.  I hope this works.
  DmWrite(t_list, x*sizeof(inven_type), t_ptr, sizeof(inven_type));

}


/* Set or unset various boolean options.		-CJS- */
// This should be done by a popup form!
void set_options()
{
  return;
}
