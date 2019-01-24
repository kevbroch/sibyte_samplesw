/* $Id: exframe_dump.c,v 1.8 2003/04/03 17:21:03 cgd Exp $ */

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

#include <stdio.h>
#include <stdlib.h>
#include "exlib.h"

void
exframe_dump(struct exframe *ef)
{
  int i;

  printf("Exception frame (%p) registers:\n", ef);
  printf(" STATUS: 0x%08x          CAUSE: 0x%08x\n",
         ef->ef_status, ef->ef_cause);
  printf("    EPC: 0x%016llx     $1: 0x%016llx\n",
         ef->ef_regs[0], ef->ef_regs[1]);
  for (i = 2; i < 32; i += 2)
    printf("%*s$%d: 0x%016llx%*s$%d: 0x%016llx\n",
           (i < 10 ? 5 : 4), "", i, ef->ef_regs[i],
	   ((i + 1) < 10 ? 5 : 4), "", i + 1, ef->ef_regs[i + 1]);
  printf("     HI: 0x%016llx     LO: 0x%016llx\n",
         ef->ef_hi, ef->ef_lo);

#ifndef __mips_soft_float
  printf(" FPSAVE: %#x (", ef->ef_fpsavemask);
  i = 0;
  if (ef->ef_fpsavemask & EXFPSAVEMASK_FCSR)
    printf("%s%s", i++ ? ", " : "", "fscr");
  if (ef->ef_fpsavemask & EXFPSAVEMASK_CALLER)
    printf("%s%s", i++ ? ", " : "", "caller-saved");
  if (ef->ef_fpsavemask & EXFPSAVEMASK_CALLEE)
    printf("%s%s", i++ ? ", " : "", "callee-saved");
  if (ef->ef_fpsavemask & EXFPSAVEMASK_MDMXACC)
    printf("%s%s", i++ ? ", " : "", "mdmx-acc");
  if (!i)
    printf("none");
  printf(")\n");

  if (ef->ef_fpsavemask & EXFPSAVEMASK_FCSR)
    printf("   FCSR: 0x%08x\n", ef->ef_fcsr);

#if (__mips_fpr == 64)
#define SHIFT  0
#else
#define SHIFT  1
#endif
#define SKIP   (1 << SHIFT)
#define	CALLEE_SAVED(n)	(((n) & 1) == 0 && (n >= 20))

  if (ef->ef_fpsavemask & (EXFPSAVEMASK_CALLER | EXFPSAVEMASK_CALLEE))
    {
      int did_caller = (ef->ef_fpsavemask & EXFPSAVEMASK_CALLER) != 0;
      int did_callee = (ef->ef_fpsavemask & EXFPSAVEMASK_CALLEE) != 0;

      for (i = 0; i < 32; i += SKIP)
        {
          printf("%*s$f%d: ", (i < 10 ? 4 : 3), "", i);
	  if ((did_caller && !CALLEE_SAVED(i))
	      || (did_callee && CALLEE_SAVED(i)))
	    printf("0x%016llx", ef->ef_fpregs[i >> SHIFT]);
          else
            printf("%-18s", "(not saved)");
	  if (i & SKIP)
	    printf("\n");
        }
    }

#undef SKIP
#undef SHIFT
#undef CALLEE_SAVED

#if (__mips_fpr == 64)
  if (ef->ef_fpsavemask & EXFPSAVEMASK_MDMXACC)
    {
      printf(" MDMX_H: 0x%016llx", ef->ef_mdmxacc[0]);
      printf(" MDMX_M: 0x%016llx\n", ef->ef_mdmxacc[1]);
      printf(" MDMX_L: 0x%016llx\n", ef->ef_mdmxacc[2]);
    }
#endif

#endif /* __mips_soft_float */
}
