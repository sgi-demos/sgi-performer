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
// volume.C
// 
// A simple volumentric rendering example.

// The demo uses texture matrix operations in a draw callback to orient
// the volume data, the geometry never changes and neither does the eye
// position. Another way to get the same result would be to animate the
// plane equations in the texgen, either will work.

// The volume is clipped efficiently, by xforming gl clip planes to
// trim the volume for more pixel fill efficiency. Another method
// would be to modify the geoset geometry and clip the volume on
// the CPUs.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfEarthSky.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>

#define VOLSIZ 64 // use a 64x64x64 3d texture
#define NUM_QUADS 60 // number of quads used for volume shading
#define SLICE_OPACITY .1f // should vary inversely with NUM_QUADS

unsigned char *MakeImage()
// volume
{
   int i, s, t, r, radius;
   unsigned char *image;
   int nbytes = VOLSIZ * VOLSIZ * VOLSIZ;
   image = (unsigned char *) new(2*nbytes) pfMemory;


   for (i = 0; i<nbytes; i++) {
     s = i%VOLSIZ;
     t = (i/VOLSIZ)%VOLSIZ;
     r = (i/(VOLSIZ*VOLSIZ))%VOLSIZ;

     // stripe intensity throughout volume
     *(image+2*i) = 127 * (1.0f + sinf(s * .5f));

     // single black stripes on other axis
     if(!(r%10))
       *(image+2*i) = 0x00;

     // alpha  a 'wireline' cube around volume
     *(image+2*i+1) = 0xff;
     if(s < 1 || s > 62 || t < 1 || t > 62 || r < 1 || r > 62)
             *(image+2*i+1) = 0x00;

     if(   (s > 8 && s < 55 && t > 8 && t < 55)
        || (s > 8 && s < 55 && r > 8 && r < 55)
        || (t > 8 && t < 55 && r > 8 && r < 55))
             *(image+2*i+1) = 0x00;

     // a black dots around the centre
     if((s == 30 || s == 33) && (t == 30 || t == 33) && (r == 30 || r == 33) )
     {
             *(image+2*i) = 0x00;
             *(image+2*i+1) = 0xFF;
     }

     // 3 spheres (antialiasing by radial displacement filter)
     radius = (s-40)*(s-40) + (t-15) * (t-15) + (r-20) * (r-20);
     if( radius < 77 )
     {
       *(image+2*i+1) = 0x3F;
       if( radius < 72 )
       {
         *(image+2*i+1) = 0x7F;
         if( radius < 68 )
         {
           *(image+2*i+1) = 0xAF;
           if( radius < 64 )
             *(image+2*i+1) = 0xFF;
         }
       }
     }
     radius = (s-40)*(s-40) + (t-35) * (t-35) + (r-20) * (r-20);
     if( radius < 72 )
     {
       *(image+2*i+1) = 0x3F;
       if( radius < 60 )
         *(image+2*i+1) = 0x7F;
     }
     if( ((s-40)*(s-40) + (t-55) * (t-55) + (r-20) * (r-20)) < 60 )
       *(image+2*i+1) = 0x3F;

     // cylinder (antialiasing by radial displacement filter)
     radius = ((t-45)*(t-45) + (r-45) * (r-45));
     if( radius < 77 && s<53 && s>25)
     {
       *(image+2*i+1) = 0x3F;
       if( radius < 72 )
       {
         *(image+2*i+1) = 0x7F;
         if( radius < 68 )
         {
           *(image+2*i+1) = 0xAF;
           if( radius < 64 )
             *(image+2*i+1) = 0xFF;
         }
       }
     }


     // hollow cylinder (antialiasing by radial displacement filter)
     radius = (s-20)*(s-20) + (t-20)*(t-20);
     if( radius < 162 && r<55 && r>35 && radius > 39)
     {
       *(image+2*i+1) = 0x3F;
       if( radius < 156 && radius > 42.25)
       {
         *(image+2*i+1) = 0x7F;
         if( radius < 150 && radius > 45)
         {
           *(image+2*i+1) = 0xAF;
           if( radius < 144 && radius > 49)
             *(image+2*i+1) = 0xFF;
         }
       }
     }

   }
   return image;
}

int PreDraw(pfTraverser *, void *)
{
  int mmode;
  static float angle = 0.0f;
  pfMatrix matey;

  GLdouble peqn[4];
  pfVec3 peqn0(0.0f, 0.0f, 1.0f);
  pfVec3 peqn1(0.0f, 0.0f, -1.0f);
  pfVec3 peqn2(0.0f, 1.0f, 0.0f);
  pfVec3 peqn3(0.0f, -1.0f, 0.0f);
  pfVec3 peqn4(1.0f, 0.0f, 0.0f);
  pfVec3 peqn5(-1.0f, 0.0f, 0.0f);

  glGetIntegerv(GL_MATRIX_MODE, &mmode);
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();

  glDisable(GL_DEPTH_TEST);

  // translate to rotate about correct point
  glTranslatef(0.5f, 0.5f, 0.5f);

  // scale to fit inside geometric cube should be
  // a little smaller but I don't want to go to root 3
  glScalef(1.55f, 1.55f, 1.55f);

  // rotate to animate
  glRotatef( angle, -1.0f, -1.0f, 1.0f );

  // translate back
  glTranslatef(-0.5f, -0.5f, -0.5f);

  glMatrixMode(mmode);

  peqn[3] = .65;
  matey.makeRot( -angle, -1.0f, -1.0f, 1.0f );

  peqn0.xformVec( peqn0, matey );
  peqn[0] = peqn0[0];
  peqn[1] = peqn0[1];
  peqn[2] = peqn0[2];
  glClipPlane(GL_CLIP_PLANE0, peqn);
  glEnable(GL_CLIP_PLANE0);

  peqn1.xformVec( peqn1, matey );
  peqn[0] = peqn1[0];
  peqn[1] = peqn1[1];
  peqn[2] = peqn1[2];
  glClipPlane(GL_CLIP_PLANE1, peqn);
  glEnable(GL_CLIP_PLANE1);

  peqn2.xformVec( peqn2, matey );
  peqn[0] = peqn2[0];
  peqn[1] = peqn2[1];
  peqn[2] = peqn2[2];
  glClipPlane(GL_CLIP_PLANE2, peqn);
  glEnable(GL_CLIP_PLANE2);

  peqn3.xformVec( peqn3, matey );
  peqn[0] = peqn3[0];
  peqn[1] = peqn3[1];
  peqn[2] = peqn3[2];
  glClipPlane(GL_CLIP_PLANE3, peqn);
  glEnable(GL_CLIP_PLANE3);

  peqn4.xformVec( peqn4, matey );
  peqn[0] = peqn4[0];
  peqn[1] = peqn4[1];
  peqn[2] = peqn4[2];
  glClipPlane(GL_CLIP_PLANE4, peqn);
  glEnable(GL_CLIP_PLANE4);

  peqn5.xformVec( peqn5, matey );
  peqn[0] = peqn5[0];
  peqn[1] = peqn5[1];
  peqn[2] = peqn5[2];
  glClipPlane(GL_CLIP_PLANE5, peqn);
  glEnable(GL_CLIP_PLANE5);

  angle += 2.71f;
  if (angle > 360.0f)
    angle -= 360.0f;
  return PFTRAV_CONT;
}

int PostDraw(pfTraverser *, void *)
{
  int mmode;
  glGetIntegerv(GL_MATRIX_MODE, &mmode);
  glMatrixMode(GL_TEXTURE);
  glEnable(GL_DEPTH_TEST);
  glPopMatrix();
  glMatrixMode(mmode);
  glDisable(GL_CLIP_PLANE0);
  glDisable(GL_CLIP_PLANE1);
  glDisable(GL_CLIP_PLANE2);
  glDisable(GL_CLIP_PLANE3);
  glDisable(GL_CLIP_PLANE4);
  glDisable(GL_CLIP_PLANE5);
  return PFTRAV_CONT;
}


int
main (int argc, char *argv[])
{
    float       t = 0.0f;
    int i, ret;

    // Initialize Performer
    pfInit();
    
    pfQuerySys(PFQSYS_GL, &ret);
    if (ret != PFGL_OPENGL)
    {
	pfNotify(PFNFY_FATAL, PFNFY_PRINT, 
	"Sorry,  OpenGL is required.");
    }

    // Use default multiprocessing mode based on number of
    // processors.
    //
    pfMultiprocess( PFMP_DEFAULT );

    // Configure multiprocessing mode and start parallel
    // processes.
    //
    pfConfig();

    // Append to PFPATH files in /usr/share/Performer/data
    pfFilePath(".:/usr/share/Performer/data:/usr/demos/data/textures/");

    // Configure and open GL window
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setName("OpenGL Performer");
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setOriginSize(0, 0, 300, 300);
    pw->open();
    pfFrame();

    // Set up textures & gstates structures
    pfTexture *tex = new pfTexture;
    pfTexEnv *tev;
    tex->setFilter(PFTEX_MINFILTER, PFTEX_TRILINEAR);
    tex->setFilter(PFTEX_MAGFILTER, PFTEX_TRILINEAR);
    tex->setFormat(PFTEX_INTERNAL_FORMAT, PFTEX_IA_8);
    tex->setRepeat(PFTEX_WRAP_S, PFTEX_CLAMP);
    tex->setRepeat(PFTEX_WRAP_T, PFTEX_CLAMP);
    tex->setRepeat(PFTEX_WRAP_R, PFTEX_CLAMP);

    pfGeoState *gstate = new pfGeoState;
    unsigned char *image;
    if (image = MakeImage())
    {
	int nc, sx, sy, sz;

	nc = 2; sx = VOLSIZ; sy = VOLSIZ; sz = VOLSIZ;
	tex->setImage((uint *) (image), nc, sx, sy, sz);

	gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_BLEND_ALPHA);
	// set alpha function to block pixels of 0 alpha for
	//   transparent textures
	gstate->setMode(PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
	gstate->setVal(PFSTATE_ALPHAREF, 0.0f);
	gstate->setAttr(PFSTATE_TEXTURE, tex);
	gstate->setMode(PFSTATE_ENTEXTURE, PF_ON);
	gstate->setMode(PFSTATE_ENLIGHTING,0);
	gstate->setMode(PFSTATE_CULLFACE,PFCF_OFF);

	tev = new pfTexEnv;
	gstate->setAttr(PFSTATE_TEXENV, tev);

    }

    // Set up geosets
    pfVec3 *coords = (pfVec3*) new((4*NUM_QUADS)*sizeof(pfVec3)) pfMemory;
    for(i = 0; i < (4*NUM_QUADS); i+=4)
    {
      static float t = 1.0f;
      coords[i].set(  -1.0f,  t,  -1.0f);
      coords[i+1].set( 1.0f,  t,  -1.0f );
      coords[i+2].set( 1.0f,  t,   1.0f );
      coords[i+3].set(-1.0f,  t,   1.0f );
      t -= 2.0f / (NUM_QUADS-1.0f);
    }

    pfVec4 *colors = (pfVec4*) new(sizeof(pfVec4)) pfMemory;
    (*colors).set(1.0f, 1.0f, 1.0f, SLICE_OPACITY);


    // try using TexGen to gain access to 3D texture map hardware
    // tell the geoState about texgen
    pfTexGen *texgen = new pfTexGen;
    gstate->setMode(PFSTATE_ENTEXGEN, PF_ON);
    gstate->setAttr(PFSTATE_TEXGEN, texgen);
    texgen->setMode(PF_S, PFTG_OBJECT_LINEAR);
    texgen->setMode(PF_T, PFTG_OBJECT_LINEAR);
    texgen->setMode(PF_R, PFTG_OBJECT_LINEAR);
    texgen->setMode(PF_Q, PFTG_OFF);
    texgen->setPlane(PF_S, 0.5f, 0.0f, 0.0f, 0.5f);
    texgen->setPlane(PF_T, 0.0f, 0.5f, 0.0f, 0.5f);
    texgen->setPlane(PF_R, 0.0f, 0.0f, 0.5f, 0.5f);


    pfGeoSet *gset = new pfGeoSet;
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, 0);
    gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, colors, 0);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(NUM_QUADS);
    gset->setGState(gstate);
    // set up scene graph
    pfGeode *geode1 = new pfGeode;
    geode1->addGSet(gset);

    pfGroup *root = new pfGroup;
    root->addChild(geode1);
    root->setTravFuncs(PFTRAV_DRAW, PreDraw, PostDraw);

    pfScene *scene = new pfScene;
    scene->addChild(root);

    // determine extent of scene's geometry
    pfSphere bsphere;
    scene->getBound(&bsphere);

    // Create and configure a pfChannel.
    pfChannel *chan = new pfChannel(p);
    chan->setScene(scene);
    chan->setNearFar(0.5f, 10.0f * bsphere.radius);
    // 45 degrees wide,  vertical=-1 to signal match window aspect
    chan->setFOV(45.0f, -1.0f);

    // Create an earth/sky model that draws sky/ground/horizon
    {
	pfEarthSky *esky = new pfEarthSky;
	esky->setMode(PFES_BUFFER_CLEAR, PFES_SKY_GRND );
	esky->setAttr(PFES_GRND_HT, -1.0f * bsphere.radius);
	esky->setColor(PFES_GRND_FAR, 0.3f, 0.1f, 0.0f, 1.0f);
	esky->setColor(PFES_GRND_NEAR, 0.5f, 0.3f, 0.1f, 1.0f);
	chan->setESky(esky);
    }

    pfCoord view;

    // Compute new view position for next frame.
    view.hpr.set(0.0f, 0.0f, 0.0f);
    view.xyz.set(0.0f, -3.0f * bsphere.radius, 0.0f);

    // set view position for next frame
    chan->setView(view.xyz, view.hpr);

    // Simulate for twenty seconds.
    while (t < 20.0f)
    {
	// Go to sleep until next frame time.
	pfSync();

	// Initiate cull/draw for this frame.
	pfFrame();

	t = pfGetTime();
    }

    // Terminate parallel processes and exit.
    pfExit();

    return 0;
}

