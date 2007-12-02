tspsrc = tsp.c tsp.h \
			io.c io.h \
			renormalization.c renormalization.h \
			distance.c distance.h \
			block.c block.h \
			path.h path.c

AM_CPPFLAGS = -Wall -pedantic -g $(GSL_CFLAGS)  -w  -std=c99
AM_LDFLAGS = $(GSL_LIBS)

noinst_PROGRAMS = tsp

tsp_SOURCES = $(tspsrc)
