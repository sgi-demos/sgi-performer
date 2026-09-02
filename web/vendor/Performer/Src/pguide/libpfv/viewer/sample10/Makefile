# -*- makefile -*-
#
# Makefile for sample/pguide/libpfv/viewer/sample10
#
# $Revision: 1.4 $
# $Date: 2002/12/08 05:36:35 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib libpfv.lib

PROGRAMS=sample10.exe

OBJS = \
	$(OUTDIR)/main.obj \
	$(OUTDIR)/myNavigator.obj

!include $(PFROOT)/pfwin32.rules

sample10.exe: $(OBJS)
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$(OUTDIR)/$@ $(EXTRALIBS) $**
