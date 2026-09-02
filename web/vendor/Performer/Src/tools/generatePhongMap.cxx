//
// Copyright 1995, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// Permission to use, copy, modify, distribute, and sell this software
// and its documentation for any purpose is hereby granted without
// fee, provided that (i) the above copyright notices and this
// permission notice appear in all copies of the software and related
// documentation, and (ii) the name of Silicon Graphics may not be
// used in any advertising or publicity relating to the software
// without the specific, prior written permission of Silicon Graphics.
//
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//
// IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL,
// INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY
// DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY
// THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE
// OR PERFORMANCE OF THIS SOFTWARE.
//
//
// simpleC.C: simple Performer program for programmer's guide
//
// $Revision: 1.2 $ 
// $Date: 2002/11/20 19:45:12 $
//



#include <stdlib.h>
#include <stdio.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfLight.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pfdu.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfLightSource.h>

int keepGoing = 1;

int size;

static void 
cbdraw(pfChannel *chan, void *data) 
{ 
  pfDraw(); 
  
  unsigned int *texels = (unsigned int *)pfMalloc(size * size * sizeof(unsigned int), pfGetSharedArena());
  
  glReadPixels(0, 0, size, size, GL_RED, GL_UNSIGNED_BYTE, texels);
  
  pfTexture *tex = (pfTexture *)data;
  tex->setImage(texels, 1, size, size, 1);
  tex->setFormat(PFTEX_EXTERNAL_FORMAT, PFTEX_PACK_8);
  keepGoing = 0;
}

int
main (int argc, char *argv[])
{
  if(argc < 4) {
    fprintf(stderr, "Usage: %s <map size> <shininess (0-128)> <texture name>\n",
	    argv[0]);
    exit(-1);
  }
    
  size = atoi(argv[1]);
  float exponent = atof(argv[2]);

  if((size - 1) & size) {
    fprintf(stderr, "Please specify a size that is a power of two\n");
    exit(-1);
  }
  
  if((exponent < 0.0) || (exponent > 128.0)) {
    fprintf(stderr, "Shininess must be in the range 0-128\n");
    exit(-1);
  }
  
  fprintf(stdout, "generating 1 component texture \"%s\": (%dx%d) with "
	  "shininess %f\n", argv[3], size, size, exponent);

  // Initialize Performer
  pfInit();
  pfMultiprocess( PFMP_APPCULLDRAW);
  pfConfig();		
  
  pfPipe *p = pfGetPipe(0);
  pfPipeWindow *pw = new pfPipeWindow(p);
  
  int fbAttrs [] = {
    PFFB_RGBA,
    PFFB_RED_SIZE, 8,
    PFFB_DEPTH_SIZE, 1,
    NULL
  };
  pw->setFBConfigAttrs (fbAttrs);
  pw->setWinType(PFPWIN_TYPE_X);
  pw->setName("specular map generator");
  pw->setOriginSize(50,50,size,size);
  pw->open();
  
  pfChannel *chan = new pfChannel(p);
  chan->makeOrtho(-1, 1, -1, 1);
  chan->setNearFar(-3, 3);
  
  pfGeoSet *gset = pfdNewSphere(50000, pfGetSharedArena());
  
  pfGeoState *gstate = new pfGeoState();
  
  pfMaterial *mtl = new pfMaterial();
  mtl->setColor(PFMTL_AMBIENT,  0, 0, 0);
  mtl->setColor(PFMTL_DIFFUSE,  0, 0, 0);
  mtl->setColor(PFMTL_EMISSION, 0, 0, 0);
  mtl->setColor(PFMTL_SPECULAR, 1, 0, 0);
  mtl->setColorMode(PFMTL_BOTH, PFMTL_CMODE_OFF);
  mtl->setShininess(exponent);
  
  gstate->setAttr(PFSTATE_FRONTMTL, mtl);
  gstate->setMode(PFSTATE_ENLIGHTING, PF_ON);
  gset->setGState(gstate);
  
  pfGeode *geode = new pfGeode();
  geode->addGSet(gset);
  
  pfScene *scene = new pfScene();
  scene->addChild(geode);
  
  chan->setScene(scene); 

  pfLightSource *ls = new pfLightSource();
  ls->setColor(PFLT_SPECULAR, 1, 0, 0);
  ls->setColor(PFLT_DIFFUSE,  0, 0, 0);
  ls->setColor(PFLT_AMBIENT,  0, 0, 0);
  ls->setPos(0, -3, 0, 0);
  ls->on();
  
  scene->addChild(ls);
  pfCoord view;
  view.hpr.set(0, 0, 0);
  view.xyz.set(0, -3, 0);
  chan->setView(view.xyz, view.hpr);
  
  chan->setTravFunc(PFTRAV_DRAW, cbdraw);
  
  pfTexture *tex = new pfTexture();
  
  chan->setChanData(tex, sizeof(pfTexture *));
  chan->passChanData();
  
  while(keepGoing) {
    pfSync();
    pfFrame();
  }
  
  tex->saveFile(argv[3]);
  
  pfExit();
}


