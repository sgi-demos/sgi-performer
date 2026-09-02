# -*- makefile -*-
#
# Makefile for sample/pguide/libpfv/viewer/sample04
#
# $Revision: 1.4 $
# $Date: 2002/12/08 05:36:35 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib libpfv.lib

PROGRAMS=sample04.exe

!include $(PFROOT)/pfwin32.rules

sample04.exe: $(OUTDIR)/main.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$(OUTDIR)/$@ $(EXTRALIBS) $**
