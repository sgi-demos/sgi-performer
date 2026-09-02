//
//
// Copyright 1997, Silicon Graphics, Inc.
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
// pfFlux.h
//
// $Revision: 1.27 $
// $Date: 2002/03/14 21:11:10 $
//

#ifndef __PF_FLUX_H__
#define __PF_FLUX_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfMemory.h>
#include <Performer/pr/pfList.h>
#include <Performer/pr/pfState.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfEngine.h>


#define PFFLUXMEMORY ((pfFluxMemory*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFFLUXMEMORYBUFFER ((pfFluxMemory*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfFluxMemory : public pfMemory
{
public:
    // Constructors, destructors
    //CAPI:private
    pfFluxMemory(pfFlux *flux);
    virtual ~pfFluxMemory();

private:
    static pfType *classType;

public:
    // virtual functions
    // CAPI:private
    void	*getData() const {
	return (void*) (((char *)this)+sizeof(pfFluxMemory));
    }

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }
    // CAPI:verb GetFluxMemory
    static pfFluxMemory* getFluxMemory(void *data) {
	return (pfFluxMemory*) (((char *)data)-sizeof(pfFluxMemory));
    }
    // CAPI:noverb

public:
    // gets and sets
    pfFlux *		getFlux()	 { return parent; }

private:
    pfFlux		*parent;
#ifndef N64
    void		*pad;		// needed to pad to double alignment
#endif
};


typedef struct {
    void *data;
    int frame;
    int flags;
} flux_buf_t;

typedef struct {
    pfEngine *eng;
    short use_count;
} c_eng_t;




// Exported Tokens
extern "C" {     // EXPORT to CAPI

/*
 *  Special config buffer number
 */
#define PFFLUX_DEFAULT_NUM_BUFFERS	0x01000000
#define PFFLUX_MAX_BUFFERS		0x0000ffff

/*
 *  Tokens for pfFlux::getNumBuffers
 */
#define PFFLUX_BUFFERS_SPECIFIED	1
#define PFFLUX_BUFFERS_GENERATED	2

/*
 *  modes
 */
#define PFFLUX_PUSH		0
#define PFFLUX_ON_DEMAND	1
#define PFFLUX_COPY_LAST_DATA	2
#define PFFLUX_WRITE_ONCE	3

/*
 *  masks
 */
#define PFFLUX_BASIC_MASK	0x00000001

}


#define PFFLUX ((pfFlux*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFFLUXBUFFER ((pfFlux*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfFlux : public pfObject
{
public:
    // Constructors, destructors
    pfFlux(size_t nbytes, int _numBuffers);
    // CAPI:verb NewFluxInitFunc
    // CAPI:newargs void *arena
    pfFlux(pfFluxInitFuncType _initFunc, int _numBuffers);
public:
    virtual ~pfFlux();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int		copy(const pfMemory *_src);
    virtual int		compare(const pfMemory *_mem) const;
    virtual int		print(uint _travMode, uint _verbose,
			      char *_prefix, FILE *file);

    virtual void*	getData() const {
	return buffers[0].data;
    }


public:
    // sets and gets
    void*			getCurData();
    void*			getWritableData();
    void*			getBufferData(int bufferid) {
	return buffers[bufferid].data;
    }
    void			setMode(int _mode, int _val);
    int				getMode(int _mode) const;
    void			setMask(uint _mask);
    uint			getMask(void) const;
    void			setSyncGroup(uint _syncGroup);
    uint			getSyncGroup(void) const;
    size_t			getDataSize(void) const;
    int				getNumBuffers(int _type) const;
    pfFluxInitFuncType		getInitFunc(void) const { return initFunc; }

    pfEngine *		getSrcEngine(int _index) const;
    int			getNumSrcEngines(void) const;
    pfEngine *		getClientEngine(int _index) const;
    int			getNumClientEngines(void) const;

    void 		setUserCopyFunc(pfFluxUserCopyFuncType func) { userCopyFunc = func; }
    pfFluxUserCopyFuncType getUserCopyFunc(void) { return userCopyFunc; }
			


public:
    // other functions
    void		writeComplete(void);
    void		srcChanged(void);
    void		initData(void *_data);
    void		callDataFunc(pfFluxDataFuncType _func, void *_funcData);
    void		evaluate(int _mask);
    // CAPI:verb FluxEvaluateEye
    void		evaluate(int _mask, pfVec3 _eye_pos);

public:
    static int		setDefaultNumBuffers(int _numBuffers);
    static int		getDefaultNumBuffers(void);
    // CAPI:verb GetFlux
    static pfFlux *	getFlux(void *_data);
    // CAPI:verb GetFluxCurDataFromData
    static void*	getCurData(void *_data)
    {
	return pfFlux::getFlux(_data)->getCurData();
    }
    // CAPI:verb GetFluxWritableDataFromData
    static void*	getWritableData(void *_data)
    {
	return pfFlux::getFlux(_data)->getWritableData();
    }
    // CAPI:verb FluxWriteCompleteFromData
    static void		writeComplete(void *_data)
    {
	pfFlux::getFlux(_data)->writeComplete();
    }
    // CAPI:noverb
    static void		setFrame(int _frame);
    static void		keepFrame(int _frame);
    static void 	shiftFrameByPID(pid_t _pid, int _frame);
    static int		getFrame(void);
    static void		enableSyncGroup(uint _syncGroup);
    static void		disableSyncGroup(uint _syncGroup);
    static int		getEnableSyncGroup(uint _syncGroup);
    static void		syncGroupReady(uint _syncGroup);
    static void		syncComplete(void);
    static int          getNamedSyncGroup(const char *_name);
    static const char*  getSyncGroupName(uint _syncGroup);
    static int          getNumNamedSyncGroups();



private:
    void	pr_construct(size_t nbytes);

private:
    size_t data_size;
    flux_buf_t *buffers;
    int num_buffers;

    int  specified_buffers;
    uint flags;    		

    // This holds the maximum frame number that should become valid after 
    // a call to SyncComplete. This is the largest frame number that was 
    // writeCompleted before the last call to syncGrounpReady.
    int	  max_frame_to_sync_complete; // Added in 2.2.3

    uint mask;
    uint lock;
    short s_eng_count;
    short s_eng_slots;
    short c_eng_count;
    short c_eng_slots;
    pfEngine **s_eng;
    c_eng_t *c_eng;
    uint sync_group;
    pfFluxInitFuncType initFunc;
    pfFluxUserCopyFuncType userCopyFunc;


private:
    static pfType *classType;
};

#endif // !__PF_FLUX_H__
