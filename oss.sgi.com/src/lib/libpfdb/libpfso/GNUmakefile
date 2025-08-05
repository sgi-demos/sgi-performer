#!smake
#-------------------------------------------------------------------------#
#-- Makefile for libpfso -- the DSO file loader				--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:35 $					--#
#-------------------------------------------------------------------------#

ifndef PFROOT
PFROOT = $(ROOT)
endif

include $(PFROOT)/usr/share/Performer/src/pfmakedefs.linux


#-- base name of library
LIBRARY	= libpfso

CXXFILES = \
        pfso.C

CFLAGS+=-O

include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux
