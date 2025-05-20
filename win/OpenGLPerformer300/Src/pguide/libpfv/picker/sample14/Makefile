# -*- makefile -*-
#
# Makefile for sample/pguide/libpfv/picker/sample14
#
# $Revision: 1.5 $
# $Date: 2002/12/09 03:13:32 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. -I../common
EXTRALIBS=libpfui.lib libpfdu-util.lib libpfv.lib

PROGRAMS=sample14.exe

OBJS = \
	$(OUTDIR)/main.obj \
	$(OUTDIR)/mySelector.obj \
	$(OUTDIR)/mySlider.obj \
	$(OUTDIR)/myNavigator.obj

!include $(PFROOT)/pfwin32.rules

{..\common}.cpp{$(OUTDIR)}.obj:
	$(CC) $(CFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@if exist "$(OUTDIR)\$(@F)" erase "$(OUTDIR)\$(@F)"
	@move $(@F) $(OUTDIR)\$(@F)

sample14.exe: $(OBJS)
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$(OUTDIR)/$@ $(EXTRALIBS) $**
