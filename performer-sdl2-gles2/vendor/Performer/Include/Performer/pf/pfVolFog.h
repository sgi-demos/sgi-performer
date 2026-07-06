//
// Copyright 1999, 2000, Silicon Graphics, Inc.
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
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfVolFog.h
//

#ifndef __PF_VOLFOG_H__
#define __PF_VOLFOG_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pr.h>
#include <Performer/prmath.h>
#include <Performer/pr/pfType.h>
#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfLight.h>
//#include <Performer/pr/pfFlux.h>
#include <Performer/pr/pfFog.h>

#include <Performer/pf.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfSCS.h>
#include <Performer/pf/pfDoubleSCS.h>
#include <Performer/pf/pfSwitch.h>
#include <Performer/pf/pfLOD.h>
#include <Performer/pf/pfSequence.h>
#include <Performer/pf/pfASD.h>
#include <Performer/pf/pfBillboard.h>


// Export to C API.
extern "C" {

#define PFVFOG_PATCHY_FOG  1
#define PFVFOG_LAYERED_FOG 2

/* fog attributes */
#define PFVFOG_COLOR        0x01
#define PFVFOG_DENSITY      0x02
#define PFVFOG_RESOLUTION   0x03
#define PFVFOG_DENSITY_BIAS 0x04
#define PFVFOG_MODE         0x05
#define PFVFOG_PATCHY_MODE  0x06
#define PFVFOG_LAYERED_MODE 0x07

#define PFVFOG_3D_TEX_SIZE  0x08
#define PFVFOG_MAX_DISTANCE 0x09

#define PFVFOG_ROTATE_NODE                 0x0c
#define PFVFOG_PATCHY_TEXTURE_BOTTOM       0x0d
#define PFVFOG_PATCHY_TEXTURE_TOP          0x0e

#define PFVFOG_LIGHT_SHAFT_ATTEN_SCALE     0x10
#define PFVFOG_LIGHT_SHAFT_ATTEN_TRANSLATE 0x11
#define PFVFOG_LIGHT_SHAFT_DARKEN_FACTOR   0x12

/* fog mode */
#define PFVFOG_LINEAR       GL_LINEAR
#define PFVFOG_EXP          GL_EXP
#define PFVFOG_EXP2         GL_EXP2

/* binary flags */
#define PFVFOG_FLAG_CLOSE_SURFACES     (1<<0) /* use stencil test to eliminate 
						 problems caused by surfaces 
						 with equal depth */
#define PFVFOG_FLAG_FORCE_2D_TEXTURE   (1<<1) /* forces using of 2D textures
						 for layered fog */
#define PFVFOG_FLAG_LAYERED_PATCHY_FOG (1<<2) /* use layered fog to control 
						 density of a patchy fog */
#define PFVFOG_FLAG_FORCE_PATCHY_PASS  (1<<3) /* do not test whether patchy 
						 fog bin is culled out */
#define PFVFOG_FASTER_PATCHY_FOG       (1<<4) /* use faster algorithm for 
						 patchy fog */
#define PFVFOG_FLAG_NO_OBJECT_IN_FOG   (1<<5) /* speeds up the faster algorithm
						 by assuming that no scene 
						 object is inside fog */

#define PFVFOG_FLAG_SELF_SHADOWING       (1<<6) /* enables self-shadowing of
						   layered fog */
#define PFVFOG_FLAG_DARKEN_OBJECTS       (1<<7) /* enables darkening of objects
						   under self-shadowed layered
						   fog */
#define PFVFOG_FLAG_FOG_FILTER           (1<<8) /* simulate secondary 
						   scattering in layered fog */
#define PFVFOG_FLAG_PATCHY_FOG_1DTEXTURE (1<<9) /* use a 1D texture to color 
						   surface of patchy fog. */
#define PFVFOG_FLAG_SEPARATE_NODE_BINS   (1<<10) /* use different bins for each
						    node supplied by addNode */
#define PFVFOG_FLAG_SCREEN_BOUNDING_RECT (1<<11) /* use bounding rectangle
						    in screen space to limit 
						    the areas that need to be
						    processesed - on by default
						    Actually, there is no 
						    reason to switch it off. */
#define PFVFOG_FLAG_DRAW_NODES_SEPARATELY (1<<12) /* draw fog nodes separately
						     in back-to-front order */
#define PFVFOG_FLAG_USER_PATCHY_FOG_TEXTURE (1<<13) /* user-supplied texture is
						       used on the surface of
						       patchy fog */
#define PFVFOG_FLAG_LIGHT_SHAFT           (1<<14) /* pfVolFog defines a light
						     shaft */
#define PFVFOG_FLAG_USE_CULL_PROGRAM      (1<<15) /* use cull programs to speed
						     up rendering passes */

#define PFVFOG_LAST_USER_FLAG             ((uint64_t)(1<<24))
}

typedef struct {
    float elevation;
    float density;
    float color[3];
} pfFogPoint;

typedef struct __pfLightPoint {
    float lightI;
    float attenuation;
    float Acolor[3];
} __pfLightPoint;


struct __pfViewType;
struct __pfPatchyFogParams;

#define PFVOLFOG ((pfVolFog*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFVOLFOGBUFFER ((pfVolFog*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfVolFog : public pfObject {

  //class type
  static pfType  *classType;

  // common variables
  pfVec3    fogColor;    // we can also have colors associated with each layer
  float     maxFogDistance;

  int       fogDefined;  // binary flags indicating the type(s) of fog
  uint64_t  flags;       // miscelaneous flags
  
  pfChannel **channels;  // array of app's channels
  pfFlux    **views;     // views per channel
  int       numChannels;  

  // patchy fog
  float     fogResolution;
  int       patchyFogMode;      // GL_LINEAR, GL_EXP, or GL_EXP2
  float     fogDensity;
  float     fogDensityBias;
  pfFog     *distanceFog;
  pfMaterial *mtl0;		// material used for patchy fog pass
  pfBox     fogBbox;            // bounding box of fog objects
  float     fogBsphere[4];      // bounding sphere 
  __pfPatchyFogParams *fogNodes;  // parameters that can vary for different
                                  // patchy fog nodes - not used now
  int       numNodes;           // not used
  float     *tex1d;             // not used
  float     *colorTable;   
  int       colorTableSize;
  int       stencilBits;

  // layered fog
  int        numFogPoints;	// number of density/elevation control points
  int        fogPointArraySize;
  pfFogPoint *fogPoints;   	// list of control points

  int       layeredFogMode;     // GL_LINEAR, GL_EXP, or GL_EXP2
  float     fogTexTop;	        // range of world space to which the fog 
  float     fogTexBottom;	// texture is mapped
  float     fogTexMinRadius;
  float     fogTexMaxRadius;
  float     fogTexEyeMin;
  float     fogTexEyeMax;

  pfTexture *fogtex;		// fog texture
  pfTexEnv  *fogtenv;		// tenv for fog texture
  pfTexGen  *fogtgen;		// texgen for fog texture
  unsigned char *teximage;
  int       fogtexSize[3];      // size of the 3D texture
  float     *fogmtab;		// table used to speedup recalc of fog texture
  float     lastR;              // previous texture R coordinate
  float     diffR;              // difference in R that causes texture subload

  pfMaterial *fogmtl;		// material used for fog texture pass
  pfLightModel *foglmodel;	// light model used for fog texture pass

  pfTexture *patchyTex;      // 1D texture used in patchy fog (texgen version)
  pfTexture *patchyTexC;     // texture used in patchy fog to modify color
  pfTexture *shaftTex;       // texture used to modify light coming from
                             // a light shaft
  pfTexGen  *patchytgen;     // texgen for patchy fog (texgen version)
  pfTexGen  *shafttgen;      // texgen for light shaft
  float     patchyTexCBottom[3]; // top color of the color patchy texture 
  float     patchyTexCTop[3];    // bottom color of the color patchy texture
  
  int       alphaBits;
  float     *texC;           // data for color texture
  
  __pfLightPoint  *lightP;   // light reaching each control point
  GLboolean statson;         // insert glFinish if stats computed
  float     shaftScale;      // scale and translation of the attenuation
  float     shaftTranslate;  // texture used in the light shaft
  float     darkenFactor;    // amount by which the scene outside shaft is darkened

public:

  pfVolFog();
  virtual ~pfVolFog();

  //// type checking functions
  static void init();
  static pfType* getClassType() {return classType;}

  //// common functions (both for layered and patchy fog)
 
  // CAPI:verb VolFogSetColor
  void setColor(float _r, float _g, float _b);
  // CAPI:verb VolFogGetColor
  void getColor(float *r, float *g, float *b);

  // CAPI:verb VolFogSetFlags
  void setFlags(int which, int val);
  // CAPI:verb VolFogGetFlags
  int getFlags(int which);
  // CAPI:verb VolFogSetVal
  void setVal(int which, float  val);
  // CAPI:verb VolFogGetVal
  void getVal(int which, float *val);
  // CAPI:verb VolFogSetAttr
  void setAttr(int which, void *attr);
  // CAPI:verb VolFogGetAttr
  void getAttr(int which, void *attr);

  void apply(pfScene *scene);
  void draw(pfChannel *channel);

  // CAPI:noverb
  void addChannel(pfChannel *channel);
  void updateView(void);
  // CAPI:verb VolFogRotateCS
  void rotateFogCS(pfMatrix *mat); // not implemented yet

  //// patchy fog

  // CAPI:verb VolFogAddNode
  void addNode(pfNode *node);
  // CAPI:verb VolFogDeleteNode
  void deleteNode(pfNode *node);  // not used now
  // CAPI:verb VolFogSetDensity
  void setDensity(float density);
  // CAPI:verb VolFogGetDensity
  float getDensity(void);

  // the following 8 function are not used now
  // CAPI:verb VolFogSetIndexVal
  void setVal(int which, int index, float  val);
  // CAPI:verb VolFogGetIndexVal
  void getVal(int which, int index, float *val);
  // CAPI:verb VolFogSetNodeVal
  void setVal(int which, pfNode *node, float  val);
  // CAPI:verb VolFogGetNodeVal
  void getVal(int which, pfNode *node, float *val);
  // CAPI:verb VolFogSetIndexAttr
  void setAttr(int which, int index, void *attr);
  // CAPI:verb VolFogGetIndexAttr
  void getAttr(int which, int index, void *attr);
  // CAPI:verb VolFogSetNodeAttr
  void setAttr(int which, pfNode *node, void *attr);
  // CAPI:verb VolFogGetNodeAttr
  void getAttr(int which, pfNode *node, void *attr);
  // CAPI:noverb

  //// layered fog

  pfTexture *getTexture(void);

  void addPoint(float elevation, float density);
  // CAPI:verb VolFogAddColoredPoint
  void addColoredPoint(float elevation, float density, float _r, float _g, float _b);

private:
  void InitializeColorTable(void);
  void makeLayeredFogTexture(pfBox *bbox);
  void MakePatchyFogTextures(int bufferdepth);
  void drawLayeredFogPass(pfChannel *channel, int inPatchyFog);
  __pfViewType *getView(pfChannel *channel);
  void drawFastPatchyFog(pfChannel *channel, float minDistance, float maxDistance, __pfViewType *view);
  void ComputeLightPropagationTexture(void);
  void integrateWithSelfShadowing(float y0, float y1, float scale,
				  float *sum, float *colorR);
  void integrateHorizontalWithSelfShadowing(float y0, float distance, 
					    float *sum, float *colorR);
  float fogFunctionTable(float x);
  void setupLayeredTexture(pfChannel *channel, __pfViewType *view, int inPatchyFog);

public:
  void *_pfVolFogExtraData;
};

#endif //__PF_VOLFOG_H__
