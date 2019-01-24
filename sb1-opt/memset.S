/* $Id: memset.S,v 1.12 2003/09/05 18:17:17 cgd Exp $ */

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

#ifndef MEMSET_FUNC
#define MEMSET_FUNC memset
#endif

#include "sb1-opt-common.h"

#define FIRST(unit) ((unit)*NBYTES)
#define REST(unit)  (FIRST(unit)+NBYTES-1)
#define DEST(unit)  FIRST(unit)

#define ADDRMASK (NBYTES-1)

#ifndef PASS1
# undef PASS2
# define PASS2
#endif

#ifdef M1_WORKAROUND
#define USE(x) lbu AT,x; sll zero, AT, 0
#else
#define USE(x)
#endif

	.text
	.set noreorder
	.set noat
#ifdef __mips64
	.set mips64
#else
	.set mips32
#endif

	.globl MEMSET_FUNC
	.ent MEMSET_FUNC
	.align 4
MEMSET_FUNC:	
	## Arguments.  Note: dst may be unaligned, len may be 0
#define dst a0
#define val a1
#define len a2
	## Temps
#define tmp    $8
#define mod    $9
#define small  $10
#define count  $11
#define adjust $12
#define dst2   $13
#define pref_base $14

	## If len < NBYTES use byte operations
	pref	PREF_STORE, 0(dst)
	andi	val, 0xff
	bnez	val, replicate_byte
	 and	mod, dst, ADDRMASK
val_ready:
	ADDU	v0, dst, zero		# Set return value.
	bnez	mod, unaligned
aligned:			# N.B.:	delay slot is also a branch target
	## len is number of bytes left to set
	## set count to number of loop iters
	## set len to final number of bytes to copy;
	##   LOG_NBYTES+4 -> 16 UNIT stores per iter
	 SRL	count, len, LOG_NBYTES+4
	
	SUB	pref_base, dst, 9*32
	beqz	count, set_units
	 and	len, len, ((1 << (LOG_NBYTES+4)) - 1)

	sltiu	pref_base, count, 2
	pref	PREF_STORE, 1*32(dst)
	pref	PREF_STORE, 2*32(dst)
	bnez	pref_base, clip_pref
	 pref	PREF_STORE, 3*32(dst)
	ADD	pref_base, dst, 0 	# {d,}mov pref_base, dst
	pref	PREF_STORE, 4*32(dst)
	pref	PREF_STORE, 5*32(dst)
	pref	PREF_STORE, 6*32(dst)
	pref	PREF_STORE, 7*32(dst)
	pref	PREF_STORE, 8*32(dst)
clip_pref:	
	SUB	count, count, 1
	USE(0(dst))
	.align 4
set_many:	
	pref	PREF_STORE, 9*32(pref_base)
#ifdef C1_WORKAROUND
	lbu	AT, 31(dst)
	sll	zero, AT, 0
	SLL	dst2, dst, 0
#else
	USE(31(dst))
#endif
	STORE	val, FIRST(0)(dst)
	STORE	val, FIRST(1)(dst)
	STORE	val, FIRST(2)(dst)
	STORE	val, FIRST(3)(dst)
#ifdef PASS2
	ssnop
#endif
#ifdef USE_DOUBLE
	pref	PREF_STORE, 10*32(pref_base)
	USE(63(dst))
#endif
	STORE	val, FIRST(4)(dst)
	STORE	val, FIRST(5)(dst)
	STORE	val, FIRST(6)(dst)
	STORE	val, FIRST(7)(dst)
#ifdef PASS2
	ssnop
#endif
#ifdef USE_DOUBLE
	pref	PREF_STORE, 11*32(pref_base)
	USE(95(dst))
#else
	pref	PREF_STORE, 10*32(pref_base)
	USE(63(dst))
	ADD	pref_base, pref_base, 16*NBYTES
#endif
	STORE	val, FIRST(8)(dst)
	STORE	val, FIRST(9)(dst)
	STORE	val, FIRST(10)(dst)
	STORE	val, FIRST(11)(dst)
#ifdef PASS2
	ssnop
#endif
#ifdef USE_DOUBLE
	pref	PREF_STORE, 12*32(pref_base)
	USE(127(dst))
	ADD	pref_base, pref_base, 16*NBYTES
#endif
	STORE	val, FIRST(12)(dst)
	STORE	val, FIRST(13)(dst)
	STORE	val, FIRST(14)(dst)
	STORE	val, FIRST(15)(dst)
	ADD	dst, dst, 16*NBYTES
#ifdef C1_WORKAROUND
	/* Keep both load pipes busy for 5 cycles by issuing prefs or loads
	   that hit in L1.  This prevents the DCFIFO from completing a store,
	   which would lock out snoops/fills for 4 cycles. */
	andi	AT, count, 0x7
	bnez	AT, set_many
	 SUB	count, count, 1
	pref	PREF_STORE, 31(dst2)
	pref	PREF_STORE, 31(dst2)
	pref	PREF_STORE, 31(dst2)
	pref	PREF_STORE, 31(dst2)
	pref	PREF_STORE, 31(dst2)
	ADD	count, count, 1
	pref	PREF_STORE, 31(dst2)
	pref	PREF_STORE, 31(dst2)
	pref	PREF_STORE, 31(dst2)
	pref	PREF_STORE, 31(dst2)
	pref	PREF_STORE, 31(dst2)
2:
#endif
	bgtz	count, set_many
	 SUB	count, count, 1
	
set_units:
	## set len bytes;  0 <= len < 16*NBYTES
	## Let adjust = number of UNIT-aligned bytes to set
	## Let count = adjust - NBYTES
	## (count  must be less than 16 UNITS)
	## set len = number of final bytes to set; len < NBYTES
#ifdef M1_WORKAROUND
	## Try to ensure an L1-hit
	pref	PREF_STORE, FIRST(0)(dst)
# ifdef USE_DOUBLE
	pref	PREF_STORE, FIRST(4)(dst)
# endif
	pref	PREF_STORE, FIRST(8)(dst)
# ifdef USE_DOUBLE
	pref	PREF_STORE, FIRST(12)(dst)
# endif
	## FIRST(0) may not be 32-byte aligned
	pref	PREF_STORE, FIRST(15)(dst)
#endif
	AND	adjust, len, (0xff << LOG_NBYTES) # L1
	beqz	adjust, set_bytes_checklen
	 AND	len, len, ADDRMASK
	SUB	count, adjust, NBYTES
#ifdef M1_WORKAROUND
#define DO_UNIT(N) \
	lbu	AT, FIRST(N)(dst); sll zero, AT, 0; \
	beqz	count, set_bytes_checklen_patch_dst; \
	 STORE	val, FIRST(N)(dst); \
	SUB	count, count, NBYTES;
#else
#define DO_UNIT(N) \
	beqz	count, set_bytes_checklen_patch_dst; \
	 STORE	val, FIRST(N)(dst); \
	SUB	count, count, NBYTES;
#endif
	DO_UNIT(0);
	DO_UNIT(1);
	DO_UNIT(2);
	DO_UNIT(3);
	DO_UNIT(4);
	DO_UNIT(5);
	DO_UNIT(6);
	DO_UNIT(7);
	DO_UNIT(8);
	DO_UNIT(9);
	DO_UNIT(10);
	DO_UNIT(11);
	DO_UNIT(12);
	DO_UNIT(13);
	## Must be exactly 1 UNIT left to do (set_units does at most 15 UNITS)
	STORE	val, FIRST(14)(dst);
set_bytes_checklen_patch_dst:
	## Increment dst by number of bytes done by set_units
	ADD	dst, dst, adjust
set_bytes_checklen:
	## len is number of bytes to set
	## 0 <= len < NBYTES
#define DO_BYTE(N) \
	beqz	len, done; \
	 SUB	len, len, 1; \
	sb	val, N(dst);
	DO_BYTE(0);
	DO_BYTE(1);
	DO_BYTE(2);
	DO_BYTE(3);
#ifdef USE_DOUBLE
	DO_BYTE(4);
	DO_BYTE(5);
	DO_BYTE(6);
	DO_BYTE(7);
#endif
done:
	jr	ra
	 nop
	
	.align 4
unaligned:
	## Use a single STFIRST to align dst to a UNIT (NBYTES) boundary
	## (unless len < NBYTES)
	li	tmp, NBYTES
	sltu	small, len, NBYTES 	# long pipe
	SUB	tmp, tmp, mod	# tmp = NBYTES - (dst % NBYTES)

	bnez	small, set_bytes_checklen
	 nop
	SUB	len, len, tmp
	STFIRST	val,FIRST(0)(dst)
	j	aligned
	 ADD	dst, dst, tmp

	.align 4
replicate_byte:	
	SLL	tmp, val, 8
	or	val, val, tmp	# val now 2 bytes wide
	SLL	tmp, val, 16
#ifdef USE_DOUBLE
	or	val, val, tmp	# val now 4 bytes wide
	dsll32	tmp, val, 0
#endif
	j	val_ready
	 or	val, val, tmp	# val now NBYTES wide
	.end	MEMSET_FUNC
