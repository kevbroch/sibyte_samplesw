/* $Id: tlb_utils.c,v 1.3 2003/10/10 06:02:54 cgd Exp $ */

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

/* tlb_utils.c: Some MIPS64 TLB utility routines.  */

#include <stdio.h>

#include "tlb.h"
#include "tlb_utils.h"

void
dump_tlb_entry (int indent, const struct mips64_tlb_entry *ep)
{
  printf ("%*sEntryLo0: 0x%016llx\n", indent, "", ep->entrylo0);
  printf ("%*sEntryLo1: 0x%016llx\n", indent, "", ep->entrylo1);
  printf ("%*sEntryHi:  0x%016llx\n", indent, "", ep->entryhi);
  printf ("%*sPageMask: 0x%08x\n", indent, "", ep->pagemask);
}

void
dump_tlb (int indent)
{
  struct mips64_tlb_entry e;
  int i, size;

  size = tlb_size ();
  printf ("%*sTLB size: %d entries (%d wired)\n", indent, "", size,
          cp0_get_wired ());

  for (i = 0; i < size; i++)
    {
      tlb_read (i, &e);
      printf ("\n");
      printf ("%*sEntry %d:\n", indent, "", i);
      dump_tlb_entry (indent + 2, &e);
    }
}

void
invalidate_tlb_entry (int i, unsigned long long invalid_range_base)
{
  struct mips64_tlb_entry e;

  e.entrylo0 = 0;				/* V = 0 -> invalid.  */
  e.entrylo1 = 0;				/* V = 0 -> invalid.  */
  e.entryhi = invalid_range_base + (8192 * i) + TLB_INVALID_ASID;
  e.pagemask = 0;				/* 4K page size.  */
  tlb_write_index (i, &e);
}

void
init_tlb (int safe, unsigned long long invalid_range_base)
{
  int i, size;

  size = tlb_size ();

  if (safe)
    {
      struct mips64_tlb_entry e;
      unsigned long long safeaddr;
      int covering_index;

      /* Invalidate the TLB, safely moving entries to addresses
         starting at 'safeaddr'.  This is only necessary after
         transfer of control from other code that might initialize the
         TLB to incompatible invalid addresses (e.g., firmware).

         'invalid_range_base' should be aligned to a power of two
         greater than or equal to (tlb_size () * 2 * 4096) or the
         maximum supported TLB entry size (2 * max page size),
         whichever is greater.

         'safeaddr' is initialized to the start of the range to use
         when doing a safe reset of all TLB entries, e.g. after
         transfer of control from the firmware.  All of the TLB
         entries are moved first into the 'safe' range, in a way that
         will keep them from overlapping other mappings which may
         already exist.  Then they're moved into the normal
         invalid-TLB range given in 'invalid_range_base'.  There must
         be (2 * 8k * tlb_size()) bytes of address space available at
         safeaddr which won't overlap a region of memory starting at
         invalid_range_base.  */

      safeaddr = invalid_range_base - (unsigned long long)(2 * 8192 * size);
      i = 0;
      while (i < size)
        {
          /* We must make sure that nothing is already mapped at
             safeaddr, or skip to the next address.  */
          covering_index = tlb_probe (safeaddr, TLB_INVALID_ASID);
          if (covering_index >= 0)
            {
              /* An entry is already covering this address.  
                 Shrink it down to minimum size and see if it's
                 still covering this address.  */
              tlb_read (covering_index, &e);
              e.pagemask = 0;			/* 4K page size.  */
              tlb_write_index (covering_index, &e);

              covering_index = tlb_probe (safeaddr, TLB_INVALID_ASID);
              if (covering_index >= 0)
                {
                  /* Still valid, so the existing entry is mapped exactly
                     at 'safeaddr'.  Increment 'safeaddr' and try again.  */
                  safeaddr += 8192;
                  continue;
                }
            }

          /* OK, the coast (or at least this VA 8-) is clear.  */
          e.entrylo0 = 0;			/* V = 0 -> invalid.  */
          e.entrylo1 = 0;			/* V = 0 -> invalid.  */
          e.entryhi = safeaddr + TLB_INVALID_ASID;
          e.pagemask = 0;			/* 4K page size.  */
          tlb_write_index (i, &e);

          /* Go on to the next entry and the next address.  */
          i++;
          safeaddr += 8192;
        }
    }
  
  /* Invalidate the TLB, moving the entries to their normal
     (invalid-entry) addresses.  */
  for (i = 0; i < size; i++)
    invalidate_tlb_entry (i, invalid_range_base);
}

