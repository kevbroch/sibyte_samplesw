/* $Id: psingle.h,v 1.4 2004/12/07 06:42:39 cgd Exp $ */

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

/* psingle.h

   Provide types, macros, and functions to allow easier access to
   Paired Single floating point operations.  (Eventually, these should
   be supported as vector operations by GCC, but that won't be for a
   while.)

   Note that this implementation is not complete.  It is meant as an
   example of use of Paired Single operations only, not a complete or
   proper implementation of them in the C language.  (In GCC, Paired
   Single operations should be implemented as V2SF vector operations.)

   Public types:
 
 	psingle

	  A Paired Single value (use this for variables).  (Convert to
	  and from "float" for direct access to the FP values held
	  within.)
 
   Public functions and macros:

	psingle psingle_load (void *pspv);

	  Load a psingle value from memory.  (The pointer must be
	  properly aligned.)

	void psingle_store (void *pspv, psingle *);

	  Store a psingle value to memory.  (The pointer must be
	  properly aligned.)

        psingle psingle_from_floats (float upper, float lower);

	  Convert a pair of 'float' values into a Paired Single value.

        float float_from_psingle_lower (psingle ps);
        float float_from_psingle_upper (psingle ps);

	  Extract, round, and return the lower or upper half of a Paired
	  Single value.

        psingle psingle_pll (psingle ps1, psingle ps2);
        psingle psingle_plu (psingle ps1, psingle ps2);
        psingle psingle_pul (psingle ps1, psingle ps2);
        psingle psingle_puu (psingle ps1, psingle ps2);

	  Merge a pair of paired-single values, with realignment.
	  (See MIPS64 Volume II for description of opcodes PLL.PS,
	  PLU.PS, etc.)

	psingle psingle_abs (psingle ps);
	psingle psingle_mov (psingle ps);
	psingle psingle_neg (psingle ps);
	psingle psingle_add (psingle ps1, psingle ps2);
	psingle psingle_mul (psingle ps1, psingle ps2);
	psingle psingle_sub (psingle ps1, psingle ps2);
	psingle psingle_madd (psingle ps1, psingle ps2, psingle ps3);
	psingle psingle_msub (psingle ps1, psingle ps2, psingle ps3);
	psingle psingle_nmadd (psingle ps1, psingle ps2, psingle ps3);
	psingle psingle_nmsub (psingle ps1, psingle ps2, psingle ps3);

	  Perform the named arithmetic operation on the given
	  Paired Single values.
 
   See also the macros and functions defined in psingle_sb1.h.  */

#ifndef _SIBYTE_PSINGLE_H_
#define _SIBYTE_PSINGLE_H_

#ifdef __mips_paired_single_float
typedef float __attribute__ ((vector_size (8))) psingle;

#define _def_psingle_unary(fn, op, expr) 				\
  static inline psingle fn (psingle)					\
    __attribute__ ((pure, always_inline));				\
  static inline psingle							\
  fn (psingle _ps)							\
  {									\
    psingle _dest;							\
    _dest = (expr);							\
    return _dest;							\
  }

#define _def_psingle_binary(fn, op, expr) 				\
  static inline psingle fn (psingle, psingle)				\
    __attribute__ ((pure, always_inline));				\
  static inline psingle							\
  fn (psingle _ps1, psingle _ps2)					\
  {									\
    psingle _dest;							\
    _dest = (expr);							\
    return _dest;							\
  }

#define _def_psingle_ternary(fn, op, expr) 				\
  static inline psingle fn (psingle, psingle, psingle)			\
    __attribute__ ((pure, always_inline));				\
  static inline psingle							\
  fn (psingle _ps1, psingle _ps2, psingle _ps3)				\
  {									\
    psingle _dest;							\
    _dest = (expr);							\
    return _dest;							\
  }
#else
typedef float psingle __attribute__ ((mode (DF)));

#define _def_psingle_unary(fn, op, expr) 				\
  static inline psingle fn (psingle)					\
    __attribute__ ((pure, always_inline));				\
  static inline psingle							\
  fn (psingle _ps)							\
  {									\
    psingle _dest;							\
    __asm__ (op " %0, %1" : "=f"(_dest) : "f"(_ps));			\
    return _dest;							\
  }

#define _def_psingle_binary(fn, op, expr) 				\
  static inline psingle fn (psingle, psingle)				\
    __attribute__ ((pure, always_inline));				\
  static inline psingle							\
  fn (psingle _ps1, psingle _ps2)					\
  {									\
    psingle _dest;							\
    __asm__ (op " %0, %1, %2" : "=f"(_dest) : "f"(_ps1), "f"(_ps2));	\
    return _dest;							\
  }

#define _def_psingle_ternary(fn, op, expr) 				\
  static inline psingle fn (psingle, psingle, psingle)			\
    __attribute__ ((pure, always_inline));				\
  static inline psingle							\
  fn (psingle _ps1, psingle _ps2, psingle _ps3)				\
  {									\
    psingle _dest;							\
    __asm__ (op " %0, %1, %2, %3" : "=f"(_dest)				\
			      : "f"(_ps1), "f"(_ps2), "f"(_ps3));	\
    return _dest;							\
  }
#endif


/* Paired Single loads and stores.  */

static inline psingle
psingle_load (void *pspv)
{
#ifdef __mips_paired_single_float
  return *(psingle *)pspv;
#else
  psingle rv;
  char *p = pspv;
  __asm__ ("ldc1 %0, %1" : "=f"(rv) : "m"(*p));
  return rv;
#endif
}

static inline void
psingle_store (void *pspv, psingle val)
{
#ifdef __mips_paired_single_float
  *(psingle *)pspv = val;
#else
  char *p = pspv;
  __asm__ ("sdc1 %1, %0" : "=m"(*p) : "f"(val));
#endif
}


/* Conversion to/from float values, and data shuffling.  */
static inline psingle psingle_from_floats (float upper, float lower)
  __attribute__ ((pure, always_inline));
static inline psingle
psingle_from_floats (float upper, float lower)
{
  psingle rv;
#ifdef __mips_paired_single_float
#ifdef __MIPSEB__
  rv = (psingle) { upper, lower };
#else
  rv = (psingle) { lower, upper };
#endif
#else
  __asm__ ("cvt.ps.s %0, %1, %2" : "=f"(rv) : "f"(upper), "f"(lower));
#endif
  return rv;
}

static inline float float_from_psingle_lower (psingle ps)
  __attribute__ ((pure, always_inline));
static inline float
float_from_psingle_lower (psingle ps)
{
  float rv;
#ifdef __mips_paired_single_float
  rv = __builtin_mips_cvt_s_pl (ps);
#else
  __asm__ ("cvt.s.pl %0, %1" : "=f"(rv) : "f"(ps));
#endif
  return rv;
}

static inline float float_from_psingle_upper (psingle ps)
  __attribute__ ((pure, always_inline));
static inline float
float_from_psingle_upper (psingle ps)
{
  float rv;
#ifdef __mips_paired_single_float
  rv = __builtin_mips_cvt_s_pu (ps);
#else
  __asm__ ("cvt.s.pu %0, %1" : "=f"(rv) : "f"(ps));
#endif
  return rv;
}

_def_psingle_binary (psingle_pll, "pll.ps", __builtin_mips_pll_ps (_ps1, _ps2))
_def_psingle_binary (psingle_plu, "plu.ps", __builtin_mips_plu_ps (_ps1, _ps2))
_def_psingle_binary (psingle_pul, "pul.ps", __builtin_mips_pul_ps (_ps1, _ps2))
_def_psingle_binary (psingle_puu, "puu.ps", __builtin_mips_puu_ps (_ps1, _ps2))


/* Unary arithmetic ops.  */

_def_psingle_unary (psingle_abs, "abs.ps", __builtin_mips_abs_ps (_ps))
_def_psingle_unary (psingle_mov, "mov.ps", _ps)
_def_psingle_unary (psingle_neg, "neg.ps", (- _ps))


/* Binary arithmetic ops.  */

_def_psingle_binary (psingle_add, "add.ps", (_ps1 + _ps2))
_def_psingle_binary (psingle_mul, "mul.ps", (_ps1 * _ps2))
_def_psingle_binary (psingle_sub, "sub.ps", (_ps1 - _ps2))


/* Ternary arithmetic ops.  */

/* madd = (b * c) + a */
_def_psingle_ternary (psingle_madd, "madd.ps", ((_ps2 * _ps3) + _ps1))

/* msub = (b * c) - a */
_def_psingle_ternary (psingle_msub, "msub.ps", ((_ps2 * _ps3) - _ps1))

/* nmadd = - ((b * c) + a) */
_def_psingle_ternary (psingle_nmadd, "nmadd.ps", (-((_ps2 * _ps3) + _ps1)))

/* nmsub = - ((b * c) - a) a.k.a. (a - (b * c)) */
_def_psingle_ternary (psingle_nmsub, "nmsub.ps", (-((_ps2 * _ps3) - _ps1)))


#include "psingle_sb1.h"

#endif /* _SIBYTE_PSINGLE_H_ */
