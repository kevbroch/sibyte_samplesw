/* $Id: psingle_sb1.h,v 1.4 2004/12/07 06:42:39 cgd Exp $ */

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

/* psingle_sb1.h

   Provide macros and functions which can be used to emit SB-1 Paired
   Single extension instructions.  Do not include this file directly,
   instead include psingle.h.

   All of the functions and macros defined in this file are intended
   to be usable by application code.  */

#if !defined(_SIBYTE_PSINGLE_H_) || defined(_SIBYTE_PSINGLE_SB1_H_)
#error do not include this file directly; include psingle.h
#endif
#if defined(_SIBYTE_PSINGLE_H_) && !defined(_SIBYTE_PSINGLE_SB1_H_)
#define _SIBYTE_PSINGLE_SB1_H_


static inline void
psingle_sb1_enable (void)
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


/* SB-1 Paired Single extensions.  */

#ifdef __mips_paired_single_float
#define __sb1_psingle_vector_1_1	((psingle) {1.0, 1.0})
#endif

_def_psingle_unary (psingle_recip, "recip.ps",
                    (__sb1_psingle_vector_1_1 / _ps))
_def_psingle_unary (psingle_rsqrt, "rsqrt.ps",
                    (__sb1_psingle_vector_1_1 / __builtin_mips_sqrt_ps (_ps)))
_def_psingle_unary (psingle_sqrt, "sqrt.ps",
                    __builtin_mips_sqrt_ps (_ps))

_def_psingle_binary (psingle_div, "div.ps",
                     (_ps1 / _ps2))

#endif /* defined(_SIBYTE_PSINGLE_H_) && !defined(_SIBYTE_PSINGLE_SB1_H_) */
