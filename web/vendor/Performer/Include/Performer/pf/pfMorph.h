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
// pfMorph.h		Morph include file
//
// $Revision: 1.37 $
// $Date: 2002/05/30 00:33:56 $
//
#ifndef __PF_MORPH_H__
#define __PF_MORPH_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfLists.h>

/* for bzero() */
#include <libpr/win32stubs.h>

class pfBuffer;

struct _MorphAttr 
{

public:
    int 	attr;

    int 	floatsPerElt;
    int 	isCBuf;
    void	*dst;
    int 	dstLen;

    float	*weights;

    int 	nSrcs;
    float	**srcAttrs;
    ushort	**srcIndices;
    int 	*srcLens;

};

SLIST_DECLARE(_MorphAttr, _pfMorphAttrList);

#define PFMORPH ((pfMorph*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFMORPHBUFFER ((pfMorph*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfMorph : public pfGroup
{
public:

    int setAttr(int  index, int  attr, int  nelts, void *dst, int  nsrcs, float *alist[], ushort *ilist[], int  n[])  {
        return PFMORPH->nb_setAttr(index, attr, nelts, dst, nsrcs, alist, ilist, n);
    }

    int getNumAttrs() const  {
        return PFMORPH->nb_getNumAttrs();
    }

    int getSrc(int  index, int  src, float **alist, ushort **ilist, int  *n) const  {
        return PFMORPH->nb_getSrc(index, src, alist, ilist, n);
    }

    int getNumSrcs(int  index) const  {
        return PFMORPH->nb_getNumSrcs(index);
    }

    void* getDst(int  index) const  {
        return PFMORPH->nb_getDst(index);
    }

    int setWeights(int  index, float *weights)  {
        return PFMORPH->nb_setWeights(index, weights);
    }

    int getWeights(int  index, float *weights) const  {
        return PFMORPH->nb_getWeights(index, weights);
    }

    void evaluate()  {
        PFMORPH->nb_evaluate();
    }

    int setWeights(int  index, float *weights, int  nprocs)  {
        return PFMORPH->nb_setWeights(index, weights, nprocs);
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfMorph();
    virtual ~pfMorph();

protected:
    //CAPI:private
    pfMorph(pfBuffer *buf);
    pfMorph(const pfMorph *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int  upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);

PFINTERNAL:	
    // pfNode public virtual functions
    virtual int	    	    needsApp() {
	return 1;
    }

PFINTERNAL:		// pfNode virtual traversals
    virtual int 	    app(pfTraverser *trav);
    virtual pfNode*	    nb_clone();

PFINTERNAL:		// specific sets and gets
    int 		    nb_setAttr(int  index, int  attr, int  nelts, void *dst, int  nsrcs, float *alist[], ushort *ilist[], int  n[]);
    int 		    nb_getNumAttrs(void) const {
	return attrList.getNum();
    }

    int 		    nb_getSrc(int  index, int  src, float **alist, ushort **ilist, int  *n) const;

    int 		    nb_getNumSrcs(int  index) const {
	return attrList[index].nSrcs;
    }

    void*	    	    nb_getDst(int  index) const;

    int 		    nb_setWeights(int  index, float *weights) {
	return nb_setWeights(index, weights, 1);
    }
    //CAPI:noverb
    int 		    nb_getWeights(int  index, float *weights) const;

    //CAPI:verb
    void		    nb_evaluate(void);

    //CAPI:private MPMorphWeights
    int 		    nb_setWeights(int  index, float *weights, int  nprocs);


private:
    friend void _pfMPMorph(pfMorph *, int );

    _pfMorphAttrList	attrList;

private:
    static pfType *classType;
};

#endif // !__PF_MORPH_H__
