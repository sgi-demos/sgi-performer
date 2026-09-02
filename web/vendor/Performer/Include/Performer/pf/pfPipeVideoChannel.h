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
//  pfPipeVideoChannel.h - pipe video channel
// 
// $Revision: 1.32 $
// $Date: 2002/11/07 03:59:17 $
// 
//

#ifndef __PF_PIPE_VIDEO_CHANNEL_H__ 
#define __PF_PIPE_VIDEO_CHANNEL_H__ 

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfUpdatable.h>
#include <Performer/pf/pfLists.h>
#include <Performer/pf/pfUpdate.h>
#include <Performer/pf/pfPipe.h>
#include <Performer/pf/pfPipeWindow.h>
#include <Performer/pr/pfVideoChannel.h>
#include <Performer/pf.h>

//-----------------------------------------------------------



extern "C" {     // EXPORT to CAPI
/* ----------------------- pfVideoChannel Tokens ----------------------- */

/* pfPVChanDVRMode */
#define PFPVC_DVR_OFF		0x0
#define PFPVC_DVR_MANUAL	0x1
#define PFPVC_DVR_AUTO		0x2
#define PFPVC_DVR_MASK		0x3
#define PFPVC_DVR_NOVR		0x4

} // extern "C" END of C include export

struct pfPipeVChanUpBuffer : public pfStruct
{
    PF_ULOCK_T	lock;
    int frameStamp;
    int dirty;
    float	curLoad;	// load from current frame
    float	drawTime;	// secs spent drawing
    float	stressLevel;	// stress from current frame
    pfVideoChannel *vChan;
    
    ~pfPipeVChanUpBuffer() {
	usfreelock(lock, pfGetSemaArena());
	pfMemory::checkDelete(vChan);
    }
};

class pfPipeVideoChannel;

#define PFPIPEVIDEOCHANNEL ((pfPipeVideoChannel*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFPIPEVIDEOCHANNELBUFFER ((pfPipeVideoChannel*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfPipeVideoChannel : public pfUpdatable
{
public:

    void setMode(int _mode, int _val)  {
        PFPIPEVIDEOCHANNEL->nb_setMode(_mode, _val);
    }

    int getMode(int _mode)  {
        return PFPIPEVIDEOCHANNEL->nb_getMode(_mode);
    }

    void setWSWindow(pfWSWindow _wsWin)  {
        PFPIPEVIDEOCHANNEL->nb_setWSWindow(_wsWin);
    }

    pfWSWindow getWSWindow() const  {
        return PFPIPEVIDEOCHANNEL->nb_getWSWindow();
    }

    void setCallig(pfCalligraphic *_ca)  {
        PFPIPEVIDEOCHANNEL->nb_setCallig(_ca);
    }

    pfCalligraphic* getCallig() const  {
        return PFPIPEVIDEOCHANNEL->nb_getCallig();
    }

    void getOrigin(int *_xo, int *_yo)  {
        PFPIPEVIDEOCHANNEL->nb_getOrigin(_xo, _yo);
    }

    void getSize(int *_xs, int *_ys)  {
        PFPIPEVIDEOCHANNEL->nb_getSize(_xs, _ys);
    }

    void getMinDeltas(int *_dx, int *_dy) const  {
        PFPIPEVIDEOCHANNEL->nb_getMinDeltas(_dx, _dy);
    }

    void getScreenOutputOrigin(int *_xo, int *_yo)  {
        PFPIPEVIDEOCHANNEL->nb_getScreenOutputOrigin(_xo, _yo);
    }

    void setOutputOrigin(int _xo, int _yo)  {
        PFPIPEVIDEOCHANNEL->nb_setOutputOrigin(_xo, _yo);
    }

    void getOutputOrigin(int *_xo, int *_yo)  {
        PFPIPEVIDEOCHANNEL->nb_getOutputOrigin(_xo, _yo);
    }

    void setOutputSize(int _xs, int _ys)  {
        PFPIPEVIDEOCHANNEL->nb_setOutputSize(_xs, _ys);
    }

    void getOutputSize(int *_xs, int *_ys)  {
        PFPIPEVIDEOCHANNEL->nb_getOutputSize(_xs, _ys);
    }

    void setAreaScale(float _s)  {
        PFPIPEVIDEOCHANNEL->nb_setAreaScale(_s);
    }

    float getAreaScale()  {
        return PFPIPEVIDEOCHANNEL->nb_getAreaScale();
    }

    void setMinScale(float _xs, float _ys)  {
        PFPIPEVIDEOCHANNEL->nb_setMinScale(_xs, _ys);
    }

    void getMinScale(float *_xs, float *_ys) const  {
        PFPIPEVIDEOCHANNEL->nb_getMinScale(_xs, _ys);
    }

    void setMaxScale(float _xs, float _ys)  {
        PFPIPEVIDEOCHANNEL->nb_setMaxScale(_xs, _ys);
    }

    void getMaxScale(float *_xs, float *_ys) const  {
        PFPIPEVIDEOCHANNEL->nb_getMaxScale(_xs, _ys);
    }

    void setScale(float _xs, float _ys)  {
        PFPIPEVIDEOCHANNEL->nb_setScale(_xs, _ys);
    }

    void getScale(float *_xs, float *_ys) const  {
        PFPIPEVIDEOCHANNEL->nb_getScale(_xs, _ys);
    }

    void setFullRect()  {
        PFPIPEVIDEOCHANNEL->nb_setFullRect();
    }

    int getScreen() const  {
        return PFPIPEVIDEOCHANNEL->nb_getScreen();
    }

    void setId(int _index)  {
        PFPIPEVIDEOCHANNEL->nb_setId(_index);
    }

    int getId()  {
        return PFPIPEVIDEOCHANNEL->nb_getId();
    }

    pfWSVideoChannelInfo getInfo()  {
        return PFPIPEVIDEOCHANNEL->nb_getInfo();
    }

    void setDVRMode(int _mode)  {
        PFPIPEVIDEOCHANNEL->nb_setDVRMode(_mode);
    }

    int getDVRMode() const  {
        return PFPIPEVIDEOCHANNEL->nb_getDVRMode();
    }

    void setStressFilter(float frac, float low, float high, float ps, float s, float max)  {
        PFPIPEVIDEOCHANNEL->nb_setStressFilter(frac, low, high, ps, s, max);
    }

    void getStressFilter(float *frac, float *low, float *high, float *ps, float *s, float *max) const  {
        PFPIPEVIDEOCHANNEL->nb_getStressFilter(frac, low, high, ps, s, max);
    }

    void setStress(float stress)  {
        PFPIPEVIDEOCHANNEL->nb_setStress(stress);
    }

    float getStress() const  {
        return PFPIPEVIDEOCHANNEL->nb_getStress();
    }

    float getLoad() const  {
        return PFPIPEVIDEOCHANNEL->nb_getLoad();
    }

    void setMinIncScale(float _xs, float _ys)  {
        PFPIPEVIDEOCHANNEL->nb_setMinIncScale(_xs, _ys);
    }

    void getMinIncScale(float *_xs, float *_ys) const  {
        PFPIPEVIDEOCHANNEL->nb_getMinIncScale(_xs, _ys);
    }

    void setMaxIncScale(float _xs, float _ys)  {
        PFPIPEVIDEOCHANNEL->nb_setMaxIncScale(_xs, _ys);
    }

    void getMaxIncScale(float *_xs, float *_ys) const  {
        PFPIPEVIDEOCHANNEL->nb_getMaxIncScale(_xs, _ys);
    }

    void setMinDecScale(float _xs, float _ys)  {
        PFPIPEVIDEOCHANNEL->nb_setMinDecScale(_xs, _ys);
    }

    void getMinDecScale(float *_xs, float *_ys) const  {
        PFPIPEVIDEOCHANNEL->nb_getMinDecScale(_xs, _ys);
    }

    void setMaxDecScale(float _xs, float _ys)  {
        PFPIPEVIDEOCHANNEL->nb_setMaxDecScale(_xs, _ys);
    }

    void getMaxDecScale(float *_xs, float *_ys) const  {
        PFPIPEVIDEOCHANNEL->nb_getMaxDecScale(_xs, _ys);
    }

    void select()  {
        PFPIPEVIDEOCHANNEL->nb_select();
    }

    void apply()  {
        PFPIPEVIDEOCHANNEL->nb_apply();
    }

    void bind()  {
        PFPIPEVIDEOCHANNEL->nb_bind();
    }

    void unbind()  {
        PFPIPEVIDEOCHANNEL->nb_unbind();
    }

    int isBound() const  {
        return PFPIPEVIDEOCHANNEL->nb_isBound();
    }

    int isActive()  {
        return PFPIPEVIDEOCHANNEL->nb_isActive();
    }

    pfPipe* getPipe()  {
        return PFPIPEVIDEOCHANNEL->nb_getPipe();
    }

    pfPipeWindow* getPWin()  {
        return PFPIPEVIDEOCHANNEL->nb_getPWin();
    }

    int getPWinIndex()  {
        return PFPIPEVIDEOCHANNEL->nb_getPWinIndex();
    }
public:
    // CAPI:basename PVChan
    //CAPI:updatable
    //CAPI:newargs
    pfPipeVideoChannel(pfPipe *p);
    virtual ~pfPipeVideoChannel();

protected:     
    // Create a pipe video channel
    pfPipeVideoChannel(pfBuffer *buf, pfPipe *p);
    
    // Clone a new pipe in buffer.
    pfPipeVideoChannel(const pfPipeVideoChannel* prev, pfBuffer *buf);

public: // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions
    virtual int		 print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

PFINTERNAL:             // pfUpdatable virtual functions
    // Update pfPipeWindow.
    virtual void	pf_applyUpdate(const pfUpdatable *prev, int  updateId);
    // Create clone of pfPipeWindow in buffer buf
    virtual pfUpdatable* pf_bufferClone(pfBuffer *buf);
    virtual void	pf_copyToDraw(const pfUpdatable *_pVChan); 
    virtual void    	pf_updateBufferList(pfBuffer *buf, int addRemove) {
        buf->pf_updateList(this, addRemove);
    }

public: // pfVideoChannel API
    void		nb_setMode(int _mode, int _val);
    int			nb_getMode(int _mode);
    void		nb_setWSWindow(pfWSWindow _wsWin);
    pfWSWindow		nb_getWSWindow() const { return vChan->getWSWindow(); }
    void		nb_setCallig(pfCalligraphic *_ca);
    pfCalligraphic	*nb_getCallig() const { return vChan->getCallig(); }
    // inherent video channel origin and size
    void		nb_getOrigin(int *_xo, int *_yo);
    void		nb_getSize(int *_xs, int *_ys);
    void		nb_getMinDeltas(int *_dx, int *_dy) const { 
	vChan->getMinDeltas(_dx, _dy);
    }
    // variable output origin and size
    void		nb_getScreenOutputOrigin(int *_xo, int *_yo) {
	vChan->getScreenOutputOrigin(_xo, _yo);
    }
    void		nb_setOutputOrigin(int _xo, int _yo);
    void		nb_getOutputOrigin(int *_xo, int *_yo);
    void		nb_setOutputSize(int _xs, int _ys);
    void		nb_getOutputSize(int *_xs, int *_ys);
    void		nb_setAreaScale(float _s);
    float		nb_getAreaScale() { return vChan->getAreaScale(); }
    void		nb_setMinScale(float _xs, float _ys) ;
    void		nb_getMinScale(float *_xs, float *_ys) const {
	vChan->getMinScale(_xs, _ys);
    }	
    void		nb_setMaxScale(float _xs, float _ys) ;
    void		nb_getMaxScale(float *_xs, float *_ys) const {
	vChan->getMaxScale(_xs, _ys);
    }	
    void		nb_setScale(float _xs, float _ys) ;
    void		nb_getScale(float *_xs, float *_ys) const {
	vChan->getScale(_xs, _ys);
    }
    void		nb_setFullRect();
    
    int			nb_getScreen() const {  return vChan->getScreen();  }
    void		nb_setId(int _index);
    int			nb_getId() { return vChan->getId(); }
    pfWSVideoChannelInfo nb_getInfo() { return vChan->getInfo(); }
    
    void	   nb_setDVRMode(int _mode);
    int	   nb_getDVRMode() const {
	return drMode;
    }
    
    void	   nb_setStressFilter(float frac, float low, float high, float ps, float s, float max);
    void	   nb_getStressFilter(float *frac, float *low, float *high, float *ps, float *s, float *max) const {
    	*low = lowLoad; *high = highLoad; *max = maxStress; *frac = frameFrac;
	*s = stressScale;
	*ps = pipeLoadScale;
    }
    
    void	   nb_setStress(float stress);
    float	   nb_getStress() const {
	return stressLevel;
    }
    float	   nb_getLoad() const {
	return curLoad;
    }
    // control the minimum/maximum amounts that a scale can do at any one step
    // separate controls for incremnting and decrementing the scale value
    void		nb_setMinIncScale(float _xs, float _ys) ;
    void		nb_getMinIncScale(float *_xs, float *_ys) const {
	*_xs = xMinIncPixScale;
	*_ys = yMinIncPixScale;
    }
    void		nb_setMaxIncScale(float _xs, float _ys) ;
    void		nb_getMaxIncScale(float *_xs, float *_ys) const {
	*_xs = xMaxIncPixScale;
	*_ys = yMaxIncPixScale;
    }
    void		nb_setMinDecScale(float _xs, float _ys) ;
    void		nb_getMinDecScale(float *_xs, float *_ys) const {
	*_xs = xMinDecPixScale;
	*_ys = yMinDecPixScale;
    }
    void		nb_setMaxDecScale(float _xs, float _ys) ;
    void		nb_getMaxDecScale(float *_xs, float *_ys) const {
	*_xs = xMaxDecPixScale;
	*_ys = yMaxDecPixScale;
    }
    
    // CAPI:verb 
    void		nb_select();
    void		nb_apply();
    void		nb_bind();
    void		nb_unbind();
    // CAPI:verb IsPVChanBound
    int			nb_isBound() const { return vChan->isBound(); } 
    // CAPI:verb IsPVChanActive
    int			nb_isActive();

    // Class specific sets/gets
    //CAPI:noverb
    //CAPI:basename PVChan
    
    pfPipe*	nb_getPipe();
    
    // get PWin
    pfPipeWindow*   nb_getPWin();
    int		    nb_getPWinIndex();
    

public:
    // CAPI:verb 
    
  
PFINTERNAL:
   
    // pfPipe which renders into this window
    _pfPipeRef	    	parentPipe;

    // pfPipeWindow that manages/is bound to this video channel
    _pfPipeWindowRef	pipeWin;
    
   // The pfVideoChannel holding all of the data
   pfVideoChannel	*vChan;
    
    int			frameStamp;
    uint		drMode;
    int			dirty;
    int			pipeNum;

    float		xMinIncPixScale, yMinIncPixScale;
    float		xMaxIncPixScale, yMaxIncPixScale;
    float		xMinDecPixScale, yMinDecPixScale;
    float		xMaxDecPixScale, yMaxDecPixScale;
    
    //////////////////// Load/Stress ////////////////////////
    float	curLoad;	// Most recent frame's load value.
    float	prevLoad;	// Previous frame's load value.
    float	drawTime;	// secs spent drawing
    float	lowLoad;   	// Load below which stress is decreased.
    float	highLoad;   	// Load above which stress is increased.
    float	maxStress;	// Maximum value stress may reach.
    float	frameFrac;	// Fraction of frame channel should occupy.
    float	stressScale;    // Stress multiplier.
    float	stressLevel; 	// Stress value computed from load.
    int 	stressFlag; 	// 1 = stress is explicitly; don't use filter
    float	pipeLoadScale;	// scale pipe load to consider for stress
   // Buffer used to propagate data back to head of pipe
    pfPipeVChanUpBuffer	*upstreamBuffer;

    // book-keep
    int		prevMod;	

private:
    static pfType *classType;
public:
    void *_pfPVChannelExtraData;
};

PF_UPDATABLE_REF(pfPipeVideoChannel, _pfPipeVideoChannelRef);

#endif // !__PF_PIPE_VIDEO_CHANNEL_H__
