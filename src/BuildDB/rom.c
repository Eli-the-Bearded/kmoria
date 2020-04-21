#include <Pilot.h>
void DrawSerialNumOrMessage(int x, int y, CharPtr noNumberMessage)
{
  CharPtr bufP;
  Word bufLen;
  Word retval;
  Short count;
  Byte checkSum;
  char checksumStr[2]; 
  // holds the dash and the checksum digit

  retval = SysGetROMToken (0, sysROMTokenSnum, 
			   (BytePtr*) &bufP, &bufLen);
  if ((!retval) && (bufP) && ((Byte) *bufP != 0xFF)) { 
    // there's a valid serial number!
    // Calculate the checksum: Start with zero, add each digit,
    // then rotate the result one bit to the left and repeat.
    checkSum = 0;
    for (count=0; count<bufLen; count++) {
      checkSum += bufP[count];
      checkSum = (checkSum<<1) | ((checkSum & 0x80) >> 7);
    }
    // Add the two hex digits (nibbles) together, +2 
    // (range: 2 - 31 ==> 2-9, A-W)
    // By adding 2 to the result before converting to ascii,
    // we eliminate the numbers 0 and 1, which can be
    // difficult to distinguish from the letters O and I.
    checkSum = ((checkSum>>4) & 0x0F) + (checkSum & 0x0F) + 2;

    // draw the serial number and find out how wide it was
    WinDrawChars(bufP, bufLen, x, y);
    x += FntCharsWidth(bufP, bufLen);

    // draw the dash and the checksum digit right after it
    checksumStr[0] = '-';
    checksumStr[1] = 
      ((checkSum < 10) ? (checkSum +'0'):(checkSum -10 +'A'));
    WinDrawChars (checksumStr, 2, x, y);
  }
  else // there's no serial number
    // draw a status message if the caller provided one
    if (noNumberMessage)
      WinDrawChars(noNumberMessage, StrLen(noNumberMessage),x, y);
}
