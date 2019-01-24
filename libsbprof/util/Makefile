# $Id: Makefile,v 1.1 2003/05/29 22:05:06 kwalker Exp $

# Makefile to build various libsbprof utilities.

CC=	gcc
CFLAGS= -O2 -Werror -Wall -Wstrict-prototypes -Wmissing-prototypes
LDFLAGS=
LDLIBS=

UTILS= tbanal

all: $(UTILS)

clean:
	rm -f core $(UTILS) *.o *~
