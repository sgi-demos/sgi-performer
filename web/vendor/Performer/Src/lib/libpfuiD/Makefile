#-------------------------------------------------------------------------#
#-- user Makefile for libpfuiD on win32                             --#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.2 $							--#
#--   $Date: 2002/11/22 23:16:38 $					--#
#-------------------------------------------------------------------------#

!include $(PFROOT)/pfwin32.defs

LIBNAME=pfuiD
LIBS=lib$(LIBNAME).lib

EXTRACFLAGS=-D__BUILD_PFUI__ -D_DLL $(LOCALCFLAGS)
EXTRACXXFLAGS=$(EXTRACFLAGS)
EXTRALIBS=libpfdu-util.lib

!include $(PFROOT)/pfwin32.rules

OBJS=\
	$(OUTDIR)/pfiInitD.obj \
	$(OUTDIR)/pfiCollideD.obj \
	$(OUTDIR)/pfiInputXformD.obj \
	$(OUTDIR)/pfiPickD.obj \
	$(OUTDIR)/pfiXformerD.obj \
	$(OUTDIR)/pfiDriveD.obj \
	$(OUTDIR)/pfiFlyD.obj \
	$(OUTDIR)/pfiSphericD.obj \
	$(OUTDIR)/pfiTrackballD.obj \
	$(OUTDIR)/cCollideD.obj \
	$(OUTDIR)/cPickD.obj \
	$(OUTDIR)/cInputXformD.obj \
	$(OUTDIR)/cXformerD.obj \
	$(NULL)

lib$(LIBNAME).lib: $(OBJS) 
	$(LINK) $(DLL_LINK_FLAGS) /out:$(OUTDIR)/$(@:.lib=).dll $(EXTRALIBS) $**
