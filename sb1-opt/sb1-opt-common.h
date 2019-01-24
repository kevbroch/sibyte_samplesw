/* $Id: sb1-opt-common.h,v 1.3 2004/02/03 07:07:05 cgd Exp $ */

/*
 * Copyright 2002, 2003, 2004
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

#ifdef __linux__

# include <sys/regdef.h>
# if (defined _ABIN32 && _MIPS_SIM == _ABIN32) \
     || (defined _ABI64 && _MIPS_SIM == _ABI64)
#  define	t6	a6
#  define	t7	a7
# endif

#else /* Not Linux, assume sb1-elf tools w/ SiByte headers.  */

#ifdef __LANGUAGE_ASSEMBLY 
# ifndef __ASSEMBLER__
#  define __ASSEMBLER__
# endif
#endif

# include <sb1250-include/sbmips.h>

#endif

#ifdef __mips64
#define USE_DOUBLE
#else
#undef USE_DOUBLE
#endif

#if defined(USE_DOUBLE)
	
#define LOAD  ld
#define LOADL ldl
#define LOADR ldr
#define STOREL sdl
#define STORER sdr
#define STORE sd
#define ADD   daddu		/* XXX */
#define ADDIU daddiu
#define ADDU  daddu
#define SUB   dsubu		/* XXX */
#define SUBU  dsubu
#define SRL   dsrl
#define SRA   dsra
#define SLL   dsll
#define SLLV  dsllv
#define SRLV  dsrlv
#define NBYTES 8
#define LOG_NBYTES 3
	
#else
	
#define LOAD  lw
#define LOADL lwl
#define LOADR lwr
#define STOREL swl
#define STORER swr
#define STORE sw
#define ADD   addu		/* XXX */
#define ADDIU addiu
#define ADDU  addu
#define SUB   subu		/* XXX */
#define SUBU  subu
#define SRL   srl
#define SLL   sll
#define SRA   sra
#define SLLV  sllv
#define SRLV  srlv
#define NBYTES 4
#define LOG_NBYTES 2
	
#endif /* USE_DOUBLE */
     
#if defined(__MIPSEB__)
#define LDFIRST LOADL
#define LDREST  LOADR
#define STFIRST STOREL
#define STREST  STORER
#define SHIFT_CPY SRLV
#endif
	
#if defined(__MIPSEL__)
#define LDFIRST LOADR
#define LDREST  LOADL
#define STFIRST STORER
#define STREST  STOREL
#define SHIFT_CPY SLLV
#endif

#define PREF_LOAD           0
#define PREF_STORE          1
#define PREF_LOAD_STREAMED  4
#define PREF_STORE_STREAMED 5
#define PREF_NUDGE          25
