/* $Id: mdmx_sb1.h,v 1.2 2003/05/09 04:48:06 cgd Exp $ */

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

/* mdmx_sb1.h

   Provide macros and functions which can be used to emit SB-1 MDMX
   extension instructions.  (SB-1 supports .ob format obly.)  Do not
   include this file directly, instead include mdmx.h.
 
   All of the functions and macros defined in this file are intended
   to be usable by application code.
 
   Many of the vector macros come in three forms: _v, _ev, and _cv.
   The '_v' form uses the entire vector specified by 'vt'.  The '_ev'
   form uses a vector in which each element is a copy of element 'e'
   of the vector specified by 'vt'.  Finally, the '_cv' form uses a
   vector in which each element is the constant 'c' (which must have a
   value in the range [0..31]).  */

#if !defined(_SIBYTE_MDMX_H_) || defined(_SIBYTE_MDMX_SB1_H_)
#error do not include this file directly; include mdmx.h
#endif
#if defined(_SIBYTE_MDMX_H_) && !defined(_SIBYTE_MDMX_SB1_H_)
#define _SIBYTE_MDMX_SB1_H_

static inline void
mdmx_sb1_enable (void)
{
#if !defined(__linux__) && !defined(__NetBSD__)
  /* Enable SB-1 extensions.  Note that this can't be done by user code
     when running under an operating system.  */
  int _sr;
  __asm__ ("mfc0 %0, $12" : "=r"(_sr));
  _sr |= (1 << 16);	/* Set SR:SBX.  Enable SB-1 Extensions.  */
  __asm__ __volatile__ ("mtc0 %0, $12" : : "r"(_sr));
  __asm__ __volatile__ ("	.set push	\n"
                        "	.set noreorder	\n"
                        "	ssnop		\n"
                        "	bnezl	$0, 1f	\n"
                        "1:	ssnop		\n"
                        "	.set pop	");
#endif
}

/* PAVG.OB */
#define mdmx_pavg_ob_v(vs, vt)		_mdmx_rop_v ("pavg.ob", (vs), (vt))
#define mdmx_pavg_ob_ev(vs, vt, e)	_mdmx_rop_ev ("pavg.ob", (vs), (vt), (e))
#define mdmx_pavg_ob_cv(vs, c)		_mdmx_rop_cv ("pavg.ob", (vs), (c))

/* PABSDIFF.OB */
#define mdmx_pabsdiff_ob_v(vs, vt)	_mdmx_rop_v ("pabsdiff.ob", (vs), (vt))
#define mdmx_pabsdiff_ob_ev(vs, vt, e)	_mdmx_rop_ev ("pabsdiff.ob", (vs), (vt), (e))
#define mdmx_pabsdiff_ob_cv(vs, c)	_mdmx_rop_cv ("pabsdiff.ob", (vs), (c))

/* PABSDIFFC.OB */
#define mdmx_pabsdiffc_ob_v(vs, vt)	_mdmx_aop_v ("pabsdiffc.ob", (vs), (vt))
#define mdmx_pabsdiffc_ob_ev(vs, vt, e)	_mdmx_aop_ev ("pabsdiffc.ob", (vs), (vt), (e))
#define mdmx_pabsdiffc_ob_cv(vs, c)	_mdmx_aop_cv ("pabsdiffc.ob", (vs), (c))

#endif /* defined(_SIBYTE_MDMX_H_) && !defined(_SIBYTE_MDMX_SB1_H_) */
