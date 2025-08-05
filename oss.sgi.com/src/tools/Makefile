#!smake
#
#-------------------------------------------------------------------#
#-- Makefile for Performer/src/sample directory                   --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2000/11/21 21:39:46 $                                --#
#-------------------------------------------------------------------#
#

PFROOT ?= $(ROOT)

CMDS = \
	dioinfo \
	rgb2raw \
	rsets \
	to5551 \
	to888 \
	to8888 \
	toL16 \
	topfi \
	shrink \
	viewtile

all: $(CMDS)

rgb2raw: rgb2raw.c
	cc -o $@ rgb2raw.c

rsets: rsets.c
	cc -o $@ rsets.c

shrink: shrink.c
	cc -o $@ shrink.c

topfi: topfi.c mallocv.o $(PFROOT)/usr/include/Performer/pfdb/pfpfi.h
	cc -I$(PFROOT)/usr/include/Performer/pfdb -o $@ \
		topfi.c mallocv.o -limage

viewtile: viewtile.c
	cc -o $@ viewtile.c -lGL -lX11

.c:
	cc -o $@ $< -limage
.C:
	CC -o $@ $< 

clean:
	rm -f $(CMDS)
