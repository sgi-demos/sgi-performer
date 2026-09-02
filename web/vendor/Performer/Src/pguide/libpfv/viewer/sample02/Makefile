# -*- makefile -*-
#
# Makefile for sample/pguide/libpfv/viewer/sample02
#
# $Revision: 1.4 $
# $Date: 2002/12/08 05:36:35 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib libpfv.lib

PROGRAMS=sample02.exe

!include $(PFROOT)/pfwin32.rules

sample02.exe: $(OUTDIR)/main.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$(OUTDIR)/$@ $(EXTRALIBS) $**
