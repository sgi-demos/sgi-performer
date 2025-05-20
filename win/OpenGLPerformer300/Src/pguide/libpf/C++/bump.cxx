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

//
// complexC.C
// 
// OpenGL Performer example using cull and draw process callbacks.
// Mouse and keyboard go through GL which is simpler than mixed
// model (GLX), but does incur some overhead in the draw process.
// X input handling is done in a forked event handling process.
//
// $Revision: 1.7 $ 
// $Date: 2002/11/18 18:14:17 $
//
// Command-line options:
//  -b	: norborder window
//  -f	: full screen
//  -F	: put X input handling in a forked process
//  -m procsplit : multiprocess mode
//  -w	: write scene to file
// 
// Run-time controls:
//       ESC-key: exits
//        F1-key: profile
//    Left-mouse: advance
//  Middle-mouse: stop
//   Right-mouse: retreat


#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h> // for sigset for forked X event handler process 
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h> // for cmdline handler 
#include <X11/keysym.h>
#endif

#include <Performer/pf/pfPipe.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pr/pfLight.h>
#include <Performer/pr/pfList.h>

#include <Performer/pfutil.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

//
// structure that resides in shared memory so that the
// application, cull, and draw processes can access it.

typedef struct
{
    pfPipeWindow    *pw;
    int		    exitFlag;
    int		    inWindow, reset;
    float	    mouseX, mouseY;
    int		    winSizeX, winSizeY;
    int		    mouseButtons;
    pfCoord	    view, viewOrig;
    float	    accelRate;
    float	    sceneSize;
    int		    drawStats;
    int		    XInputInited;
} SharedData;

static SharedData *Shared;

//
// APP process variables

// for configuring multi-process 
static int ProcSplit = PFMP_DEFAULT;
// write out scene upon read-in - uses pfDebugPrint 
static int WriteScene = 0;
static int FullScreen = 0;
static int WinType = PFPWIN_TYPE_X;
static int NoBorder = 0;
static int ForkedXInput = 0;
char ProgName[PF_MAXSTRING];

// light source created and updated in DRAW-process 
static pfLight *Sun;

#ifdef WIN32
#define GL_FUNC_ADD_EXT                       0x8006
#define GL_FUNC_SUBTRACT_EXT                  0x800A
#define GL_FUNC_REVERSE_SUBTRACT_EXT          0x800B

typedef void (APIENTRY *PFGL_BLEND_EQUATION) (GLenum mode);

static PFGL_BLEND_EQUATION pfglBlendEquation = NULL;
#endif

static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);
static void UpdateView(void);
static void GetGLInput(void);
#ifndef WIN32
static void InitXInput(pfWSConnection dsp);
static void DoXInput(void);
static void GetXInput(Display *dsp);
static void Usage(void);
#endif

//
//	Usage() -- print usage advice and exit. This procedure
//	is executed in the application process.


static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, 
	     "Usage: %s [-m procSplit] [file.ext ...]\n", ProgName);
    exit(1);
}

//
//	docmdline() -- use getopt to get command-line arguments, 
//	executed at the start of the application process.


static int
docmdline(int argc, char *argv[])
{
    int	    opt;
    
    strcpy(ProgName, argv[0]);
    
    // process command-line arguments 
    while ((opt = getopt(argc, argv, "fFbm:wxp:?")) != -1)
    {
	switch (opt)
	{
	case 'f': 
	    FullScreen = 1;
	    break;
	case 'F': 
	    ForkedXInput = 1;
	    break;
	case 'm':
	case 'p':
	    ProcSplit = atoi(optarg);
	    break;
	case 'w': 
	    WriteScene = 1;
	    break;
	case 'x': 
	    WinType &= ~(PFPWIN_TYPE_X);
	    break;
	case 'b': 
	    NoBorder ^= 1;
	    break;
	case '?': 
	    Usage();
	}
    }
    return optind;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////// Bumpmap logo geoset generation and update ///////////////
//////////////////////////////////////////////////////////////////////
//////////// This should use a GL_ADD environment on the /////////////
///////// first pass to combine lz and bump texture on iR  ///////////
// but for broader support lz is in the first pass (happy now tom?) //
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define RADIUS 1.0f

// sphere cap longitude
#define QTR_RINGS 6
// visual cross sectional divisions (latitude on caps)
// should be 1/2 of DIV to match cylinders
#define QTR_DIV 8
// visual edge divisions
#define RINGS 2
// visual cross sectional divisions (latitude on caps)
#define DIV 16
// base ambient value
#define LAMB 0.5f
// texture coordinate peturbation logo
#define DERIVSCALE 0.012f

// texture bar repeats (2 repeats per bar)
#define TBARREP 1.75f

// texture corner delta (1/8 repeats per corner)
#define TCRNREP 0.25f

char *Bname = "perflogo.bw";

//#define WIRE

static pfTexture *BumpTexture;
pfGeoState *Tgstate, *B1gstate, *B2gstate;

static pfVec4 *Qcolours;
static pfVec2 *Qtex;
static pfVec3 *Qcoords;
static pfVec3 *Qnorms;
static pfVec3 *Qsderiv;
static pfVec3 *Qtderiv;
static pfVec2 *QBtex;
static pfVec2 *QB2tex;

static pfVec2 *Ttex;
static pfVec4 *Tcolours;

static pfVec3 *Bcoords;
static pfVec3 *Bnorms;
static pfVec3 *Bsderiv;
static pfVec3 *Btderiv;
static pfVec2 *Btex;
static pfVec4 *Bcolours;
static pfVec2 *B2tex;

static pfGeode *Qgeode1;
static pfGeode *geode1;

static int PreDrawPass(pfTraverser *, void *data)
{
  pfTransparency(PFTR_OFF);
  pfDisable(PFEN_TEXTURE);
  pfOverride(PFSTATE_ENTEXTURE, PF_ON);

  return PFTRAV_CONT;
}

static int PostDrawPass(pfTraverser *, void *)
{
  pfOverride(PFSTATE_ENTEXTURE, PF_OFF);
  return PFTRAV_CONT;
}

static int PreDrawPass0(pfTraverser *, void *data)
{
  pfEnable(PFEN_TEXTURE);
  pfTransparency(PFTR_BLEND_ALPHA);
  glBlendFunc(GL_ONE, GL_ONE);
  pfOverride(PFSTATE_TRANSPARENCY, PF_ON);

  return PFTRAV_CONT;
}

static int PostDrawPass0(pfTraverser *, void *)
{
  pfOverride(PFSTATE_TRANSPARENCY, PF_OFF);
  return PFTRAV_CONT;
}

static int PreDrawPass1(pfTraverser *, void *)
{
#ifndef WIN32
  glPolygonOffsetEXT(-0.5, -0.000001);
#else
  glPolygonOffset(-0.5, -0.000001);
#endif
  glEnable(GL_POLYGON_OFFSET_EXT);
  pfTransparency(PFTR_BLEND_ALPHA);
  glBlendFunc(GL_ONE, GL_ONE);
#ifndef WIN32
  glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT_EXT);  
#else
  if(pfglBlendEquation)
    (*pfglBlendEquation)(GL_FUNC_REVERSE_SUBTRACT_EXT);
#endif
  glDepthMask(GL_FALSE);
  pfOverride(PFSTATE_TRANSPARENCY, PF_ON);

  return PFTRAV_CONT;
}

static int PostDrawPass1(pfTraverser *, void *data)
{
  glDisable(GL_POLYGON_OFFSET_EXT);
  pfOverride(PFSTATE_TRANSPARENCY, PF_OFF);
  pfTransparency(PFTR_OFF);
#ifndef WIN32
  glBlendEquationEXT(GL_FUNC_ADD_EXT);
#else
  if(pfglBlendEquation)
    (*pfglBlendEquation)(GL_FUNC_ADD_EXT);
#endif
  glDepthMask(GL_TRUE);

  return PFTRAV_CONT;
}

// Update the Texture coordinate info for logo bump mapping
void UpdateBumpMap(pfVec3 *light, float amplitude)
{
    int i;
    pfVec3 L, Lst;
    pfVec2 Pderiv;
    float lz, divvey;


    amplitude = powf(amplitude, 0.2f);
    divvey = DERIVSCALE * amplitude;

    float divscale;

    if(light)
    {
      for(i = 0; i < 18*QTR_RINGS*(QTR_DIV*2); i++)
      {
        // calculate lighting
        L = *light - Qcoords[i];
        L.normalize();
        lz = L.dot(Qnorms[i]); // L dot N
        if(lz < 0.0f)
        { // no delta tcoord & black colour
          Qcolours[i].set(0.0f, 0.0f, 0.0f, 1.0f );
          Qcolours[i].set(0.0f, 0.0f, 0.0f, 1.0f );
          QB2tex[i] = QBtex[i];
        }
        else
        {
          Lst = L - (Qnorms[i] * lz);
          // artificially reduce differentials as lz tends to zero
          // to avoid nasty pop on lz == 0 shadow clamp
          if(lz < 0.5f)
          {
            divscale = lz * 2.0f;
            Pderiv.set(divscale * divvey * Lst.dot(Qsderiv[i]),
                       divscale * divvey * Lst.dot(Qtderiv[i]) );
          }
          else
          {
            Pderiv.set(divvey * Lst.dot(Qsderiv[i]),
                       divvey * Lst.dot(Qtderiv[i]) );
          }
          // calculate lighting
          Qcolours[i].set(lz*LAMB, lz*LAMB, lz*LAMB, 1.0f);
          QB2tex[i] = QBtex[i] + Pderiv;
        }
      }

      Bcolours->set(1.0f, 1.0f, 1.0f, 1.0f );
      for(i = 0; i < 18*RINGS*(DIV*2+2); i++)
      {
        // calculate lighting
        L = *light - Bcoords[i];
        L.normalize();
        lz = L.dot(Bnorms[i]); // L dot N
        if(lz < 0.0f)
        { // no delta tcoord & black colour
          Tcolours[i].set(0.0f, 0.0f, 0.0f, 1.0f );
          B2tex[i] = Btex[i];
        }
        else
        {
          Lst = L - (Bnorms[i] * lz);
          // artificially reduce differentials as lz tends to zero
          // to avoid nasty pop on lz == 0 shadow clamp
          if(lz < 0.5f)
          {
            divscale = lz * 2.0f;
            Pderiv.set(divscale * divvey * Lst.dot(Bsderiv[i]),
                       divscale * divvey * Lst.dot(Btderiv[i]) );
          }
          else
          {
            Pderiv.set(divvey * Lst.dot(Bsderiv[i]),
                       divvey * Lst.dot(Btderiv[i]) );
          }
          // calculate lighting
          Tcolours[i].set(lz*LAMB, lz*LAMB, lz*LAMB, 1.0f);
          B2tex[i] = Btex[i] + Pderiv;
        }
      }
    }
}

// make the bumpmap geometry
pfNode *MakeBumpMap(void)
{
    float angle, theta, s, c, h, ht;
    int i, j, g, trip;

    pfTexEnv *Btenv = new pfTexEnv;
    Btenv->setMode(GL_REPLACE);

    B1gstate = new pfGeoState;
    B1gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_OFF);
    B1gstate->setMode(PFSTATE_ENTEXTURE, PF_ON);
    B1gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
    B1gstate->setMode(PFSTATE_CULLFACE, PFCF_BACK);
    B1gstate->setAttr(PFSTATE_TEXENV,Btenv);

    B2gstate = new pfGeoState;
    B2gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_OFF);
    B2gstate->setMode(PFSTATE_ENTEXTURE, PF_ON);
    B2gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
    B2gstate->setMode(PFSTATE_CULLFACE, PFCF_BACK);
    B2gstate->setAttr(PFSTATE_TEXENV,Btenv);

    BumpTexture = new pfTexture();
    BumpTexture->loadFile(Bname);

    B1gstate->setAttr(PFSTATE_TEXTURE, BumpTexture);
    B2gstate->setAttr(PFSTATE_TEXTURE, BumpTexture);

    Qcoords = (pfVec3*) new(18*QTR_RINGS*(QTR_DIV*2)*sizeof(pfVec3) ) pfMemory;
    Qtex = (pfVec2*) new(18*QTR_RINGS*(QTR_DIV*2)*sizeof(pfVec2) ) pfMemory;
    QBtex = (pfVec2*) new(18*QTR_RINGS*(QTR_DIV*2)*sizeof(pfVec2) ) pfMemory;
    QB2tex = (pfVec2*) new(18*QTR_RINGS*(QTR_DIV*2)*sizeof(pfVec2) ) pfMemory;
    Qnorms = (pfVec3*) new(18*QTR_RINGS*(QTR_DIV*2)*sizeof(pfVec3) ) pfMemory;
    Qsderiv = (pfVec3*) new(18*QTR_RINGS*(QTR_DIV*2)*sizeof(pfVec3) ) pfMemory;
    Qtderiv = (pfVec3*) new(18*QTR_RINGS*(QTR_DIV*2)*sizeof(pfVec3) ) pfMemory;
    Qcolours = (pfVec4*) new(18*QTR_RINGS*(QTR_DIV*2)*sizeof(pfVec4)) pfMemory;
    int *Qlengths = (int *) new(18*QTR_RINGS*sizeof(int)) pfMemory;

    // Set up geoset
    int *lengths = (int *) new(18*RINGS*sizeof(int)) pfMemory;
    Ttex = (pfVec2*) new(18*RINGS*(DIV*2+2)*sizeof(pfVec2)) pfMemory;
    Tcolours = (pfVec4*) new(18*RINGS*(DIV*2+2)*sizeof(pfVec4)) pfMemory;
    Bcoords = (pfVec3*) new(18*RINGS*(DIV*2+2)*sizeof(pfVec3) ) pfMemory;
    Btex = (pfVec2*) new(18*RINGS*(DIV*2+2)*sizeof(pfVec2)) pfMemory;
    Bcolours = (pfVec4*) new(sizeof(pfVec4)) pfMemory;
    Bnorms = (pfVec3*) new(18*RINGS*(DIV*2+2)*sizeof(pfVec3)) pfMemory;
    Bsderiv = (pfVec3*) new(18*RINGS*(DIV*2+2)*sizeof(pfVec3)) pfMemory;
    Btderiv = (pfVec3*) new(18*RINGS*(DIV*2+2)*sizeof(pfVec3)) pfMemory;
    B2tex = (pfVec2*) new(18*RINGS*(DIV*2+2)*sizeof(pfVec2)) pfMemory;

#define M_XF(a) a.postTrans(a, -5.0, -5.0, -5.0); \
                a.postRot(a, 45.0f, -1.0f, 1.0f, 0.0f);

    int G, Q_G;
    pfMatrix matey;
    // do logo bars
    for(trip=0; trip<3;trip++)
    {
      float TBdelt = TBARREP/RINGS;
      float TBoff = TCRNREP;
      float TCoff = 0.0f;
      float tex_roll;

      float TCdelt = TCRNREP/QTR_RINGS;

     for(g=0; g < 6; g++, TBoff += TCRNREP+TBARREP, TCoff += TCRNREP+TBARREP ) // 6 logo segments
     {
      while(TBoff >= 1.0f) TBoff -= 1.0f;
      while(TCoff >= 1.0f) TCoff -= 1.0f;
      Q_G = trip * 6 * QTR_RINGS*(QTR_DIV*2) + g * QTR_RINGS * (QTR_DIV*2);
      G = trip * 6 * RINGS*(DIV*2+2) + g * RINGS * (DIV*2+2);

      matey.makeIdent();
      switch(g) {
        case(0):
          matey.postTrans(matey, 1.0f, -1.0f, 1.0f);
          matey.postRot(matey, 120.0f*(trip+1), 1.0f, 1.0f, 1.0f);
          M_XF(matey);
          ht = 10.0f/(RINGS);
          tex_roll = 0.0f;
          break;
        case(1):
          matey.postRot(matey, 90.0f, 0.0f, 1.0f, 0.0f);
          matey.postTrans(matey, 1.0f, -1.0f, 11.0f);
          matey.postRot(matey, 120.0f*(trip+1), 1.0f, 1.0f, 1.0f);
          M_XF(matey);
          ht = 8.0f/(RINGS);
          tex_roll = 0.0f;
          break;
        case(2):
          matey.postRot(matey, 180.0f, 0.0f, 0.0f, 1.0f);
          matey.postRot(matey, 90.0f, -1.0f, 0.0f, 0.0f);
          matey.postTrans(matey, 9.0f, -1.0f, 11.0f);
          matey.postRot(matey, 120.0f*(trip+1), 1.0f, 1.0f, 1.0f);
          M_XF(matey);
          ht = 10.0f/(RINGS);
          tex_roll = 0.25f;
          break;
        case(3):
          matey.postRot(matey, 270.0f, 0.0f, 0.0f, 1.0f);
          matey.postRot(matey, 90.0f, 0.0f, -1.0f, 0.0f);
          matey.postTrans(matey, 9.0f, 9.0f, 11.0f);
          matey.postRot(matey, 120.0f*(trip+1), 1.0f, 1.0f, 1.0f);
          M_XF(matey);
          ht = 10.0f/(RINGS);
          tex_roll = 0.25f;
          break;
        case(4):
          matey.postRot(matey, 90.0f, 1.0f, 0.0f, 0.0f);
          matey.postTrans(matey, -1.0f, 9.0f, 11.0f);
          matey.postRot(matey, 120.0f*(trip+1), 1.0f, 1.0f, 1.0f);
          M_XF(matey);
          ht = 8.0f/(RINGS);
          tex_roll = 0.25f;
          break;
        case(5):
          matey.postRot(matey, 270.0f, 0.0f, 0.0f, 1.0f);
          matey.postRot(matey, 180.0f, 1.0f, 0.0f, 0.0f);
          matey.postTrans(matey, -1.0f, 1.0f, 11.0f);
          matey.postRot(matey, 120.0f*(trip+1), 1.0f, 1.0f, 1.0f);
          M_XF(matey);
          ht = 10.0f/(RINGS);
          tex_roll = 0.0f;
          break;
      }

      // build visible geoset Qtr sphere information
      for(j = 0, theta = 0.0f; j < QTR_RINGS; j++, theta+= 90.0f/QTR_RINGS)
      {
        float st, ct, stp, ctp, sp, cp;
        pfSinCos(theta, &st, &ct);
        pfSinCos(theta+90.0f/QTR_RINGS, &stp, &ctp);
        *(Qlengths+j+QTR_RINGS*g+QTR_RINGS*6*trip) = QTR_DIV*2;
        for(i = 0, angle = 0.0f; i < QTR_DIV; i++, angle += 180.0f/QTR_DIV)
        {
          pfSinCos(angle, &s, &c);
          pfSinCos(angle+180.0f/QTR_DIV, &sp, &cp);
          Qcoords[Q_G+j*QTR_DIV*2+i*2].set(-st*s*RADIUS, -c*RADIUS, -ct*s*RADIUS);
          Qcoords[Q_G+j*QTR_DIV*2+i*2+1].set(-stp*sp*RADIUS, -cp*RADIUS, -ctp*sp*RADIUS);
          Qcoords[Q_G+j*QTR_DIV*2+i*2].xformPt(Qcoords[Q_G+j*QTR_DIV*2+i*2], matey);
          Qcoords[Q_G+j*QTR_DIV*2+i*2+1].xformPt(Qcoords[Q_G+j*QTR_DIV*2+i*2+1], matey);
          Qnorms[Q_G+j*QTR_DIV*2+i*2].set(-st*s*RADIUS, -c*RADIUS, -ct*s*RADIUS);
          Qnorms[Q_G+j*QTR_DIV*2+i*2+1].set(-stp*sp*RADIUS, -cp*RADIUS, -ctp*sp*RADIUS);
          Qnorms[Q_G+j*QTR_DIV*2+i*2].xformVec(Qnorms[Q_G+j*QTR_DIV*2+i*2], matey);
          Qnorms[Q_G+j*QTR_DIV*2+i*2+1].xformVec(Qnorms[Q_G+j*QTR_DIV*2+i*2+1], matey);

          Qtderiv[Q_G+j*QTR_DIV*2+i*2].set(-ct, 0, st);
          Qtderiv[Q_G+j*QTR_DIV*2+i*2+1].set(-ctp, 0, stp);
          Qtderiv[Q_G+j*QTR_DIV*2+i*2].xformVec(Qtderiv[Q_G+j*QTR_DIV*2+i*2], matey);
          Qtderiv[Q_G+j*QTR_DIV*2+i*2+1].xformVec(Qtderiv[Q_G+j*QTR_DIV*2+i*2+1], matey);

          Qsderiv[Q_G+j*QTR_DIV*2+i*2].cross(-Qnorms[Q_G+j*QTR_DIV*2+i*2], Qtderiv[Q_G+j*QTR_DIV*2+i*2]);
          Qsderiv[Q_G+j*QTR_DIV*2+i*2+1].cross(-Qnorms[Q_G+j*QTR_DIV*2+i*2+1], Qtderiv[Q_G+j*QTR_DIV*2+i*2+1]);

          Qtex[Q_G+j*QTR_DIV*2+i*2].set(tex_roll+.5f -angle/360.0f, TCoff+j*TCdelt);
          Qtex[Q_G+j*QTR_DIV*2+i*2+1].set(tex_roll+.5f -(angle+180.0f/QTR_DIV)/360.0f, TCoff+j*TCdelt+TCdelt);
          QBtex[Q_G+j*QTR_DIV*2+i*2].set(tex_roll+.5f -angle/360.0f, TCoff+j*TCdelt);
          QBtex[Q_G+j*QTR_DIV*2+i*2+1].set(tex_roll+.5f -(angle+180.0f/QTR_DIV)/360.0f, TCoff+j*TCdelt+TCdelt);
        }
      }

      // build visible geoset cylinder information
      for(j = 0, h = 0.0f; j < RINGS; j++, h+= ht)
      {
        *(lengths+j+RINGS*g+RINGS*6*trip) = DIV*2+2;
        for(i = 0, angle = 0.0f; i < DIV+1; i++, angle += 360.0f/DIV)
        {
          pfSinCos(angle, &s, &c);
          Bcoords[G+(j*(DIV*2+2))+i*2].set(s*RADIUS, c*RADIUS, h);
          Bcoords[G+(j*(DIV*2+2))+i*2+1].set(s*RADIUS, c*RADIUS, h+ht);
          Bcoords[G+(j*(DIV*2+2))+i*2].xformPt(Bcoords[G+(j*(DIV*2+2))+i*2], matey);
          Bcoords[G+(j*(DIV*2+2))+i*2+1].xformPt(Bcoords[G+(j*(DIV*2+2))+i*2+1], matey);
          Bnorms[G+(j*(DIV*2+2))+i*2].set(s, c, 0.0f);
          Bnorms[G+(j*(DIV*2+2))+i*2+1].set(s, c, 0.0f);
          Bnorms[G+(j*(DIV*2+2))+i*2].xformVec(Bnorms[G+(j*(DIV*2+2))+i*2], matey);
          Bnorms[G+(j*(DIV*2+2))+i*2+1].xformVec(Bnorms[G+(j*(DIV*2+2))+i*2+1], matey);

          Btex[G+(j*(DIV*2+2))+i*2].set(tex_roll -angle/360.0f, TBoff+j*TBdelt);
          Btex[G+(j*(DIV*2+2))+i*2+1].set(tex_roll -angle/360.0f, TBoff+j*TBdelt+TBdelt);
          Ttex[G+(j*(DIV*2+2))+i*2].set(tex_roll -angle/360.0f, TBoff+j*TBdelt);
          Ttex[G+(j*(DIV*2+2))+i*2+1].set(tex_roll -angle/360.0f, TBoff+j*TBdelt+TBdelt);

          Btderiv[G+(j*(DIV*2+2))+i*2].set(0.0f, 0.0f, 1.0f);
          Btderiv[G+(j*(DIV*2+2))+i*2+1].set(0.0f, 0.0f, 1.0f);
          Btderiv[G+(j*(DIV*2+2))+i*2].xformVec(Btderiv[G+(j*(DIV*2+2))+i*2], matey);
          Btderiv[G+(j*(DIV*2+2))+i*2+1].xformVec(Btderiv[G+(j*(DIV*2+2))+i*2+1], matey);

          Bsderiv[G+(j*(DIV*2+2))+i*2].cross( -Bnorms[G+(j*(DIV*2+2))+i*2], Btderiv[G+(j*(DIV*2+2))+i*2] );
          Bsderiv[G+(j*(DIV*2+2))+i*2+1].cross( -Bnorms[G+(j*(DIV*2+2))+i*2+1], Btderiv[G+(j*(DIV*2+2))+i*2+1] );

          //Bsderiv[G+(j*(DIV*2+2))+i*2].set(-c, s, 0.0f);
          //Bsderiv[G+(j*(DIV*2+2))+i*2+1].set(-c, s, 0.0f);
          //Bsderiv[G+(j*(DIV*2+2))+i*2].xformVec(Bsderiv[G+(j*(DIV*2+2))+i*2], matey);
          //Bsderiv[G+(j*(DIV*2+2))+i*2+1].xformVec(Bsderiv[G+(j*(DIV*2+2))+i*2+1], matey);

        }
      }

     }
    }

    // visible cap geosets
    pfGeoSet *Qgset0 = new pfGeoSet;
    Qgset0->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, Qcoords, 0);
    Qgset0->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, Qcolours, 0);
    Qgset0->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, QBtex, 0);
    Qgset0->setPrimLengths(Qlengths);
    Qgset0->setPrimType(PFGS_TRISTRIPS);
    Qgset0->setNumPrims(18*QTR_RINGS);
    Qgset0->setGState(B1gstate);
    Qgset0->setIsectMask(0x00000000, PFTRAV_SELF|PFTRAV_IS_CACHE, PF_SET);

    pfGeoSet *Qgset1 = new pfGeoSet;
    Qgset1->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, Qcoords, 0);
    Qgset1->setAttr(PFGS_COLOR4, PFGS_OVERALL, Bcolours, 0);
    Qgset1->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, QB2tex, 0);
    Qgset1->setPrimLengths(Qlengths);
    Qgset1->setPrimType(PFGS_TRISTRIPS);
    Qgset1->setNumPrims(18*QTR_RINGS);
    Qgset1->setGState(B2gstate);
    Qgset1->setIsectMask(0x00000000, PFTRAV_SELF|PFTRAV_IS_CACHE, PF_SET);

#ifdef WIRE
    Qgset0->setDrawMode(PFGS_WIREFRAME, PF_ON);
    Qgset1->setDrawMode(PFGS_WIREFRAME, PF_ON);
    Qgset2->setDrawMode(PFGS_WIREFRAME, PF_ON);
#endif

    // visible edge geosets
    pfGeoSet *gset0 = new pfGeoSet;
    gset0->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, Bcoords, 0);
    gset0->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, Tcolours, 0);
    gset0->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, Btex, 0);
    gset0->setPrimLengths(lengths);
    gset0->setPrimType(PFGS_TRISTRIPS);
    gset0->setNumPrims(18*RINGS);
    gset0->setGState(B1gstate);
    gset0->setIsectMask(0x00000000, PFTRAV_SELF|PFTRAV_IS_CACHE, PF_SET);

    pfGeoSet *gset1 = new pfGeoSet;
    gset1->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, Bcoords, 0);
    gset1->setAttr(PFGS_COLOR4, PFGS_OVERALL, Bcolours, 0);
    gset1->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, B2tex, 0);
    gset1->setPrimLengths(lengths);
    gset1->setPrimType(PFGS_TRISTRIPS);
    gset1->setNumPrims(18*RINGS);
    gset1->setGState(B2gstate);
    gset1->setIsectMask(0x00000000, PFTRAV_SELF|PFTRAV_IS_CACHE, PF_SET);

#ifdef WIRE
    gset0->setDrawMode(PFGS_WIREFRAME, PF_ON);
    gset1->setDrawMode(PFGS_WIREFRAME, PF_ON);
    gset2->setDrawMode(PFGS_WIREFRAME, PF_ON);
#endif


    // set up scene graph
    pfGeode *Qgeode = new pfGeode;
    Qgeode->addGSet(Qgset0);
    Qgeode->setTravMask(PFTRAV_ISECT, 0x00000000, PFTRAV_SELF|PFTRAV_DESCEND, PF_SET);
    Qgset0->setIsectMask(0x00000000, PFTRAV_SELF, PF_SET);
    Qgeode->setTravFuncs(PFTRAV_DRAW, PreDrawPass, PostDrawPass);

    // set up scene graph
    pfGeode *Qgeode0 = new pfGeode;
    Qgeode0->addGSet(Qgset0);
    Qgeode0->setTravMask(PFTRAV_ISECT, 0x00000000, PFTRAV_SELF|PFTRAV_DESCEND, PF_SET);
    Qgset0->setIsectMask(0x00000000, PFTRAV_SELF, PF_SET);
    Qgeode0->setTravFuncs(PFTRAV_DRAW, PreDrawPass0, PostDrawPass0);

    Qgeode1 = new pfGeode;
    Qgeode1->addGSet(Qgset1);
    Qgeode1->setTravMask(PFTRAV_ISECT, 0x00000000, PFTRAV_SELF|PFTRAV_DESCEND, PF_SET);
    Qgset1->setIsectMask(0x00000000, PFTRAV_SELF, PF_SET);
    Qgeode1->setTravFuncs(PFTRAV_DRAW, PreDrawPass1, PostDrawPass1);

    // set up scene graph
    pfGeode *geode = new pfGeode;
    geode->addGSet(gset0);
    geode->setTravMask(PFTRAV_ISECT, 0x00000000, PFTRAV_SELF|PFTRAV_DESCEND, PF_SET);
    gset0->setIsectMask(0x00000000, PFTRAV_SELF, PF_SET);
    geode->setTravFuncs(PFTRAV_DRAW, PreDrawPass, PostDrawPass);

    // set up scene graph
    pfGeode *geode0 = new pfGeode;
    geode0->addGSet(gset0);
    geode0->setTravMask(PFTRAV_ISECT, 0x00000000, PFTRAV_SELF|PFTRAV_DESCEND, PF_SET);
    gset0->setIsectMask(0x00000000, PFTRAV_SELF, PF_SET);
    geode0->setTravFuncs(PFTRAV_DRAW, PreDrawPass0, PostDrawPass0);

    geode1 = new pfGeode;
    geode1->addGSet(gset1);
    geode1->setTravMask(PFTRAV_ISECT, 0x00000000, PFTRAV_SELF|PFTRAV_DESCEND, PF_SET);
    gset1->setIsectMask(0x00000000, PFTRAV_SELF, PF_SET);
    geode1->setTravFuncs(PFTRAV_DRAW, PreDrawPass1, PostDrawPass1);

    pfGroup *group = new pfGroup;
    group->addChild(Qgeode);
    group->addChild(Qgeode0);
    group->addChild(Qgeode1);

    group->addChild(geode);
    group->addChild(geode0);
    group->addChild(geode1);

    return (pfNode  *) group;
}

//
//	main() -- program entry point. this procedure
//	is executed in the application process.


int
main (int argc, char *argv[])
{
    int		    arg;
    int		    found;
    pfPipe         *p;
    pfBox           bbox;
    float	    myfar = 10000.0f;
    pfWSConnection  dsp=NULL;
    
    arg = docmdline(argc, argv);
    
    pfInit();
    
    // configure multi-process selection 
    pfMultiprocess(ProcSplit);
    
    // allocate shared before fork()'ing parallel processes 
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
    Shared->drawStats = 1;
    
    // Load all loader DSO's before pfConfig() forks 
    for (found = arg; found < argc; found++)
	pfdInitConverter(argv[found]);

    // initiate multi-processing mode set in pfMultiprocess call 
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    
    pfConfig();
    
    // configure pipes and windows 
    p = pfGetPipe(0);
    Shared->pw = new pfPipeWindow(p);
    Shared->pw->setName("OpenGL Performer");
    Shared->pw->setWinType(WinType);
    if (NoBorder)
	Shared->pw->setMode(PFWIN_NOBORDER, 1);
    // Open and configure the GL window. 
    Shared->pw->setConfigFunc(OpenPipeWin);
    Shared->pw->config();
    
    if (FullScreen)
	Shared->pw->setFullScreen();
    else
	Shared->pw->setOriginSize(0, 0, 300, 300);
    
    // set off the draw process to open windows and call init callbacks 
    pfFrame();
    
#ifndef WIN32
    // create forked XInput handling process 
    // since the Shared pointer has already been initialized, that structure
    // will be visible to the XInput process. Nothing else created in the
    // application after this fork whose handles are not put in shared memory
    // (such as the database and channels) will be visible to the
    // XInput process.
    
    if (WinType & PFPWIN_TYPE_X)
    {
	pid_t	    fpid = 0;
	if (ForkedXInput)
	{
	    if ((fpid = fork()) < 0)
		pfNotify(PFNFY_FATAL, PFNFY_SYSERR, "Fork of XInput process failed.");
	    else if (fpid)
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"XInput running in forked process %d",
			 fpid);
	    else if (!fpid)
		DoXInput();
	}
	else
	{
	    dsp = pfGetCurWSConnection();
	}
    }
#endif

    // specify directories where geometry and textures exist 
    if (!(getenv("PFPATH")))
        pfFilePath(
                   "."
                   ":./data"
                   ":../data"
                   ":../../data"
                   ":/usr/share/Performer/data"
                   );
    pfNotify(PFNFY_INFO, PFNFY_PRINT,"FilePath: %s\n", pfGetFilePath());
    
    // load files named by command line arguments 
    pfScene *scene = new pfScene();
    for (found = 0; arg < argc; arg++)
    {
        pfNode	   *root;
	if ((root = pfdLoadFile(argv[arg])) != NULL)
	{
	    scene->addChild(root);
	    found++;
	}
    }

    pfNode *root = MakeBumpMap();
    scene->addChild(root);
    pfVec3 lightpos;

    lightpos.set(1000.0f, 0.0f, 0.0f);
    // set light position
    UpdateBumpMap(&lightpos, 0.0f);
    
    // Write out nodes in scene (for debugging) 
    if (WriteScene)
    {
	FILE *fp;
	if (fp = fopen("scene.out", "w"))
	{
	    pfPrint(scene, PFTRAV_SELF|PFTRAV_DESCEND, PFPRINT_VB_DEBUG, fp);
	    fclose(fp);
	}
	else
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		     "Could not open scene.out for debug printing.");
    }
    
    // determine extent of scene's geometry 
    pfuTravCalcBBox(scene, &bbox);
    
    pfFrameRate(30.0f);
    pfPhase(PFPHASE_FREE_RUN);
    
    pfChannel *chan = new pfChannel(p);
    Shared->pw->addChan(chan);
    chan->setTravFunc(PFTRAV_CULL, CullChannel);
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);
    chan->setScene(scene);
    chan->setNearFar(0.1f, myfar);
    
    // Create an earth/sky model that draws sky/ground/horizon 
    pfEarthSky *eSky = new pfEarthSky();
    eSky->setMode(PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    eSky->setAttr(PFES_GRND_HT, -10.0f);
    chan->setESky(eSky);
    
    chan->setTravMode(PFTRAV_CULL, PFCULL_VIEW|PFCULL_GSET);
    
    // vertical FOV is matched to window aspect ratio 
    chan->setFOV(45.0f, -1.0f);
    if (found)
    {
	float sceneSize;
	// Set initial view to be "in front" of scene 
	
	// view point at center of bbox 
	Shared->view.xyz.add(bbox.min, bbox.max);
	Shared->view.xyz.scale(0.5f, Shared->view.xyz);
	
	// find max dimension 
	sceneSize = bbox.max[PF_X] - bbox.min[PF_X];
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Y] - bbox.min[PF_Y]);
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Z] - bbox.min[PF_Z]);
	sceneSize = PF_MIN2(sceneSize, 0.5f * myfar);
	Shared->sceneSize = sceneSize;
	
	// offset so all is visible 
	Shared->view.xyz[PF_Y] -=      sceneSize;
	Shared->view.xyz[PF_Z] += 0.25f*sceneSize;	
    }  else
    {
	Shared->view.xyz.set(0.0f, 0.0f, 100.0f);
	PFSET_VEC3(bbox.min, -5000.0f, -5000.0f, -1000000.0f);
	PFSET_VEC3(bbox.max, 5000.0f, 5000.0f, 10000000.0f);
	Shared->sceneSize = 10000.0f;
    }
    Shared->view.hpr.set(0.0f, 0.0f, 0.0f);
    chan->setView(Shared->view.xyz, Shared->view.hpr);
    PFCOPY_VEC3(Shared->viewOrig.xyz, Shared->view.xyz);
    PFCOPY_VEC3(Shared->viewOrig.hpr, Shared->view.hpr);
    
    // main simulation loop 
    while (!Shared->exitFlag)
    {
	// wait until next frame boundary 
	pfSync();
	
	pfFrame();
	
	// Set view parameters for next frame 
	UpdateView();
	chan->setView(Shared->view.xyz, Shared->view.hpr);
	// initiate traversal using current state 
    
#ifndef WIN32
	if (!ForkedXInput)
	{
	    if (!Shared->XInputInited)
		InitXInput(dsp);
	    if (Shared->XInputInited)
		GetXInput(dsp);
	}
#endif
    }
    
    // terminate cull and draw processes (if they exist) 
    pfExit();
    
    // exit to operating system 
    return 0;
}

#ifndef WIN32
static void 
InitXInput(pfWSConnection dsp)
{
    Window w;
    
    /* wait for X Window to exist in Performer shared memory */
   if (w = Shared->pw->getWSWindow())
   {
	XSelectInput(dsp, w, PointerMotionMask |
			ButtonPressMask | ButtonReleaseMask | 
			KeyPressMask | KeyReleaseMask);
	XMapWindow(dsp, w);
	XFlush(dsp);
	Shared->XInputInited = 1;
    }
}

//
// DoXInput() runs an asychronous forked even handling process.
//  Shared memory structures can be read from this process
//  but NO performer calls that set any structures should be 
//  issues by routines in this process.

void
DoXInput(void)
{
    // windows from draw should now exist so can attach X input handling
    // to the X window 
    
    Display *dsp = pfGetCurWSConnection();
    
    #ifndef __linux__
    prctl(PR_TERMCHILD);        // Exit when parent does 
    sigset(SIGHUP, SIG_DFL);    // Exit when sent SIGHUP by TERMCHILD 
    #endif
    
    InitXInput(dsp);
    
    while (1)
    {
	XEvent          event;
	if (!Shared->XInputInited)
	    InitXInput(dsp);
	if (Shared->XInputInited)
	{
	    XPeekEvent(dsp, &event);
	    GetXInput(dsp);
	}
    }
}
#endif

// 
//	UpdateView() updates the eyepoint based on the information
//	placed in shared memory by GetInput().

static void    
UpdateView(void)
{
    static float speed = 0.0f;
    pfCoord *view = &Shared->view;
    float cp;
    float mx, my;
    static double thisTime = -1.0f;
    double prevTime;
    float deltaTime;

    prevTime = thisTime;
    thisTime = pfGetTime();

    if (prevTime < 0.0f)
	return;

    if (!Shared->inWindow || Shared->reset)
    {
	speed = 0;
	Shared->reset = 0;
	Shared->accelRate = 0.1f * Shared->sceneSize;
	return;
    }

    deltaTime = thisTime - prevTime;
#ifndef WIN32
    switch (Shared->mouseButtons)
    {
    case Button1Mask: /* LEFTMOUSE: faster forward or slower backward*/
    case Button1Mask|Button2Mask:
	speed += Shared->accelRate * deltaTime;
	if (speed > Shared->sceneSize)
	    speed = Shared->sceneSize;
	break;
    case Button3Mask: /* RIGHTMOUSE: faster backward or slower foreward*/
    case Button3Mask|Button2Mask:
	speed -= Shared->accelRate * deltaTime;
	if (speed < -Shared->sceneSize)
	    speed = -Shared->sceneSize;
	break;
    }
    if (Shared->mouseButtons)
    {
	mx = 2.0f * (Shared->mouseX / (float)Shared->winSizeX) - 1.0f;
	my = 2.0f * (Shared->mouseY / (float)Shared->winSizeY) - 1.0f;
				     
	/* update view direction */
	view->hpr[PF_H] -= mx * PF_ABS(mx) * 30.0f * deltaTime;
	view->hpr[PF_P] += my * PF_ABS(my) * 30.0f * deltaTime;
	view->hpr[PF_R]  = 0.0f;
	
	/* update view position */
	cp = cosf(PF_DEG2RAD(view->hpr[PF_P]));
	view->xyz[PF_X] += speed*sinf(-PF_DEG2RAD(view->hpr[PF_H]))*cp;
	view->xyz[PF_Y] += speed*cosf(-PF_DEG2RAD(view->hpr[PF_H]))*cp;
	view->xyz[PF_Z] += speed*sinf( PF_DEG2RAD(view->hpr[PF_P]));
    }
    else
    {
	speed = 0.0f;
	Shared->accelRate = 0.1f * Shared->sceneSize;
    }
#endif
}

//
//	CullChannel() -- traverse the scene graph and generate a
// 	display list for the draw process.  This procedure is 
//	executed in the CULL process.


static void
CullChannel(pfChannel *, void *)
{
    // 
    // pfDrawGeoSet or other display listable Performer routines
    // could be invoked before or after pfCull()
    pfCull();
}

//
//	OpenPipeWin() -- create a win: setup the GL and OpenGL Performer.
//	This procedure is executed in the DRAW process 
//	(when there is a separate draw process).


static void
OpenPipeWin(pfPipeWindow *pw)
{
    pw->open();
    
    // create a light source in the "south-west" (QIII) 
    Sun = new pfLight();
    Sun->setPos(-0.3f, -0.3f, 1.0f, 0.0f);
}


//
//	DrawChannel() -- draw a channel and read input queue. this
//	procedure is executed in the draw process (when there is a
//	separate draw process).

static void
DrawChannel (pfChannel *channel, void *)
{
    // rebind light so it stays fixed in position 
    Sun->on();

#ifdef WIN32
    {
      static int firsttime = 1;
      int res;
      
      if(firsttime)
      {
	pfQueryFeature(PFQFTR_BLEND_FUNC_SUBTRACT, &res);
	if(res != FALSE)
	{
	  pfglBlendEquation = 
	    (PFGL_BLEND_EQUATION)wglGetProcAddress("glBlendEquationEXT");
	}
	
	if(res == FALSE || pfglBlendEquation == NULL)
	  pfNotify(PFNFY_ALWAYS, PFNFY_PRINT, 
		   "glBlendEquationEXT doesn't exist on this platform, rendering"
		   " will not be correct!!!\n");

	firsttime = 0;
      }
    }
#endif
    
    // erase framebuffer and draw Earth-Sky model 
    channel->clear();
    
    // invoke Performer draw-processing for this frame 
    pfDraw();
    
    // draw Performer throughput statistics 
    
    if (Shared->drawStats)
	channel->drawStats();
    
    // read window origin and size (it may have changed) 
    channel->getPWin()->getSize(&Shared->winSizeX, &Shared->winSizeY);
    
}


#ifndef WIN32
static void
GetXInput(pfWSConnection dsp)
{
    static int x=0, y=0;
    
    if (XEventsQueued(dsp, QueuedAfterFlush))
    while (XEventsQueued(dsp, QueuedAlready))
    {
	XEvent event;
	
	XNextEvent(dsp, &event);
	
	switch (event.type) 
	{
	case ConfigureNotify:
	    break;
	case FocusIn:
	    Shared->inWindow = 1;
	    break;
	case FocusOut:
	    Shared->inWindow = 0;
	    break;
	case MotionNotify: 
	    {
		XMotionEvent *motion_event = (XMotionEvent *) &event;
		x =  motion_event->x;
		y = Shared->winSizeY - motion_event->y;
	    }
	    break;
	case ButtonPress: 
	    {
		XButtonEvent *button_event = (XButtonEvent *) &event;
		x = event.xbutton.x;
		y = Shared->winSizeY - event.xbutton.y;
		Shared->inWindow = 1;
		switch (button_event->button) {
		case Button1:
		    Shared->mouseButtons |= Button1Mask;
		    break;
		case Button2:
		    Shared->mouseButtons |= Button2Mask;
		    break;
		case Button3:
		    Shared->mouseButtons |= Button3Mask;
		    break;
		}
	    }
	    break;
	case ButtonRelease:
	    {
		XButtonEvent *button_event = (XButtonEvent *) &event;
		switch (button_event->button) {
		case Button1:
		    Shared->mouseButtons &= ~Button1Mask;
		    break;
		case Button2:
		    Shared->mouseButtons &= ~Button2Mask;
		    break;
		case Button3:
		    Shared->mouseButtons &= ~Button3Mask;
		    break;
		}
	    }
	    break;
	case KeyPress:
	    {
		char buf[100];
		KeySym ks;
		XLookupString(&event.xkey, buf, sizeof(buf), &ks, 0);
		switch(ks) {
		case XK_Escape: 
		    Shared->exitFlag = 1;
		    exit(0);
		    break;
		case XK_space:
		    Shared->reset = 1;
		    PFCOPY_VEC3(Shared->view.xyz, Shared->viewOrig.xyz);
		    PFCOPY_VEC3(Shared->view.hpr, Shared->viewOrig.hpr);
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Reset");
		    break;
		case XK_g:
		    Shared->drawStats = !Shared->drawStats;
		    break;
		default:
		    break;
		}
	    }
	    break;
	default:
	    break;
	}// switch 
    }
    Shared->mouseX = x;
    Shared->mouseY = y;
}
#endif
