#-------------------------------------------------------------------------#
#-- Makefile for libpfct -- the CT file loader				--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.4 $							--#
#--   $Date: 2002/12/01 21:11:03 $					--#
#-------------------------------------------------------------------------#

!include $(PFROOT)/pfwin32.defs

LIBNAME = pfct

LOCALCFLAGS=

LIBS=lib$(LIBNAME).lib

OBJS=\
	$(OUTDIR)/$(LIBNAME).obj  \
	$(OUTDIR)/pfcte.obj       \
	$(NULL)

EXTRACFLAGS=-D__BUILD_LIBPFDB__ -D_DLL $(LOCALCFLAGS)
EXTRALIBS=libpfdu-util.lib /LIBPATH:"$(PFROOT)"\lib\libpfdb 
EXTRACXXFLAGS=$(EXTRACFLAGS)

!include $(PFROOT)/pfwin32.rules

lib$(LIBNAME).lib: $(OBJS) 
	$(LINK) $(DLL_LINK_FLAGS) /out:$(OUTDIR)/$(@:.lib=).dll $(EXTRALIBS) $**
