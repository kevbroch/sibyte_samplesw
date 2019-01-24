# $Id: Makefile,v 1.10 2003/11/14 17:46:21 kwalker Exp $

# Utility programs meant to run on a host system:
HOST_CC= gcc
HOST_CFLAGS= -g -DHOST_UTILITY \
	     -Werror -Wall -Wmissing-prototypes -Wstrict-prototypes
HOST_LDFLAGS=

# standalone (run on top of CFE) build:
CC=	sb1-elf-gcc
CFLAGS=	-march=sb1 -g \
	-Werror -Wall -Wmissing-prototypes -Wstrict-prototypes
LDFLAGS= -Tcfe.ld

PROGS = tlbdump tlbtest tlbtestalloc tlbconvert

default: all

tlbdump: tlbdump.o tlb_utils.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

tlbtest: tlbtest.o tlb_utils.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

tlbconvert: tlbconvert.c
	$(HOST_CC) $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $@ $^

tlballoc: tlballoc.o tlb_utils.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

tlbtestalloc: tlballoc.o tlbtestalloc.o tlb_utils.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

tlbdump.o: tlbdump.c tlb.h tlb_utils.h Makefile

tlbtest.o: tlbtest.c tlb.h tlb_utils.h Makefile

tlballoc.o: tlballoc.c tlb.h tlb_utils.h Makefile

tlbtestalloc.o: tlbtestalloc.c tlb.h tlb_utils.h Makefile

tlb_utils.o: tlb_utils.c tlb.h tlb_utils.h Makefile

-include Makefile.private

all: $(PROGS)

clean:
	rm -f $(PROGS) *.o *~ *.bak
