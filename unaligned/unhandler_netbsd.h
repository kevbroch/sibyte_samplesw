/* $Id: unhandler_netbsd.h,v 1.2 2004/03/20 06:56:35 cgd Exp $ */

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

#ifdef _KERNEL
# include <sys/systm.h>
# include <sys/proc.h>
# include <sys/user.h>
# include <mips/regnum.h>
# define BUILD_HANDLERS
#else
# include <sys/types.h>
#endif

#define INT16_TYPE		int16_t
#define UINT16_TYPE		uint16_t
#define INT32_TYPE		int32_t
#define UINT32_TYPE		uint32_t
#define INT64_TYPE		int64_t
#define UINT64_TYPE		uint64_t

#define EXC_RETURN_TYPE		int
#define EXC_ARGS		struct lwp *l, struct frame *f, unsigned cause
#define EXC_ARGS_CALL		l, f, cause
#define EXC_ARGS_CALL_MACRO	(l, f, cause)
#define EXC_FRAME		f
#define EXC_RETURN(ef)		return 0
#define EXC_FATAL(arglist)	return 1
#define EXC_INTPTR_TYPE		intptr_t
#define EXC_REGSAVE_TYPE	mips_reg_t

#define GET_EXC_PC(ef)		((ef)->f_regs[_R_PC])
#define SET_EXC_PC(ef, v)	((ef)->f_regs[_R_PC] = (v))
#define GET_EXC_CAUSE(ef)	(cause)
/* Note that the following macros are required to implement the $0 == 0
   semantics!  */
#define GET_EXC_REG(ef, r) \
  ((r) == 0 ? 0 : (ef)->f_regs[(r) - (1 - _R_AST)])
#define SET_EXC_REG(ef, r, v)				\
  do							\
    {							\
      if ((r) != 0)					\
	(ef)->f_regs[(r) - (1 - _R_AST)] = (v);	\
    }							\
  while (0)
#define GET_RAW_FSR(l)			PCB_FSR(&l->l_addr->u_pcb)
#define GET_EXC_FP_COND_CODES(ef)	(((GET_RAW_FSR(l) >> 23) & 0x1) \
					 | ((GET_RAW_FSR(l) >> 24) & 0xfe))

#define COPYIN(dest, src, size)		(copyin ((src), (dest), (size)) != 0)
#define COPYOUT(dest, src, size)	(copyout ((src), (dest), (size)) != 0)

#define REGISTER_TYPE	mips_reg_t

#define M_CAUSE_BD	(1 << 31)

#ifdef BUILD_HANDLERS
EXC_RETURN_TYPE unaligned_load_handler (EXC_ARGS);
EXC_RETURN_TYPE unaligned_store_handler (EXC_ARGS);
#endif
