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
// pfRotorWash.h
//

#ifndef __PF_ROTORWASH_H__
#define __PF_ROTORWASH_H__

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
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfFlux.h>

#include <Performer/pf.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfGroup.h>


#define MAX_ANIM_FRAMES 64
#define W_NAME_LEN 120

#define PFSTATEMAPLIST ((pfStateMapList*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFSTATEMAPLISTBUFFER ((pfStateMapList*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfStateMapList : public pfObject {    

public:

  //class type
  static pfType         *classType;

  pfTexture *texArray[MAX_ANIM_FRAMES];
  int numframes;
  int currentframe;
  float frametime;
  pfGeoSet *washgset;  // fluxed geoset
  pfGeoSet *wgset;     // current copy of the fluxed geoset
  int Vcount, Tcount;
  unsigned short *WtriIndex;
  float *Wranges;
  pfVec4 *Wcolours;
  pfVec2 *Wtex;
  pfVec3 *Wcoords;
  pfVec4 *color;
  int used;

  pfStateMapList *prev;
  pfStateMapList *next;

  pfStateMapList(pfStateMapList *last);

  virtual ~pfStateMapList();

  //type checking functions
  static void init();

  static pfType* getClassType() {return classType;}

};

typedef struct pfStateIndex {
    pfGeoState     *gstate;
    pfStateMapList *statelist;
} pfStateIndex;

#define PFROTORWASH ((pfRotorWash*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFROTORWASHBUFFER ((pfRotorWash*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfRotorWash : public pfObject {

  //class type
  static pfType         *classType;

  pfStateMapList *statelist;
  pfStateMapList *endstatelist;
  pfStateIndex   *stateindex;
  int numgeostates;
  pfGeode *geomGeode;
  pfGroup *geomGraph;
  char defaultName[W_NAME_LEN];
  pfGeoSet *infogset;
  int rings, spokes;
  float tex_repeat_s, tex_repeat_t;
  float xpos, ypos, displacement, wash_phase;
  float radii[4];
  int   subdivisionLevel;
  int   maxTriangles;
  float alpha1, alpha2, Dalpha;
  int   *wedge;

  pfVec3 *Wmarkers;

public:

  pfRotorWash(int mesh_rings, int mesh_spokes);
  virtual ~pfRotorWash();

  //type checking functions
  static void init();
  static pfType* getClassType() {return classType;}

  pfNode *getNode(void);
  pfTexture *getTexture(pfGeoState *gstate, int frame);
  void setTextures(pfGeoState **gstates, int num, char *name, int numtex);
  void setColor(pfGeoState *geostate, float r, float g, float b, float a);
  void setRadii(float in_1, float in_2, float out_1, float out_2);
  void setAlpha(float a_inner, float a_outer);
  void setDisplacement(float value);
  void setSubdivisionLevel(int level);
  int  getSubdivisionLevel(void);
  void setMaxTriangles(int maxTris);
  int  getMaxTriangles(void);
  void highlight(float red, float green, float blue);
  void unhighlight(void);
  void compute(pfNode *terrain, float x, float y,float phase);

private:

  void getVerts(pfNode *node);
  void sectEdges(pfStateMapList *curlist, int &Vcount, int &Tcount, int &Mcount);
  void moreTris(pfStateMapList *curlist, int &Vcount, int &Tcount, int &Mcount);
  void splitTriangle(pfStateMapList *curlist, int contacts, int triIndex, int &Vcount, int &Tcount, int Mcount);
  pfStateMapList *findGState(pfGeoState *gstate);
public:
  void *_pfRotorWashExtraData;
};

#endif //__PF_ROTORWASH_H__
