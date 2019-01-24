/* $Id: unhandler_vxworks.h,v 1.1 2004/05/16 23:05:10 cgd Exp $ */

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

#include <vxWorks.h>
#include <string.h>
#include <sys/types.h>
#include <arch/mips/esfMips.h>

#define BUILD_HANDLERS
#define DEMO_FUNCTION		un_demo

#define INT16_TYPE		int16_t
#define UINT16_TYPE		uint16_t
#define INT32_TYPE		int32_t
#define UINT32_TYPE		uint32_t
#define INT64_TYPE	        long long 	
#define UINT64_TYPE		unsigned long long

#define EXC_RETURN_TYPE		void	
#define EXC_ARGS		unsigned code, ESFMIPS *ef, REG_SET *regs
#define EXC_ARGS_CALL		code, ef, regs
#define EXC_ARGS_CALL_MACRO	(code, ef, regs)
#define EXC_FRAME		ef
#define EXC_RETURN(ef)		return
#define EXC_FATAL(arglist)	excExcHandle arglist; return	
#define EXC_INTPTR_TYPE		signed long 
#define EXC_REGSAVE_TYPE	mips_reg_t

#define GET_EXC_PC(ef)		((EXC_INTPTR_TYPE)((ef)->esfRegs.pc))
#define SET_EXC_PC(ef, v)	((ef)->esfRegs.pc = (INSTR *)(v))
#define GET_EXC_CAUSE(ef)	((ef)->cause)

/* Note that the following macros are required to implement the $0 == 0
   semantics!  */
#define GET_EXC_REG(ef, r) \
  ((r) == 0 ? 0 : (ef)->esfRegs.gpreg[(r)])
#define SET_EXC_REG(ef, r, v)				\
  do							\
    {							\
      if ((r) != 0)					\
	(ef)->esfRegs.gpreg[(r)] = (v);	\
    }							\
  while (0)

#define GET_EXC_FP_COND_CODES(ef)       ((((ef)->fpcsr >> 23) & 0x1) \
	                                 | (((ef)->fpcsr >> 24) & 0xfe))
	
#define COPYIN(dest, src, size)		(bcopy ((const char *)(src), (char *)(dest), (size)) , 0)
#define COPYOUT(dest, src, size)	(bcopy ((const char *)(src), (char *)(dest), (size)) , 0)

#define REGISTER_TYPE	_RType

#define M_CAUSE_BD	(1 << 31)

extern void excExcHandle(unsigned code, ESFMIPS *ef, REG_SET *regs);

#ifdef BUILD_HANDLERS
EXC_RETURN_TYPE unaligned_load_handler (EXC_ARGS);
EXC_RETURN_TYPE unaligned_store_handler (EXC_ARGS);
#endif
