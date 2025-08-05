#!smake -J 1
#-------------------------------------------------------------------#
#-- Makefile for libpfiv -- the OpenInventor file loader          --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                            --#
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
LIBRARY	= libpfiv

#if !defined(PFSTYLE)
PFSTYLE = 32
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

PFROOT ?= $(ROOT)

LCINCS = -I$(PFROOT)/usr/include
LCXXINCS = -I$(PFROOT)/usr/include/CC -I$(PFROOT)/usr/include

#-- interface version pfdLoadFile uses to open loaders
#-- (should be same as that of libpf_*.so and libpfdu_*.so)
DSOVERSION = sgi4.0

#-- object files from which target built {some are in the common directory}
OBJECTS	= \
        pfiv.o \
	Rotor.o \
	Pendulum.o \
	Shuttle.o 

#--
#--	generic targets
#--

#-- make optimized non-dso version of library by default
default: ogldso

#-- make all versions of library
all: dbg opt dso ddso oglopt ogldso ogldbg oglddso

#-- clean up directories {remove junk}
clean:
	if test -d DBG.$(PFRELEASE).IRISGL; then cd DBG.$(PFRELEASE).IRISGL ; rm -f ${OBJECTS} Makedepend core so_locations; cd .. ; fi
	if test -d DBG.$(PFRELEASE).OPENGL; then cd DBG.$(PFRELEASE).OPENGL ; rm -f ${OBJECTS} Makedepend core so_locations; cd .. ; fi
	if test -d OPT.$(PFRELEASE).IRISGL; then cd OPT.$(PFRELEASE).IRISGL ; rm -f ${OBJECTS} Makedepend core so_locations; cd .. ; fi
	if test -d OPT.$(PFRELEASE).OPENGL; then cd OPT.$(PFRELEASE).OPENGL ; rm -f ${OBJECTS} Makedepend core so_locations; cd .. ; fi
	rm -f so_locations

#-- remove all machine-built files
clobber: clean
	if test -d DBG.$(PFRELEASE).IRISGL ; then rm -rf DBG.$(PFRELEASE).IRISGL ; fi
	if test -d DBG.$(PFRELEASE).OPENGL ; then rm -rf DBG.$(PFRELEASE).OPENGL ; fi
	if test -d OPT.$(PFRELEASE).IRISGL ; then rm -rf OPT.$(PFRELEASE).IRISGL ; fi
	if test -d OPT.$(PFRELEASE).OPENGL ; then rm -rf OPT.$(PFRELEASE).OPENGL ; fi

#--
#--	library targets
#--

#-- make a debugging version of the library
igldbg: .MAKE
	@ echo "\nmaking IrisGL DBG version of ${LIBRARY}"
	@ if test ! -d DBG.$(PFRELEASE).IRISGL ; then mkdir -p DBG.$(PFRELEASE).IRISGL ; fi
	@ cd DBG.$(PFRELEASE).IRISGL ; ${MAKE} -f ../Makefile OPTIMIZER=-g LCDEFS=-DIRISGL LCXXDEFS=-DIRISGL \
	    ${LIBRARY}_igl.a

ogldbg: .MAKE
	@ echo "\nmaking OpenGL DBG version of ${LIBRARY}"
	@ if test ! -d DBG.$(PFRELEASE).OPENGL ; then mkdir -p DBG.$(PFRELEASE).OPENGL ; fi
	@ cd DBG.$(PFRELEASE).OPENGL ; ${MAKE} -f ../Makefile OPTIMIZER=-g \
	    ${LIBRARY}_ogl.a

#-- make an optimized version of the library
iglopt: .MAKE
	@ echo "making IrisGL OPT version of ${LIBRARY}"
	@ if test ! -d OPT.$(PFRELEASE).IRISGL ; then mkdir -p OPT.$(PFRELEASE).IRISGL ; fi
	@ cd OPT.$(PFRELEASE).IRISGL ; ${MAKE} -f ../Makefile OPTIMIZER="-O -Olimit 2000" LCDEFS=-DIRISGL LCXXDEFS=-DIRISGL \
	    ${LIBRARY}_igl.a

oglopt: .MAKE
	@ echo "making OpenGL OPT version of ${LIBRARY}"
	@ if test ! -d OPT.$(PFRELEASE).OPENGL ; then mkdir -p OPT.$(PFRELEASE).OPENGL ; fi
	@ cd OPT.$(PFRELEASE).OPENGL ; ${MAKE} -f ../Makefile OPTIMIZER="-O -Olimit 2000" \
	    ${LIBRARY}_ogl.a

#-- make an optimized version of the library that uses DSOs
igldso: .MAKE
	@ echo "making IrisGL DSO version of ${LIBRARY}"
	@ if test ! -d OPT.$(PFRELEASE).IRISGL ; then mkdir -p OPT.$(PFRELEASE).IRISGL ; fi
	@ cd OPT.$(PFRELEASE).IRISGL ; ${MAKE} -f ../Makefile OPTIMIZER="-O -Olimit 2000" LCDEFS=-DIRISGL LCXXDEFS=-DIRISGL \
	${LIBRARY}_igl.so
	@ echo "${LIBRARY}_igl.so done"

ogldso: .MAKE
	@ echo "making OpenGL DSO version of ${LIBRARY}"
	@ if test ! -d OPT.$(PFRELEASE).OPENGL ; then mkdir -p OPT.$(PFRELEASE).OPENGL ; fi
	@ cd OPT.$(PFRELEASE).OPENGL ; ${MAKE} -f ../Makefile OPTIMIZER="-O -Olimit 2000" \
	${LIBRARY}_ogl.so 
	@ echo "${LIBRARY}_ogl.so done"

#-- make an debugging version of the library that uses DSOs
iglddso: .MAKE
	@ echo "making IrisGL DDSO version of ${LIBRARY}"
	@ if test ! -d DBG.$(PFRELEASE).IRISGL ; then mkdir -p DBG.$(PFRELEASE).IRISGL ; fi
	@ cd DBG.$(PFRELEASE).IRISGL ; ${MAKE} -f ../Makefile OPTIMIZER=-g LCDEFS=-DIRISGL LCXXDEFS=-DIRISGL \
	${LIBRARY}_igl.so 
	@ echo "${LIBRARY}_igl.so done"

oglddso: .MAKE
	@ echo "making OpenGL DDSO version of ${LIBRARY}"
	@ if test ! -d DBG.$(PFRELEASE).OPENGL ; then mkdir -p DBG.$(PFRELEASE).OPENGL ; fi
	@ cd DBG.$(PFRELEASE).OPENGL ; ${MAKE} -f ../Makefile OPTIMIZER=-g \
	${LIBRARY}_ogl.so 
	@ echo "${LIBRARY}_ogl.so done"

dso:	ogldso
opt:	oglopt
ddso:	oglddso
dbg:	ogldbg

#--
#--	internal targets
#--

${LIBRARY}_igl.a: ${OBJECTS}
	ar crv $@ $?

${LIBRARY}_ogl.a: ${OBJECTS}
	ar crv $@ $?

${LIBRARY}_igl.so: ${OBJECTS}
	ld  -shared -all -check_registry $(DSOREGFILE) $(DSOVERSIONOPT) \
	$(LDOPTS) -lInventor $(OBJECTS) -o $@

${LIBRARY}_ogl.so: ${OBJECTS}
	ld  -shared -all -check_registry $(DSOREGFILE) $(DSOVERSIONOPT) \
	$(LDOPTS) -lInventor $(OBJECTS) -o $@

#-- look for sources in this directory
.PATH: ..
