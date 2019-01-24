/* $Id: cache_err.c,v 1.4 2003/11/13 18:59:28 kwalker Exp $ */

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

/*
 * cache_err.c: small harness for installing the cache error handler.
 *
 * The code stays in a loop touching both the icache and dcache
 * repeatedly.  The JTAG dinter can be used to inject failures to
 * these cache lines with the 'inject_cerr.oo' stub.  The indexes have
 * to be figured out from the 'cerr' object file and plugged into
 * inject_cerr.S.  Use the address of "done" for dcache index and the
 * cacheline containing the while loop for icache index).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <exlib.h>

void sb1_cerr_vec(void);
void _flush_cache(void *, size_t, int);

volatile unsigned long long done = 0;

int main(int argc, char **argv)
{
	exlib_init ();
	exlib_set_common_handlers ();

	memcpy ((void *)(long)(int)0x80000100, &sb1_cerr_vec, 0x80);
	memcpy ((void *)(long)(int)0xa0000100, &sb1_cerr_vec, 0x80);
	_flush_cache((void *)(long)(int)0x80000100, 0x80, 3);

	while (!done)
		printf("foo\n");

	exit(EXIT_SUCCESS);
}
