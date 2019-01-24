/* $Id: strcmp.S,v 1.14 2003/09/05 17:54:44 cgd Exp $ */

/*
 * Copyright 2002, 2003
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

#ifndef STRCMP_FUNC
#define STRCMP_FUNC strcmp
#endif

#include "sb1-opt-common.h"

/* Some general notes about this implementation:

   1) Last comparisons of (aligned) word data, before existing:

      When big-endian, we take advantage of the fact that the
      high order bits of the word reflect the earlier characters in
      the string, and use direct word comparison to generate the
      sign of the result.  We still need to check for NUL in
      the last word, though, because its position matters.

      When little-endian, we can't play this arithmetic trick, and
      therefore we have to do byte-by-byte comparisons of the bytes
      in the last word.  We don't bother doing a special NUL check,
      since that's handled by the byte-by-byte checks.

   2) Constants used by this algorithm.

      This algorithm uses two constants to help it quickly do
      NUL-checking.  They must come from *somewhere*.  Generating
      them in the 32-bit case would take two cycles.  Generating them
      in the 64-bit case would take substantially longer.  Therefore,
      we load them from memory, which if they're in the cache takes
      just one cycle's worth of LS ops.  Finding them in memory,
      however, is a bit of a problem.

      For PIC code, loading the constants may be hard: typically,
      they'd have GOT entries relative to GP.  We'd have to chase
      through those (after loading GP!).  By the time you add up
      setting GP, saving and restoring it (for n32/n64), then chasing
      through appropriately-calculated GOT entry, it's a bunch of
      cycles.  Luckily, abicalls/PIC code, we know that we have the
      address of strcmp in incoming t9 (required by the calling
      conventions), so we waste a dcache line on two constants and
      load the constants from text (relative to the function).

      For non-PIC code, we hope for GP-relative (single-instruction)
      loads.  If the program is being compiled with a -G value less
      than NBYTES, the data placement and the loads in strcmp may well
      produce a less-than-optimal result.

      If you change the constant loading or placement code *AT ALL*,
      please note the comments down by the actual loads of the
      constants.  */


/* Temporary regs used to hold constants.  */
#define	c1reg a2
#define	c2reg a3


#ifdef __PIC__	/* See note 2, above.  */
	.text
	.align 5
#else
	.sdata
	.type	 const1,@object
	.size	 const1,NBYTES
	.type	 const2,@object
	.size	 const2,NBYTES
#endif
const1:
#if NBYTES == 8
	.dword	0x8080808080808080
#else
	.word	0x80808080
#endif
const2:
#if NBYTES == 8
	.dword	0xfefefefefefefeff
#else
	.word	0xfefefeff
#endif

#define LS1NOP	addu zero, zero, zero

	.text
	.set noreorder
	.set noat
#ifdef __mips64
	.set mips64
#else
	.set mips32
#endif

        .globl STRCMP_FUNC
	.ent   STRCMP_FUNC

	/* Hand scheduled strcmp() */

	/* On entry, pointers to the strings are in a0 and a1 */
	.align	5
STRCMP_FUNC:
	/* First, determine if the strings are aligned.  If they are,
	then go do the compare in dword blocks */
	ADDIU	v0, zero, 0      /* Default return value is equal (0) */
	or	t1, a0, a1
#ifdef __PIC__	/* See note 2, above.  */
	/* This is made even more evil by the fact that the assembler
	   won't calculate (const1 - strcmp) for us, even though it
	   *is* a constant offset.  So, hard-code.  */
# if 0
	LOAD	c1reg,(const1 - STRCMP_FUNC)(t9)
	LOAD	c2reg,(const2 - STRCMP_FUNC)(t9)
# else
	LOAD	c1reg,(-32 + (0 * NBYTES))(t9)
	LOAD	c2reg,(-32 + (1 * NBYTES))(t9)
# endif
#else
	LOAD	c1reg,const1
	LOAD	c2reg,const2
#endif

	LS1NOP
	andi	t1, t1, (NBYTES - 1)

	bnez	t1, unaligned_loop
	 nop

	/* The aligned case. */
	/* Not sure how these will issue, other than that they'll
	   issue 2 at a time */
	/* Probably worth it to replace these with a pair of loads
	   from global space */ 
	.align 4
aligned_loop:

	/* Issue block */
	LOAD	t0, (a0)
	ADDIU	a0, a0, NBYTES   /* Short pipe */
	ADDU	t3, t0, c2reg
	nor	t2, zero, t0
	
	/* Issue break */
	LOAD	t1, (a1)
	ADDIU	a1, a1, NBYTES   
	bne	t0, t1, early_mismatch
	 and	t3, t3, t2

	/* Issue break */
	and	t3, t3, c1reg

	/* Issue break */
	bnez	t3, zero_detected
	 SUBU	t2, t0, t1
	/* Unroll loop iteration 1 */
	LOAD	t0, (a0)

	/* Issue break	*/
	ADDIU	a0, a0, NBYTES   /* Short pipe */
	ADDU	t3, t0, c2reg
	nor	t2, zero, t0
	LOAD	t1, (a1)
	
	/* Issue break */
	ADDIU	a1, a1, NBYTES   
	bne	t0, t1, early_mismatch
	 and	t3, t3, t2

	/* Issue break */
	and	t3, t3, c1reg

	/* Issue block */
	bnez	t3, zero_detected
	 SUBU	t2, t0, t1
	/* Unroll Loop iteration 2 */
	LOAD	t0, (a0)
	ADDU	t3, t0, c2reg
	
	/* Issue break */
	ADDIU	a0, a0, NBYTES   /* Short pipe */
	nor	t2, zero, t0
	LOAD	t1, (a1)
	
	/* Issue break */
	ADDIU	a1, a1, NBYTES   
#if defined (__MIPSEL__)  /* See note 1, above.  */
	/* If little-endian, we can't just compare the words to get a
	   meaningful result, so we detect mismatch and bail out
	   early.  We'll check byte-by-byte, so the zero checks will
	   be taken care of that way.

	   For BE, we could branch to early_mismatch here, but for
	   dhrystone it isn't faster so we don't.  */
	bne	t0, t1, early_mismatch
#endif
	 and	t3, t3, t2
	
	/* Issue break */
#if !defined (__MIPSEL__)  /* See note 1, above.  */
        /* Compare words, for loop_exit test.  */
	sltu	t2, t0, t1
#endif
	and	t3, t3, c1reg

	/* Issue block */
	bnez	t3, zero_detected
	 SUBU	zero, t0, t1

	/* Issue block */
	beq	t0, t1, aligned_loop
	 nop

loop_exit:	
	/* Issue block */
	bgtz	t2, aligned_done
	 ADDIU	v0, zero, -1  /* Short pipe */
	ADDIU	v0, zero, 1
aligned_done:
	/* Return 0 */
	jr	ra
	 nop
	
early_mismatch:
#if defined (__MIPSEL__)  /* See note 1, above.  */
	/* If little-endian, you have to compare byte by byte, so
	   just fall through; t0 will always != t1.  */
#else
	/* If big-endian, comparing the words will produce a
	   meaningful result.  So, we check for zero, to see if we
	   must go byte-by-byte.  */
	LS1NOP
	sltu	t2, t0, t1
	and	t3, t3, c1reg

	/* Issue block */
	LS1NOP
	beqz	t3, loop_exit
	 nop
#endif
	

zero_detected:
	/* If we hit a zero and the dwords match, cool, we know the
	strings are equal.  If the dwords DON'T match, we need to to
	some byte compares.  It's possible to have dwords be unequal
	and yet have the strings be equal if the terminating 
	null is somewhere mid-dword */
	ADDIU	a0, a0, -NBYTES  /* Undo speculative increment */
	ADDIU	a1, a1, -NBYTES
	bne	t0, t1, unaligned_loop
	nop;

	jr	ra
	nop
	
unaligned_loop:
	/* 4x unroll gets us down close to 2 cycles per byte compare */
	/* Issue block */
	lbu	t0, 0(a0)
	lbu	t1, 0(a1)
	bne	t1, t0, unaligned_unequal
	 and	t2, t1, t0
	/* Issue block */
	beqz	t2, unaligned_done
	 ADDIU	zero, zero, 0	 /* Short pipe */
	ADDIU	zero, zero, 0	
	ADDIU	zero, zero, 0	
	/* Issue block */
	lbu	t0, 1(a0)
	lbu	t1, 1(a1)
	bne	t1, t0, unaligned_unequal
	 and	t2, t1, t0
	/* Issue block */
	beqz	t2, unaligned_done
	 ADDIU	zero, zero, 0	 /* Short pipe */
	ADDIU	zero, zero, 0	
	ADDIU	zero, zero, 0	
	/* Issue block */
	lbu	t0, 2(a0)
	lbu	t1, 2(a1)
	bne	t1, t0, unaligned_unequal
	 and	t2, t1, t0
	/* Issue block */
	beqz	t2, unaligned_done
	 ADDIU	zero, zero, 0	 /* Short pipe */
	ADDIU	zero, zero, 0	
	ADDIU	zero, zero, 0	
	/* Issue block */
	lbu	t0, 3(a0)
	lbu	t1, 3(a1)
	bne	t1, t0, unaligned_unequal
	 and	t2, t1, t0
	/* Issue block */
	beqz	t2, unaligned_done
	 ADDIU	a0, a0, 4        /* Short pipe */
	/* Issue block */
	b	unaligned_loop
	 ADDIU	a1, a1, 4

	/* When we get here, the last bytes are in t0 and t1 */
unaligned_unequal:
	SUBU	t2, t0, t1
	bltz	t2, unaligned_done
	 ADDIU	v0, zero, -1
	ADDIU	v0, zero, 1	
unaligned_done:
	jr	ra
	 nop

	.end	STRCMP_FUNC
