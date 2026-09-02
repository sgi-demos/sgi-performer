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
//  pfVideoChan.h - video channel
// 
// $Revision: 1.29 $
// $Date: 2002/11/07 03:59:17 $
// 
//

#ifndef __PF_VIDEO_CHANNEL_H__ 
#define __PF_VIDEO_CHANNEL_H__ 
#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfList.h>
#include <Performer/pr/pfCalligraphic.h>

//-----------------------------------------------------------


#define VC_MODE_DIRTY	0x0800
#define VC_DIRTY_MASK	0x0fff

/* flags */
#define _PFIS_BOUND		0x0001
#define VCHAN_MODE_SYNC_FIELD	0x0002
#define VCHAN_MODE_AUTO_APPLY	0x0004

extern pfVideoChannel *pfCurVideoChannel; /* current tracked video channel */
extern pfVideoChannel *pfCurRTVideoChannel; /* current video channel is real-time */



extern "C" {     // EXPORT to CAPI
/* ----------------------- pfVideoChannel Tokens ----------------------- */

/* pfVChanMode */
#define PFVCHAN_SYNC		0
#define PFVCHAN_AUTO_APPLY	1

#define PFVCHAN_SYNC_FIELD	0
#define PFVCHAN_SYNC_SWAP	1


} // extern "C" END of C include export

#define  _PF_CUR_VCHAN_AUTO_APPLY()

#define PFVIDEOCHANNEL ((pfVideoChannel*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFVIDEOCHANNELBUFFER ((pfVideoChannel*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfVideoChannel : public pfObject
{
    // CAPI:basename VChan
public:
    pfVideoChannel();
    virtual ~pfVideoChannel();

public:
    static void		init();
    static pfType*	getClassType() {return classType; }

public:
    virtual int compare (const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

public:
    void		setWSWindow(pfWSWindow _wsWin);
    pfWSWindow		getWSWindow() const { return wsWin; };
    // inherent video channel origin and size
    void		getOrigin(int *_xo, int *_yo);
    void		getSize(int *_xs, int *_ys);
    // variable output origin and size
    void		getScreenOutputOrigin(int *_xo, int *_yo);
    void		setOutputOrigin(int _xo, int _yo);
    void		getOutputOrigin(int *_xo, int *_yo);
    void		setOutputSize(int _xs, int _ys);
    void		getOutputSize(int *_xs, int *_ys) ;
    void		setAreaScale(float _s);
    float		getAreaScale() { return chanScale; }
    void		setScale(float _xs, float _ys);
    void		getScale(float *_xs, float *_ys) const {
	*_xs = xPixScale; *_ys = yPixScale;
    }
    void		setMinScale(float _xs, float _ys) ;
    void		getMinScale(float *_xs, float *_ys) const {
	*_xs = xMinPixScale; *_ys = yMinPixScale;
    }	
    void		setMaxScale(float _xs, float _ys) ;
    void		getMaxScale(float *_xs, float *_ys) const {
	*_xs = xMaxPixScale; *_ys = yMaxPixScale;
    }
    void		getMinDeltas(int *_dx, int *_dy) const {
	*_dx = xMinDelta; *_dy = yMinDelta;
    }
    void		setFullRect();
    
    void		setScreen(int _screen);
    int			getScreen() const { return screen; }
    void		setId(int _index);
    int			getId() { return vChanId; }
    void		setMode(int mode, int val);
    int	 		getMode(int mode);
    void		setCallig(pfCalligraphic *_calligraphic);
    pfCalligraphic *	getCallig() const { return calligraphic; }
    pfWSVideoChannelInfo getInfo();

    // CAPI:verb 
    void		select();
    void		apply();
    void		bind();
    void		unbind();
    // CAPI:verb IsVChanBound
    int			isBound() const { return ((flags & _PFIS_BOUND) != 0); } 
    // CAPI:verb IsVChanActive
    int			isActive();
PFINTERNAL: 

    void		pr_copyModified(pfVideoChannel *_src, uint _modMask);
    void		pr_setWSWindow(pfWSConnection _dsp, pfWSWindow _wsWin);
    int			pr_isDirty(void) const { return dirty & VC_DIRTY_MASK; }
    int			pr_isSyncOnField(void) const { return (flags & VCHAN_MODE_SYNC_FIELD); }
    int			pr_isAutoApply(void) const {
	return ((flags & VCHAN_MODE_AUTO_APPLY));
    }
    int			pr_needsAutoApply(void) const {
	return (pr_isAutoApply() && pr_isDirty());
    }

private:
    int			pr_updateChanInfo();
    
PFINTERNAL:  // XXX should make protected???
    ushort		dirty;
    ushort		modified;
    ushort		flags;
    short		screen;
    int			vChanId;
    int			displayWidth, displayHeight;
    pfWSWindow		wsWin;
    int			xOriginRes, yOriginRes;
    int			xMinDelta, yMinDelta;
    int			xOrg, yOrg, xSize, ySize;
    int			xOutOrg, yOutOrg, xOutSize, yOutSize;
    float		xPixScale, yPixScale;
    float		xMinPixScale, yMinPixScale;
    float		xMaxPixScale, yMaxPixScale;
    float		chanScale;
    float		aspect;
    pfWSVideoChannelInfo vChanInfo;
    pfCalligraphic	*calligraphic;
    
    
private:
    static pfType *classType;
public:
    void                *_pfVideoChannelExtraData;
};

#endif // !__PF_VIDEO_CHANNEL_H__
