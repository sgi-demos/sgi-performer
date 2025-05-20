/*
 * Copyright 1995-1996-1997, Silicon Graphics, Inc.
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
 *
 *
 * $Revision: 1.6 $ $Date: 2002/11/11 00:55:26 $ 
 *
 */



#include <stdlib.h>
#include <math.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>
#include <Performer/pf/pfPipe.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfLight.h>

#ifndef WIN32
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xmu/StdCmap.h>
#include <X11/Xresource.h>
#include <X11/Xlibint.h>
#endif
#ifdef __linux__
#include <asm/byteorder.h>
#endif

static int FBAttrs[] = {
  PFFB_RGBA,
  PFFB_DOUBLEBUFFER,
  PFFB_BUFFER_SIZE, 24,
  PFFB_DEPTH_SIZE, 23,
  PFFB_RED_SIZE, 8,
  PFFB_GREEN_SIZE, 8,
  PFFB_BLUE_SIZE, 8,
  NULL,
};

/*
  This function creates a pfGeode containing two pfGeoSets; one representing
  a floor, and one representing a rear wall. 
  */

static pfGeode*
initBackground() {
  void *arena = pfGetSharedArena();
  pfGeoSet *wallgset = new pfGeoSet();
  pfGeoSet *floorgset = new pfGeoSet();
  

  //Vertex lists for the wall and the floor
  pfVec3 *v1 = (pfVec3 *)pfMalloc(4 * sizeof(pfVec3), arena);
  pfVec3 *v2 = (pfVec3 *)pfMalloc(4 * sizeof(pfVec3), arena);
  
  //normals for the wall and floor
  pfVec3 *n1, *n2;
  n1 = (pfVec3 *)pfMalloc(sizeof(pfVec3), arena);
  n2 = (pfVec3 *)pfMalloc(sizeof(pfVec3), arena);

  pfGeode *geode = new pfGeode();
  
  pfVec4 *color = (pfVec4 *)pfMalloc(sizeof(pfVec4), arena);
  
  /*
    Make the walls a neutral grey color
    */
  pfMaterial *m = new pfMaterial();
  m->setShininess(100.0);
  m->setColor(PFMTL_AMBIENT,  0.1, 0.1, 0.1);
  m->setColor(PFMTL_DIFFUSE,  0.5, 0.5, 0.5);
  m->setColor(PFMTL_EMISSION, 0.0, 0.0, 0.0);
  m->setColor(PFMTL_SPECULAR, 0.5, 0.5, 0.5);

  pfGeoState *gstate = new pfGeoState();
  gstate->setAttr(PFSTATE_FRONTMTL, m);
  gstate->setMode(PFSTATE_ENLIGHTING, 1);
  
  color->set(1.0, 1.0, 1.0, 1.0);
  
  //floor vertices
  v1[0].set(-1,  1,  -1);
  v1[1].set(-1, -1,  -1);
  v1[2].set( 1, -1,  -1);
  v1[3].set( 1,  1,  -1);

  floorgset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, v1, NULL);  
  floorgset->setGState(gstate);
  
  n1->set(0, 0, 1);
  floorgset->setAttr(PFGS_NORMAL3, PFGS_OVERALL, n1, NULL);
  floorgset->setAttr(PFGS_COLOR4, PFGS_OVERALL, color, NULL);
  floorgset->setPrimType(PFGS_QUADS);
  floorgset->setNumPrims(1);
  geode->addGSet(floorgset);
  
  //rear wall vertices
  v2[0].set(-1,  1,  1);
  v2[1].set(-1,  1, -1);
  v2[2].set( 1,  1, -1);
  v2[3].set( 1,  1,  1);

  wallgset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, v2, NULL);  
  wallgset->setGState(gstate);
  n2->set(0, -1, 0);
  wallgset->setAttr(PFGS_NORMAL3, PFGS_OVERALL, n2, NULL);
  wallgset->setAttr(PFGS_COLOR4, PFGS_OVERALL, color, NULL);
  wallgset->setPrimType(PFGS_QUADS);
  wallgset->setNumPrims(1);
  
  geode->addGSet(wallgset);

  return geode;
}


#define SPOT_SIZE 512 //the size of the projective texture
#define SPOT_RADIUS 200 //the radius of the light lobe in the texture


/*
  Calculate the lobe light intensity for a point in the top left quadrant
  of the light lobe texture. The function below produces a texture with a
  slight dark spot in the center.
  */
float 
spotIntensity(int i,
	      int j) {
  float angle = atan2(j, i);
  float r = sqrt(i * i + j * j) / (float)SPOT_RADIUS;
    
  float intensity = 1.0 - r * r;
  
  if( r <= 0.3 ) intensity *=  (0.5 + 0.5 * (r / 0.3));

  return intensity;
}

/*
  Here, the light lobe texture is generated
  */

pfTexture *
initTexture() {
  long            	i, j;
  pfTexture		*tex;
  unsigned short 	*image;
  
  //allocate a buffer for image data
  image =  
    (unsigned short *)pfMalloc(sizeof(unsigned short) * SPOT_SIZE * SPOT_SIZE,
			       pfGetSharedArena());
  
  /*
    Loop through the top left quadrant of the light lobe texture, and
    generate intensities using the spotIntensity function above. The values
    for the top left quadrant are mirrored to the other quadrants 
    */
  
  for (i = 0; i < SPOT_SIZE / 2; i++)
    {
      for (j = 0; j < SPOT_SIZE / 2; j++)
	{
	  unsigned short val;
	  val = 0;
	  
	  if ((i * i + j * j) <= SPOT_RADIUS * SPOT_RADIUS)
	    val = (unsigned short)(spotIntensity(i ,j) * 65535.0f);
	  unsigned char tmp;
#ifdef __LITTLE_ENDIAN
	  tmp = val >> 8;
	  val = val << 8 | tmp;
#endif

	  /* Set all 4 quadrants to val */
	  image[(SPOT_SIZE / 2 - i) * SPOT_SIZE + (SPOT_SIZE / 2 - j)] =
	    image[(SPOT_SIZE / 2 - i) * SPOT_SIZE + (SPOT_SIZE / 2 + j)] =
	    image[(SPOT_SIZE / 2 + i) * SPOT_SIZE + (SPOT_SIZE / 2 + j)] =
	    image[(SPOT_SIZE / 2 + i) * SPOT_SIZE + (SPOT_SIZE / 2 - j)] =
	    val;
	}
    }
  
  
  tex = new pfTexture();
  tex->setImage((uint*) image, 2, SPOT_SIZE, SPOT_SIZE, 0);
  tex->setFormat(PFTEX_INTERNAL_FORMAT, PFTEX_IA_8);
  tex->setRepeat(PFTEX_WRAP, PFTEX_CLAMP);
  
  return tex;
}

static void
lookAt(pfDCS *dcs, pfVec3 at)
{
  pfVec3		v, pos;
  pfMatrix		mat;
  pfVec3	yaxis;
  
  yaxis.set(0.0f, 1.0f, 0.0f);
    
  /* v is origin of dcs */
  dcs->getMatPtr()->getRow(3, pos);
  
  /* v is direction vector from dcs to 'at' */
  v.sub(at, pos);
  v.normalize();
  
  /* Rotate y-axis onto v */
  mat.makeVecRotVec(yaxis, v);
  mat.setRow(3, pos); /* restore dcs origin */
  dcs->setMat(mat);
}



#define FOV 55

int main(int argc, char **argv) {
  pfInit();
  
  pfMultiprocess(PFMP_DEFAULT);
  
  pfdInitConverter("esprit.flt");
  
  pfConfig();
  
  pfFilePath(".:/usr/share/Performer/data");

  void *arena = pfGetSharedArena();
  
  // Configure and open GL window
  pfPipe *p = pfGetPipe(0);
  pfPipeWindow *pw = new pfPipeWindow(p);
  pw->setWinType(PFPWIN_TYPE_X);
  pw->setFBConfigAttrs(FBAttrs);
  pw->setName("OpenGL Performer Projective Lighting Demo");
  pw->setOriginSize(0,0,512,512);
  pw->open();
  
  
  pfScene *scene = new pfScene();
  
  //DCS which will hold wall and floor
  pfDCS *dcs = new pfDCS();
  //DCS which will hold the esprit model.
  pfDCS *modelDCS = new pfDCS();
  scene->addChild(dcs);
  scene->addChild(modelDCS);
  
  dcs->addChild(initBackground());
  dcs->setScale(2.0, 2.0, 1.0);

  
  // Create and configure a pfChannel.
  pfChannel *chan = new pfChannel(p);
  chan->setScene(scene);
  chan->setFOV(45.0f, 0.0f);
  chan->setNearFar(1.0f, 20.0f);
  
  chan->setTravMode(PFTRAV_MULTIPASS, 
		    PFMPASS_EMISSIVE_PASS | PFMPASS_BLACK_PASS);

  
  //Set up the channel geostate to something reasonable
  pfMaterial *m = new pfMaterial();
  m->setShininess(100.0);
  m->setColor(PFMTL_AMBIENT,  0.1, 0.1, 0.1);
  m->setColor(PFMTL_DIFFUSE,  0.5, 0.5, 0.5);
  m->setColor(PFMTL_EMISSION, 0.0, 0.0, 0.0);
  m->setColor(PFMTL_SPECULAR, 0.5, 0.5, 0.5);
  
  pfGeoState *gstate = new pfGeoState();
  gstate->setAttr(PFSTATE_FRONTMTL, m);
  gstate->setMode(PFSTATE_ENLIGHTING, 1);
  
  pfLightModel *lm = new pfLightModel();
  lm->setAmbient(0.0, 0.0, 0.0);
  gstate->setAttr(PFSTATE_LIGHTMODEL, lm);
  chan->setGState(gstate);
  
  //Set up a view into the scene.
  pfCoord view;
  view.xyz.set(-3, -4, 0.5);
  view.hpr.set(-30, -14, 0);
  chan->setView(view.xyz, view.hpr);
  
  
  /*
    The next few lines set up a green, projective pfLightSource
    */

  pfLightSource *ls = new pfLightSource;
  
  /*
    The light's initial position is (0, 0, 0, 0). The fourth coordinate, w, 
    specifies whether the light is a local or a distant light source. Here,
    we choose a distant light source, but local lighting would have worked
    just as well. In OpenGL (0, 0, 0, 0) is an invalid position for a distant
    light, but this light's position will be transformed through the DCS's
    above it in the scene, so it will work out to something acceptable. 
    */
  ls->setPos(0.0, 0.0, 0.0, 0.0);
  
  /*
    Set up the light's color properties
    */
  ls->setAmbient(0.0, 0.0, 0.0);
  ls->setColor(PFLT_AMBIENT, 0.0, 0.0, 0.0);
  ls->setColor(PFLT_DIFFUSE, 0.0, 0.5, 0.0);
  ls->setColor(PFLT_SPECULAR, 0.0, 0.5, 0.0);
  
  
  //enable projective texturing
  ls->setMode(PFLS_PROJTEX_ENABLE, 1);

  /*
    Spot lights can have an arbitrary direction, which is transformed through
    the DCS's above it. 
    */
  ls->setSpotDir(0.0, 1.0, 0.0);
  
  /*
    Each projective light needs to have a frustum, which is used in generating
    the texture projection matrix
    */
  pfFrustum *frust = new pfFrustum();
  frust->makeSimple(FOV);
  ls->setAttr(PFLS_PROJ_FRUST, frust);

  //Finally, pass the lobe texture to the light
  ls->setAttr(PFLS_PROJ_TEX, initTexture());
  
  //Create some DCS's to hold our lights
  pfDCS *projDCS = new pfDCS();
  pfDCS *projDCS2 = new pfDCS();
  pfDCS *projDCS3 = new pfDCS();
  
  
  /*
    The light below will be a shadow casting light on machines which support
    the shadow texture extension (IR, RE2), but will revert to a regular 
    projective light on machines without the extension. It's created much the
    same as the light above
    */
  
  //see if shadows are available on the current platform
  int ret;
  pfQueryFeature(PFQFTR_TEXTURE_SHADOW, &ret);

  pfLightSource *ls2 = new pfLightSource();
  ls2->setPos(0.0, 0.0, 0.0, 1.0);
  ls2->setAmbient(0.0, 0.0, 0.0);
  ls2->setColor(PFLT_AMBIENT, 0.0, 0.0, 0.0);
  ls2->setColor(PFLT_DIFFUSE, 0.5, 0.5, 0.0);
  ls2->setColor(PFLT_SPECULAR, 1.0, 1.0, 0.0);

  /*
    If shadows aren't available, make a standard projective light
    */
  if(ret != PFQFTR_FAST) {
    ls2->setMode(PFLS_PROJTEX_ENABLE, 1);
    ls2->setAttr(PFLS_PROJ_TEX, initTexture());
    fprintf(stdout, "Shadows not supported, using lobe light instead\n");
  }
  else {
    /* Otherwise, set up the shadow light properties */
    //enable shadowing
    ls2->setMode(PFLS_SHADOW_ENABLE, 1);
    //set up the size of the shadow map. It must be equal to or smaller in size
    //as compared to the window this program is running in, since Performer 
    //uses the window to render the shadowmap. 
    ls2->setVal(PFLS_SHADOW_SIZE, 512);
    //Sets the intensity of the light outside of the shadow region
    ls2->setVal(PFLS_INTENSITY, 0.4);
    ls2->setColor(PFLT_AMBIENT, 0.0, 0.0, 0.0);
  }
  
  //Once again, set the spotlight direction and frustum
  ls2->setSpotDir(0.0, 1.0, 0.0);
  ls2->setAttr(PFLS_PROJ_FRUST, frust);
 
  
  
  /*
    Lastly, we create a regular non-projective light to shed some light on
    the scene outside of the light lobes
    */
  pfLightSource *ls3 = new pfLightSource();
  ls3->setPos(0.0, -10.0, 10.0, 0.0);
  ls3->setAmbient(0.0, 0.0, 0.0);
  ls3->setColor(PFLT_AMBIENT, 0.0, 0.0, 0.0);
  ls3->setColor(PFLT_DIFFUSE, 0.2, 0.2, 0.2);
  ls3->setColor(PFLT_SPECULAR, 0.5, 0.5, 0.5);


  //Create a polygon with a tree image on it, it's in just the right place
  //to cast shadows onto the esprit and onto the wall behind it. 
  pfDCS *treeDCS = new pfDCS();
  
  pfGeoSet *gset = new pfGeoSet();
  pfVec3 *v = (pfVec3 *)pfMalloc(4 * sizeof(pfVec4), arena);
  pfVec3 *n = (pfVec3 *)pfMalloc(sizeof(pfVec3), arena);
  pfVec4 *c = (pfVec4 *)pfMalloc(sizeof(pfVec4), arena);
  
  //The tree is just a simple quad
  v[0].set(-1, 0,  1);
  v[3].set( 1, 0,  1);
  v[2].set( 1, 0, -1);
  v[1].set(-1, 0, -1);
  n->set(0, -1, 0);
  c->set(1.0, 1.0, 1.0, 1.0);
  
  pfVec2 *tc = (pfVec2 *)pfMalloc(4 * sizeof(pfVec2), arena);
  tc[0].set(1, 1);
  tc[1].set(1, 0);
  tc[2].set(0, 0);
  tc[3].set(0, 1);

  gstate = new pfGeoState();
  gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, v, NULL);
  gset->setAttr(PFGS_NORMAL3, PFGS_OVERALL, n, NULL);
  gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, c, NULL);
  gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tc, NULL);
  gset->setPrimType(PFGS_QUADS);
  gset->setNumPrims(1);
  gset->setGState(gstate);
  
  //Load up the tree texture image
  pfTexture *tex = new pfTexture();

  if(!tex->loadFile("ash.rgba")) {
    fprintf(stderr, "Could not locate tree texture file ash.rgba!\n");
    exit(-1);
  }
  tex->setRepeat(PFTEX_WRAP, PFTEX_CLAMP);

  //Using point sampling, since bilinear filtering introduces blending 
  //artifacts into textures with "holes" in them, such as trees. 
  tex->setFilter(PFTEX_MINFILTER, PFTEX_POINT);
  tex->setFilter(PFTEX_MAGFILTER, PFTEX_POINT);
  
  //material for the tree, It's pure white so that modulation by the texture
  //looks ok.
  m = new pfMaterial();
  m->setColor(PFMTL_AMBIENT,  0.0, 0.0, 0.0);
  m->setColor(PFMTL_DIFFUSE,  1.0, 1.0, 1.0);
  m->setColor(PFMTL_EMISSION, 0.0, 0.0, 0.0);
  m->setColor(PFMTL_SPECULAR, 0.0, 0.0, 0.0);
  gstate->setAttr(PFSTATE_FRONTMTL, m);
  
  gstate->setAttr(PFSTATE_TEXTURE, tex);
  gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_FAST);
  gstate->setMode(PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
  gstate->setVal(PFSTATE_ALPHAREF, 0.0f);
  gstate->setMode(PFSTATE_ENTEXTURE, 1);
  gstate->setMode(PFSTATE_ENLIGHTING, 1);
  gstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);
  
  //set up a modulating texture environment
  pfTexEnv *tev = new pfTexEnv();
  tev->setMode(PFTE_MODULATE);
  gstate->setAttr(PFSTATE_TEXENV, tev);
  
  //add the tree to a geode and put the geode in a dcs and into the scene
  pfGeode *geode = new pfGeode();
  geode->addGSet(gset);
  treeDCS->addChild(geode);
  treeDCS->setScale(0.3, 1, 1);
  treeDCS->setTrans(0.0, -1.3, 0);
  scene->addChild(treeDCS);

  pfNode *node = 0;
  node = pfdLoadFile("esprit.flt");
  
  if(!node) {
    fprintf(stderr, "Could not load esprit.flt!\n");
    exit(-1);
  }

  modelDCS->setScale(0.5, 0.5, 0.5);
  modelDCS->setTrans(0.0, 0.0, -1);
  modelDCS->addChild(node);
  
  
  

  //set up initial light positions
  projDCS->addChild(ls);
  projDCS->setTrans(0, 0, 2);

  projDCS2->addChild(ls2);
  projDCS2->setTrans(0, -4, 0.5);
  
  projDCS3->addChild(ls3);
  projDCS3->setTrans(0, -10, 10);
  
  //add the lights to the scene
  scene->addChild(projDCS);
  scene->addChild(projDCS2);
  scene->addChild(projDCS3);
  
  for(;;) {
    pfVec3 l, l2; 
    float t = pfGetTime();

    //rotate the esprit on the X axis at a rate of 20 degrees / second
    modelDCS->setRot(t * 20, 0, 0);
    

    //Move a point in a circle on the ground, and force the first light
    //to aim at this point.
    l.set(0.5 * cos(2.0 * t), 0.5 * sin(2.0 * t), 0);
    lookAt(projDCS, l);
    
    //translate the second light left and right along the X axis
    l2.set(cos(2.3 * t), 0.5, 0);
    projDCS2->setTrans(cos(2.0 * t), -4, 0.5);
    lookAt(projDCS2, l2);
    
    
    pfSync();
    pfFrame();
  }
  return 0;
}



