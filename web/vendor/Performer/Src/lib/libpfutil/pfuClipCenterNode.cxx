/*
 * Copyright 2000, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that (i) the above copyright notices and this
 * permission notice appear in all copies of the software and related
 * documentation, and (ii) the name of Silicon Graphics may not be
 * used in any advertising or publicity relating to the software
 * without the specific, prior written permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL,
 * INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY
 * THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 */

//  pfuClipCenterNode.C
//
//  Clip centering node class 
// 

#include <Performer/pf.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfMPClipTexture.h>
#include <Performer/pfutil/pfuClipCenterNode.h>

/* PFUDLLEXPORT */ pfType *pfuClipCenterNode::classType = NULL;

// called after traversing children and after post-app func XXX 2nd part true?
static int
clipCenterPostApp(pfTraverser *trav)
{
    int travmode = PFUTRAV_SW_CUR | PFUTRAV_SEQ_CUR |
	           PFUTRAV_LOD_RANGE0 | PFUTRAV_LAYER_BOTH;

    pfVec3 eye;
    pfMatrix viewmat, travmat, invtravmat, mat;
    float x, y, z, s, t, r; /* vertex coords and tex coords of closest point */
    pfuClipCenterNode *ccn = (pfuClipCenterNode *)trav->getNode();
    pfMPClipTexture *mpcliptex = ccn->getMPClipTexture();
    pfClipTexture *cliptex = ccn->getClipTexture();
    pfNode *refnode = ccn->getRefNode();
    if(!refnode)
	refnode = ccn;
    
    //If a channel pointer is set, then only update the cliptexture center
    //if the specified channel is traversing the node
    if(ccn->getChannel() &&  ccn->getChannel() != trav->getChan())
	return PFTRAV_CONT;

    /*
     * Get the eye in the coordinate space of refnode...
     */
    eye[0] = eye[1] = eye[2] = 0;
    trav->getChan()->getViewMat(viewmat);
    trav->getMat(travmat);
    invtravmat.invertFull(travmat);
    PFCOPY_MAT(mat, viewmat);
    mat.postMult(invtravmat);
    eye.fullXformPt(eye, mat);

    if (mpcliptex && mpcliptex->getPipe() != NULL)
    {
	if (pfuGetClosestPoint(refnode,
			  eye[0], eye[1], eye[2],
			  (pfTexture *)cliptex,
			  travmode,
			  &x, &y, &z, &s, &t, &r))
	{
	    int width, height, depth, curS, curT, curR;
	    static int limit = -1;

	    cliptex->getVirtualSize(&width, &height, &depth);
#ifdef DEBUG
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
	    "%d: MP PostApp(%s): %g %g %g -> %d %d %d\n",
	    pfGetFrameCount(),
	    pfGetTexName((pfTexture*)cliptex),
	    s, t, r, (int)(s*width), (int)(t*height), (int)(r*depth));
#endif /* DEBUG */

	    curS = (int)(s * width);
	    curT = (int)(t * height);
	    curR = (int)(r * depth);

	    if (limit != -1)
	    {
		int prevS, prevT, prevR;
	
	        mpcliptex->getCenter(&prevS, &prevT, &prevR);
		curS = PF_CLAMP(curS, prevS - limit, prevS + limit);
		curT = PF_CLAMP(curT, prevT - limit, prevT + limit);
		curR = PF_CLAMP(curR, prevR - limit, prevR + limit);
	    }
	    mpcliptex->setCenter(curS, curT, curR);
	}
    }
    else /* not attached to a pipe! */
    {
	// XXX make more specific message (handle both cases separately
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfuClipCenterNode post-app callback called,");
	pfNotify(PFNFY_WARN, PFNFY_MORE,
		 "but clip texture not defined or not attached to a pipe.");
	pfNotify(PFNFY_WARN, PFNFY_MORE,
		 "Use pfuProcessClipCenters() and pfuAddMPClipTexturesToPipes().");
    }

    return PFTRAV_CONT;
}

// C bindings

PFUDLLEXPORT void /* static */
pfuInitClipCenterNodeClass(void)
{
    if(pfIsConfiged())
        pfNotify(PFNFY_DEBUG, PFNFY_USAGE, 
	"pfuInitClipCenterNodeClasse() should be called before pfConfig()"
	" for multiprocessed operation");
    pfuClipCenterNode::init();
}

PFUDLLEXPORT pfType * /* static */
pfuGetClipCenterNodeClassType(void)
{
    return pfuClipCenterNode::getClassType();
}

PFUDLLEXPORT pfuClipCenterNode *
pfuNewClipCenterNode(void)
{
    return new pfuClipCenterNode();
}

PFUDLLEXPORT void
pfuClipCenterNodeCallback(pfuClipCenterNode *_ccn,
			  pfuClipCenterPostAppCallbackType *_fun)
{
    _ccn->setCallback(_fun);
}

PFUDLLEXPORT pfuClipCenterPostAppCallbackType *
pfuClipCenterNodeCallback(pfuClipCenterNode *_ccn)
{
    return _ccn->getCallback();
}

PFUDLLEXPORT void
pfuClipCenterNodeRefNode(pfuClipCenterNode *_ccn, pfNode *_node)
{
    _ccn->setRefNode(_node);
}

PFUDLLEXPORT pfNode *
pfuGetClipCenterNodeRefNode(pfuClipCenterNode *_ccn)
{
    return _ccn->getRefNode();
}

PFUDLLEXPORT void
pfuClipCenterNodeClipTexture(pfuClipCenterNode *_ccn, pfClipTexture *_ct)
{
    _ccn->setClipTexture(_ct);
}

PFUDLLEXPORT pfClipTexture *
pfuGetClipCenterNodeClipTexture(pfuClipCenterNode *_ccn)
{
    return _ccn->getClipTexture();
}

PFUDLLEXPORT void
pfuClipCenterNodeMPClipTexture(pfuClipCenterNode *_ccn,
				  pfMPClipTexture *_mpct)
{
    _ccn->setMPClipTexture(_mpct);
}

PFUDLLEXPORT pfMPClipTexture *
pfuGetClipCenterNodeMPClipTexture(pfuClipCenterNode *_ccn)
{
    return _ccn->getMPClipTexture();
}

PFUDLLEXPORT void
pfuClipCenterNodeChannel(pfuClipCenterNode *_ccn, pfChannel *_chan)
{
    _ccn->setChannel(_chan);
}

PFUDLLEXPORT pfChannel *
pfuGetClipCenterNodeChannel(pfuClipCenterNode *_ccn)
{
    return _ccn->getChannel();
}


pfuClipCenterNode::pfuClipCenterNode()
{
    clipTex = NULL;
    mpClipTex = NULL;
    refNode = NULL; //default point to ourselves (== no simplfying geometry)
    callback = (pfuClipCenterPostAppCallbackType *)clipCenterPostApp;
    type = pfuClipCenterNode::getClassType();
    chan = NULL;
}

pfuClipCenterNode::~pfuClipCenterNode()
{
    if(refNode)
	pfUnrefDelete(refNode);
    if(mpClipTex)
	pfUnrefDelete(mpClipTex);
    if(clipTex)
	pfUnrefDelete(clipTex);
}

pfChannel *
pfuClipCenterNode::getChannel(void)
{
    return chan;
}

void
pfuClipCenterNode::setChannel(pfChannel *_chan)
{
    chan = _chan;
}

int
pfuClipCenterNode::needsApp(void)
{
    return 1; //this node should always be traversed
}

/* overrider pfGroup's  traverser so that we can call our post app callback */
int
pfuClipCenterNode::app(pfTraverser *trav)
{
    int state;
    state = pfGroup::app(trav);
    /* do our own post app callback */
    if (callback != NULL)
	callback(trav);

    return state;
}

void
pfuClipCenterNode::setMPClipTexture(pfMPClipTexture *_mpcliptex)
{
    if(_mpcliptex)
	pfRef(_mpcliptex); //ref first; in case re-setting the same object

    if(mpClipTex)
	pfUnrefDelete(mpClipTex);

    mpClipTex = _mpcliptex;

}

pfMPClipTexture *
pfuClipCenterNode::getMPClipTexture(void)
{
    //if it hasn't been set, and it hasn't been made before, promote
    //a cliptexture to an mpcliptexure, save and return it
    if(!mpClipTex && clipTex)
    {
	mpClipTex = new pfMPClipTexture();
	pfRef(mpClipTex);
	mpClipTex->setClipTexture(clipTex);
    }

    return mpClipTex;
}

void
pfuClipCenterNode::setClipTexture(pfClipTexture *_cliptex)
{
    if(_cliptex) //ref first in case we're re-setting the same object
	pfRef(_cliptex);

    if(clipTex)
	pfUnrefDelete(clipTex);

    clipTex = _cliptex;
}

void
pfuClipCenterNode::setRefNode(pfNode *_node)
{
    if(_node)
	pfRef(_node); //ref first in case we're re-setting the same object

    if(refNode)
	pfUnrefDelete(refNode);

    refNode = _node; 
}

void
pfuClipCenterNode::init()
{
    if(!classType)
    {
	pfGroup::init();
	classType = new pfType(pfGroup::getClassType(), "pfuClipCenterNode");
    }
}

//This is for dynamically changing callback at run time
//to create a different callback needed different args
// subclass node (see example!)
void
pfuClipCenterNode::setCallback(pfuClipCenterPostAppCallbackType *_fun)
{
    callback = _fun;
}


//*************************************************************************

//SUBCLASS EXAMPLE: pfuExampleClipCenterNode

class pfuExampleClipCenterNode : public pfuClipCenterNode
{
public:

    pfuExampleClipCenterNode();

    void setCenterDisplace(int _displaceS, int _displaceT);
    void getCenterDisplace(int *_displaceS, int *_displaceT) const;

private:

    int displaceS; // displacement from directly "below" node
    int displaceT; // displacement from directly "below" node
};


static int
exampleClipCenterPostApp(pfTraverser *trav)
{
    int dispS, dispT;
    // same stuff as clipCenterPostApp()...
    pfuExampleClipCenterNode *eccn = 
	(pfuExampleClipCenterNode *)trav->getNode();

    //If a channel pointer is set, then only update the cliptexture center
    //if the specified channel is traversing the node
    if(eccn->getChannel() &&  eccn->getChannel() != trav->getChan())
        return PFTRAV_CONT;

    eccn->getCenterDisplace(&dispS, &dispT);
    
    //use displacement to offset computed center

    return PFTRAV_CONT;
}

pfuExampleClipCenterNode::pfuExampleClipCenterNode()
{
    displaceS = 0;
    displaceT = 0;

    setCallback(exampleClipCenterPostApp);
}


void
pfuExampleClipCenterNode::setCenterDisplace(int _displaceS, int _displaceT)
{
    displaceS = _displaceS;
    displaceT = _displaceT;
}

void
pfuExampleClipCenterNode::getCenterDisplace(int *_displaceS, 
					    int *_displaceT) const
{
    if(_displaceS)
	*_displaceS = displaceS;
    if(_displaceT)
	*_displaceT = displaceT;
}

//*************************************************************************

//SUBCLASS  pfuTexGenClipCenterNode

static int
texGenClipCenterPostApp(pfTraverser *trav)
{
    pfVec3 eye;
    pfTexGen *tgen;
    pfMatrix viewmat, travmat, invtravmat, mat;
    pfuTexGenClipCenterNode *ccn = (pfuTexGenClipCenterNode *)trav->getNode();
    pfMPClipTexture *mpcliptex = ccn->getMPClipTexture();
    pfClipTexture *cliptex = ccn->getClipTexture();
    
    //If a channel pointer is set, then only update the cliptexture center
    //if the specified channel is traversing the node
    if(ccn->getChannel() &&  ccn->getChannel() != trav->getChan())
        return PFTRAV_CONT;

    tgen = ccn->getTexGen();

    if(!tgen)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, 
	  	"texGenClipCenter: texgen is not set");
	return PFTRAV_CONT;
    }

    /*
     * Get the eye in the coordinate space of refnode...
     */
    eye[0] = eye[1] = eye[2] = 0;
    trav->getChan()->getViewMat(viewmat);
    trav->getMat(travmat);
    invtravmat.invertFull(travmat);
    PFCOPY_MAT(mat, viewmat);
    mat.postMult(invtravmat);
    eye.fullXformPt(eye, mat);

    if (mpcliptex && mpcliptex->getPipe() != NULL)
    {
            /* get the texture matrix and normalize eye into texture space */
        {
            pfVec3          tCenter;
            pfMatrix        tMtx;

            tgen->getPlane( PF_S,
                            & tMtx[ 0 ][ PF_S ], & tMtx[ 1 ][ PF_S ],
                            & tMtx[ 2 ][ PF_S ], & tMtx[ 3 ][ PF_S ] );
            tgen->getPlane( PF_T,
                            & tMtx[ 0 ][ PF_T ], & tMtx[ 1 ][ PF_T ],
                            & tMtx[ 2 ][ PF_T ], & tMtx[ 3 ][ PF_T ] );
            tgen->getPlane( PF_R,
                            & tMtx[ 0 ][ PF_R ], & tMtx[ 1 ][ PF_R ],
                            & tMtx[ 2 ][ PF_R ], & tMtx[ 3 ][ PF_R ] );
/*	we do need Q in cliptexture --- hatch */
            tgen->getPlane( PF_Q,
                            & tMtx[ 0 ][ PF_Q ], & tMtx[ 1 ][ PF_Q ],
                            & tMtx[ 2 ][ PF_Q ], & tMtx[ 3 ][ PF_Q ] );

	    tCenter.fullXformPt(eye, tMtx);

	    tCenter[0] = PF_CLAMP(tCenter[0], 0.0, 1.0);
	    tCenter[1] = PF_CLAMP(tCenter[1], 0.0, 1.0);

            /* special case because "terrain" is always in "X-Y" plane */
            /* scale normalized texture space center into virtual space */
            {
                int depth, height, width;

                cliptex->getVirtualSize( & width, & height, & depth );
                mpcliptex->setCenter( ( int ) ( tCenter[ 0 ] * width ),
                                    ( int ) ( tCenter[ 1 ] * height ), 0 );
            }
        }
    }
    else /* not attached to a pipe! */
    {
        // XXX make more specific message (handle both cases separately
        pfNotify(PFNFY_WARN, PFNFY_USAGE,
                 "pfuTexGenClipCenterNode post-app callback called,");
        pfNotify(PFNFY_WARN, PFNFY_MORE,
                 "but clip texture not defined or not attached to a pipe.");
        pfNotify(PFNFY_WARN, PFNFY_MORE,
                 "Use pfuProcessClipCenters() and pfuAddMPClipTexturesToPipes().");
    }

    return PFTRAV_CONT;
}

pfuTexGenClipCenterNode::pfuTexGenClipCenterNode()
{
    tgen = NULL;
    setCallback(texGenClipCenterPostApp);
}


pfuTexGenClipCenterNode::~pfuTexGenClipCenterNode()
{
    if(tgen)
        pfUnrefDelete(tgen);
    #ifndef __linux__
    pfuClipCenterNode::~pfuClipCenterNode();
    #else
    pfNotify(PFNFY_WARN, PFNFY_PRINT,
	"Linux: apparently bogus destruct in [%s:%d]", __FILE__, __LINE__);
    #endif  /* __linux__ */
}

void
pfuTexGenClipCenterNode::setTexGen(pfTexGen *_tgen)
{
    tgen = _tgen;
    pfRef(tgen);
}

pfTexGen *
pfuTexGenClipCenterNode::getTexGen(void)
{
    return tgen;
}

/* the C-API wrappers */
PFUDLLEXPORT pfuTexGenClipCenterNode *
pfuNewTexGenClipCenterNode(void)
{
    return new pfuTexGenClipCenterNode();
}

PFUDLLEXPORT void
pfuTexGenClipCenterNodeTexGen(pfuTexGenClipCenterNode *_ccn, pfTexGen *_tgen)
{
    _ccn->setTexGen(_tgen);
}

PFUDLLEXPORT pfTexGen *
pfuGetTexGenClipCenterNodeTexGen(pfuTexGenClipCenterNode *_ccn)
{
    return _ccn->getTexGen();
}
