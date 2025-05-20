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
// pfBuffer.h
//
// $Revision: 1.74 $
// $Date: 2002/09/20 09:08:10 $
//
//
#ifndef __PF_BUFFER_H__
#define __PF_BUFFER_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <stdio.h>		// __file_s in ulocks.h
#if defined(__linux__has_MP__) || !defined(__linux__)
#include <ulocks.h>
#endif  /* __linux__ */

#include <Performer/pf/pfUpdate.h>
#include <Performer/pf/pfLists.h>


struct _pfDelUp
{

PFINTERNAL:
    pfMemory		*mem;		// pfMemory to delete

    int 		frameStamp;	// Frame count when delete was requested

    uint 		ref;		// Bit mask reference counter. If
					// bit 'i' is 1, then output i still
					// references the updatable.
};

SLIST_DECLARE(_pfDelUp, _pfDeleteList);

// _pfMergeUp is currently restricted to UPBUF_ADD, UPBUF_REMOVE, UPBUF_MOVE.
// In general, _pfMergeUp should handle arbitrary data
struct _pfMergeUp
{

PFINTERNAL:
    int		upId, i;
    void	*a, *b, *c;
};

SLIST_DECLARE(_pfMergeUp, _pfMergeUpdateList);

class _pfUpdateList;
class pfUpdateQueue;


// Buffer has list of pfUpdatables indexed by updatable id and an update list
// for each buffer output. A pfBuffer may have 0 outputs in which case update
// are ignored.
class DLLEXPORT pfBuffer : public pfStruct
{
public:
    //CAPI:private
    void* operator new(size_t s);
    void  operator delete(void *p) {pfStruct::operator delete(p);}

public:
    //CAPI:newargs
    pfBuffer();
    ~pfBuffer();

protected:
    pfBuffer(pfBuffer *up, int numd, int un, int sflag);
    
public:
    void		setScope(pfObject *obj, int scope);
    int			getScope(pfObject *obj);
    //CAPI:verb
    static void		merge();
    //CAPI:verb UnrefDelete
    static int		unrefDelete(void *mem);
    //CAPI:verb Delete
    static int		checkDelete(void *mem);
    //CAPI:verb BufferInsert
    static int		insert(void *parent, int index, void *child);
    //CAPI:verb BufferRemove
    static int		remove(void *parent, void *child);
    //CAPI:verb BufferAdd
    static int		add(void *parent, void *child);
    //CAPI:verb BufferReplace
    static int		replace(void *parent, void *oldChild, void *newChild);
    //CAPI:verb BufferSet
    static int		set(void *parent, int index, void *child);

    void		select();

    // Get handle to object in this buffer corresponding to u
    pfUpdatable*	pf_indexUpdatable(const pfUpdatable *u) const;

PFINTERNAL:
    //CAPI:private
    friend class pfLightSource;
    void		pf_updateList(pfLightSource *ls, int addRemove);
    friend class pfScene;
    void		pf_updateList(pfScene *scene, int addRemove);
    friend class pfSequence;
    void		pf_updateList(pfSequence *seq, int addRemove);
    friend class pfPipe;
    void		pf_updateList(pfPipe *pipe, int addRemove);
    friend class pfChannel;
    void		pf_updateList(pfChannel *chan, int addRemove);
    friend class pfEarthSky;
    void		pf_updateList(pfEarthSky *esky, int addRemove);
    friend class pfPipeVideoChannel;
    void		pf_updateList(pfPipeVideoChannel *pvchan, int addRemove);
    friend class pfPipeWindow;
    void		pf_updateList(pfPipeWindow *pwin, int addRemove);
    friend class pfFrameStats;
    void		pf_updateList(pfFrameStats *fstats, int addRemove);
  

private:
    friend DLLEXPORT void pfDBase(void);
    friend DLLEXPORT int pfFrame();

    int			sivaFlag;	// TRUE if buffer is creator/destroyer
    
    PF_ULOCK_T		idLock;		// Lock for idTable and dirtyIdList
    PF_ULOCK_T		mergeLock;	// Lock for mergeList
    PF_ULOCK_T		listLock;	// Lock for scene, seq, lsourceLists
    _pfUpdatablePList   	idTable;	// Updatable list indexed by pfId
    int 		cBufIndex;	// Current pfCycleBuffer index
    
    // Creation/deletion buffer only
    int			numDownstream;	// Total number of downstream buffers

    pfUpdateQueue	*updateQueue;   // Yair: One for all downstream buffers.
    
    _pfIntList		newList;	// List of new updatables
    
    _pfDeleteList    	deleteList;	// List of delete requests. Actual
    // delete is delayed
    
    int 		updateIndex;	// Downstream buffers attach to this output
    
    _pfVoidList		mergeList;	// List of mergable buffers, visible
    // to all processes
    
    _pfVoidList		localMergeList;	// List of mergable buffers, visible
    // to APP only
    
    _pfMergeUpdateList	*mergeUpdateList;// List of updates which are
                                        // processed at merge time only
    PF_USEMA_T		*mergeSema;

    _pfVoidList		*dirtyIdList;   // List of dirtied idTable entries
    _pfVoidList		*updateIdList;  // idTable entries needing update
    
    // Downstream buffer only
    pfBuffer	   	*upstreamBuffer;// pfBuffer which 'this' is updated 
    // from
    int 		upstreamOutput; // Integer id of upstream output
    
    // APP buffer only 
    PF_USEMA_T		*deleteSema, *deleteDoneSema;

    
PFINTERNAL:
    // Node lists for special database processing. These should be locked when
    // accessing since the deletion process may change them. These also need
    // to be merged in nb_merge()
    _pfSceneList		*sceneList;
    _pfLSourceList	*lsourceList;
    _pfSequenceList	*seqList;
    
    // List of updatables which need pf_copy() rather than pf_applyUpdate()
    // This includes updatables that must exist in the DRAW: 
    // pfPipe, pfChannel, pfPipeVideoChannel, pfPipeWindow, pfEarthSky 
    //	(basically all non-nodes)
    _pfUpdatableList	copyList;
};

extern "C" {     // EXPORT to CAPI

/* ------------------ pfBuffer Related Functions--------------------- */

extern DLLEXPORT pfBuffer*	pfGetCurBuffer(void);
} // extern "C" END of C include export

#ifndef __BUILDCAPI__

#define pfDelete 	pfBuffer::checkDelete
#define pfUnrefDelete 	pfBuffer::unrefDelete

#endif

#endif	// !__PF_BUFFER_H__




