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

#ifndef _PFU_CLIP_CENTER_NODE_H_
#define _PFU_CLIP_CENTER_NODE_H_

/*
// External C api...
*/

#include <Performer/pfutil-DLL.h>

#ifdef __cplusplus
extern "C" {
#endif
extern PFUDLLEXPORT void pfuInitClipCenterNodeClass(void);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pf/pfGroup.h>



//---------------------------------------------------
// pfuClipCenterNode
//  PURPOSE:
//  This is the node used to find update the center of 
//  a pfMPClipTexture. It uses the geometry and the
//  viewpoint to compute the proper center.
//---------------------------------------------------


class PFUDLLEXPORT pfuClipCenterNode : public pfGroup
{	

public:
    pfuClipCenterNode();
    ~pfuClipCenterNode();
    static void init(); //initialize the class type
    static pfType* getClassType() { return classType; }
    virtual const char* getTypeName(void) const { return "pfuClipCenterNode"; }

    // set or get the centering callback
    void setCallback(pfuClipCenterPostAppCallbackType *cb);
    pfuClipCenterPostAppCallbackType  *getCallback(void) const {return callback; }

    // get and set reference node
    void    setRefNode(pfNode *_node);
    pfNode *getRefNode(void) const {return refNode; }

    // get and set cliptexture center channel
    void      setChannel(pfChannel *_channel);
    pfChannel *getChannel(void);
    
    //make it the first time called
    pfMPClipTexture *getMPClipTexture(void);
    void setMPClipTexture(pfMPClipTexture *_mpcliptex);
 
    pfClipTexture *getClipTexture(void) const {return clipTex; }
    void setClipTexture(pfClipTexture *_cliptex);
    /*pfGroup's*/virtual int app(pfTraverser *_trav);
    /*pfNode's*/virtual int needsApp(void);

private:
    static pfType *classType;
    pfuClipCenterPostAppCallbackType *callback;
    pfNode *refNode; //simplified geometry;
    pfClipTexture *clipTex;
    pfMPClipTexture *mpClipTex;
    pfChannel *chan;
};


//SUBCLASS EXAMPLE: pfuExampleClipCenterNode

class PFUDLLEXPORT pfuTexGenClipCenterNode : public pfuClipCenterNode
{
public:

    pfuTexGenClipCenterNode();
    ~pfuTexGenClipCenterNode();

    void setTexGen(pfTexGen *_tgen);
    pfTexGen *getTexGen();

private:

    pfTexGen *tgen;
};


#endif /* __cplusplus */
#endif /* _PFU_CLIP_CENTER_NODE_H_ */

