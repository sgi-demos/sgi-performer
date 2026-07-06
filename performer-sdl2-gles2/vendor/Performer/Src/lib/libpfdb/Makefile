#
#-------------------------------------------------------------------#
#-- Makefile for Src/lib/libpfdb directory                        --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.2 $                                           --#
#--   $Date: 2002/11/22 23:16:38 $                                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that are distributed in source form
SUBDIRS = \
	libpfpfb.dir \
	libpfarcinfo.dir \
	libpfbw.dir \
	libpfbyu.dir \
	libpfclosest.dir \
	libpfct.dir \
	libpfdem.dir \
	libpfdoublerot.dir \
	libpfdoublescale.dir \
	libpfdoubletrans.dir \
	libpfdwb.dir \
	libpfdxf.dir \
	libpfgds.dir \
	libpfgfo.dir \
	libpflodfix.dir \
	libpflsa.dir \
	libpfim.dir \
	libpfirtp.dir \
	libpfm.dir \
	libpfnff.dir \
	libpfobj.dir \
	libpfphd.dir \
	libpfpoly.dir \
	libpfproc.dir \
	libpfprojtex.dir \
	libpfpts.dir \
	libpfrot.dir \
	libpfscale.dir \
	libpfsgf.dir \
	libpfsphere.dir \
	libpfspherepatch.dir \
	libpfspherepatch3.dir \
	libpfspf.dir \
	libpfsponge.dir \
	libpfstar.dir \
	libpfstla.dir \
	libpfsubstclip.dir \
	libpfsv.dir \
	libpftri.dir \
	libpftrans.dir \
	libpfunc.dir \
	libpfcsb.dir \
	libpfpost.dir \
	libpfsgo.dir \
	libpfstlb.dir \
	libpflsb.dir \
	libpfevt.dir  \
	libpfbpoly.dir  \
	libpfbin.dir  \
	libpfh.dir \
	libpfdted.dir  \
	libpfvct.dir \
	libpfpegg.dir \
	libpfmedit.dir \
#	libpfso.dir \
#	libpfrgb.dir \
#	libpfptu.dir \
#	libpfflt1.dir1 \
#	libpfiv.dir \
#	libpfwrl.dir
#	libpfiv20.dir \
#	libpfegg.dir \
#	libpfflt13.dir \
#	libpfs1k.dir \
#	libpfstm.dir \
#	libpftmppfb.dir \

#--
#--	generic targets
#--

#-- default action is to make everything
default: $(SUBDIRS)


$(SUBDIRS):
	@echo making in $* subdirectory
	@cd $*
	@$(MAKE)
	@cd ..
