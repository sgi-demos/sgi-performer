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
// text.C:    simple Performer program to demonstrate using 
//             GL text
//
// $Revision: 1.17 $ 
// $Date: 2002/11/14 01:40:18 $ 
//
//

#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pfdu.h>


static void DrawChannel(pfChannel *chan, void *data);

pfSphere *bsphere;



//
// Would normally use pfuXFont routines, see pfuXFont man page
//

static char fontName[] ="-*-courier-bold-r-normal--14-*-*-*-m-90-iso8859-1";
#ifndef WIN32
static int fontDLHandle;
static XFontStruct *fontInfo;

#define FIRSTGLYPH	32
#define LASTGLYPH	128

#else
static pfuXFont fnt;
#endif

#ifndef WIN32
void loadXFont(void)
{
    Display *xdisplay;

	// get font from X Server
    xdisplay = pfGetCurWSConnection();
    fontInfo = XLoadQueryFont(xdisplay, fontName);
    if(fontInfo == NULL){
        pfNotify(PFNFY_FATAL, PFNFY_USAGE, "loadXFont: Couldn't load X font\n");
        exit(1);
    }

	// Create GL display lists for font
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "loadXFont: making X font display "
        "lists\n");
    fontDLHandle = glGenLists((GLuint) LASTGLYPH + 1);
    if(fontDLHandle == 0)
    {
        pfNotify(PFNFY_FATAL, PFNFY_USAGE, "loadXFont: Couldn't get %d "
            "display lists\n", LASTGLYPH + 1);
        exit(1);
    }
    glXUseXFont(fontInfo->fid, FIRSTGLYPH, LASTGLYPH - FIRSTGLYPH + 1, 
        fontDLHandle + FIRSTGLYPH);
}

void drawXFontString(char *s)
{
    glPushAttrib(GL_LIST_BIT);
    glListBase(fontDLHandle);
    glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *)s);
    glPopAttrib();
}
#endif


static void OpenPipeWin(pfPipeWindow *pw)
{
    pw->open();

#ifndef WIN32
    loadXFont();
#else
    pfuMakeRasterXFont(fontName, &fnt);
    pfuSetXFont(&fnt);
#endif
}


static void DrawChannel (pfChannel *channel, void *)
{
    static pfMatrix tempmat;
    
    channel->clear();
    pfDraw();
    

    glDepthFunc(GL_ALWAYS); // always draw
    glDepthMask(GL_FALSE);

    
    // text moving with the scene
    pfPushState();
    
    pfBasicState();
    

    glColor3ub(255,255,255);
#ifndef WIN32
    glRasterPos3f(0.5f * bsphere->radius, 0.0f, 0.5f * bsphere->radius);
    drawXFontString("Text-in-Scene");
#else
    pfuDrawStringPos("Text-in-Scene", 
		     0.5f * bsphere->radius, 0.0f, 0.5f * bsphere->radius);
#endif

    
    pfPopState();
    
    // text overlaid on top of scene, like a HUD
    pfPushState();
    pfBasicState();
    

    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)tempmat.mat);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat *)pfIdentMat.mat);
    glOrtho(-10.0, 10.0, -10.0, 10.0, 1.0, -1.0);

    glMatrixMode(GL_MODELVIEW);

    
    pfPushMatrix();
    pfLoadMatrix(pfIdentMat);


#ifndef WIN32
    glRasterPos2f (-3.0f, 0.0f);
    drawXFontString("text over scene");
#else
    pfuDrawStringPos("text over scene", -3.0f, 0.0f, 0.0f);
#endif


    pfPopMatrix();
    

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat *)tempmat.mat);
    glMatrixMode(GL_MODELVIEW);


    pfPopState();
    

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);


}

//
//	Usage() -- print usage advice and exit. This
//      procedure is executed in the application process.
//
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: text file.ext ...\n");
    exit(1);
}


int
main (int argc, char *argv[])
{
    float       t = 0.0f;
    
    if (argc < 2)
	Usage();
    
    // Initialize Performer
    pfInit();	
    
    // Use default multiprocessing mode based on number of
    // processors.
    //
    pfMultiprocess( PFMP_DEFAULT );
    bsphere = (pfSphere *)pfMalloc(sizeof(pfSphere),pfGetSharedArena());
    
    // Load all loader DSO's before pfConfig() forks 
    pfdInitConverter(argv[1]);

    // Configure multiprocessing mode and start parallel
    // processes.
    //
    pfConfig();			
    
    // Append to PFPATH additional standard directories where 
    // geometry and textures exist 
    //
    pfFilePath(".:/usr/share/Performer/data");
    
    // Read a single file, of any known type.
    pfNode *root = pfdLoadFile(argv[1]);
    if (root == NULL) 
    {
	pfExit();
	exit(-1);
    }
    
    // Attach loaded file to a pfScene.
    pfScene *scene = new pfScene;
    scene->addChild(root);
    
    // determine extent of scene's geometry
    scene->getBound(bsphere);
    
    // Create a pfLightSource and attach it to scene.
    scene->addChild(new pfLightSource);
    
    // Configure and open GL window
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setName(argv[0]);
    pw->setOriginSize(0, 0, 500, 500);
    // Open and configure the GL window.
    pw->setConfigFunc(OpenPipeWin);
    pw->config();
    
    // Create and configure a pfChannel.
    pfChannel *chan = new pfChannel(p);	
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);
    chan->setScene(scene);
    chan->setNearFar(1.0f, 10.0f * bsphere->radius);
    chan->setFOV(45.0f, 0.0f);
    
    // Simulate for twenty seconds.
    while (t < 20.0f)
    {
	float      s, c;
	pfCoord	   view;
	
	// Go to sleep until next frame time.
	pfSync();		
	
	// Compute new view position.
	t = pfGetTime();
	pfSinCos(45.0f*t, &s, &c);
	view.hpr.set(45.0f*t, -10.0f, 0);
	view.xyz.set(2.0f * bsphere->radius * s, 
		     -2.0f * bsphere->radius *c, 
		     0.5f * bsphere->radius);
	chan->setView(view.xyz, view.hpr);
	
	// Initiate cull/draw for this frame.
	pfFrame();		
    }
    
    // Terminate parallel processes and exit.
    pfExit();
    
    return 0;
}
