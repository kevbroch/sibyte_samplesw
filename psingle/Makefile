# $Id: Makefile,v 1.4 2004/12/07 07:14:22 cgd Exp $

# standalone (run on top of CFE) build:
CC=	sb1-elf-gcc
LDFLAGS= -Tcfe.ld

CFLAGS=	-march=sb1 -ffast-math -g -O3 \
	-Werror -Wall -Wmissing-prototypes -Wstrict-prototypes

# Remove -mpaired-single below if using a version of gcc which doesn't
# support Paired Single operations.  The generated code won't be as good,
# and code which uses psingle.h won't necessarily be correct for a strict
# interpretation of the MIPS64 architecture, but it's good enough for SB-1.
CFLAGS+= -mpaired-single

all: psingletest

psingletest.o: psingletest.c psingle.h psingle_sb1.h Makefile

clean:
	rm -f psingletest *.o *~ *.bak
