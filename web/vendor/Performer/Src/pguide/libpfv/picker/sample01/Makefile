# -*- makefile -*-
#
# Makefile for sample/pguide/libpfv/picker/sample01
#
# $Revision: 1.4 $
# $Date: 2002/12/08 05:36:35 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib libpfv.lib

PROGRAMS=sample01.exe

OBJS = \
	$(OUTDIR)/main.obj \
	$(OUTDIR)/myInteractor.obj

!include $(PFROOT)/pfwin32.rules

sample01.exe: $(OBJS)
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$(OUTDIR)/$@ $(EXTRALIBS) $**
