#!smake -J 1
#-------------------------------------------------------------------------#
#-- Makefile for "fogfly" the basic demonstration program		--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:43 $					--#
#-------------------------------------------------------------------------#

PFROOT ?= $(ROOT)
include $(PFROOT)/usr/share/Performer/src/pfmakedefs

#-- provide a list of alternate locations for file searches
UNIQUE	= ..
COMMON	= ../../common

PFROOT ?= $(ROOT)

#-- alternate locatins for included files
LCINCS += \
	-I${UNIQUE} \
	-I${COMMON}


#-- base name of program
TARGET	= fogfly

#-- files from which the target is built {some are in the common directory}
CFILES = \
	cmdline.c \
	configfile.c \
	generic.c \
	env.c \
	gui.c \
	keybd.c \
	perfly.c \
        fog.c \
	main.c


include $(PFROOT)/usr/share/Performer/src/pfmakerules

#-- objects are built from either unique or common files
.PATH: ${UNIQUE} ${COMMON}
