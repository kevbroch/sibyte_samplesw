/* $Id: tlballoc.c,v 1.2 2003/05/29 21:15:15 tbroch Exp $ */

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

/* tlballoc.c: allocates tlb entries to usable physical memory.

   The following are all mandatory functions if user desires to map
   memory not occupied by cfe & program to heap via tlb and use it

   __libcfe_meminit
   __libcfe_stack_top
   sbrk

   CFE's default is to locate the heap above _end (this program's
   highest address) which will result in a maximum heap of < 256MB

*/

#include <cfe_api.h>
#include <unistd.h>

#include <sb1250-include/sbmips.h>

#include "tlb_utils.h"

/* Pointer to end of program */
extern char _end[];

/* Maximum pagesize index (64MB) allowed by sb1 and mips architecture.
   User may change. 
   Allowable ranges: 12 <= MAX_PAGESIZE_INDEX <= 26 */
#define MAX_PAGESIZE_INDEX 26

/* Minimum pagesize index (4KB) allowed by sb1 and mips architecture.
   DO NOT CHANGE */
#define MIN_PAGESIZE_INDEX 12

/* Maximum pagesize index that should be wasted when mapping tlb.
   User may change.  
   Allowable ranges: 12 <= MAX_WASTED_PAGESIZE_INDEX <= 26 */
#define MAX_WASTED_PAGESIZE_INDEX 26

/* Starting virtual address to map in tlb. 
   User may change.
   Allowable ranges: 0x00001000 <= VA_START <= 0x7ffff000 */
#define VA_START 0x100000

/* Maximum number of tlb entries to use.  Note, tlb entry 0 always
   mapped to invalid.
   User may change.
   Allowable ranges: 1 <= MAX_TLB_ENTRIES_USED <= K_NTLBENTRIES */
#define MAX_TLB_ENTRIES_USED K_NTLBENTRIES

/* Align virtual address X to position for tlb entryHi register */
#define VA_TO_VPN2(x) (x >> S_TLBHI_VPN2)

/* Align physical address X to position for tlb entryLo register */
#define PA_TO_PFN(x)  (x >> MIN_PAGESIZE_INDEX)

/* Align pagemask X to position for tlb pagemask register */
#define C0_PGMASK_REG(x)            (V_TLB_PGMSK(((x-1) >> MIN_PAGESIZE_INDEX)))

/* Create register value for tlb entryLo */
#define C0_TLBLO_REG(pfn,cca,d,v,g) (V_TLBLO_PFNMASK(pfn) | V_TLBLO_CALG(cca) | \
                                     V_TLBLO_D(d) | V_TLBLO_V(v) | V_TLBLO_G(g))

/* Create register value for tlb entryHi */
#define C0_TLBHI_REG(va,asid)       (V_TLBHI_VPN2(va) | V_TLBHI_ASID(asid))

/* Pointer to lower limit of memory */
static char *heap_start;

/* Current pointer to memory for dynamic memory allocation */
static char *heap_limit;

/* Pointer to upper limit of memory */
static char *heap_end;

/* Number of bytes allocated to programs stack */
static unsigned long stack_size = (32 * 1024);

/* Virtual address of programs stack top */
static unsigned long stack_top;

/* Sets up memory (stack, heap) for program running on CFE under K0.
   Stack is placed in K0 space after program image.  Heap is allocated
   in KUSEG with all remaining memory by mapping tlb. */
void
__libcfe_meminit (void)
{

  /* Current physical address to map in tlb */
  uint64_t pa;

  /* Size (in bytes) of memory region returned by cfe_enummem */
  uint64_t pa_length;

  /* Integer representing type of memory region returned by cfe_enummem */
  uint64_t pa_type;
  
  /* Holds starting physical address memory region available to program */
  uint64_t pa_regions[8];

  /* Holds size (in bytes) memory region available to program */
  uint64_t pa_size[8];

  /* Current index to place memory region retrieved by cfe_enummem into */
  int pa_idx = 0;

  /* Current virtual address to map in tlb */
  uint64_t va = VA_START;

  /* Previous virtual address mapped into tlb */
  uint64_t prev_va = VA_START;

  /* Used to locate alignment virtual address */
  uint64_t va_shift;

  /* Shift amount allowable based on va alignment */
  int max_pagesize_index;		

  /* Holds number of bytes remaining in memory region */
  uint64_t mem_left = 0;

  /* Cache coherency attributes */  
  int cca; 				

  /* 2x pagesize to be allocated */
  int pagesize_2x = (1<<(MAX_PAGESIZE_INDEX+1)); 

  /* Current index to write tlb entry to */
  int tlb_idx = 0;

  /* Boolean to identify if final memory to allocate in tlb only covered half an entry */
  int half_entry = 0;

  /* Holds all register values associated with a tlb entry */
  struct mips64_tlb_entry tlb_entry;

  /* Size (in bytes) of program plus stack */
  int end_size;

  int i = 0;

  if (&__libcfe_stack_size != NULL)
    stack_size = __libcfe_stack_size();

  stack_top = (unsigned long) _end + stack_size;

  /* Find memory regions available.  Note, this command is 'aware' of
     what portion of the memory cfe has occupied, and removes it. */

  while (cfe_enummem(i, 1, &pa, &pa_length, &pa_type) == 0) 
    {
      if (pa_type == CFE_MI_AVAILABLE) 
	{
	  if ((K0_TO_PHYS(stack_top) & 0xf0000000) == pa) 
	    {

	      /* Remove memory associated w/ this program from tlb
		 mapping at a reasonable page boundary. */

	      end_size = (K0_TO_PHYS(stack_top));
	      pagesize_2x = 1 << MAX_WASTED_PAGESIZE_INDEX;
	      while (end_size > pagesize_2x) 
		{
		  pagesize_2x = pagesize_2x << 2;
		}

	      /* Increase stack size to top pa which will NOT be
                 mapped */

	      pa += pagesize_2x;
	      stack_top = PHYS_TO_K0(pa);

	      /* Insert a buffer area for the stack just in case user
		 program runs amuck */

	      pa += (1 << MIN_PAGESIZE_INDEX);
	      pa_length = pa_length - pagesize_2x - (1 << MIN_PAGESIZE_INDEX);
	    }
      
	  pa_regions[pa_idx] = pa;
	  pa_size[pa_idx] = pa_length;
	  pa_idx++;
	}
      i++;
    }

  /* 
     bug in cfe which won't allow this to work at the moment (RT ticket 3395)
  */

  // init_tlb(1);
  
  /* Allocate first tlb entry to virtual address zero and invalidate */

  // tlb_entry.entryhi  = 0;
  // tlb_entry.pagemask = C0_PGMASK_REG((va >> 1));
  // tlb_entry.entrylo0 = 0;
  // tlb_entry.entrylo1 = 0;
  // tlb_write_index(tlb_idx++, &tlb_entry);

  heap_limit = heap_start = (char *) (unsigned int) va;
  cca = G_CFG_K0COH(cp0_get_config());

  for (i = pa_idx-1; i >= 0; i--) 
    {

      mem_left = pa_size[i];
      pa = pa_regions[i];
    
      while ((mem_left >= (1 << MAX_WASTED_PAGESIZE_INDEX))
	     && (tlb_idx < MAX_TLB_ENTRIES_USED) 
	     && (prev_va <= va)) 
	{
	  /*  Determine the maximum pagesize based on va alignment */

	  va_shift = (va >> MIN_PAGESIZE_INDEX);
	  max_pagesize_index = MIN_PAGESIZE_INDEX;
	  while ((max_pagesize_index < MAX_PAGESIZE_INDEX)
		 && !(va_shift & 0x1)) 
	    {
	      va_shift = va_shift >> 1;
	      max_pagesize_index++;
	    }

	  /* Determine the maximum pagesize based on available memory */

	  max_pagesize_index -= 1; 
	  pagesize_2x = 1 << ((max_pagesize_index & ~1) + 1);
	  while ((mem_left < pagesize_2x)
		 && (pagesize_2x > (1 << (MIN_PAGESIZE_INDEX + 1)))) 
	    {
	      pagesize_2x = pagesize_2x >> 2;
	    }

	  tlb_entry.entryhi  = C0_TLBHI_REG(VA_TO_VPN2(va), 0);
	  tlb_entry.pagemask = C0_PGMASK_REG((pagesize_2x >> 1));

	  /* Handle case when only only one 4KB page remains.  Note,
	     global bit can't be asserted if only partial tlb entry
	     formed */
	      
	  if ((mem_left >= (1 << MIN_PAGESIZE_INDEX))
	      && (mem_left < (1 << (MIN_PAGESIZE_INDEX + 1))))
	    {
	      tlb_entry.entrylo0 = C0_TLBLO_REG(PA_TO_PFN(pa), cca, 1, 0, 0);
	      tlb_entry.entrylo1 = 0;
	      half_entry = 1;
	      mem_left = 0;
	    } 
	  else 
	    {
	      tlb_entry.entrylo0 = C0_TLBLO_REG(PA_TO_PFN(pa), cca, 1, 1, 1);
	      tlb_entry.entrylo1 = C0_TLBLO_REG(PA_TO_PFN((pa + (pagesize_2x >> 1))), 
						cca, 1, 1, 1);
	      mem_left -= pagesize_2x;
	    }

	  tlb_write_index (tlb_idx, &tlb_entry);
	  tlb_idx++;
	  pa += pagesize_2x;
	  prev_va = va;
	  va = ~USIZE & (va + pagesize_2x);
	}
    }

  cp0_set_wired (tlb_idx-1);

  if (half_entry) 
    {
      heap_end = (char *) (unsigned long) (va - (pagesize_2x >> 1));
    } 
  else 
    {
      heap_end = (char *) (unsigned long) (va);
    }
}

/* Returns virtual address STACK_TOP */
void 
*__libcfe_stack_top(void) 
{
  return (void *) (stack_top);
}


/* Increments/Decrements virtual address HEAP_LIMIT by bytes NBYTES.

   Returns virtual address BASE if allocation remains between virtual
   address bounds of HEAP_START and HEAP_END otherwise null */

void *
sbrk (ptrdiff_t nbytes)
{
  char *base;

  if ((nbytes >= (heap_end-heap_limit)) || ((heap_limit + nbytes) < heap_start)) 
    {
      base = (char *)-1;
    } 
  else 
    {
      base = heap_limit;
      heap_limit += nbytes;
    }
  return base;
}
