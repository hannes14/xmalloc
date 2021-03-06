/**
 * \file   bin.h
 * \author Christian Eder ( ederc@mathematik.uni-kl.de )
 * \date   August 2012
 * \brief  Bin handlers for xmalloc.
 *         This file is part of XMALLOC, licensed under the GNU General
 *         Public License version 3. See COPYING for more information.
 * \note   There are some page based functions implemented in the bin.*
 *         files since they depend internally on the strong page <-> bin
 *         connection.
 */

#ifndef XMALLOC_BIN_H
#define XMALLOC_BIN_H

#include <stdlib.h>
#include <string.h>
#include <limits.h> // for ULONG_MAX etc.
#include "xassert.h"
#include "xmalloc-config.h"
#include "data.h"
#include "globals.h"
#include "page.h"
#include "region.h"
#include "align.h"

/************************************************
 * NOTE: The functionality of getting and freeing
 *       xSpecBins must be located in xmalloc.*
 *       since it depends on xMalloc(), etc.
 ***********************************************/
// xBin xGetSpecBin(size_t size); => see xmalloc.h
// void xUnGetSpecBin(xBin *bin); => see xmalloc.h

/************************************************
 * FREEING OPERATIONS CONCERNING PAGES: THOSE
 * DEPEND ON BINS => THEY ARE IMPLEMENTED HERE
 ***********************************************/
/**
 * \fn void xFreeToPageFault(xPage page, void *addr)
 *
 * \brief If there was a problem in \c FreeToPage() this function has to
 * take care of the freeing: At the point this function is called we already
 * know that \c page->numberUsedBlocks <= 0.
 * There are 3 different strategies on what to do with pages which were full and
 * have a free block now:
 * 1. Insert at the end ( default ).
 * 2. Insert after \c current_page
 *    => define \c __XMALLOC_PAGE_AFTER_CURRENT
 * 3. Insert before \c current_page , i.e. let it be the new current page
 *    => define \c __XMALLOC_PAGE_BEFORE_CURRENT
 *
 * \param page \c xPage the freed memory should be given to
 *
 * \param addr memory address to be checked
 *
 */
void xFreeToPageFault(xPage page, void *addr); // TOODOO

/**
 * \fn static inline void xFreeToPage(xPage page, void *addr)
 *
 * \brief Frees memory at \c addr to \c xPage \c page .
 *
 * \param page \c xPage the freed memory should be given to
 *
 * \param addr memory address to be checked
 *
 */
static inline void xFreeToPage(xPage page, void *addr)
{
  if (page->numberUsedBlocks > 0L)
  {
    *((void **) addr) = page->current;
    page->current     = addr;
  }
  else
  {
    xFreeToPageFault(page, addr);
  }
  //printf("free p=%p, numberUsedBlocks:%d\n",page,page->numberUsedBlocks);
}

/************************************************
 * STICKY BUSINESS OF BINS
 ***********************************************/
/**
 * \fn static inline int xIsStickyBin(xBin bin)
 *
 * \brief Tests if \c bin is sticky or not.
 *
 * \param bin \c xBin to be tested
 *
 * \return true if \c bin is sticky, false else
 *
 */
static inline int xIsStickyBin(xBin bin) {
#if __XMALLOC_DEBUG > 1
  printf("bin %p -- sticky %p\n", bin, bin->sticky);
#endif
  return (bin->sticky >= __XMALLOC_SIZEOF_VOIDP);
}

// xBin xGetStickyBinOfBin(xBin bin); -> see xmalloc.h


/************************************************
 * LIST HANDLING FOR SPECIAL BINS
 ***********************************************/

/**
 * \fn static inline void* xFindInSortedList(xSpecBin bin, long numberBlocks)
 *
 * \brief Tries to find a special bin for the size class given by
 * \c numberBlocks . If there does not exist such a special bin, then
 * it returns NULL. Here the list searched through is already sorted by
 * increasing size classes.
 *
 * \param bin \c xSpecBin usually base of special bins, but can be any
 * \c xSpecBin .
 *
 * \param numberBlocks \c long number of blocks in bin, i.e. size class
 * needed for special bin.
 *
 * \return address of found xSpecBin, NULL if none is found
 */
static inline void* xFindInSortedList(xSpecBin bin, long numberBlocks) {
  while (NULL != bin) {
    if (bin->numberBlocks >= numberBlocks) {
      if (bin->numberBlocks == numberBlocks)
        return bin;
      // list is sorted, thus we know that there does not exist
      // such a special bin we need in this list
      return NULL;
    }
    bin = bin->next;
  }
  return NULL;
}

/**
 * \fn static inline void* xFindInList(xSpecBin bin, long numberBlocks, xSpecBin
 * sBin)
 *
 * \brief Tries to find a special bin for the size class given by
 * \c numberBlocks . If there does not exist such a special bin, then
 * it returns NULL. Here the list searched through is possibly not sorted.
 *
 * \param bin \c xSpecBin usually base of special bins, but can be any
 * \c xSpecBin .
 *
 * \param numberBlocks \c long number of blocks in bin, i.e. size class
 * needed for special bin.
 *
 * \param sBin \c xSpecBin to be found in the list.
 *
 * \return address of found xSpecBin, NULL if none is found
 */
static inline void* xFindInList(xSpecBin bin, long numberBlocks, xSpecBin sBin) {
  while (NULL != bin) {
    if (bin->numberBlocks == numberBlocks)
      return bin;
    bin = bin->next;
  }
  return NULL;
}

/**
 * \fn static inline void* xInsertIntoSortedList(xSpecBin rootBin, xSpecBin sBin,
 * long numberBlocks)
 *
 * \brief Inserts spec bin \c bin into sorted list of spec bins. The list is
 * sorted by increasing \c numberBlocks .
 *
 * \param rootBin \c xSpecBin usually root of the list of special bins, but can be
 * any \c xSpecBin .
 *
 * \param sBin \c xSpecBin special bin to be inserted into the list of special
 * bins
 *
 * \param numberBlocks \c long number of blocks in bin, i.e. size class
 * needed for special bin.
 *
 * \return address of the root of the list of the special bins.
 *
 */
static inline xSpecBin xInsertIntoSortedList(xSpecBin rootBin, xSpecBin sBin,
    long numberBlocks) {
  if (NULL == rootBin || numberBlocks <= rootBin->numberBlocks) {
    sBin->next = rootBin;
    return sBin;
  } else {
    xSpecBin prev = rootBin;
    xSpecBin curr = rootBin->next;

    while (NULL != curr && curr->numberBlocks < numberBlocks) {
      prev  = curr;
      curr  = curr->next;
    }
    prev->next  = sBin;
    sBin->next  = curr;

    return rootBin;
  }
}

/**
 * \fn static inline xSpecBin xRemoveFromSortedList(xSpecBin rootBin, xSpecBin sBin)
 *
 * \brief Removes \c sBin from list of spec bins beginning at \c rootBin .
 *
 * \param rootBin \c xSpecBin usually root of the list of special bins, but can be
 * any \c xSpecBin .
 *
 * \param sBin \c xSpecBin special bin to be removed from the list of special
 * bins
 *
 * \return address of the root of the list of the special bins.
 *
 */
static inline xSpecBin xRemoveFromSortedList(xSpecBin rootBin, xSpecBin sBin)
{
  // if root of spec bins is NULL, we are done
  if (NULL == rootBin)
    return NULL;

  xSpecBin listIterator       = rootBin->next;
  long numberBlocks  = sBin->numberBlocks;

  // if root of spec bins coincides with sBin, just cut it out
  if (rootBin == sBin)
    return listIterator;
  // now we really need to search for sBin in the list of spec bins
  // NOTE: If root of spec bins has a greater number of blocks, then we are
  // done at the first step since sBin cannot be in this list at all

  // we need to remember the starting point of the list of spec bins
  xSpecBin rootBinAnchor = rootBin;
  while (NULL != listIterator && listIterator != sBin)
  {
    if (rootBin->numberBlocks > numberBlocks)
      return rootBinAnchor;
    rootBin       = listIterator;
    listIterator  = listIterator->next;
  }

  if (NULL != listIterator)
    rootBin->next = listIterator->next;

  return rootBinAnchor;
}

/************************************************
 * HANDLING LISTS OF PAGES / PAGE INFORAMTION
 * IN BINS
 ***********************************************/
/**
 * \fn void xTakeOutPageFromBin(xPage page, xBin bin)
 *
 * \brief Takes \c page out of \c bin.
 *
 * \param bin \c xBin the new page is a part of
 *
 * \param page \c xPage taken out of \c bin
 *
 */
static inline void xTakeOutPageFromBin(xPage page, xBin bin)
{
  if (bin->currentPage == page)
  {
    if (NULL == page->next)
    {
      if (NULL == page->prev)
      {
        bin->lastPage = NULL;
        bin->currentPage  = __XMALLOC_ZERO_PAGE;
        return;
      }
      bin->currentPage  = page->prev;
    }
    else
    {
      bin->currentPage  = page->next;
    }
  }
  if (bin->lastPage == page)
  {
    bin->lastPage = page->prev;
  }
  else
  {
    page->next->prev  = page->prev;
  }
  if (NULL != page->prev)
    page->prev->next  = page->next;
}

/**
 * \fn void xInsertPageToBin(xPage page, xBin bin)
 *
 * \brief Inserts the newly allocated \c xPage \c page to \c bin.
 *
 * \param page \c xPage the new page
 *
 * \param bin \c xBin* the new page becomes a part of
 *
 */
static inline void xInsertPageToBin(xPage page, xBin bin) {
  if (__XMALLOC_ZERO_PAGE == bin->currentPage) {
    page->prev        = NULL;
    page->next        = NULL;
    bin->currentPage  = page;
    bin->lastPage     = page;
  } else {
    if (bin->currentPage == bin->lastPage) {
      bin->lastPage = page;
    } else {
      bin->currentPage->next->prev  = page;
    }
    page->next              = bin->currentPage->next;
    bin->currentPage->next  = page;
    page->prev              = bin->currentPage;
  }
}

/**
 * \fn xPage xAllocNewPageForBin(xBin bin)
 *
 * \brief Allocates a new \c xPage to \c bin.
 *
 * \param bin \c xBin the new page becomes a part of
 *
 * \return \c xPage allocated
 */
xPage xAllocNewPageForBin(xBin bin);

/**
 * \fn xPage xAllocSmallBlockPageForBin()
 *
 * \brief Allocates a new \c xPage for small block free lists.
 *
 * \note This function does NEITHER subdivide NOR structure the allocated page.
 * This must be done afterwards
 *
 * \return \c xPage allocated.
 *
 */
xPage xAllocSmallBlockPageForBin();

/**
 * \fn xPage xAllocBigBlockPagesForBin(int numberNeeded)
 *
 * \brief Allocates new \c xPages for big block memory.
 *
 * \param numberNeeded is the number of pages to be allocated.
 *
 * \note This function does NEITHER subdivide NOR structure the allocated pages.
 * This must be done afterwards.
 *
 * \return \c xPage representing the first one of \c numberNeeded \c
 * xPages needed for this allocation
 *
 */
xPage xAllocBigBlockPagesForBin(int numberNeeded);

/************************************************
 * ALLOCATING PAGES IN BINS
 ***********************************************/
/**
 * \fn static inline void* xAllocFromFullPage(xBin bin)
 *
 * \brief Returns memory address from a newly allocated page.
 *
 * \param bin \c xBin* the new page becomes a part of
 *
 * \return address of allocated memory
 *
 */
static inline void* xAllocFromFullPage(xBin bin)
{
  xPage newPage;
  if (__XMALLOC_ZERO_PAGE != bin->currentPage)
  {
    bin->currentPage->numberUsedBlocks  = 0;
  }

  if(!bin->sticky && (NULL != bin->currentPage->next))
  {
    newPage = bin->currentPage->next;
  }
  else
  {
    newPage = xAllocNewPageForBin(bin);
    xInsertPageToBin(newPage, bin);
  }
  __XMALLOC_ASSERT(NULL != newPage);
  __XMALLOC_ASSERT(__XMALLOC_ZERO_PAGE != newPage);
  __XMALLOC_ASSERT(NULL != newPage->current);
  bin->currentPage  = newPage;
  return xAllocFromNonEmptyPage(newPage);
}

/************************************************
 * FREEING BINS
 ***********************************************/

// see src/xmalloc.h

/************************************************
 * ALLOCATING MEMORY FROM BINS
 ***********************************************/
/**
 * \fn static inline void xAllocFromBin(xBin bin)
 *
 * \brief Generic memory allocation from \c bin.
 *
 * \param bin \c xBin the bin memory should be allocated from
 *
 * \return address of allocated memory
 *
 */
static inline void* xAllocFromBin(xBin bin)
{
  register xPage page = bin->currentPage;
  if ((page!=NULL) && (page->current != NULL))
    return xAllocFromNonEmptyPage(page);
  else
    return xAllocFromFullPage(bin);
}

/**
 * \fn static inline void xAlloc0FromBin(xBin bin)
 *
 * \brief Generic memory allocation from \c bin , sets to zero.
 *
 * \param bin \c xBin the bin memory should be allocated from
 *
 * \return address of allocated memory
 *
 */
static inline void* xAlloc0FromBin(xBin bin)
{
  void *addr  = xAllocFromBin(bin);
  memset(addr, 0, bin->sizeInWords * __XMALLOC_SIZEOF_ALIGNMENT);
  return addr;
}

/************************************************
 * DEBUGGING INFORMATION
 ***********************************************/
//#ifdef __XMALLOC_DEBUG

/************************************************
 * STICKY BIN PAGE BUSINESS
 ***********************************************/
/**
 * \fn static inline unsigned long xGetStickyOfPage(xPage page)
 *
 * \brief Gets sticky of \c xBin of \c page .
 *
 * \param page \c xPage
 *
 * \return sticky of \c page
 *
 */
static inline unsigned long xGetStickyOfPage(xPage page)
{
  return (((unsigned long) page->bin) &
      (unsigned long) __XMALLOC_SIZEOF_VOIDP_MINUS_ONE);
}

/**
 * \fn static inline void xSetTopBinAndStickyOfPage(xPage page, xBin bin)
 *
 * \brief Set top bin and sticky bin of the page \c page .
 *
 * \param page \c xPage .
 *
 * \param bin \c xBin .
 *
 */
static inline void xSetTopBinAndStickyOfPage(xPage page, xBin bin)
{
  page->bin = (void *)(((unsigned long)bin->sticky & (__XMALLOC_SIZEOF_VOIDP - 1))
                  + (unsigned long)bin);
}

/**
 * \fn static inline void xSetTopBinOfPage(xPage page, xBin bin)
 *
 * \brief Set top bin of the page \c page .
 *
 * \param page \c xPage .
 *
 * \param bin \c xBin .
 *
 */
static inline void xSetTopBinOfPage(xPage page, xBin bin)
{
  page->bin = (void *)((unsigned long)bin + xGetStickyOfPage(page));
}

/**
 * \fn static inline xBin xGetTopBinOfPage(const xPage page)
 *
 * \brief Get top bin the page \c page .
 *
 * \param page Const \c xPage .
 *
 * \return top \c xBin of \c xPage \c page
 *
 */
static inline xBin xGetTopBinOfPage(const xPage page)
{
#if __XMALLOC_DEBUG > 1
  printf("page %p -- gtpoba %p\n", page, page->bin);
#endif
  return (xBin) (((unsigned long) page->bin) &
      ~((unsigned long) __XMALLOC_SIZEOF_VOIDP_MINUS_ONE));
}

/**
 * \fn static inline void xSetStickyOfPage(xPage page, xBin sBin)
 *
 * \brief Sets sticky bin of \c page .
 *
 * \param page \c xPage
 *
 * \param sBin \c xBin sticky
 *
 */
static inline void xSetStickyOfPage(xPage page, xBin bin)
{
  page->bin = (void *)(((unsigned long)bin->sticky
                          & ((unsigned long)__XMALLOC_SIZEOF_VOIDP - 1)) +
                        (unsigned long)xGetTopBinOfPage(page));
}

/**
 * \fn static inline xBin xGetBinOfPage(const xPage page)
 *
 * \brief Get top bin the page \c page .
 *
 * \param page Const \c xPage .
 *
 * \return top \c xBin of \c xPage \c page
 *
 */
static inline xBin xGetBinOfPage(const xPage page)
{
  unsigned long sticky  = xGetStickyOfPage(page);
  xBin bin              = xGetTopBinOfPage(page);
  if (!xIsStickyBin(bin))
    while ((bin->sticky != sticky) && (NULL != bin->next))
      bin = bin->next;

  return bin;
}

/**
 * \fn static inline xBin xGetBinOfAddr(void *addr)
 *
 * \brief Get bin of address \c addr .
 *
 * \param addr \c void* of which the bin should be found.
 *
 * \return \c xBin of address \c addr
 *
 */
static inline xBin xGetBinOfAddr(void *addr)
{
  return xGetBinOfPage((xPage) xGetPageOfAddr(addr));
}

/**
 * \fn static inline void xSetBinOfPage(xPage page, xBin bin)
 *
 * \brief Set top bin the page \c page to sticky of \c bin .
 *
 * \param page \c xPage .
 *
 * \param bin \c xbin .
 *
 */
static inline void xSetBinOfPage(xPage page, xBin bin)
{
  page->bin = (void *)((unsigned long)bin + xGetStickyOfPage(page));
}

/**
 * \fn xPage xGetPageFromBin(xBin bin)
 *
 * \brief Gets a currently free page from \c bin.
 *
 * \param bin \c xBin the bin the page should be part of
 *
 * \return „current free \c xPage from \c bin
 *
 */
xPage xGetPageFromBin(xBin bin);

/******************************************************
 * STATIC BIN TESTINGS
 *****************************************************/
/**
 * \fn static inline int xIsStaticBin(xBin bin)
 *
 * \brief Tests if \c bin is an \c xStaticBin array entry.
 *
 * \param bin \c xBin to be be tested.
 *
 * \return true if \c bin is an entry of \c xStaticBin, false otherwise.
 *
 */
static inline int xIsStaticBin(xBin bin)
{
  return  ((unsigned long) bin >= (unsigned long) &xStaticBin[0]) &&
  ((unsigned long) bin <= (unsigned long) &xStaticBin[__XMALLOC_MAX_BIN_INDEX]);
}
#endif
