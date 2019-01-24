/* $Id: mdmx.h,v 1.23 2003/05/09 04:48:06 cgd Exp $ */

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

/* mdmx.h

   Provide types, macros, and functions to allow easier access to MDMX
   resources.

   Note that this implementation is meant as an example of use of MDMX
   operations only, not a complete or proper implementation of them in
   the C language.  (In GCC, MDMX operations should be implemented as
   V8QI vector operations.)
 
   Public types:
 
 	mdmxint: an integer the size of an MDMX register.
 		 Print using format string PRIxMDMXINT.
 
 	mdmxreg: an MDMX value (use this for most variables).
 		 To print, convert to mdmxint with mdmx_int_from_reg().
 		 To compare, convert to mdmxint or write your own
 		 comparison asms.
 
   Public functions:
 
 	void mdmx_enable (void)
 		Enable access to MDMX resources (including SB-1
 		extensions).
 
 	mdmxint mdmx_int_from_reg (mdmxreg rv)
 		Convert from an MDMX register value to an integer.
 
 	mdmxreg mdmx_reg_from_int (mdmxint rv)
 		Convert from an integer to a MDMX register value;
 
 	mdmxreg mdmx_reg_load (void *rpv)
 		Load a value from memory into an MDMX register.
 
 	void mdmx_reg_store (void *rpv, mdmxreg rv)
 		Store an MDMX register into memory.
 
   See also the macros and functions defined in mdmx_ob.h and
   mdmx_ob_sb1.h.  */

#ifndef _SIBYTE_MDMX_H_
#define _SIBYTE_MDMX_H_

typedef unsigned int mdmxint __attribute__ ((mode (DI)));
#define PRIxMDMXINT	"llx"

typedef float mdmxreg __attribute__ ((mode (DF)));

typedef union {
  mdmxint i;
  mdmxreg r;
} _mdmxreg_or_int;

#define _mdmx_alnop_i(op, vs, vt, imm)					\
  ({									\
    mdmxreg _dest;							\
    __asm__ (op " %0, %1, %2, %3" : "=f"(_dest)				\
                                  : "f"(vs), "f"(vt), "i"(imm));	\
    _dest;								\
  })

#define _mdmx_alnop_r(op, vs, vt, rs)					\
  ({									\
    mdmxreg _dest;							\
    __asm__ (op " %0, %1, %2, %3" : "=f"(_dest)				\
                                  : "f"(vs), "f"(vt), "r"(rs));		\
    _dest;								\
  })


#define _mdmx_aop_v(op, vs, vt)						\
  __asm__ __volatile__ (op " %0, %1" : : "f"(vs), "f"(vt))

#define _mdmx_aop_ev(op, vs, vt, vtsel)					\
  __asm__ __volatile__ (op " %0, %1[%2]" : : "f"(vs), "f"(vt), "i"(vtsel))

#define _mdmx_aop_cv(op, vs, vtconst)					\
  __asm__ __volatile__ (op " %0, %1" : : "f"(vs), "i"(vtconst))


#define _mdmx_raop_v(op, vt)						\
  ({									\
    mdmxreg _dest;							\
    __asm__ __volatile__ (op " %0, %1" : "=f"(_dest) : "f"(vt));	\
    _dest;								\
  })

#define _mdmx_raop_ev(op, vt, vtsel)					\
  ({									\
    mdmxreg _dest;							\
    __asm__ __volatile__ (op " %0, %1[%2]" : "=f"(_dest)		\
                                           : "f"(vt), "i"(vtsel));	\
    _dest;								\
  })

#define _mdmx_raop_cv(op, vtconst)					\
  ({									\
    mdmxreg _dest;							\
    __asm__ __volatile__ (op " %0, %1" : "=f"(_dest) : "i"(vtconst));	\
    _dest;								\
  })


#define _mdmx_rop_v(op, vs, vt)						\
  ({									\
    mdmxreg _dest;							\
    __asm__ (op " %0, %1, %2" : "=f"(_dest) : "f"(vs), "f"(vt));	\
    _dest;								\
  })

#define _mdmx_rop_ev(op, vs, vt, vtsel)					\
  ({									\
    mdmxreg _dest;							\
    __asm__ (op " %0, %1, %2[%3]" : "=f"(_dest)				\
                                  : "f"(vs), "f"(vt), "i"(vtsel));	\
    _dest;								\
  })

#define _mdmx_rop_cv(op, vs, vtconst)					\
  ({									\
    mdmxreg _dest;							\
    __asm__ (op " %0, %1, %2" : "=f"(_dest) : "f"(vs), "i"(vtconst));	\
    _dest;								\
  })

#define _mdmx_zero(op)						\
  ({									\
    mdmxreg _dest;							\
    __asm__ (op " %0, %0, %0" : "=f"(_dest));				\
    _dest;								\
  })


static inline void
mdmx_enable (void)
{
#if !defined(__linux__) && !defined(__NetBSD__)
  /* Enable MDMX.  Note that OSes which support 64-bit operation should
     handle this automatically when the first MDMX operation occurs!  */
  int _sr;
  __asm__ ("mfc0 %0, $12" : "=r"(_sr));
  _sr |= (1 << 24);	/* Set SR:MX.  Enable MDMX.  */
  __asm__ __volatile__ ("mtc0 %0, $12" : : "r"(_sr));
  __asm__ __volatile__ ("	.set push	\n"
                        "	.set noreorder	\n"
                        "	ssnop		\n"
                        "	bnezl	$0, 1f	\n"
                        "1:	ssnop		\n"
                        "	.set pop	");
#endif
}

static inline mdmxint
mdmx_int_from_reg (mdmxreg rv)
{
  mdmxint iv;
  __asm__ ("dmfc1 %0, %1" : "=r"(iv) : "f"(rv));
  return iv;
}

static inline mdmxreg
mdmx_reg_from_int (mdmxint iv)
{
  mdmxreg rv;
  __asm__ ("dmtc1 %1, %0" : "=f"(rv) : "r"(iv));
  return rv;
}

static inline mdmxreg
mdmx_reg_load (void *rpv)
{
  _mdmxreg_or_int *rp = rpv;
  mdmxreg rv;
  __asm__ ("ldc1 %0, %1" : "=f"(rv) : "m"(*rp));
  return rv;
}

static inline void
mdmx_reg_store (void *rpv, mdmxreg rv)
{
  _mdmxreg_or_int *rp = rpv;
  __asm__ ("sdc1 %1, %0" : "=m"(*rp) : "f"(rv));
}

#include "mdmx_ob.h"
#include "mdmx_sb1.h"

#endif /* _SIBYTE_MDMX_H_ */
