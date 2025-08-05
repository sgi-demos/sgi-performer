#!smake -J 1
#-------------------------------------------------------------------#
#-- Makefile for Performer/src/pguide/libpui directory            --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2000/11/21 21:39:39 $                                --#
#-------------------------------------------------------------------#

PFROOT ?= $(ROOT)

# set some flags used by pfcommondefs

# set PFDOMOTIF to build with motif libs
# needed for motifxformer.c

PFDOMOTIF ?= 1
#if $(PFDOMOTIF) == 1
NEED_MOTIF_LIBS=1
#endif


include $(PFROOT)/usr/share/Performer/src/pfmakedefs



#-- targets are the executables
TARGETS	= \
	trackball	\
	drive		\
	fly		\
	motifxformer	\
	pick

OBJECTS = $(TARGETS:=.o)


include $(PFROOT)/usr/share/Performer/src/pfmakerules
