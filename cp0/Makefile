# $Id: Makefile,v 1.3 2004/12/07 07:22:18 cgd Exp $

# standalone (run on top of CFE) build:
CC=	sb1-elf-gcc
CFLAGS=	-march=sb1 -g \
	-Werror -Wall -Wmissing-prototypes -Wstrict-prototypes
LDFLAGS= -Tcfe.ld

PROGS = cp0dump prid_decode

all: $(PROGS)

cp0dump.o: cp0dump.c Makefile

cp0dump: cp0dump.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

prid_decode.o: prid_decode.c Makefile

prid_decode: prid_decode.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(PROGS) *.o *~ *.bak
