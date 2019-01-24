/* $Id: spinlock.c,v 1.4 2003/05/09 04:34:05 cgd Exp $ */

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

/* spinlock.c: simple spinlock demonstration.
 
   This program demonstrates the spinlock/spinunlock functions
   provided by the spinlock.h header file.  Note that it doesn't
   actually test that the functions there do what they're supposed to
   (i.e., atomically acquire locks), but really, they do.  */

#include <stdio.h>
#include <stdlib.h>

/* XXX: These will often be defined by standard system headers.
   Unfortunately, right now they're not defined by the newlib
   headers provided with the sb1-elf tools.  If you're using this
   example in another environment, you'll probably have to remove
   these definitions.  */
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#include "spinlock.h"

uint32_t lock32;
uint64_t lock64;

int
main (void)
{
#define PRINTLOCK32() \
  printf ("@ %d: lock32 = %#x\n", __LINE__, (unsigned int)lock32)

  PRINTLOCK32 ();
  spinlock32_lock (&lock32, 0x12345678);
  PRINTLOCK32 ();
  printf ("@ %d: spinlock32_trylock returned: %#x\n", __LINE__,
	  (unsigned int)spinlock32_trylock (&lock32, 0x9abcdef0));
  PRINTLOCK32 ();
  spinlock32_release (&lock32);
  PRINTLOCK32 ();
  printf ("@ %d: spinlock32_trylock returned: %#x\n", __LINE__,
	  (unsigned int)spinlock32_trylock (&lock32, 0x9abcdef0));
  PRINTLOCK32 ();
  spinlock32_release (&lock32);
  PRINTLOCK32 ();

  printf ("\n");

#define PRINTLOCK64() \
  printf ("@ %d: lock64 = %#llx\n", __LINE__, (unsigned long long)lock64)

  PRINTLOCK64 ();
  spinlock64_lock (&lock64, 0x123456789abcdef0);
  PRINTLOCK64 ();
  printf ("@ %d: spinlock64_trylock returned: %#llx\n", __LINE__,
	  (unsigned long long)spinlock64_trylock (&lock64,
						  0x0fedcba987654321));
  PRINTLOCK64 ();
  spinlock64_release (&lock64);
  PRINTLOCK64 ();
  printf ("@ %d: spinlock64_trylock returned: %#llx\n", __LINE__,
	  (unsigned long long)spinlock64_trylock (&lock64,
						  0x0fedcba987654321));
  PRINTLOCK64 ();
  spinlock64_release (&lock64);
  PRINTLOCK64 ();

  exit (EXIT_SUCCESS);
}
