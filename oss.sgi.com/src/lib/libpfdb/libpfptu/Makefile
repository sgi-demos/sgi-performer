#!smake
#-------------------------------------------------------------------------#
#-- Makefile for libpfptu -- the PTU file loader			--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:34 $					--#
#-------------------------------------------------------------------------#

PFROOT ?= $(ROOT)
include $(PFROOT)/usr/share/Performer/src/pfmakedefs

#if $(PFSTYLE) != "64"
LLDLIBS += -lil
#endif


#-- base name of library
LIBRARY	= libpfptu

CFILES = \
        pfptu.c

CXXFILES = \
	ilgettile.C


include $(PFROOT)/usr/share/Performer/src/pfmakerules
