/* LzFindMt.c -- multithreaded Match finder for LZ algorithms
2015-10-15 : Igor Pavlov : Public domain */

#include "Precomp.h"

#include "LzHash.h"

#include "LzFindMt.h"

static void MtSync_Construct(CMtSync *p)
{
  p->wasCreated = False;
  p->csWasInitialized = False;
  p->csWasEntered = False;
  Thread_Construct(&p->thread);
  Event_Construct(&p->canStart);
  Event_Construct(&p->wasStarted);
  Event_Construct(&p->wasStopped);
  Semaphore_Construct(&p->freeSemaphore);
  Semaphore_Construct(&p->filledSemaphore);
}

static void MtSync_GetNextBlock(CMtSync *p)
{
  if (p->needStart)
  {
    p->numProcessedBlocks = 1;
    p->needStart = False;
    p->stopWriting = False;
    p->exit = False;
    Event_Reset(&p->wasStarted);
    Event_Reset(&p->wasStopped);

    Event_Set(&p->canStart);
    Event_Wait(&p->wasStarted);
  }
  else
  {
    CriticalSection_Leave(&p->cs);
    p->csWasEntered = False;
    p->numProcessedBlocks++;
    Semaphore_Release1(&p->freeSemaphore);
  }
  Semaphore_Wait(&p->filledSemaphore);
  CriticalSection_Enter(&p->cs);
  p->csWasEntered = True;
}

/* MtSync_StopWriting must be called if Writing was started */

static void MtSync_StopWriting(CMtSync *p)
{
  UInt32_7z myNumBlocks = p->numProcessedBlocks;
  if (!Thread_WasCreated(&p->thread) || p->needStart)
    return;
  p->stopWriting = True;
  if (p->csWasEntered)
  {
    CriticalSection_Leave(&p->cs);
    p->csWasEntered = False;
  }
  Semaphore_Release1(&p->freeSemaphore);
 
  Event_Wait(&p->wasStopped);

  while (myNumBlocks++ != p->numProcessedBlocks)
  {
    Semaphore_Wait(&p->filledSemaphore);
    Semaphore_Release1(&p->freeSemaphore);
  }
  p->needStart = True;
}

static void MtSync_Destruct(CMtSync *p)
{
  if (Thread_WasCreated(&p->thread))
  {
    MtSync_StopWriting(p);
    p->exit = True;
    if (p->needStart)
      Event_Set(&p->canStart);
    Thread_Wait(&p->thread);
    Thread_Close(&p->thread);
  }
  if (p->csWasInitialized)
  {
    CriticalSection_Delete(&p->cs);
    p->csWasInitialized = False;
  }

  Event_Close(&p->canStart);
  Event_Close(&p->wasStarted);
  Event_Close(&p->wasStopped);
  Semaphore_Close(&p->freeSemaphore);
  Semaphore_Close(&p->filledSemaphore);

  p->wasCreated = False;
}

#define RINOK_THREAD(x) { if ((x) != 0) return SZ_ERROR_THREAD; }

static SRes MtSync_Create2(CMtSync *p, THREAD_FUNC_TYPE startAddress, void *obj, UInt32_7z numBlocks)
{
  if (p->wasCreated)
    return SZ_OK;

  RINOK_THREAD(CriticalSection_Init(&p->cs));
  p->csWasInitialized = True;

  RINOK_THREAD(AutoResetEvent_CreateNotSignaled(&p->canStart));
  RINOK_THREAD(AutoResetEvent_CreateNotSignaled(&p->wasStarted));
  RINOK_THREAD(AutoResetEvent_CreateNotSignaled(&p->wasStopped));
  
  RINOK_THREAD(Semaphore_Create(&p->freeSemaphore, numBlocks, numBlocks));
  RINOK_THREAD(Semaphore_Create(&p->filledSemaphore, 0, numBlocks));

  p->needStart = True;
  
  RINOK_THREAD(Thread_Create(&p->thread, startAddress, obj));
  p->wasCreated = True;
  return SZ_OK;
}

static SRes MtSync_Create(CMtSync *p, THREAD_FUNC_TYPE startAddress, void *obj, UInt32_7z numBlocks)
{
  SRes res = MtSync_Create2(p, startAddress, obj, numBlocks);
  if (res != SZ_OK)
    MtSync_Destruct(p);
  return res;
}

void MtSync_Init(CMtSync *p) { p->needStart = True; }

#define kMtMaxValForNormalize 0xFFFFFFFF

#define DEF_GetHeads2(name, v, action) \
  static void GetHeads ## name(const Byte *p, UInt32_7z pos, \
      UInt32_7z *hash, UInt32_7z hashMask, UInt32_7z *heads, UInt32_7z numHeads, const UInt32_7z *crc) \
    { action; for (; numHeads != 0; numHeads--) { \
      const UInt32_7z value = (v); p++; *heads++ = pos - hash[value]; hash[value] = pos++;  } }

#define DEF_GetHeads(name, v) DEF_GetHeads2(name, v, ;)

DEF_GetHeads2(2,  (p[0] | ((UInt32_7z)p[1] << 8)), UNUSED_VAR(hashMask); UNUSED_VAR(crc); )
DEF_GetHeads(3,  (crc[p[0]] ^ p[1] ^ ((UInt32_7z)p[2] << 8)) & hashMask)
DEF_GetHeads(4,  (crc[p[0]] ^ p[1] ^ ((UInt32_7z)p[2] << 8) ^ (crc[p[3]] << 5)) & hashMask)
DEF_GetHeads(4b, (crc[p[0]] ^ p[1] ^ ((UInt32_7z)p[2] << 8) ^ ((UInt32_7z)p[3] << 16)) & hashMask)
/* DEF_GetHeads(5,  (crc[p[0]] ^ p[1] ^ ((UInt32_7z)p[2] << 8) ^ (crc[p[3]] << 5) ^ (crc[p[4]] << 3)) & hashMask) */

static void HashThreadFunc(CMatchFinderMt *mt)
{
  CMtSync *p = &mt->hashSync;
  for (;;)
  {
    UInt32_7z numProcessedBlocks = 0;
    Event_Wait(&p->canStart);
    Event_Set(&p->wasStarted);
    for (;;)
    {
      if (p->exit)
        return;
      if (p->stopWriting)
      {
        p->numProcessedBlocks = numProcessedBlocks;
        Event_Set(&p->wasStopped);
        break;
      }

      {
        CMatchFinder *mf = mt->MatchFinder;
        if (MatchFinder_NeedMove(mf))
        {
          CriticalSection_Enter(&mt->btSync.cs);
          CriticalSection_Enter(&mt->hashSync.cs);
          {
            const Byte *beforePtr = Inline_MatchFinder_GetPointerToCurrentPos(mf);
            ptrdiff_t offset;
            MatchFinder_MoveBlock(mf);
            offset = beforePtr - Inline_MatchFinder_GetPointerToCurrentPos(mf);
            mt->pointerToCurPos -= offset;
            mt->buffer -= offset;
          }
          CriticalSection_Leave(&mt->btSync.cs);
          CriticalSection_Leave(&mt->hashSync.cs);
          continue;
        }

        Semaphore_Wait(&p->freeSemaphore);

        MatchFinder_ReadIfRequired(mf);
        if (mf->pos > (kMtMaxValForNormalize - kMtHashBlockSize))
        {
          UInt32_7z subValue = (mf->pos - mf->historySize - 1);
          MatchFinder_ReduceOffsets(mf, subValue);
          MatchFinder_Normalize3(subValue, mf->hash + mf->fixedHashSize, (size_t)mf->hashMask + 1);
        }
        {
          UInt32_7z *heads = mt->hashBuf + ((numProcessedBlocks++) & kMtHashNumBlocksMask) * kMtHashBlockSize;
          UInt32_7z num = mf->streamPos - mf->pos;
          heads[0] = 2;
          heads[1] = num;
          if (num >= mf->numHashBytes)
          {
            num = num - mf->numHashBytes + 1;
            if (num > kMtHashBlockSize - 2)
              num = kMtHashBlockSize - 2;
            mt->GetHeadsFunc(mf->buffer, mf->pos, mf->hash + mf->fixedHashSize, mf->hashMask, heads + 2, num, mf->crc);
            heads[0] += num;
          }
          mf->pos += num;
          mf->buffer += num;
        }
      }

      Semaphore_Release1(&p->filledSemaphore);
    }
  }
}

static void MatchFinderMt_GetNextBlock_Hash(CMatchFinderMt *p)
{
  MtSync_GetNextBlock(&p->hashSync);
  p->hashBufPosLimit = p->hashBufPos = ((p->hashSync.numProcessedBlocks - 1) & kMtHashNumBlocksMask) * kMtHashBlockSize;
  p->hashBufPosLimit += p->hashBuf[p->hashBufPos++];
  p->hashNumAvail = p->hashBuf[p->hashBufPos++];
}

#define kEmptyHashValue 0

/* #define MFMT_GM_INLINE */

#ifdef MFMT_GM_INLINE

#define NO_INLINE MY_FAST_CALL

static Int32 NO_INLINE GetMatchesSpecN(UInt32_7z lenLimit, UInt32_7z pos, const Byte *cur, CLzRef *son,
    UInt32_7z _cyclicBufferPos, UInt32_7z _cyclicBufferSize, UInt32_7z _cutValue,
    UInt32_7z *_distances, UInt32_7z _maxLen, const UInt32_7z *hash, Int32 limit, UInt32_7z size, UInt32_7z *posRes)
{
  do
  {
  UInt32_7z *distances = _distances + 1;
  UInt32_7z curMatch = pos - *hash++;

  CLzRef *ptr0 = son + (_cyclicBufferPos << 1) + 1;
  CLzRef *ptr1 = son + (_cyclicBufferPos << 1);
  UInt32_7z len0 = 0, len1 = 0;
  UInt32_7z cutValue = _cutValue;
  UInt32_7z maxLen = _maxLen;
  for (;;)
  {
    UInt32_7z delta = pos - curMatch;
    if (cutValue-- == 0 || delta >= _cyclicBufferSize)
    {
      *ptr0 = *ptr1 = kEmptyHashValue;
      break;
    }
    {
      CLzRef *pair = son + ((_cyclicBufferPos - delta + ((delta > _cyclicBufferPos) ? _cyclicBufferSize : 0)) << 1);
      const Byte *pb = cur - delta;
      UInt32_7z len = (len0 < len1 ? len0 : len1);
      if (pb[len] == cur[len])
      {
        if (++len != lenLimit && pb[len] == cur[len])
          while (++len != lenLimit)
            if (pb[len] != cur[len])
              break;
        if (maxLen < len)
        {
          *distances++ = maxLen = len;
          *distances++ = delta - 1;
          if (len == lenLimit)
          {
            *ptr1 = pair[0];
            *ptr0 = pair[1];
            break;
          }
        }
      }
      if (pb[len] < cur[len])
      {
        *ptr1 = curMatch;
        ptr1 = pair + 1;
        curMatch = *ptr1;
        len1 = len;
      }
      else
      {
        *ptr0 = curMatch;
        ptr0 = pair;
        curMatch = *ptr0;
        len0 = len;
      }
    }
  }
  pos++;
  _cyclicBufferPos++;
  cur++;
  {
    UInt32_7z num = (UInt32_7z)(distances - _distances);
    *_distances = num - 1;
    _distances += num;
    limit -= num;
  }
  }
  while (limit > 0 && --size != 0);
  *posRes = pos;
  return limit;
}

#endif

static void BtGetMatches(CMatchFinderMt *p, UInt32_7z *distances)
{
  UInt32_7z numProcessed = 0;
  UInt32_7z curPos = 2;
  UInt32_7z limit = kMtBtBlockSize - (p->matchMaxLen * 2);
  
  distances[1] = p->hashNumAvail;
  
  while (curPos < limit)
  {
    if (p->hashBufPos == p->hashBufPosLimit)
    {
      MatchFinderMt_GetNextBlock_Hash(p);
      distances[1] = numProcessed + p->hashNumAvail;
      if (p->hashNumAvail >= p->numHashBytes)
        continue;
      distances[0] = curPos + p->hashNumAvail;
      distances += curPos;
      for (; p->hashNumAvail != 0; p->hashNumAvail--)
        *distances++ = 0;
      return;
    }
    {
      UInt32_7z size = p->hashBufPosLimit - p->hashBufPos;
      UInt32_7z lenLimit = p->matchMaxLen;
      UInt32_7z pos = p->pos;
      UInt32_7z cyclicBufferPos = p->cyclicBufferPos;
      if (lenLimit >= p->hashNumAvail)
        lenLimit = p->hashNumAvail;
      {
        UInt32_7z size2 = p->hashNumAvail - lenLimit + 1;
        if (size2 < size)
          size = size2;
        size2 = p->cyclicBufferSize - cyclicBufferPos;
        if (size2 < size)
          size = size2;
      }
      
      #ifndef MFMT_GM_INLINE
      while (curPos < limit && size-- != 0)
      {
        UInt32_7z *startDistances = distances + curPos;
        UInt32_7z num = (UInt32_7z)(GetMatchesSpec1(lenLimit, pos - p->hashBuf[p->hashBufPos++],
            pos, p->buffer, p->son, cyclicBufferPos, p->cyclicBufferSize, p->cutValue,
            startDistances + 1, p->numHashBytes - 1) - startDistances);
        *startDistances = num - 1;
        curPos += num;
        cyclicBufferPos++;
        pos++;
        p->buffer++;
      }
      #else
      {
        UInt32_7z posRes;
        curPos = limit - GetMatchesSpecN(lenLimit, pos, p->buffer, p->son, cyclicBufferPos, p->cyclicBufferSize, p->cutValue,
            distances + curPos, p->numHashBytes - 1, p->hashBuf + p->hashBufPos, (Int32)(limit - curPos), size, &posRes);
        p->hashBufPos += posRes - pos;
        cyclicBufferPos += posRes - pos;
        p->buffer += posRes - pos;
        pos = posRes;
      }
      #endif

      numProcessed += pos - p->pos;
      p->hashNumAvail -= pos - p->pos;
      p->pos = pos;
      if (cyclicBufferPos == p->cyclicBufferSize)
        cyclicBufferPos = 0;
      p->cyclicBufferPos = cyclicBufferPos;
    }
  }
  
  distances[0] = curPos;
}

static void BtFillBlock(CMatchFinderMt *p, UInt32_7z globalBlockIndex)
{
  CMtSync *sync = &p->hashSync;
  if (!sync->needStart)
  {
    CriticalSection_Enter(&sync->cs);
    sync->csWasEntered = True;
  }
  
  BtGetMatches(p, p->btBuf + (globalBlockIndex & kMtBtNumBlocksMask) * kMtBtBlockSize);

  if (p->pos > kMtMaxValForNormalize - kMtBtBlockSize)
  {
    UInt32_7z subValue = p->pos - p->cyclicBufferSize;
    MatchFinder_Normalize3(subValue, p->son, (size_t)p->cyclicBufferSize * 2);
    p->pos -= subValue;
  }

  if (!sync->needStart)
  {
    CriticalSection_Leave(&sync->cs);
    sync->csWasEntered = False;
  }
}

void BtThreadFunc(CMatchFinderMt *mt)
{
  CMtSync *p = &mt->btSync;
  for (;;)
  {
    UInt32_7z blockIndex = 0;
    Event_Wait(&p->canStart);
    Event_Set(&p->wasStarted);
    for (;;)
    {
      if (p->exit)
        return;
      if (p->stopWriting)
      {
        p->numProcessedBlocks = blockIndex;
        MtSync_StopWriting(&mt->hashSync);
        Event_Set(&p->wasStopped);
        break;
      }
      Semaphore_Wait(&p->freeSemaphore);
      BtFillBlock(mt, blockIndex++);
      Semaphore_Release1(&p->filledSemaphore);
    }
  }
}

void MatchFinderMt_Construct(CMatchFinderMt *p)
{
  p->hashBuf = NULL;
  MtSync_Construct(&p->hashSync);
  MtSync_Construct(&p->btSync);
}

static void MatchFinderMt_FreeMem(CMatchFinderMt *p, ISzAlloc *alloc)
{
  alloc->Free(alloc, p->hashBuf);
  p->hashBuf = NULL;
}

void MatchFinderMt_Destruct(CMatchFinderMt *p, ISzAlloc *alloc)
{
  MtSync_Destruct(&p->hashSync);
  MtSync_Destruct(&p->btSync);
  MatchFinderMt_FreeMem(p, alloc);
}

#define kHashBufferSize (kMtHashBlockSize * kMtHashNumBlocks)
#define kBtBufferSize (kMtBtBlockSize * kMtBtNumBlocks)

static THREAD_FUNC_RET_TYPE THREAD_FUNC_CALL_TYPE HashThreadFunc2(void *p) { HashThreadFunc((CMatchFinderMt *)p);  return 0; }
static THREAD_FUNC_RET_TYPE THREAD_FUNC_CALL_TYPE BtThreadFunc2(void *p)
{
  Byte allocaDummy[0x180];
  unsigned i = 0;
  for (i = 0; i < 16; i++)
    allocaDummy[i] = (Byte)0;
  if (allocaDummy[0] == 0)
    BtThreadFunc((CMatchFinderMt *)p);
  return 0;
}

SRes MatchFinderMt_Create(CMatchFinderMt *p, UInt32_7z historySize, UInt32_7z keepAddBufferBefore,
    UInt32_7z matchMaxLen, UInt32_7z keepAddBufferAfter, ISzAlloc *alloc)
{
  CMatchFinder *mf = p->MatchFinder;
  p->historySize = historySize;
  if (kMtBtBlockSize <= matchMaxLen * 4)
    return SZ_ERROR_PARAM;
  if (!p->hashBuf)
  {
    p->hashBuf = (UInt32_7z *)alloc->Alloc(alloc, (kHashBufferSize + kBtBufferSize) * sizeof(UInt32_7z));
    if (!p->hashBuf)
      return SZ_ERROR_MEM;
    p->btBuf = p->hashBuf + kHashBufferSize;
  }
  keepAddBufferBefore += (kHashBufferSize + kBtBufferSize);
  keepAddBufferAfter += kMtHashBlockSize;
  if (!MatchFinder_Create(mf, historySize, keepAddBufferBefore, matchMaxLen, keepAddBufferAfter, alloc))
    return SZ_ERROR_MEM;

  RINOK(MtSync_Create(&p->hashSync, HashThreadFunc2, p, kMtHashNumBlocks));
  RINOK(MtSync_Create(&p->btSync, BtThreadFunc2, p, kMtBtNumBlocks));
  return SZ_OK;
}

/* Call it after ReleaseStream / SetStream */
void MatchFinderMt_Init(CMatchFinderMt *p)
{
  CMatchFinder *mf = p->MatchFinder;
  p->btBufPos = p->btBufPosLimit = 0;
  p->hashBufPos = p->hashBufPosLimit = 0;

  /* Init without data reading. We don't want to read data in this thread */
  MatchFinder_Init_2(mf, False);
  
  p->pointerToCurPos = Inline_MatchFinder_GetPointerToCurrentPos(mf);
  p->btNumAvailBytes = 0;
  p->lzPos = p->historySize + 1;

  p->hash = mf->hash;
  p->fixedHashSize = mf->fixedHashSize;
  p->crc = mf->crc;

  p->son = mf->son;
  p->matchMaxLen = mf->matchMaxLen;
  p->numHashBytes = mf->numHashBytes;
  p->pos = mf->pos;
  p->buffer = mf->buffer;
  p->cyclicBufferPos = mf->cyclicBufferPos;
  p->cyclicBufferSize = mf->cyclicBufferSize;
  p->cutValue = mf->cutValue;
}

/* ReleaseStream is required to finish multithreading */
void MatchFinderMt_ReleaseStream(CMatchFinderMt *p)
{
  MtSync_StopWriting(&p->btSync);
  /* p->MatchFinder->ReleaseStream(); */
}

static void MatchFinderMt_Normalize(CMatchFinderMt *p)
{
  MatchFinder_Normalize3(p->lzPos - p->historySize - 1, p->hash, p->fixedHashSize);
  p->lzPos = p->historySize + 1;
}

static void MatchFinderMt_GetNextBlock_Bt(CMatchFinderMt *p)
{
  UInt32_7z blockIndex;
  MtSync_GetNextBlock(&p->btSync);
  blockIndex = ((p->btSync.numProcessedBlocks - 1) & kMtBtNumBlocksMask);
  p->btBufPosLimit = p->btBufPos = blockIndex * kMtBtBlockSize;
  p->btBufPosLimit += p->btBuf[p->btBufPos++];
  p->btNumAvailBytes = p->btBuf[p->btBufPos++];
  if (p->lzPos >= kMtMaxValForNormalize - kMtBtBlockSize)
    MatchFinderMt_Normalize(p);
}

static const Byte * MatchFinderMt_GetPointerToCurrentPos(CMatchFinderMt *p)
{
  return p->pointerToCurPos;
}

#define GET_NEXT_BLOCK_IF_REQUIRED if (p->btBufPos == p->btBufPosLimit) MatchFinderMt_GetNextBlock_Bt(p);

static UInt32_7z MatchFinderMt_GetNumAvailableBytes(CMatchFinderMt *p)
{
  GET_NEXT_BLOCK_IF_REQUIRED;
  return p->btNumAvailBytes;
}

static UInt32_7z * MixMatches2(CMatchFinderMt *p, UInt32_7z matchMinPos, UInt32_7z *distances)
{
  UInt32_7z h2, curMatch2;
  UInt32_7z *hash = p->hash;
  const Byte *cur = p->pointerToCurPos;
  UInt32_7z lzPos = p->lzPos;
  MT_HASH2_CALC
      
  curMatch2 = hash[h2];
  hash[h2] = lzPos;

  if (curMatch2 >= matchMinPos)
    if (cur[(ptrdiff_t)curMatch2 - lzPos] == cur[0])
    {
      *distances++ = 2;
      *distances++ = lzPos - curMatch2 - 1;
    }
  
  return distances;
}

static UInt32_7z * MixMatches3(CMatchFinderMt *p, UInt32_7z matchMinPos, UInt32_7z *distances)
{
  UInt32_7z h2, h3, curMatch2, curMatch3;
  UInt32_7z *hash = p->hash;
  const Byte *cur = p->pointerToCurPos;
  UInt32_7z lzPos = p->lzPos;
  MT_HASH3_CALC

  curMatch2 = hash[                h2];
  curMatch3 = hash[kFix3HashSize + h3];
  
  hash[                h2] = lzPos;
  hash[kFix3HashSize + h3] = lzPos;

  if (curMatch2 >= matchMinPos && cur[(ptrdiff_t)curMatch2 - lzPos] == cur[0])
  {
    distances[1] = lzPos - curMatch2 - 1;
    if (cur[(ptrdiff_t)curMatch2 - lzPos + 2] == cur[2])
    {
      distances[0] = 3;
      return distances + 2;
    }
    distances[0] = 2;
    distances += 2;
  }
  
  if (curMatch3 >= matchMinPos && cur[(ptrdiff_t)curMatch3 - lzPos] == cur[0])
  {
    *distances++ = 3;
    *distances++ = lzPos - curMatch3 - 1;
  }
  
  return distances;
}

/*
static UInt32_7z *MixMatches4(CMatchFinderMt *p, UInt32_7z matchMinPos, UInt32_7z *distances)
{
  UInt32_7z h2, h3, h4, curMatch2, curMatch3, curMatch4;
  UInt32_7z *hash = p->hash;
  const Byte *cur = p->pointerToCurPos;
  UInt32_7z lzPos = p->lzPos;
  MT_HASH4_CALC
      
  curMatch2 = hash[                h2];
  curMatch3 = hash[kFix3HashSize + h3];
  curMatch4 = hash[kFix4HashSize + h4];
  
  hash[                h2] = lzPos;
  hash[kFix3HashSize + h3] = lzPos;
  hash[kFix4HashSize + h4] = lzPos;

  if (curMatch2 >= matchMinPos && cur[(ptrdiff_t)curMatch2 - lzPos] == cur[0])
  {
    distances[1] = lzPos - curMatch2 - 1;
    if (cur[(ptrdiff_t)curMatch2 - lzPos + 2] == cur[2])
    {
      distances[0] = (cur[(ptrdiff_t)curMatch2 - lzPos + 3] == cur[3]) ? 4 : 3;
      return distances + 2;
    }
    distances[0] = 2;
    distances += 2;
  }
  
  if (curMatch3 >= matchMinPos && cur[(ptrdiff_t)curMatch3 - lzPos] == cur[0])
  {
    distances[1] = lzPos - curMatch3 - 1;
    if (cur[(ptrdiff_t)curMatch3 - lzPos + 3] == cur[3])
    {
      distances[0] = 4;
      return distances + 2;
    }
    distances[0] = 3;
    distances += 2;
  }

  if (curMatch4 >= matchMinPos)
    if (
      cur[(ptrdiff_t)curMatch4 - lzPos] == cur[0] &&
      cur[(ptrdiff_t)curMatch4 - lzPos + 3] == cur[3]
      )
    {
      *distances++ = 4;
      *distances++ = lzPos - curMatch4 - 1;
    }
  
  return distances;
}
*/

#define INCREASE_LZ_POS p->lzPos++; p->pointerToCurPos++;

static UInt32_7z MatchFinderMt2_GetMatches(CMatchFinderMt *p, UInt32_7z *distances)
{
  const UInt32_7z *btBuf = p->btBuf + p->btBufPos;
  UInt32_7z len = *btBuf++;
  p->btBufPos += 1 + len;
  p->btNumAvailBytes--;
  {
    UInt32_7z i;
    for (i = 0; i < len; i += 2)
    {
      *distances++ = *btBuf++;
      *distances++ = *btBuf++;
    }
  }
  INCREASE_LZ_POS
  return len;
}

static UInt32_7z MatchFinderMt_GetMatches(CMatchFinderMt *p, UInt32_7z *distances)
{
  const UInt32_7z *btBuf = p->btBuf + p->btBufPos;
  UInt32_7z len = *btBuf++;
  p->btBufPos += 1 + len;

  if (len == 0)
  {
    /* change for bt5 ! */
    if (p->btNumAvailBytes-- >= 4)
      len = (UInt32_7z)(p->MixMatchesFunc(p, p->lzPos - p->historySize, distances) - (distances));
  }
  else
  {
    /* Condition: there are matches in btBuf with length < p->numHashBytes */
    UInt32_7z *distances2;
    p->btNumAvailBytes--;
    distances2 = p->MixMatchesFunc(p, p->lzPos - btBuf[1], distances);
    do
    {
      *distances2++ = *btBuf++;
      *distances2++ = *btBuf++;
    }
    while ((len -= 2) != 0);
    len = (UInt32_7z)(distances2 - (distances));
  }
  INCREASE_LZ_POS
  return len;
}

#define SKIP_HEADER2_MT  do { GET_NEXT_BLOCK_IF_REQUIRED
#define SKIP_HEADER_MT(n) SKIP_HEADER2_MT if (p->btNumAvailBytes-- >= (n)) { const Byte *cur = p->pointerToCurPos; UInt32_7z *hash = p->hash;
#define SKIP_FOOTER_MT } INCREASE_LZ_POS p->btBufPos += p->btBuf[p->btBufPos] + 1; } while (--num != 0);

static void MatchFinderMt0_Skip(CMatchFinderMt *p, UInt32_7z num)
{
  SKIP_HEADER2_MT { p->btNumAvailBytes--;
  SKIP_FOOTER_MT
}

static void MatchFinderMt2_Skip(CMatchFinderMt *p, UInt32_7z num)
{
  SKIP_HEADER_MT(2)
      UInt32_7z h2;
      MT_HASH2_CALC
      hash[h2] = p->lzPos;
  SKIP_FOOTER_MT
}

static void MatchFinderMt3_Skip(CMatchFinderMt *p, UInt32_7z num)
{
  SKIP_HEADER_MT(3)
      UInt32_7z h2, h3;
      MT_HASH3_CALC
      hash[kFix3HashSize + h3] =
      hash[                h2] =
        p->lzPos;
  SKIP_FOOTER_MT
}

/*
static void MatchFinderMt4_Skip(CMatchFinderMt *p, UInt32_7z num)
{
  SKIP_HEADER_MT(4)
      UInt32_7z h2, h3, h4;
      MT_HASH4_CALC
      hash[kFix4HashSize + h4] =
      hash[kFix3HashSize + h3] =
      hash[                h2] =
        p->lzPos;
  SKIP_FOOTER_MT
}
*/

void MatchFinderMt_CreateVTable(CMatchFinderMt *p, IMatchFinder *vTable)
{
  vTable->Init = (Mf_Init_Func)MatchFinderMt_Init;
  vTable->GetNumAvailableBytes = (Mf_GetNumAvailableBytes_Func)MatchFinderMt_GetNumAvailableBytes;
  vTable->GetPointerToCurrentPos = (Mf_GetPointerToCurrentPos_Func)MatchFinderMt_GetPointerToCurrentPos;
  vTable->GetMatches = (Mf_GetMatches_Func)MatchFinderMt_GetMatches;
  
  switch (p->MatchFinder->numHashBytes)
  {
    case 2:
      p->GetHeadsFunc = GetHeads2;
      p->MixMatchesFunc = (Mf_Mix_Matches)0;
      vTable->Skip = (Mf_Skip_Func)MatchFinderMt0_Skip;
      vTable->GetMatches = (Mf_GetMatches_Func)MatchFinderMt2_GetMatches;
      break;
    case 3:
      p->GetHeadsFunc = GetHeads3;
      p->MixMatchesFunc = (Mf_Mix_Matches)MixMatches2;
      vTable->Skip = (Mf_Skip_Func)MatchFinderMt2_Skip;
      break;
    default:
    /* case 4: */
      p->GetHeadsFunc = p->MatchFinder->bigHash ? GetHeads4b : GetHeads4;
      p->MixMatchesFunc = (Mf_Mix_Matches)MixMatches3;
      vTable->Skip = (Mf_Skip_Func)MatchFinderMt3_Skip;
      break;
    /*
    default:
      p->GetHeadsFunc = GetHeads5;
      p->MixMatchesFunc = (Mf_Mix_Matches)MixMatches4;
      vTable->Skip = (Mf_Skip_Func)MatchFinderMt4_Skip;
      break;
    */
  }
}
