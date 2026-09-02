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
// pfLightpoint.h
//
// $Revision: 1.55 $
// $Date: 2002/05/30 00:33:56 $
//
//
#ifndef __PF_LIGHTPOINT_H__
#define __PF_LIGHTPOINT_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfLists.h>
#include <Performer/pr/pfGeoSet.h>


class pfBuffer;

extern "C" {     // EXPORT to CAPI

/* ----------------------- pfLightPoint Tokens ----------------------- */

/* pfLPointShape() */
#define PFLP_OMNIDIRECTIONAL	0
#define PFLP_UNIDIRECTIONAL	1
#define PFLP_BIDIRECTIONAL	2

/* pfLPointColor() */
#define PFLP_OVERALL		-1
} // extern "C" END of C include export

#define PFLIGHTPOINT ((pfLightPoint*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFLIGHTPOINTBUFFER ((pfLightPoint*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfLightPoint : public pfNode
{
public:

    int getNumPoints() const  {
        return PFLIGHTPOINT->nb_getNumPoints();
    }

    void setSize(float s)  {
        PFLIGHTPOINT->nb_setSize(s);
    }

    float getSize() const  {
        return PFLIGHTPOINT->nb_getSize();
    }

    void setFogScale(float onset, float opaque)  {
        PFLIGHTPOINT->nb_setFogScale(onset, opaque);
    }

    void getFogScale(float *onset, float *opaque) const  {
        PFLIGHTPOINT->nb_getFogScale(onset, opaque);
    }

    void setRot(float azim, float elev, float roll)  {
        PFLIGHTPOINT->nb_setRot(azim, elev, roll);
    }

    void getRot(float *azim, float *elev, float *roll) const  {
        PFLIGHTPOINT->nb_getRot(azim, elev, roll);
    }

    void setShape(int  dir, float he, float ve, float f)  {
        PFLIGHTPOINT->nb_setShape(dir, he, ve, f);
    }

    void getShape(int  *dir, float *he, float *ve, float *f) const  {
        PFLIGHTPOINT->nb_getShape(dir, he, ve, f);
    }

    pfGeoSet* getGSet() const  {
        return PFLIGHTPOINT->nb_getGSet();
    }

    void setPos(int  i, pfVec3& p)  {
        PFLIGHTPOINT->nb_setPos(i, p);
    }

    void getPos(int  i, pfVec3& p) const  {
        PFLIGHTPOINT->nb_getPos(i, p);
    }

    void setColor(int  i, pfVec4& clr)  {
        PFLIGHTPOINT->nb_setColor(i, clr);
    }

    void getColor(int  i, pfVec4& clr) const  {
        PFLIGHTPOINT->nb_getColor(i, clr);
    }
public:		// Constructors/Destructors
    //CAPI:basename LPoint
    //CAPI:updatable
    //CAPI:newargs
    pfLightPoint(int n);
    virtual ~pfLightPoint();

protected:
    pfLightPoint(const pfLightPoint *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int  upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);
    // XXXXXXXXX Get rid of this by properly implementing pf_copy()

PFINTERNAL:		// pfNode virtual traversals
    virtual pfNode*	    nb_clone();
    virtual void	    nb_clean(uint64_t cleanBits, pfNodeCounts *counts);
    virtual int 	    nb_cull(int  mode, int  cullResult, _pfCuller *trav);
    virtual int 	    nb_cullProgram(int  mode, int  cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual int  	    nb_flatten(pfTraverser *trav);
    virtual int  	    nb_intersect(_pfIsector *isector);
    virtual void 	    nb_write(pfTraverser *trav, uint  which, uint  verbose);
   

PFINTERNAL:		// specific sets and gets
    //CAPI:verb GetNumLPoints
    int 		    nb_getNumPoints(void) const {
	return lpgset->getNumPrims();
    }
    //CAPI:noverb
    void		    nb_setSize(float s);
    float		    nb_getSize(void) const;

    void		    nb_setFogScale(float onset, float opaque);
    void		    nb_getFogScale(float *onset, float *opaque) const;

    void		    nb_setRot(float azim, float elev, float roll);
    void		    nb_getRot(float *azim, float *elev, float *roll) const;

    void		    nb_setShape(int  dir, float he, float ve, float f);
    void		    nb_getShape(int  *dir, float *he, float *ve, float *f) const;

    pfGeoSet*		    nb_getGSet() const {
	return lpgset;
    }

    void		    nb_setPos(int  i, pfVec3& p);
    void		    nb_getPos(int  i, pfVec3& p) const;

    void		    nb_setColor(int  i, pfVec4& clr);
    void		    nb_getColor(int  i, pfVec4& clr) const;
    

protected:
    friend class _pfCuller;

    pfVec3		eulers;
    pfLPointState	*lplpstate;
    pfGeoSet		*lpgset;
    pfGeoState		*lpgstate;
    
private:
    static pfType *classType;
};

#endif // !__PF_LIGHTPOINT_H__ 
