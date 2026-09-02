//
//
// Copyright 2001, Silicon Graphics, Inc.
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
// pfIBRnode.h		IBRnode include file
//
// $Revision: 1.15 $
// $Date: 2002/11/05 22:48:15 $
//
#ifndef __PF_IBRNODE_H__
#define __PF_IBRNODE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfLists.h>
#include <Performer/pf/pfCuller.h>
#include <Performer/pf/pfBillboard.h>
#include <Performer/pf/pfIBR.h>

// Export to C API.
extern "C" {
#define PFIBRN_USE_PROXY           (1<<0) /* use proxy, not a billboard */
#define PFIBRN_VARY_PROXY_GEOSETS  (1<<1) /* one group of geosets per view 
					     (only for proxy) */
#define PFIBRN_TEXCOORDS_DEFINED   (1<<2) /* set automatically when setProxyTexCoords() is called */
}

#define PFIBRNODE ((pfIBRnode*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFIBRNODEBUFFER ((pfIBRnode*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfIBRnode : public pfBillboard
{
public:

    void setIBRtexture(pfIBRtexture *tex)  {
        PFIBRNODE->nb_setIBRtexture(tex);
    }

    pfIBRtexture* getIBRtexture()  {
        return PFIBRNODE->nb_getIBRtexture();
    }

    void setAngles(int i, float horAngle, float verAngle)  {
        PFIBRNODE->nb_setAngles(i, horAngle, verAngle);
    }

    void getAngles(int i, float *horAngle, float *verAngle)  {
        PFIBRNODE->nb_getAngles(i, horAngle, verAngle);
    }

    int getNumAngles()  {
        return PFIBRNODE->nb_getNumAngles();
    }

    void setProxyTexCoords(pfVec2 ***texCoords)  {
        PFIBRNODE->nb_setProxyTexCoords(texCoords);
    }

    pfVec2*** getProxyTexCoords()  {
        return PFIBRNODE->nb_getProxyTexCoords();
    }

    void setFlags(int which, int value)  {
        PFIBRNODE->nb_setFlags(which, value);
    }

    int getFlags(int which) const  {
        return PFIBRNODE->nb_getFlags(which);
    }
public:		// Constructors/Destructors
    //CAPI:basename IBRnode
    //CAPI:updatable
    //CAPI:newargs
    pfIBRnode();
    virtual ~pfIBRnode();

protected:
    pfIBRnode(pfBuffer *buf);
    pfIBRnode(const pfIBRnode *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void         pf_applyUpdate(const pfUpdatable *prev, int  upId);
    virtual pfUpdatable* pf_bufferClone(pfBuffer *buf);

PFINTERNAL:		// pfNode virtual traversals
    virtual void        nb_clean(uint64_t cleanBits, pfNodeCounts *counts);
    virtual pfNode*	nb_clone();
    virtual int 	nb_cull(int  mode, int  cullResult, _pfCuller *trav);
    virtual int 	nb_cullProgram(int  mode, int  cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual int  	nb_flatten(pfTraverser *trav);

PFINTERNAL:		// class specific sets and gets
    void 	  nb_setIBRtexture(pfIBRtexture *tex);
    pfIBRtexture *nb_getIBRtexture(void);
    void        nb_setAngles(int i, float horAngle, float verAngle);
    void        nb_getAngles(int i, float *horAngle, float *verAngle);
    int         nb_getNumAngles(void);

    void        nb_setProxyTexCoords(pfVec2 ***texCoords);
    pfVec2   ***nb_getProxyTexCoords(void);
    void        nb_setFlags(int which, int value);
    int         nb_getFlags(int which) const {
	return flags & which;
    }

    virtual void pf_draw(int  i, _pfCuller *cull, pfVec3 *eye);
    virtual void pf_draw(pfGeoState *gstate, pfDispList *dl, pfGeomStats *gstats, int  i, _pfCuller *cull, pfVec3 *eye);


private:
    friend class _pfCuller;

    pfIBRtexture *IBRtex;
    int          flags;

    pfVec2     ***texCoords;  /* for each geoset there is an array of pointers
				 to texture coordinates( different for each 
				 texture) */

    pfVec2       *angles;  // array of initial horizontal and vertical angles
    int          anglesSize;

private:
    static pfType *classType;
};

#endif // !__PF_IBRNODE_H___
