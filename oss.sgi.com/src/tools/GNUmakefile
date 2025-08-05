#-------------------------------------------------------------------#
#-- Makefile for Performer/src/sample directory                   --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2000/11/21 21:39:46 $                                --#
#-------------------------------------------------------------------#
#

ifndef PFROOT
PFROOT = $(ROOT)
endif


CMDS = \
	rgb2raw \
	rsets \
	shrink \
	viewtile

all: $(CMDS)

rgb2raw: rgb2raw.c
	cc -o $@ rgb2raw.c

rsets: rsets.c
	cc -o $@ rsets.c

shrink: shrink.c
	cc -o $@ shrink.c

viewtile: viewtile.c
	cc -o $@ viewtile.c -lGL -L/usr/X11R6/lib -lX11 -lm

.c:
	cc -o $@ $< -limage
.C:
	CC -o $@ $< 

clean:
	rm -f $(CMDS)
