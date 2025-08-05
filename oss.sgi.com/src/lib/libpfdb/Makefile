#!smake -J 1
#-------------------------------------------------------------------#
#-- Makefile for Performer/src/lib/libpfdb directory              --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                            --#
#--   $Date: 2000/11/21 21:39:32 $                                --#
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
	libpfbin \
	libpfbpoly \
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
	libpfevt \
	libpfflt11 \
	libpfgds \
	libpfgfo \
	libpfim \
	libpfirtp \
	libpfiv \
	libpflodfix \
	libpflsa \
	libpflsb \
	libpfm \
	libpfmedit \
	libpfnff \
	libpfobj \
	libpfpegg \
	libpfpfb \
	libpfpfs \
	libpfphd \
	libpfpoly \
	libpfpost \
	libpfproc \
	libpfprojtex \
	libpfpts \
	libpfptu \
	libpfrot \
	libpfscale \
	libpfsgf \
	libpfsgo \
	libpfso \
	libpfspf \
	libpfsphere \
	libpfspherepatch \
	libpfspherepatch3 \
	libpfsponge \
	libpfstar \
	libpfstla \
	libpfstlb \
	libpfsubstclip \
	libpfsv \
	libpftrans \
	libpftri \
	libpfunc \
	libpfvct


all debug dbg opt dso ddso clean clobber: .MAKE
	@for i in $(DIRS); \
	do \
		echo "\nmaking $$i ($@)"; \
		cd $$i; ${MAKE} -k -f Makefile $@; cd ..; \
	done
