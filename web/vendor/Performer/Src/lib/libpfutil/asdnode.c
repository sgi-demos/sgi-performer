/*
 * Copyright 1996, Silicon Graphics, Inc.
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
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
*/

/*
 * file: asdnode.c
 * ----------------
 *
 * Contains the functions
 *      pfuFindASDs()
 *
 * which returns a list of pfASD nodes in the scene graph.
 *
 * $Revision: 1.3 $
 * $Date: 2002/05/15 00:31:09 $
 */

#include <stdlib.h>
#include <Performer/pfutil.h>
#include <math.h>
#ifndef WIN32
#include <values.h>
#endif

typedef struct {
    pfList *asdlist;
} _pfASDTrav;


static int
cbFindASDNodes(pfuTraverser *trav)
{
    pfNode *node = (pfNode*)trav->node;
    pfList *asdlist = ((_pfASDTrav *)trav->data)->asdlist;

    if(pfIsOfType(node, pfGetASDClassType()))
    {
	pfAdd(asdlist, node);
    }
    return PFTRAV_CONT;
}

/*
** Find all pfASD nodes
*/

extern PFUDLLEXPORT void
pfuFindASDs(pfNode *_node, pfList *_ASDs)
{
    pfuTraverser trav;
    _pfASDTrav data;
    pfuInitTraverser(&trav);

    trav.preFunc = cbFindASDNodes;
    data.asdlist = _ASDs;
    trav.data = (void *)&data;
    pfuTraverse(_node, &trav);
}



