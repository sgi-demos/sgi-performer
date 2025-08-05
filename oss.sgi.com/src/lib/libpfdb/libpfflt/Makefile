#!smake
#-------------------------------------------------------------------#
#-- Makefile for libpfflt -- OpenFlight file loader               --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $					  --#
#--   $Date: 2000/11/21 21:39:33 $                                --#
#-------------------------------------------------------------------#

#-------------------------------------------------------------------#
#-- The default make target makes optimized libraries.            --#
#-- The other targets are:                                        --#
#--    build debugging libraries: dbg (or debug)                  --#
#--    build optimized libraries: opt (or optimize)               --#
#--    build dynamic shared object libraries: dso                 --#
#--    remove all unneeded files after a build: clean             --#
#--    remove all machine generated files: clobber                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- base name of library
LIBRARY	= libpfflt

#if !defined(PFSTYLE)
PFSTYLE = N32
#endif
#if $(PFSTYLE) == "64"
OBJECT_STYLE = 64
LIBBITSUF=64
PFRELEASE=N64
#endif
#if $(PFSTYLE) == "N32"
OBJECT_STYLE = N32_M3
LIBBITSUF=32
PFRELEASE=N32
#endif
#if $(PFSTYLE) == "32"
OBJECT_STYLE = 32
LIBBITSUF=
PFRELEASE=O32
#endif

include $(ROOT)/usr/include/make/commondefs

#-- interface version pfdLoadFile uses to open loaders
#-- (should be same as that of libpf_*.so and libpfdu_*.so)
#
# PF 2.0 is sgi2.0
# PF 2.1 is sgi3.0
# PF 2.2 is sgi4.0
# PF 2.4 is sgi5.0

#if !defined(PFVERSION)
PFVERSION = PF240
#endif
#if $(PFVERSION) == "PF202"
DSOVERSION = sgi2.0
#endif
#if $(PFVERSION) == "PF210"
DSOVERSION = sgi3.0
#endif
#if $(PFVERSION) == "PF220"
DSOVERSION = sgi4.0
#endif
#if $(PFVERSION) == "PF240"
DSOVERSION = sgi5.0
#endif


#-- object files from which target built {some are in the common directory}
OBJECTS	= \
	geom.o \
	hier.o \
	adapters.o \
	align.o \
	behaviors.o \
	cTable.o \
	cat.o \
	clipPlane.o \
	driverControl.o \
	feature.o \
	filterDof.o \
	filterDofHandle.o \
	filterSwitch.o \
	filterSwitchHandle.o \
	gc.o \
	init.o \
	keyTable.o \
	keyTableHandle.o \
	ltPointAnim.o \
	lSource.o \
	mTable.o \
	sTable.o \
	tTable.o \
	tmTable.o \
	userData.o \
	pfdTerrain.o

#if $(DSOVERSION:S/sgi//) < 4.0

OBJECTS += pfuAutoList.o

#endif

BHV_OBJECTS = \
	lnkDriver.o \
	lnkEval.o \
	lnkExpr.o \
	lnkGlue.o \
	lnkGraph.o \
	lnkGraph2.o \
	lnkTrav.o

CAT_OBJECTS = \
	catcompiler.o \
	catlink.o \
	catio.o \
	catloader.o \
	catmerge.o \
	fltstream.o \
	fltvertexpool.o \
	fltkeytable.o \
	mgarray.o \
	mgdictionary.o \
	mgexception.o \
	mgixarray.o \
	mgmemorymgr.o \
	mgreporter.o \
	mgsearch.o \
	mgstringtable.o \
	geom1d.o \
	geom2d.o \
	geom3d.o \
	tin.o \
	tintstrip.o \
	tinnode.o \
	tinquery.o


# IRIX 6.2 includes are broken for -ansi :-/
CVERSION = -xansi

LCOPTS = \
	-fullwarn

LCINCS = \
	-I../libfltbhv/include \
	-I../libfltcat

LC++OPTS = \
	-fullwarn

LC++INCS = \
	-I../libfltbhv/include \

LLDOPTS =

LLDLIBS =

#--
#--	generic targets
#--

#-- make optimized non-dso version of library by default
default: dso

#-- make all versions of library
all: dbg opt dso ddso 
dist: opt dso 

#-- fetch generated dependencies
sinclude Makedepend

#-- clean up directories {remove junk}
clean:
	if test -d DBG.$(PFRELEASE).$(PFVERSION).IRISGL; then cd DBG.$(PFRELEASE).$(PFVERSION).IRISGL ; rm -f ${OBJECTS} Makedepend core so_locations; cd .. ; fi
	if test -d DBG.$(PFRELEASE).$(PFVERSION).OPENGL; then cd DBG.$(PFRELEASE).$(PFVERSION).OPENGL ; rm -f ${OBJECTS} Makedepend core so_locations; cd .. ; fi
	if test -d OPT.$(PFRELEASE).$(PFVERSION).IRISGL; then cd OPT.$(PFRELEASE).$(PFVERSION).IRISGL ; rm -f ${OBJECTS} Makedepend core so_locations; cd .. ; fi
	if test -d OPT.$(PFRELEASE).$(PFVERSION).OPENGL; then cd OPT.$(PFRELEASE).$(PFVERSION).OPENGL ; rm -f ${OBJECTS} Makedepend core so_locations; cd .. ; fi
	rm -f so_locations

#-- remove all machine-built files
clobber: clean
	if test -d DBG.$(PFRELEASE).$(PFVERSION).IRISGL ; then rm -rf DBG.$(PFRELEASE).$(PFVERSION).IRISGL ; fi
	if test -d DBG.$(PFRELEASE).$(PFVERSION).OPENGL ; then rm -rf DBG.$(PFRELEASE).$(PFVERSION).OPENGL ; fi
	if test -d OPT.$(PFRELEASE).$(PFVERSION).IRISGL ; then rm -rf OPT.$(PFRELEASE).$(PFVERSION).IRISGL ; fi
	if test -d OPT.$(PFRELEASE).$(PFVERSION).OPENGL ; then rm -rf OPT.$(PFRELEASE).$(PFVERSION).OPENGL ; fi

#--
#--	library targets
#--

#-- make an optimized version of the library
iglopt: .MAKE
	@ echo "making ${PFVERSION} IrisGL OPT version of ${LIBRARY}"
	@ if test ! -d OPT.$(PFRELEASE).$(PFVERSION).IRISGL ; then mkdir -p OPT.$(PFRELEASE).$(PFVERSION).IRISGL ; fi
	@ cd OPT.$(PFRELEASE).$(PFVERSION).IRISGL ; \
		${MAKE} -f ../Makefile OPTIMIZER=-O \
			VCDEFS=-DIRISGL VCXXDEFS=-DIRISGL \
			${LIBRARY}_igl.a

oglopt: .MAKE
	@ echo "making ${PFVERSION} OpenGL OPT version of ${LIBRARY}"
	@ if test ! -d OPT.$(PFRELEASE).$(PFVERSION).OPENGL ; then mkdir -p OPT.$(PFRELEASE).$(PFVERSION).OPENGL ; fi
	@ cd OPT.$(PFRELEASE).$(PFVERSION).OPENGL ; \
		${MAKE} -f ../Makefile OPTIMIZER=-O \
			${LIBRARY}_ogl.a

#-- make an optimized version of the library that uses DSOs
igldso: .MAKE
	@ echo "making ${PFVERSION} IrisGL DSO version of ${LIBRARY}"
	@ if test ! -d OPT.$(PFRELEASE).$(PFVERSION).IRISGL ; then mkdir -p OPT.$(PFRELEASE).$(PFVERSION).IRISGL ; fi
	@ cd OPT.$(PFRELEASE).$(PFVERSION).IRISGL ; \
		${MAKE} -f ../Makefile OPTIMIZER=-O DSOVERSION=${DSOVERSION} \
			VCDEFS=-DIRISGL VCXXDEFS=-DIRISGL \
			${LIBRARY}_igl.so
	@ echo "${LIBRARY}_igl.so done"

ogldso: .MAKE
	@ echo "making ${PFVERSION} OpenGL DSO version of ${LIBRARY}"
	@ if test ! -d OPT.$(PFRELEASE).$(PFVERSION).OPENGL ; then mkdir -p OPT.$(PFRELEASE).$(PFVERSION).OPENGL ; fi
	@ cd OPT.$(PFRELEASE).$(PFVERSION).OPENGL ; \
		${MAKE} -f ../Makefile OPTIMIZER=-O DSOVERSION=${DSOVERSION} \
			${LIBRARY}_ogl.so 
	@ echo "${LIBRARY}_ogl.so done"

#-- make a debugging version of the library
igldbg: .MAKE
	@ echo "\nmaking ${PFVERSION} IrisGL DBG version of ${LIBRARY}"
	@ if test ! -d DBG.$(PFRELEASE).$(PFVERSION).IRISGL ; then mkdir -p DBG.$(PFRELEASE).$(PFVERSION).IRISGL ; fi
	@ cd DBG.$(PFRELEASE).$(PFVERSION).IRISGL ; \
		${MAKE} -f ../Makefile OPTIMIZER=-g \
			VCDEFS=-DIRISGL VCXXDEFS=-DIRISGL \
			${LIBRARY}_igl.a

ogldbg: .MAKE
	@ echo "\nmaking ${PFVERSION} OpenGL DBG version of ${LIBRARY}"
	@ if test ! -d DBG.$(PFRELEASE).$(PFVERSION).OPENGL ; then mkdir -p DBG.$(PFRELEASE).$(PFVERSION).OPENGL ; fi
	@ cd DBG.$(PFRELEASE).$(PFVERSION).OPENGL ; \
		${MAKE} -f ../Makefile OPTIMIZER=-g \
			${LIBRARY}_ogl.a

#-- make an debugging version of the library that uses DSOs
iglddso: .MAKE
	@ echo "making ${PFVERSION} IrisGL DDSO version of ${LIBRARY}"
	@ if test ! -d DBG.$(PFRELEASE).$(PFVERSION).IRISGL ; then mkdir -p DBG.$(PFRELEASE).$(PFVERSION).IRISGL ; fi
	@ cd DBG.$(PFRELEASE).$(PFVERSION).IRISGL ; \
		${MAKE} -f ../Makefile OPTIMIZER=-g DSOVERSION=${DSOVERSION} \
			VCDEFS=-DIRISGL VCXXDEFS=-DIRISGL \
			${LIBRARY}_igl.so 
	@ echo "${LIBRARY}_igl.so done"

oglddso: .MAKE
	@ echo "making ${PFVERSION} OpenGL DDSO version of ${LIBRARY}"
	@ if test ! -d DBG.$(PFRELEASE).$(PFVERSION).OPENGL ; then mkdir -p DBG.$(PFRELEASE).$(PFVERSION).OPENGL ; fi
	@ cd DBG.$(PFRELEASE).$(PFVERSION).OPENGL ; \
		${MAKE} -f ../Makefile OPTIMIZER=-g DSOVERSION=${DSOVERSION} \
			${LIBRARY}_ogl.so
	@ echo "${LIBRARY}_ogl.so done"

dso:	ogldso
opt:	oglopt
ddso:	oglddso
dbg:	ogldbg

#--
#--	internal targets
#--

${BHV_OBJECTS} ${CAT_OBJECTS}: .DONTCARE

${LIBRARY}_igl.a: ${OBJECTS} ${BHV_OBJECTS} ${CAT_OBJECTS}
	${AR} crv $@ $?

${LIBRARY}_ogl.a: ${OBJECTS} ${BHV_OBJECTS} ${CAT_OBJECTS}
	${AR} crv $@ $?

${LIBRARY}_igl.so: ${LIBRARY}_igl.a
	${LD} -shared -all -check_registry $(DSOREGFILE) $(DSOVERSIONOPT) \
	${VLDOPTS} \
	${GLDOPTS} \
	${LLDOPTS} \
	$? \
	${LLDLIBS} \
	${VLDLIBS} \
	${GLDLIBS} \
	-o $@

${LIBRARY}_ogl.so: ${LIBRARY}_ogl.a
	${LD} -shared -all -check_registry $(DSOREGFILE) $(DSOVERSIONOPT) \
	${VLDOPTS} \
	${GLDOPTS} \
	${LLDOPTS} \
	$? \
	${LLDLIBS} \
	${VLDLIBS} \
	${GLDLIBS} \
	-o $@

#-- look for sources in this directory
.PATH: \
	.. \
	../common

#-- look for Behavior and CAT objects in these directories
.PATH.o: \
	../libfltbhv/BHV/OPT.${PFRELEASE} \
	../libfltcat/CAT/OPT.${PFRELEASE}
