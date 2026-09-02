/*
 * Copyright 1996, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * This source code ("Source Code") was originally derived from a
 * code base owned by Silicon Graphics, Inc. ("SGI")
 * 
 * LICENSE: SGI grants the user ("Licensee") permission to reproduce,
 * distribute, and create derivative works from this Source Code,
 * provided that: (1) the user reproduces this entire notice within
 * both source and binary format redistributions and any accompanying
 * materials such as documentation in printed or electronic format;
 * (2) the Source Code is not to be used, or ported or modified for
 * use, except in conjunction with OpenGL Performer; and (3) the
 * names of Silicon Graphics, Inc.  and SGI may not be used in any
 * advertising or publicity relating to the Source Code without the
 * prior written permission of SGI.  No further license or permission
 * may be inferred or deemed or construed to exist with regard to the
 * Source Code or the code base of which it forms a part. All rights
 * not expressly granted are reserved.
 * 
 * This Source Code is provided to Licensee AS IS, without any
 * warranty of any kind, either express, implied, or statutory,
 * including, but not limited to, any warranty that the Source Code
 * will conform to specifications, any implied warranties of
 * merchantability, fitness for a particular purpose, and freedom
 * from infringement, and any warranty that the documentation will
 * conform to the program, or any warranty that the Source Code will
 * be error free.
 * 
 * IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
 * LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
 * ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
 * SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
 * OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
 * PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
 * OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
 * USE, THE SOURCE CODE.
 * 
 * Contact information:  Silicon Graphics, Inc., 
 * 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
 * or:  http://www.sgi.com
*/

/*
 * file: clipcenter.c
 * ----------------
 *
 * Contains the functions
 *      pfuProcessClipCenters()
 *
 * which create a scene graph node with callbacks for
 * automatic clipmap centering.
 * See the comments above the functions for more details.
 *
 * $Revision: 1.27 $
 * $Date: 2002/05/15 00:31:09 $
 */

#include <stdlib.h>
#include <Performer/pfutil.h>
#include <math.h>
#ifndef WIN32
#include <values.h>
#endif

static void
pfuReplaceNodeEverywhere(pfNode *node, pfNode *replacement)
{
    int numParents;
    if (node == replacement)
	return;
    numParents = pfGetNumParents(node);
    while (numParents > 0)
	pfReplaceChild(pfGetParent(node, --numParents), node, replacement);
}


typedef struct {
    pfList *mpctlist;
    pfChannel *centerchan;
} _pfClipCenterNodeTrav;

static int
cbProcessClipCenterNodes(pfuTraverser *trav)
{
    pfuClipCenterNode *node = (pfuClipCenterNode *)trav->node;
    _pfClipCenterNodeTrav *data = (_pfClipCenterNodeTrav *)trav->data;
    pfList *mpcliptextures = data->mpctlist;
    pfChannel *chan = data->centerchan;

    if(node == NULL)
    {
        pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbFindClipCenterNode: null node\n");
        return PFTRAV_CONT;
    }

    if(pfIsOfType(node, pfuGetClipCenterNodeClassType()))
    {
        pfMPClipTexture *mpct;
	mpct = pfuGetClipCenterNodeMPClipTexture(node);
	if(mpct)
	    pfAdd(mpcliptextures, mpct);
	if(chan) /* set a channel pointer if one has been defined */
	    pfuClipCenterNodeChannel(node, chan);
    }
    return PFTRAV_CONT;
}


/*
** Find all clipcenter nodes, replace the cliptextures with mpcliptextures
** and add all discovered mpcliptextures to list. If _chan is not null, set
** the channel on all found clipcenter nodes (channel controls which channel
** can actually cause the clipcenter node to update its cliptexture)
*/

extern PFUDLLEXPORT void
pfuProcessClipCentersWithChannel(pfNode *_node, pfList *_mpcliptextures,
				 pfChannel *_chan)
{
  pfuTraverser trav;
  _pfClipCenterNodeTrav data;
  pfuInitTraverser(&trav);

  trav.preFunc = cbProcessClipCenterNodes;
  data.mpctlist = _mpcliptextures;
  data.centerchan = _chan;
  trav.data = (void *)&data;
  pfuTraverse(_node, &trav);

}

/*
** Same as above, but doesn't set the channel (NULL channel == all
** traversal set clipcenter)
*/

extern PFUDLLEXPORT void
pfuProcessClipCenters(pfNode *_node, pfList *_mpcliptextures)
{
  pfuTraverser trav;
  _pfClipCenterNodeTrav data;
  pfuInitTraverser(&trav);

  trav.preFunc = cbProcessClipCenterNodes;
  data.mpctlist = _mpcliptextures;
  data.centerchan = NULL;
  trav.data = (void *)&data;
  pfuTraverse(_node, &trav);

}
