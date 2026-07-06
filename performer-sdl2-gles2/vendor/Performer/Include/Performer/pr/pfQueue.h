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
// pfQueue.h
//
// $Revision: 1.21 $
// $Date: 2002/04/11 23:44:51 $
//
//

#ifndef __PF_QUEUE_H__
#define __PF_QUEUE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfImageTile.h>

extern "C" {     // EXPORT to CAPI
/* ----------------------- pfQueue Tokens ----------------------- */
#define PFQUEUE_PROC_ACTIVE_ALWAYS		0x0
#define PFQUEUE_PROC_ACTIVE_INSIDE_RANGE	0x1
#define PFQUEUE_PROC_ACTIVE_OUTSIDE_RANGE	0x2
#define PFQUEUE_PROC_ACTIVE_LESS_THAN		0x3
#define PFQUEUE_PROC_ACTIVE_GREATER_THAN	0x4
#define PFQUEUE_PROC_EXIT			0xffffffff //special value; ugh!

extern DLLEXPORT int pfGetNumGlobalQueueServiceProcs(void);
extern DLLEXPORT int pfGetGlobalQueueServiceProcPID(int _n);
extern DLLEXPORT pfQueue* pfGetGlobalQueueServiceProcQueue(int _n);

} // extern "C" END of C include export

typedef struct process
{
    pid_t			pid;
    volatile int 		done;
    int				lowBlock;
    int				lowUnBlock;
    int				highBlock;
    int				highUnBlock;
    int 			blockMode;
    pfQueueServiceFuncType 	func;
    pfQueue			*queue;
} Process;


#define PFQUEUE ((pfQueue*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFQUEUEBUFFER ((pfQueue*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfQueue : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename Queue
    pfQueue(int _eltSize, int _nElts);

    // CAPI:private
    // constructor with reasonable defaults
    pfQueue();
    virtual ~pfQueue();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

/*----------------------------- pfImageTile --------------------------------*/
public:
    // sets and gets
    // CAPI:basename
    // CAPI:basename Queue
    void		setArrayLen(int _length);
    int			getArrayLen(void);
    int			getNum(void);
    int                 getElementSize(void);

    int			getNumServiceProcs(void);
    int			getServiceProcPID(int which);

    //Sort routines
    void                setSortFunc(pfQueueSortFuncType _func);
    pfQueueSortFuncType getSortFunc(void);

    void                setSortMode(int _bool);
    int                 getSortMode(void);

    void                setInputRange(int _low, int _hi);
    void                getInputRange(int *_low, int *_hi);
    void                setOutputRange(int _low, int _hi);
    void                getOutputRange(int *_low, int *_hi);
    int                 getSortProcPID(void);

public:
    // Methods
    // CAPI:verb InsertQueueElt
    void		insert(void *_elt);
    // CAPI:verb InsertFrontQueueElt
    void		insertFront(void *_elt);
    // CAPI:verb RemoveQueueElt
    void*		remove(void);
    // CAPI:verb AttemptRemoveQueueElt
    void*		attemptRemove(void);
    // CAPI:verb AddQueueServiceProc
    int			addServiceProc(pfQueueServiceFuncType _fnc);
    // CAPI:verb SignalAllQueueServiceProcs
    int			signalAllServiceProcs(int _count, int _token);
    // CAPI:verb NotifyQueueSortProc
    void                notifySortProc(void);


protected:
#if defined(__linux__has_MP__) || !defined(__linux__)
    usema_t		*insertLock;
    usema_t		*removeLock;
    usema_t		*queueSema;
    usema_t		*procExitSema;
    usema_t             *sortSema; //when sortProc needs to be active
#endif


    pfList		*procs;
    pfList		*blockLT;
    pfList		*blockedLT;
    pfList		*blockGT;
    pfList		*blockedGT;

    int			eltSize;
    int			arrLen;
    int			insertElt;
    int			removeElt;
    void		**q;

    //Sort queue specific state
    volatile int        sortMode; //boolean
    pfQueueSortFuncType sortFunc;
    pfList              *sortList;
    pfQueue             *outQueue;
    int                 inLo, inHi;
    int                 outLo, outHi;
    int                 sortProcPID;

private:
    static pfType *classType;
};


#endif // !__PF_QUEUE_H__

