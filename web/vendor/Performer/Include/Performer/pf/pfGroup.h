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
// pfGroup.h
//
// $Revision: 1.77 $
// $Date: 2002/05/30 00:33:56 $
//
//
#ifndef __PF_GROUP_H__
#define __PF_GROUP_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfLists.h>

class pfBuffer;
struct _pfMergeUp;

#define PFGROUP ((pfGroup*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFGROUPBUFFER ((pfGroup*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfGroup : public pfNode
{
public:

    int addChild(pfNode *child)  {
        return PFGROUP->nb_addChild(child);
    }

    int insertChild(int  index, pfNode *child)  {
        return PFGROUP->nb_insertChild(index, child);
    }

    int removeChild(pfNode *child)  {
        return PFGROUP->nb_removeChild(child);
    }

    int replaceChild(pfNode *oldn, pfNode *newn)  {
        return PFGROUP->nb_replaceChild(oldn, newn);
    }

    int bufferAddChild(pfNode *child)  {
        return PFGROUP->nb_bufferAddChild(child);
    }

    int bufferRemoveChild(pfNode *child)  {
        return PFGROUP->nb_bufferRemoveChild(child);
    }

    pfNode* getChild(int  i) const  {
        return PFGROUP->nb_getChild(i);
    }

    int getNumChildren() const  {
        return PFGROUP->nb_getNumChildren();
    }

    int searchChild(pfNode *n) const  {
        return PFGROUP->nb_searchChild(n);
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfGroup();
    virtual ~pfGroup();

friend class pfASD;
protected:
    pfGroup(pfBuffer *buf);
    pfGroup(const pfGroup *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions
    virtual int 	    checkDelete(void);


PFINTERNAL:		// pfUpdatable virtual functions
    virtual void 	    pf_applyUpdate(const pfUpdatable *prev, int upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);

PFINTERNAL:	
    // pfNode public virtual functions
    virtual int	    	    needsApp();
    virtual int	    	    needsASD();

PFINTERNAL:		// pfNode virtual traversals
    virtual void	    nb_clean(uint64_t cleanBits, pfNodeCounts *counts);
    virtual pfNode*	    nb_clone();
    virtual int 	    app(pfTraverser *trav);
    virtual int 	    ASDtrav(pfTraverser *trav);
    virtual int 	    nb_cull(int  mode, int  cullResult, _pfCuller *trav);
    virtual int 	    nb_cullProgram(int  mode, int  cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual pfNode*	    nb_find(const char *_name, pfType *_find);
    virtual int 	    nb_flatten(pfTraverser *trav);
    virtual int  	    nb_intersect(_pfIsector *isector);
    virtual void 	    nb_setTravMask(int  which, uint  bits, int  setWho, int  lop);
    virtual int 	    nb_traverse(pfTraverser *trav);
    virtual void 	    nb_write(pfTraverser *trav, uint  which, uint  verbose);

PFINTERNAL:		// pfGroup virtual functions
    virtual int 	    nb_setChild(int  i, pfNode *n);
    //CAPI:virtual
    //CAPI:basename 
    virtual int 	    nb_addChild(pfNode *child);
    virtual int 	    nb_insertChild(int  index, pfNode *child);
    virtual int 	    nb_removeChild(pfNode *child);
    virtual int 	    nb_replaceChild(pfNode *oldn, pfNode *newn);
    //CAPI:noupdatable
    virtual int		    nb_bufferAddChild(pfNode *child);
    virtual int		    nb_bufferRemoveChild(pfNode *child);
    //CAPI:updatable

PFINTERNAL:		// specific sets and gets
    pfNode*		    nb_getChild(int  i) const;
    int 		    nb_getNumChildren() const {
	return childList.getNum();
    }
    int 		    nb_searchChild(pfNode *n) const {
	return childList.search(n);
    }


protected:	// data, This should be private.
    pfNodeList    	childList;

private:
    static pfType *classType;
};


#endif // !__PF_GROUP_H__
