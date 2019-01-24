/* $Id: tlbtestalloc.c,v 1.1 2003/05/09 21:29:25 tbroch Exp $

   tlbtestalloc.c: Tests full range of memory allocated to heap 

   returns non-negative integer if success, otherwise failure
*/

/*
 * Copyright 2003
 * Broadcom Corporation. All rights reserved.
 *
 * This software is furnished under license and may be used and copied only
 * in accordance with the following terms and conditions.  Subject to these
 * conditions, you may download, copy, install, use, modify and distribute
 * modified or unmodified copies of this software in source and/or binary
 * form. No title or ownership is transferred hereby.
 *
 * 1) Any source code used, modified or distributed must reproduce and
 *    retain this copyright notice and list of conditions as they appear in
 *    the source file.
 *
 * 2) No right is granted to use any trade name, trademark, or logo of
 *    Broadcom Corporation.  The "Broadcom Corporation" name may not be
 *    used to endorse or promote products derived from this software
 *    without the prior written permission of Broadcom Corporation.
 *
 * 3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR IMPLIED
 *    WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED WARRANTIES OF
 *    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR
 *    NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT SHALL BROADCOM BE LIABLE
 *    FOR ANY DAMAGES WHATSOEVER, AND IN PARTICULAR, BROADCOM SHALL NOT BE
 *    LIABLE FOR DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *    OR OTHERWISE), EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sb1250-include/sbmips.h>

#include "tlb.h"
#include "tlb_utils.h"

#define NUM_PTRS 128

typedef unsigned long long uint64_t;

static char *progname;

int 
main (int argc, char *argv[]) 
{

  uint64_t mem_used = 0;

  int size = 1 << 29;
  int i = 0;

  char * ptr_array[NUM_PTRS];
  int size_array[NUM_PTRS];

  int ptr_cnt = 0;

  char * ptr = 0;

  for (i = 0; i < NUM_PTRS; i++) 
    {
      ptr_array[i] = 0;
    }

  struct mips64_tlb_entry ep;

  int tlb_cnt = 0;

  progname = "tlbtestalloc";

  tlb_read (0, &ep);

  i = 0;

  printf("Allocating all available memory ...\n");

  while ((size > 0) && (ptr_cnt < NUM_PTRS))
    {

      /* Allocate all memory catching the pointers to each region */

      while ((size > 0) && !(ptr_array[ptr_cnt] = malloc(size)))
	{
	  size = size >> 1;
	}

      if (size > 0)
	{
	  while ((ep.entryhi <= ((unsigned long) ptr_array[ptr_cnt] + size))
		 && tlb_cnt < K_NTLBENTRIES)
	    {
	      printf ("TLB%02d\n",tlb_cnt);
	      dump_tlb_entry (0, &ep);
	      tlb_cnt++;
	      tlb_read (tlb_cnt, &ep);
	    }
	  mem_used += size;
	  size_array[ptr_cnt] = size;
	  printf ("%02d alloc of %012d sbrk = %p ptr_array = %p\n", 
		  i++, size, sbrk(0), ptr_array[ptr_cnt]);
	  ptr_cnt++;
	}
    }

  /* Check to see if there are any valid un-allocated tlb entries */

  if ((G_TLBLO_V(ep.entrylo0)) && (tlb_cnt < 64))
    {
      printf ("WARNING: found valid tlb entry which didn't allocate ... shouldn't be > 4KB pages\n");
      printf ("%02d ",tlb_cnt);
      dump_tlb_entry (0, &ep);
    }
  printf ("total_mem alloc = %lld (MB) = %lld (KB) = %lld :: ptr_cnt = %d\n",
	  mem_used >> 20,mem_used >> 10,mem_used, ptr_cnt);
  
  /* Write the memory you've allocated */

  printf("Writing and freeing all available memory ...\n");

  for (i = 0; i < ptr_cnt; i++)
    {
      ptr = ptr_array[i];
      size = size_array[i];
      while ((size >= 0))
	{
	  *ptr = 'a';
	  if (*ptr != 'a')
	    {
	      printf ("%s: %s.c: failed write followed by read for address %p\n", 
		      progname, progname, ptr);
	      return -1;
	    }
	  ptr++;
	  size--;
	}
      printf ("write & free %02d %p\n", i, ptr_array[i]);
      free (ptr_array[i]);
    }

  /* Re-check that memory can still be allocated the exact same amount */

  for (i = 0; i < ptr_cnt; i++)
    {
      if (!malloc(size_array[i]))
	{ 
	  printf ("%s: %s.c: couldn't re-malloc original size of %08X\n",
		  progname, progname, size_array[i]);
	  return -1;
	}
    }

  printf ("SUCCESS!!\n");
  return 0;
}
