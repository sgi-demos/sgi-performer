# -*- makefile -*-
#
# Makefile for sample/apps/C++/rotorwash
#
# $Revision: 1.1 $
# $Date: 2002/12/04 22:02:40 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib libpfv.lib

PROGRAMS=framework.exe

!include $(PFROOT)/pfwin32.rules

$(PROGRAMS): $(OUTDIR)/framework.obj $(OUTDIR)/sea.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$(OUTDIR)/$@ $(EXTRALIBS) $**
