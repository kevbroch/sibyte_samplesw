/* $Id: sbprof_cpu_speed.c,v 1.4 2004/12/12 05:23:29 cgd Exp $ */

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

#include <stdlib.h>
#include <sb1250-include/sb1250_regs.h>
#include <sb1250-include/sb1250_scd.h>
#include <sb1250-include/bcm1480_scd.h>

#include "sbprof_int.h"

unsigned long long
sbprof_cpu_speed (void)
{
  unsigned long long sysrev, syscfg;
  int plldiv = 0;

  sysrev = SBPROF_READ_CSR(A_SCD_SYSTEM_REVISION);
  syscfg = SBPROF_READ_CSR(A_SCD_SYSTEM_CFG);

  switch (SYS_SOC_TYPE (sysrev))
    {
    case K_SYS_SOC_TYPE_BCM1250:
    case K_SYS_SOC_TYPE_BCM1120:
    case K_SYS_SOC_TYPE_BCM1125:
    case K_SYS_SOC_TYPE_BCM1125H:
      plldiv = G_SYS_PLL_DIV (syscfg);
      break;

    case K_SYS_SOC_TYPE_BCM1x80:
    case K_SYS_SOC_TYPE_BCM1x55:
      plldiv = G_BCM1480_SYS_PLL_DIV (syscfg);
      break;

    default:
      abort ();
      break;
    }

  return (plldiv * 50 * 1000000);
}
