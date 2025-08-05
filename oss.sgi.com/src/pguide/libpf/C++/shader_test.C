//
// Copyright 1995, Silicon Graphics, Inc.
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
// shader_test.C:  Basic shader test example
//
// $Revision: 1.1 $ 
// $Date: 2000/11/21 21:39:38 $
//

#include <stdlib.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>

#include <Performer/pf/pfShader.h>
#include <Performer/pf/pfShaderManager.h>

#include <Performer/pfdu.h>

#include <Performer/pfutil.h>

///////////////////////////////////////////////////////////////////////////
// main
///////////////////////////////////////////////////////////////////////////

pfGeoSet* makeTorus (float rMajor, float rMinor,
		     int nDivMajor, int nDivMinor)
{
    pfGeoSet *gs;
    pfVec3 *coords;
    pfVec3 *normals;
    pfVec2 *texCoords;
    int *primLengths;

    float x1, y1, z1;
    float x2, y2, z2;
    float i1, j1, k1;
    float i2, j2, k2;
    float s1, t1;
    float s2, t2;

    float theta;
    float thetaMin = 0.0;
    float thetaMax = 360.0;
    float dTheta = (thetaMax - thetaMin) / (float) nDivMajor;
    float sinTheta, cosTheta;

    float omega;
    float omegaMin = 0.0;
    float omegaMax = 360.0;
    float dOmega = (omegaMax - omegaMin) / (float) nDivMinor;
    float sinOmega, cosOmega;

    int numStrips = nDivMajor;
    int numVertsPerStrip = 2 * (nDivMinor + 1);

    gs = new pfGeoSet;
    coords = (pfVec3*) pfMalloc (sizeof(pfVec3) * 
				 numStrips * numVertsPerStrip,
				 pfGetSharedArena ());
    normals = (pfVec3*) pfMalloc (sizeof(pfVec3) * 
				  numStrips * numVertsPerStrip,
				  pfGetSharedArena ());
    texCoords = (pfVec2*) pfMalloc (sizeof(pfVec3) * 
				    numStrips * numVertsPerStrip,
				    pfGetSharedArena ());
    primLengths = (int*) pfMalloc (sizeof (int) * numStrips, 
				   pfGetSharedArena ());

    gs->setPrimType (PFGS_TRISTRIPS);
    gs->setNumPrims (numStrips);
    gs->setPrimLengths (primLengths);
    gs->setAttr (PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gs->setAttr (PFGS_NORMAL3, PFGS_PER_VERTEX, normals, NULL);
    gs->setAttr (PFGS_TEXCOORD2, PFGS_PER_VERTEX, texCoords, NULL);

    theta = thetaMin;
    for (int divMaj = 0; divMaj < nDivMajor; divMaj++) {
	omega = omegaMin;
	primLengths [divMaj] = numVertsPerStrip;
	for (int divMin = 0; divMin <= nDivMinor; divMin++) {
	    // compute all params for first vertex
	    pfSinCos (theta, &sinTheta, &cosTheta);	    
	    pfSinCos (omega, &sinOmega, &cosOmega);
	    
	    x1 = cosTheta * (rMajor - rMinor * cosOmega);
	    y1 = rMinor * sinOmega;
	    z1 = sinTheta * (rMajor - rMinor * cosOmega);

	    i1 = x1 - cosTheta * rMajor;
	    j1 = y1;
	    k1 = z1 - sinTheta * rMajor;

	    s1 = (theta - thetaMin) / (thetaMax - thetaMin);
	    t1 = (omega - omegaMin) / (omegaMax - omegaMin);

	    // compute all params for second vertex.  it's
	    // one step away in the theta direction.
	    pfSinCos (theta + dTheta, &sinTheta, &cosTheta);	    
	    
	    x2 = cosTheta * (rMajor - rMinor * cosOmega);
	    y2 = y1;
	    z2 = sinTheta * (rMajor - rMinor * cosOmega);

	    i2 = x2 - cosTheta * rMajor;
	    j2 = y2;
	    k2 = z2 - sinTheta * rMajor;

	    s2 = (theta + dTheta - thetaMin) / (thetaMax - thetaMin);
	    t2 = t1;

	    coords[divMaj * numVertsPerStrip + 2 * divMin + 0].
		set (x1, y1, z1);
	    coords[divMaj * numVertsPerStrip + 2 * divMin + 1].
		set (x2, y2, z2);

	    normals[divMaj * numVertsPerStrip + 2 * divMin + 0].
		set (i1, j1, k1);
	    normals[divMaj * numVertsPerStrip + 2 * divMin + 1].
		set (i2, j2, k2);

	    texCoords[divMaj * numVertsPerStrip + 2 * divMin + 0].
		set (s1, t1);
	    texCoords[divMaj * numVertsPerStrip + 2 * divMin + 1].
		set (s2, t2);

	    omega += dOmega;
	}
	theta += dTheta;
    }

    return (gs);
}

int
main (int argc, char *argv[])
{
    if ((argc < 2) ||
	(argc > 3)) {
	pfNotify (PFNFY_ALWAYS, PFNFY_PRINT,
		  "USAGE: %s <shader_file> [<model_file>]\n", argv[0]);
	exit (-1);
    }

    // Initialize and configure OpenGL Performer
    pfInit();
    pfMultiprocess( PFMP_DEFAULT );

    if (argc == 3) {
	pfdInitConverter (argv[2]);
    }

    pfConfig();

    pfShaderManager *shaderManager = pfGetShaderManager ();
    
    // Look for files in PFPATH, ".", and  "/usr/share/Performer/data"
    pfFilePath(".:/usr/share/Performer/data:/usr/share/Performer/data/shaders");
    
    // Create a scene
    pfScene *scene = new pfScene;
       
    pfDCS *root = new pfDCS;
    pfNode *model;
    if (argc == 3) {
	// load the specified file
	model = pfdLoadFile (argv[2]);
    } else {
	// create a sphere 
	model = new pfGeode;
	pfGeoSet *torusGSet = makeTorus (3, 1, 20, 20);
	torusGSet->setBound (NULL, PFBOUND_DYNAMIC);
	((pfGeode*)model)->addGSet (torusGSet);
    }

    root->addChild (model);

    // load a shader and apply it
    pfShader *shader1 = pfdLoadShader (argv[1]);

    if (shader1 != NULL) {
	shaderManager->applyShader (root, shader1);
	shaderManager->resolveShaders (root);
    } else {
	pfNotify (PFNFY_WARN, PFNFY_RESOURCE, 
		  "pfdLoadShader returned NULL shader.  Probably due to a "
		  "parse error.\n");
    }

    // add the root to the scene
    scene->addChild (root);
    
    // Create and configure a pfPipe and pfChannel.
    pfPipe *pipe = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(pipe);
    int fbAttrs [] = {
            GLX_RGBA,
	    GLX_DOUBLEBUFFER,
            GLX_RED_SIZE, 1,
            GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1,
            GLX_ALPHA_SIZE, 1,	
            GLX_DEPTH_SIZE, 1,
            GLX_STENCIL_SIZE, 1, 
	    None
    };
    pw->setFBConfigAttrs (fbAttrs);
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setName("pfShader Test");
    pw->setOriginSize(10,10,512,512);
    pw->open();

    pfChannel *chan = new pfChannel(pipe);
    chan->setFOV(60.0f, 0.0f);
    chan->setScene(scene);
    
    // Determine extent of scene's geometry.
    pfSphere bsphere;
    root->getBound(&bsphere);
    chan->setNearFar(1.0f, 10.0f*bsphere.radius);
    
    // set view
    pfCoord view;
    view.hpr.set(0, 0, 0);
    view.xyz.set(0, -15, 0);
    chan->setView(view.xyz, view.hpr);

    float h = 0;
    float p = 0;
    float r = 0;

    // rotate model indefinitely until someone kills the program
    while (1)
    {
	root->setRot (h, p, r);
	h+=0.25;
	p+=0.25;

	// Initiate cull/draw processing for this frame.
	pfFrame();
    }
    
    // Terminate parallel processes and exit.
    pfExit();
    return 0;
}
