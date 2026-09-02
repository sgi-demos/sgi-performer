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
// pfScene.h		Scene include file
//
// $Revision: 1.79 $
// $Date: 2002/09/20 09:08:10 $
//
#ifndef __PF_SCENE_H__
#define __PF_SCENE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pf/pfGroup.h>
#include <Performer/pr/pfGeoSet.h>



#define PFSCENE ((pfScene*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFSCENEBUFFER ((pfScene*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfScene : public pfGroup
{
public:

    void setGState(pfGeoState *gs)  {
        PFSCENE->nb_setGState(gs);
    }

    pfGeoState* getGState() const  {
        return PFSCENE->nb_getGState();
    }

    void setGStateIndex(int gs)  {
        PFSCENE->nb_setGStateIndex(gs);
    }

    int getGStateIndex() const  {
        return PFSCENE->nb_getGStateIndex();
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfScene();
    virtual ~pfScene();

protected:
    //CAPI:private
    pfScene(pfBuffer *buf);
    pfScene(const pfScene* scene, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions
    virtual int 	    checkDelete(void);

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int updateId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);
    virtual void	    pf_updateBufferList(pfBuffer *buf, int addRemove) {
        buf->pf_updateList(this, addRemove);
    }
    // XXXXXXXXX Get rid of this by properly implementing pf_copy()
    virtual int	    	    pf_needsUpdate(void) {
	return 1;
    }    

PFINTERNAL:		// pfNode virtual traversals
    virtual pfNode*	    nb_clone();
    virtual int		    nb_cull(int cullMode, int cullResult, _pfCuller *trav);
    virtual int		    nb_cullProgram(int cullMode, int cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual void 	    nb_write(pfTraverser *trav, uint which, uint verbose);

private:	// pfNode virtual non-traversal functions
friend class pfGroup;
    virtual int    	    pf_addParent(pfGroup *g);

PFINTERNAL:		// class specific sets and gets
    void		    nb_setGState(pfGeoState *gs);
    pfGeoState*	   	    nb_getGState() const;
    void		    nb_setGStateIndex(int gs);
    int		   	    nb_getGStateIndex() const;


private:
    uint		flags;

    // Global geostate which is loaded before scene is rendered
    GStateId		gstate;

    // List of pfPaths used for lightsources
    _pfVoidList		lspathList;

private:
    static pfType *classType;

};

PF_UPDATABLE_REF(pfScene, _pfSceneRef);

#endif // !__PF_SCENE_H__
