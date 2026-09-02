//
// Copyright 2002, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// This source code ("Source Code") was originally derived from a
// code base owned by Silicon Graphics, Inc. ("SGI")
// 
// LICENSE: SGI grants the user ("Licensee") permission to reproduce,
// distribute, and create derivative works from this Source Code,
// provided that: (1) the user reproduces this entire notice within
// both source and binary format redistributions and any accompanying
// materials such as documentation in printed or electronic format;
// (2) the Source Code is not to be used, or ported or modified for
// use, except in conjunction with OpenGL Performer; and (3) the
// names of Silicon Graphics, Inc.  and SGI may not be used in any
// advertising or publicity relating to the Source Code without the
// prior written permission of SGI.  No further license or permission
// may be inferred or deemed or construed to exist with regard to the
// Source Code or the code base of which it forms a part. All rights
// not expressly granted are reserved.
// 
// This Source Code is provided to Licensee AS IS, without any
// warranty of any kind, either express, implied, or statutory,
// including, but not limited to, any warranty that the Source Code
// will conform to specifications, any implied warranties of
// merchantability, fitness for a particular purpose, and freedom
// from infringement, and any warranty that the documentation will
// conform to the program, or any warranty that the Source Code will
// be error free.
// 
// IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
// LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
// ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
// SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
// OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
// PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
// OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
// USE, THE SOURCE CODE.
// 
// Contact information:  Silicon Graphics, Inc., 
// 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
// or:  http://www.sgi.com
//
//
// simple.C: simple Performer program for programmer's guide
//
// $Revision: 1.1 $ 
// $Date: 2002/03/05 03:11:59 $
//

/*****************************************************************************/

#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfScene.h>

#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfCombiner.h>

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#include <GL/glext.h>

/*****************************************************************************/

#define SIZE 15.0f
#define HALFSIZE (SIZE/2.0f)

/*****************************************************************************/

typedef struct GlobalData
{
    pfCombiner* cmb;
    pfGeoSet* gset;
    
}GlobalData;

static GlobalData* global = NULL;

/*****************************************************************************/

int
CmbPostDraw( pfTraverser *_trav, void *_userData )
{
    pfNotify( PFNFY_INFO, PFNFY_PRINT, "Disabling Combiners");
    pfDisable( PFEN_COMBINER );
    return 0;
}

/*****************************************************************************/

pfNode* CreateGeometry( char* tex1name, char* tex2name  /*, float BLEND_FACTOR */ ) 
{
    pfGeode* geode = new pfGeode;
    PFASSERTALWAYS(geode!=NULL);

    pfGeoSet* gset = new pfGeoSet;
    PFASSERTALWAYS(gset!=NULL);
    global->gset = gset;


    pfVec3 *coords = (pfVec3*) pfMalloc( 4*sizeof(pfVec3),pfGetSharedArena());
    PFASSERTALWAYS(coords!=NULL);
    coords[0].set( -HALFSIZE, 0.0f, -HALFSIZE );
    coords[1].set(  HALFSIZE, 0.0f,  -HALFSIZE );
    coords[2].set(  HALFSIZE, 0.0f,  HALFSIZE );
    coords[3].set( -HALFSIZE, 0.0f,  HALFSIZE );

    pfBox bbox;
    bbox.min.set( -HALFSIZE,0,-HALFSIZE );
    bbox.max.set( HALFSIZE,0,HALFSIZE );
    gset->setBound(&bbox, PFBOUND_STATIC);

    pfVec4 *colors = (pfVec4*) pfMalloc( 4*sizeof(pfVec4),pfGetSharedArena());
    PFASSERTALWAYS(colors!=NULL);
    colors[0].set(1.0f, 1.0f, 1.0f, 1.0f);
    colors[1].set(1.0f, 1.0f, 1.0f, 1.0f);
    colors[2].set(1.0f, 1.0f, 1.0f, 1.0f);
    colors[3].set(1.0f, 1.0f, 1.0f, 1.0f);

    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(1);

    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, colors, NULL);

    pfVec2 *tcoords = (pfVec2*)pfMalloc(4*sizeof(pfVec2),pfGetSharedArena());       
    PFASSERTALWAYS(tcoords!=NULL);          
    tcoords[0].set( 0.0f,  0.0f );    
    tcoords[1].set( 1.0f,  0.0f );     
    tcoords[2].set( 1.0f,  1.0f );
    tcoords[3].set( 0.0f,  1.0f );

    gset->setMultiAttr(PFGS_TEXCOORD2, 0, PFGS_PER_VERTEX, tcoords, NULL);
    gset->setMultiAttr(PFGS_TEXCOORD2, 1, PFGS_PER_VERTEX, tcoords, NULL);

    geode->addGSet(gset);
   
    pfGeoState* gstate = new pfGeoState;
    gstate->makeBasic();
    gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF );
    gstate->setMode(PFSTATE_CULLFACE, PFCF_OFF );

    gset->setGState(gstate);



    pfTexture* tex;


    /* texture 1 */
    if( tex1name==NULL || *tex1name=='\0' )
    {
      pfNotify( PFNFY_FATAL, PFNFY_PRINT,
	"Invalid texture1 name: \"%s\"", tex1name );
    }
    tex = new pfTexture;
    tex->setFilter(PFTEX_MINFILTER, PFTEX_BILINEAR);
    if(!tex->loadFile(tex1name))
    {
      pfNotify( PFNFY_FATAL, PFNFY_PRINT,
	"Failed to load texture1 %s", tex1name );
    }
    gstate->setMultiMode(PFSTATE_ENTEXTURE, 0, PF_ON );
    gstate->setMultiAttr(PFSTATE_TEXTURE, 0, tex );


    /* texture 2 */
    if( tex2name==NULL || *tex2name=='\0' )
    {
      pfNotify( PFNFY_FATAL, PFNFY_PRINT,
	"Invalid texture2 name: \"%s\"", tex2name );
    }
    tex = new pfTexture;
    tex->setFilter(PFTEX_MINFILTER, PFTEX_BILINEAR);
    if(!tex->loadFile(tex2name))
    {
      pfNotify( PFNFY_FATAL, PFNFY_PRINT,
	"Failed to load texture2 %s", tex2name );
    }
    gstate->setMultiMode(PFSTATE_ENTEXTURE, 1, PF_ON );
    gstate->setMultiAttr(PFSTATE_TEXTURE, 1, tex );



    pfCombiner* cmb = new pfCombiner();    
    global->cmb = cmb;

    cmb->setActiveCombiners(1);
    cmb->setActiveConstColors(1);

    cmb->setGeneralInput( GL_COMBINER0_NV, GL_RGB,
      GL_VARIABLE_A_NV, GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);

    cmb->setGeneralInput( GL_COMBINER0_NV, GL_RGB,
      GL_VARIABLE_B_NV, GL_TEXTURE0_ARB, GL_UNSIGNED_IDENTITY_NV, GL_RGB);

    cmb->setGeneralInput( GL_COMBINER0_NV, GL_RGB,
      GL_VARIABLE_C_NV, GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_INVERT_NV, GL_RGB);

    cmb->setGeneralInput( GL_COMBINER0_NV, GL_RGB,
      GL_VARIABLE_D_NV, GL_TEXTURE1_ARB, GL_UNSIGNED_IDENTITY_NV, GL_RGB);

    cmb->setGeneralOutput( GL_COMBINER0_NV, GL_RGB,
      GL_DISCARD_NV, GL_DISCARD_NV, GL_SPARE0_NV, GL_NONE, GL_NONE,
      GL_FALSE, GL_FALSE, GL_FALSE);


    cmb->setFinalInput( GL_VARIABLE_A_NV, GL_SPARE0_NV,
      GL_UNSIGNED_IDENTITY_NV, GL_RGB);

    cmb->setFinalInput( GL_VARIABLE_B_NV, GL_ZERO,
      GL_UNSIGNED_INVERT_NV, GL_RGB);

    cmb->setFinalInput( GL_VARIABLE_C_NV, GL_ZERO,
      GL_UNSIGNED_IDENTITY_NV, GL_RGB);

    cmb->setFinalInput( GL_VARIABLE_D_NV, GL_ZERO,
      GL_UNSIGNED_IDENTITY_NV, GL_RGB);

    gstate->setAttr( PFSTATE_COMBINER, cmb );
    gstate->setMode( PFSTATE_ENCOMBINER, PF_ON );

    pfGroup* group = new pfGroup;
    group->addChild(geode);
    group->setTravFuncs( PFTRAV_DRAW, NULL, CmbPostDraw );
    return (pfNode*)group;
}

/*****************************************************************************/

int
main (int argc, char *argv[])
{
    pfInit();

    if (argc != 3)
    {
    	pfNotify( PFNFY_FATAL, PFNFY_PRINT,
	    "Usage: %s texture1 texture2",argv[0] );
	exit(0);
    }

    global = (GlobalData*) pfCalloc( 1, sizeof(GlobalData),
	pfGetSharedArena());

    pfMultiprocess( PFMP_DEFAULT );			
    pfConfig();			

    pfFilePath(".:/usr/share/Performer/data");

    pfScene *scene = new pfScene;
    pfNode *root = CreateGeometry( argv[1], argv[2] );
    scene->addChild(root);
    
    // Create a pfLightSource and attach it to scene
    scene->addChild(new pfLightSource);
    
    
    // Configure and open GL window
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setName("OpenGL Performer");
    pw->setOriginSize(0,0,500,500);
    pw->open();
    
    // Create and configure a pfChannel.
    pfChannel *chan = new pfChannel(p);
    chan->setScene(scene);
    chan->setFOV(45.0f, 0.0f);
    
    pfCoord view;
    view.hpr.set(0.0f, -10.0f, 0.0f );
    view.xyz.set(0.0f, -20.0f, 5.0f );
    chan->setView(view.xyz, view.hpr);
    
    pfuInitInput(pw, PFUINPUT_X);
        
    
    // determine extent of scene's geometry
    pfSphere bsphere;
    root->getBound(&bsphere);
    chan->setNearFar(1.0f, 10.0f * bsphere.radius);
    
    
    while (TRUE)
    {
	pfuMouse mouse;
        float normX, normY;
	pfVec4 blendcol;    
	    
	// Go to sleep until next frame time.
	pfSync();		
	
	/* use mouse Y to determine blend factor */
	pfuGetMouse(&mouse);
	pfuCalcNormalizedChanXY( &normX, &normY, chan, mouse.xpos, mouse.ypos );
	
	blendcol.set( normY, normY, normY, 1.0f );
	global->cmb->setConstantColor0(blendcol);	
	

	// Initiate cull/draw for this frame.
	pfFrame();
	
    }
    
    // Terminate parallel processes and exit
    pfExit();
    return 0;
}

/*****************************************************************************/


