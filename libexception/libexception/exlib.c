/* $Id: exlib.c,v 1.25 2005/01/05 05:43:59 cgd Exp $ */

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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "exlib_int.h"

static int exlib_initted;
int exlib_mips64, exlib_fpu, exlib_mdmx;

static int orig_status_bev_was_set;
static int orig_cause_iv_was_set;

/* These arrays have type "int" rather than type char, so that
   they will be properly aligned to hold executable instructions.  */
static unsigned int old_vec_tlb[EXVEC_SIZE / sizeof (int)];
static unsigned int old_vec_xtlb[EXVEC_SIZE / sizeof (int)];
static unsigned int old_vec_gen[EXVEC_SIZE / sizeof (int)];

static struct {
  exhandler_func func;
  int fpsavemask;
} handlers[EXCODE_NCODES];

#ifndef __mips_soft_float
static void exframe_save_fp(struct exframe *ef, unsigned int fpsavemask);
#endif

void
exlib_init(void)
{
  int status, cause, config, config1;
  int status_ie_was_set;

  if (exlib_initted)
    return;
  exlib_initted = 1;

  config = cp0_get_config ();
  config1 = cp0_get_config1 ();

  exlib_mips64 = (G_CFG_AT(config) == K_CFG_AT_MIPS64_32
		  || G_CFG_AT(config) == K_CFG_AT_MIPS64);
  exlib_fpu = (config1 & M_CFG_FP) != 0;
  exlib_mdmx = (config1 & (1 << 5)) != 0;	// No MD bit check.

#if 0
  /* Enable only for testing of MIPS32 code on MIPS64 parts.  Note
     that for the MIPS32 exception vector code to be valid on MIPS64,
     you have to add the commented-out 'sll' back into the REG_STORE
     macro in exlib_vectors_mips32.S.  */
  exlib_mips64 = 0;
#endif
#ifdef __mips64
  if (! exlib_mips64)
    {
      printf ("exlib_init: 64-bit code on a 32-bit part?\n");
      abort ();
    }
#endif
#ifndef __mips_soft_float
  if (! exlib_fpu
      || (__mips_fpr == 64 && ! exlib_mips64))
    {
      printf ("exlib_init: bad FPU setup (fpu? = %d, mips64? = %d, __mips_fpr: %d\n",
	      exlib_fpu, exlib_mips64, __mips_fpr);
      abort ();
    }
#endif

  __asm__ ("mfc0 %0, $%1" : "=r"(status) : "i"(C0_STATUS));

  /* Disable interrupts while adjusting vectors.  */
  status_ie_was_set = ((status & M_SR_IE) != 0);
  if (status_ie_was_set);
    {
      status &= ~M_SR_IE;
      __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(status), "i"(C0_STATUS));
      __asm__ __volatile__ (".set push ; "
			    ".set mips64 ; "
			    "ssnop ; ssnop ; ssnop ; "
			    ".set pop");
    }

  orig_status_bev_was_set = ((status & M_SR_BEV) != 0);
  if (orig_status_bev_was_set)
    {
      /* Install cache error stub, which jumps to ROM on cache error.
         We install the stub in two places: in KSEG0 (typically cacheable
         coherent), and in KSEG1 (uncached).

         We do the KSEG0 setup first, then flush the L1 cache, then set up
         the KSEG1 copy.  That way, if the cacheable copy ends up in L2
         (where it may reside for some time without being evicted to
         memory), the copy in memory will still have the correct
         instructions.

         The cache error handler is run uncached, so the instruction
         fetches will bypass all caches (i.e., will come straight from
	 memory).  */
      if (exlib_mips64)
	{
	  INSTALL_VECTOR (EXVEC_ADDR_CERR, exlib_vec_cerr_stub_mips64,
			  EXVEC_SIZE);
	  memcpy (EXVEC_ADDR_CERR_K1, exlib_vec_cerr_stub_mips64,
		  EXVEC_SIZE);
	}
      else
	{
	  INSTALL_VECTOR (EXVEC_ADDR_CERR, exlib_vec_cerr_stub_mips32,
			  EXVEC_SIZE);
	  memcpy (EXVEC_ADDR_CERR_K1, exlib_vec_cerr_stub_mips32,
		  EXVEC_SIZE);
	}
    }
  else
    {
      /* Save old vectors so that we can restore them at shutdown.
         Note that we don't touch the Cause:IV handler.  */
      INSTALL_VECTOR (old_vec_tlb, EXVEC_ADDR_TLB, EXVEC_SIZE);
      INSTALL_VECTOR (old_vec_xtlb, EXVEC_ADDR_XTLB, EXVEC_SIZE);
      INSTALL_VECTOR (old_vec_gen, EXVEC_ADDR_GEN, EXVEC_SIZE);
    }

  exlib_set_common_handlers ();

  /* Install new vectors.  */
  if (exlib_mips64)
    {
      INSTALL_VECTOR (EXVEC_ADDR_TLB, exlib_vec_general_mips64, EXVEC_SIZE);
      INSTALL_VECTOR (EXVEC_ADDR_XTLB, exlib_vec_general_mips64, EXVEC_SIZE);
      INSTALL_VECTOR (EXVEC_ADDR_GEN, exlib_vec_general_mips64, EXVEC_SIZE);
    }
  else
    {
      INSTALL_VECTOR (EXVEC_ADDR_TLB, exlib_vec_general_mips32, EXVEC_SIZE);
      INSTALL_VECTOR (EXVEC_ADDR_XTLB, exlib_vec_general_mips32, EXVEC_SIZE);
      INSTALL_VECTOR (EXVEC_ADDR_GEN, exlib_vec_general_mips32, EXVEC_SIZE);
    }

  /* Use RAM vectors, not ROM (bootstrap) vectors.  */
  status &= ~M_SR_BEV;
  __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(status), "i"(C0_STATUS));
  
  __asm__ ("mfc0 %0, $%1" : "=r"(cause) : "i"(C0_CAUSE));
  orig_cause_iv_was_set = ((cause & M_CAUSE_IV) != 0);
  cause &= ~M_CAUSE_IV;
  __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(cause), "i"(C0_CAUSE));

  if (status_ie_was_set)
    {
      status |= M_SR_IE;
      __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(status), "i"(C0_STATUS));
    }
}

void
exlib_shutdown(void)
{
  int status, status_ie_was_set;
  int cause;

  if (!exlib_initted)
    return;
  exlib_initted = 0;

  __asm__ ("mfc0 %0, $%1" : "=r"(status) : "i"(C0_STATUS));

  /* Disable interrupts while adjusting vectors.  */
  status_ie_was_set = ((status & M_SR_IE) != 0);
  if (status_ie_was_set)
    {
      status &= ~M_SR_IE;
      __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(status), "i"(C0_STATUS));
      __asm__ __volatile__ (".set push ; "
			    ".set mips64 ; "
			    "ssnop ; ssnop ; ssnop ; "
			    ".set pop");
    }
  
  if (orig_status_bev_was_set)
    {
      status |= M_SR_BEV;
      __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(status), "i"(C0_STATUS));
    }
  else
    {
      /* Resture old vectors.  */
      INSTALL_VECTOR (EXVEC_ADDR_TLB, old_vec_tlb, EXVEC_SIZE);
      INSTALL_VECTOR (EXVEC_ADDR_XTLB, old_vec_xtlb, EXVEC_SIZE);
      INSTALL_VECTOR (EXVEC_ADDR_GEN, old_vec_gen, EXVEC_SIZE);
    }
  
  if (orig_cause_iv_was_set)
    {
      __asm__ ("mfc0 %0, $%1" : "=r"(cause) : "i"(C0_CAUSE));
      cause |= M_CAUSE_IV;
      __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(cause), "i"(C0_CAUSE));
    }

  /* Re-enable interrupts if necessary.  */
  if (status_ie_was_set)
    {
      status |= M_SR_IE;
      __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(status), "i"(C0_STATUS));
    }
}

int
exlib_set_handler(unsigned int excode, exhandler_func f)
{
  if (excode >= EXCODE_NCODES)
    return -1;
  handlers[excode].func = f;
  return 0;
}

exhandler_func
exlib_get_handler(unsigned int excode)
{
  if (excode >= EXCODE_NCODES)
    return exhandler_abort;
  return (handlers[excode].func);
}

int
exlib_set_fpsavemask(unsigned int excode, unsigned int fpsavemask)
{
  if (excode >= EXCODE_NCODES)
    return -1;

  if (! exlib_fpu)
    fpsavemask = EXFPSAVEMASK_NONE;
  if (! exlib_mdmx)
    fpsavemask &= ~EXFPSAVEMASK_MDMXACC;

  handlers[excode].fpsavemask = fpsavemask;
  return 0;
}

unsigned int
exlib_get_fpsavemask(unsigned int excode)
{
  if (excode >= EXCODE_NCODES)
    return EXFPSAVEMASK_NONE;
  return (handlers[excode].fpsavemask);
}

/* exlib_dispatch: dispatch an exeption to the appropriate handler.
   On entry, k1's register slot (27) contains exeption code.  */
void
exlib_dispatch(struct exframe *ef)
{
  unsigned long long excode = ef->ef_regs[27];

#ifndef __mips_soft_float
  if (handlers[excode].fpsavemask != EXFPSAVEMASK_NONE)
    exframe_save_fp (ef, handlers[excode].fpsavemask);
  else
    ef->ef_fpsavemask = EXFPSAVEMASK_NONE;
#endif

  (*handlers[excode].func)(excode, ef);

  exlib_return (ef);
}


void
exlib_return (struct exframe *ef)
{
  if (exlib_mips64)
    exlib_return_mips64 (ef);
  else
    exlib_return_mips32 (ef);
}


#ifndef __mips_soft_float
/* MIPS FP registers save conventions:
   Callee-saved: n64 = $f24..$f31; n32 and o32 = $f20..$f30, evens only.
   Caller-saved: the rest.
   If saving anything, save/restore FCSR.

   This code handles o32 (and o64) ONLY, and works only because the
   path from exception entry to exframe_save_fp doesn't use FP at all.
   (It's much nicer to do this in C than in assembly code, though.)  */
static void
exframe_save_fp(struct exframe *ef, unsigned int fpsavemask)
{
  unsigned int actualsavemask = 0;

  if (fpsavemask & EXFPSAVEMASK_FCSR)
    {
      unsigned int fcsr;
      __asm__ ("cfc1 %0, $31" : "=r"(fcsr));
      ef->ef_fcsr = fcsr;
      actualsavemask |= EXFPSAVEMASK_FCSR;
    }

#if (__mips_fpr == 64)

# define SAVE_FPREG(r) \
  __asm__ ("sdc1 $%1, %0" : "=m"(ef->ef_fpregs[(r)]) : "i"(r))

  if (fpsavemask & EXFPSAVEMASK_CALLER)
    {
      SAVE_FPREG ( 0);  SAVE_FPREG ( 1);
      SAVE_FPREG ( 2);  SAVE_FPREG ( 3);
      SAVE_FPREG ( 4);  SAVE_FPREG ( 5);
      SAVE_FPREG ( 6);  SAVE_FPREG ( 7);
      SAVE_FPREG ( 8);  SAVE_FPREG ( 9);
      SAVE_FPREG (10);  SAVE_FPREG (11);
      SAVE_FPREG (12);  SAVE_FPREG (13);
      SAVE_FPREG (14);  SAVE_FPREG (15);
      SAVE_FPREG (16);  SAVE_FPREG (17);
      SAVE_FPREG (18);  SAVE_FPREG (19);
                        SAVE_FPREG (21);
                        SAVE_FPREG (23);
                        SAVE_FPREG (25);
                        SAVE_FPREG (27);
                        SAVE_FPREG (29);
                        SAVE_FPREG (31);
      actualsavemask |= EXFPSAVEMASK_CALLER;
    }
  if (fpsavemask & EXFPSAVEMASK_CALLEE)
    {
      SAVE_FPREG (20);
      SAVE_FPREG (22);
      SAVE_FPREG (24);
      SAVE_FPREG (26);
      SAVE_FPREG (28);
      SAVE_FPREG (30);
      actualsavemask |= EXFPSAVEMASK_CALLEE;
    }
  if ((fpsavemask & EXFPSAVEMASK_MDMXACC) != 0
      && (ef->ef_status & M_SR_MX) != 0)
    {
      double tmp;
      /* Note that this assumes that if Status:MX wasn't set at the time of
	 the exception we shouldn't save the registers, and that if it _was_
         set at the time of the exception it's still set now!  */
      __asm__ (".set push ; "
	       ".set mdmx ; "
	       "rach.ob %0 ; sdc1 %0, %1 ; "
	       "racm.ob %0 ; sdc1 %0, %2 ; "
	       "racl.ob %0 ; sdc1 %0, %3 ; "
	       ".set pop"
               : "=f"(tmp), "=m"(ef->ef_mdmxacc[0]),
                 "=m"(ef->ef_mdmxacc[1]), "=m"(ef->ef_mdmxacc[2]));
      actualsavemask |= EXFPSAVEMASK_MDMXACC;
    }
# undef SAVE_FPREG

#else /* __mips_fpr != 64 */

# define SAVE_FPREG(r) \
  __asm__ (".set push ; .set mips2 ; sdc1 $%1, %0 ; .set pop" \
           : "=m"(ef->ef_fpregs[(r) >> 1]) : "i"(r))

  if (fpsavemask & EXFPSAVEMASK_CALLER)
    {
      SAVE_FPREG ( 0);  SAVE_FPREG ( 2);
      SAVE_FPREG ( 4);  SAVE_FPREG ( 6);
      SAVE_FPREG ( 8);  SAVE_FPREG (10);
      SAVE_FPREG (12);  SAVE_FPREG (14);
      SAVE_FPREG (16);  SAVE_FPREG (18); 
      actualsavemask |= EXFPSAVEMASK_CALLER;
    }
  if (fpsavemask & EXFPSAVEMASK_CALLEE)
    {
      SAVE_FPREG (20);  SAVE_FPREG (22);
      SAVE_FPREG (24);  SAVE_FPREG (26);
      SAVE_FPREG (28);  SAVE_FPREG (30);
      actualsavemask |= EXFPSAVEMASK_CALLEE;
    }
# undef SAVE_FPREG

#endif /* __mips_fpr != 64) */

    ef->ef_fpsavemask = actualsavemask;
}

/* exframe_restore_fp: restore floating point state when returning from
   an exception.  Note that this is run with EXL **SET**, so keep it
   short and sweet.  */
void
exframe_restore_fp(struct exframe *ef)
{

  if (ef->ef_fpsavemask == EXFPSAVEMASK_NONE)
    return;

  if (ef->ef_fpsavemask & EXFPSAVEMASK_FCSR)
    __asm__ __volatile__ ("ctc1 %0, $31" : : "r"(ef->ef_fcsr));

#if (__mips_fpr == 64)

# define RESTORE_FPREG(r) \
  __asm__ __volatile__ ("ldc1 $%1, %0" : :  "m"(ef->ef_fpregs[(r)]), "i"(r))

  if (ef->ef_fpsavemask & EXFPSAVEMASK_CALLER)
    {
      RESTORE_FPREG ( 0);  RESTORE_FPREG ( 1);
      RESTORE_FPREG ( 2);  RESTORE_FPREG ( 3);
      RESTORE_FPREG ( 4);  RESTORE_FPREG ( 5);
      RESTORE_FPREG ( 6);  RESTORE_FPREG ( 7);
      RESTORE_FPREG ( 8);  RESTORE_FPREG ( 9);
      RESTORE_FPREG (10);  RESTORE_FPREG (11);
      RESTORE_FPREG (12);  RESTORE_FPREG (13);
      RESTORE_FPREG (14);  RESTORE_FPREG (15);
      RESTORE_FPREG (16);  RESTORE_FPREG (17);
      RESTORE_FPREG (18);  RESTORE_FPREG (19);
                           RESTORE_FPREG (21);
                           RESTORE_FPREG (23);
                           RESTORE_FPREG (25);
                           RESTORE_FPREG (27);
                           RESTORE_FPREG (29);
                           RESTORE_FPREG (31);
    }
  if (ef->ef_fpsavemask & EXFPSAVEMASK_CALLEE)
    {
      RESTORE_FPREG (20);
      RESTORE_FPREG (22);
      RESTORE_FPREG (24);
      RESTORE_FPREG (26);
      RESTORE_FPREG (28);
      RESTORE_FPREG (30);
    }
  if (ef->ef_fpsavemask & EXFPSAVEMASK_MDMXACC)
    {
      double tmp1, tmp2;
      /* Note that this assumes that Status:MX is set!  */
      __asm__ (".set push ; "
	       ".set mdmx ; "
	       "ldc1 %0, %3 ; ldc1 %1, %4 ; wacl.ob %0, %1 ; "
	       "ldc1 %0, %2 ; wach.ob %0 ; "
	       ".set pop"
               : "=f"(tmp1), "=f"(tmp2), "=m"(ef->ef_mdmxacc[0]),
                 "=m"(ef->ef_mdmxacc[1]), "=m"(ef->ef_mdmxacc[2]));
    }
# undef RESTORE_FPREG

#else /* __mips_fpr != 64 */

# define RESTORE_FPREG(r) \
  __asm__ (".set push ; .set mips2 ; ldc1 $%1, %0 ; .set pop" \
           : : "m"(ef->ef_fpregs[(r) >> 1]), "i"(r))

  if (ef->ef_fpsavemask & EXFPSAVEMASK_CALLER)
    {
      /* Horrible hack: if you don't keep these all on one line, the
         compiler emits a label per line, which seems to cause the
         assembler to put a nop between each line's ldc1 ops.  (This
         is true even if ".set mips32" is used.)  Easier to just
         continue the line.  */
      RESTORE_FPREG ( 0);  RESTORE_FPREG ( 2); \
      RESTORE_FPREG ( 4);  RESTORE_FPREG ( 6); \
      RESTORE_FPREG ( 8);  RESTORE_FPREG (10); \
      RESTORE_FPREG (12);  RESTORE_FPREG (14); \
      RESTORE_FPREG (16);  RESTORE_FPREG (18);
    }
  if (ef->ef_fpsavemask & EXFPSAVEMASK_CALLEE)
    {
      /* Horrible hack: see above.  */
      RESTORE_FPREG (20);  RESTORE_FPREG (22); \
      RESTORE_FPREG (24);  RESTORE_FPREG (26); \
      RESTORE_FPREG (28);  RESTORE_FPREG (30);
    }
# undef RESTORE_FPREG

#endif /* __mips_fpr != 64 */
}
#endif /* !__mips_soft_float  */

