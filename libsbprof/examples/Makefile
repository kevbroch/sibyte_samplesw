# $Id: Makefile,v 1.5 2004/11/23 05:51:50 cgd Exp $

# Makefile to build various libexception example programs.

CC=	sb1-elf-gcc
CFLAGS= -O2 -Werror -Wall -Wstrict-prototypes -Wmissing-prototypes
LDFLAGS=
LDLIBS=	-lsbprof -lexception -Tcfe.ld

CFLAGS_perfex1=
CFLAGS_memfuncs= -DPROFILING -DDO_MEMSET \
		 -DMEMCPY_FUNC=memcpy_sb1 -DMEMSET_FUNC=memset_sb1

EXAMPLES= memfuncs perfex1

all: $(EXAMPLES)

perfex1: perfex1.c
	$(CC) $(CFLAGS) $(CFLAGS_prefex1) $(LDFLAGS) -o $@ $^ $(LDLIBS)

memfuncs: memfuncs.c ../../sb1-opt/memcpy.S ../../sb1-opt/memset.S
	$(CC) $(CFLAGS) $(CFLAGS_memfuncs) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f core $(EXAMPLES) *.o *~ zb-samples cpu-samples
