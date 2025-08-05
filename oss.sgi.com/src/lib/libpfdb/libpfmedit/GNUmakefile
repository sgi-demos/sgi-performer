#!smake
#-------------------------------------------------------------------------#
#-- Makefile for libpfmedit -- the Medit file loader			--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:33 $					--#
#-------------------------------------------------------------------------#

ifndef PFROOT
PFROOT = $(ROOT)
endif

include $(PFROOT)/usr/share/Performer/src/pfmakedefs.linux


#-- base name of library
LIBRARY	= libpfmedit

CFILES = \
	meditfile.c \
	pfSaveMedit.c \
	pfmedit.c 


include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux
