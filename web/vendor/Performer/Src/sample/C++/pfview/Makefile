# -*- makefile -*-
#
# Makefile for sample/apps/C++/pfview
#
# $Revision: 1.1 $
# $Date: 2002/11/21 21:44:57 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib libpfv.lib

PROGRAMS=pfview.exe

!include $(PFROOT)/pfwin32.rules

$(PROGRAMS): $(OUTDIR)/pfview.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$(OUTDIR)/$@ $(EXTRALIBS) $**
