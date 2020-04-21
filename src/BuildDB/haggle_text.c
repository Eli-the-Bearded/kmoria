#include <Pilot.h>
#include "kMoria.h"

extern DmOpenRef MoriaDB;

// Oh, what the hell, I'll just make each string-array a record.

#define HAGGLE_LEN 52
#define HAGGLE_QTY 76
// 76 strings, max length is 52, I will pack them however.
Char haggle_text[76][52] = {
//Char comment1[14][44] = {
  "Done!",  "Accepted!",  "Fine.",  "Agreed!",  "Ok.",  "Taken!",
  "You drive a hard bargain, but taken.",
  "You'll force me bankrupt, but it's a deal.",  "Sigh.  I'll take it.",
  "My poor sick children may starve, but done!",  "Finally!  I accept.",
  "Robbed again.",  "A pleasure to do business with you!",
  "My spouse will skin me, but accepted.",
  //Char comment2a[3][44] = {
  "%A2 is my final offer; take it or leave it.",
  "I'll give you no more than %A2.",
  "My patience grows thin.  %A2 is final.",
  //Char comment2b[16][52] = {
  "%A1 for such a fine item?  HA!  No less than %A2.",
  "%A1 is an insult!  Try %A2 gold pieces.",
  "%A1?!?  You would rob my poor starving children?",
  "Why, I'll take no less than %A2 gold pieces.",
  "Ha!  No less than %A2 gold pieces.",
  "Thou knave!  No less than %A2 gold pieces.",
  "%A1 is far too little, how about %A2?",
  "I paid more than %A1 for it myself, try %A2.",
  "%A1?  Are you mad?!?  How about %A2 gold pieces?",
  "As scrap this would bring %A1.  Try %A2 in gold.",
  "May the fleas of 1000 orcs molest you.  I want %A2.",
  "My mother you can get for %A1, this costs %A2.",
  "May your chickens grow lips.  I want %A2 in gold!",
  "Sell this for such a pittance?  Give me %A2 gold.",
  "May the Balrog find you tasty!  %A2 gold pieces?",
  "Your mother was a Troll!  %A2 or I'll tell.",
  //Char comment3a[3][48] = {
  "I'll pay no more than %A1; take it or leave it.",
  "You'll get no more than %A1 from me.",
  "%A1 and that's final.",
  //Char comment3b[15][48] = {
  "%A2 for that piece of junk?  No more than %A1.",
  "For %A2 I could own ten of those.  Try %A1.",
  "%A2?  NEVER!  %A1 is more like it.",
  "Let's be reasonable. How about %A1 gold pieces?",
  "%A1 gold for that junk, no more.",
  "%A1 gold pieces and be thankful for it!",
  "%A1 gold pieces and not a copper more.",
  "%A2 gold?  HA!  %A1 is more like it.",  "Try about %A1 gold.",
  "I wouldn't pay %A2 for your children, try %A1.",
  "*CHOKE* For that!?  Let's say %A1.",  "How about %A1?",
  "That looks war surplus!  Say %A1 gold.",
  "I'll buy it as scrap for %A1.",
  "%A2 is too much, let us say %A1 gold.",
  //Char comment4a[5][48] = {
  "ENOUGH!  You have abused me once too often!",
  "THAT DOES IT!  You shall waste my time no more!",
  "This is getting nowhere.  I'm going home!",
  "BAH!  No more shall you insult me!",
  "Begone!  I have had enough abuse for one day.",
  //Char comment4b[5][32] = {
  "Out of my place!",  "out... Out... OUT!!!",  "Come back tomorrow.",
  "Leave my place.  Begone!",  "Come back when thou art richer.",
  //Char comment5[10][38] = {
  "You will have to do better than that!",  "That's an insult!",
  "Do you wish to do business or not?",  "Hah!  Try again.",
  "Ridiculous!",  "You've got to be kidding!",  "You'd better be kidding!",
  "You try my patience.",  "I don't hear you.",
  "Hmmm, nice weather we're having.",
  //Char comment6[5][29] = {
  "I must have heard you wrong.",  "What was that?",
  "I'm sorry, say that again.",  "What did you say?",
  "Sorry, what was that again?"
};

Short qtys[9]  = {14,  3, 16,  3, 15,  5,  5, 10,  5};
Short sizes[9] = {44, 44, 52, 48, 48, 48, 32, 38, 29};
// Actually I'll make it simple. 1 size fits all

void newrec_haggle_texts()
{
  ULong uniqueID;
  VoidHand rec;
  Ptr p;
  ULong size, offset;
  UInt index;
  Short i, str_offset;
  ULong qty = HAGGLE_QTY;

  // First print a record containing the offsets of the strings
  uniqueID = REC_HAGGLE;
  if (0 == DmFindRecordByID(MoriaDB, uniqueID, &index))
    DmRemoveRecord(MoriaDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaDB);
  size = qty * sizeof(Short);
  rec = DmNewRecord(MoriaDB, &index, size);
  p = MemHandleLock(rec);
  // print stuff to it.
  for (i = 0, str_offset = 0 ; i < qty ; i++) {
    DmWrite(p, i * sizeof(Short), &str_offset, sizeof(Short));
    str_offset += StrLen(haggle_text[i]) + 1;
  }
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaDB, index, true);
  DmSetRecordInfo(MoriaDB, index, NULL, &uniqueID); // set its uid!
  
  uniqueID = REC_HAGGLE_TEXT;
  // qty remains the same
  if (0 == DmFindRecordByID(MoriaDB, uniqueID, &index))
    DmRemoveRecord(MoriaDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaDB);
  size = str_offset * sizeof(Char); // handy to have it already calculated..
  rec = DmNewRecord(MoriaDB, &index, size);
  p = MemHandleLock(rec);
  // print stuff to it.
  for (i = 0, str_offset = 0 ; i < qty; i++) {
    DmStrCopy(p, str_offset, haggle_text[i]);
    str_offset += StrLen(haggle_text[i]) + 1;    
  }
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaDB, index, true);
  DmSetRecordInfo(MoriaDB, index, NULL, &uniqueID); // set its uid!

}
