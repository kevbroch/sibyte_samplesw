/* $Id: unhandler_exlib.h,v 1.1 2004/03/20 06:45:02 cgd Exp $ */

/*
 * Copyright 2004
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

/* To port this to a different environment, you will probably need to change
   the includes and defines below.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exlib.h>
#include <sb1250-include/sbmips.h>

#define BUILD_HANDLERS
#define SET_EXLIB_HANDLERS
#define DO_TIMINGS

#define INT16_TYPE		short
#define UINT16_TYPE		unsigned short
#define INT32_TYPE		int
#define UINT32_TYPE		unsigned int
#define INT64_TYPE		long long
#define UINT64_TYPE		unsigned long long

#define EXC_RETURN_TYPE		void
#define EXC_ARGS		unsigned int code, struct exframe *ef
#define EXC_ARGS_CALL		code, ef
#define EXC_ARGS_CALL_MACRO	(code, ef)
#define EXC_FRAME		ef
#define EXC_RETURN(ef)		return
#define _EXC_FATAL(code, ef)	exhandler_panic (code, ef)
#define EXC_FATAL(arglist)	_EXC_FATAL arglist
#define EXC_INTPTR_TYPE		long
#define EXC_REGSAVE_TYPE	unsigned long long

#define GET_EXC_PC(ef)		((ef)->ef_pc)
#define SET_EXC_PC(ef, v)	((ef)->ef_pc = (v))
#define GET_EXC_CAUSE(ef)	((ef)->ef_cause)
/* Note that the following macros are required to implement the $0 == 0
   semantics!  */
#define GET_EXC_REG(ef, r)	((r) == 0 ? 0 : (ef)->ef_regs[(r)])
#define SET_EXC_REG(ef, r, v)		\
  do					\
    {					\
      if ((r) != 0)			\
	(ef)->ef_regs[(r)] = (v);	\
    }					\
  while (0)
#ifndef __mips_soft_float
#define GET_EXC_FP_COND_CODES(ef)	((((ef)->ef_fcsr >> 23) & 0x1) \
					 | (((ef)->ef_fcsr >> 24) & 0xfe))
#else
#define GET_EXC_FP_COND_CODES(ef)	0
#endif

#define COPYIN(dest, src, size)		(memcpy ((dest), (src), (size)), 0)
#define COPYOUT(dest, src, size)	(memcpy ((dest), (src), (size)), 0)

/* These should be left as-is, but note that this code won't necessarly
   work right if compiled w/ 32-bit GPRs on a processor that has 64-bit
   registers.  */
#ifdef __mips64
#define REGISTER_TYPE	INT64_TYPE
#else
#define REGISTER_TYPE	INT32_TYPE
#endif

#ifdef BUILD_HANDLERS
EXC_RETURN_TYPE unaligned_load_handler (EXC_ARGS);
EXC_RETURN_TYPE unaligned_store_handler (EXC_ARGS);
#endif
