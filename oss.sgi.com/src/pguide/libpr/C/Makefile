#!smake -J 1
#-------------------------------------------------------------------#
#-- Makefile for Performer/src/pguide/libpr/C directory           --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2000/11/21 21:39:39 $                                --#
#-------------------------------------------------------------------#

PFROOT ?= $(ROOT)
include $(PFROOT)/usr/share/Performer/src/pfmakedefs

# Need /usr/include/vl* and /usr/lib/libvl.so for video programs.
# LIBVL = -lvl
# VIDEOCMDS = siriusvidtex 


#-- targets are the executables
TARGETS	= \
	benchgs \
	cliptex \
	cliptex_mwin \
	fillstats \
	geomath \
	gset \
	hlcube \
	icache \
	icache_mwin \
	linmath \
	mipmap \
	texlist \
	texlod \
	zrgb \
	packedattrs \
	pipestats \
	queryftr \
	querysys \
	queryvchan \
	queryperf \
	vchan \
	winfbconfig \
	$(VIDEOCMDS)

OBJECTS = $(TARGETS:=.o)


include $(PFROOT)/usr/share/Performer/src/pfmakerules
