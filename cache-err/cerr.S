/* $Id: cerr.S,v 1.9 2003/11/13 17:56:25 kwalker Exp $ */

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

/*
 * cerr.S: Example code for dealing with cache errors on the SB1 core.
 *
 * This code is the fast, recoverable cache error path, and runs at
 * the exception vector.  It (1) acknowledges h/w-recovered single-bit
 * ECC errors in the Dcache, and (2) invalidates Icache lines that
 * encounter non-external tag or data parity errors.
 *
 * It should be noted that the ErrCtl and CacheErr registers may only
 * be inspected on the occurence of a cache error exception.  On exit
 * from a recovered error, contents are stale and change only when a
 * subsequent error happens.  'Unlocking' of the registers simply
 * allows them to be updated by hardware; it doesn't actually clear
 * them.
 *
 * Any Dcache error recovery code will need to be located outside the
 * exception vector space.  A limited set of error conditions may be
 * recoverable, but the code to detect and correct will be too large
 * to fit at the vector.  Any code that will be jumped to must be
 * up-to-date in memory since the vector runs from Kseg1.
 */

#include <sb1250-include/sbmips.h>

/* Eventually these will be in sbmips.h */
#define C0_CERR_I     $27             /* CP0: cache error */
#define C0_CERR_D     $27,1           /* CP0: Dcache error */

#define M_ERRCTL_RECOVERABLE _MM_MAKEMASK1(31)
#define M_ERRCTL_DCACHE      _MM_MAKEMASK1(30)
#define M_ERRCTL_ICACHE      _MM_MAKEMASK1(29)

#define M_CERRI_EXTERNAL     _MM_MAKEMASK1(26)

	.text
	.set	noreorder
	.set	noat

	/*
	 * sb1_cerr_vec: code to be copied to the Cache Error
	 * Exception vector.  The code must be pushed out to memory
	 * (either by copying to Kseg0 and Kseg1 both, or by flushing
	 * the L1 and L2) since it is fetched as 0xa0000100.
	 *
	 * NOTE: Be sure this handler is at most 28 instructions long
	 * since the final 16 bytes of the exception vector memory
	 * (0x170-0x17f) are used to preserve k0, k1, and ra.
	 */

	.align	5
	.globl	sb1_cerr_vec
	.ent	sb1_cerr_vec
sb1_cerr_vec:
	/*
	 * If this error is recoverable, we need to exit the handler
	 * without having dirtied any registers.  To do this,
	 * save/restore k0 and k1 from low memory (Useg is direct
	 * mapped while ERL=1). Note that we can't save to a
	 * CPU-specific location without ruining a register in the
	 * process.  This means we are vulnerable to data corruption
	 * whenever the handler is reentered by a second CPU.
	 */
	sd	k0,0x170($0)
	sd	k1,0x178($0)

	/*
	 * M_ERRCTL_RECOVERABLE is bit 31, which makes it easy to tell
	 * if we can fast-path out of here for a h/w-recovered error.
	 */
	mfc0	k1,C0_ERRCTL
	bgtz	k1,attempt_recovery
	 sll	k0,k1,1

recovered_dcache:
	/*
	 * Unlock CacheErr-D (which in turn unlocks CacheErr-DPA).
	 * Ought to log the occurence of this recovered dcache error.
	 */
	b	recovered
	 mtc0	$0,C0_CERR_D

attempt_recovery:
	/*
	 * k0 has C0_ERRCTL << 1, which puts 'DC' at bit 31.  Any
	 * Dcache errors we can recover from will take more extensive
	 * processing.  For now, they are considered "unrecoverable".
	 * Note that 'DC' becoming set (outside of ERL mode) will
	 * cause 'IC' to clear; so if there's an Icache error, we'll
	 * only find out about it if we recover from this error and
	 * continue executing.
	 */
	bltz	k0,unrecoverable
	 sll	k0,1

	/*
	 * k0 has C0_ERRCTL << 2, which puts 'IC' at bit 31.  If an
	 * Icache error isn't indicated, I'm not sure why we got here.
	 * Consider that case "unrecoverable" for now.
	 */
	bgez	k0,unrecoverable

attempt_icache_recovery:
	/*
	 * External icache errors are due to uncorrectable ECC errors
	 * in the L2 cache or Memory Controller and cannot be
	 * recovered here.
	 */
	 mfc0	k0,C0_CERR_I		/* delay slot */
	li	k1,M_CERRI_EXTERNAL
	and	k1,k0
	bnez	k1,unrecoverable
	 andi	k0,0x1fe0

	/*
	 * Since the error is internal, the 'IDX' field from
	 * CacheErr-I is valid and we can just invalidate all blocks
	 * in that set.
	 */
	cache	Index_Invalidate_I,(0<<13)(k0)
	cache	Index_Invalidate_I,(1<<13)(k0)
	cache	Index_Invalidate_I,(2<<13)(k0)
	cache	Index_Invalidate_I,(3<<13)(k0)

	/* Ought to log this recovered icache error */

recovered:
	/* Restore the saved registers */
	ld	k0,0x170($0)
	ld	k1,0x178($0)
	eret

unrecoverable:
	/* Unrecoverable Icache or Dcache error; log it and/or fail */
1:	b	1b
	 nop

	.end	sb1_cerr_vec

	. = sb1_cerr_vec + 0x80

