#-------------------------------------------------------------------------#
#-- Makefile for "asdfly" the demonstration program for ASD		--#
#-- morphing terrain library						--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:43 $					--#
#-------------------------------------------------------------------------#

ifndef PFROOT
PFROOT = $(ROOT)
endif

include $(PFROOT)/usr/share/Performer/src/pfmakedefs.linux

#-- provide a list of alternate locations for file searches
UNIQUE	= ..
COMMON	= ../../common

ifndef PFROOT
PFROOT = $(ROOT)
endif


#-- alternate locatins for included files
LCINCS += \
	-I${UNIQUE} \
	-I${COMMON}


#-- base name of program
TARGET	= asdfly

#-- files from which the target is built {some are in the common directory}
CFILES = \
	cmdline.c \
	configfile.c \
	generic.c \
	env.c \
	gui.c \
	keybd.c \
	perfly.c \
	main.c \
        terrain.c \
        eval_function.c \
        flyLightPoints.c \
        pfuLightPoints.c


include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux

#-- objects are built from either unique or common files
VPATH=${UNIQUE}:${COMMON}
