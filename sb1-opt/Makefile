# Makefile to build optimized functions into a library.
# $Id: Makefile,v 1.10 2004/09/02 22:15:49 cgd Exp $

# This Makefile assumes that 'CC', 'AR', and 'RANLIB' have been set
# appropriately for the target, and that CC and CFLAGS together
# adequately specify any compilation flags you might want.
#
# Useful defines that you might want to put into CFLAGS:
#
# C1_WORKAROUND		Enable workaround for SB-1 rev 2 C1 erratum.
# M1_WORKAROUND		Enable workaround for SB-1 rev 2 M1 erratum.
# MEMCMP_FUNC		Function to build from memcmp.S (defaults to memcmp).
# MEMCPY_FUNC		Function to build from memcpy.S (defaults to memcpy).
# STRCMP_FUNC		Function to build from strcmp.S (defaults to strcmp).
# FFS_FUNC		Function to build from ffs.S (defaults to ffs).
# FFS64_FUNC		Function to build from ffs64.S (defaults to ffs64).
#
# Note that if you are compiling PIC code (e.g. Linux userland code),
# you should have the __PIC__ preprocessor macro defined.  (MIPS Linux
# tools will take care of this for you automatically.)

# Default values for tools, if not specified already.  (Assumes native,
# e.g. compiling under Linux.)
CC ?= gcc
AR ?= ar
RANLIB ?= ranlib

OPT_LIB  = libsb1opt.a
OPT_OBJS = csum_partial.o memcpy.o memset.o strcmp.o ffs.o ffs64.o

all:

-include Makefile.private

all: $(OPT_LIB)

$(OPT_LIB): $(OPT_OBJS)
	$(AR) crv $@ $(OPT_OBJS)
	$(RANLIB) $@

.S.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean cleandir distclean:
	rm -f $(OPT_LIB) $(OPT_OBJS)
	rm -f a.out core *~
