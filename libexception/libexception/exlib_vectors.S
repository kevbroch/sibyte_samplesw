/* $Id: exlib_vectors.S,v 1.15 2005/01/05 05:43:59 cgd Exp $ */

/*
 * Copyright 2003, 2004
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

#include <sb1250-include/sbmips.h>
#include "exlib_int.h"

/* We abstract some of the instructions so that similar code can be
   used for mips32 or mips64.

   MFC0/MTC0/PTR_ADDIU were named to match SGI/linux sgidefs.h.

   REG_STORE/REG_LOAD are similar to the sgidefs.h REG_S/REG_L, but
   on 32-bit architectures accomodate the fact that our structure
   definitions think registers are 64-bits.  Not efficient, but 
   we'd like to support 64-bit values in regs on MIPS64 parts even
   if using a "-mips32" library.

   Note that REG_STORE is allowed to clobber "r" and does, for the
   mips32 exception handlers.  REG_LOAD **must** expand to exactly
   one instruction.  */
#ifndef MIPS_ISA
#define MIPS_ISA			mips64
#define REG_STORE(r, off, base_r)	sd	r, (off)(base_r)
#define REG_LOAD(r, off, base_r)	ld	r, (off)(base_r)
#define MFC0				dmfc0
#define MTC0				dmtc0
#define PTR_ADDIU			daddiu
#endif

#define CONCAT_(a,b)		a ## _ ## b
#define CONCAT(a,b)		CONCAT_(a,b)
#define VEC_NAME(n)		CONCAT(n,MIPS_ISA)

	/* .set mips64 isn't applied to the whole file, since it may
	   do icky thinks to address computations (as compared to what's
	   expected for the ABI.  */
	.set noreorder
	.set noat

	/* Cache error stub.  This is placed at the RAM cache error vector
	   if Status:BEV was set (i.e., there was not a cache error vector
	   in RAM previously).  All it does is jump to the ROM cache error
	   vector.  */
	.globl VEC_NAME(exlib_vec_cerr_stub)
	.ent VEC_NAME(exlib_vec_cerr_stub)
VEC_NAME(exlib_vec_cerr_stub):
	.set push
	.set MIPS_ISA
	REG_STORE(k0, (0x180 - 8), zero) /* Vector is in phys 0x100..0x180.  */
	li	k0, 0xbfc00300		/* Cache error vector in ROM.  */
	jr	k0
	REG_LOAD(k0, (0x180 - 8), zero)	/* (delay) */
	.set pop
	. = VEC_NAME(exlib_vec_cerr_stub) + EXVEC_SIZE
	.end VEC_NAME(exlib_vec_cerr_stub)

	/* The same vector is used for TLB Miss, XTLB miss, and general
	   exception vectors.  We don't attempt to optimize.  This code
	   simple plucks the code from the Cause register and heads to
	   the common dispatcher.  */
	.globl VEC_NAME(exlib_vec_general)
	.ent VEC_NAME(exlib_vec_general)
VEC_NAME(exlib_vec_general):
	la	k0, VEC_NAME(exlib_dispatch_asm)
	mfc0	k1, C0_CAUSE
	srl	k1, k1, 2		/* Shift ExcCode into low bits.  */
	jr	k0
	andi	k1, k1, 0x1f		/* Mask so that ExcCode remains.  */
	. = VEC_NAME(exlib_vec_general) + EXVEC_SIZE
	.end VEC_NAME(exlib_vec_general)


	.globl	VEC_NAME(exlib_dispatch_asm)	/* from C.  */

	.ent VEC_NAME(exlib_dispatch_asm)
VEC_NAME(exlib_dispatch_asm):
	.set push
	.set MIPS_ISA
	move	k0, sp
	PTR_ADDIU sp, sp, -EXFRAME_SIZE
	ori	sp, sp, 15
	xori	sp, sp, 15			# force 16-byte alignment

	REG_STORE(k0, EXFRAME_REG_OFF(29), sp)	# save old SP.
	REG_STORE(k1, EXFRAME_REG_OFF(27), sp)	# k1 slot = code

	MFC0	k0, C0_EPC
	REG_STORE(k0, EXFRAME_PC_OFF, sp)	# save exception PC.

	mfc0	k0, C0_STATUS
	mfc0	k1, C0_CAUSE

	// zero ($0) doesn't need to be saved, EPC put in its place.
	REG_STORE($1,  EXFRAME_REG_OFF( 1), sp)
	REG_STORE($2,  EXFRAME_REG_OFF( 2), sp)
	REG_STORE($3,  EXFRAME_REG_OFF( 3), sp)
	REG_STORE($4,  EXFRAME_REG_OFF( 4), sp)
	REG_STORE($5,  EXFRAME_REG_OFF( 5), sp)
	REG_STORE($6,  EXFRAME_REG_OFF( 6), sp)
	REG_STORE($7,  EXFRAME_REG_OFF( 7), sp)
	REG_STORE($8,  EXFRAME_REG_OFF( 8), sp)
	REG_STORE($9,  EXFRAME_REG_OFF( 9), sp)
	REG_STORE($10, EXFRAME_REG_OFF(10), sp)
	REG_STORE($11, EXFRAME_REG_OFF(11), sp)
	REG_STORE($12, EXFRAME_REG_OFF(12), sp)
	REG_STORE($13, EXFRAME_REG_OFF(13), sp)
	REG_STORE($14, EXFRAME_REG_OFF(14), sp)
	REG_STORE($15, EXFRAME_REG_OFF(15), sp)
	REG_STORE($16, EXFRAME_REG_OFF(16), sp)
	REG_STORE($17, EXFRAME_REG_OFF(17), sp)
	REG_STORE($18, EXFRAME_REG_OFF(18), sp)
	REG_STORE($19, EXFRAME_REG_OFF(19), sp)
	REG_STORE($20, EXFRAME_REG_OFF(20), sp)
	REG_STORE($21, EXFRAME_REG_OFF(21), sp)
	REG_STORE($22, EXFRAME_REG_OFF(22), sp)
	REG_STORE($23, EXFRAME_REG_OFF(23), sp)
	REG_STORE($24, EXFRAME_REG_OFF(24), sp)
	REG_STORE($25, EXFRAME_REG_OFF(25), sp)
	REG_STORE(zero, EXFRAME_REG_OFF(26), sp) # k0
	// k1 ($27) saved above, contains code.
	REG_STORE($28, EXFRAME_REG_OFF(28), sp)
	// sp ($29) saved above, contains SP.
	REG_STORE($30, EXFRAME_REG_OFF(30), sp)
	REG_STORE($31, EXFRAME_REG_OFF(31), sp)

	sw	k0, EXFRAME_STATUS_OFF(sp)	
	sw	k1, EXFRAME_CAUSE_OFF(sp)	

	mfhi	k0
	mflo	k1
	REG_STORE(k0, EXFRAME_HI_OFF, sp)
	REG_STORE(k1, EXFRAME_LO_OFF, sp)

	/* OK!  Everything has now been saved.  Turn off interrupts,
	   turn off EXL, and continue in C code.  */

	mfc0	k0, C0_STATUS
	ori	k0, k0, M_SR_EXL | M_SR_IE
	xori	k0, k0, M_SR_EXL | M_SR_IE
	mtc0	k0, C0_STATUS
	ssnop
	ssnop
	ssnop
	.set pop

	/* Set up our GP for C code.  */
	la	gp, _gp

	/* Invoke exlib_dispatch, with exframe pointer as only arg.  */
	move	a0, sp
	jal	exlib_dispatch
	.set push
	.set MIPS_ISA
	PTR_ADDIU sp, sp, -32			# (delay) O32/O64 reg save area.
	.set pop

1:	# exlib_dispatch should never return
	b	1b
	nop
	.end VEC_NAME(exlib_dispatch_asm)

	/* exlib_return: a0 contains frame pointer.  We restore
	   everything, saving the SP for last.  */
	.globl	VEC_NAME(exlib_return)
	.ent	VEC_NAME(exlib_return)
VEC_NAME(exlib_return):
	.set push
	.set MIPS_ISA

	/* Restore Status first, and make sure that EXL is set so that
	   the rest of the operations can proceed safely.  (Note that
	   we don't restore Cause.)  */
	lw	t4, EXFRAME_STATUS_OFF(a0)
	ori	t4, t4, M_SR_EXL
	mtc0	t4, C0_STATUS
#ifdef __mips_soft_float
	/* Clear EXL hazard, move exframe pointer to k0.  */
	ssnop
	ssnop
	ssnop
	move	k0, a0
#else
	/* Clear EXL and execution hazards, in preparation for possible
	   FP restore.  */
	ssnop
	bnezl	zero, 1f
	ssnop
1:

	/* Save exframe pointer around FP restore.  */
	move	s0, a0
	.set pop

	/* Set up GP in case it's needed by call to exframe_restore_fp.
	   It doesn't matter that we may be trashing the existing GP
	   here, we're going to trash it even harder later.  */
	la	gp, _gp

	/* Invoke exframe_restore_fp, with exframe pointer as only arg.
	   (exframe_restore_fp is run with EXL set.)  */
	move	a0, s0
	jal	exframe_restore_fp
	.set push
	.set MIPS_ISA
	PTR_ADDIU sp, sp, -32			# (delay) O32/O64 reg save area.
	.set pop

	.set push
	.set MIPS_ISA
	/* Move exframe pointer into k0.  */
	move	k0, s0
#endif

	REG_LOAD(k1, EXFRAME_PC_OFF, k0)	# restore exception PC.
	MTC0	k1, C0_EPC

	// zero ($0) doesn't need to be restored, EPC was put in its place.
	REG_LOAD($1,  EXFRAME_REG_OFF( 1), k0)
	REG_LOAD($2,  EXFRAME_REG_OFF( 2), k0)
	REG_LOAD($3,  EXFRAME_REG_OFF( 3), k0)
	REG_LOAD($4,  EXFRAME_REG_OFF( 4), k0)
	REG_LOAD($5,  EXFRAME_REG_OFF( 5), k0)
	REG_LOAD($6,  EXFRAME_REG_OFF( 6), k0)
	REG_LOAD($7,  EXFRAME_REG_OFF( 7), k0)
	REG_LOAD($8,  EXFRAME_REG_OFF( 8), k0)
	REG_LOAD($9,  EXFRAME_REG_OFF( 9), k0)
	REG_LOAD($10, EXFRAME_REG_OFF(10), k0)
	REG_LOAD($11, EXFRAME_REG_OFF(11), k0)
	REG_LOAD($12, EXFRAME_REG_OFF(12), k0)
	REG_LOAD($13, EXFRAME_REG_OFF(13), k0)
	REG_LOAD($14, EXFRAME_REG_OFF(14), k0)
	REG_LOAD($15, EXFRAME_REG_OFF(15), k0)
	REG_LOAD($16, EXFRAME_REG_OFF(16), k0)
	REG_LOAD($17, EXFRAME_REG_OFF(17), k0)
	REG_LOAD($18, EXFRAME_REG_OFF(18), k0)
	REG_LOAD($19, EXFRAME_REG_OFF(19), k0)
	REG_LOAD($20, EXFRAME_REG_OFF(20), k0)
	REG_LOAD($21, EXFRAME_REG_OFF(21), k0)
	REG_LOAD($22, EXFRAME_REG_OFF(22), k0)
	REG_LOAD($23, EXFRAME_REG_OFF(23), k0)
	REG_LOAD($24, EXFRAME_REG_OFF(24), k0)
	REG_LOAD($25, EXFRAME_REG_OFF(25), k0)
	// k0 is being used for exframe pointer.
	// k1 didn't really have a saved value.
	REG_LOAD($28, EXFRAME_REG_OFF(28), k0)
	// sp ($29) restored below.
	REG_LOAD($30, EXFRAME_REG_OFF(30), k0)
	REG_LOAD($31, EXFRAME_REG_OFF(31), k0)

	REG_LOAD(k1, EXFRAME_HI_OFF, k0)
	mthi	k1
	REG_LOAD(k1, EXFRAME_LO_OFF, k0)
	mtlo	k1

	REG_LOAD(sp, EXFRAME_REG_OFF(29), k0)	# Restore SP.

	# Return from exception.
	eret

	.set pop
	.end VEC_NAME(exlib_return)
