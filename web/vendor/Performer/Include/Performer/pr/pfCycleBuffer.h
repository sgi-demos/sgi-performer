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
// pfCycleBuffer.h
//
// $Revision: 1.32 $
// $Date: 2002/03/14 21:11:10 $
//

#ifndef __PF_CYCLEBUFFER_H__
#define __PF_CYCLEBUFFER_H__

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


// ANY CHANGE IN THE SIZE OF PFMEMORY MUST BE REFLECTED IN PFCMEM_WORDS
// in pfObject.h

#define PFCYCLEMEMORY ((pfCycleMemory*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFCYCLEMEMORYBUFFER ((pfCycleMemory*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfCycleMemory : public pfMemory
{
    // CAPI:basename CMem
public:
    // Constructors, destructors
    //CAPI:private
    pfCycleMemory(pfCycleBuffer *cBuf);
    virtual ~pfCycleMemory();

private:
    static pfType *classType;

public:
    // virtual functions
    // CAPI:private
    virtual void 	*getData() const { 
	return (void*) (((char *)this)+sizeof(pfCycleMemory)); 
    }

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // gets and sets
    pfCycleBuffer*	getCBuffer() 	 { return parent; }
    int 		getFrame() const { return frameStamp; }
    // CAPI:private
    void 		setFrame(int fs) { frameStamp = fs; }


					//  32bits    64bits
private:				//  #bytes    #bytes
    pfCycleBuffer	*parent;	//	4	8
    int			frameStamp;	//	4	4
#ifdef N64 // pad to 2x32bit align
    int			pad;		//		4
#endif
    //					//-------------------
    //			TOTAL		//	8	16
};

#define PFCYCLEBUFFER ((pfCycleBuffer*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFCYCLEBUFFERBUFFER ((pfCycleBuffer*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfCycleBuffer : public pfObject
{
    // CAPI:basename CBuffer
    // CAPI:argname cBuf
public:
    // Constructors, destructors
    pfCycleBuffer(size_t nbytes);
    virtual ~pfCycleBuffer();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int 	copy(const pfMemory *_src);
    virtual int 	compare(const pfMemory *_mem) const;
    virtual int 	print(uint _travMode, uint _verbose, 
			      char *_prefix, FILE *file);

    virtual void*	getData() const;

    
public:	// statics
    static int		curCBufIndex;

public:
    // sets and gets
    pfCycleMemory*	getCMem(int index) const {
	return buffers[index];
    }
    // CAPI:verb GetCurCBufferData
    void*		getCurData() const {
	return buffers[curCBufIndex]->getData();
    }

public:
    // other functions
    void 		changed();
    // CAPI:verb InitCBuffer
    void 		init(void *data);

public:
    static int		config(int _numBuffers);
    static int		getConfig();
    static int		frame();
    static int		getFrameCount();

    // CAPI:verb GetCurCBufferIndex
    static int		getCurIndex();
    // CAPI:verb CurCBufferIndex
    static void		setCurIndex(int _index);
    // CAPI:verb GetCBuffer
    static pfCycleBuffer *getCBuffer(void *data);

private:
    pfCycleMemory  **buffers;

private:
    static pfType *classType;

};

#endif // !__PF_CYCLEBUFFER_H__
