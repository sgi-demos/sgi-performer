#-------------------------------------------------------------------#
#-- Makefile for Performer/src/pguide/libpr/C directory           --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2000/11/21 21:39:39 $                                --#
#-------------------------------------------------------------------#

ifndef PFROOT
PFROOT = $(ROOT)
endif

include $(PFROOT)/usr/share/Performer/src/pfmakedefs.linux

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
	queryperf \
	vchan \
	winfbconfig

OBJECTS = $(TARGETS:=.o)


include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux
