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


Boolean is_room(UChar element)
{
  return ((element == DARK_FLOOR) || (element == LIGHT_FLOOR));
}

Boolean is_corr(UChar element)
{
  return (element == CORR_FLOOR || element == BLOCKED_FLOOR);
}

Boolean is_floor(UChar element)
{
  return (element <= MAX_CAVE_FLOOR);
}

Boolean is_corrodes(inven_type *item)
{
  switch(item->tval) {
    case TV_SWORD: case TV_HELM: case TV_SHIELD: case TV_HARD_ARMOR:
    case TV_WAND:
      return true;
    }
  return false;
}

Boolean is_flammable(inven_type *item)
{
  switch(item->tval) {
  case TV_ARROW: case TV_BOW: case TV_HAFTED: case TV_POLEARM:
  case TV_BOOTS: case TV_GLOVES: case TV_CLOAK: case TV_SOFT_ARMOR:
    /* Items of (RF) should not be destroyed.  */
    if (item->flags & TR_RES_FIRE)
      return false;
    else
      return true;

  case TV_STAFF: case TV_SCROLL1: case TV_SCROLL2:
    return true;
  }
  return false;
}

Boolean is_frost_destroy(inven_type *item)
{
  if ((item->tval == TV_POTION1) || (item->tval == TV_POTION2)
      || (item->tval == TV_FLASK))
    return true;
  return false;
}

Boolean is_acid_affect(inven_type *item)
{
  switch(item->tval) {
  case TV_MISC: case TV_CHEST:
    return true;
  case TV_BOLT: case TV_ARROW:
  case TV_BOW: case TV_HAFTED: case TV_POLEARM: case TV_BOOTS:
  case TV_GLOVES: case TV_CLOAK: case TV_SOFT_ARMOR:
    if (item->flags & TR_RES_ACID)
      return false;
    else
      return true;
  }
  return false;
}

Boolean is_lightning_destroy(inven_type *item)
{
  if ((item->tval == TV_RING) || (item->tval == TV_WAND)
      || (item->tval == TV_SPIKE))
    return true;
  else
    return false;
}

Boolean is_null(inven_type *item)
{
  return false;
}

Boolean is_acid_destroy(inven_type *item)
{
  switch(item->tval) {
  case TV_ARROW: case TV_BOW: case TV_HAFTED: case TV_POLEARM:
  case TV_BOOTS: case TV_GLOVES: case TV_CLOAK: case TV_HELM:
  case TV_SHIELD: case TV_HARD_ARMOR: case TV_SOFT_ARMOR:
    if (item->flags & TR_RES_ACID)
      return false;
    else
      return true;
  case TV_STAFF:
  case TV_SCROLL1: case TV_SCROLL2: case TV_FOOD: case TV_OPEN_DOOR:
  case TV_CLOSED_DOOR:
    return true;
  }
  return false;
}

Boolean is_fire_destroy(inven_type *item)
{
  switch(item->tval) {
  case TV_ARROW: case TV_BOW: case TV_HAFTED: case TV_POLEARM:
  case TV_BOOTS: case TV_GLOVES: case TV_CLOAK: case TV_SOFT_ARMOR:
    if (item->flags & TR_RES_FIRE)
      return false;
    else
      return true;
  case TV_STAFF: case TV_SCROLL1: case TV_SCROLL2: case TV_POTION1:
  case TV_POTION2: case TV_FLASK: case TV_FOOD: case TV_OPEN_DOOR:
  case TV_CLOSED_DOOR:
    return true;
  }
  return false;
}

/* Items too large to fit in chests 	-DJG- */
/* Use treasure_type since item not yet created */
Boolean is_large(treasure_type *item)
{
  switch(item->tval) {
  case TV_CHEST:      case TV_BOW:        case TV_POLEARM: 
  case TV_HARD_ARMOR: case TV_SOFT_ARMOR: case TV_STAFF:
    return true;
  case TV_HAFTED: case TV_SWORD: case TV_DIGGING:
    if (item->weight > 150)
      return true;
    else
      return false;
  default:
    return false;
  }
}


Boolean is_general_store(UChar element)
{
  switch(element) {
  case TV_DIGGING: case TV_BOOTS: case TV_CLOAK: case TV_FOOD:
  case TV_FLASK: case TV_LIGHT: case TV_SPIKE:
    return true;
  }
  return false;
}


Boolean is_armory(UChar element)
{
  switch(element) {
  case TV_BOOTS: case TV_GLOVES: case TV_HELM: case TV_SHIELD:
  case TV_HARD_ARMOR: case TV_SOFT_ARMOR:
    return true;
  }
  return false;
}

Boolean is_weaponsmith(UChar element)
{
  switch(element) {
  case TV_SLING_AMMO: case TV_BOLT: case TV_ARROW: case TV_BOW:
  case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
    return true;
  }
  return false;
}

Boolean is_temple(UChar element)
{
  switch(element) {
  case TV_HAFTED: case TV_SCROLL1: case TV_SCROLL2: case TV_POTION1:
  case TV_POTION2: case TV_PRAYER_BOOK:
    return true;
  }
  return false;
}

Boolean is_alchemist(UChar element)
{
  switch(element) {
  case TV_SCROLL1: case TV_SCROLL2: case TV_POTION1: case TV_POTION2:
    return true;
  }
  return false;
}

Boolean is_magic_shop(UChar element)
{
  switch(element) {
  case TV_AMULET: case TV_RING: case TV_STAFF: case TV_WAND:
  case TV_SCROLL1: case TV_SCROLL2: case TV_POTION1: case TV_POTION2:
  case TV_MAGIC_BOOK:
    return true;
  }
  return false;
}

Boolean is_store_buy(Short storeno, UChar element)
{
  switch (storeno) {
  case 0: return is_general_store(element);
  case 1: return is_armory(element);
  case 2: return is_weaponsmith(element);
  case 3: return is_temple(element);
  case 4: return is_alchemist(element);
  case 5: return is_magic_shop(element);
  }
  return false;
}
