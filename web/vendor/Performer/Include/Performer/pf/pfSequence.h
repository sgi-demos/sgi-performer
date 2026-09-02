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
// pfSequence.h
//
// $Revision: 1.46 $
// $Date: 2002/05/30 00:33:56 $
//
//
#ifndef __PF_SEQUENCE_H__
#define __PF_SEQUENCE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfGroup.h>


extern "C" {
/* ----------------------- pfBuffer Tokens ----------------------- */

/* pfSeqFrame() */
#define PFSEQ_STOP	0
#define PFSEQ_START	1
#define PFSEQ_PAUSE	2
#define PFSEQ_RESUME	3

#define PFSEQ_ALL	-1

/* pfSeqInterval() */ 
#define PFSEQ_CYCLE	0
#define PFSEQ_SWING	1
}

class pfBuffer;

#define PFSEQUENCE ((pfSequence*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFSEQUENCEBUFFER ((pfSequence*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfSequence : public pfGroup
{
public:

    void setDuration(float sp, int nRep)  {
        PFSEQUENCE->nb_setDuration(sp, nRep);
    }

    void getDuration(float *sp, int *nRep) const  {
        PFSEQUENCE->nb_getDuration(sp, nRep);
    }

    void setInterval(int imode, int beg, int e)  {
        PFSEQUENCE->nb_setInterval(imode, beg, e);
    }

    void getInterval(int *imode, int *beg, int *e) const  {
        PFSEQUENCE->nb_getInterval(imode, beg, e);
    }

    void setMode(int m)  {
        PFSEQUENCE->nb_setMode(m);
    }

    int getMode() const  {
        return PFSEQUENCE->nb_getMode();
    }

    void setTime(int index, double time)  {
        PFSEQUENCE->nb_setTime(index, time);
    }

    double getTime(int index) const  {
        return PFSEQUENCE->nb_getTime(index);
    }

    int getFrame(int *rep) const  {
        return PFSEQUENCE->nb_getFrame(rep);
    }

    void setEvaluation(int state)  {
        PFSEQUENCE->nb_setEvaluation(state);
    }

    int getEvaluation()  {
        return PFSEQUENCE->nb_getEvaluation();
    }
public:		// Constructors/Destructors
    //CAPI:basename Seq
    //CAPI:updatable
    //CAPI:newargs
    pfSequence();
    virtual ~pfSequence();

protected:
    //CAPI:private
    pfSequence(pfBuffer *buf);
    pfSequence(const pfSequence *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);
    virtual void	    pf_updateBufferList(pfBuffer *buf, int addRemove) {
        buf->pf_updateList(this, addRemove);
    }

PFINTERNAL:		// pfNode virtual traversals
    virtual int 	    app(pfTraverser *trav);
    virtual int 	    ASDtrav(pfTraverser *trav);
    virtual pfNode*	    nb_clone();
    virtual int 	    nb_cull(int mode, int cullResult, _pfCuller *trav);
    virtual int 	    nb_cullProgram(int mode, int cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual int 	    nb_intersect(_pfIsector *isector);
    virtual int 	    nb_traverse(pfTraverser *trav);

PFINTERNAL:		// pfGroup virtual functions
    virtual int 	    nb_addChild(pfNode *n);
    virtual int 	    nb_insertChild(int index, pfNode *n);
    virtual int 	    nb_removeChild(pfNode *n);

PFINTERNAL:		// class specific sets and gets
    void		    nb_setDuration(float sp, int nRep);
    void		    nb_getDuration(float *sp, int *nRep) const;
    void		    nb_setInterval(int imode, int beg, int e);
    void		    nb_getInterval(int *imode, int *beg, int *e) const;
    void		    nb_setMode(int m);
    int 		    nb_getMode() const;
    void		    nb_setTime(int index, double time);
    double		    nb_getTime(int index) const {
	return times[index];
    }
    int 		    nb_getFrame(int *rep) const;
    void	    	    nb_setEvaluation(int state);
    int 		    nb_getEvaluation(void) { return (evaluationState); }


protected:
    int 		begin, end;	// begin/end indexes, < 0 == num-1
    int 		iMode;		// Interval mode
    int 		child;		// Current child to draw.
    double		startTime;	// Time sequence started.
    double		nextTime;	// Next time boundary
    double		defaultTime;	// Default frame time for each child
    _pfFloatList		times;		// List of times per frame
    float		speed;		// Speed up/down, >1.,  <1.
    float		invDftSpeed;	// 1/(defaultTime * speed)
    int 		repeat;		// Number of times seq repeated
    int 		numRepeats;	// Total number of repeats
    int 		seqMode;	// Current mode(START, STOP, PAUSE etc)
    int			allSameTime;	// all the children have the same time
    unsigned char	evaluationState;// flag: is this node on the global
					// pfSequence list (and so it will be
					// evaluated every frame).

private:
    static pfType *classType;
};

#endif // !__PF_SEQUENCE_H__
