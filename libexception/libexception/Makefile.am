
AUTOMAKE_OPTIONS=foreign

AM_CFLAGS = -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes

lib_LIBRARIES = libexception.a

libexception_a_SOURCES = \
			 exframe_dump.c \
			 exhandler_abort.c \
			 exhandler_mdmxenable.c \
			 exhandler_panic.c \
			 exlib.c \
			 exlib_panic.c \
			 exlib_set_common_handlers.c \
			 exlib_vectors.S \
			 exlib_vectors_mips32.S \
			 intr.c

include_HEADERS = exlib.h

