
/*
 * Copyright 1995, Silicon Graphics, Inc.
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
#include <ctype.h>
#include <Performer/pf.h>

#include <Performer/pr/pfHighlight.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfChannel.h>

#include <Performer/pfutil.h>

#include <Performer/pfv/pfvInputMngrPicker.h>

#include "UndoManager.h"
#include "mySelector.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

pfGeoSet *
pfmdNewTransBox( float length, float centerx, float centery, float centerz )
{
    // A set of coordinate axes
    pfVec3 *face_coords = (pfVec3*)pfMalloc(24*sizeof(pfVec3));

    // Top
    face_coords[ 0].set(centerx-length, centery-length, centerz-length);
    face_coords[ 1].set(centerx-length, centery+length, centerz-length);
    face_coords[ 2].set(centerx+length, centery+length, centerz-length);
    face_coords[ 3].set(centerx+length, centery-length, centerz-length);
    // Bottom
    face_coords[ 4].set(centerx+length, centery-length, centerz+length);
    face_coords[ 5].set(centerx+length, centery+length, centerz+length);
    face_coords[ 6].set(centerx-length, centery+length, centerz+length);
    face_coords[ 7].set(centerx-length, centery-length, centerz+length);
    // Left
    face_coords[ 8].set(centerx-length, centery-length, centerz+length);
    face_coords[ 9].set(centerx-length, centery+length, centerz+length);
    face_coords[10].set(centerx-length, centery+length, centerz-length);
    face_coords[11].set(centerx-length, centery-length, centerz-length);
    // Right
    face_coords[12].set(centerx+length, centery-length, centerz-length);
    face_coords[13].set(centerx+length, centery+length, centerz-length);
    face_coords[14].set(centerx+length, centery+length, centerz+length);
    face_coords[15].set(centerx+length, centery-length, centerz+length);
    // Front
    face_coords[16].set(centerx+length, centery+length, centerz+length);
    face_coords[17].set(centerx+length, centery+length, centerz-length);
    face_coords[18].set(centerx-length, centery+length, centerz-length);
    face_coords[19].set(centerx-length, centery+length, centerz+length);
    // Back
    face_coords[20].set(centerx-length, centery-length, centerz+length);
    face_coords[21].set(centerx-length, centery-length, centerz-length);
    face_coords[22].set(centerx+length, centery-length, centerz-length);
    face_coords[23].set(centerx+length, centery-length, centerz+length);

    pfVec4 *box_color = (pfVec4*)pfMalloc(sizeof(pfVec4));
    box_color->set(1,0,1,0.1);

    pfGeoSet *gset = new pfGeoSet;
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, face_coords, NULL);
    gset->setAttr(PFGS_COLOR4, PFGS_OVERALL,    box_color,  NULL);
    gset->setLineWidth(1);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(6);
    gset->setDrawMode(PFGS_WIREFRAME, 1);
    int *prim_lengths = (int*)pfMalloc(6*sizeof(int));
    for( int j=0; j<6; j++ ) prim_lengths[j] = 4;
    gset->setPrimLengths(prim_lengths);

    //gset->ref();
    return gset;
}

///////////////////////////////////////////////////////////////////////////////

pfGeode *pfmNewTransBoxNode( float length, 
                             float centerx,
                             float centery,
                             float centerz )
{
    pfGeode *_geode = new pfGeode();
    pfGeoState *_gstate = new pfGeoState();
    pfGeoSet* _gset = pfmdNewTransBox( length, centerx, centery, centerz );
    _gset->setGState(_gstate);

    _geode->addGSet(_gset);
    return _geode;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

pfGeoSet *
pfmdNewRotManipRibbons(float radius, 
	       float centerx, float centery, float centerz,
	       int slices)
{
    const float ringOffset = 0.0618f * radius;
    const float ribbonFraction = 0.25f;
    const float ribbonThickness = ringOffset*ribbonFraction;
    const float ribbonHi = ringOffset+ribbonThickness/2.0f;
    const float ribbonLo = ringOffset-ribbonThickness/2.0f;
    const int numRings = 6;
    //const int numVertexesPerSlice = 4;

    // Set up a geoset
    float angle, cosa, sina;
    pfVec3 *coords = (pfVec3*)pfMalloc(numRings*(slices*2+2)*sizeof(pfVec3));
    pfVec3 *norms = (pfVec3*)pfMalloc(numRings*(slices*2+2)*sizeof(pfVec3));

    for( int i = 0; i < slices; i++ ) {
       angle = 2 * M_PI * i / slices;
       cosa = cosf(angle);
       sina = sinf(angle);
       
       norms[i*2].set(cosa, sina, 0.0f);
       norms[i*2+1].set(cosa, sina, 0.0f);
       norms[(slices*2)+i*2].set(cosa, sina, 0.0f);
       norms[(slices*2)+i*2+1].set(cosa, sina, 0.0f);
	   
       norms[(slices*4)+2+i*2].set( cosa, 0.0f, sina );
       norms[(slices*4)+2+i*2+1].set( cosa, 0.0f, sina );
       norms[(slices*6)+2+i*2].set( cosa, 0.0f, sina );
       norms[(slices*6)+2+i*2+1].set( cosa, 0.0f, sina );
	   
       norms[(slices*8)+4+i*2].set( 0.0f, cosa, sina );
       norms[(slices*8)+4+i*2+1].set( 0.0f, cosa, sina );
       norms[(slices*10)+4+i*2].set( 0.0f, cosa, sina );
       norms[(slices*10)+4+i*2+1].set( 0.0f, cosa, sina );
       
       cosa *= radius;
       sina *= radius;
    
       if( i == 0 || i == slices || i == (slices/2) || i == (slices/4) || i == (3*slices/4) ) {
           coords[i*2].set(cosa, sina, 0.0f);
           coords[i*2+1].set(cosa, sina, 0.0f);
	   coords[(slices*2)+i*2].set(cosa, sina, 0.0f);
	   coords[(slices*2)+i*2+1].set(cosa, sina, 0.0f);
	   
           coords[(slices*4)+2+i*2].set( cosa, 0.0f, sina );
           coords[(slices*4)+2+i*2+1].set( cosa, 0.0f, sina );
	   coords[(slices*6)+2+i*2].set( cosa, 0.0f, sina );
	   coords[(slices*6)+2+i*2+1].set( cosa, 0.0f, sina );
	   
           coords[(slices*8)+4+i*2].set( 0.0f, cosa, sina );
           coords[(slices*8)+4+i*2+1].set( 0.0f, cosa, sina );
	   coords[(slices*10)+4+i*2].set( 0.0f, cosa, sina );
	   coords[(slices*10)+4+i*2+1].set( 0.0f, cosa, sina );
	   
	   
        } else { 
           coords[i*2].set( cosa, sina, ribbonHi );
           coords[i*2+1].set( cosa, sina, ribbonLo );
	   coords[(slices*2)+i*2].set( cosa, sina, -1.0f*ribbonHi );
	   coords[(slices*2)+i*2+1].set( cosa, sina, -1.0f*ribbonLo );
	   
           coords[(slices*4)+2+i*2].set( cosa, ribbonLo, sina );
           coords[(slices*4)+2+i*2+1].set( cosa, ribbonHi, sina  );
	   coords[(slices*6)+2+i*2].set( cosa, -1.0f*ribbonLo, sina );
	   coords[(slices*6)+2+i*2+1].set( cosa, -1.0f*ribbonHi, sina );
	   
           coords[(slices*8)+4+i*2].set( ribbonHi, cosa, sina );
           coords[(slices*8)+4+i*2+1].set( ribbonLo, cosa, sina  );
	   coords[(slices*10)+4+i*2].set( -1.0f*ribbonHi, cosa, sina );
	   coords[(slices*10)+4+i*2+1].set( -1.0f*ribbonLo, cosa, sina );
	   

       }
    }  // i == slices

    // Closures
    coords[slices*2].copy(coords[0]);
    coords[slices*2+1].copy(coords[1]);
    coords[slices*4].copy(coords[0]);
    coords[slices*4+1].copy(coords[1]);
    coords[(slices*4)+2+slices*2].copy(coords[slices*4+2]);
    coords[(slices*4)+2+1+slices*2].copy(coords[slices*4+2+1]);
    coords[(slices*6)+2+slices*2].copy(coords[slices*6+2]);
    coords[(slices*6)+2+1+slices*2].copy(coords[slices*6+2+1]);
    coords[(slices*8)+4+slices*2].copy(coords[slices*8+4]);
    coords[(slices*8)+4+1+slices*2].copy(coords[slices*8+4+1]);
    coords[(slices*10)+4+slices*2].copy(coords[slices*10+4]);
    coords[(slices*10)+4+1+slices*2].copy(coords[slices*10+4+1]);
    norms[slices*2].copy(norms[0]);
    norms[slices*2+1].copy(norms[1]);
    norms[slices*4].copy(norms[0]);
    norms[slices*4+1].copy(norms[1]);
    norms[(slices*4)+2+slices*2].copy(norms[slices*4+2]);
    norms[(slices*4)+2+1+slices*2].copy(norms[slices*4+2+1]);
    norms[(slices*6)+2+slices*2].copy(norms[slices*6+2]);
    norms[(slices*6)+2+1+slices*2].copy(norms[slices*6+2+1]);
    norms[(slices*8)+4+slices*2].copy(norms[slices*8+4]);
    norms[(slices*8)+4+1+slices*2].copy(norms[slices*8+4+1]);
    norms[(slices*10)+4+slices*2].copy(norms[slices*10+4]);
    norms[(slices*10)+4+1+slices*2].copy(norms[slices*10+4+1]);

    // Transform coords to the correct location
    pfMatrix center;
    center.makeTrans( centerx, centery, centerz );
    int num = 2*(slices+1);
    for( int k = 0; k < num; ++k ) {
        pfNotify( PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "orig coord: (%f, %f, %f)",
	         (coords[k])[PF_X], (coords[k])[PF_Y], (coords[k])[PF_Z] ); 
        coords[k].xformPt(coords[k], center);
        pfNotify( PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "xformed coord: (%f, %f, %f)",
	          (coords[k])[PF_X], (coords[k])[PF_Y], (coords[k])[PF_Z] ); 
    }
    pfNotify( PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "center: (%f, %f, %f)",
	      centerx, centery, centerz);
    pfNotify( PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "translation matrix = \n"
              "%f %f %f %f\n"
              "%f %f %f %f\n"
              "%f %f %f %f\n"
              "%f %f %f %f\n",
              center[0][0], center[0][1], center[0][2], center[0][3],
              center[1][0], center[1][1], center[1][2], center[1][3],
              center[2][0], center[2][1], center[2][2], center[2][3],
              center[3][0], center[3][1], center[3][2], center[3][3] );

  int *prim_lengths = (int*)pfMalloc(6*sizeof(int));
  for( int m = 0; m < 6; m++ ) prim_lengths[m] = slices*2+1;

  // Default to magenta
  pfVec4 *colors = (pfVec4*)pfMalloc(6*sizeof(pfVec4));
  for( int t = 0; t < 6; t++ )
      colors[t].set(0.0f, 0.5f, 1.0f, 1.0f);
  //Pretty demo shot    
  //colors[5].set(1.0f, 0.5f, 1.0f, 1.0f);

  pfGeoSet *gset = new pfGeoSet;
  gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
  gset->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
  gset->setAttr(PFGS_COLOR4, PFGS_PER_PRIM, colors, NULL);
  //gset->setAttr(PFGS_COLOR4, PFGS_OFF, NULL, NULL);
 // gset->setLineWidth(1);
  gset->setPrimType(PFGS_TRISTRIPS);
  gset->setNumPrims(6);
  gset->setPrimLengths(prim_lengths);
  // Since it's linestrips, just default to wireframe
  gset->setDrawMode(PFGS_FLATSHADE, PF_OFF);
  //gset->setDrawMode(PFGS_WIREFRAME, PF_ON);
  
  return gset;
}

///////////////////////////////////////////////////////////////////////////////

pfGeode*
pfmNewRotManipNode( float radius = 1.0, 
                     float centerx = 0.0,
                     float centery = 0.0,
                     float centerz = 0.0, 
                     int slices = 36 );

pfGeode *pfmNewRotManipNode(float radius, 
			    float centerx,
			    float centery,
			    float centerz, 
			    int slices)
{
  pfGeoState *_gstate = new pfGeoState();
#if 1
  pfGeoSet   *_gset   = pfmdNewRotManipRibbons(radius, centerx, centery, centerz, slices);
#else
  pfGeoSet   *_gset   = pfmdNewRotManipLines(radius, centerx, centery, centerz, slices);
#endif
  pfGeode *_geode = new pfGeode();
  
  if( !_geode || !_gset || !_gstate ) {
      return NULL;
  }
  _gstate->setMode(PFSTATE_CULLFACE, PF_OFF);
  _gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
  _gset->setGState(_gstate);
  _geode->addGSet(_gset);

  return _geode;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


/* UNDOABLE SELECTION ACTION */

class UndoableSelAction : public UndoableAction
{
public:
    UndoableSelAction( mySelector*_sel, pfNode*_old, pfNode*_new ) {
	sel=_sel; oldnode=_old; newnode=_new;
    }
    
    ~UndoableSelAction() {;}
    
    int undo() {
	sel->select(oldnode, 0);
	return 1;
    }
    int redo(){
	sel->select(newnode, 0);
	return 1;
    }
    
    mySelector* sel;
    pfNode* oldnode;
    pfNode* newnode;
    
};


///////////////////////////////////////////////////////////////////////////////


class UndoableDeleteAction : public UndoableAction
{
public:
	UndoableDeleteAction( mySelector*s, pfNode*_child ) {
		sel = s;
		child = _child;
		parent = child->getParent(0);
		redo();		
	}
	
    ~UndoableDeleteAction() {;}
    
    int undo() {
		parent->insertChild(index,child);
		sel->select(child, 0);
		mySelector::setClipboard(clip);
		return 1;
    }

	int redo(){
		index = parent->searchChild(child);
		parent->removeChild(child);
		sel->select(NULL, 0);
		clip=mySelector::getClipboard();
		mySelector::setClipboard(child);
		return 1;
    }
    
	mySelector*sel;
    pfGroup* parent;
    pfNode* child;
	int index;
	pfNode* clip;

};


///////////////////////////////////////////////////////////////////////////////


class UndoablePasteAction : public UndoableAction
{
public:
	UndoablePasteAction( mySelector*s, pfNode*_child ) {
		sel = s;
		child = _child;
		if(sel->node){
			if(pfIsOfType(sel->node,pfGroup::getClassType()))
				parent = (pfGroup*)(sel->node);
			else
				parent = sel->node->getParent(0);
		}
		else
			parent = sel->scene; 			
		oldsel = sel->getSelection();
		redo();	
	}
	
    ~UndoablePasteAction() {;}
    
    int undo() {
		parent->removeChild(child);
		sel->select(oldsel, 0);
		return 1;
    }

	int redo(){
		parent->addChild(child);
		sel->select(child, 0);
		return 1;
    }
    
	mySelector*sel;
    pfGroup* parent;
    pfNode* child;
	pfNode* oldsel;
};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

pfType* mySelector::classType = NULL;
pfNode* mySelector::clipboard = NULL;

///////////////////////////////////////////////////////////////////////////////

void mySelector::init()
{
    if(classType==NULL)
    {
	pfvSelector::init();
	classType= new pfType(pfvSelector::getClassType(), "mySelector" );
    }
}

///////////////////////////////////////////////////////////////////////////////

mySelector::mySelector(pfvXmlNode* xml)
{
    if(classType==NULL)
	    init();
    setType(classType);

    hl_sel = new pfHighlight;
    PFASSERTALWAYS(hl_sel);
    hl_sel->setMode( PFHL_LINES );
    hl_sel->setColor( PFHL_FGCOLOR, 0.15f, 0.5f, 0.5f );

    node = NULL;
	root = NULL;

    dcsGeom = new pfDCS;
    PFASSERTALWAYS(dcsGeom);
    
    //geom = (pfNode*)pfmNewRotManipNode(1.0f);
	geom = (pfNode*)pfmNewTransBoxNode(0.5f,0.0f,0.0f,0.0f);


    PFASSERTALWAYS(geom);
    
    dcsGeom->addChild(geom);


    undoMngr = new UndoManager;
    PFASSERTALWAYS(undoMngr);

    dragger = new myDragger(undoMngr);
    PFASSERTALWAYS(dragger);
	printf("mySelector::mySelector - dragger created at %x\n", dragger );
    
}

///////////////////////////////////////////////////////////////////////////////

mySelector::~mySelector()
{
    pfDelete(hl_sel);
}

//////////////////////////////////////////////////////////////////////////////

int mySelector::startSelection( pfvPicker*p, int ev )
{
    printf("mySelector::startSelection called\n");
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

void mySelector::resizeGeom(pfNode*_node)
{
    pfBox bbox;
	bbox.makeEmpty();
	
    pfBox bbox2;
	bbox2.makeEmpty();

	if( pfIsOfType(_node,pfSCS::getClassType()) )
	{
		int i,n=((pfGroup*)_node)->getNumChildren();
		for(i=0;i<n;i++)
		{
			pfNode* child = (pfNode*)(((pfGroup*)_node)->getChild(i));
			pfuTravCalcBBox( child, &bbox2);
			
			printf("CHILD %d BBOX: min=%f %f %f  max=%f %f %f\n", i,
				bbox2.min[0],bbox2.min[1],bbox2.min[2],
				bbox2.max[0],bbox2.max[1],bbox2.max[2]);

			bbox.min[0] = PF_MIN2(bbox.min[0],bbox2.min[0]);
			bbox.min[1] = PF_MIN2(bbox.min[1],bbox2.min[1]);
			bbox.min[2] = PF_MIN2(bbox.min[2],bbox2.min[2]);
			
			bbox.max[0] = PF_MAX2(bbox.max[0],bbox2.max[0]);
			bbox.max[1] = PF_MAX2(bbox.max[1],bbox2.max[1]);
			bbox.max[2] = PF_MAX2(bbox.max[2],bbox2.max[2]);
		}
	}

	printf("BBOX: min=%f %f %f  max=%f %f %f \n",
			bbox.min[0],bbox.min[1],bbox.min[2],
			bbox.max[0],bbox.max[1],bbox.max[2]);
	
	pfVec3 boxSize = bbox.max - bbox.min;
	printf("BOX SIZE: %f %f %f\n", boxSize[0], boxSize[1], boxSize[2] );

	
			
    pfVec3 center;
    center.combine(0.5f, bbox.min, 0.5f, bbox.max );
    dcsGeom->setTrans(center[0], center[1], center[2] );
    
    pfVec3 size;
    size.addScaled( bbox.max, -1.0f, bbox.min );
    dcsGeom->setScale( size[0], size[1], size[2] );
}

///////////////////////////////////////////////////////////////////////////////

void mySelector::select(pfNode*_node, int addToUndoList)
{
    printf("mySelector::select - %s %x\n",_node?_node->getTypeName():"NULL",_node );
    
    if(node==_node)
    {
		printf("mySelector::select - %x matches current selection\n",_node );
		return;
    }

    if(node) // deselect currently selected node
    {
		printf("mySelector::select - deselecting %x\n", node );
		pfuTravNodeHlight( node, NULL );
	
		//remove decoration geometry
		dcs->removeChild(dcsGeom);
		printf("CLEARING SLOT for dcs %x\n", dcs );
		pfvInteractor::clearNodeSlot(dcs);
    }

    
    if(_node)
    {
		printf("mySelector::select - selecting %x\n", _node );
    
		pfuTravNodeHlight( _node, hl_sel );

	
		if( pfIsOfType(_node,pfScene::getClassType()) )
		{
	    	printf("mySelector::select - selecting a pfScene node\n");
	    	
	    	pfScene* s = (pfScene*)_node;
	    	if( (s->getNumChildren()==1) && 
			    (pfIsOfType(s->getChild(0),pfDCS::getClassType())) )
	    	{
				printf("mySelector::select - scene already has a DCS single child\n");

				if(addToUndoList)
				    undoMngr->add( new UndoableSelAction(this,node,s->getChild(0)));
		
				dcs = (pfDCS*)s->getChild(0);
				node = (pfNode*)dcs;
				dragger->node = (pfNode*)dcs;
				dragger->nodeSetup((pfNode*)dcs);
				printf("SETTING SLOT for dcs %x\n", dcs );
				resizeGeom(dcs);
				dcs->addChild(dcsGeom);
				return;
	    	}

	    	printf("mySelector::select - creating a DCS to select all scene geom\n");
	    	pfDCS* _dcs = new pfDCS;
	    	PFASSERTALWAYS(_dcs);

	    	int i, numChildren=((pfGroup*)_node)->getNumChildren();
	    
	    	printf("mySelector::select - scene has %d children\n", numChildren );
	    	for(i=0;i<numChildren;i++)
	    	{
			    pfNode* child = ((pfGroup*)_node)->getChild(0);
			    if(child!=NULL)
				    _dcs->addChild(child);
			    printf("mySelector::select - removing child from scene\n");
			    ((pfGroup*)_node)->removeChild(child);
	    	}
	    	((pfGroup*)_node)->addChild(_dcs);

	    	if(addToUndoList)
				undoMngr->add( new UndoableSelAction(this,node,_dcs));

	    	dcs = _dcs;
	    	node = (pfNode*)dcs;
	    	dragger->node = (pfNode*)dcs;
	    	dragger->nodeSetup((pfNode*)dcs);
	    	printf("SETTING SLOT for dcs %x\n", dcs );
	    	resizeGeom(dcs);
	    	dcs->addChild(dcsGeom);
	    	return;
		}

		pfGroup* parent = _node->getParent(0);
		if(parent==NULL) // pfScene?
		{
			printf("mySelector::select - SELECTED node has no parent\n");

			if(addToUndoList && (node!=NULL))
		    	undoMngr->add( new UndoableSelAction(this,node,NULL));

			node = NULL;
			dcs = NULL;
			return;
		}
	
		printf("mySelector::select - found parent %x\n", parent );
	
		if( (pfIsOfType(parent,pfDCS::getClassType())) &&
			(parent->getNumChildren()==1) )
		{
			printf("mySelector::select - parent is a DCS with 1 child\n");
			dcs = (pfDCS*)parent;
			
			if(addToUndoList)
			    undoMngr->add( new UndoableSelAction(this,node,_node));
		
			node = _node;
			dragger->node = (pfNode*)dcs;
			dragger->nodeSetup((pfNode*)dcs);
			printf("SETTING SLOT for dcs %x\n", dcs );
			resizeGeom(dcs);
			dcs->addChild(dcsGeom);
			return;
		}
	
	
		printf("mySelector::select - adding a DCS above node %x\n", _node );		
		pfDCS* _dcs = new pfDCS;
		PFASSERTALWAYS(_dcs);
		
		parent->replaceChild(_node,_dcs);
		_dcs->addChild(_node);
	
		if(addToUndoList)
		    undoMngr->add( new UndoableSelAction(this,node,_node));
	
		dcs = _dcs;
		node = _node;
		dragger->node = (pfNode*)dcs;
		dragger->nodeSetup((pfNode*)dcs);
		printf("SETTING SLOT for dcs %x\n", dcs );
		resizeGeom(dcs);
		dcs->addChild(dcsGeom);
		return;
    }
    
    printf("mySelector::select - all is NULL\n");
    
    if(addToUndoList)
		undoMngr->add( new UndoableSelAction(this,node,NULL));
	    
    node = NULL;
    dcs = NULL;	
}

///////////////////////////////////////////////////////////////////////////////

pfNode* mySelector::FIND_RECURSE_CHILD(pfGroup* parent, int childIndex)
{
	printf("FIND_RECURSE_CHILD - parent=%x, index=%d\n", parent,childIndex);
	
	pfNode* child = parent->getChild(childIndex);

	if(child==NULL)
	{
		printf("FIND_RECURSE_CHILD - NULL child\n");
		return NULL;
	}	
	
	if(pfIsOfType(child,pfGroup::getClassType()))
	{
		pfGroup* group = (pfGroup*)child;
		int i, numChildren = group->getNumChildren();
		
		if( numChildren == 1 )
		{
			pfNode* grandchild = FIND_RECURSE_CHILD(group,0);
			if(grandchild)
				return grandchild;
		}
		return child;
	}
	if(pfIsOfType(child,pfLightSource::getClassType()))
	{
		printf("FIND_RECURSE_CHILD - ignoring pfLightSource\n");
		return NULL;
	}
		
	printf("FIND_RECURSE_CHILD - child is of type %s\n", child->getTypeName());
	return child;
	
}

///////////////////////////////////////////////////////////////////////////////

pfNode* mySelector::FIND_FIRST_CHILD( pfNode* _node, int childIndex )
{
	printf("FIND_FIRST_CHILD - node=%x, index=%d\n", _node,childIndex);
	if(_node==NULL)
	{
		printf("FIND_FIRST_CHILD - node is NULL\n" );
		return NULL;
	}
		
	if(!pfIsOfType(_node,pfGroup::getClassType()))
	{
		printf("FIND_FIRST_CHILD - node is not a group\n" );
		return NULL;
	}

	pfGroup* group = (pfGroup*)_node;
		
	int i, numChildren = group->getNumChildren();
	pfNode* child = NULL;
	
	for( i=childIndex; i<numChildren; i++ )
	{
		child = FIND_RECURSE_CHILD(group,i);
		if(child)
		{
				
			printf("FIND_FIRST_CHILD - child is of type %s\n", child->getTypeName());
			return child;
		}
	}
	for( i=0; i<childIndex; i++ )
	{
		child = FIND_RECURSE_CHILD(group,i);
		if(child)
		{
				
			printf("FIND_FIRST_CHILD - child is of type %s\n", child->getTypeName());
			return child;
		}
	}

#if 0
	{	
		pfNode* child = group->getChild(i);
		if(child!=NULL)
		{
			if(pfIsOfType(child,pfGroup::getClassType()))
			{
				if( ((pfGroup*)child)->getNumChildren() == 1 )
				{
					pfNode* grandchild = FIND_FIRST_CHILD(child);
					if(grandchild)
						return grandchild;
				}
				return child;
			}
			if(pfIsOfType(child,pfLightSource::getClassType()))
			{
				printf("FIND_FIRST_CHILD - ignoring pfLightSource\n");
				continue;
			}
				
			printf("FIND_FIRST_CHILD - child is of type %s\n", child->getTypeName());
			return child;
		}
	}
#endif
	
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////

pfGroup* mySelector::FIND_PARENT( pfNode* child, int* childIndex )
{
	if(childIndex)
		*childIndex=-1;
	
	printf("mySelector::FIND_PARENT - root = %x child=%x\n", root, child );

	if(child==NULL)
	{
		printf("FIND_PARENT: NULL node\n");
		return NULL;
	}

	if(pfIsOfType(child,pfScene::getClassType()))
	{
		printf("FIND_PARENT - node is a pfScene\n" );
		return NULL;
	}

	if(child==(pfNode*)root)
	{
		printf("FIND_PARENT - selector's root node is already selected\n" );
		return NULL;
	}		

	pfGroup* parent = child->getParent(0);
	if(parent==NULL)
	{
		printf("FIND_PARENT: node has no parent0\n");
		return NULL;
	}

	printf("FIND_PARENT - parent is of type %s\n", parent->getTypeName());
	
	if(parent==root)
	{
		printf("FIND_PARENT - returning selector's root node\n" );
		if(childIndex)
			*childIndex = parent->searchChild(child);
		return parent;
	}

	if( parent->getNumChildren() == 1 )
	{
		pfGroup* grandpa = FIND_PARENT(parent,childIndex);
		if(grandpa)
		{
			printf("FIND_PARENT - grandpa is of type %s\n", grandpa->getTypeName());
			if(childIndex)
				printf("FIND_PARENT - returning childIndex=%d\n", *childIndex );
			return grandpa;
		}
		else
		{
			printf("FIND_PARENT - no grandpa found\n");
			return NULL;
		}
	}
	
	if(childIndex)
	{
		*childIndex = parent->searchChild(child);
		printf("FIND_PARENT - returning childIndex=%d\n", *childIndex );
	}

	return parent;
}

///////////////////////////////////////////////////////////////////////////////

pfNode* mySelector::FIND_NEXT_SIBLING( pfNode* _node )
{
	int childIndex;
	pfNode* sibling;
	
	pfGroup* parent = FIND_PARENT(_node, &childIndex);
	
	if(parent==NULL)
	{
		printf("FIND_NEXT_SIBLING - node has no parent\n");
		return NULL;
	}

	printf("FIND_NEXT_SIBLING - parent %x found (type=%s), childIndex=%d\n", 
				parent, parent->getTypeName(), childIndex);
	
	int numChildren = parent->getNumChildren();
	
	if(numChildren<=1)
	{
		printf("FIND_NEXT_SIBLING - parent only has %d children\n", numChildren );
		return NULL;
	}
	
	if(childIndex==(numChildren-1))
	{
		printf("FIND_NEXT_SIBLING - going back to child0\n");
		childIndex=0;
	}
	else
		childIndex++;

	int i;
	for(i=childIndex;i<numChildren;i++)
	{
		sibling = FIND_RECURSE_CHILD(parent,i);
		if(sibling)
		{
			if(sibling!=_node)
				return sibling;
			else
				return NULL;
		}
	}
	for(i=0;i<childIndex;i++)
	{
		sibling = FIND_RECURSE_CHILD(parent,i);
		if(sibling)
		{
			if(sibling!=_node)
				return sibling;
			else
				return NULL;
		}
	}	
	
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////


pfNode* mySelector::FIND_PREVIOUS_SIBLING( pfNode* _node )
{
	int childIndex;
	pfNode* sibling;
	
	pfGroup* parent = FIND_PARENT(_node, &childIndex);
	
	if(parent==NULL)
	{
		printf("FIND_NEXT_SIBLING - node has no parent\n");
		return NULL;
	}

	int numChildren = parent->getNumChildren();

	printf("FIND_PREVIOUS_SIBLING - parent %x found (type=%s), childIndex=%d\n", 
				parent, parent->getTypeName(), childIndex);

	if(numChildren<=1)
	{
		printf("FIND_NEXT_SIBLING - parent only has %d children\n", numChildren );
		return NULL;
	}
	
	if(childIndex==0)
	{
		printf("FIND_NEXT_SIBLING - going back to last child\n");
		childIndex=numChildren-1;
	}
	else
		childIndex--;

	int i;
	for(i=childIndex;i>=0;i--)
	{
		printf("FIND_NEXT_SIBLING - calling FIND_RECURSE_CHILD %x, %d\n",parent,i);
		sibling = FIND_RECURSE_CHILD(parent,i);
		if(sibling)
		{
			if(sibling!=_node)
				return sibling;
			else
				return NULL;
		}
	}
	for(i=numChildren-1;i>childIndex;i--)
	{
		sibling = FIND_RECURSE_CHILD(parent,i);
		if(sibling)
		{
			if(sibling!=_node)
				return sibling;
			else
				return NULL;
		}
	}	
	
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::specialFocus( pfvPicker* p,int ev,int prmsn,pfvPickerRequest*r )
{
	printf("mySelector::specialFocus - ev=%d (%c)\n", ev, ev );
	
	if(ev==LEFT_DOWN)
	{
		printf("mySelector::specialFocus - requesting selection\n");
		r->state = PFPICKER_SELECT | PFPICKER_FOCUS_EVENT;
		r->selector = this;

		if(!(state&PFPICKER_SELECT))
		{
			printf("mySelector::specialFocus - calling updateSelection\n");
			updateSelection(p,ev,r);
		}
	}
	return 1;	
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::updateSelection( pfvPicker* p, int ev, pfvPickerRequest* r )
{
	
	printf("mySelector::updateSelection - node=%x\n", node );
	
	if(ev)
		printf("mySelector::updateSelection -ev=%d (LEFT_DOWN=%d)\n", 
				ev, LEFT_DOWN );

	if(ev=='x') /* cut selected geometry */
	{
		if(node)
		{
			pfGroup* parent = node->getParent(0);
			if(parent)
			{
				printf("Deleting selected node!!\n");
				undoMngr->add( new UndoableDeleteAction(this,node));
			}
			else
				printf("Cannot delete selected Node (it has no parent)\n");
		}			
	}

	if(ev=='v') /* paste copied/cut geometry (from clipboard) */
	{
		if(mySelector::getClipboard())
		{
			undoMngr->add( new UndoablePasteAction(this,mySelector::getClipboard()));
		}			
	}

	if(ev=='c') /* copy selected geometry to clipboard */
	{
		if(node)
		{
			mySelector::setClipboard(node);

		}
	}

	else if(ev==LEFT_DOWN)
	{
		int btn = ((pfvInputMngrPicker*)p)->getMouse()->flags & PFUDEV_MOUSE_DOWN_MASK;
		printf("LEFT_DOWN! btn mask = %d (PFUDEV_MOUSE_LEFT_DOWN=%d)\n", 
				btn,PFUDEV_MOUSE_LEFT_DOWN );
		
		if(btn==PFUDEV_MOUSE_LEFT_DOWN)
		{
			pfvInteractor* p_ia = p->pick();

			if(p_ia == this )
			{
				pfNode *p_node;
				p->getPickResults( &p_node );

				printf("picked node=%x\n", p_node );
				select(p_node);
			}
			else if( p_ia == NULL )
			{
				printf("p_ia==NULL - deselecting\n");
				select(NULL);
			}	
		}
	}

	else if( ev==50 ) // '2' (in numeric keypad this is DOWN)
	{
		printf("DOWN ARROW: selecting first child\n");
		pfNode* child = FIND_FIRST_CHILD((pfGroup*)node);
		if(child)
		{
			select(child);
			/*
			pfuTravNodeHlight( node, NULL );
			pfuTravNodeHlight( child, hl_sel );
			node = child;
			*/
		}
	}
	
	
	else if( ev==52 ) // '4' (in numeric keypad this is LEFT)
	{
		printf("LEFT ARROW: selecting previous sibling\n");

		pfNode* sibling = FIND_PREVIOUS_SIBLING(/*node*/dcs);
		if(sibling)
		{
			select(sibling);
			/*
			pfuTravNodeHlight( node, NULL );
			pfuTravNodeHlight( sibling, hl_sel );
			node = sibling;
			*/
		}
	}

	
	else if( ev==54 ) // '6' (in numeric keypad this is RIGHT)
	{
		printf("RIGHT ARROW: selecting next sibling\n");
		
		pfNode* sibling = FIND_NEXT_SIBLING(/*node*/dcs);
		if(sibling)
		{
			select(sibling);
			/*
			pfuTravNodeHlight( node, NULL );
			pfuTravNodeHlight( sibling, hl_sel );
			node = sibling;
			*/
		}
	}

	
	else if( ev==56 ) // '8' (in numeric keypad this is UP)
	{
		printf("UP ARROW: selecting parent\n");
		if(node!=(pfNode*)root)
		{
			pfGroup* parent = FIND_PARENT(/*node*/dcs);
			if(parent)
			{
				select(parent);
			}
		}
	}
				

	else if( ev=='u' )
	{
	    printf("UNDO ACTION\n");
	    undoMngr->undo();
	}

	else if( ev=='r')
	{
	    printf("REDO ACTION\n");
	    undoMngr->redo();
	}
	
#if 0
	else if( ev=='p' )
	{
	    pfChannel* chan = ((pfvInputMngrPicker*)p)->chan;
	    pfVec3 xyz, hpr;
	    chan->getView(xyz,hpr);
	    printf("chan view: %f %f %f   %f %f %f\n", 
		xyz[0], xyz[1], xyz[2], hpr[0], hpr[1], hpr[2] );
	}
#endif
	
	//printSel();
	return 1;

}
	
///////////////////////////////////////////////////////////////////////////////

void mySelector::endSelection( pfvPicker* p )
{
	printf("mySelector::endSelection called\n");

	
	if(node)
	{
		select(NULL);
		
		//pfuTravNodeHlight( node, NULL );
		//node=NULL;
	}
	//deselectAll();
}

///////////////////////////////////////////////////////////////////////////////


