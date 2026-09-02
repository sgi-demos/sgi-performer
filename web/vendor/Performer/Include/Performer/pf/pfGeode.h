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
// pfGeode.h		Geode include file
//
// $Revision: 1.63 $
// $Date: 2002/08/08 02:45:06 $
//
#ifndef __PF_GEODE_H__
#define __PF_GEODE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfLists.h>
#include <Performer/pr/pfFlux.h>

class pfBuffer;

#define PFGEODE ((pfGeode*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFGEODEBUFFER ((pfGeode*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfGeode : public pfNode
{
public:

    int addGSet(pfGeoSet *gset)  {
        return PFGEODE->nb_addGSet(gset);
    }

    int insertGSet(int index, pfGeoSet *gset)  {
        return PFGEODE->nb_insertGSet(index, gset);
    }

    int replaceGSet(pfGeoSet *oldgs, pfGeoSet *newgs)  {
        return PFGEODE->nb_replaceGSet(oldgs, newgs);
    }

    int removeGSet(pfGeoSet *gset)  {
        return PFGEODE->nb_removeGSet(gset);
    }

    int countShadedGSets()  {
        return PFGEODE->nb_countShadedGSets();
    }

    pfGeoSet* getGSet(int i) const  {
        return PFGEODE->nb_getGSet(i);
    }

    int getNumGSets() const  {
        return PFGEODE->nb_getNumGSets();
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfGeode();
    virtual ~pfGeode();

friend class pfASD;
protected:
    pfGeode(pfBuffer *buf);
    pfGeode(const pfGeode *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions
    virtual int		    checkDelete(void);

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);

PFINTERNAL:		// pfNode virtual traversals
    virtual void	    nb_clean(uint64_t cleanBits, pfNodeCounts *counts);
    virtual pfNode*	    nb_clone();
    virtual int 	    nb_cull(int mode, int cullResult, _pfCuller *trav);
    virtual int 	    nb_cullProgram(int mode, int cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual int 	    nb_flatten(pfTraverser *trav);
    virtual int 	    nb_intersect(_pfIsector *isector);
    virtual void 	    nb_setTravMask(int which, uint bits, int setWho, int lop);
    virtual void	    nb_write(pfTraverser *trav, uint which, uint verbose);

PFINTERNAL:	// pfGeoSet virtual functions
    //CAPI:basename 
    //CAPI:virtual
    virtual int 	    nb_addGSet(pfGeoSet *gset);
    virtual int 	    nb_insertGSet(int index, pfGeoSet *gset);
    virtual int 	    nb_replaceGSet(pfGeoSet *oldgs, pfGeoSet *newgs);
    virtual int 	    nb_removeGSet(pfGeoSet *gset);
    virtual int             nb_countShadedGSets();
PFINTERNAL:	// Specific
    pfGeoSet*		    nb_getGSet(int i) const
    {
	if (numFluxed > 0 && ((pfMemory*)gsetList[i])->isFluxed())
	    return (pfGeoSet*)pfFlux::getCurData(gsetList[i]);
	else
	    return (pfGeoSet*)gsetList[i];
    }
    int 		    nb_getNumGSets() const
    {
	return gsetList.getNum();
    }


protected:
    _pfVoidList	gsetList;	// List of pfGeoSets
    int		numFluxed;	// number of fluxed pfGeoSets
    int         numShaded;      // number of shaded pfGeoSets

private:
    static pfType *classType;
};



#endif // !__PF_GEODE_H__
