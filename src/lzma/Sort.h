/* Sort.h -- Sort functions
2014-04-05 : Igor Pavlov : Public domain */

#ifndef __7Z_SORT_H
#define __7Z_SORT_H

#include "7zTypes.h"

EXTERN_C_BEGIN

void HeapSort(UInt32_7z *p, size_t size);
void HeapSort64(UInt64 *p, size_t size);

/* void HeapSortRef(UInt32_7z *p, UInt32_7z *vals, size_t size); */

EXTERN_C_END

#endif
