/* 
 * Copyright 1992, 1993, 1994, 1995, Silicon Graphics, Inc.
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

//
// pfiv.C: 
// $Revision: 1.1 $
// $Date: 2000/11/21 21:39:33 $
//

#include <sys/types.h>
#include <malloc.h>
#include "pfivdefs.h"
#include "Rotor.h"
#include "Pendulum.h"
#include "Shuttle.h"
#include <Performer/pfdb/pfiv.h>

// default IV revision, 2.1 is the first to define these
#ifndef SO_VERSION
#define SO_VERSION 2
#define SO_VERSION_REVISION 0
#endif

#if SO_VERSION != 2
#error
#endif

#define TOP_STACK(c)  ((*(c)->parentStack)[(c)->parentStack->getLength()-1])

#define POP_STACK(c) do {	\
   (c)->parentStack->remove((c)->parentStack->getLength()-1);  \
   (c)->parent = (pfGroup*)TOP_STACK(c);		       \
} while (0);

static pfNode *	loadIv(char *file);

static int DoFlatten = 1;
static int DoClean = 1;
static int ConvertXforms = 0;
static int ConvertLights = 0;
static int ConvertTwoSide = 0;

// #define DEBUG_MESSAGES

//--------------------------------------------------------------------------

void
pfdInitConverter_iv(void)
{
    Rotor::init();
    Pendulum::init();
    Shuttle::init();
}

void
pfdConverterMode_iv(int mode, int val)
{
    switch (mode) {
    case PFIV_FLATTEN:
	DoFlatten = val;
	break;
    case PFIV_CLEAN:
	DoClean = val;
	break;
    case PFIV_CONVERT_XFORMS:
	ConvertXforms = val;
	break;
    case PFIV_CONVERT_LIGHTS:
	ConvertLights = val;
	break;
    case PFIV_CONVERT_TWOSIDE:
	ConvertTwoSide = val;
	break;
    default:
 	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfdConverterMode_iv: Unknown mode %d", mode);
    }
}

int
pfdGetConverterMode_iv(int mode)
{
    switch (mode) {
    case PFIV_FLATTEN:
	return DoFlatten;
    case PFIV_CLEAN:
	return DoClean;
    case PFIV_CONVERT_XFORMS:
	return ConvertXforms;
    case PFIV_CONVERT_LIGHTS:
	return ConvertLights;
    case PFIV_CONVERT_TWOSIDE:
	return ConvertTwoSide;
    default:
 	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfdGetConverterMode_iv: Unknown mode %d", mode);
	return -1;
    }
}

//
// pfdLoadFile_iv -- Load SGI ".iv" files into IRIS Performer
//

//
// For Inventor 2.0 loader, provide pfdLoadFile_iv20 entry point in addition to the 
// default pfdLoadFile_iv since the DSO will be named libpf_iv20.
//

#ifndef __linux__
#if SO_VERSION_REVISION == 0

extern "C" {
extern pfNode* pfdLoadFile_iv20 (char *fileName);
}
extern pfNode* pfdLoadFile_iv20 (char *fileName)
{
    return pfdLoadFile_iv(fileName);
}
#endif
#else /* linux */
extern "C" {
extern pfNode* pfdLoadFile_iv20 (char *fileName);
}
extern pfNode* pfdLoadFile_iv20 (char *fileName)
{
    return pfdLoadFile_iv(fileName);
}
#endif

extern pfNode*
pfdLoadFile_iv (char *fileName)
{
    pfNode	*sceneGraph	= NULL;
    char	 filePath[PF_MAXSTRING];
    
    // restore builder to initial state
    pfdResetBldrGeometry();
    pfdResetBldrState();

    // check file name for Null-pointer or Null-string
    if (fileName == NULL || *fileName == '\0')
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadFile_iv: Null file name provided as input");
	return NULL;
    }
    
    // find file in IRIS Performer search path
    if (!pfFindFile(fileName, filePath, R_OK))
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadFile_iv: Could not find file \"%s\" in PFPATH", fileName);
	return NULL;
    }
    
    // print file name
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_iv: %s", fileName);
    
    // import file into IRIS Performer scene graph
    sceneGraph = loadIv(filePath);
    
    // check for failure of file importation
    if (sceneGraph == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadFile_iv: Empty scene graph created from file \"%s\".  Load error?", filePath);
	return NULL;
    }
    
    // return root of new scene graph to caller
    return sceneGraph;
}

//////////////////////////////////////////////////////////////
//
// GroupLevelOfDetail - SoLevelOfDetail subclassed to get
// group traversal actions
//
/////////////////////////////////////////////////////////////
SO_NODE_SOURCE(GroupLevelOfDetail);

//
// Initializes the LOD class. This is a one-time thing that is
// done after database initialization and before any instance of
// this class is constructed.
//

void
GroupLevelOfDetail::initClass()
{
    // These calls tell Inventor to override its level of detail node
    // with this node.
    classTypeId = SoType::overrideType(SoLevelOfDetail::getClassTypeId(),
                                       createInstance);
    parentFieldData = SoLevelOfDetail::getFieldDataPtr();
}

//
// Constructor
//

GroupLevelOfDetail::GroupLevelOfDetail()
{
    SO_NODE_CONSTRUCTOR(GroupLevelOfDetail);
}

//
// Destructor
//

GroupLevelOfDetail::~GroupLevelOfDetail()
{
}

//
// Implements callback action.
//

void
GroupLevelOfDetail::callback(SoCallbackAction *action)
{
    // use the group nodes action, which will traverse all children.
    SoGroup::doAction(action);
}


#if SO_VERSION_REVISION >= 1
//////////////////////////////////////////////////////////////
//
// GroupLOD - SoLOD subclassed to get
// group traversal actions
//
/////////////////////////////////////////////////////////////
SO_NODE_SOURCE(GroupLOD);

//
// Initializes the LOD class. This is a one-time thing that is
// done after database initialization and before any instance of
// this class is constructed.
//

void
GroupLOD::initClass()
{
    // These calls tell Inventor to override its level of detail node
    // with this node.
    classTypeId = SoType::overrideType(SoLOD::getClassTypeId(),
                                       createInstance);
    parentFieldData = SoLOD::getFieldDataPtr();
}

//
// Constructor
//

GroupLOD::GroupLOD()
{
    SO_NODE_CONSTRUCTOR(GroupLOD);
}

//
// Destructor
//

GroupLOD::~GroupLOD()
{
}

//
// Implements callback action.
//

void
GroupLOD::callback(SoCallbackAction *action)
{
    // use the group nodes action, which will traverse all children.
    SoGroup::doAction(action);
}
#endif

//////////////////////////////////////////////////////////////
//
// GroupBlinker - SoBlinker subclassed to get
// group traversal actions
//
/////////////////////////////////////////////////////////////

SO_NODE_SOURCE(GroupBlinker);

void
GroupBlinker::initClass()
{
    classTypeId = SoType::overrideType(SoBlinker::getClassTypeId(),
                                       createInstance);
    parentFieldData = SoBlinker::getFieldDataPtr();
}

GroupBlinker::GroupBlinker()
{
    SO_NODE_CONSTRUCTOR(GroupBlinker);
}

GroupBlinker::~GroupBlinker()
{
}

void
GroupBlinker::callback(SoCallbackAction *action)
{
    // use the group nodes action, which will traverse all children.
    SoGroup::doAction(action);
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//   Build an IRIS Performer pfGroup from an Inventor format file
//
// Use: private
//
////////////////////////////////////////////////////////////////////////

static pfNode *
loadIv(char *file)
{
    pfNotify(PFNFY_INFO, PFNFY_MORE, "  Status:");
   
    // Initialize Inventor (database + interaction + node kits)
    pfNotify(PFNFY_INFO, PFNFY_MORE,  "    Initializing OpenInventor");
    SoInteraction::init();
    GroupLevelOfDetail::initClass();
#if SO_VERSION_REVISION >= 1
    GroupLOD::initClass();
#endif
    GroupBlinker::initClass();

    // open file
    SoNode	*node;
    SoInput     in;
    SbBool      ok = TRUE;
   
    pfNotify(PFNFY_INFO, PFNFY_MORE,  "    Opening file \"%s\"", file);
    // open file
    if (!in.openFile(file))
    {
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadFile_iv: Could not open \"%s\"", file);
	return NULL;
    }
   
    SoSeparator *root = new SoSeparator;
   
    // load file
    pfNotify(PFNFY_INFO, PFNFY_MORE,  "    Loading file into OpenInventor");
    root->ref();
    while (TRUE)
    {
        ok = SoDB::read(&in, node) && ok;
	
        if (node != NULL)
            root->addChild(node);
        else
            break;
    }
   
    // close file
    in.closeFile();
   
    // build IRIS Performer version of file's data
    pfNotify(PFNFY_INFO, PFNFY_MORE,  "    Converting scene graph to Performer");
    pfNode *result = (pfNode *)pfdConvertFrom_iv(root);
   
    // Delete Inventor scene graph
    pfNotify(PFNFY_INFO, PFNFY_MORE,  "    Deleting OpenInventor scene graph");
    root->unref();
   
    // Optimize converted scene graph
    pfNotify(PFNFY_INFO, PFNFY_MORE,  "    Optimizing Performer scene graph");


    if (DoFlatten)
    {
	result = pfdFreezeTransforms(result, NULL);
	result->flatten(0);
    }
   
    if (DoClean)
	result = pfdCleanTree(result, NULL);
   
    // remember file-name as top-level node's name
    if (result != NULL)
	result->setName(file);
   
    return result;
}

static int reverseOrder = 0;
static int reverseNormal = 0;

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Load a vertex into Performer structures.
//
////////////////////////////////////////////////////////////////////////

static void
loadVertex(CbData *cbd, const SoPrimitiveVertex *v, int index)
{
    const SbVec3f 	&pt   = v->getPoint();
    const SbVec3f 	&norm = v->getNormal();
    const SbVec4f 	&tc   = v->getTextureCoords();
    SbColor       	ambient, diffuse, specular, emission;
    float         	transparency, shininess;
    pfdGeom		*pb;
   
    pb = cbd->geom;
   
    // Make sure Geom is big enough
    if (cbd->numVerts >= cbd->pbSize)
    {
	cbd->pbSize *= 2;
	pfdResizeGeom(pb, cbd->pbSize);
    }
   
    // Load the vertex coordinates
    pb->coords[cbd->numVerts].set(pt[0], pt[1], pt[2]);
   
    // Load the vertex normals
    if (cbd->normBind == PFGS_PER_VERTEX)
    {
	// Reverse normal if primitive is clockwise-oriented
	if (reverseNormal)
	    pb->norms[cbd->numVerts].set(-norm[0], -norm[1], -norm[2]);
	else
	    pb->norms[cbd->numVerts].set( norm[0],  norm[1],  norm[2]);
    }
    else
	if (cbd->normBind == PFGS_PER_PRIM && index == 0)
	{
	    // Reverse normal if primitive is clockwise-oriented
	    if (reverseNormal)
		pb->norms[cbd->numPrims].set(-norm[0], -norm[1], -norm[2]);
	    else
		pb->norms[cbd->numPrims].set( norm[0],  norm[1],  norm[2]);
	}
   
    // Load the vertex texture coordinates
    if (cbd->doTextures)
	pb->texCoords[0][cbd->numVerts].set(tc[0], tc[1]);
   
    // Load the material diffuse color into the color array since
    // Performer uses colormode for performance. OVERALL is taken
    // care of in initializeNewShape
    if (cbd->colorBind == PFGS_PER_VERTEX ||
	cbd->colorBind == PFGS_PER_PRIM)
    {
        cbd->action->getMaterial(ambient, diffuse, specular, emission,
				 shininess, transparency, v->getMaterialIndex());
	
        if (cbd->colorBind == PFGS_PER_VERTEX)
	    pb->colors[cbd->numVerts].set(diffuse[0], diffuse[1], diffuse[2], 
					  1.0f - transparency);
        else if (cbd->colorBind == PFGS_PER_PRIM && index == 0)
	    pb->colors[cbd->numPrims].set(diffuse[0], diffuse[1], diffuse[2], 
					  1.0f - transparency);
    }
   
    // Advance primitive's vertex count
    cbd->numVerts++;
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Callback for receiving triangle primitives.
//
////////////////////////////////////////////////////////////////////////

static SoCallbackAction::Response
finishNewShape(void *data, SoCallbackAction *act, const SoNode *node);
static SoCallbackAction::Response
initializeNewShape(void *data, SoCallbackAction *act, const SoNode *node);

static void
triangleCB(void *data, SoCallbackAction *act,
	   const SoPrimitiveVertex *v0,
	   const SoPrimitiveVertex *v1,
	   const SoPrimitiveVertex *v2)
{
    CbData *cbd = (CbData *)data;
   
    // Reverse ordering of clockwise-oriented triangle
    if (reverseOrder)
    {
	loadVertex(cbd, v0, 0);
	loadVertex(cbd, v2, 1);
	loadVertex(cbd, v1, 2);
    }
    else
    {
	loadVertex(cbd, v0, 0);
	loadVertex(cbd, v1, 1);
	loadVertex(cbd, v2, 2);
    }
   
    cbd->primtype = MYTRIS;
    cbd->numPrims++;

    // XXX - workaround for large databases, 
    // break large shape before we run out of memory
    if (cbd->numPrims > 256*256)
    {
	finishNewShape(cbd, act, cbd->node);
	initializeNewShape(cbd, act, cbd->node);
    }
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Callback for receiving triangle primitives.
//
////////////////////////////////////////////////////////////////////////

static void
lineSegmentCB(void *data, SoCallbackAction *,
	      const SoPrimitiveVertex *v0,
	      const SoPrimitiveVertex *v1)
{
    CbData *cbd = (CbData *)data;
   
    loadVertex(cbd, v0, 0);
    loadVertex(cbd, v1, 1);
   
    cbd->primtype = MYLINES;
    cbd->numPrims++;
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Callback for receiving point primitives.
//
////////////////////////////////////////////////////////////////////////

static void
pointCB(void *data, SoCallbackAction *,
	const SoPrimitiveVertex *v0)
{
    CbData *cbd = (CbData *)data;
   
    loadVertex(cbd, v0, 0);
   
    cbd->primtype = MYPOINTS;
    cbd->numPrims++;
}

static void
pushGroup(CbData *cbd, pfGroup *group)
{
    cbd->parent->addChild(group);
   
    // Push the group node onto the parentStack
    cbd->parentStack->append((void *)group);
    cbd->parent = (pfGroup *)group;
}


static void
xformLight(SoCallbackAction *cba, const SbVec3f &v, pfVec3& vt, int isVec)
{
    pfMatrix 	mat;
   
    // Figure out light transformation
    if (!ConvertXforms)
	mat.copy(*(pfMatrix*)cba->getModelMatrix().getValue());
    else
	mat.makeIdent();
   
    // Add in Inventor->Performer coordinate system transform
    // since pfFlatten() does not affect pfLights
    if (DoFlatten)
	mat.postRot(mat, 90.0f, 1, 0, 0);
   
    // Transform vector
    vt.set(v[0], v[1], v[2]);
    if (isVec)
	vt.xformVec(vt, mat);
    else
	vt.xformPt(vt, mat);
}


extern "C" {
int enableSphereMap(pfTraverser *, void *);
int disableSphereMap(pfTraverser *, void *);
}

int
enableSphereMap(pfTraverser *, void *)
{
    glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
    glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    return PFTRAV_CONT;
}

int
disableSphereMap(pfTraverser *, void *)
{
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    return PFTRAV_CONT;
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//   Translate the given node into the Performer database
//
// Use: private
//
////////////////////////////////////////////////////////////////////////

struct UserData
{
    SoNode* node;
    int	   switchVal;
};

static pfSCS*
addXform(CbData *cbd, SoCallbackAction *, const SoNode *node, pfDCS* dcs)
{
    const char *name = node->getName().getString();
    
    // create and initialize a pfDCS node
    pfSCS *scs;
    if (dcs == NULL)
	scs = new pfSCS(cbd->stateStack[cbd->stateDepth].currentXform);
    else
    {
	scs = (pfSCS*)dcs;
	dcs->setMat(cbd->stateStack[cbd->stateDepth].currentXform);
    }

    if (strlen(name) > 0)
	scs->setName(name);
    
    // Now push dcs onto parent stack
    pushGroup(cbd, (pfGroup*)scs);
    
    cbd->mstack->push();
    cbd->invmstack->push();
    cbd->mstack->preMult(cbd->stateStack[cbd->stateDepth].currentXform);
    cbd->invmstack->getTop()->invertFull(*cbd->mstack->getTop());
    
#ifdef DEBUG_MESSAGES
    pfNotify(PFNFY_DEBUG, PFNFY_MORE,
	"addXform() SoNode 0x%x, pfXCS 0x%x, matDepth %d parentdepth %d",
	    node, 
	    scs, 
	    pfGetMStackDepth(cbd->mstack), 
	    cbd->parentStack->getLength() - 1);
    ((SbMatrix)(cbd->stateStack[cbd->stateDepth].currentXform)).print(stdout);
#endif

    cbd->stateStack[cbd->stateDepth].currentXform.makeIdent();
    cbd->stateStack[cbd->stateDepth].xformDirty = 0;

    return scs;
}

static void
preGroup(CbData *cbd, SoCallbackAction *, const SoNode *node)
{
    pfGroup		*grp;
    
    // Push state on stack when encounter separator
    if (node->isOfType(SoSeparator::getClassTypeId()))
    {
	if (cbd->stateDepth < PFIV_MAX_STACK-1)
	{
	    memcpy(&cbd->stateStack[cbd->stateDepth+1],
		   &cbd->stateStack[cbd->stateDepth],
		   sizeof(pfivState));
	    cbd->stateDepth++;
	}
	else
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		     "pfLoad_iv() Exceeded state stack.");
    }
    
    // User data for pfNode to establish correspondence with SoNode
    UserData *ud = (UserData*) pfMalloc(sizeof(UserData)+10,
					cbd->arena);
    ud->node = (SoNode*) node;
    ud->switchVal = SO_SWITCH_NONE;
   
    if ((node->getTypeId() == SoSeparator::getClassTypeId()) ||
	(node->getTypeId() == SoTransformSeparator::getClassTypeId()) ||
	(node->getTypeId() == SoGroup::getClassTypeId()) ||
	(node->getTypeId() == SoArray::getClassTypeId()) ||
	(node->getTypeId() == SoMultipleCopy::getClassTypeId()))
    {
	grp = new pfGroup;
    }
    else if (node->getTypeId() == SoSwitch::getClassTypeId())
    {
	pfSwitch 	*sw;
	int 	i;
	
	i = ((SoSwitch *)node)->whichChild.getValue();
	
	if (i == SO_SWITCH_INHERIT)
	{
	    // If switch is inherited, then treat as group
	    grp = new pfGroup;
	    ud->switchVal = i;
	}
	else
	{
	    sw = new pfSwitch;
	    grp = (pfGroup*)sw;
	    // save the switch value for later use
	    ud->switchVal = i;
	}
	// This doesn't work - it appears that the callback action 
	// can't handle changes to the scene graph. I get a core dump in
	// >  0 SoAuditorList::getType(long) 
	//    const(0x1040fdb0, 0x0, 0x0, 0x7c0000a4)
	// ["SoAuditorList.c++":138, 0x5c8195f8]
	
	// Insert Separator for all children which are not enabled
	// by switch so their state is not inherited.
	if (i != SO_SWITCH_ALL)
	{
	    int	j;
	    for (j=0; j<((SoSwitch *)node)->getNumChildren(); j++)
	    {
		if (i == j)
		    continue;
		
		SoSeparator *sep = new SoSeparator;
		SoNode	*child;
		
		child = ((SoSwitch *)node)->getChild(j);
		child->ref(); 	// So replaceChild() doesn't nuke it.
		((SoSwitch *)node)->replaceChild(j, sep);
		sep->addChild(child);
	    }
	}

	// Force SoSwitch to traverse all its children
	((SoSwitch *)node)->whichChild = SO_SWITCH_ALL;
    }
    else if (node->getTypeId() == SoBlinker::getClassTypeId())
    {
	grp = (pfGroup*)new pfSequence;
    }
    else if (node->getTypeId() == GroupLevelOfDetail::getClassTypeId())
    {
	float 	x, y, z, tmp;
	float 	range;
	int 	num, i;
	pfLOD 	*pflod = new pfLOD;
	pfVec3 	newcenter;
	const SoLevelOfDetail *lod = (const SoLevelOfDetail *)node;
	
	// Set LOD center to bounding box center
	SoGetBoundingBoxAction *bba = new SoGetBoundingBoxAction(
							SbViewportRegion());
	bba->apply((SoNode *)node);
	SbXfBox3f &bbox = bba->getXfBoundingBox();
	const SbVec3f &center = bba->getCenter();
	newcenter.set(center[0], center[1], center[2]);
	pflod->setCenter(newcenter);
	
	// Get maximum projected area?
	bbox.getSize(x, y, z);
	if (x >= y)
	{
	    if (y >= z)
		tmp = x*y;
	    else
		tmp = x*z;
	}
	else
	{
	    if (x >= z)
		tmp = x*y;
	    else
		tmp = y*z;
	}
	delete bba; // careful not to use bbox or center after this deletion
	
	// Convert projected screen area to range
	tmp = 1024.0f*sqrt(tmp);
	const float *screenArea = lod->screenArea.getValues(0);
	num = lod->screenArea.getNum();
	range = 0.0f;
	pflod->setRange(0, range);
	for (i = 0; i < num; i++)
	{
	    range = tmp / sqrt(screenArea[i]);
	    pflod->setRange(i+1, range);
	}
	
	grp = (pfGroup*)pflod;
    }
#if SO_VERSION_REVISION >= 1
    else if (node->getTypeId() == GroupLOD::getClassTypeId())
    {
	int 	num, i;
	pfLOD 	*pflod = new pfLOD;
	pfVec3 	newcenter;

	const SoLOD *lod = (const SoLOD *)node;

	const SbVec3f &center = lod->center.getValue();
	newcenter.set(center[0], center[1], center[2]);
	pflod->setCenter(newcenter);
	num = lod->getChildren()->getLength();
	
	pflod->setRange(0, 0);
	for (i = 0; i < num; i++)
	{
	    float range = lod->range[i];
	    pflod->setRange(i+1, range);
	}
	pflod->setRange(i, 1.0e10f);
	
	grp = (pfGroup*)pflod;
    }
#endif
    else
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfLoad_iv() Unsupported SoGroup type \"%s\". "
		 "Treating as SoGroup.\n",
		 node->getTypeId().getName().getString());
	
	grp = new pfGroup;
    }
    
    const char *name = node->getName().getString();
    if (strlen(name) > 0)
	grp->setName(name);
    
    // Set user data on pfNode to SoNode to establish correspondence
    grp->setUserData(ud); 
    
    // Now push group onto parent stack
    pushGroup(cbd, grp);
    
#ifdef DEBUG_MESSAGES
    if ((node->getTypeId() == SoSeparator::getClassTypeId()) ||
	(node->getTypeId() == SoTransformSeparator::getClassTypeId()))
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, "preSeparator");
    else
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, "preGroup");

    pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
	"() SoNode 0x%x, pfGroup 0x%x, depths(%d, %d)",
	    node, 
	    grp, 
	    pfGetMStackDepth(cbd->mstack), 
	    cbd->parentStack->getLength() - 1);
#endif
}

//
// ----------------------- SoTransform --------------------------
//
static void
preTransform(CbData *cbd, SoCallbackAction *cba, const SoNode *node)
{
    // The next set of cases are transform nodes of some type.
    // Concatenate transform to current one but don't create
    // a pfDCS until necessary.
    //
    if ((node->getTypeId() == SoRotation::getClassTypeId())    ||
	(node->getTypeId() == SoRotationXYZ::getClassTypeId()) ||
	(node->getTypeId() == SoScale::getClassTypeId())       ||
	(node->getTypeId() == SoTransform::getClassTypeId())   ||
	(node->getTypeId() == SoTranslation::getClassTypeId()) ||
	(node->getTypeId() == SoUnits::getClassTypeId()) ||
	(node->getTypeId() == SoMatrixTransform::getClassTypeId()))
    {
	cbd->getMatrixAction->apply((SoNode *)node);
#ifdef DEBUG_MESSAGES
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
	    "PRETRANSFORM: (%d %d, %d, cbd->parentStack.getLength()) cur", 
		cbd->stateDepth, 
		cbd->parentStack->getLength()-1);
	cbd->getMatrixAction->getMatrix().print(stdout);
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
	    "PRETRANSFORM: (%d, %d) concat", 
		cbd->stateDepth, 
		cbd->parentStack->getLength()-1);
	cba->getModelMatrix().print(stdout);
#endif
	cbd->stateStack[cbd->stateDepth].currentXform.
	    preMult(*(pfMatrix*)cbd->getMatrixAction->getMatrix().getValue());
	
	cbd->stateStack[cbd->stateDepth].xformDirty = 1;

	const char *name = node->getName().getString();
	// Use DCS if ConvertXforms is TRUE or if node name has
	// the string "dcs" in it.
	if (ConvertXforms ||
	    (name && (strstr(name, "dcs") || strstr(name, "DCS"))))
	{
	    addXform(cbd, cba, node, new pfDCS);
	}
    }
    else if (node->getTypeId() == SoResetTransform::getClassTypeId())
    {
	cbd->stateStack[cbd->stateDepth].currentXform.makeIdent();
	cbd->stateStack[cbd->stateDepth].xformDirty = 0;
	
	// Try to pop back into world space
	while(pfIsOfType(TOP_STACK(cbd), pfSCS::getClassType()))
	{
#ifdef DEBUG_MESSAGES
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
		     "popDCS() SoNode 0x%x, pfDCS 0x%x, matDepth %d parentdepth %d",
		    node, 
		    TOP_STACK(cbd), 
		    pfGetMStackDepth(cbd->mstack), 
		    cbd->parentStack->getLength() - 1);
#endif
	    // Pop matrix stack since we're popping a transform node
	    cbd->mstack->pop();
	    cbd->invmstack->pop();
	    
	    POP_STACK(cbd);
	}
	
	if (!cbd->mstack->getTop()->almostEqual(pfIdentMat, .0001f))
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		     "pfLoad_iv() Cannot handle SoResetTransform 0x%x with "
		     "name \"%s\". ", node,
		     (node->getName().getString() == NULL) ? "<noname>" :
		     node->getName().getString());
    }
    else if (node->getTypeId() == SoRotor::getClassTypeId())
    {
	// insert a DCS or SCS above the animated xform
	if (cbd->stateStack[cbd->stateDepth].xformDirty)
	    addXform(cbd, cba, node, ConvertXforms?(new pfDCS):NULL);

	// get the current state of this node, so that our MatStack
	// jibes with inventor's notion of a concatenated xform
	cbd->getMatrixAction->apply((SoNode *)node);
	cbd->stateStack[cbd->stateDepth].currentXform.
	    preMult(*(pfMatrix*)cbd->getMatrixAction->getMatrix().getValue());
	cbd->stateStack[cbd->stateDepth].xformDirty = 1;

#ifdef DEBUG_MESSAGES
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
	    "PREROTOR: depths (%d, %d)", 
		cbd->stateDepth, 
		cbd->parentStack->getLength()-1);
	cbd->getMatrixAction->getMatrix().print(stdout);
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
	    "PREROTOR: depths (%d, %d) concat", 
		cbd->stateDepth, 
		cbd->parentStack->getLength()-1);
	cba->getModelMatrix().print(stdout);
#endif

	SoRotor *soRot = (SoRotor*)node;
	pfVec3 axis;
	float angle;

	Rotor *rotor = (Rotor*)addXform(cbd, cba, node, new Rotor);
	
	soRot->rotation.getValue(*(SbVec3f*)&axis, angle);
	if (soRot->on.getValue())
	    rotor->on();
	else 
	    rotor->off();
	rotor->setFreq(soRot->speed.getValue());
	rotor->setAxis(axis[0], axis[1], axis[2]);
	rotor->setAngle(angle);
	
	const char *name = soRot->getName().getString();
	if (name != NULL && strlen(name) > 0)
	    rotor->setName(name);
	else
	    rotor->setName("Rotor");
    }	
    else if (node->getTypeId() == SoPendulum::getClassTypeId())
    {
	// insert a DCS or SCS above the animated xform
	if (cbd->stateStack[cbd->stateDepth].xformDirty)
	    addXform(cbd, cba, node, ConvertXforms?(new pfDCS):NULL);

	// get the current state of this node, so that our MatStack
	// jibes with inventor's notion of a concatenated xform
	cbd->getMatrixAction->apply((SoNode *)node);
	cbd->stateStack[cbd->stateDepth].currentXform.
	    preMult(*(pfMatrix*)cbd->getMatrixAction->getMatrix().getValue());
	
	cbd->stateStack[cbd->stateDepth].xformDirty = 1;

#ifdef DEBUG_MESSAGES
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
	    "PREPENDULUM: depths (%d, %d)", 
		cbd->stateDepth, 
		cbd->parentStack->getLength()-1);
	cbd->getMatrixAction->getMatrix().print(stdout);
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
		 "PREPENDULUM: depths (%d, %d) concat", 
		cbd->stateDepth, 
		cbd->parentStack->getLength()-1);
	cba->getModelMatrix().print(stdout);
#endif
	Pendulum *pend = (Pendulum*)addXform(cbd, cba, node, new Pendulum);
	SoPendulum *soPend = (SoPendulum*)node;
	
	if (soPend->on.getValue())
	    pend->on();
	else
	    pend->off();

	pend->setFreq(soPend->speed.getValue());
	pfVec3 axis0, axis1;
	float angle;

	soPend->rotation0.getValue(*(SbVec3f*)&axis0, angle);
	pend->setAxis(axis0[0], axis0[1], axis0[2]);
	pend->setStartAngle(PF_RAD2DEG(angle));

	soPend->rotation1.getValue(*(SbVec3f*)&axis1, angle);
	if (PFDOT_VEC3(axis0, axis1) < 0.0f)
	{
	    axis1.negate(axis1);
	    angle = -angle;
	}
	pend->setEndAngle(PF_RAD2DEG(angle));
	    
	if (!axis0.almostEqual(axis1, .01f))
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		     "pfLoad_iv() Unsupported SoPendulum between different axes");
	const char *name = soPend->getName().getString();
	if (name != NULL && strlen(name) > 0)
	    pend->setName(name);
	else
	    pend->setName("Pendulum");
    }	
    else if (node->getTypeId() == SoShuttle::getClassTypeId())
    {
	// insert a DCS or SCS above the animated xform
	if (cbd->stateStack[cbd->stateDepth].xformDirty)
	    addXform(cbd, cba, node, ConvertXforms?(new pfDCS):NULL);

	// get the current state of this node, so that our MatStack
	// jibes with inventor's notion of a concatenated xform
	cbd->getMatrixAction->apply((SoNode *)node);
	cbd->stateStack[cbd->stateDepth].currentXform.
	    preMult(*(pfMatrix*)cbd->getMatrixAction->getMatrix().getValue());
	
	cbd->stateStack[cbd->stateDepth].xformDirty = 1;

#ifdef DEBUG_MESSAGES
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
	    "PRESHUTTLE: depths (%d, %d) cur", 
		cbd->stateDepth, 
		cbd->parentStack->getLength()-1);
	cbd->getMatrixAction->getMatrix().print(stdout);
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
	    "PRESHUTTLE: depths (%d, %d) concat", 
		cbd->stateDepth, 
		cbd->parentStack->getLength()-1);
	cba->getModelMatrix().print(stdout);
#endif
	SoShuttle *soShut = (SoShuttle*)node;

	Shuttle *shut = (Shuttle*)addXform(cbd, cba, node, new Shuttle);

	if (soShut->on.getValue())
	    shut->on();
	else
	    shut->off();

	shut->setFreq(soShut->speed.getValue());

	SbVec3f trans;
	trans = soShut->translation0.getValue();
	shut->setStartPos(trans[0], trans[1], trans[2]);
	trans = soShut->translation1.getValue();
	shut->setEndPos(trans[0], trans[1], trans[2]);
	
	const char *name = soShut->getName().getString();
	if (name != NULL && strlen(name) > 0)
	    shut->setName(name);
	else
	    shut->setName("Shuttle");
    }
    else 	// Unknown
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfLoad_iv() Unsupported SoTransform type \"%s\". "
		 "Using SoGetMatrixAction and hoping for the best.\n",
		 node->getTypeId().getName().getString());
	
	cbd->getMatrixAction->apply((SoNode *)node);
	
	cbd->stateStack[cbd->stateDepth].currentXform.
	    preMult(*(pfMatrix*)cbd->getMatrixAction->getMatrix().getValue());
	cbd->stateStack[cbd->stateDepth].xformDirty = 1;
    }
    
}

//
// -----------------------------  SoLight -------------------------
//
static void
preLight(CbData *cbd, SoCallbackAction *cba, const SoNode *node)
{
    pfLightSource *perfLSource = NULL;
    pfLight *perfLight = NULL;

    const SoLight 	*lgt = (const SoLight *)node;
    
    if (!ConvertLights)
    {
	static int notifylights = 0;
	
	if (!notifylights)
	{
	    notifylights = 1;
	    
	    pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE, "\n");
	    pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE,
		     "pfLoad_iv() SoLight conversion not enabled. Call ");
	    pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE,
		     "pfLoad_ivMode(PFIV_CONVERT_LIGHTS, 1) to enable ");
	    pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE,
		     "SoLight conversion.");
	    pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE,
		     "!!!WARNING: Converted lights will have GLOBAL scope!!!");
	}
    }
    else
    {
	
	// Create pfLightSource if light has global effect but
	// be sloppy about determining "global effect".
	// if you really must have this email jrohlf@sgi.com
	if (cbd->parentStack->getLength() <= 1)
	{
	    perfLSource = new pfLightSource;
	    
	    // Turn the light on or off
	    if (lgt->on.getValue())
		perfLSource->on();
	    else
		perfLSource->off();
	    
	    // Add the light to the Performer database.
	    // !!!! NOTE that this light has global, not local scope.
	    cbd->parent->addChild(perfLSource);
	}
	// if you really must have this email jrohlf@sgi.com
	else if (lgt->on.getValue())
	{
	    int	i;
	    
	    perfLight = new pfLight;
	    for (i=0; i<PF_MAX_LIGHTS; i++)
		if (cbd->stateStack[cbd->stateDepth].lights[i] == NULL)
		    break;
	    
	    if (i == PF_MAX_LIGHTS)
		pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
			 "pfLoad_iv() Too many lights at level %d. "
			 "%d is maximum.", cbd->stateDepth,
			 PF_MAX_LIGHTS);
	    else
		cbd->stateStack[cbd->stateDepth].lights[i] = perfLight;
	}

	const SbVec3f &clr = lgt->color.getValue();
	float intens = lgt->intensity.getValue();
	
	perfLight->setColor(PFLT_DIFFUSE,
			    clr[0]*intens, clr[1]*intens, clr[2]*intens);
	
	// Check the type of light and make the appropriate Performer light
	if (node->isOfType(SoDirectionalLight::getClassTypeId()))
	{
	    const SoDirectionalLight *dirLgt =
		(const SoDirectionalLight *)node;
	    pfVec3	vt;
	    
	    xformLight(cba, dirLgt->direction.getValue(), vt, 0);
	    if (perfLSource)
		perfLSource->setPos(vt[0], vt[1], vt[2], 0.0f);
	    else
		perfLight->setPos(vt[0], vt[1], vt[2], 0.0f);
	    cbd->stateStack[cbd->stateDepth].localLightFlag = 0;
	}
	else if (node->isOfType(SoPointLight::getClassTypeId()))
	{
	    const SoPointLight *ptLgt = (const SoPointLight *)node;
	    pfVec3	vt;
	    
	    xformLight(cba, ptLgt->location.getValue(), vt, 1);
	    if (perfLSource)
		perfLSource->setPos(vt[0], vt[1], vt[2], 0.0f);
	    else
		perfLight->setPos(vt[0], vt[1], vt[2], 0.0f);
	    cbd->stateStack[cbd->stateDepth].localLightFlag = 1;
	}
	else if (node->isOfType(SoSpotLight::getClassTypeId()))
	{
	    const SoSpotLight *spLgt = (const SoSpotLight *)node;
	    pfVec3	vt;
	    
	    if (perfLSource)
		perfLSource->setSpotCone(spLgt->dropOffRate.getValue()*128.0f,
					 spLgt->cutOffAngle.getValue()*180.0f/PF_PI);
	    else
		perfLight->setSpotCone(spLgt->dropOffRate.getValue()*128.0f,
				       spLgt->cutOffAngle.getValue()*180.0f/PF_PI);
	    
	    xformLight(cba, spLgt->location.getValue(), vt, 1);
	    if (perfLSource)
		perfLSource->setPos(vt[0], vt[1], vt[2], 0.0f);
	    else
		perfLight->setPos(vt[0], vt[1], vt[2], 0.0f);
	    cbd->stateStack[cbd->stateDepth].localLightFlag = 1;
	    
	    xformLight(cba, spLgt->direction.getValue(), vt, 0);
	    if (perfLSource)
		perfLSource->setSpotDir(vt[0], vt[1], vt[2]);
	    else
		perfLight->setSpotDir(vt[0], vt[1], vt[2]);
	}
    }
}

static SoCallbackAction::Response
preNode (void *data, SoCallbackAction *cba, const SoNode *node)
{
    CbData *cbd = (CbData *)data;
   
#ifdef DEBUG_MESSAGES
    pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
	"preNode 0x%x (%s) type = %s, depths (xform, parent): (%d %d)", 
	    node,
	    node->getName().getString(), 
	    node->getTypeId().getName().getString(), 
	    pfGetMStackDepth(cbd->mstack), 
	    cbd->parentStack->getLength() - 1);
#endif

    // First flush accumulated transforms into pfDCS if necessary.
    // This assumes that all property nodes (except SoLight) are unaffected
    // by transforms since they do not force a flush
    if (cbd->stateStack[cbd->stateDepth].xformDirty)
    {
	if (node->isOfType(SoGroup::getClassTypeId()) ||
	    node->isOfType(SoShape::getClassTypeId()) ||
	    (node->getTypeId() == SoLight::getClassTypeId() && ConvertLights))
	{
	    const char *name = node->getName().getString();
	    // Use DCS if ConvertXforms is TRUE or if node name has
	    // the string "dcs" in it.
	    if (ConvertXforms ||
		(name && (strstr(name, "dcs") || strstr(name, "DCS"))))
	    {
		addXform(cbd, cba, node, (new pfDCS));
	    }
	}
    }
    //
    // ----------------------- SoGroup --------------------------
    //
    if (node->isOfType(SoGroup::getClassTypeId()))
    {
	preGroup(cbd, cba, node);
    }
    else if (node->isOfType(SoTransformation::getClassTypeId()))
    {
	preTransform(cbd, cba, node);
    }
    else if (node->isOfType(SoLight::getClassTypeId()))
    {
	preLight(cbd, cba, node);
    }
    else if (node->getTypeId() == SoTexture2::getClassTypeId())
    {
	cbd->stateStack[cbd->stateDepth].soTex = (SoTexture2*)node;
    }
    else if (node->getTypeId() == SoShapeHints::getClassTypeId())
    {
	cbd->stateStack[cbd->stateDepth].shapeHints =
	    (SoShapeHints*)node;
    }
    else if (node->getTypeId() == SoTextureCoordinateEnvironment::getClassTypeId())
    {
	cbd->parent->setTravFuncs(PFTRAV_DRAW,
			enableSphereMap, disableSphereMap);
    }
    else if (node->getTypeId() == SoLabel::getClassTypeId())
    {
	// try sticking the label name onto the parent pfNode
	pfNode *pfnode = (pfNode *)(*cbd->parentStack)[cbd->parentStack->getLength()-1];
	if (pfnode && !pfnode->getName())
	    pfnode->setName(((SoLabel*)node)->label.getValue().getString());
    }
    else if (node->getTypeId() == SoInfo::getClassTypeId() ||
	     node->getTypeId() == SoEnvironment::getClassTypeId() ||
	     node->getTypeId() == SoText2::getClassTypeId())
    {
#ifdef	VERBOSE_WARNINGS
	pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE,
		 "pfLoad_iv() Unsupported SoNode type \"%s\". Ignoring...",
		 node->getTypeId().getName().getString());
#endif
    }
    return SoCallbackAction::CONTINUE;
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//   This callback routine is called after a group node has been traversed.
//   The parent node is popped from the parentStack.  This correctly resets
//   the parent after traversal of transformation nodes that would have
//   created intermediate dcs nodes.
//
// Use: private
//
////////////////////////////////////////////////////////////////////////

static SoCallbackAction::Response
postGroup(void *data, SoCallbackAction *, const SoNode *node)
{
    CbData 	*cbd;
    int    	stackDepth;
    pfGroup 	*group;
    UserData	*ud;
   
    cbd = (CbData *)data;

#ifdef DEBUG_MESSAGES
    pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
	"postGroup 0x%x type = %s, depths (xform, parent): (%d %d)", 
	    node,
	    node->getTypeId().getName().getString(), 
	    pfGetMStackDepth(cbd->mstack), 
	    cbd->parentStack->getLength() - 1);
#endif
   
    stackDepth = cbd->parentStack->getLength();
   
    // First patch up groups which depend on the number of their children
    if (node->getTypeId() == SoBlinker::getClassTypeId() ||
	node->getTypeId() == SoSwitch::getClassTypeId())
    {
	int	       i;
	
	// Search for corresponding pfGroup on parentStack
	for (i=stackDepth-1; i>=0; i--)
	{
	    ud = (UserData *)((pfNode*)(*cbd->parentStack)[i])->getUserData();
	    if (ud && ud->node == node)
		break;
	}
	if (i >= 0)
	    group = (pfGroup*)(*cbd->parentStack)[i];
	else
	    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, "pfLoad_iv() Could not find "
		     "pfNode corresponding to SoNode 0x%x of type \"%s\"\n",
		     node, node->getTypeId().getName().getString());
    }
    if (node->getTypeId() == SoSwitch::getClassTypeId() &&
	pfIsOfType(group, pfSwitch::getClassType()))
    {
	if (ud->switchVal == SO_SWITCH_NONE)
	    ((pfSwitch*)group)->setVal(PFSWITCH_OFF);

	else if (ud->switchVal == SO_SWITCH_INHERIT)
	    ((pfSwitch*)group)->setVal(PFSWITCH_ON);

	else if (ud->switchVal == SO_SWITCH_ALL)
	    ((pfSwitch*)group)->setVal(PFSWITCH_ON);

	else if (ud->switchVal < group->getNumChildren())
	    ((pfSwitch*)group)->setVal(ud->switchVal);
	else
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
		     "pfLoad_iv() switch value greater than number of children");
    }
    if (node->getTypeId() == SoBlinker::getClassTypeId())
    {
	SoBlinker	*blinker = (SoBlinker*)node;
	int		 num;
	
	num = group->getNumChildren();
	
	// Need to add dummy geode so single child blinks
	if (num == 1)
	    group->addChild(new pfGeode);
	
	num = group->getNumChildren();
	float speed = blinker->speed.getValue();
	
	pfSequence *seq = ((pfSequence*)group);

	seq->setTime(PFSEQ_ALL, 1.0f / (double)(num * speed));
	seq->setDuration(1.0f, -1);
	
	if (blinker->on.getValue())
	    seq->setMode(PFSEQ_START);
    }
    else if (node->isOfType(SoSeparator::getClassTypeId()))
	cbd->stateDepth--;
   
    if (stackDepth == 0)
	return SoCallbackAction::CONTINUE;
   
    // Pop parents off stack until we find separator
    if ((node->getTypeId() == SoSeparator::getClassTypeId()) ||
	(node->getTypeId() == SoTransformSeparator::getClassTypeId()))
    {
	group = (pfGroup *)((*(cbd->parentStack))[stackDepth-1]);
	ud = (UserData*)group->getUserData();
	while (ud == NULL || node != ud->node)
	{
	    // No longer need user data
	    group->setUserData(NULL);
	   
#ifdef DEBUG_MESSAGES
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
		"popSeparator() SoNode 0x%x, pfGroup 0x%x, depths (%d, %d)",
		    node, 
		    group, 
		    pfGetMStackDepth(cbd->mstack), 
		    cbd->parentStack->getLength() - 1);
#endif
	   
	    // Pop matrix stack if we're popping a transform node
	    if (pfIsOfType(group, pfSCS::getClassType()))
	    {
		cbd->mstack->pop();
		cbd->invmstack->pop();
	    }
	   
	    // Remove the last entry in the parentStack
	    cbd->parentStack->remove(--stackDepth);
	   
	    group = (pfGroup *)((*(cbd->parentStack))[stackDepth-1]);
	    ud = (UserData*)group->getUserData();
	}
    }
   
    // Pop off separator or if not separator, pop only if parent is
    // the same as the top of stack. If not, there is a transform (DCS)
    // in the way so we can't pop until we see a separator.
    stackDepth = cbd->parentStack->getLength();
    group = (pfGroup *)((*(cbd->parentStack))[stackDepth-1]);
    ud = (UserData*)group->getUserData();
    if (ud && node == ud->node)
    {
	pfFree(ud);
	// No longer need user data
	group->setUserData(NULL);
#ifdef DEBUG_MESSAGES
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
		"popGroup() SoNode 0x%x, pfGroup 0x%x, depths (%d,%d)",
		    node, 
		    group, 
		    pfGetMStackDepth(cbd->mstack), 
		    cbd->parentStack->getLength() - 1);
#endif
	// Pop matrix stack if we're popping a transform node
	if (group->isOfType(pfSCS::getClassType()))
	{
	    cbd->mstack->pop();
	    cbd->invmstack->pop();
	}
	
	// Remove the last entry in the parentStack
	cbd->parentStack->remove(stackDepth - 1);
    }
   
    // Current parent is top of stack (last in list)
    cbd->parent = (pfGroup*)
	(*cbd->parentStack)[cbd->parentStack->getLength() - 1];
   
    return SoCallbackAction::CONTINUE;
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//   This callback routine is called to add a texture map to the shape.
//   If a texture exists for the current shape, fill in a Performer
//   texture sructure for it.
//
// Use: private
//
////////////////////////////////////////////////////////////////////////

static void
getTexture(CbData *cbd, SoCallbackAction *act, const SoNode *)
{
    SbVec2s  texSize;
    int      	numComponents, share, needImage;
    const    unsigned char *texImage;
    unsigned int     *perfImage;
    pfTexture *perfTex = NULL;
    char	*texName;
   
    if (cbd->stateStack[cbd->stateDepth].soTex == NULL ||
	(texImage = cbd->stateStack[cbd->stateDepth].soTex->image.
	 getValue(texSize, numComponents)) == NULL)
    {
        cbd->doTextures = FALSE;
        return;
    }
   
    texName = (char*) cbd->stateStack[cbd->stateDepth].soTex->
	filename.getValue().getString();
   
    // Do not try to share texture if it does not reference an image file
    if (strlen(texName)==0)
    {
	share = 0;
	perfTex = new pfTexture;
    }
    else
    {
	share = 1;
	perfTex = cbd->dummyTex;
	perfTex->setName(texName);
    }
   
    // Set the filter modes of the texture
    switch (act->getTextureWrapS()) {
    case SoTexture2::CLAMP:
	perfTex->setRepeat(PFTEX_WRAP_S, PFTEX_CLAMP);
	break;
    case SoTexture2::REPEAT:
	perfTex->setRepeat(PFTEX_WRAP_S, PFTEX_REPEAT);
	break;
    }
    switch (act->getTextureWrapT()) {
    case SoTexture2::CLAMP:
	perfTex->setRepeat(PFTEX_WRAP_T, PFTEX_CLAMP);
	break;
    case SoTexture2::REPEAT:
	perfTex->setRepeat(PFTEX_WRAP_T, PFTEX_REPEAT);
	break;
    }
   
    needImage = 0;
    if (share)
    {
	// Get a shared texture or create a new one and add to share list
	perfTex = (pfTexture*)pfdFindSharedObject(cbd->share,
						  (pfObject*)cbd->dummyTex);
	if (!perfTex)
	{
	    perfTex = (pfTexture*)pfdNewSharedObject(cbd->share,
						     (pfObject*)cbd->dummyTex);
	    needImage = 1;
	}
    }
   
    // Now set up the texture image
    if (needImage || !share)
    {
	// Pixel information is stored with rows rounded to 32bit boundaries 
    	int rowSize = (texSize[0] * numComponents + 3) & ~0003;

	const unsigned char 	*soImage = texImage;

    	// Need to copy image to shared memory for Performer
    	if ((perfImage = (unsigned int *)pfCalloc(rowSize *
						  texSize[1], sizeof(uint),
						  cbd->arena)) == NULL)
	{
            cbd->doTextures = FALSE;
            return;
        }

	
        unsigned char *pImage = (unsigned char *)perfImage;
        for (int i = 0; i<texSize[1]; i++)
            for (int j=0; j<texSize[0]; j++)
	    {
		for (int comp = 0; comp < numComponents ; comp++)
		    pImage[i*rowSize + j*numComponents + comp] = *soImage++;
	    }

    	perfTex->setImage(perfImage, numComponents,
			  texSize[0], texSize[1], 1);
    }
   
    cbd->dummyGState->setAttr(PFSTATE_TEXTURE, perfTex);
    cbd->dummyGState->setMode(PFSTATE_ENTEXTURE, PF_ON);
    cbd->doTextures = TRUE;
   
    pfTexEnv *tEnv;
    if (act->getTextureModel() == SoTexture2::MODULATE)
	tEnv = cbd->modTEnv;
    else if (act->getTextureModel() == SoTexture2::DECAL)
	tEnv = cbd->decalTEnv;
    else if (act->getTextureModel() == SoTexture2::BLEND)
    {
	const SbColor bcolor = act->getTextureBlendColor();
	
	tEnv = new pfTexEnv;
	tEnv->setBlendColor(bcolor[0], bcolor[1],
			    bcolor[2], 1.0f);
	tEnv->setMode(PFTE_BLEND);
    }
    cbd->dummyGState->setAttr(PFSTATE_TEXENV, tEnv);
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//   This callback routine is called before every shape node is encountered
//   during traversal.  A Performer geode is initialized.
//   Subsequent primitives will be placed in this geoSet.
//
// Use: private
//
////////////////////////////////////////////////////////////////////////

static SoCallbackAction::Response
initializeNewShape(void *data, SoCallbackAction *act, const SoNode *node)
{
    CbData 		*cbd = (CbData *)data;
    SbColor 		ambient, diffuse, specular, emission;
    float   		shininess, transparency;
   
    cbd->numVerts = 0;
    cbd->numPrims = 0;
   
    // Construct new geode and add to scene graph
    cbd->geode = new pfGeode;
   
    // Reset dummy geostate
    cbd->dummyGState->setInherit(~0);
   
    if (act->getPickStyle() == SoPickStyle::UNPICKABLE)
	cbd->geode->setTravMask(PFTRAV_ISECT, 0, PFTRAV_SELF, PF_SET);
    else if (act->getPickStyle() == SoPickStyle::BOUNDING_BOX)
	pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE,
		 "pfLoad_iv() Do not support SoPickStyle::BOUNDING_BOX at "
		 "the node level. Use PFTRAV_IS_GEODE|PFTRAV_IS_GSET mode "
		 "to pfNodeIsectSegs() for this behavior.\n");
   
    getTexture(cbd, act, node);	// Set the texture if any
    cbd->dummyGState->setMode(PFSTATE_ENTEXTURE, cbd->doTextures != 0);
   
    // Get the material colors
    act->getMaterial(ambient, diffuse, specular, emission,
		     shininess, transparency, 0);
   
    // Set transparency mode
    if (transparency > 0.0f)
	cbd->dummyGState->setMode(PFSTATE_TRANSPARENCY, PFTR_ON);
    //    else  inherit from the global default which should be PFTR_OFF
    //	cbd->dummyGState->setMode(PFSTATE_TRANSPARENCY, PFTR_OFF);
   
    // Determine normal and material binding
    if (node->isOfType(SoVertexShape::getClassTypeId()))
    {
	// Check the normal binding
	switch(act->getNormalBinding()) {
	case SoNormalBinding::OVERALL:
	    {
		const SbVec3f &norm = act->getNormal(0);
		cbd->geom->norms[0].set(norm[0], norm[1], norm[2]);
		cbd->normBind = PFGS_OVERALL;
	    }
	    break;
	case SoNormalBinding::PER_PART:
	case SoNormalBinding::PER_PART_INDEXED:
	case SoNormalBinding::PER_FACE:
	case SoNormalBinding::PER_FACE_INDEXED:
	    cbd->normBind = PFGS_PER_PRIM;
	    break;
#if SO_VERSION_REVISION < 1
	case SoNormalBinding::DEFAULT:
#endif
	case SoNormalBinding::PER_VERTEX:
	case SoNormalBinding::PER_VERTEX_INDEXED:
	default:
	    cbd->normBind = PFGS_PER_VERTEX;
	    break;
	}
	
	// Check the material binding
	switch(act->getMaterialBinding()) {
#if SO_VERSION_REVISION < 1
	case SoMaterialBinding::DEFAULT:
#endif
	case SoMaterialBinding::OVERALL:
	    cbd->colorBind = PFGS_OVERALL;
	    cbd->geom->colors[0].set(diffuse[0], diffuse[1], diffuse[2],
				     1.0f - transparency);
	    break;
	case SoMaterialBinding::PER_PART:
	case SoMaterialBinding::PER_PART_INDEXED:
	case SoMaterialBinding::PER_FACE:
	case SoMaterialBinding::PER_FACE_INDEXED:
	    cbd->colorBind = PFGS_PER_PRIM;
	    break;
	case SoMaterialBinding::PER_VERTEX:
	case SoMaterialBinding::PER_VERTEX_INDEXED:
	default:
	    cbd->colorBind = PFGS_PER_VERTEX;
	    break;
	}
    }
    else
    {
	cbd->colorBind = PFGS_PER_VERTEX;
	cbd->normBind = PFGS_PER_VERTEX;
    }
   
    int twoSide = 0;
    reverseOrder = 0;
    reverseNormal = 0;
   
    // Reverse just vertex order when mirroring transform is present
    if (((pfMatrix*)act->getModelMatrix().getValue())->getMatType() &
	PFMAT_MIRROR)
	reverseOrder = !reverseOrder;
   
    // Enable face culling when appropriate.
    // Never disable face culling in the geostate -
    // instead inherit face culling mode from global state. This gives us
    // a big performance boost when the global mode is PFCF_BACK.
    if (act->getShapeType() == SoShapeHints::SOLID)
    {
	if (act->getVertexOrdering() == SoShapeHints::COUNTERCLOCKWISE)
	{
#ifdef	DONT_PRESUME_BACKFACE_CULLING
	    cbd->dummyGState->setMode(PFSTATE_CULLFACE, PFCF_BACK);
#endif
	}
	else
	    if (act->getVertexOrdering() == SoShapeHints::CLOCKWISE)
	    {
		// In this case we'll change the vertex and normal orientation
		// to be counterclockwise so PFCF_BACK will work
		reverseOrder = !reverseOrder;
		reverseNormal = !reverseNormal;
#ifdef	DONT_PRESUME_BACKFACE_CULLING
		cbd->dummyGState->setMode(PFSTATE_CULLFACE, PFCF_BACK);
#endif
	    }
	// else SoShapeHints::UNKNOWN_ORDERING so
	// inherit face culling  mode from global state
    }
    else
	if (act->getVertexOrdering() == SoShapeHints::COUNTERCLOCKWISE ||
	    act->getVertexOrdering() == SoShapeHints::CLOCKWISE)
	{
	    static int    notify2side = 0;
	   
	    if (act->getVertexOrdering() == SoShapeHints::CLOCKWISE)
	    {
		// In this case we'll change the vertex and normal orientation
		// to be counterclockwise so PFCF_BACK will work
		reverseOrder = !reverseOrder;
		reverseNormal = !reverseNormal;
#ifdef	DONT_PRESUME_BACKFACE_CULLING
		cbd->dummyGState->setMode(PFSTATE_CULLFACE, PFCF_BACK);
#endif
	    }
	   
	    if (ConvertTwoSide)
	    {
		// non-solid, ordered geometry requires two-sided lighting
		if (act->getLightModel() != SoLightModel::BASE_COLOR) // Lighting ON
		{
		    cbd->dummyGState->setAttr(PFSTATE_LIGHTMODEL,
					      cbd->twoSideLM);
		    twoSide = 1;
		}
	    }
	    else if (!notify2side)
	    {
		notify2side = 1;
		
		pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE, "\n");
		pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE,
			 "pfLoad_iv() Two-sided lighting support is disabled for ");
		pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE,
			 "improved rendering performance. Call");
		pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE,
			 "pfLoad_ivMode(PFIV_CONVERT_TWOSIDE, 1); to enable it.");
		pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE,
			 "However, make sure you *really* need two-sided lighting.");
	    }
	}
   
    if (act->getLightModel() == SoLightModel::BASE_COLOR)// Lighting OFF
    {
	cbd->dummyGState->setMode(PFSTATE_ENLIGHTING, PF_OFF);
	pfdMesherMode(PFDMESH_LOCAL_LIGHTING, FALSE);
    }
    else						// Lighting ON
    {
	pfMaterial 	*mtl;
	pfVec3	dftDiff;
	pfVec3	dftAmb;
	pfVec3	dftSpe;
	pfVec3	dftEmi;
	pfVec2	dftShiTra;
	pfVec2	tmp;
	
	dftDiff.set(0.8f, 0.8f, 0.8f);
	dftAmb.set(0.2f, 0.2f, 0.2f);
	dftSpe.set(0.0f, 0.0f, 0.0f);
	dftEmi.set(0.0f, 0.0f, 0.0f);
	dftShiTra.set(0.2f, 0.0f);

	// inherit lighting enable from global state
	// 	cbd->dummyGState->setMode(PFSTATE_ENLIGHTING, PF_OFF);
	
	tmp[0] = shininess;
	tmp[1] = transparency;
	
	// Compare material values to default. If same, use a default material
	// to avoid creating zillions of pfMaterials. N.B.: It would be
	// better to use the isDefault() method of each field but
	// SoCallbackAction does not give us the current SoMaterial node.
	// Use a tolerance of 1 in 4095 == 12 bit color
	if (
	    !PFALMOST_EQUAL_VEC3(ambient.getValue(), dftAmb, .00024f)  ||
	    !PFALMOST_EQUAL_VEC3(specular.getValue(), dftSpe, .00024f) ||
	    !PFALMOST_EQUAL_VEC3(emission.getValue(), dftEmi, .00024f) ||
	    !PFALMOST_EQUAL_VEC2(tmp, dftShiTra, .00024f))
	{
	    // Set up dummy material, cbd->dummyMtl
	   
	    cbd->dummyMtl->setColor(PFMTL_AMBIENT,
				    ambient[0], ambient[1], ambient[2]);
	    // Do not set DIFFUSE if not two-sided since we always use 
	    // color mode to reduce the number of materials required. 
	    // IrisGL does not have back-side color mode.
	    cbd->dummyMtl->setColor(PFMTL_SPECULAR,
				    specular[0], specular[1], specular[2]);
	    cbd->dummyMtl->setColor(PFMTL_EMISSION,
				    emission[0], emission[1], emission[2]);
	   
	    //
	    // Map Inventor's 0-1 shininess range into Performer/OpenGL's
	    // open-ended range. Note that we arbitrarily choose 128 as the
	    // maximum. However, disable shininess if specular color is black.
	    // Otherwise, a non-zero shininess may put us in a slow, specular
	    // lighting path.
	    //
	    if (PFALMOST_EQUAL_VEC3(specular.getValue(), dftSpe, .00024f))
		cbd->dummyMtl->setShininess(0.0f);
	    else
		cbd->dummyMtl->setShininess(shininess * 128.0f);
	   
	    cbd->dummyMtl->setAlpha(1.0f - transparency);
	   
	    // Get shared material or create new one and add to shared list.
	    mtl = (pfMaterial*)pfdNewSharedObject(cbd->share,
						  (pfObject*)cbd->dummyMtl);
	}
	else
	    mtl = cbd->dftMtl;
	
	cbd->dummyGState->setMode(PFSTATE_ENLIGHTING, PF_ON);
	cbd->dummyGState->setAttr(PFSTATE_FRONTMTL, mtl);
	if (twoSide)
	    cbd->dummyGState->setAttr(PFSTATE_BACKMTL, mtl);
	
	// Always use color mode to reduce the number of materials required
	mtl->setColorMode(PFMTL_BOTH, PFMTL_CMODE_DIFFUSE);
	
	// Set pfLight array if necessary
	if (ConvertLights && cbd->stateStack[cbd->stateDepth].lights[0])
	{
	    cbd->dummyGState->setAttr(PFSTATE_LIGHTS,
			 cbd->stateStack[cbd->stateDepth].lights);
	   
	    if (cbd->stateStack[cbd->stateDepth].localLightFlag)
	    {
		// Force mesher to break up flat-shaded meshes
		pfdMesherMode(PFDMESH_LOCAL_LIGHTING, TRUE);
	    }
	    else
		pfdMesherMode(PFDMESH_LOCAL_LIGHTING, FALSE);
	   
	}
    }
   
    // Get shared geostate or create new one and add to shared list.
    cbd->geostate = (pfGeoState*)pfdNewSharedObject(cbd->share,
						    (pfObject*)cbd->dummyGState);
   
    cbd->node = (SoNode *)node;
    return SoCallbackAction::CONTINUE;
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//   This callback routine is called after every shape node is encountered
//   during traversal.  The remaining portions of the Performer geode
//   are filled in.
//
// Use: private
//
////////////////////////////////////////////////////////////////////////

static SoCallbackAction::Response
finishNewShape(void *data, SoCallbackAction *act, const SoNode *node)
{
    CbData 	*cbd = (CbData *)data;
    const pfList   *gsetList;
    pfdGeoBuilder  *builder;
    pfdGeom	*pb;
    static int 	j;
    pfMatrix	invMat;
    int		t;
   
    builder = cbd->builder;
    pb = cbd->geom;
    pb->numVerts = cbd->numVerts;
    pb->nbind = cbd->normBind;
    pb->cbind = cbd->colorBind;
   
    if (cbd->doTextures)
    {
    	pb->tbind[0] = PFGS_PER_VERTEX;
	for (t = 1 ; t < PF_MAX_TEXTURES ; t ++)
	    pb->tbind[t] = PFGS_OFF;
    }
    else
	for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    pb->tbind[t] = PFGS_OFF;
   
    pb->flags = 0; // not indexed
   
    pfMatrix	mat;
    mat.copy(*(pfMatrix*)act->getModelMatrix().getValue());
   
    // Compare accumulated transform to that inherited through the
    // Performer scene graph (if any). Any extra transform is applied 
    // directly to the coordinates here. Extra transforms are provided 
    // by SoArray, SoMultipleCopy
    
    mat.postMult(*cbd->invmstack->getTop());

    if (!PFALMOST_EQUAL_MAT(mat, pfIdentMat, 0.0001f))
    {
	int		i, n;
	
#ifdef DEBUG_MESSAGES
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, "%.2f\t%.2f\t%.2f\t%.2f",
	    mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, "%.2f\t%.2f\t%.2f\t%.2f",
	    mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, "%.2f\t%.2f\t%.2f\t%.2f",
	    mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, "%.2f\t%.2f\t%.2f\t%.2f",
	    mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
#endif
	
	for (i=0; i<cbd->numVerts; i++)
	    pb->coords[i].xformPt(pb->coords[i], mat);
	
	switch (pb->nbind) {
	case PFGS_OFF:
	    n = 0;
	    break;
	case PFGS_OVERALL:
	    n = 1;
	    break;
	case PFGS_PER_PRIM:
	    n = cbd->numPrims;
	    break;
	case PFGS_PER_VERTEX:
	    n = cbd->numVerts;
	    break;
	}
	invMat.invertAff(mat);
	invMat.transpose(invMat);
	for (i=0; i<n; i++)
	{
	    pb->norms[i].xformVec(pb->norms[i], invMat);
	    pb->norms[i].normalize();
	}
    }
   
    // Transform texture coordinates if texture matrix is not identity
    if (cbd->doTextures)
    {
	mat.copy(*(pfMatrix*)act->getTextureMatrix().getValue());
	
	if (!PFALMOST_EQUAL_MAT(mat, pfIdentMat, 0.0001f))
	{
	    int		i;
	   
	    for (i=0; i<cbd->numVerts; i++)
	    {
		pfVec3	tmp;
		
		tmp.set(pb->texCoords[0][i][0], pb->texCoords[0][i][1], 0.0f);
		tmp.xformPt(tmp, mat);
		pb->texCoords[0][i].set(tmp[0], tmp[1]);
	    }
	}
    }
   
    switch(cbd->primtype) {
    case MYTRIS:
	pb->primtype = PFGS_TRIS;
	break;
    case MYLINES:
	pb->primtype = PFGS_LINES;
	pb->pixelsize = act->getLineWidth();
	break;
    case MYPOINTS:
	pb->primtype = PFGS_POINTS;
	pb->pixelsize = act->getPointSize();
	break;
    default:
	break;
    }
   
    pfdAddGeom(builder, pb, 0);
   
    gsetList = pfdBuildGSets(builder);
    for (j=0; j<gsetList->getNum(); j++)
    {
        pfGeoSet *gset = (pfGeoSet*)gsetList->get(j);
	
	gset->setLineWidth(act->getLineWidth());
	gset->setPntSize(act->getPointSize());
	
	// Set the drawing style and line width
	switch (act->getDrawStyle())
	{
	case SoDrawStyle::FILLED:
	    // Do nothing
	    break;
	case SoDrawStyle::LINES:
	    gset->setDrawMode(PFGS_WIREFRAME, PF_ON);
	    break;
	case SoDrawStyle::INVISIBLE:
	    cbd->geode->setTravMask(PFTRAV_DRAW, 0, PFTRAV_SELF, PF_SET);
	    break;
	default:
	    pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE,
		     "pfLoad_iv() Cannot support draw style %d\n",
		     act->getDrawStyle());
	}
	gset->setGState(cbd->geostate);
	cbd->geode->addGSet(gset);
    }
    if (cbd->geode->getNumGSets() == 0)
	pfDelete(cbd->geode);
    else
    {
	cbd->parent->addChild(cbd->geode);
	
#ifdef BREAKUP
	hierarchy = pfdBreakup(cbd->geode, 100.0f, 256, 32);
	if (hierarchy != NULL)
	{
	    for (i=0; i < pfGetNumParents(cbd->geode); i++)
	    {
		group = pfGetParent(cbd->geode, i);
		pfReplaceChild(group, cbd->geode, hierarchy);
	    }
	    pfDelete(cbd->geode);
	    cbd->geode = (pfGeode *)hierarchy;
	}
#endif
	
	const char *name = node->getName().getString();
	if (strlen(name) > 0)
	    cbd->geode->setName(name);
    }
   
    return SoCallbackAction::CONTINUE;
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//   Build a Performer scene graph from an Inventor scene graph
//
// Use: public via pfdConvertFrom()
//
////////////////////////////////////////////////////////////////////////

pfGroup *
pfdConvertFrom_iv(SoSeparator *ivRoot)
{
    SoType		shapeType = SoShape::getClassTypeId();
    SoCallbackAction	ca;
    CbData      *cbData;
    pfSCS     	*pfRoot;
    void	*arena;

    // Get IRIS Performer shared memory arena
    arena = pfGetSharedArena();
   
    // Transform from GL/Inventor's Y-up coordinate system to Performer's Z-up
    pfMatrix	mat (1.0, 0.0, 0.0, 0.0,
                     0.0, 0.0, 1.0, 0.0,
                     0.0,-1.0, 0.0, 0.0,
                     0.0, 0.0, 0.0, 1.0);
    pfRoot = new pfSCS(mat);
   
    // Create callback data that will be used by the conversion action.
    cbData = (CbData *)pfMalloc(sizeof(CbData), arena);
    cbData->action = &ca;
    cbData->arena = arena;
    cbData->parent = (pfGroup *)pfRoot;
    cbData->builder = pfdNewGeoBldr();
    cbData->parentStack = new SbPList;
    cbData->parentStack->append((void *)pfRoot);
    cbData->mstack = new pfMatStack(64);	// Matrix stack
    cbData->invmstack = new pfMatStack(64);	// Inverse matrix stack
    cbData->modTEnv = new pfTexEnv;
    cbData->modTEnv->setMode(PFTE_MODULATE);
    cbData->decalTEnv = new pfTexEnv;
    cbData->decalTEnv->setMode(PFTE_DECAL);
    cbData->twoSideLM = new pfLightModel;
    cbData->twoSideLM->setTwoSide(1);
    cbData->dftMtl = new pfMaterial;
    cbData->dftMtl->setColorMode(PFMTL_BOTH, PFMTL_DIFFUSE);
    cbData->dftMtl->setColor(PFMTL_SPECULAR, 0.0f, 0.0f, 0.0f);
    cbData->dftMtl->setShininess(0.0f);
    cbData->geostate = NULL;
    cbData->dummyGState = new pfGeoState;
    cbData->dummyMtl = new pfMaterial;
    cbData->dummyMtl->setColorMode(PFMTL_BOTH, PFMTL_DIFFUSE);
    cbData->dummyMtl->setColor(PFMTL_SPECULAR, 0.0f, 0.0f, 0.0f);
    cbData->dummyMtl->setShininess(0.0f);
    cbData->dummyTex = new pfTexture;
    // Need image so pfdNewSharedObject doesn't try to load tex file
    cbData->dummyTex->setImage((unsigned int*)pfMalloc(sizeof(int), arena), 
			       4, 1, 1, 1);
    cbData->share = pfdGetGlobalShare();
    cbData->geom = pfdNewGeom(cbData->pbSize = 4096);
    cbData->numVerts      = 0;
    cbData->numPrims      = 0;
    cbData->stateDepth 	  = 0;
    bzero(cbData->stateStack, sizeof(pfivState)*PFIV_MAX_STACK);
    cbData->stateStack[0].currentXform.makeIdent();

    cbData->getMatrixAction = new SoGetMatrixAction(SbViewportRegion());
   
    // Create an SoCallbackAction which will traverse the scene graph,
    // converting it into an IRIS Performer database.
    ca.addPreCallback(SoNode::getClassTypeId(), preNode, cbData);
    ca.addPreCallback(SoShape::getClassTypeId(), initializeNewShape, cbData);
    ca.addPostCallback(SoShape::getClassTypeId(), finishNewShape, cbData);
    ca.addPostCallback(SoGroup::getClassTypeId(), postGroup, cbData);
   
    ca.addTriangleCallback(shapeType, triangleCB, cbData);
    ca.addLineSegmentCallback(shapeType, lineSegmentCB, cbData);
    ca.addPointCallback(shapeType, pointCB, cbData);
   
    ca.apply(ivRoot);
   
    // Free data structures, note that pfDelete checks ref count first
    pfDelete(cbData->mstack);
    pfDelete(cbData->invmstack);
    delete cbData->parentStack;
    pfdDelGeoBldr(cbData->builder);
    pfDelete(cbData->modTEnv);
    pfDelete(cbData->decalTEnv);
    pfDelete(cbData->dftMtl);
    pfDelete(cbData->dummyGState);
    pfDelete(cbData->dummyMtl);
    pfDelete(cbData->dummyTex);
    pfDelete(cbData->twoSideLM);
    pfdDelGeom(cbData->geom);
    delete cbData->getMatrixAction;
    pfFree(cbData);
   
    return (pfGroup *)pfRoot;
}

//
//  The following fixes a bug in
//  SoMultipleCopy::doAction(SoAction *action) which results in N^2
//  behavior when applying a callback action. This code will become
//  obsolete at the next Inventor release.
//
//

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Typical action traversal.
//
// Use: extender
//
////////////////////////////////////////////////////////////////////////

void
SoMultipleCopy::doAction(SoAction *action)
{
    int         numIndices;
    const int   *indices;
    int         lastChild;
    int         index;
    SbBool      gettingBBox;
   
    SbVec3f     totalCenter(0,0,0);             // For bbox
    int         numCenters = 0, i;              // For bbox
   
    // We have to do some extra stuff here for computing bboxes
    gettingBBox = action->isOfType(SoGetBoundingBoxAction::getClassTypeId());
   
    // Determine which children to traverse, if any
    switch (action->getPathCode(numIndices, indices)) {
    case SoAction::NO_PATH:
    case SoAction::BELOW_PATH:
        lastChild = getNumChildren() - 1;
        break;
	
    case SoAction::IN_PATH:
        lastChild = indices[numIndices - 1];
	action->getState()->push();
	children->traverse(action, 0, lastChild);
	action->getState()->pop();
	return;
	
    case SoAction::OFF_PATH:
        // This differs from SoGroup: if the node is not on the
        // path, don't bother traversing its children. Effectively the
        // same as a separator to the rest of the graph.
        return;
    }
   
    for (index = 0; index < matrix.getNum(); index++) {
        action->getState()->push();
	
        // Set value in switch element to current index
        SoSwitchElement::set(action->getState(), this, index);
	
        // Transform
        SoModelMatrixElement::mult(action->getState(), this, matrix[index]);
	
        // Set the center correctly after each child
        if (gettingBBox) {
            SoGetBoundingBoxAction *bba = (SoGetBoundingBoxAction *) action;
	   
            for (i = 0; i <= lastChild; i++) {
                children->traverse(action, i, i);
                if (bba->isCenterSet()) {
                    totalCenter += bba->getCenter();
                    numCenters++;
                    bba->resetCenter();
                }
            }
        }
        else
            children->traverse(action, 0, lastChild);
	
        action->getState()->pop();
    }
   
    if (gettingBBox && numCenters > 0)
        ((SoGetBoundingBoxAction *) action)->setCenter(totalCenter/numCenters,
                                                       FALSE);
}
