#!smake
#-------------------------------------------------------------------------#
#-- Makefile for libpfcsb						--#
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
LIBRARY	= libpfcsb

CXXFILES = \
        pfcsb.C

CFLAGS+=-O

include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux
