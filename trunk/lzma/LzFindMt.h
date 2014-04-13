/* LzFindMt.h -- multithreaded Match finder for LZ algorithms
2009-02-07 : Igor Pavlov : Public domain */

#ifndef __LZ_FIND_MT_H
#define __LZ_FIND_MT_H

#include "LzFind.h"
#include "Threads.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kMtHashBlockSize (1 << 13)
#define kMtHashNumBlocks (1 << 3)
#define kMtHashNumBlocksMask (kMtHashNumBlocks - 1)

#define kMtBtBlockSize (1 << 14)
#define kMtBtNumBlocks (1 << 6)
#define kMtBtNumBlocksMask (kMtBtNumBlocks - 1)

typedef struct _CMtSync
{
  Bool7z wasCreated;
  Bool7z needStart;
  Bool7z exit;
  Bool7z stopWriting;

  CThread thread;
  CAutoResetEvent canStart;
  CAutoResetEvent wasStarted;
  CAutoResetEvent wasStopped;
  CSemaphore freeSemaphore;
  CSemaphore filledSemaphore;
  Bool7z csWasInitialized;
  Bool7z csWasEntered;
  CCriticalSection cs;
  UInt32_7z numProcessedBlocks;
} CMtSync;

typedef UInt32_7z * (*Mf_Mix_Matches)(void *p, UInt32_7z matchMinPos, UInt32_7z *distances);

/* kMtCacheLineDummy must be >= size_of_CPU_cache_line */
#define kMtCacheLineDummy 128

typedef void (*Mf_GetHeads)(const Byte *buffer, UInt32_7z pos,
  UInt32_7z *hash, UInt32_7z hashMask, UInt32_7z *heads, UInt32_7z numHeads, const UInt32_7z *crc);

typedef struct _CMatchFinderMt
{
  /* LZ */
  const Byte *pointerToCurPos;
  UInt32_7z *btBuf;
  UInt32_7z btBufPos;
  UInt32_7z btBufPosLimit;
  UInt32_7z lzPos;
  UInt32_7z btNumAvailBytes;

  UInt32_7z *hash;
  UInt32_7z fixedHashSize;
  UInt32_7z historySize;
  const UInt32_7z *crc;

  Mf_Mix_Matches MixMatchesFunc;
  
  /* LZ + BT */
  CMtSync btSync;
  Byte btDummy[kMtCacheLineDummy];

  /* BT */
  UInt32_7z *hashBuf;
  UInt32_7z hashBufPos;
  UInt32_7z hashBufPosLimit;
  UInt32_7z hashNumAvail;

  CLzRef *son;
  UInt32_7z matchMaxLen;
  UInt32_7z numHashBytes;
  UInt32_7z pos;
  Byte *buffer;
  UInt32_7z cyclicBufferPos;
  UInt32_7z cyclicBufferSize; /* it must be historySize + 1 */
  UInt32_7z cutValue;

  /* BT + Hash */
  CMtSync hashSync;
  /* Byte hashDummy[kMtCacheLineDummy]; */
  
  /* Hash */
  Mf_GetHeads GetHeadsFunc;
  CMatchFinder *MatchFinder;
} CMatchFinderMt;

void MatchFinderMt_Construct(CMatchFinderMt *p);
void MatchFinderMt_Destruct(CMatchFinderMt *p, ISzAlloc *alloc);
SRes MatchFinderMt_Create(CMatchFinderMt *p, UInt32_7z historySize, UInt32_7z keepAddBufferBefore,
    UInt32_7z matchMaxLen, UInt32_7z keepAddBufferAfter, ISzAlloc *alloc);
void MatchFinderMt_CreateVTable(CMatchFinderMt *p, IMatchFinder *vTable);
void MatchFinderMt_ReleaseStream(CMatchFinderMt *p);

#ifdef __cplusplus
}
#endif

#endif
