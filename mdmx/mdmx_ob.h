/* $Id: mdmx_ob.h,v 1.16 2003/05/09 04:48:06 cgd Exp $ */

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

/* mdmx_ob.h

   Provide macros and functions which can be used to emit MDMX .ob
   format operations.  Do not include this file directly, instead
   include mdmx.h.

   All of the functions and macros defined in this file are intended
   to be usable by application code.

   Many of the vector macros come in three forms: _v, _ev, and _cv.
   The '_v' form uses the entire vector specified by 'vt'.  The '_ev'
   form uses a vector in which each element is a copy of element 'e'
   of the vector specified by 'vt'.  Finally, the '_cv' form uses a
   vector in which each element is the constant 'c' (which must have a
   value in the range [0..31]).  */

#if !defined(_SIBYTE_MDMX_H_) || defined(_SIBYTE_MDMX_OB_H_)
#error do not include this file directly; include mdmx.h
#endif
#if defined(_SIBYTE_MDMX_H_) && !defined(_SIBYTE_MDMX_OB_H_)
#define _SIBYTE_MDMX_OB_H_

#define	mdmx_zero_ob()			_mdmx_zero ("xor.ob")

/* ADD.OB */
#define mdmx_add_ob_v(vs, vt)		_mdmx_rop_v ("add.ob", (vs), (vt))
#define mdmx_add_ob_ev(vs, vt, e)	_mdmx_rop_ev ("add.ob", (vs), (vt), (e))
#define mdmx_add_ob_cv(vs, c)		_mdmx_rop_cv ("add.ob", (vs), (c))

/* ADDA.OB */
#define mdmx_adda_ob_v(vs, vt)		_mdmx_aop_v ("adda.ob", (vs), (vt))
#define mdmx_adda_ob_ev(vs, vt, e)	_mdmx_aop_ev ("adda.ob", (vs), (vt), (e))
#define mdmx_adda_ob_cv(vs, c)		_mdmx_aop_cv ("adda.ob", (vs), (c))

/* ADDL.OB */
#define mdmx_addl_ob_v(vs, vt)		_mdmx_aop_v ("addl.ob", (vs), (vt))
#define mdmx_addl_ob_ev(vs, vt, e)	_mdmx_aop_ev ("addl.ob", (vs), (vt), (e))
#define mdmx_addl_ob_cv(vs, c)		_mdmx_aop_cv ("addl.ob", (vs), (c))

/* ALNI.OB */
#define mdmx_alni_ob(vs, vt, imm)	_mdmx_alnop_i ("alni.ob", (vs), (vt), (imm))

/* ALNV.OB */
#define mdmx_alnv_ob(vs, vt, rs)	_mdmx_alnop_r ("alnv.ob", (vs), (vt), (rs))

/* AND.OB */
#define mdmx_and_ob_v(vs, vt)		_mdmx_rop_v ("and.ob", (vs), (vt))
#define mdmx_and_ob_ev(vs, vt, e)	_mdmx_rop_ev ("and.ob", (vs), (vt), (e))
#define mdmx_and_ob_cv(vs, c)		_mdmx_rop_cv ("and.ob", (vs), (c))

/* C.EQ.OB must be used very carefully; write your own asms. */

/* C.LE.OB must be used very carefully; write your own asms. */

/* C.LT.OB must be used very carefully; write your own asms. */

/* MAX.OB */
#define mdmx_max_ob_v(vs, vt)		_mdmx_rop_v ("max.ob", (vs), (vt))
#define mdmx_max_ob_ev(vs, vt, e)	_mdmx_rop_ev ("max.ob", (vs), (vt), (e))
#define mdmx_max_ob_cv(vs, c)		_mdmx_rop_cv ("max.ob", (vs), (c))

/* MIN.OB */
#define mdmx_min_ob_v(vs, vt)		_mdmx_rop_v ("min.ob", (vs), (vt))
#define mdmx_min_ob_ev(vs, vt, e)	_mdmx_rop_ev ("min.ob", (vs), (vt), (e))
#define mdmx_min_ob_cv(vs, c)		_mdmx_rop_cv ("min.ob", (vs), (c))

/* MUL.OB */
#define mdmx_mul_ob_v(vs, vt)		_mdmx_rop_v ("mul.ob", (vs), (vt))
#define mdmx_mul_ob_ev(vs, vt, e)	_mdmx_rop_ev ("mul.ob", (vs), (vt), (e))
#define mdmx_mul_ob_cv(vs, c)		_mdmx_rop_cv ("mul.ob", (vs), (c))

/* MULA.OB */
#define mdmx_mula_ob_v(vs, vt)		_mdmx_aop_v ("mula.ob", (vs), (vt))
#define mdmx_mula_ob_ev(vs, vt, e)	_mdmx_aop_ev ("mula.ob", (vs), (vt), (e))
#define mdmx_mula_ob_cv(vs, c)		_mdmx_aop_cv ("mula.ob", (vs), (c))

/* MULL.OB */
#define mdmx_mull_ob_v(vs, vt)		_mdmx_aop_v ("mull.ob", (vs), (vt))
#define mdmx_mull_ob_ev(vs, vt, e)	_mdmx_aop_ev ("mull.ob", (vs), (vt), (e))
#define mdmx_mull_ob_cv(vs, c)		_mdmx_aop_cv ("mull.ob", (vs), (c))

/* MULS.OB */
#define mdmx_muls_ob_v(vs, vt)		_mdmx_aop_v ("muls.ob", (vs), (vt))
#define mdmx_muls_ob_ev(vs, vt, e)	_mdmx_aop_ev ("muls.ob", (vs), (vt), (e))
#define mdmx_muls_ob_cv(vs, c)		_mdmx_aop_cv ("muls.ob", (vs), (c))

/* MULSL.OB */
#define mdmx_mulsl_ob_v(vs, vt)		_mdmx_aop_v ("mulsl.ob", (vs), (vt))
#define mdmx_mulsl_ob_ev(vs, vt, e)	_mdmx_aop_ev ("mulsl.ob", (vs), (vt), (e))
#define mdmx_mulsl_ob_cv(vs, c)		_mdmx_aop_cv ("mulsl.ob", (vs), (c))

/* NOR.OB */
#define mdmx_nor_ob_v(vs, vt)		_mdmx_rop_v ("nor.ob", (vs), (vt))
#define mdmx_nor_ob_ev(vs, vt, e)	_mdmx_rop_ev ("nor.ob", (vs), (vt), (e))
#define mdmx_nor_ob_cv(vs, c)		_mdmx_rop_cv ("nor.ob", (vs), (c))

/* OR.OB */
#define mdmx_or_ob_v(vs, vt)		_mdmx_rop_v ("or.ob", (vs), (vt))
#define mdmx_or_ob_ev(vs, vt, e)	_mdmx_rop_ev ("or.ob", (vs), (vt), (e))
#define mdmx_or_ob_cv(vs, c)		_mdmx_rop_cv ("or.ob", (vs), (c))

/* PICKF.OB must be used very carefully; write your own asms. */

/* PICKT.OB must be used very carefully; write your own asms. */

/* RACH.OB is provided by mdmx_acc_read_ob(). */

/* RACL.OB is provided by mdmx_acc_read_ob(). */

/* RACM.OB is provided by mdmx_acc_read_ob(). */

/* RNAU.OB */
#define mdmx_rnau_ob_v(vt)		_mdmx_raop_v ("rnau.ob", (vt))
#define mdmx_rnau_ob_ev(vt, e)		_mdmx_raop_ev ("rnau.ob", (vt), (e))
#define mdmx_rnau_ob_cv(c)		_mdmx_raop_cv ("rnau.ob", (c))

/* RNEU.OB */
#define mdmx_rneu_ob_v(vt)		_mdmx_raop_v ("rneu.ob", (vt))
#define mdmx_rneu_ob_ev(vt, e)		_mdmx_raop_ev ("rneu.ob", (vt), (e))
#define mdmx_rneu_ob_cv(c)		_mdmx_raop_cv ("rneu.ob", (c))

/* RZU.OB */
#define mdmx_rzu_ob_v(vt)		_mdmx_raop_v ("rzu.ob", (vt))
#define mdmx_rzu_ob_ev(vt, e)		_mdmx_raop_ev ("rzu.ob", (vt), (e))
#define mdmx_rzu_ob_cv(c)		_mdmx_raop_cv ("rzu.ob", (c))

/* SHFL.op.OB */
#define mdmx_shfl_mixh_ob(vs, vt)	_mdmx_rop_v ("shfl.mixh.ob", (vs), (vt))
#define mdmx_shfl_mixl_ob(vs, vt)	_mdmx_rop_v ("shfl.mixl.ob", (vs), (vt))
#define mdmx_shfl_pach_ob(vs, vt)	_mdmx_rop_v ("shfl.pach.ob", (vs), (vt))
/* shfl.upsl.ob ignores 'vt', so don't make callers supply a dummy.  */
#define mdmx_shfl_upsl_ob(vs)		_mdmx_rop_v ("shfl.upsl.ob", (vs), (vs))

/* SLL.OB */
#define mdmx_sll_ob_v(vs, vt)		_mdmx_rop_v ("sll.ob", (vs), (vt))
#define mdmx_sll_ob_ev(vs, vt, e)	_mdmx_rop_ev ("sll.ob", (vs), (vt), (e))
#define mdmx_sll_ob_cv(vs, c)		_mdmx_rop_cv ("sll.ob", (vs), (c))

/* SRL.OB */
#define mdmx_srl_ob_v(vs, vt)		_mdmx_rop_v ("srl.ob", (vs), (vt))
#define mdmx_srl_ob_ev(vs, vt, e)	_mdmx_rop_ev ("srl.ob", (vs), (vt), (e))
#define mdmx_srl_ob_cv(vs, c)		_mdmx_rop_cv ("srl.ob", (vs), (c))

/* SUB.OB */
#define mdmx_sub_ob_v(vs, vt)		_mdmx_rop_v ("sub.ob", (vs), (vt))
#define mdmx_sub_ob_ev(vs, vt, e)	_mdmx_rop_ev ("sub.ob", (vs), (vt), (e))
#define mdmx_sub_ob_cv(vs, c)		_mdmx_rop_cv ("sub.ob", (vs), (c))

/* SUBA.OB */
#define mdmx_suba_ob_v(vs, vt)		_mdmx_aop_v ("suba.ob", (vs), (vt))
#define mdmx_suba_ob_ev(vs, vt, e)	_mdmx_aop_ev ("suba.ob", (vs), (vt), (e))
#define mdmx_suba_ob_cv(vs, c)		_mdmx_aop_cv ("suba.ob", (vs), (c))

/* SUBL.OB */
#define mdmx_subl_ob_v(vs, vt)		_mdmx_aop_v ("subl.ob", (vs), (vt))
#define mdmx_subl_ob_ev(vs, vt, e)	_mdmx_aop_ev ("subl.ob", (vs), (vt), (e))
#define mdmx_subl_ob_cv(vs, c)		_mdmx_aop_cv ("subl.ob", (vs), (c))

/* WACH.OB is provided by mdmx_acc_write_ob(). */

/* WACL.OB is provided by mdmx_acc_write_ob(). */

/* XOR.OB */
#define mdmx_xor_ob_v(vs, vt)		_mdmx_rop_v ("xor.ob", (vs), (vt))
#define mdmx_xor_ob_ev(vs, vt, e)	_mdmx_rop_ev ("xor.ob", (vs), (vt), (e))
#define mdmx_xor_ob_cv(vs, c)		_mdmx_rop_cv ("xor.ob", (vs), (c))

static inline void
mdmx_acc_read_ob (mdmxreg *hip, mdmxreg *midp, mdmxreg *lop)
{
  /* These must be volatile because they read accumulator state, and
     that dependency is not expressible via the constraints.  */
  if (lop != 0)
    __asm__ __volatile__ ("racl.ob %0" : "=f"(*lop));
  if (midp != 0)
    __asm__ __volatile__ ("racm.ob %0" : "=f"(*midp));
  if (hip != 0)
    __asm__ __volatile__ ("rach.ob %0" : "=f"(*hip));
}

static inline void
mdmx_acc_write_ob (mdmxreg hi, mdmxreg mid, mdmxreg lo)
{
  /* These must be volatile because they write accumulator state, and
     that dependency is not expressible via the constraints.  */
  __asm__ __volatile__ ("wacl.ob %0, %1" : : "f"(mid), "f"(lo));
  __asm__ __volatile__ ("wach.ob %0" : : "f"(hi));
}

static inline void
mdmx_acc_write_low_ob (mdmxreg mid, mdmxreg lo)
{
  /* This must be volatile because they write accumulator state, and
     that dependency is not expressible via the constraints.  */
  __asm__ __volatile__ ("wacl.ob %0, %1" : : "f"(mid), "f"(lo));
}

#endif /* defined(_SIBYTE_MDMX_H_) && !defined(_SIBYTE_MDMX_OB_H_) */
