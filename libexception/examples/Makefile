# $Id: Makefile,v 1.7 2004/03/31 03:45:18 cgd Exp $

# Makefile to build various libexception example programs.

CC=	sb1-elf-gcc
CFLAGS= -O2 -Werror -Wall -Wstrict-prototypes -Wmissing-prototypes
LDFLAGS=
LDLIBS=	-lexception -Tcfe.ld

EXAMPLES= exsimple1 exsimple2 exsimple3 extimer1 extimer2 extrace

all: $(EXAMPLES)

clean:
	rm -f $(EXAMPLES) *.o *~
