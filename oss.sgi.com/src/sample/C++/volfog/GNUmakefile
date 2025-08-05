#-------------------------------------------------------------------------#
#-- Makefile for "volfog" the demonstration program			--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:46 $					--#
#-------------------------------------------------------------------------#

ifndef PFROOT
PFROOT = $(ROOT)
endif

include $(PFROOT)/usr/share/Performer/src/pfmakedefs.linux

#-- provide a list of alternate locations for file searches
UNIQUE	= ..

ifndef PFROOT
PFROOT = $(ROOT)
endif


#-- alternate locatins for included files
LCINCS += \
	-I${UNIQUE}


#-- base name of program
TARGET	= volfog

#-- files from which the target is built {some are in the common directory}
CXXFILES = \
	draw.C \
	smoke.C \
	fog.C


include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux

#-- objects are built from either unique or common files
VPATH=${UNIQUE}
