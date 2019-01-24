/* $Id: tlb.h,v 1.3 2003/05/09 04:23:54 cgd Exp $ */

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

/* tlb.h: MIPS64 TLB access.

   Structures:

     struct mips64_tlb_entry {
       unsigned long long entrylo0;
       unsigned long long entrylo1;
       unsigned long long entryhi;
       unsigned int pagemask;
     };

   Functions:

     unsigned int tlb_size (void);

       Get the number of entries in the TLB (by reading the Config1 register).

     void tlb_read (unsigned int i, struct mips64_tlb_entry *ep);

       Read TLB entry 'i' and fill in the structure pointed to by 'ep'
       with the entry's contents.

     int tlb_probe (unsigned long long va, unsigned int asid);

       Probe the TLB for an entry matching the the virtual address
       'va' in the ASID 'asid'.  If no match is found, a negative
       value is returned.  If a matching entry is found, the entry's
       index is returned.

     void tlb_write_index (unsigned int i, const struct mips64_tlb_entry *ep);

       Write TLB entry 'i' with the values in the structure pointed to
       by 'ep'.

     void tlb_write_random (const struct mips64_tlb_entry *ep);

       Write a random TLB entry with the values in the structure pointed
       to by 'ep'.

   Notes:

   1. This header uses 'unsigned long long' and 'unsigned int' (or
   'int') to represent 64- and 32-bit values respectively.  It would be
   better to use the fixed-size types uint64_t, uint32_t (and int32_t),
   but we can't be sure that they will be defined when this header is
   included.

   2. Including this header will produce an error if compiling for an
   environment which doesn't support direct use of 64-bit integer registers.
   (I.e., you need to be compiling -mips3, -mips4, or -mips64 code to
   include this header.)

   3. In order to use these functions reliably, you cannot take a TLB
   miss during their operation.  In other words, they should run from
   and access data in unmapped space or mapped space covered by wired
   TLB entries.  If the environment in which you are using this header
   supports interrupts, you should either disable interrupts while
   manipulating the TLB using these functions, or you should make sure
   that your interrupt handlers similarly will not take TLB misses or
   otherwise manipulate the TLB.  */

#ifndef _SIBYTE_TLB_H_
#define _SIBYTE_TLB_H_

#ifndef __mips64
# error can only use in a 64-bit environment.
#endif

#include <sb1250-include/sbmips.h>

struct mips64_tlb_entry {
  unsigned long long entrylo0;
  unsigned long long entrylo1;
  unsigned long long entryhi;
  unsigned int pagemask;
};

static inline unsigned int
tlb_size (void)
{
  unsigned int config1;
  __asm__ (
    "	.set push	\n"
    "	.set mips64	\n"
    "	mfc0 %0, $16, 1	\n"	/* Read Config1 register.  */
    "	.set pop	\n"
    : "=r"(config1));
  return (((config1 >> 25) & 0x3f) + 1);
}

static inline void
tlb_read (unsigned int i, struct mips64_tlb_entry *ep)
{
  cp0_set_index (i);

  __asm__ __volatile__ ("tlbr");

  ep->entrylo0 = cp0_get_entrylo0 ();
  ep->entrylo1 = cp0_get_entrylo1 ();
  ep->entryhi = cp0_get_entryhi ();
  ep->pagemask = cp0_get_pagemask ();
}

static inline int
tlb_probe (unsigned long long va, unsigned int asid)
{
  unsigned long long entryhi;

  entryhi = (va & ~ 0x1fffULL) | (asid & 0xff);
  cp0_set_entryhi (entryhi);

  __asm__ __volatile__ ("tlbp");

  return  (cp0_get_index ());
}

static inline void
tlb_write_index (unsigned int i, const struct mips64_tlb_entry *ep)
{
  cp0_set_index (i);
  cp0_set_entrylo0 (ep->entrylo0);
  cp0_set_entrylo1 (ep->entrylo1);
  cp0_set_entryhi (ep->entryhi);
  cp0_set_pagemask (ep->pagemask);

  __asm__ __volatile__ ("tlbwi");
}

static inline void
tlb_write_random (const struct mips64_tlb_entry *ep)
{
  cp0_set_entrylo0 (ep->entrylo0);
  cp0_set_entrylo1 (ep->entrylo1);
  cp0_set_entryhi (ep->entryhi);
  cp0_set_pagemask (ep->pagemask);

  __asm__ __volatile__ ("tlbwr");
}

#endif /* _SIBYTE_TLB_H_ */
