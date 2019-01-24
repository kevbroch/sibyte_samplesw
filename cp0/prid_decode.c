/* $Id: prid_decode.c,v 1.1 2004/12/07 07:22:18 cgd Exp $

   prid_decode.c: Decode CP0 PRID (Processor Identification) register,
   in the style suggested by the SB1-AN100 Application Note.
*/

/*
 * Copyright 2004
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

#include <sb1250-include/sbmips.h>

int
main (void)
{
  unsigned int raw_prid, adjusted_prid;
  int arch, microarch, major_rev, minor_rev;
  char microarch_str[2];

  raw_prid = cp0_get_prid();

  printf ("Raw PRID value:      0x%08x\n", raw_prid);

  if (((raw_prid >> 16) & 0xff) != 0x4)
    {
      printf ("Not a SiByte CPU.\n");
      return 0;
    }

  /* This is a SiByte CPU, convert any old-format PRID
     values so that they are consistently in the new
     format. */
  if ((raw_prid & 0xf000) != 0)
    {
      /* The new-format Major Version is non-zero. Therefore,
         the PRID value is in the new format. */
      adjusted_prid = raw_prid;
    }
  else
    {
      /* The new-format Major Version is zero. Therefore,
         the PRID value is in the old format. Shift the
         low 12 bits of the PRID up by 4, so that the
         CPU Architecture and Major Version values are in
         the locations used in the new-format PRID. */
      adjusted_prid = (raw_prid & 0xffff0000) | ((raw_prid & 0x0fff) << 4);
    }

  printf ("Adjusted PrID value: 0x%08x\n", adjusted_prid);

  arch      = (adjusted_prid & 0xf000) >> 12;
  microarch = (adjusted_prid & 0x0f00) >>  8;
  major_rev = (adjusted_prid & 0x00f0) >>  4;
  minor_rev = (adjusted_prid & 0x000f) >>  0;

  microarch_str[0] = microarch ? ('A' + microarch - 1) : '\0';
  microarch_str[1] = '\0';

  printf ("SB-%d%s rev %d.%d\n", arch, microarch_str, major_rev, minor_rev);

  if (arch == 1)
    {
      printf ("%sprocessor, CPU ID %d\n",
              ((adjusted_prid >> 24) & 0x1) ? "Multi" : "Uni",
	      (adjusted_prid >> 25) & 0x7);
    }

  return (0);
}
