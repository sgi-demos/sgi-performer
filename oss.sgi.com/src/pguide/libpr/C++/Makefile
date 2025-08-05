#!smake -J 1
#-------------------------------------------------------------------#
#-- Makefile for Performer/src/pguide/libpr/C++ directory         --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2000/11/21 21:39:41 $                                --#
#-------------------------------------------------------------------#

PFROOT ?= $(ROOT)
include $(PFROOT)/usr/share/Performer/src/pfmakedefs

# C++ programs must use CC as linker
LINKERF=$(CXXF)

#-- targets are the executables
TARGETS	= \
	fillstats \
	geomath \
	gset \
	hlcube \
	iostream \
	linmath \
	winfbconfig \
	zrgb 

OBJECTS = $(TARGETS:=.o)


include $(PFROOT)/usr/share/Performer/src/pfmakerules
