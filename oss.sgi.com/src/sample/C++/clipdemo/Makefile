#!smake
#-------------------------------------------------------------------------#
#-- Makefile for "clipdemo" the cliptexture demonstration program	--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:45 $					--#
#-------------------------------------------------------------------------#

PFROOT ?= $(ROOT)
include $(PFROOT)/usr/share/Performer/src/pfmakedefs

#-- provide a list of alternate locations for file searches
UNIQUE	= ..

PFROOT ?= $(ROOT)

#-- alternate locatins for included files
LCXXINCS = \
	-I${UNIQUE} \
	-I$(PFROOT)/usr/include/CC \
	-I$(PFROOT)/usr/include

#-- this is a C++ program, so use CC as the linker
LINKERF = $(CXXF)

#-- base name of program
TARGET	= clipdemo

#-- files from which the target is built
CXXFILES = \
	Bg.C \
	clipdemo.C \
	dbase.C \
	keybd.C \
	pov.C

include $(PFROOT)/usr/share/Performer/src/pfmakerules

