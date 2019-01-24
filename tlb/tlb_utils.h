/* $Id: tlb_utils.h,v 1.2 2003/10/10 06:02:54 cgd Exp $ */

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

/* tlb_utils.h: Definitions of some MIPS64 TLB utility routines.  */

#ifndef _SIBYTE_TLB_UTILS_H_
#define _SIBYTE_TLB_UTILS_H_

#include "tlb.h"

/* TLB_INVALID_RANGE defines the start of the range used for invalid TLB
   entries.  Since even TLB entries with no valid mappings aren't allowed
   to overlap each other, we assign each invalid TLB to the address
   TLB_INVALID_RANGE + (i * 8k) to avoid overlap.

   This value will usually be passed as the second argument to the
   init_tlb() and invalidate_tlb_entry() functions.  In certain
   circumstances, it may be desirable to use a different value.  */
#define	TLB_INVALID_RANGE	0xffffffffa0000000ULL	/* kseg1.  */

/* TLB_INVALID_ASID is the ASID to use in invalid TLB entries.  Note that
   this is a perfectly good (valid) ASID, it's just the one we happen
   to use for invalid entries.  (This code doesn't allow this value
   to be overridden; it is exposed only for testing purposes.)  */
#define	TLB_INVALID_ASID	0

void dump_tlb (int indent);
void dump_tlb_entry (int indent, const struct mips64_tlb_entry *ep);
void init_tlb (int safe, unsigned long long invalid_range_base);
void invalidate_tlb_entry (int i, unsigned long long invalid_range_base);

#endif /* _SIBYTE_TLB_UTILS_H_ */
