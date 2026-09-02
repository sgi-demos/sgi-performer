//
//
// Copyright 1995, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfTexLoad.h
//
// $Revision: 1.7 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_TEXLOAD_H__
#define __PF_TEXLOAD_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>


extern "C" {     // EXPORT to CAPI

/* ------------------ pfTexLoad Related Functions--------------------- */

/* PFTEXLOAD ATTRS */
#define	PFTLOAD_SOURCE			0
#define PFTLOAD_DEST			1

/* PFTEXLOAD VALUES */
#define PFTLOAD_SOURCE_LEVEL		10
#define	PFTLOAD_SOURCE_S		0
#define	PFTLOAD_SOURCE_T		1
#define	PFTLOAD_SOURCE_R		2
#define PFTLOAD_DEST_LEVEL		9
#define	PFTLOAD_DEST_S			3
#define	PFTLOAD_DEST_T			4
#define	PFTLOAD_DEST_R			5
#define	PFTLOAD_WIDTH			6
#define	PFTLOAD_HEIGHT			7
#define	PFTLOAD_DEPTH			8

/* PFTEXLOAD MODES */

/* TEXLOAD SOURCE TYPES */
#define PFTLOAD_SOURCE			0
#define	PFTLOAD_SOURCE_IMAGEARRAY	0
#define	PFTLOAD_SOURCE_IMAGETILE	1
#define PFTLOAD_SOURCE_TEXTURE		2
#define	PFTLOAD_SOURCE_VIDEO		3
#define	PFTLOAD_SOURCE_FRAMEBUFFER	4

/* TEXLOAD DEST TYPES */
#define PFTLOAD_DEST			1
#define PFTLOAD_DEST_TEXTURE		0

/* TEXLOAD SYNC TYPE */
#define PFTLOAD_SYNC			2
#define PFTLOAD_SYNC_TEXTURE		0
#define PFTLOAD_SYNC_FRAME		1
#define PFTLOAD_SYNC_IMMEDIATE		2
#define PFTLOAD_SYNC_DURING_IDLE	3
#define PFTLOAD_SYNC_OFF		4

/* TEXLOAD AUTOREFERENCE DLIST LOADS */
#define PFTLOAD_AUTOREF			3

/* TEXLOAD SYNC ON SOURCE DATA */
#define PFTLOAD_SYNC_SOURCE		4

/* PFTEXLOAD DIRTY BITS */
#define PFTLOAD_DIRTY_SOURCETYPE	0x1
#define PFTLOAD_DIRTY_DESTTYPE		0x2
#define PFTLOAD_DIRTY_SYNC		0x4
#define PFTLOAD_DIRTY_SOURCE		0x8
#define PFTLOAD_DIRTY_DEST		0x10
#define PFTLOAD_DIRTY_SIZE		0x20
#define PFTLOAD_DIRTY_FRAME		0x40

/* PFTEXLOAD STATUS */
#define PFTLOAD_STATUS_WAITING		0x0
#define PFTLOAD_STATUS_COMPLETE		0x1
#define PFTLOAD_STATUS_PARTIAL		0x2
} // extern "C" END of C include export


#define PFTEXLOAD ((pfTexLoad*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFTEXLOADBUFFER ((pfTexLoad*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfTexLoad : public pfObject
{
public:
    // CAPI:basename TLoad
    // constructors and destructors
    pfTexLoad();
    virtual ~pfTexLoad();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

/*----------------------------- pfTexLoad --------------------------------*/
public:
    // sets and gets
    // CAPI:basename TLoad
    void	setAttr(int _attr, void *_val);
    void*	getAttr(int _attr);
    void	setMode(int _mode, int _val);
    int		getMode(int _mode);
    void	setVal(int _which, float _val);
    float	getVal(int _which);

    void	setSrcOrg(int _s, int _t, int _r);
    void	getSrcOrg(int *_s, int *_t, int *_r);

    void	setDstOrg(int _s, int _t, int _r);
    void	getDstOrg(int *_s, int *_t, int *_r);

    void	setSrc(void *_src);
    void*	getSrc(void);

    void	setSrcLevel(int _lvl);
    int		getSrcLevel(void);

    void	setDst(void *_tex);
    void*	getDst(void);

    void	setDstLevel(int _lvl);
    int		getDstLevel(void);

	

    void	setSize(int _w, int _h, int _d);
    void	getSize(int *_w, int *_h, int *_d);

    void	setFrame(int _frameCount);
    int		getFrame(void);

    int		getPrevLoadedTexels(void);
    int		getDirty(void);
    int		setDirty(int _dirtmask);
    int		getStatus(void);

public:
    // CAPI:basename TLoad
    // CAPI:verb 
   int		apply();
    // CAPI:verb ApplyPartial
   int		apply(int _texels);

protected:
    int		srcS,srcT,srcR,srcLvl;		// Src Info - org and lvl
    int		dstS,dstT,dstR,dstLvl;		// Dst Info - org and lvl
    int		srcType,dstType;
    int		w,h,d;				// Load size
    volatile int prevLoadedTexels;		// Prev Partial Download
    volatile int completed;			// Load Finished ?
    int		dirty;				// Load Dirty
    int		frame;				// Frame Initiated
    int 	sync;				// Load Priority (mode)
    int		syncSrc;			// Sync on src data?
    int		autoRef;			// AutoReference when dlisting
    void	*src;				// Source 
    void	*dst;				// Destination Texture

private:
    static pfType *classType;
};

#endif // !__PF_TEXLOAD_H__











