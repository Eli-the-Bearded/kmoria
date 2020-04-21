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
#ifndef __KMORIA_GLOBALS_H__
#define __KMORIA_GLOBALS_H__


// Yes, I have registered this Creator ID.  Yay.
#define MoriaAppType 'Mria'
#define MoriaDBType  'Data'
#define MoriaSaveDBType  'Data'
#define MoriaAppID   'Mria'
extern DmOpenRef       MoriaDB;
extern DmOpenRef       MoriaSaveDB;
#define MoriaDBName "kMoriaDB"
#define MoriaSaveDBName "kMoriaSaveDB"

#define MoriaAppPrefID 0x00
#define MoriaAppPrefVersion 0x01


// also do some includes, perhaps.
#include "sections.h"
#include "constant.h"
#include "types.h"
#include "prototypes.h"
#include "records.h"

#endif
