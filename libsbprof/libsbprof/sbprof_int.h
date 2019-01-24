/* $Id: sbprof_int.h,v 1.5 2003/05/16 02:08:37 cgd Exp $ */

/*
 * Copyright 2001, 2002, 2003
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

#ifndef _SIBYTE_SBPROF_INT_H_
#define _SIBTYE_SBPROF_INT_H_

#include <sys/types.h>
#include <sb1250-include/sbmips.h>

#include "sbprof.h"

#ifdef LIBSBPROF_PROVIDE_EXTERN_INLINE_CODE
#define EXTERN_INLINE
#else
#define EXTERN_INLINE extern inline
#endif

/* Default sizes of profiling buffers, if SBPROF_BUFSIZE_DEFAULT
   is passed to sbprof_init().  */
#define	SBPROF_CPU_BUFSIZE_DEFAULT	(24 * 1024 * 1024)
#define	SBPROF_ZB_BUFSIZE_DEFAULT	(12 * 1024 * 1024)

/* This code should support multiple CPUs eventually, but
   currently does not.  */
#define MAX_CPUS 1

/* Cope with some lossage caused by these defines not being present
   by default in the headers.  */
#undef int64_t
#undef uint64_t

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

#ifdef __mips64
EXTERN_INLINE unsigned long long
sbprof_read_csr(unsigned long long *va)
{
  return *(volatile unsigned long long *)va;
}

EXTERN_INLINE void
sbprof_write_csr(unsigned long long *va, unsigned long long val)
{
  *(volatile unsigned long long *)va = val;
}
#endif /* __mips64 */

#define SBPROF_READ_CSR(pa) \
  sbprof_read_csr ((void *)(PHYS_TO_K1 (pa)))
#define SBPROF_WRITE_CSR(pa, val) \
  sbprof_write_csr ((void *)(PHYS_TO_K1 (pa)), (val))

int sbprof_cpu_init (long);
int sbprof_zb_init (long);
void sbprof_get_cpu_buf (void **, size_t *);
void sbprof_get_zb_buf (void **, size_t *);

/* Upon return, Status.IE is clear and interrupts are blocked */
#define disable_intr()	intr_clear_ie(NULL)

/* Upon return, Status.IE is set, but there may be a few
   cycles more delay before interrupts are truly enabled. */
#define enable_intr()	intr_set_ie(NULL)

/* Writes to cp0 regs happen at pipestage 8; interrupts at pipestage 4.
   3 cycles of delay are needed between writing a cp0 reg to disable
   interrupts or a particular counter before the write takes effect. */
#define PAUSE()							\
  __asm__ __volatile__ (".set push; .set mips64; "		\
		        "ssnop; ssnop; ssnop; "			\
			".set pop")

#define SYNC()							\
    __asm__ __volatile__ (".set push; .set mips64;"		\
			  "sync;"				\
			  ".set pop" : : : "memory")

#endif /* _SIBYTE_SBPROF_INT_H_ */
