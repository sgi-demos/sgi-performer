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
// pfPipe.h
//
// $Revision: 1.150 $
// $Date: 2002/09/20 09:08:10 $
//
//
#ifndef __PF_PIPE_H__
#define __PF_PIPE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfUpdatable.h>
#include <Performer/pf/pfUpdate.h>
#include <Performer/pr/pfVideoChannel.h>
#include <Performer/pr/pfDispList.h>
#include <Performer/pf.h>


class 	pfChannel;
class	pfMPClipTexture;
class 	pfPipeWindow;
class   pfCompositor;


//#define EMULATE_HYPER
struct pfPipeUpBuffer : public pfStruct
{
    PF_ULOCK_T	lock;

    double	afterSwap;
    int 	screen;
    int         server;
    int		xSize, ySize;
    float	curLoad;
    char	displayString[PF_MAXSTRING];
};

extern "C" {     // EXPORT to CAPI

/*----------------- pfPipe Functions ------------------*/

extern DLLEXPORT pfPipe*	pfGetPipe(int _pipeNum);
extern DLLEXPORT int	pfInitPipe(pfPipe *_pipe, pfPipeFuncType _configFunc);

} // extern "C" END of C include export

#define PFPIPE ((pfPipe*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFPIPEBUFFER ((pfPipe*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfPipe : public pfUpdatable 
{
public:

    void setSwapFunc(pfPipeSwapFuncType func)  {
        PFPIPE->nb_setSwapFunc(func);
    }

    pfPipeSwapFuncType getSwapFunc() const  {
        return PFPIPE->nb_getSwapFunc();
    }

    float getLoad() const  {
        return PFPIPE->nb_getLoad();
    }

    void getSize(int  *xs, int  *ys) const  {
        PFPIPE->nb_getSize(xs, ys);
    }

    void setScreen(int scr)  {
        PFPIPE->nb_setScreen(scr);
    }

    int getScreen() const  {
        return PFPIPE->nb_getScreen();
    }

    void setServer(int dis)  {
        PFPIPE->nb_setServer(dis);
    }

    int getServer() const  {
        return PFPIPE->nb_getServer();
    }

    void setWSConnectionName(const char *name)  {
        PFPIPE->nb_setWSConnectionName(name);
    }

    const char* getWSConnectionName() const  {
        return PFPIPE->nb_getWSConnectionName();
    }

    pfMPClipTexture* getMPClipTexture(int  i) const  {
        return PFPIPE->nb_getMPClipTexture(i);
    }

    int getNumMPClipTextures() const  {
        return PFPIPE->nb_getNumMPClipTextures();
    }

    pfChannel* getChan(int  i) const  {
        return PFPIPE->nb_getChan(i);
    }

    int getNumChans() const  {
        return PFPIPE->nb_getNumChans();
    }

    pfPipeWindow* getPWin(int  i) const  {
        return PFPIPE->nb_getPWin(i);
    }

    int getNumPWins() const  {
        return PFPIPE->nb_getNumPWins();
    }

    void setIncrementalStateChanNum(int _num)  {
        PFPIPE->nb_setIncrementalStateChanNum(_num);
    }

    int getIncrementalStateChanNum() const  {
        return PFPIPE->nb_getIncrementalStateChanNum();
    }

    int getHyperId() const  {
        return PFPIPE->nb_getHyperId();
    }

    void bindPVChans()  {
        PFPIPE->nb_bindPVChans();
    }

    void unbindPVChans()  {
        PFPIPE->nb_unbindPVChans();
    }

    int movePWin(int  where, pfPipeWindow *_pw)  {
        return PFPIPE->nb_movePWin(where, _pw);
    }

    void addMPClipTexture(pfMPClipTexture *clip)  {
        PFPIPE->nb_addMPClipTexture(clip);
    }

    int removeMPClipTexture(pfMPClipTexture *clip)  {
        return PFPIPE->nb_removeMPClipTexture(clip);
    }

    void setTotalTexLoadTime(float _totalTexLoadTime)  {
        PFPIPE->nb_setTotalTexLoadTime(_totalTexLoadTime);
    }

    float getTotalTexLoadTime() const  {
        return PFPIPE->nb_getTotalTexLoadTime();
    }

    void setSize(int xs, int ys)  {
        PFPIPE->nb_setSize(xs, ys);
    }
public:
    //CAPI:updatable
    virtual ~pfPipe();

protected:
    friend void doCullForkedDraw(void);
    friend DLLEXPORT int pfConfig(void);
    friend void appendIncDList(int *drawn, pfPipe *p1, pfPipe *p2);
    friend void mpDraw(void);
    friend void doDraw(pfChannel *drawChan, pfPipe *cPipe, int *itime);
    friend void _pfCalcVideoRate(void);
    
    // Create a pipe 
    pfPipe(pfBuffer *buf, usptr_t *arena);
    
    // Clone a new pipe in buffer.
    pfPipe(const pfPipe* pipe, pfBuffer *buf);

public:
    // per class functions;
    static void	   init(void);
    static pfType* getClassType() { return classType; }
    
public:			// pfMemory virtual functions

PFINTERNAL:		// pfUpdatable virtual functions
    // Update pipe.
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int  updateId);

    // Create clone of pipe in buffer buf
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);
    virtual void	    pf_copyToDraw(const pfUpdatable *_srcBuf);

    virtual void	    pf_updateBufferList(pfBuffer *buf, int addRemove) {
        buf->pf_updateList(this, addRemove);
    }
    
PFINTERNAL:		// specific sets and gets
    void	    	nb_setSwapFunc(pfPipeSwapFuncType func);
    pfPipeSwapFuncType	nb_getSwapFunc(void) const {
	return swapFunc;
    }
    
   
    float	   nb_getLoad(void) const {
	return curLoad;
    }
    
    // Get pipe size
    void	    nb_getSize(int  *xs, int  *ys) const;
    void	    nb_setScreen(int scr);
    int 	    nb_getScreen(void) const;
    
    void	    nb_setServer(int dis);
    int 	    nb_getServer(void) const;
    int	            pf_getDisplayID(void) const;

    void	    nb_setWSConnectionName(const char *name);
    const char	*   nb_getWSConnectionName(void) const {
	return displayString;
    }
    
    pfMPClipTexture* nb_getMPClipTexture(int  i) const {
	return clipList[i];
    }
    int 	    nb_getNumMPClipTextures() const { 
	return clipList.getNum(); 
    }

    pfChannel*      nb_getChan(int  i) const {
	return channelList[i];
    }
    int 	    nb_getNumChans() const { 
	return channelList.getNum(); 
    }

    pfPipeWindow*   nb_getPWin(int  i) const {
	return pipeWinList[i];
    }
    int 	    nb_getNumPWins(void) const { 
	return pipeWinList.getNum(); 
    }

    // Channel on which to apply clipmaps/icaches
    void	    nb_setIncrementalStateChanNum(int _num);
    int		    nb_getIncrementalStateChanNum(void) const {
	return incStateChannelNum;
    }

    int 	    nb_getHyperId(void) const {
	return hyperId;
    }
    
    /* pfPipeVideoChannel Management */
    //CAPI:verb BindPipeVChans
    void	    nb_bindPVChans();
    //CAPI:verb UnbindPipePVChans
    void	    nb_unbindPVChans();

    //CAPI:verb MovePWin
    int		    nb_movePWin(int  where, pfPipeWindow *_pw);
    
    // Clip Texture access functions
    //CAPI:verb AddMPClipTexture
    void	    nb_addMPClipTexture(pfMPClipTexture *clip);
    int		    nb_removeMPClipTexture(pfMPClipTexture *clip);

    //CAPI:noverb
    void	    nb_setTotalTexLoadTime(float _totalTexLoadTime);
    float	    nb_getTotalTexLoadTime(void) const {
	return totalTexLoadTime;
    }

PFINTERNAL:
    //CAPI:private
    void	    nb_setSize(int xs, int ys) {
	xSize = xs; ySize = ys;
    }

private:
    // Specifies which hardware pipe(X screen) this structure uses.
    int 		screen;
    // Specifies which X server (X display) this structure uses.
    int                 server;
    // App connection to X display managing the hardware pipe
    pfWSConnection	appWSConnection; 
    // name of X server managing the hardware pipe
    char		displayString[PF_MAXSTRING];

    // flag saying we have requested events on this display
    int			haveXEvents;
    
    // Used to synchronize video clocks across multiple pipes
    int			vclockInited;	
    double		vclock;
    
    // List of pfPipeWindows using this pipe.
    _pfPipeWindowList	pipeWinList;
    
    // List of channels using this pipe.
    _pfChannelList	channelList;
    
    // mswapbuffers() token
    int			swapMode;
    
    // Position of this pipe in hyperpipe group.
    int 		hyperId;
        
    // List of pipes which in hyperpipe group. Hyperpipes can NOT be
    // locked together.
    _pfPipeList	    	hyperList;
    
    // Token for determining which hyperpipe's turn it is
    struct _pfHyperInfo *hyperInfo;

    int swapParams; // for holding hyper swap tokens
    
#ifdef EMULATE_HYPER
    int 		*hyperDrawTurn;
    PF_USEMA_T		**hyperDrawSema;
#endif

    // If we are on a compositor, which one is it ?
    pfCompositor 	*compositor;
    
    // List of pipes which must swapbuffers the same time as this pipe.
    // Hyperpipes can be locked together (all the first pipes in a network, 
    // all the second pipes in a network, etc)..
    _pfPipePList    	lockedList;

    // List of pipes that must synchronize when calling glXBindHyperpipeSGIX
    _pfPipePList    	hyperpipeBindLockList;
    
    // TRUE if pipe should synchronize with others through GANGDRAW
    int 		gangDraw;
    
    // Semaphores used by pipes locked to this one.
    PF_USEMA_T		*cullDoneSema, *drawDoneSema;
    
    // Display attributes
    int 		xSize, ySize;    
    
    // Desired frame rate in frames/sec. Will be < system frame rate for
    // hyperpipes.
    float		frameRate;
    
    // Number of video retraces per frame.  Will be > system field rate for
    // hyperpipes.
    int 		fieldsPerFrame;
    
    // VClock offset from frame boundary. Only non-zero for hyperpipes.
    int 		fieldOffset;
    
    // Locked, floating, or free run phase.
    int 		phase;
    
    // swapbuffers callback
    pfPipeSwapFuncType	swapFunc;
    
    // time return from swapbuffers call
    double		afterSwap;
    
    // Frame this pfPipe is working on.
    int 		frameCount;
    
    // Time current frame started draw process
    double		startDraw;
    
    // DR and Load 
    int		dvrMode;	// max dvr mode of PipeVideoChannels
    int		dvrSync;	// pipe has a vchan with field sync DVR
    float	curLoad;	// load from current frame

    
    // Buffer used to propagate data back to head of pipe
    pfPipeUpBuffer	*upstreamBuffer;

    _pfMPClipTextureList 	clipList;

    float			totalTexLoadTime;     // time to divide up

    int				hasIncState;

    pfDispList			*incStateDList;

    int				incStateChannelNum;

    PF_ULOCK_T                  incStateDListLock;
private:
    static pfType *classType;
public:
    void *_pfPipeExtraData;
};

PF_UPDATABLE_REF(pfPipe, _pfPipeRef);

#endif // !__PF_PIPE_H__
