/* $Id: semaphore.c,v 1.3 2003/05/09 04:34:05 cgd Exp $ */

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

/* semaphore.c: simple semaphore demonstration.
 
   This program demonstrates the sem_take and sem_release functions
   provided by the semaphore.h header file.  Note that it doesn't
   actually test that the functions there do what they're supposed to
   (i.e., operate atomically), but really, they do.  */

#include <stdio.h>
#include <stdlib.h>

#include "semaphore.h"

int
main (void)
{
  semaphore s = 1;
  semaphore t = 2;

#define PRINT_SEMAPHORE(x) printf ("@ %d: " #x " = %#x\n", __LINE__, (int)x)

  PRINT_SEMAPHORE (s);
  sem_take (&s);
  PRINT_SEMAPHORE (s);
#if 0
  sem_take (&s);        /* Would spin forever, since no one will release.  */
  PRINT_SEMAPHORE (s);
#endif
  sem_release (&s);
  PRINT_SEMAPHORE (s);
 
  printf ("\n");
 
  PRINT_SEMAPHORE (t);
  sem_take (&t);
  PRINT_SEMAPHORE (t);
  sem_take (&t);
  PRINT_SEMAPHORE (t);
#if 0
  sem_take (&t);        /* Would spin forever, since no one will release.  */
  PRINT_SEMAPHORE (t);
#endif
  sem_release (&t);
  PRINT_SEMAPHORE (t);
  sem_take (&t);
  PRINT_SEMAPHORE (t);
  sem_release (&t);
  PRINT_SEMAPHORE (t);
  sem_release (&t);
  PRINT_SEMAPHORE (t);
  
  exit (EXIT_SUCCESS);
}
