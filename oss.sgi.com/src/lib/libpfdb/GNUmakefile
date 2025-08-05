#-------------------------------------------------------------------#
#-- Makefile for Performer/src/lib/libpfdb directory              --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.2 $                                            --#
#--   $Date: 2000/11/21 23:36:28 $                                --#
#-------------------------------------------------------------------#

#-------------------------------------------------------------------#
#-- This makefile builds the programs in IRIS Performer's source  --#
#-- directories. The default make target makes "opt" versions of  --#
#-- each program and library. Other (actions) targets are:        --#
#--    build debugging versions: dbg (or debug)                   --#
#--    build optimized versions: opt (or optimize)                --#
#--    build dynamic shared object versions: dso                  --#
#--    remove all unneeded files after a build: clean             --#
#--    remove all machine generated files: clobber                --#
#-------------------------------------------------------------------#

default: dso

DIRS = \
	libpf3ds \
	libpfarcinfo \
	libpfbw \
	libpfbyu \
	libpfclosest \
	libpfct \
	libpfdem \
	libpfdoublerot \
	libpfdoublescale \
	libpfdoubletrans \
	libpfdted \
	libpfdwb \
	libpfdxf \
	libpfgds \
	libpfgfo \
	libpfim \
	libpfiv \
	libpfirtp \
	libpflodfix \
	libpflsa \
	libpfm \
	libpfnff \
	libpfobj \
	libpfpfb \
	libpfpfs \
	libpfphd \
	libpfpoly \
	libpfproc \
	libpfprojtex \
	libpfpts \
	libpfrot \
	libpfscale \
	libpfsgf \
	libpfspf \
	libpfsphere \
	libpfspherepatch \
	libpfspherepatch3 \
	libpfsponge \
	libpfstar \
	libpfstla \
	libpfsubstclip \
	libpfsv \
	libpftrans \
	libpftri \
	libpfunc \
	libpfbin \
	libpfcsb \
	libpfbpoly \
	libpfevt \
	libpfpegg \
	libpfpost \
	libpfsgo \
	libpfso \
	libpfstlb \
	libpfmedit \
	libpflsb \
	libpfvct



all debug dbg opt dso ddso iglopt igldbg igldso iglddso oglopt ogldbg ogldso oglddso clean clobber: 
	@for i in $(DIRS); \
	do \
		echo "\nmaking $$i ($@)"; \
		(cd $$i && ${MAKE} -k -f GNUmakefile $@) || exit 1; \
	done
