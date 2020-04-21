/*********************************************************************
 * iLarn - Larn adapted for the PalmPilot.                           *
 * Copyright (C) 2000 Bridget Spitznagel                             *
 * iLarn is derived from Ularn by Phil Cordier                       *
 * which in turn was based on Larn by Noah Morgan                    *
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
//#include <Globals.h>
#include "fix_db.h"

/*****************************************************************************
 *                                                                           *
 *                      store.......                                         *
 *                                                                           *
 *****************************************************************************/

//#define MAXITEM 92

//struct _itm itm[92] = {
Short itm[5*93] = {
  //cost	 iven name	iven arg      how
  // gp	 	 iven[]		ivenarg[]     many
  0,0,0,0,0, // don't ask me why, but it hates the first set.
  2,	OLEATHER,	0,	3,      3,
  10,	OSTUDLEATHER,	0,	2,      2,
  40,	ORING,		0,	2,      2,
 85,	OCHAIN,		0,	2,      2,
 220,	OSPLINT,	0,	1,      1,
 400,	OPLATE,		0,	1,      1,
 900,	OPLATEARMOR,	0,	1,      1,
 2600,	OSSPLATE,	0,	1,      1,
 150,	OSHIELD,	0,	1,      1,
 5000, 	OELVENCHAIN,    0,      0,      0,
 1000, 	OORB,		0,	0,      0,
 10000,	OSLAYER,	0,	0,      0,
  		
  //cost	iven name	iven arg   how
  //gp	 	  iven[]	ivenarg[]  many
  		
 2,	ODAGGER,	0,	3,      3,
 20,	OSPEAR,		0,	3,      3,
 80,	OFLAIL,		0,	2,      2,
 150,	OBATTLEAXE,	0,	2,      2,
 450,	OLONGSWORD,	0,	2,      2,
 1000,	O2SWORD,	0,	2,      2,
 5000,	OSWORD,		0,	1,      1,
 20000,	OLANCE,		0,	1,      1,
 2000,	OSWORDofSLASHING,0,	0,      0,
 7500, 	OHAMMER,	0,	0,      0,
  		
  //cost	iven name	iven arg   how
  //gp	 	  iven[]	ivenarg[]  many
 150,	OPROTRING,	1,	1,      1,
 85,	OSTRRING,	1,	1,      1,
 120,	ODEXRING,	1,	1,      1,
 120,	OCLEVERRING,	1,	1,      1,
 180,	OENERGYRING,	0,	1,      1,
 125,	ODAMRING,	0,	1,      1,
 220,	OREGENRING,	0,	1,      1,
 1000,	ORINGOFEXTRA,	0,	1,      1,
  				          
 280,	OBELT,		0,	1,      1,
 400,	OAMULET,	5,	1,      1,
  				          
 500,	OCUBEofUNDEAD,	0,	0,      0,
 600,	ONOTHEFT,	0,	0,      0,
  				          
 590,	OCHEST,		3,	1,      1,
 200,	OBOOK,		2,	1,      1,
 10,	OCOOKIE,	0,	3,      3,
 666,  	OHANDofFEAR,    0,	0,      0,
  		
  //cost		iven name	iven arg   how
  //gp	 		  iven[]	ivenarg[]  many 
  
 20,   OPOTION,		0,	6,      6,
 90,   OPOTION,		1,	5,      5,
 520,  OPOTION,		2,	1,      1,
 100,  OPOTION,		3,	2,      2,
 50,   OPOTION,		4,	2,      2,
 150,  OPOTION,		5,	2,      2,
 70,   OPOTION,		6,	1,      1,
 30,   OPOTION,		7,	7,      7,
 200,  OPOTION,		8,	1,      1,
 50,   OPOTION,		9,	1,      1,
 80,   OPOTION,		10,	1,      1,
  
  //cost		iven name	iven arg   how
  //gp	 		  iven[]	ivenarg[]  many 
  
 30,   	OPOTION,	11,	3,     3, 
 20, 	OPOTION,	12,	5,     5, 
 40,   	OPOTION,	13,	3,     3, 
 35,   	OPOTION,	14,	2,     2, 
 520,  	OPOTION,	15,	1,     1, 
 90,   	OPOTION,	16,	2,     2, 
 200,  	OPOTION,	17,	2,     2, 
 220,  	OPOTION,	18,	4,     4, 
 80,  	OPOTION,	19,	6,     6, 
 370,  	OPOTION,	20,	3,     3, 
 50,   	OPOTION,	22,	1,     1, 
 150,  	OPOTION,	23,	15,     15,
  
  //cost	iven name	iven arg   how
  //gp	 	  iven[]	ivenarg[]  many 
  		
 2750, 	OORBOFDRAGON,	0,	0,      0,
 750,  	OSPIRITSCARAB,	0,	0,      0,
 8000, 	OVORPAL,	0,	0,      0,
  				          
 100,  	OSCROLL,	0,	2,      2,
 125,  	OSCROLL,	1,	2,      2,
 60,   	OSCROLL,	2,	4,      4,
 10,   	OSCROLL,	3,	4,      4,
 100,  	OSCROLL,	4,	3,      3,
 200,  	OSCROLL,	5,	2,      2,
 110,  	OSCROLL,	6,	1,      1,
 500,  	OSCROLL,	7,	2,      2,
 200,  	OSCROLL,	8,	2,      2,
 250,  	OSCROLL,	9,	4,      4,
 20,  	OSCROLL,	10,	5,     5,
 30,   	OSCROLL,	11,	3,     3,
  
  //cost		iven name	iven arg   how
  //gp			  iven[]	ivenarg[]  many 
  
 340,  OSCROLL,		12,	1,     1,
 340,  OSCROLL,		13,	1,     1,
 300,  OSCROLL,		14,	2,     2,
 400,  OSCROLL,		15,	2,     2,
 500,  OSCROLL,		16,	2,     2,
 1000, OSCROLL,		17,	1,     1,
 500,  OSCROLL,		18,	1,     1,
 340,  OSCROLL,		19,	2,     2,
 220,  OSCROLL,		20,	3,     3,
 3900, OSCROLL,		21,	0,     0,
 610,  OSCROLL,		22,	1,     1,
 3000, OSCROLL,		23,	0,     0,
 300,  OSPHTALISMAN,   	0,	0,     0,
 150,  OWWAND,		0,	0,     0,
 50,   OBRASSLAMP,	0,	0,     0,
 9500, OPSTAFF,		0,	0,     0 
};

void make_dndstore_rec(DmOpenRef iLarnDB)
{
  Err error;
  VoidHand fooRec;
  Ptr p;
  Int i, offset, size;
  UInt index;
  ULong uniqueID = REC_DNDSTORE;

  index = 0; // DNDSTORE_RECORD == 0
  offset = 0;
  size = sizeof(itm);  //  size = sizeof(struct _itm) * MAXITEM;

  fooRec = DmNewRecord(iLarnDB, &index, size);
  p = MemHandleLock(fooRec);
  //  for (i = 0 ; i < MAXITEM ; i++) {
  //    error = DmWrite(p, offset, &(itm[i]), sizeof(struct _itm));
  //    offset += sizeof(struct _itm);
  //  }
  for (i = 5 ; i < 93*5 ; i++) {
    error = DmWrite(p, offset, itm+i, sizeof(Short));
    offset += sizeof(Short);
  }
  //  error = DmWrite(p, offset, &itm, sizeof(itm));
  MemPtrUnlock(p);
  DmReleaseRecord(iLarnDB, index, true);
  DmSetRecordInfo(iLarnDB, index, NULL, &uniqueID); // set its uid!
}
