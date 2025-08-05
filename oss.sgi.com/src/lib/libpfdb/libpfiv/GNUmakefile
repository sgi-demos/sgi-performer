#-------------------------------------------------------------------------#
#-- Makefile for libpfiv -- the OpenInventor file loader		--#
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
LIBRARY	= libpfiv

LCXXDEFS = -D_GCC_ -D__STDC_LIMIT_MACROS

CXXFILES = \
        pfiv.C \
	Rotor.C \
	Shuttle.C \
	Pendulum.C

LDOPTS += -lInventor -lInventorXt -lc

include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux
