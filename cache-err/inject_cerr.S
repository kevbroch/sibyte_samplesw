/* $Id: inject_cerr.S,v 1.1 2003/11/13 16:52:11 kwalker Exp $ */

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
 * inject_ic_err.S is a little JTAG dinter stub intended to introduce Icache
 * errors.  The IDX must be set to something that should cause trouble for 
 * CPU0.  Select tag or data error with the other #defines.
 */

#include "sbmips.h"

#define I_IDX		0x3a0
#define D_IDX		0x660
#define ICACHE_TAG	0
#define ICACHE_DATA	0
#define DCACHE_TAG_ST	0
#define DCACHE_TAG_ADDR	0
#define DCACHE_TAG	(DCACHE_TAG_ST || DCACHE_TAG_ADDR)
#define DCACHE_DATA_SGL	1
#define DCACHE_DATA_DBL	0

#define C0_DATALO C0_TAGLO
#define C0_DATAHI C0_TAGHI

#define Index_Load_Data_I   6
#define Index_Store_Data_I 10
#define Index_Load_Data_D   7
#define Index_Store_Data_D 11

	.macro	itag_corrupt	index tmp way
	cache	Index_Load_Tag_I,(\way<<13)(\index)
	mfc0	\tmp,C0_TAGLO
	xori	\tmp,0xc00
	mtc0	\tmp,C0_TAGLO
	cache	Index_Store_Tag_I,(\way<<13)(\index)
	.endm

	.macro	idata_corrupt	index tmp way offset
	cache	Index_Load_Data_I,(\way<<13)+\offset(\index)
	mfc0	\tmp,$29,1
	xori	\tmp,0xff
	mtc0	\tmp,$29,1
	cache	Index_Store_Data_I,(\way<<13)+\offset(\index)
	.endm

	.macro	dtag_corrupt	index tmp tmp2 way
	cache	Index_Load_Tag_D,(\way<<13)(\index)
#if DCACHE_TAG_ST
	mfc0	\tmp,C0_TAGHI,2
	li	\tmp2,0x06000000
	xor	\tmp,\tmp2
	mtc0	\tmp,C0_TAGHI,2
#endif
#if DCACHE_TAG_ADDR
	mfc0	\tmp,C0_TAGLO,2
	xor	\tmp,0xc00
	mtc0	\tmp,C0_TAGLO,2
#endif
	cache	Index_Store_Tag_D,(\way<<13)(\index)
	.endm

	.macro	ddata_corrupt	index tmp way offset pattern
	cache	Index_Load_Data_D,(\way<<13)+\offset(\index)
	dmfc0	\tmp,C0_DATALO,3
	xori	\tmp,\pattern
	dmtc0	\tmp,C0_DATALO,3
	cache	Index_Store_Data_D,(\way<<13)+\offset(\index)
	.endm

	.macro	idata_corrupt_all	index tmp way
	idata_corrupt \index \tmp \way 0
	idata_corrupt \index \tmp \way 8
	idata_corrupt \index \tmp \way 16
	idata_corrupt \index \tmp \way 24
	.endm

	.macro	ddata_corrupt_all	index tmp way pattern
	ddata_corrupt \index \tmp \way 0  \pattern
	ddata_corrupt \index \tmp \way 8  \pattern
	ddata_corrupt \index \tmp \way 16 \pattern
	ddata_corrupt \index \tmp \way 24 \pattern
	.endm

	.set	noat
	.set	noreorder

	.globl	_start
	.ent	_start
_start:
	dmtc0	$15,$31		# save a register in DESAVE
	la	$15,save_reg
	sd	$16,0($15)	# save more
	sd	$17,8($15)
	
	# Inject an Icache error here
#if ICACHE_TAG
	li	$15,I_IDX
	itag_corrupt $15 $16 0
	itag_corrupt $15 $16 1
	itag_corrupt $15 $16 2
	itag_corrupt $15 $16 3
#endif	
#if ICACHE_DATA
	li	$15,I_IDX
	idata_corrupt_all $15 $16 0
	idata_corrupt_all $15 $16 1
	idata_corrupt_all $15 $16 2
	idata_corrupt_all $15 $16 3
#endif
#if DCACHE_TAG
	li	$15,D_IDX
	dtag_corrupt $15 $16 $17 0
	dtag_corrupt $15 $16 $17 1
	dtag_corrupt $15 $16 $17 2
	dtag_corrupt $15 $16 $17 3
#endif
#if DCACHE_DATA_SGL
	li	$15,D_IDX
	ddata_corrupt_all $15 $16 0 1
	ddata_corrupt_all $15 $16 1 1
	ddata_corrupt_all $15 $16 2 1
	ddata_corrupt_all $15 $16 3 1
#endif
#if DCACHE_DATA_DBL
	li	$15,D_IDX
	ddata_corrupt_all $15 $16 0 3
	ddata_corrupt_all $15 $16 1 3
	ddata_corrupt_all $15 $16 2 3
	ddata_corrupt_all $15 $16 3 3
#endif
	# end of stuff
	
	# restore state
	la	$15,save_reg
	ld	$16,0($15)	# restore more
	ld	$17,8($15)
	dmfc0	$15,$31		# restore the first register
	sync			# push out any pending JTAG ld/st
	
	deret

	.end	_start

	.data

save_reg:
	.dword	0,0		# space for two registers
	.dword	0,0		# scratch
