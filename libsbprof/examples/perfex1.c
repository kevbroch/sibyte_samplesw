/* $Id: perfex1.c,v 1.4 2003/05/16 02:04:41 cgd Exp $ */

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
#include <string.h>

#include <sbprof.h>

int
main (void)
{
  unsigned long long starttime, endtime;
  unsigned long long cpu_speed;

  
  if (sbprof_init (SBPROF_BUFSIZE_DEFAULT, SBPROF_BUFSIZE_DEFAULT) != 0)
    {
      printf ("profiling library initialization failed!\n");
      exit (EXIT_FAILURE);
    }

  cpu_speed = sbprof_cpu_speed ();
  starttime = sbprof_zbbus_count ();

  /* Stop after one second.  (Remember, cpu speed is zbbus speed * 2.)  */
  endtime = starttime + (cpu_speed / 2);

  sbprof_start ();
  sbprof_zbprof_start ();

  /* Loop.  This will generate predictable, if somewhat boring, traces.  */
  while (sbprof_zbbus_count () < endtime)
    ;

  sbprof_stop ();
  sbprof_zbprof_stop ();

  exit (EXIT_SUCCESS);
}
