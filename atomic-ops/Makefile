# $Id: Makefile,v 1.5 2003/03/26 18:41:26 cgd Exp $

# standalone (run on top of CFE) build:
CC=	sb1-elf-gcc
LDFLAGS= -Tcfe.ld

CFLAGS=	-march=sb1 -g -O3 \
	-Werror -Wall -Wmissing-prototypes -Wstrict-prototypes

all: semaphore spinlock

semaphore.o: semaphore.c semaphore.h Makefile

spinlock.o: spinlock.c spinlock.h Makefile

clean:
	rm -f semaphore spinlock *.o *~ *.bak
