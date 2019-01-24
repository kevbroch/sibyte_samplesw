/* $Id: memfuncs.c,v 1.2 2004/11/23 05:51:50 cgd Exp $ */

/*
 * Copyright 2001, 2002, 2003, 2004
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

#define SIZE (4*1024*1024)
struct {
  long long x; // force 8-byte alignment
  char src[SIZE];
  char pad1[1024];
  char dst[SIZE];
  char pad2[1024];
  char back[SIZE];
} x;

int     memcheck (char *a, char *b, int size, int print);
void    test_memcpy (void);
void    test_memset (void);
int     main (void);
void    *memset_sb1 (void *s, int c, size_t n);
void    *memcpy_sb1 (void *dest, void *src, size_t n);


int
memcheck(char *a, char *b, int size, int print)
{
  int i;
  int fail = -1;
  for (i = 0; i < size; i++) {
    if (a[i] != b[i]) {
      if (print) printf("X");
      if (fail < 0) fail = i;
    } else {
      if (print) printf(".");
    }
  }
  if (print) printf("\n");
  return fail;
}

void
test_memcpy(void)
{
  int i, soff, doff;
  int test_size = 256;

  if (test_size > SIZE) {
    printf("SIZE too small to test\n");
    return;
  }

  memset(x.dst, 0xbd, test_size);
  memset(x.back, 0xbd, test_size);
  for (i = 0; i < test_size; i += sizeof(unsigned int)) {
    unsigned int bits = rand();
    memcpy(&x.src[i], &bits, sizeof(unsigned int));
  }
  for (soff = 0; soff < 16; soff++) {
    for (doff = 0; doff < 16; doff++) {
      int maxoff = (soff > doff) ? soff : doff;
      //printf("soff %d doff %d max %d\n", soff, doff, maxoff);
      for (i = 0; i <= test_size - maxoff ; i++) {
	int diff = 0;
	char *src = &x.src[soff];
	char *dst = &x.dst[doff];
	//printf("dst %p src %p len %u \n", dst, src, i);
	memset(x.dst, 0xbd, test_size);
	memcpy_sb1(dst, src, i);
	diff = memcheck(dst, src, i, 0);
	if (diff >= 0) {
	  printf("memcpy failed for size %d at byte %d\n", i, diff);
	}
	diff = memcheck(x.dst, x.back, dst - x.dst, 0);
	if (diff >= 0) {
	  //printf("length soon %d\n", dst - x.dst);
	  printf("memcpy started too soon for size %d\n", i);
	}
	diff = memcheck(dst+i, x.back+i, (x.dst+test_size)-(dst+i), 0);
	if (diff >= 0) {
	  //printf("length far  %d\n", (x.dst+test_size)-(dst+i));
	  printf("memcpy went too far for size %d\n", i);
	}
      }
    }
  }
}

void
test_memset(void)
{
  int i, doff;
  int test_size = 256;

  if (test_size > SIZE) {
    printf("SIZE too small to test\n");
    return;
  }

  memset(x.src, 0x5a, test_size);
  memset(x.dst, 0xbd, test_size);
  memset(x.back, 0xbd, test_size);
  for (doff = 0; doff < 16; doff++) {
    for (i = 0; i <= test_size - doff ; i++) {
      int diff = 0;
      char *dst = &x.dst[doff];
      //printf("dst %p len %u \n", dst, i);
      memset(x.dst, 0xbd, test_size);
      memset_sb1(dst, 0x5a, i);
      diff = memcheck(dst, x.src, i, 0);
      if (diff >= 0) {
	printf("memset failed for size %d at byte %d\n", i, diff);
      }
      diff = memcheck(x.dst, x.back, dst - x.dst, 0);
      if (diff >= 0) {
	//printf("length soon %d\n", dst - x.dst);
	printf("memset started too soon for size %d\n", i);
      }
      diff = memcheck(dst+i, x.back+i, (x.dst+test_size)-(dst+i), 0);
      if (diff >= 0) {
	//printf("length far  %d\n", (x.dst+test_size)-(dst+i));
	printf("memset went too far for size %d\n", i);
      }
    }
  }
}

int
main(void)
{
  unsigned long long starttime, endtime, zclks;
  unsigned long long cpu_speed;
  int i;
  char *src = &x.src[0];
  char *dst = &x.dst[0];
  int copy_size = 1024;
  /* Do enough reps so that 200 cycles samples = CPI of 1.
     N.B. The sampling period is 2^17, and the inner loop of memcpy_sb1
     processes 64 bytes IFF compiled -D__mips64 */
#ifdef __mips64
  unsigned long long reps = ((1ULL << 17) * 128 * 200) / copy_size;
#else
  unsigned long long reps = ((1ULL << 17) * 32 * 200) / copy_size;
#endif
  unsigned long long bytes = copy_size * reps;

#ifdef PROFILING
  if (sbprof_init(SBPROF_BUFSIZE_DEFAULT, SBPROF_BUFSIZE_DEFAULT) != 0) {
    printf("profiling library initialization failed!\n");
    exit (EXIT_FAILURE);
  }
#endif

#ifdef DO_MEMSET
  printf("running memset\n");
#else
  printf("running memcpy\n");
#endif

  /* Make sure we're profiling correct code */
  test_memset();
  test_memcpy();

  printf("dst %p src %p\n", dst, src);
  printf("copy size  dclks/cacheline   MB/s       zclks       reps\n");
	
  cpu_speed = sbprof_cpu_speed();

#ifdef PROFILING
  sbprof_start();
  sbprof_zbprof_start();
#endif

  starttime = sbprof_zbbus_count();
  for (i = 0; i < reps; i++) {
#ifdef DO_MEMSET
    memset_sb1(dst, 0xe1, copy_size);
#else
    memcpy_sb1(dst, src, copy_size);
#endif
  }
  endtime = sbprof_zbbus_count();
  zclks = endtime - starttime;

#ifdef PROFILING
  sbprof_stop();
  sbprof_zbprof_stop();
#endif

  printf("%8u ", copy_size);
  /* 2 dclks per zclk; 32 bytes per cache line */
  printf("  %8llu    ", (zclks * 2 * 32) / bytes);
  /* Each copied byte is in 3 memory transacations: read of source, and
     read-modify-write of dest.  Hence the numbers below are 1/3rd of the
     memory accesses for the L1-miss case.
     There are cpu_hz() dclks per second.
     1 MB = 10^6 bytes, as in benchmarks like LMbench.
     Similarly, 1 Mhz = 10^6 hz.
  */
  printf("  %8llu", (bytes * cpu_speed) / (1000 * 1000 * (zclks+1) * 2));
  printf("  %10llu %10llu\n", zclks, reps);
  
  exit(EXIT_SUCCESS);
}
