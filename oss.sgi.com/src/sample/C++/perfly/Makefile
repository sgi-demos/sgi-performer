#!smake
#-------------------------------------------------------------------------#
#-- Makefile for "perfly" the basic demonstration program		--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:45 $					--#
#-------------------------------------------------------------------------#

PFROOT ?= $(ROOT)
include $(PFROOT)/usr/share/Performer/src/pfmakedefs

#-- provide a list of alternate locations for file searches
UNIQUE	= ..
COMMON	= ../../common

PFROOT ?= $(ROOT)

#-- alternate locatins for included files
LCXXINCS = \
	-I${UNIQUE} \
	-I${COMMON} \
	-I$(PFROOT)/usr/include/CC \
	-I$(PFROOT)/usr/include

#-- this is a C++ program, so use CC as the linker
LINKERF = $(CXXF)

#-- base name of program
TARGET	= perfly

#-- files from which the target is built {some are in the common directory}
CXXFILES = \
	cmdline.C \
	generic.C \
	env.C \
	gui.C \
	keybd.C \
	perfly.C \
        configfile.C \
	main.C


include $(PFROOT)/usr/share/Performer/src/pfmakerules

#-- objects are built from either unique or common files
.PATH: ${UNIQUE} ${COMMON}
