/* $Id: sbprof.c,v 1.8 2003/10/01 21:07:22 cgd Exp $ */

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

#include <assert.h>
#include <cfe_api.h>
#include <exlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "sbprof_int.h"

static int sbprof_initted;
static int sbprof_initflags = 0;
static int cfe_is_32bit;
static char cfe_bootdev[20] = "eth0";
static char cfe_bootpath[256] = "server_name:path/";

static void
sbprof_exit (void)
{
  void *buf;
  size_t size;

  if (sbprof_initflags & SBPROF_FLG_QUIET) 
      return;

  sbprof_get_cpu_buf(&buf, &size);
  if (buf == NULL)
    printf("*** libsbprof: no cpu profiling data buffer\n");
  else if (size == 0)
    printf("*** libsbprof: no cpu profiling data recorded\n");
  else
    {
      printf("*** libsbprof: to save cpu profiling data, use a command like:\n");
      printf(" ifconfig %s -auto; save %scpu-samples ",
             cfe_bootdev, cfe_bootpath);
      if (cfe_is_32bit)
	printf ("0x%08x", (int)(long)buf);
      else
	printf ("0x%016llx", (long long)(long)buf);
      printf (" 0x%lx\n", (long)size);
    }

  sbprof_get_zb_buf(&buf, &size);
  if (buf == NULL)
    printf("*** libsbprof: no zbbus profiling data buffer\n");
  else if (size == 0)
    printf("*** libsbprof: no zbbus profiling data recorded\n");
  else
    {
      printf("*** libsbprof: to save zbbus profiling data, use a command like:\n");
      printf(" ifconfig %s -auto; save %szb-samples ",
             cfe_bootdev, cfe_bootpath);
      if (cfe_is_32bit)
	printf ("0x%08x", (int)(long)buf);
      else
	printf ("0x%016llx", (long long)(long)buf);
      printf (" 0x%lx\n", (long)size);
    }
}

int
sbprof_init2 (long cpu_bufsize, long zb_bufsize, unsigned int flags)
{
  cfe_fwinfo_t cfe_fwi;
  char buf[256];

  if (sbprof_initted)
    return -1;
  sbprof_initted = 1;

  sbprof_initflags = flags;

  /* This code should support multiple CPUs eventually, but
     currently does not.  */
  assert (MAX_CPUS == 1);

  /* XXXCGD: check pass2 and later 1250, or any 112x.  */

  if (cfe_getfwinfo (&cfe_fwi) != 0)
    return -1;
  cfe_is_32bit = (cfe_fwi.fwi_flags & CFE_FWI_32BIT) != 0;

  if (atexit (sbprof_exit) != 0)
    return -1;

  exlib_init ();
  intr_init ();

  if (sbprof_cpu_init (cpu_bufsize) != 0
      || sbprof_zb_init (cpu_bufsize) != 0)
    return -1;

  /* Try to come up with useful values for cfe_bootpath and cfe_bootdev,
     so that people will be able to just cut and paste the output
     printed at the end of the run, to save their profiling data.

     Better hope that the buffer is large enough; CFE truncates the
     result to fit and provides no indication that it did so!

     We assume the boot device is reasonable for the 'save' command
     if it's a network device and fits in the buffer.  */
  if (cfe_getenv ("BOOT_DEVICE", buf, sizeof buf) == 0
      && strlen (buf) < sizeof cfe_bootdev
      && strncmp (buf, "eth", 3) == 0)
    {
      /* Length check done above.  */
      strcpy (cfe_bootdev, buf);

      /* If the boot device looked reasonable, try to munge the
         boot file into a path so that samples will go in the
         same directory.  */
      if (cfe_getenv ("BOOT_FILE", buf, sizeof buf) == 0)
        {
	   char *end;

	   /* Look for last directory separator.  If no directory
	      separators, the program has to be in the default directory,
	      so look for the *first* colon.  */
	   end = strrchr (buf, '/');
	   if (end == NULL)
	     end = strchr (buf, ':');
	   if (end != NULL)
	     {
               end++;
	       *end = '\0';
	       if (strlen (buf) < sizeof cfe_bootpath)
	         strcpy (cfe_bootpath, buf);
	     }
        }
    }

  return 0;
}

int
sbprof_init (long cpu_bufsize, long zb_bufsize)
{
    return sbprof_init2 (cpu_bufsize, zb_bufsize, 0);
}

