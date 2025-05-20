# -*- makefile -*-
#
# Makefile for sample/pguide/libpfv/picker/sample04
#
# $Revision: 1.3 $
# $Date: 2002/12/08 05:36:35 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib libpfv.lib

PROGRAMS=sample04.exe

OBJS = \
	$(OUTDIR)/main.obj \
	$(OUTDIR)/myInteractor.obj

!include $(PFROOT)/pfwin32.rules

sample04.exe: $(OBJS)
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$(OUTDIR)/$@ $(EXTRALIBS) $**
