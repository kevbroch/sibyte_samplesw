CC=	sb1-elf-gcc
LD=	sb1-elf-ld
CFLAGS=	-O2 -Werror -Wall -Wstrict-prototypes -Wmissing-prototypes
CPPFLAGS=	-I../sb1250/include
LDFLAGS=
LDLIBS=	-lexception -Tcfe.ld

EXAMPLES= cerr inject_cerr.oo

all: $(EXAMPLES)

cerr: cerr.o cache_err.o

inject_cerr.oo: inject_cerr.o
	$(LD) -o $@ -Ttext 0xb0000480 -Tdata 0xb0010000 $^

clean:
	rm -f $(EXAMPLES) *.o *~
