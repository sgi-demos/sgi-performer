# -*- makefile -*-
#
# Makefile for sample/pguide/libpfv/viewer/sample06
#
# $Revision: 1.4 $
# $Date: 2002/12/08 05:36:35 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib libpfv.lib

PROGRAMS=sample06.exe

OBJS = \
	$(OUTDIR)/main.obj \
	$(OUTDIR)/myNavigator.obj \
	$(OUTDIR)/UndoManager.obj \
	$(OUTDIR)/myDragger.obj \
	$(OUTDIR)/myPickerModule.obj \
	$(OUTDIR)/myRotProjector.obj \
	$(OUTDIR)/mySelector.obj \
	$(OUTDIR)/myTransProjector.obj

!include $(PFROOT)/pfwin32.rules

sample06.exe: $(OBJS)
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$(OUTDIR)/$@ $(EXTRALIBS) $**
