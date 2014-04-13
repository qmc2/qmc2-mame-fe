/* LzFind.h -- Match finder for LZ algorithms
2009-04-22 : Igor Pavlov : Public domain */

#ifndef __LZ_FIND_H
#define __LZ_FIND_H

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

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

  UInt32_7z matchMaxLen;
  CLzRef *hash;
  CLzRef *son;
  UInt32_7z hashMask;
  UInt32_7z cutValue;

  Byte *bufferBase;
  ISeqInStream *stream;
  int streamEndWasReached;

  UInt32_7z blockSize;
  UInt32_7z keepSizeBefore;
  UInt32_7z keepSizeAfter;

  UInt32_7z numHashBytes;
  int directInput;
  size_t directInputRem;
  int btMode;
  int bigHash;
  UInt32_7z historySize;
  UInt32_7z fixedHashSize;
  UInt32_7z hashSizeSum;
  UInt32_7z numSons;
  SRes result;
  UInt32_7z crc[256];
} CMatchFinder;

#define Inline_MatchFinder_GetPointerToCurrentPos(p) ((p)->buffer)
#define Inline_MatchFinder_GetIndexByte(p, index) ((p)->buffer[(Int32)(index)])

#define Inline_MatchFinder_GetNumAvailableBytes(p) ((p)->streamPos - (p)->pos)

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
void MatchFinder_Normalize3(UInt32_7z subValue, CLzRef *items, UInt32_7z numItems);
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
typedef Byte (*Mf_GetIndexByte_Func)(void *object, Int32 index);
typedef UInt32_7z (*Mf_GetNumAvailableBytes_Func)(void *object);
typedef const Byte * (*Mf_GetPointerToCurrentPos_Func)(void *object);
typedef UInt32_7z (*Mf_GetMatches_Func)(void *object, UInt32_7z *distances);
typedef void (*Mf_Skip_Func)(void *object, UInt32_7z);

typedef struct _IMatchFinder
{
  Mf_Init_Func Init;
  Mf_GetIndexByte_Func GetIndexByte;
  Mf_GetNumAvailableBytes_Func GetNumAvailableBytes;
  Mf_GetPointerToCurrentPos_Func GetPointerToCurrentPos;
  Mf_GetMatches_Func GetMatches;
  Mf_Skip_Func Skip;
} IMatchFinder;

void MatchFinder_CreateVTable(CMatchFinder *p, IMatchFinder *vTable);

void MatchFinder_Init(CMatchFinder *p);
UInt32_7z Bt3Zip_MatchFinder_GetMatches(CMatchFinder *p, UInt32_7z *distances);
UInt32_7z Hc3Zip_MatchFinder_GetMatches(CMatchFinder *p, UInt32_7z *distances);
void Bt3Zip_MatchFinder_Skip(CMatchFinder *p, UInt32_7z num);
void Hc3Zip_MatchFinder_Skip(CMatchFinder *p, UInt32_7z num);

#ifdef __cplusplus
}
#endif

#endif
