/* $Id: psingletest.c,v 1.7 2004/12/07 07:14:22 cgd Exp $ */

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

/* psingletest.c

   Simple Paired Single test program and demonstration.
 
   This program serves to test the Paired Single access macro/asm
   implementation provided by psingle.h, and to provide a short
   demonstration of the Paired Single oeprations.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psingle.h"

int errors;
int verbose = 0;

typedef union {
  float f;
  int i;
} float_or_int;

#define CHECK_F(f, exp)							\
  do {									\
    float_or_int _f, _exp;						\
    _f.f = f;								\
    _exp.f = exp;							\
    if (verbose)							\
      printf ("%s:%d: expected 0x%08x, got 0x%08x\n",			\
              __FILE__, __LINE__, _exp.i, _f.i);			\
    if (_f.i != _exp.i)							\
      {									\
        printf ("%s:%d: mismatch "					\
		"(expected 0x%08x, got 0x%08x)\n",			\
                __FILE__, __LINE__, _exp.i, _f.i);			\
        errors++;							\
      }									\
  } while (0)

#define CHECK_PS(ps, exp_upper, exp_lower)				\
  do {									\
    float_or_int _psl, _psu, _expl, _expu;				\
    _expl.f = exp_lower;						\
    _expu.f = exp_upper;						\
    _psl.f = float_from_psingle_lower (ps);				\
    _psu.f = float_from_psingle_upper (ps);				\
    if (verbose)							\
      {									\
        printf ("%s:%d: lower part: expected 0x%08x, got 0x%08x\n",	\
                __FILE__, __LINE__, _expl.i, _psl.i);			\
        printf ("%s:%d: upper part: expected 0x%08x, got 0x%08x\n",	\
                __FILE__, __LINE__, _expu.i, _psu.i);			\
      }									\
    if (_psl.i != _expl.i)						\
      {									\
        printf ("%s:%d: lower-part mismatch "				\
		"(expected 0x%08x, got 0x%08x)\n",			\
                __FILE__, __LINE__, _expl.i, _psl.i);			\
        errors++;							\
      }									\
    if (_psu.i != _expu.i)						\
      {									\
        printf ("%s:%d: upper-part mismatch "				\
		"(expected 0x%08x, got 0x%08x)\n",			\
                __FILE__, __LINE__, _expu.i, _psu.i);			\
        errors++;							\
      }									\
  } while (0)

unsigned long long psm0 = 0xc1a8000042200000ULL;	/* -21.0, 40.0 */
psingle psm1;
psingle psm2;


static void
check_ps (void)
{
  /* ps1, ps2, and ps3 are volatile to keep the compiler from doing CSE
     and constant-folding on inputs and results.  */
  volatile psingle ps1, ps2, ps3;
  psingle r;
  float f;

  memcpy (&psm1, &psm0, sizeof psm1);
  r = psingle_load (&psm1);		CHECK_PS (r, -21.0, 40.0);

  r = psingle_from_floats (-4.0, 32.0);
  psingle_store (&psm2, r);
  memcpy (&psm1, &psm2, sizeof psm1);
  r = psingle_load (&psm1);		CHECK_PS (r, -4.0, 32.0);

  f = float_from_psingle_lower (r);	CHECK_F (f, 32.0);
  f = float_from_psingle_upper (r);	CHECK_F (f, -4.0);

  r = psingle_from_floats (1.0, 3.0);	CHECK_PS (r, 1.0, 3.0);

  ps1 = psingle_from_floats (4.0, 16.0);
  ps2 = psingle_from_floats (-1.0, 2.0);
  ps3 = psingle_from_floats (17.0, -8.0);

  r = psingle_pll (ps1, ps2);		CHECK_PS (r, 16.0, 2.0);
  r = psingle_plu (ps1, ps2);		CHECK_PS (r, 16.0, -1.0);
  r = psingle_pul (ps1, ps2);		CHECK_PS (r, 4.0, 2.0);
  r = psingle_puu (ps1, ps2);		CHECK_PS (r, 4.0, -1.0);

  r = psingle_abs (ps2);		CHECK_PS (r, 1.0, 2.0);
  r = psingle_mov (ps2);		CHECK_PS (r, -1.0, 2.0);
  r = psingle_neg (ps2);		CHECK_PS (r, 1.0, -2.0);

  r = psingle_add (ps1, ps2);		CHECK_PS (r, 3.0, 18.0);
  r = psingle_mul (ps1, ps2);		CHECK_PS (r, -4.0, 32.0);
  r = psingle_sub (ps1, ps2);		CHECK_PS (r, 5.0, 14.0);

  r = psingle_madd (ps3, ps1, ps2);	CHECK_PS (r, 13.0, 24.0);
  r = psingle_msub (ps3, ps1, ps2);	CHECK_PS (r, -21.0, 40.0);
  r = psingle_nmadd (ps3, ps1, ps2);	CHECK_PS (r, -13.0, -24.0);
  r = psingle_nmsub (ps3, ps1, ps2);	CHECK_PS (r, 21.0, -40.0);


  /* Check SB-1 Paired Single extensions.  */
  psingle_sb1_enable ();
  r = psingle_recip (ps1);		CHECK_PS (r, 0.25, 0.0625);
  r = psingle_rsqrt (ps1);		CHECK_PS (r, 0.5, 0.25);
  r = psingle_sqrt (ps1);		CHECK_PS (r, 2.0, 4.0);
  r = psingle_div (ps1, ps2);		CHECK_PS (r, -4.0, 8.0);
}

int
main (void)
{
  int saveerrors;

  printf ("PS test program.\n");
#ifdef __mips_paired_single_float
  printf ("Using compiler paired-single builtins.\n");
#else
  printf ("Using asms for paired-single instructions.\n");
#endif
  saveerrors = errors;
  check_ps ();
  printf ("PS checks %s!\n", (saveerrors == errors) ? "passed" : "failed");

  exit (errors == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
