#-------------------------------------------------------------------------#
#-- Makefile for pfvmPicker Module for pfvViewer			--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.2 $							--#
#--   $Date: 2002/12/06 00:37:01 $					--#
#-------------------------------------------------------------------------#

LIBNAME=pfvmPicker

LOCALCFLAGS= /TP

!include ../win32nmake

OBJS= \
	$(OUTDIR)/pfvmDragger.obj \
	$(OUTDIR)/pfvmPicker.obj \
	$(OUTDIR)/pfvmRotProjector.obj \
	$(OUTDIR)/pfvmSelector.obj \
	$(OUTDIR)/pfvmTransProjector.obj \
	$(OUTDIR)/pfvmUndoManager.obj

lib$(LIBNAME).lib: $(OBJS)
        $(LINK) $(DLL_LINK_FLAGS) /out:$(OUTDIR)/$(@:.lib=).dll $(EXTRALIBS) $**
