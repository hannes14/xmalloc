/**
 * \file   system.c
 * \author Christian Eder ( ederc@mathematik.uni-kl.de )
 * \date   August 2012
 * \brief  Source file for system allocations in xmalloc.
 *         This file is part of XMALLOC, licensed under the GNU General
 *         Public License version 3. See COPYING for more information.
 */

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "src/page.h" // for xIsAddrPageAligned
#include "src/system.h"
#include <errno.h>

void* xAllocFromSystem(size_t size)
{
  void *addr  = malloc(size);
  if (NULL == addr)
  {
    // try it once more
    addr  = malloc(size);
    if (NULL == addr)
    {
      printf("out of memory in malloc(%d)\n",errno);
      exit(1);
    }
  }

#ifndef __XMALLOC_NDEBUG
  // track some statistics if in debugging mode
  info.currentBytesFromMalloc +=  size;
  if (info.currentBytesFromMalloc > info.maxBytesFromMalloc)
  {
    info.maxBytesFromMalloc = info.currentBytesFromMalloc;
#ifdef __XMALLOC_HAVE_MMAP
    if (info.currentBytesFromValloc > info.maxBytesSystem)
      info.maxBytesSystem = info.currentBytesFromValloc;
#endif
  }
#endif
  return addr; // possibly addr == NULL
}

void* xReallocSizeFromSystem(void *addr, size_t oldSize, size_t newSize)
{
  void *newAddr = realloc(addr, newSize);
  if (NULL == newAddr)
  {
    newAddr = realloc(addr, newSize);
    if (NULL == newAddr)
    {
      printf("out of memory in realloc\n");
      exit(1);
    }
  }

#ifndef __XMALLOC_NDEBUG
  info.currentBytesFromMalloc +=  (long) newSize - (long) oldSize;

  if (info.currentBytesFromMalloc > info.maxBytesFromMalloc)
  {
    info.maxBytesFromMalloc = info.currentBytesFromMalloc;
#ifdef __XMALLOC_HAVE_MMAP
    if (info.currentBytesFromValloc > info.maxBytesSystem)
      info.maxBytesSystem = info.currentBytesFromValloc;
#endif
  }
#endif
  return newAddr;
}

void* xVallocFromSystem(size_t size)
{
  void *addr  = __XMALLOC_VALLOC(size);
  if (NULL == addr)
    // try it once more
    addr = __XMALLOC_VALLOC(size);

#ifndef __XMALLOC_NDEBUG
  __XMALLOC_ASSERT(xIsAddrPageAligned(addr));

  // track some statistics if in debugging mode
  info.currentBytesFromMalloc +=  size;
  if (info.currentBytesFromMalloc > info.maxBytesFromMalloc)
  {
    info.maxBytesFromMalloc = info.currentBytesFromMalloc;
#ifdef __XMALLOC_HAVE_MMAP
    if (info.maxBytesFromValloc > info.maxBytesSystem)
      info.maxBytesSystem = info.maxBytesFromValloc;
#endif
  }
#endif
      
  return addr; // possibly addr == NULL
}

void xVfreeToSystem(void *addr, size_t size)
{
  __XMALLOC_ASSERT(xIsAddrPageAligned(addr));
  munmap(addr, size);
#ifndef __XMALLOC_NDEBUG
  info.currentBytesFromMalloc -=  size;
#endif
}

void xVfreeNoMmap(void *addr, size_t size)
{
  __XMALLOC_ASSERT(xIsAddrPageAligned(addr));
#ifndef __XMALLOC_NDEBUG
  info.currentBytesFromMalloc -=  size;
#endif
  free(addr);
}

void xFreeSizeToSystem(void *addr, size_t size)
{
#ifndef __XMALLOC_NDEBUG
  info.currentBytesFromMalloc -=  size;
#endif
  free(addr);
}

void* xVallocMmap(size_t size)
{
  void *addr;
  addr  = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if ((void *)-1 == addr)
    return NULL;
#ifndef __XMALLOC_NDEBUG
  info.currentBytesFromMalloc +=  size;
#endif
  return addr;
}

void* xVallocNoMmap(size_t size)
{
#ifndef __XMALLOC_NDEBUG
  info.currentBytesFromMalloc +=  size;
#endif
  return valloc(size);
}
