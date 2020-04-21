#include <Pilot.h>
#include "kMoria.h"

extern DmOpenRef MoriaDB;
Short t_level[MAX_OBJ_LEVEL+1] = {
    24,  55,  69,  81,  86, 111, 116, 138, 142, 145, 163, 164,
   181, 183, 190, 203, 205, 207, 209, 210, 230, 230, 231, 231,
   234, 248, 250, 250, 252, 252, 265, 265, 265, 265, 266, 270,
   271, 271, 272, 272, 300, 300, 301, 301, 309, 313, 314, 316,
   318, 319, 344,
};
Short sorted_objects[MAX_DUNGEON_OBJ] = {
   343, 342, 341, 336, 335, 334, 255, 247, 246, 245, 237, 224,
   223, 222, 193, 192, 181,  82,  54,  51,  32,  30,  29,  21,
   340, 339, 338, 337, 333, 332, 268, 264, 262, 261, 259, 257,
   250, 249, 238, 217, 208, 201, 185, 184, 176, 124, 123, 102,
    91,  86,  85,  84,  79,  31,  24, 292, 282, 278, 277, 276,
   275, 269, 239, 103,  94,  81,  80,  43,  28, 267, 263, 258,
   248, 240, 226, 182, 126, 104,  83,  74,  53,  95,  92,  70,
    47,  35, 317, 308, 307, 297, 296, 293, 290, 265, 241, 220,
   210, 202, 197, 194, 190, 189, 188, 177, 105,  68,  59,  49,
    48,  36,  22,  93,  58,  25,  12,   8, 326, 279, 183, 180,
   154, 153, 150, 145, 143, 142, 139, 138, 137, 107, 106,  96,
    39,  33,  13,   9,   3,   0, 195, 127,  61,   5,  37,   2,
     1, 306, 305, 304, 300, 295, 294, 254, 198, 186, 178, 129,
   108,  75,  69,  57,  23,  14,   6,  66, 281, 273, 242, 218,
   205, 204, 203, 196, 175, 174, 173, 125, 109,  55,  45,  26,
     7,  72,   4, 167, 166, 165, 141, 140, 110,  34, 327, 284,
   270, 216, 199, 144, 128,  77,  67,  62,  60,  16,  10, 169,
    20,  73,  63,  44,  17,  18, 313, 303, 298, 286, 285, 274,
   271, 187, 164, 163, 152, 151, 130,  97,  89,  87,  64,  52,
    27,  11,  65, 219, 168, 111, 328, 314, 283, 266, 251, 243,
   235, 234, 232, 231, 229, 228, 225, 170, 113, 112, 114,  19,
   272, 209, 179, 135, 134, 133, 132, 131, 115,  78,  71,  38,
    15, 116, 329, 287, 207,  40, 117, 118, 325, 324, 323, 322,
   321, 320, 319, 318, 310, 309, 299, 288, 260, 256, 253, 252,
   236, 233, 230, 227, 221, 211, 206, 155,  98,  90,  76,  41,
   119, 161, 160, 159, 158, 157, 156, 120,  99, 330,  56,  46,
    42, 311, 312, 100, 291, 121, 315, 331, 316, 302, 301, 289,
   280, 244, 215, 214, 213, 212, 200, 191, 172, 171, 162, 149,
   148, 147, 146, 136, 122, 101,  88,  50,
};

Short m_level[MAX_MONS_LEVEL+1] = {
     8,  19,  34,  49,  59,  73,  81,  90,  96, 105, 113, 118,
   128, 135, 143, 153, 160, 164, 167, 174, 178, 182, 187, 193,
   197, 202, 208, 212, 215, 221, 227, 231, 235, 239, 244, 250,
   256, 261, 265, 270, 277,
};

void newrec_tlevel()
{
  ULong uniqueID;
  VoidHand rec;
  Ptr p;
  ULong size, offset;
  UInt index;
  Short i;
  ULong qty = MAX_OBJ_LEVEL+1;
  ULong sze = sizeof(Short);

  uniqueID = REC_TLEVEL;
  if (0 == DmFindRecordByID(MoriaDB, uniqueID, &index))
    DmRemoveRecord(MoriaDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaDB);
  size = qty * sze;
  rec = DmNewRecord(MoriaDB, &index, size);
  p = MemHandleLock(rec);
  // print stuff to it.
  for (i = 0, offset = 0 ; i < qty; i++, offset += sze)
    DmWrite(p, offset, &(t_level[i]), sze);
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaDB, index, true);
  DmSetRecordInfo(MoriaDB, index, NULL, &uniqueID); // set its uid!
}

void newrec_sortedobjects()
{
  ULong uniqueID;
  VoidHand rec;
  Ptr p;
  ULong size, offset;
  UInt index;
  Short i;
  ULong qty = MAX_DUNGEON_OBJ;
  ULong sze = sizeof(Short);

  uniqueID = REC_SORTEDOBJ;
  if (0 == DmFindRecordByID(MoriaDB, uniqueID, &index))
    DmRemoveRecord(MoriaDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaDB);
  size = qty * sze;
  rec = DmNewRecord(MoriaDB, &index, size);
  p = MemHandleLock(rec);
  // print stuff to it.
  for (i = 0, offset = 0 ; i < qty; i++, offset += sze)
    DmWrite(p, offset, &(sorted_objects[i]), sze);
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaDB, index, true);
  DmSetRecordInfo(MoriaDB, index, NULL, &uniqueID); // set its uid!
}

void newrec_mlevel()
{
  ULong uniqueID;
  VoidHand rec;
  Ptr p;
  ULong size, offset;
  UInt index;
  Short i;
  ULong qty = MAX_MONS_LEVEL+1;
  ULong sze = sizeof(Short);

  uniqueID = REC_MLEVEL;
  if (0 == DmFindRecordByID(MoriaDB, uniqueID, &index))
    DmRemoveRecord(MoriaDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaDB);
  size = qty * sze;
  rec = DmNewRecord(MoriaDB, &index, size);
  p = MemHandleLock(rec);
  // print stuff to it.
  for (i = 0, offset = 0 ; i < qty; i++, offset += sze)
    DmWrite(p, offset, &(m_level[i]), sze);
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaDB, index, true);
  DmSetRecordInfo(MoriaDB, index, NULL, &uniqueID); // set its uid!
}
