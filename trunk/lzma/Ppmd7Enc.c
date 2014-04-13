/* Ppmd7Enc.c -- PPMdH Encoder
2010-03-12 : Igor Pavlov : Public domain
This code is based on PPMd var.H (2001): Dmitry Shkarin : Public domain */

#include "Ppmd7.h"

#define kTopValue (1 << 24)

void Ppmd7z_RangeEnc_Init(CPpmd7z_RangeEnc *p)
{
  p->Low = 0;
  p->Range = 0xFFFFFFFF;
  p->Cache = 0;
  p->CacheSize = 1;
}

static void RangeEnc_ShiftLow(CPpmd7z_RangeEnc *p)
{
  if ((UInt32_7z)p->Low < (UInt32_7z)0xFF000000 || (unsigned)(p->Low >> 32) != 0)
  {
    Byte temp = p->Cache;
    do
    {
      p->Stream->Write(p->Stream, (Byte)(temp + (Byte)(p->Low >> 32)));
      temp = 0xFF;
    }
    while(--p->CacheSize != 0);
    p->Cache = (Byte)((UInt32_7z)p->Low >> 24);
  }
  p->CacheSize++;
  p->Low = (UInt32_7z)p->Low << 8;
}

static void RangeEnc_Encode(CPpmd7z_RangeEnc *p, UInt32_7z start, UInt32_7z size, UInt32_7z total)
{
  p->Low += start * (p->Range /= total);
  p->Range *= size;
  while (p->Range < kTopValue)
  {
    p->Range <<= 8;
    RangeEnc_ShiftLow(p);
  }
}

static void RangeEnc_EncodeBit_0(CPpmd7z_RangeEnc *p, UInt32_7z size0)
{
  p->Range = (p->Range >> 14) * size0;
  while (p->Range < kTopValue)
  {
    p->Range <<= 8;
    RangeEnc_ShiftLow(p);
  }
}

static void RangeEnc_EncodeBit_1(CPpmd7z_RangeEnc *p, UInt32_7z size0)
{
  UInt32_7z newBound = (p->Range >> 14) * size0;
  p->Low += newBound;
  p->Range -= newBound;
  while (p->Range < kTopValue)
  {
    p->Range <<= 8;
    RangeEnc_ShiftLow(p);
  }
}

void Ppmd7z_RangeEnc_FlushData(CPpmd7z_RangeEnc *p)
{
  unsigned i;
  for (i = 0; i < 5; i++)
    RangeEnc_ShiftLow(p);
}


#define MASK(sym) ((signed char *)charMask)[sym]

void Ppmd7_EncodeSymbol(CPpmd7 *p, CPpmd7z_RangeEnc *rc, int symbol)
{
  size_t charMask[256 / sizeof(size_t)];
  if (p->MinContext->NumStats != 1)
  {
    CPpmd_State *s = Ppmd7_GetStats(p, p->MinContext);
    UInt32_7z sum;
    unsigned i;
    if (s->Symbol == symbol)
    {
      RangeEnc_Encode(rc, 0, s->Freq, p->MinContext->SummFreq);
      p->FoundState = s;
      Ppmd7_Update1_0(p);
      return;
    }
    p->PrevSuccess = 0;
    sum = s->Freq;
    i = p->MinContext->NumStats - 1;
    do
    {
      if ((++s)->Symbol == symbol)
      {
        RangeEnc_Encode(rc, sum, s->Freq, p->MinContext->SummFreq);
        p->FoundState = s;
        Ppmd7_Update1(p);
        return;
      }
      sum += s->Freq;
    }
    while (--i);
    
    p->HiBitsFlag = p->HB2Flag[p->FoundState->Symbol];
    PPMD_SetAllBitsIn256Bytes(charMask);
    MASK(s->Symbol) = 0;
    i = p->MinContext->NumStats - 1;
    do { MASK((--s)->Symbol) = 0; } while (--i);
    RangeEnc_Encode(rc, sum, p->MinContext->SummFreq - sum, p->MinContext->SummFreq);
  }
  else
  {
    UInt16 *prob = Ppmd7_GetBinSumm(p);
    CPpmd_State *s = Ppmd7Context_OneState(p->MinContext);
    if (s->Symbol == symbol)
    {
      RangeEnc_EncodeBit_0(rc, *prob);
      *prob = (UInt16)PPMD_UPDATE_PROB_0(*prob);
      p->FoundState = s;
      Ppmd7_UpdateBin(p);
      return;
    }
    else
    {
      RangeEnc_EncodeBit_1(rc, *prob);
      *prob = (UInt16)PPMD_UPDATE_PROB_1(*prob);
      p->InitEsc = PPMD7_kExpEscape[*prob >> 10];
      PPMD_SetAllBitsIn256Bytes(charMask);
      MASK(s->Symbol) = 0;
      p->PrevSuccess = 0;
    }
  }
  for (;;)
  {
    UInt32_7z escFreq;
    CPpmd_See *see;
    CPpmd_State *s;
    UInt32_7z sum;
    unsigned i, numMasked = p->MinContext->NumStats;
    do
    {
      p->OrderFall++;
      if (!p->MinContext->Suffix)
        return; /* EndMarker (symbol = -1) */
      p->MinContext = Ppmd7_GetContext(p, p->MinContext->Suffix);
    }
    while (p->MinContext->NumStats == numMasked);
    
    see = Ppmd7_MakeEscFreq(p, numMasked, &escFreq);
    s = Ppmd7_GetStats(p, p->MinContext);
    sum = 0;
    i = p->MinContext->NumStats;
    do
    {
      int cur = s->Symbol;
      if (cur == symbol)
      {
        UInt32_7z low = sum;
        CPpmd_State *s1 = s;
        do
        {
          sum += (s->Freq & (int)(MASK(s->Symbol)));
          s++;
        }
        while (--i);
        RangeEnc_Encode(rc, low, s1->Freq, sum + escFreq);
        Ppmd_See_Update(see);
        p->FoundState = s1;
        Ppmd7_Update2(p);
        return;
      }
      sum += (s->Freq & (int)(MASK(cur)));
      MASK(cur) = 0;
      s++;
    }
    while (--i);
    
    RangeEnc_Encode(rc, sum, escFreq, sum + escFreq);
    see->Summ = (UInt16)(see->Summ + sum + escFreq);
  }
}
