# $Id: Makefile,v 1.10 2003/03/27 00:52:38 cgd Exp $

# standalone (run on top of CFE) build:
CC=	sb1-elf-gcc
LDFLAGS= -Tcfe.ld

CFLAGS=	-march=sb1 -g -O3 \
	-Werror -Wall -Wmissing-prototypes -Wstrict-prototypes

all: mdmxtest accswitchtest

mdmxtest.o: mdmxtest.c mdmx.h mdmx_ob.h mdmx_sb1.h Makefile

accswitchtest.o: accswitchtest.c mdmx.h Makefile

clean:
	rm -f accswitchtest mdmxtest *.o *~ *.bak
