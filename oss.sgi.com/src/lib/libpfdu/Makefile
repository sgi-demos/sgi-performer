#!smake -J 1
#-------------------------------------------------------------------------#
#-- Makefile for Performer/src/lib/libpfdu				--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:35 $					--#
#-------------------------------------------------------------------------#

PFROOT ?= $(ROOT)
include $(PFROOT)/usr/share/Performer/src/pfmakedefs

SRCDIR ?= ..

#-- base name of library
LIBRARY	= libpfdu

CFILES = \
    objfnt.c         \
    pfdASDAlign.c    \
    pfdASDGen.c      \
    pfdASDClipring.c \
    pfdBillboard.c   \
    pfdBreakup.c     \
    pfdBuildASD.c    \
    pfdProcASD.c     \
    pfdBuilder.c     \
    pfdCallbacks.c   \
    pfdCleanTree.c   \
    pfdExtractTris.c \
    pfdGSet.c        \
    pfdGeoBuilder.c  \
    pfdImage.c       \
    pfdLayer.c       \
    pfdLoadFile.c    \
    pfdLoadFont.c    \
    pfdLoadImage.c   \
    pfdLoadShader.c  \
    pfdLoadShader_Lex.c     \
    pfdLoadShader_Parse.c   \
    pfdOpenFile.c    \
    pfdShare.c       \
    pfdSpatial.c     \
    pfdTMesher.c     \
    type1rd.c


include $(PFROOT)/usr/share/Performer/src/pfmakerules

#if $(RELEASE) < 6.5
pfdLoadShader_Lex.c: pfdLoadShader_Lex.lxx pfdLoadShader_Parse.c
        flex -t -P__pfdLoadShader_ $(SRCDIR)/pfdLoadShader_Lex.lxx > $@
#else
pfdLoadShader_Lex.c: pfdLoadShader_Lex.lxx pfdLoadShader_Parse.c
	flex -P__pfdLoadShader_ -o$@ $(SRCDIR)/pfdLoadShader_Lex.lxx
#endif

pfdLoadShader_Parse.c: pfdLoadShader_Parse.yxx
	bison -t -v -d -p__pfdLoadShader_ $(SRCDIR)/pfdLoadShader_Parse.yxx -o$@
