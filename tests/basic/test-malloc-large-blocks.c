/**
 * \file   test-malloc-large-blocks.c
 * \author Christian Eder ( ederc@mathematik.uni-kl.de )
 * \date   October 2012
 * \brief  Unit test for large block allocations for xmalloc.
 *         This file is part of XMALLOC, licensed under the GNU General
 *         Public License version 3. See COPYING for more information.
 */

#include <stdio.h>
#include "xmalloc-config.h"
#include "xmalloc.h"

int main()
{
  int i = __XMALLOC_MAX_SMALL_BLOCK_SIZE;
  for (i; i < 10 * __XMALLOC_MAX_SMALL_BLOCK_SIZE; i++)
  {
    void *p = xMalloc(i);
    __XMALLOC_ASSERT(NULL != p &&
        "xMalloc should have allocated addr != NULL.");
    xFree(p);
  }
  return 0;
}
