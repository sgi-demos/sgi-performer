//
// Copyright 1999, 2000, Silicon Graphics, Inc.
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
// pfCompositor.h
//


#ifndef __PF_COMPOSITOR_H__
#define __PF_COMPOSITOR_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pr.h>
#include <Performer/prmath.h>
#include <Performer/pr/pfType.h>
#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>

#include <Performer/pf.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLists.h>
#include <Performer/pf/pfLoadBalance.h>
#include <Performer/pf/pfPipe.h>


extern "C" {

/* constants */

/* Compositor types */

#define		PFCOMP_TYPE	0
#define		PFCOMP_2x2	1
#define		PFCOMP_4x1HORIZ	2
#define		PFCOMP_4x1VERT	3
#define		PFCOMP_4xAA	4
#define		PFCOMP_2x1HORIZ	5
#define		PFCOMP_2x1VERT	6

/* flags */
#define		PFLOAD_BALANCE  5
#define		PFCOMP_CLIPPING 6

#define		PFCOMP_MAX_INPUTS	4

}

class pfPipe;
class pfCompositor;

#define		PF_CHANNEL	1
#define		PF_COMPOSITOR	2

#define		PF_PERSPECTIVE	1
#define		PF_ORTHO	2

typedef struct
{
	float            left, right, bottom, top, nearFOV, farFOV, type;
} _pfScreenArea;

typedef struct
{
	uint32_t          xorigin, yorigin, width, height;
} _pfPipeRect;

typedef struct
{
    int			what;
    int			pipe_id;
    pfPipe		*pipe;
    pfChannel		*channel;
    pfCompositor 	*compositor;
} _pfCompositorChild;

#define PFCOMPOSITOR ((pfCompositor*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFCOMPOSITORBUFFER ((pfCompositor*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfCompositor : public pfObject {

    //class type
    static pfType  *classType;

public:

	//CAPI:newargs
    pfCompositor();
    virtual ~pfCompositor();
    
    //// type checking functions
    static void init();
    static pfType* getClassType() {return classType;}

	//CAPI:virtual
    virtual int	getCompType(void) {return composition_type; }

	//
	// addChild() 
	// adds either a pipe (specified by pipe number) or pfCompositor to the heirarchy.
	//
    //CAPI:verb CompositorAddChild
    virtual int  addChild(int pipe_id);

    //CAPI:verb CompositorAddCompositor
    virtual int  addChild(pfCompositor *comp); 

    //CAPI:verb GetCompositorChild
    virtual pfCompositor *getChild(int index) 
	    {return ((_pfCompositorChild *) (children[index])) -> compositor; };

    //CAPI:noverb
    virtual pfChannel *getChan(int index) 
	    {return ((_pfCompositorChild *) (children[index])) -> channel; };

    virtual pfPipe *getPipe(int index) 
	    {return ((_pfCompositorChild *) (children[index])) -> pipe; };

    virtual int     getPipeId(int index) 
	    {return ((_pfCompositorChild *) (children[index])) -> pipe_id; };

	// 
	// setPerspective() and setOrtho() set the frusum that this pfCompositor()
	//               will cover.  The default is a perspective frustum with a 
	//               left, right, bottom, top of -1, 1, -1, 1 respectively.
	// 
    //CAPI:verb CompositorPerspective
    virtual void setPerspective(uint32_t index, float left, float right, float bottom, float top)
    { 
		frusta[index].left = left; 
		frusta[index].right = right; 
		frusta[index].bottom = bottom; 
		frusta[index].top = top;
		frusta[index].type = PF_PERSPECTIVE;
    }

    //CAPI:verb CompositorOrtho
    virtual void setOrtho(uint32_t index, float left, float right, float bottom, float top)
    { 
		frusta[index].left = left; 
		frusta[index].right = right; 
		frusta[index].bottom = bottom; 
		frusta[index].top = top;
		frusta[index].type = PF_ORTHO;
    }

    //CAPI:verb CompositorNearFar
    virtual void setNearFar(uint32_t index, float n, float f)
    {
		frusta[index].nearFOV = n;
		frusta[index].farFOV = f;
    }

    //CAPI:verb GetCompositorNearFar
    virtual void getNearFar(uint32_t index, float *n, float *f)
    {
		*n = frusta[index].nearFOV;
		*f = frusta[index].farFOV;
    }

    //CAPI:verb GetCompositorFrustum
    virtual void getFrustum(uint32_t index, float *left, float *right, float *bottom, float *top)
    { 
		*left   = frusta[index].left;
		*right  = frusta[index].right;
		*bottom = frusta[index].bottom;
		*top    = frusta[index].top;
    }

    //CAPI:verb GetCompositorFrustType
    virtual int getFrustType(void)
    {
		return projection;
    }
    
    //CAPI:verb CompositorViewport
    virtual void setViewport(float left, float right, float bottom, float top)
    { 
		viewport.left   = left; 
		viewport.right  = right; 
		viewport.bottom = bottom; 
		viewport.top    = top;
    }

    //CAPI:verb GetCompositorViewport
    virtual void getViewport(float *left, float *right, float *bottom, float *top)
    { 
		*left = viewport.left; 
		*right = viewport.right; 
		*bottom = viewport.bottom; 
		*top = viewport.top; 
    }

	//
	// setVal() and getVal() currently only accept PFLOAD_COEFF and pass the
	//          value onto the pfLoadBalance class.  This coefficient
	//          determines how quickly the balancer transition from the current
	//          state to the desired balanced state.
    //
    //CAPI:verb CompositorVal
    virtual void setVal(int what, float val);
    //CAPI:verb GetCompositorVal
    virtual float getVal(int what);

	// setMode() and getMode() take PFCOMP_TYPE and PFLOAD_BALANCE as arguements.
    //        PFCOMP_TYPE can be one of:  
    //               PFCOMP_2x2, 
    //               PFCOMP_4x1HORIZ, 
    //               PFCOMP_4x1VERT, 
    //               PFCOMP_4xAA
    //        And specifies the type of load balancing and subdivision the
    //        compositor should be using.
    // 
    //        PFLOAD_BALANCE can be either PF_ON or PF_OFF.  This specifies
    //        whether the pfCompositor should be performing automatic load
    //        balancing.  If it is PF_OFF, no load balancing computations are
    //        made and no glXHyperpipe calls are made unless specified
    //        otherwise via setSubdivision().
    //
    //CAPI:verb CompositorMode
    virtual void setMode(int what, int val);
    //CAPI:verb GetCompositorMode
    virtual int  getMode(int what);

	//
	// setLoadBalancer() and getLoadBalancer() allow setting the pfLoadBalance
    //       class to be used by the pfCompositor.  The setLoadBalancer()
    //       call must happen before pfConfig().
    //
    //CAPI:verb CompositorLoadBalancer
    virtual void setLoadBalancer(pfLoadBalance *balancer);
    //CAPI:verb GetCompositorLoadBalancer
    virtual pfLoadBalance *getLoadBalancer(void);

    //CAPI:verb CompositorChildViewport
	virtual void setChildViewport(uint32_t index, float left, float right, float bottom, float top);
    //CAPI:verb CompositorChildFrustum
	virtual void setChildFrustum(uint32_t index, float left, float right, float bottom, float top);
    //CAPI:verb CompositorChildSubdivision
	virtual void setChildSubdivision(uint32_t index, uint32_t xorigin, uint32_t yorigin, uint32_t width, uint32_t height);

    //CAPI:verb GetCompositorChildViewport
	virtual void getChildViewport(uint32_t index, float *left, float *right, float *bottom, float *top);
    //CAPI:verb GetCompositorChildFrustum
	virtual void getChildFrustum(uint32_t index, float *left, float *right, float *bottom, float *top);
    //CAPI:verb GetCompositorChildSubdivision
	virtual void getChildSubdivision(uint32_t index, uint32_t *xorigin, uint32_t *yorigin, uint32_t *width, uint32_t *height);

    //CAPI:verb CompositorMasterPipe
	virtual void setMasterPipe(pfPipe *master) { masterPipe = master; };
    //CAPI:verb GetCompositorMasterPipe
	virtual pfPipe *getMasterPipe(void) { return masterPipe; };

    //CAPI:verb CompositorReconfig
	virtual void reconfig(void) { channelsDirty = 1; };

    //CAPI:verb CompositorChannelClipped
	virtual void setChannelClipped(uint64_t chan_num, unsigned char clip_this_channel);
    //CAPI:verb GetCompositorChannelClipped
	virtual unsigned char getChannelClipped(uint64_t chan_num);

PFINTERNAL:
    virtual void programCompositor(void); // Called in DRAW process every frame.

    virtual void config(void);  // Called by pfConfig. Populate full info
				// about children in _pfCompositorChild
				// structs. Mark pfPipes as composited 
				// pipes.

    virtual void frame(void);  // Called inside APP process every frame.
    virtual void pf_initHyperpipes(void);

protected:
    virtual void pf_balance(void);
    virtual void pf_apply(void);
    virtual float pf_computeStress(void);
    virtual float pf_getStress(void) { return total_stress; }
    virtual void pf_populateMasterChannel(pfCompositor *which);
    virtual void pf_populateSlaveChannels(pfCompositor *which);

protected:

    int			composition_type;

    _pfVoidList		children;	// Children of this compositor. For a
					// single layer setup they can only
					// be pfChannels. For multi-layer
					// setups, they ca also be compositors.

    pfFlux		*hw_params;	// Parameters to be sent to the 
					// compositor hardware in the DRAW 
					// process.
    pfFlux		*compState;
	
    pfPipe		*masterPipe;	

    pfLoadBalance	*loadBalance;
    int             *load_balancing;
    float		*stress;
    float		total_stress;   

    float		total_work;	// Allocation of work for this 
					// compositor. Maximum (full screen) 
					// is 1.0

    _pfScreenArea	_viewport[PFCOMP_MAX_INPUTS];
    _pfScreenArea	_frustum[PFCOMP_MAX_INPUTS];
    _pfPipeRect     piperects[PFCOMP_MAX_INPUTS];
    _pfScreenArea	viewport;
    _pfScreenArea       frusta[8]; // XXX flynnt - don't hard-code this!
    _pfPipeRect      oldPipeRects[PFCOMP_MAX_INPUTS];
    int			chanWidth;
    int			chanHeight;
    int			hasBeenConfiged;
    int			hyperId;        // hyperpipe id
    int			projection;	// parallel or ortho ?
    int			channelsDirty;
    int			channelsInited;
    int			hyperpipesDirty;
    int			subdivisionDirty;
    int			clipping;
    uint32_t	chan_clip[8];
    void		*_pfCompositorExtraData;		
					// For keeping binary compatibility 
					// in minor releases.
};

#endif //__PF_COMPOSITOR_H__
