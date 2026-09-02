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
// pfNode.h		Node class include file
//
// $Revision: 1.179 $
// $Date: 2002/08/02 02:10:54 $
//
#ifndef __PF_NODE_H__
#define __PF_NODE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pf/pfUpdatable.h>
#include <Performer/pf/pfFrameStats.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pf.h>	// get basic class defs



extern pfBuffer		*_pfCloneBuffer;

class pfTraverser;
class _pfCuller;
class _pfIsector;
struct _pfCullPgInfo;

extern "C" {     // EXPORT to CAPI
/* ----------------------- pfNode Tokens ----------------------- */

/* additional intersection modes for libpf */
#define PFTRAV_IS_GEODE		0x100 /* intersect with geode bounding volume */
#define PFTRAV_IS_TEXT		0x200 /* intersect with text bounding volume */
#define PFTRAV_IS_PATH		0x400 /* return path information */
#define PFTRAV_IS_NO_PART	0x800 /* don't use partitions */


#define PFPK_M_NEAREST		0x0 /* return nearest pick - the default */
#define PFPK_M_ALL		0x800 /* return all hits instead of last one */

/* additional pfHit validity bits in flags */
#define PFHIT_XFORM		0x40 /* transformation is valid */

/* additional pfHit queries for libpf  */
#define PFQHIT_NODE		20
#define PFQHIT_NAME		21
#define PFQHIT_XFORM		22
#define PFQHIT_PATH		23

/* pfNodeBufferMode() */
#define PFN_AUTO_CLONE		0

/* pfNodeTravMode() */
#define PFN_CULL_SORT	0
    #define PFN_CULL_SORT_UNCONTAINED  0
    #define PFN_CULL_SORT_CONTAINED  1

/* NOTE: 1.2 -> 2.0 porting: PFN_BOUND_STATIC/DYNAMIC are 
 * superseded by PFBOUND_STATIC and PFBOUND_DYNAMIC
 */

} // extern "C" END of C include export


struct _pfCallback;

#define PFNODE ((pfNode*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFNODEBUFFER ((pfNode*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfNode : public pfUpdatable
{
public:

    void setTravMask(int which, uint mask, int setMode, int bitOp)  {
        PFNODE->nb_setTravMask(which, mask, setMode, bitOp);
    }

    pfNode* find(const char *_name, pfType *_type)  {
        return PFNODE->nb_find(_name, _type);
    }

    int setName(const char *name)  {
        return PFNODE->nb_setName(name);
    }

    const char* getName() const  {
        return PFNODE->nb_getName();
    }

    void setTravFuncs(int which, pfNodeTravFuncType pre, pfNodeTravFuncType post)  {
        PFNODE->nb_setTravFuncs(which, pre, post);
    }

    void getTravFuncs(int which, pfNodeTravFuncType *pre, pfNodeTravFuncType *post) const  {
        PFNODE->nb_getTravFuncs(which, pre, post);
    }

    void setTravData(int which, void *data)  {
        PFNODE->nb_setTravData(which, data);
    }

    void* getTravData(int which) const  {
        return PFNODE->nb_getTravData(which);
    }

    unsigned int getTravMask(int which) const  {
        return PFNODE->nb_getTravMask(which);
    }

    void setTravMode(int which, int mode, int val)  {
        PFNODE->nb_setTravMode(which, mode, val);
    }

    int getTravMode(int which, int mode) const  {
        return PFNODE->nb_getTravMode(which, mode);
    }

    void setBufferMode(int mode, int val)  {
        PFNODE->nb_setBufferMode(mode, val);
    }

    int getBufferMode(int mode) const  {
        return PFNODE->nb_getBufferMode(mode);
    }

    pfGroup* getParent(int i) const  {
        return PFNODE->nb_getParent(i);
    }

    int getNumParents() const  {
        return PFNODE->nb_getNumParents();
    }

    void setBound(pfSphere *sph, int mode)  {
        PFNODE->nb_setBound(sph, mode);
    }

    int getBound(pfSphere *sph)  {
        return PFNODE->nb_getBound(sph);
    }

    pfNode* lookup(const char *_name, pfType* _type)  {
        return nb_lookup(_name, _type);
    }

    int isect(pfSegSet *segSet, pfHit **hits[])  {
        return PFNODE->nb_isect(segSet, hits);
    }

    int flatten(int _mode)  {
        return PFNODE->nb_flatten(_mode);
    }

    pfNode* clone(int _mode)  {
        return PFNODE->nb_clone(_mode);
    }

    pfNode* bufferClone(int _mode, pfBuffer *buf)  {
        return PFNODEBUFFER->nb_bufferClone(_mode, buf);
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    virtual ~pfNode();
    
protected:
    pfNode(pfBuffer *buf);
    // Buffer-copy a node
    pfNode(const pfNode *prev, pfBuffer *buf);
    
public:
    // per class functions;
    static void	    init();
    static pfType*  getClassType() { return classType; }
    
PFINTERNAL:		
    // pfMemory virtual functions
    // CAPI:private
    virtual int     checkDelete(void);
    virtual int	    print(uint travMode, uint verbose, char *_prefix, FILE *file);
    
PFINTERNAL:		
    // pfUpdatable virtual functions
    virtual void    pf_applyUpdate(const pfUpdatable *prev, int upId);
    virtual int	    pf_needsUpdate(void);
    virtual int	    pf_isBufferCloned(void);
    
private:
    // pfNode internal virtual functions
    friend class pfGroup;

    virtual int     pf_addParent(pfGroup *p);

PFINTERNAL:	
    // pfNode public virtual functions
    virtual int	    needsApp();
    virtual int	    needsASD();

PFINTERNAL:	
    // pfNode virtual traversals
    virtual void    nb_clean(uint64_t cleanBits, pfNodeCounts *counts);	 
    virtual pfNode* nb_clone() { return this; }
    virtual int     app(pfTraverser *trav);
    virtual int     ASDtrav(pfTraverser *trav);
    virtual int     nb_cull(int mode, int cullResult, _pfCuller *trav) = 0;
    virtual int     nb_cullProgram(int mode, int cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav) = 0;
    virtual int     nb_flatten(pfTraverser *trav);
    virtual int     nb_intersect(_pfIsector *itrav) = 0;
    //CAPI:virtual
    virtual void    nb_setTravMask(int which, uint mask, int setMode, int bitOp);
    //CAPI:virtual
    //CAPI:verb
    virtual pfNode* nb_find(const char *_name, pfType *_type);

    //CAPI:novirtual
    virtual int     nb_traverse(pfTraverser *trav);
    virtual void    nb_write(pfTraverser *trav, uint which, uint verbose);
    
PFINTERNAL:
    // public sets and gets
    int 	    nb_setName(const char *name);
    const char*	    nb_getName() const {
	return pfName;
    }
    void	    nb_setTravFuncs(int which, pfNodeTravFuncType pre, pfNodeTravFuncType post);
    
    void	    nb_getTravFuncs(int which, pfNodeTravFuncType *pre, pfNodeTravFuncType *post) const;
    
    void	    nb_setTravData(int which, void *data);
    
    void*	    nb_getTravData(int which) const;
    
    unsigned int    nb_getTravMask(int which) const {
	return travMasks[which];
    }

    void	    nb_setTravMode(int which, int mode, int val);
    int	    	    nb_getTravMode(int which, int mode) const;

    void	    nb_setBufferMode(int mode, int val);
    int		    nb_getBufferMode(int mode) const;

    //CAPI:verb GetParent
    pfGroup*	    nb_getParent(int i) const;
    //CAPI:verb GetNumParents
    int 	    nb_getNumParents() const {
	return parentList.getNum();
    }

    //CAPI:verb NodeBSphere
    void 	    nb_setBound(pfSphere *sph, int mode);
    //CAPI:verb GetNodeBSphere
    int 	    nb_getBound(pfSphere *sph);

PFINTERNAL:
    // other public functions
    //CAPI:verb
    static pfNode*  nb_lookup(const char *_name, pfType* _type);
    //CAPI:verb NodeIsectSegs
    int 	    nb_isect(pfSegSet *segSet, pfHit **hits[]);
    //CAPI:verb Flatten
    int		    nb_flatten(int _mode);
    //CAPI:verb	Clone
    pfNode*	    nb_clone(int _mode);
    //CAPI:verb	BufferClone
    //CAPI:noupdatable
    //CAPI:buffer
    pfNode*	    nb_bufferClone(int _mode, pfBuffer *buf);
    //CAPI:nobuffer
    

protected:	// data
    
    friend class pfASD;
    // Node name is shared amongst buffer copies
    char		*pfName;	
    
    _pfGroupList	parentList;
    
    pfSphere		bSphere;
    
    uint64_t 		nodeFlags;
    
    uint 		travMasks[4];
    
    _pfCallback		*callback;   // The callback structure is multibuffered.
public:
    void *_pfNodeExtraData;
private:
    static pfType *classType;
};


#endif // !__PF_NODE_H__
