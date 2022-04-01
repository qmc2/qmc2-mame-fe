/* LzFind.h -- Match finder for LZ algorithms
2015-10-15 : Igor Pavlov : Public domain */

#ifndef __LZ_FIND_H
#define __LZ_FIND_H

#include "7zTypes.h"

EXTERN_C_BEGIN

typedef UInt32_7z CLzRef;

typedef struct _CMatchFinder
{
  Byte *buffer;
  UInt32_7z pos;
  UInt32_7z posLimit;
  UInt32_7z streamPos;
  UInt32_7z lenLimit;

  UInt32_7z cyclicBufferPos;
  UInt32_7z cyclicBufferSize; /* it must be = (historySize + 1) */

  Byte streamEndWasReached;
  Byte btMode;
  Byte bigHash;
  Byte directInput;

  UInt32_7z matchMaxLen;
  CLzRef *hash;
  CLzRef *son;
  UInt32_7z hashMask;
  UInt32_7z cutValue;

  Byte *bufferBase;
  ISeqInStream *stream;
  
  UInt32_7z blockSize;
  UInt32_7z keepSizeBefore;
  UInt32_7z keepSizeAfter;

  UInt32_7z numHashBytes;
  size_t directInputRem;
  UInt32_7z historySize;
  UInt32_7z fixedHashSize;
  UInt32_7z hashSizeSum;
  SRes result;
  UInt32_7z crc[256];
  size_t numRefs;
} CMatchFinder;

#define Inline_MatchFinder_GetPointerToCurrentPos(p) ((p)->buffer)

#define Inline_MatchFinder_GetNumAvailableBytes(p) ((p)->streamPos - (p)->pos)

#define Inline_MatchFinder_IsFinishedOK(p) \
    ((p)->streamEndWasReached \
        && (p)->streamPos == (p)->pos \
        && (!(p)->directInput || (p)->directInputRem == 0))
      
int MatchFinder_NeedMove(CMatchFinder *p);
Byte *MatchFinder_GetPointerToCurrentPos(CMatchFinder *p);
void MatchFinder_MoveBlock(CMatchFinder *p);
void MatchFinder_ReadIfRequired(CMatchFinder *p);

void MatchFinder_Construct(CMatchFinder *p);

/* Conditions:
     historySize <= 3 GB
     keepAddBufferBefore + matchMaxLen + keepAddBufferAfter < 511MB
*/
int MatchFinder_Create(CMatchFinder *p, UInt32_7z historySize,
    UInt32_7z keepAddBufferBefore, UInt32_7z matchMaxLen, UInt32_7z keepAddBufferAfter,
    ISzAlloc *alloc);
void MatchFinder_Free(CMatchFinder *p, ISzAlloc *alloc);
void MatchFinder_Normalize3(UInt32_7z subValue, CLzRef *items, size_t numItems);
void MatchFinder_ReduceOffsets(CMatchFinder *p, UInt32_7z subValue);

UInt32_7z * GetMatchesSpec1(UInt32_7z lenLimit, UInt32_7z curMatch, UInt32_7z pos, const Byte *buffer, CLzRef *son,
    UInt32_7z _cyclicBufferPos, UInt32_7z _cyclicBufferSize, UInt32_7z _cutValue,
    UInt32_7z *distances, UInt32_7z maxLen);

/*
Conditions:
  Mf_GetNumAvailableBytes_Func must be called before each Mf_GetMatchLen_Func.
  Mf_GetPointerToCurrentPos_Func's result must be used only before any other function
*/

typedef void (*Mf_Init_Func)(void *object);
typedef UInt32_7z (*Mf_GetNumAvailableBytes_Func)(void *object);
typedef const Byte * (*Mf_GetPointerToCurrentPos_Func)(void *object);
typedef UInt32_7z (*Mf_GetMatches_Func)(void *object, UInt32_7z *distances);
typedef void (*Mf_Skip_Func)(void *object, UInt32_7z);

typedef struct _IMatchFinder
{
  Mf_Init_Func Init;
  Mf_GetNumAvailableBytes_Func GetNumAvailableBytes;
  Mf_GetPointerToCurrentPos_Func GetPointerToCurrentPos;
  Mf_GetMatches_Func GetMatches;
  Mf_Skip_Func Skip;
} IMatchFinder;

void MatchFinder_CreateVTable(CMatchFinder *p, IMatchFinder *vTable);

void MatchFinder_Init_2(CMatchFinder *p, int readData);
void MatchFinder_Init(CMatchFinder *p);

UInt32_7z Bt3Zip_MatchFinder_GetMatches(CMatchFinder *p, UInt32_7z *distances);
UInt32_7z Hc3Zip_MatchFinder_GetMatches(CMatchFinder *p, UInt32_7z *distances);

void Bt3Zip_MatchFinder_Skip(CMatchFinder *p, UInt32_7z num);
void Hc3Zip_MatchFinder_Skip(CMatchFinder *p, UInt32_7z num);

EXTERN_C_END

#endif
