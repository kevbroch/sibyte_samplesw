/* $Id: exlib_int.h,v 1.18 2005/01/05 05:43:59 cgd Exp $ */

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

#include <sb1250-include/sbmips.h>
#include "exlib.h"

#define EXVEC_SIZE		0x80	/* Maximum vector size.  */

#ifndef __LANGUAGE_ASSEMBLY
#define	EXVEC_ADDR_TLB		((void *)(long)0xffffffff80000000ULL)
#define	EXVEC_ADDR_XTLB		((void *)(long)0xffffffff80000080ULL)
#define	EXVEC_ADDR_CERR		((void *)(long)0xffffffff80000100ULL)
#define	EXVEC_ADDR_CERR_K1	((void *)(long)0xffffffffa0000100ULL)
#define	EXVEC_ADDR_GEN		((void *)(long)0xffffffff80000180ULL)

/* The _flush_cache() function must be provided by a system library.  */
#define BCACHE 3		/* Flush both caches.  */
extern int _flush_cache(void *addr, int nbytes, int);

#define INSTALL_VECTOR(dest, code, size)				\
  do									\
    {									\
      memcpy ((dest), (code), (size));					\
      _flush_cache ((dest), (size), BCACHE);				\
    }									\
  while (0)

extern char exlib_vec_cerr_stub_mips32[EXVEC_SIZE];
extern char exlib_vec_general_mips32[EXVEC_SIZE];
void exlib_return_mips32 (struct exframe * /* ef */)
  __attribute__ ((__noreturn__));

extern char exlib_vec_cerr_stub_mips64[EXVEC_SIZE];
extern char exlib_vec_general_mips64[EXVEC_SIZE];
void exlib_return_mips64 (struct exframe * /* ef */)
  __attribute__ ((__noreturn__));

extern int exlib_mips64, exlib_fpu, exlib_mdmx;

void exlib_dispatch(struct exframe *);
#ifndef __mips_soft_float
void exframe_restore_fp(struct exframe *ef);
#endif

#endif /* !__LANGUAGE_ASSEMBLY */
