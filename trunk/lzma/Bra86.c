/* Bra86.c -- Converter for x86 code (BCJ)
2008-10-04 : Igor Pavlov : Public domain */

#include "Bra.h"

#define Test86MSByte(b) ((b) == 0 || (b) == 0xFF)

const Byte kMaskToAllowedStatus[8] = {1, 1, 1, 0, 1, 0, 0, 0};
const Byte kMaskToBitNumber[8] = {0, 1, 2, 2, 3, 3, 3, 3};

SizeT x86_Convert(Byte *data, SizeT size, UInt32_7z ip, UInt32_7z *state, int encoding)
{
  SizeT bufferPos = 0, prevPosT;
  UInt32_7z prevMask = *state & 0x7;
  if (size < 5)
    return 0;
  ip += 5;
  prevPosT = (SizeT)0 - 1;

  for (;;)
  {
    Byte *p = data + bufferPos;
    Byte *limit = data + size - 4;
    for (; p < limit; p++)
      if ((*p & 0xFE) == 0xE8)
        break;
    bufferPos = (SizeT)(p - data);
    if (p >= limit)
      break;
    prevPosT = bufferPos - prevPosT;
    if (prevPosT > 3)
      prevMask = 0;
    else
    {
      prevMask = (prevMask << ((int)prevPosT - 1)) & 0x7;
      if (prevMask != 0)
      {
        Byte b = p[4 - kMaskToBitNumber[prevMask]];
        if (!kMaskToAllowedStatus[prevMask] || Test86MSByte(b))
        {
          prevPosT = bufferPos;
          prevMask = ((prevMask << 1) & 0x7) | 1;
          bufferPos++;
          continue;
        }
      }
    }
    prevPosT = bufferPos;

    if (Test86MSByte(p[4]))
    {
      UInt32_7z src = ((UInt32_7z)p[4] << 24) | ((UInt32_7z)p[3] << 16) | ((UInt32_7z)p[2] << 8) | ((UInt32_7z)p[1]);
      UInt32_7z dest;
      for (;;)
      {
        Byte b;
        int index;
        if (encoding)
          dest = (ip + (UInt32_7z)bufferPos) + src;
        else
          dest = src - (ip + (UInt32_7z)bufferPos);
        if (prevMask == 0)
          break;
        index = kMaskToBitNumber[prevMask] * 8;
        b = (Byte)(dest >> (24 - index));
        if (!Test86MSByte(b))
          break;
        src = dest ^ ((1 << (32 - index)) - 1);
      }
      p[4] = (Byte)(~(((dest >> 24) & 1) - 1));
      p[3] = (Byte)(dest >> 16);
      p[2] = (Byte)(dest >> 8);
      p[1] = (Byte)dest;
      bufferPos += 5;
    }
    else
    {
      prevMask = ((prevMask << 1) & 0x7) | 1;
      bufferPos++;
    }
  }
  prevPosT = bufferPos - prevPosT;
  *state = ((prevPosT > 3) ? 0 : ((prevMask << ((int)prevPosT - 1)) & 0x7));
  return bufferPos;
}
