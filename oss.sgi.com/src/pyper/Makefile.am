## Process this with automake to produce Makefile.in

##
## $Source: /oss/CVS/cvs/performer/src/pyper/Makefile.am,v $
## $Author: flynnt $
## $Revision: 1.1 $
##

SUBDIRS = tests

CLEANFILES = libpyper.py libpyperbonus.py libpyper_wrap.C libpyperbonus_wrap.C 


BUILT_SOURCES = \
libpyper_wrap.C \
libpyperbonus_wrap.C \
libpyper.py \
libpyperbonus.py

PYPER_INTERFACEFILES = \
billboard.i \
channel.i \
dcs.i \
font.i \
geode.i \
geomath.i \
geoset.i \
geostate.i \
group.i \
lightsource.i \
linmath.i \
material.i \
memory.i \
node.i \
opengl.i \
pyper.i \
pf.i \
pfType.i \
pfdb.i \
pfdu.i \
pfutil.i \
pipe.i \
pipewindow.i \
pr.i \
scene.i \
scs.i \
string.i \
switch.i \
transformanimator.i \
text.i

SUPPORT_INTERFACEFILES = \
bonus.i

EXTRA_DIST = $(PYPER_INTERFACEFILES) $(SUPPORT_INTERFACEFILES)

pkglib_LTLIBRARIES = libpypercmodule.la libpyperbonuscmodule.la


pkgdata_SCRIPTS = \
  libpyper.py \
  libpyperbonus.py


libpypercmodule_la_SOURCES = \
  libpyper_wrap.C 

libpyperbonuscmodule_la_SOURCES = \
  libpyperbonus_wrap.C \
  anievent.C \
  anievent.hh \
  aniint.C \
  aniint.hh \
  bugpatch.C \
  handlers.C \
  handlers.h \
  hermite.hh \
  prt.h \
  ptimer.hh \
  quatevnt.C \
  quatevnt.hh \
  quatint.C \
  quatint.hh \
  scalevnt.C \
  scalevnt.hh \
  scalint.C \
  scalint.hh \
  strevent.C \
  strevent.hh \
  strint.C \
  strint.hh \
  transformanimator.C \
  transformanimator.h \
  v3event.C \
  v3event.hh \
  v3int.C \
  v3int.hh


INCLUDES += \
  -I@PYTHON_INC@


##
## Writing Makefile.am's for SWIG related stuff took great effort.
## However, here's the result in all its glory.
## Note that the space after '-o' is compulsory, yet after '-I' it's forbidden.
## Also note: automake forces me to have a shared lib name be prefixed with 'lib',
## this is a nuisance!
##      - Bram Stolk
##

libpyper_wrap.C: $(PYPER_INTERFACEFILES)
	swig -c -shadow -module libpyper -python -c++  \
        -I$(top_srcdir) \
        -o $(top_builddir)/libpyper_wrap.C \
        $(top_srcdir)/pyper.i 

libpyperbonus_wrap.C: $(SUPPORT_INTERFACEFILES)
	swig -c -shadow -module libpyperbonus -python -c++  \
        -I$(top_srcdir) \
        -o $(top_builddir)/libpyperbonus_wrap.C \
        $(top_srcdir)/bonus.i 


## $Log: Makefile.am,v $
## Revision 1.1  2001/05/21 21:40:00  flynnt
## Doing some cleanup and adding the pfgtk example and the python wrapper for
## Performer (pyper).
##
## Revision 1.11  2001/04/09 13:50:20  bram
## Renamed pypersupport to pyperbonus
##
## Revision 1.10  2000/11/21 13:05:21  bram
## improved transform animator
##
## Revision 1.9  2000/11/16 13:51:56  bram
## added transform animator
##
## Revision 1.8  2000/11/14 14:59:26  bram
## added non working bugpatch
##
## Revision 1.7  2000/11/09 14:57:11  bram
## added switch node, improved event routing
##
## Revision 1.6  2000/09/11 15:11:33  bram
## created seperate module for pyper addons
##
## Revision 1.5  2000/09/07 16:29:11  bram
## improved configure.in
##
## Revision 1.4  2000/09/07 15:47:17  bram
## misc fixes
##
## Revision 1.3  2000/08/31 17:23:19  bram
## working anim
##
## Revision 1.2  2000/08/23 13:01:50  bram
## fixed installation process
##
## Revision 1.1.1.1  2000/08/23 08:02:41  bram
## Added a PyPer version to the repository
##

