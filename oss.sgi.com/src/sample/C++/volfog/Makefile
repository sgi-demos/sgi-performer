#!smake -J 1
#-------------------------------------------------------------------------#
#-- Makefile for "volfog" the demonstration program			--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:46 $					--#
#-------------------------------------------------------------------------#

PFROOT ?= $(ROOT)
include $(PFROOT)/usr/share/Performer/src/pfmakedefs

#-- provide a list of alternate locations for file searches
UNIQUE	= ..

PFROOT ?= $(ROOT)

#-- alternate locatins for included files
LCINCS += \
	-I${UNIQUE}

#-- this is a C++ program, so use CC as the linker
LINKERF = $(CXXF)

#-- base name of program
TARGET	= volfog

#-- files from which the target is built {some are in the common directory}
C++FILES = \
	draw.C \
	smoke.C \
	fog.C


include $(PFROOT)/usr/share/Performer/src/pfmakerules

#-- objects are built from either unique or common files
.PATH: ${UNIQUE}
