# $Id: Makefile,v 1.2 2004/03/20 06:49:06 cgd Exp $

# pick one of: sb1-elf, NetBSD
ENVIRONMENT=	sb1-elf

ifeq ($(ENVIRONMENT), sb1-elf)
# standalone (run on top of CFE) build:
CC=	sb1-elf-gcc
CFLAGS= -march=sb1 -DUNHANDLER_HEADER="\"unhandler_exlib.h\""
LDFLAGS=
LDLIBS= -lexception -Tcfe.ld
endif

ifeq ($(ENVIRONMENT), NetBSD)
# NetBSD native
CC=	gcc
CFLAGS= -DUNHANDLER_HEADER="\"unhandler_netbsd.h\""
LDFLAGS=
LDLIBS=
endif

CFLAGS+= -g -O3 \
	-Werror -Wall -Wmissing-prototypes -Wstrict-prototypes

all: undemo

undemo: undemo.o unhandler.o

clean:
	rm -f undemo *.o *~ *.bak
