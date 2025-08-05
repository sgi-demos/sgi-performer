#-------------------------------------------------------------------#
#-- Makefile for Performer/src/pguide/libpr/C++ directory         --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2000/11/21 21:39:41 $                                --#
#-------------------------------------------------------------------#

ifndef PFROOT
PFROOT = $(ROOT)
endif

include $(PFROOT)/usr/share/Performer/src/pfmakedefs.linux

# C++ programs must use CC as linker
LINKERF=$(CXXF)

#-- targets are the executables
TARGETS	= \
	fillstats \
	geomath \
	gset \
	hlcube \
	linmath \
	winfbconfig \
	iostream \
	zrgb

OBJECTS = $(TARGETS:=.o)
CXXFILES = $(TARGETS:=.C)


include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux
