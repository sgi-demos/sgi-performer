/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <assert.h>
#include <sys/types.h>

#include "perfly.h"
#include "gui.h"
#include "flyLightPoints.h"

#define log2(x) (log(x)*M_LOG2E)

/*
 *  These prototypes are for the terrain LOD evaluation functions
 *  in eval_function.c.
 */
extern float  lodEvalFunc(pfASD *mesh, pfVec3 eyept, int faceid, int splitid);
extern void   lodEvalFunc_pass(void);
extern void   lodEvalFunc_init(pfASD *asd);

void printMallInfo(struct mallinfo *mall)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Heap - total bytes used: %d",
        mall->usmblks + mall->uordblks);
}

void printAMallInfo(struct mallinfo *mall)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Arena - total bytes used: %d",
        mall->usmblks + mall->uordblks);
}

int CLIP_TEXTURE;

int
cliptexApp(pfTraverser *trav, void *data);

pfASD *setASDData(char *confname);

pfMPClipTexture *mpcliptex;
pfClipTexture *clip;

/* Keep a default Material Handy */
static pfMaterial *
defaultMaterial (void)
{
    static pfMaterial   *material       = NULL;

    if (material == NULL)
    {
        material = pfNewMtl(pfGetSharedArena());
        pfMtlColor(material, PFMTL_AMBIENT,  0.8f, 0.8f, 0.8f);
        pfMtlColor(material, PFMTL_DIFFUSE,  0.8f, 0.8f, 0.8f);
        pfMtlColor(material, PFMTL_SPECULAR, 0.0f, 0.0f, 0.0f);
        pfMtlShininess(material, 0.0f);

        pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_OFF);
    }

    /* return pointer to default material */
    return material;
}

static pfMatrix TexMat;
char texfile[100];

/* construct global geostate for terrain. apply texture or cliptexture
 * to terrain based on texgen that is defined to .config file */
static void
buildGState(pfGeoState *gs)
{
    int width, height, depth;

    /*
     *	Start off with a clean state.
     */
    pfMakeBasicGState(gs);

    pfGStateMode(gs, PFSTATE_CULLFACE, PFCF_BACK);
    pfGStateMode(gs, PFSTATE_ENLIGHTING, PF_ON);
    pfGStateMode(gs, PFSTATE_ENTEXTURE, PF_ON);
    ViewState->mtl = defaultMaterial();
    pfGStateAttr(gs, PFSTATE_FRONTMTL, ViewState->mtl);

   /* texture geostate */
   {
        pfTexture* tex = pfNewTex( pfGetSharedArena() );
        pfTexture* dtex = pfNewTex( pfGetSharedArena() );
        pfTexGen* tgen = pfNewTGen( pfGetSharedArena() );

   	if(CLIP_TEXTURE==1) {
	    if(!(clip = pfdLoadClipTexture(texfile))) {
	        pfNotify(PFNFY_WARN, PFNFY_PRINT, "asdfly:bad clip map\n");
	        return;
	    }
            pfGStateAttr( gs, PFSTATE_TEXTURE, clip );
	    mpcliptex = pfNewMPClipTexture();
            pfMPClipTextureClipTexture(mpcliptex, clip);
	    pfAddMPClipTexture(pfGetPipe(0), mpcliptex);
	    pfPipeIncrementalStateChanNum(pfGetPipe(0),1);
	}
	else 
	{
   	    pfLoadTexFile( tex, texfile );
            pfGStateAttr( gs, PFSTATE_TEXTURE, tex );
	}

/* load texgen and texgen matrix */
        pfGStateMode(gs, PFSTATE_ENTEXGEN, PF_ON);
        pfGStateAttr( gs, PFSTATE_TEXGEN, tgen );
        pfGStateAttr( gs, PFSTATE_TEXENV, pfNewTEnv( pfGetSharedArena() ) );

        pfTGenPlane(tgen, PF_S, TexMat[0][0], TexMat[1][0], TexMat[2][0], TexMat[3][0]);
	pfTGenPlane(tgen, PF_T, TexMat[0][1], TexMat[1][1], TexMat[2][1], TexMat[3][1]);
        pfTGenPlane(tgen, PF_R, TexMat[0][2], TexMat[1][2], TexMat[2][2], TexMat[3][2]);
        pfTGenPlane(tgen, PF_Q, TexMat[0][3], TexMat[1][3], TexMat[2][3], TexMat[3][3]);

	pfTGenMode(tgen, PF_S, PFTG_OBJECT_LINEAR);
        pfTGenMode(tgen, PF_T, PFTG_OBJECT_LINEAR);
        pfTGenMode(tgen, PF_R, PFTG_OBJECT_LINEAR);
        pfTGenMode(tgen, PF_Q, PFTG_OBJECT_LINEAR);
   }

    /*pfGStateMode (gs, PFSTATE_DECAL,
        PFDECAL_LAYER_DISPLACE_AWAY | PFDECAL_LAYER_7 | PFDECAL_LAYER_OFFSET);*/
}

/* the config file format for asdfly is the following:
 * config mode: 1 -- clipmap; 0 -- regular texture; 2 -- paging ASD
 * texgen matrix: 4x4 entries
 * texture name/ clipmap .ct file
 * pfASD file name
 * a good viewing position: x,y,z h,p,r
 * alignment mode: 1 -- take an alignment positions; 0 -- no alignment
 * number of files
 * name of alignment files
 *
 * the colormorph mode will assign an OVERALL color to the terrain and morph
 * the color based on morph weight. when morph weight = 1.0, the vertex
 * takes a pink color. as the morph weight decreases, the color is morphed
 * towards a grey color.
 *
 * the colorband mode will show the color of terrain. .evt file assigned
 * a distinguished color to each LOD. colorband will indicate the
 * LOD ranges for terrain.
 * other builders may assign different colors to the terrain.
 */

pfASD *
setASDData(char *confname)
{
    FILE *fp;
    char fname[200];
    int numlods;
    int i, j;
    pfASD *asdNode = NULL;
    int mode, do_paging;

    if(!confname) 
    {
	pfNotify(PFNFY_FATAL, PFNFY_USAGE,
		 "Usage: asdfly config_file\n");
	return NULL;
    }
    if(!(fp = fopen(confname, "r"))) 
    {
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, 
		 "Can't open configuration file") ;
	return NULL;
    }
    fscanf(fp, "%d", &mode);
    switch(mode)
    {
	case 0: 
	    CLIP_TEXTURE = 0;
	    do_paging = 0;
	    break;
	case 1: 
	    CLIP_TEXTURE = 1;
	    do_paging = 0;
	    break;
	case 2: 
	    CLIP_TEXTURE = 0;
	    do_paging = 1;
	    break;
    }

    for(i = 0; i < 4; i++)
    	for(j = 0; j < 4; j++)
    	    fscanf(fp, "%f", &(TexMat[i][j]));
    fscanf(fp, "%s", texfile);
    fscanf(fp, "%s", fname);

    /* load paging configurations, refer to libpfdu/pfdBuildASD.c for details */
    if(do_paging)
    {
	char pagename[300];
	fscanf(fp, "%s", pagename);
	asdNode = (pfASD *)pfdLoadConfig(fname, pagename);
    }
    else
    {
	pfNode *tmp;

	tmp = pfdLoadFile(fname);
	if(pfIsOfType(tmp, pfGetASDClassType()))
	    asdNode = (pfASD *)tmp;
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		"the node created in this config file is not a pfASD node.  please extend the filetype to accept cliptexture or alignment internally. config file format doesn't not handle it.");
	    asdNode = NULL;
	}
    }
    if(asdNode == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
	    "There is no ASD node loaded.");
	return(NULL);
    }

    pfASDEvalMethod(asdNode, PFASD_DIST_SQUARE);

    fscanf(fp, "%f,%f,%f", &(ViewState->goodview[0].xyz[PF_X]),
                &(ViewState->goodview[0].xyz[PF_Y]), &(ViewState->goodview[0].xyz[PF_Z]));
    fscanf(fp, "%f,%f,%f", &(ViewState->goodview[0].hpr[PF_H]),
                &(ViewState->goodview[0].hpr[PF_P]), &(ViewState->goodview[0].hpr[PF_R]));

    /*
     * Read light point selection.
     */
    ViewState->lightPoints.addPoints = 0;
    ViewState->lightPoints.useRealLightPoints = 0;
    if (fscanf (fp, "%d%d%s", 
			&ViewState->lightPoints.addPoints, 
			&ViewState->lightPoints.useRealLightPoints,
			ViewState->lightPoints.filename) > 0)
    {
	if (ViewState->lightPoints.addPoints)
	    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			"Will be reading light points from file <%s>\n", 
		    	ViewState->lightPoints.filename);
	else
	    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			"Will NOT be reading light points from file <%s>\n", 
		    	ViewState->lightPoints.filename);
    }


    fclose(fp);

    return asdNode;
}

/* center cliptexture center around eyepoint using texgen */
/* this can be accomplished by pfuTexGenClipCenter node as well. */
int
cliptexApp(pfTraverser *trav, void *data)
{
    pfChannel *chan;
    pfVec3 pos, f;
    pfMatrix travmat, invtravmat;
    int width, height, depth;
    int id;

    if(CLIP_TEXTURE == 1) {

        chan = ViewState->masterChan;
        if(chan == NULL) 
	    return(1);
        pfGetChanEye(chan, pos);
    	pfGetTravMat(trav, travmat);
    	pfInvertFullMat(invtravmat, travmat);
    	pfFullXformPt3(pos, pos, invtravmat);

    	f[0] = pos[0] * TexMat[0][0] + pos[1] * TexMat[1][0] + TexMat[3][0];
    	f[1] = pos[0] * TexMat[0][1] + pos[1] * TexMat[1][1] + TexMat[3][1];

    	if(f[0] < 0.0) f[0] = 0.0;
    	if(f[0] > 1.0) f[0] = 1.0;

    	if(f[1] < 0.0) f[1] = 0.0;
    	if(f[1] > 1.0) f[1] = 1.0;

    	pfGetClipTextureVirtualSize(clip, &width, &height, &depth);
    	pfMPClipTextureCenter(mpcliptex, (int)(f[0]*width), 
		(int)(f[1]*height), 0);
    }
    return PFTRAV_CONT;
}

pfNode *
loadASDConfigFile(char *confname)
{
    pfBuffer *buf;
    int i, num, numlods, bind;
    void *dummy;
    pfASD *asd;
    pfBox box;
    pfGeoState *gs, **gsa;

    asd = setASDData(confname);
    if(asd == NULL) 
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
            "setASDGstates:There is no ASD node loaded.");
        return(NULL);
    }
    pfNodeTravFuncs(asd, PFTRAV_APP, cliptexApp, NULL);

    pfGetASDAttr(asd, PFASD_LODS, &bind, &numlods, &dummy);
    /*
     *  Replace the loader default GeoState with the (clip)texture
     *  GeoState as supplied in the .config file.
     */

    pfGetASDGStates(asd, &gsa, &num);
    if (gsa)
    {
	for (i=0; i < num; i++)
	{
	    pfUnrefDelete(gsa[i]);
	}
    }

    gsa = pfMalloc(sizeof(pfGeoState*), pfGetSharedArena());
    gs = pfNewGState(pfGetSharedArena());
    buildGState(gs);
    *gsa = gs;
    pfASDGStates(asd, gsa, 1);

    return (pfNode *)asd;
}

/* the top view channel shares the active mesh output from the master
 * channel. the channel itself does not do any evaluation */
void
initInset(pfChannel *masterChan)
{
    pfVec3 xyz, hpr, center;
    float r;
    pfSphere    bsphere;

    /* set up an inset channel for a topdown view of the terrain */
    ViewState->insetChannel = pfNewChan(pfGetChanPipe(masterChan));
    pfChanViewport(ViewState->insetChannel, 0.75, 0.95, 0.75, 1.0);
    pfChanScene(ViewState->insetChannel, ViewState->scene);

    /* top down view of the active geometry in master Channel */
    pfASDattachChan(ViewState->insetChannel, masterChan);
    pfGetNodeBSphere(ViewState->scene, &bsphere);
    pfMakeOrthoChan(ViewState->insetChannel,
        bsphere.center[0]-bsphere.radius,
        bsphere.center[0]+bsphere.radius,
        bsphere.center[1]-bsphere.radius,
        bsphere.center[1]+bsphere.radius);
    pfChanNearFar(ViewState->insetChannel, ViewState->near/3,ViewState->far*2);
    xyz[PF_X] = 0;
    xyz[PF_Y] = 0;
    xyz[PF_Z] = bsphere.center[PF_Z]+2*ViewState->sceneSize;
    pfSetVec3(hpr, 0.0, -90.0, 0);
    pfChanView(ViewState->insetChannel, xyz, hpr);
    pfChanTravFunc(ViewState->insetChannel, PFTRAV_DRAW, insetDraw);
}

/* topview channel draws scribed terrain from a bird's eye view
 * on top of the regular channel. use a red dot to indicate the eye position
 */
void
insetDraw(pfChannel * chan, void *data)
{
    static pfVec4 col={1.0, 1.0, 1.0, 0.0};
    static pfVec4 ptcol = {1.0, 0, 0, 0};
    static pfVec3 ptcoords[4];
    static pfGeoSet *gset;
    static pfGeoState *gstate;

    static pfMaterial   *material       = NULL;
    static int overrides = PFSTATE_ENTEXTURE|PFSTATE_ENLIGHTING|
        PFSTATE_FRONTMTL|PFSTATE_BACKMTL;

    if (material == NULL)
    {
	/* set up a material for the topdown terrain - make it black */
        material = pfNewMtl(pfGetSharedArena());
        pfMtlColor(material, PFMTL_AMBIENT,  0.0f, 0.0f, 0.0f);
        pfMtlColor(material, PFMTL_DIFFUSE,  0.0f, 0.0f, 0.0f);
        pfMtlColor(material, PFMTL_SPECULAR, 0.0f, 0.0f, 0.0f);
        pfMtlShininess(material, 0.0f);
        pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_OFF);

	/* create a gset for the viewpoint dot */
        gset = pfNewGSet(NULL);
        pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, ptcoords, NULL);
        pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, ptcol, NULL);
        pfGSetPrimType(gset, PFGS_POINTS);
        pfGSetNumPrims(gset, 1);
        pfGSetPntSize(gset, 12.0);
        gstate = pfNewGState (NULL);
        pfGStateMode(gstate, PFSTATE_ENLIGHTING, 0);
        pfGSetGState (gset, gstate);
    }

    pfPushState();

    /* set up render state to draw the topdown outline view */
    pfApplyMtl(material);
    pfClear(PFCL_DEPTH, NULL);
    pfEnable(PFEN_LIGHTING);
    pfDisable(PFEN_TEXTURE);

    /* draw the topdown outline view */
    pfOverride(overrides, 1);
    pfuPreDrawStyle(PFUSTYLE_SCRIBED, col);
    pfDraw();
    pfuPostDrawStyle(PFUSTYLE_SCRIBED);
    pfOverride(overrides, 0);

    /* draw the viewpoint dot */
    pfCopyVec3(ptcoords[0], ViewState->viewCoord.xyz);
    pfDrawGSet(gset);

    pfPopState();
}

void 
scapeEvents(pfuWidget *w)
{

  switch( pfuGetWidgetId(w) )
  {
    case GUI_RANGE_SCALE:
      {
	double slidervalue, switchin;

	slidervalue = pfuGetWidgetValue(w);
	ViewState->switchin = pow( 2, slidervalue );
      }
      break;

    case GUI_MAX_LOD:
      {
         ViewState->morphconstraint = pfuGetWidgetValue(w);
      }
      break;

    case GUI_MIN_RANGE:
      {
	ViewState->minRange = pfuGetWidgetValue(w);
      }
      break;

    case GUI_INSET:
      {
	ViewState->insetWin = (int)pfuGetWidgetValue(w);
      }
      break;

    case GUI_DEMO:
      {
        ViewState->demoMode = pfuGetWidgetValue(w);
      }
      break;

    case GUI_COLOR:
      {
	ViewState->colorstyle = (int)pfuGetWidgetValue(w);
      }
      break;

    case GUI_FIXPOS:
      {
	ViewState->fixpos = (int)pfuGetWidgetValue(w);
      }
      break;

    case GUI_MOVERATIO:
      {
	ViewState->moveratio = pfuGetWidgetValue(w);
      }
      break;

    case GUI_LOAD_BALANCE:
      {
	ViewState->loadRatio = pfuGetWidgetValue(w);
      }
      break;

    case GUI_BOX_LOD:
      {
	ViewState->boxLOD = pfuGetWidgetValue(w);
      }
      break;

    case GUI_MORPH_LIMIT:
      {
	ViewState->morphLimit = pfuGetWidgetValue(w);
      }
      break;

    case GUI_EVAL_METHOD:
      {
	ViewState->evalMethod = pfuGetWidgetValue(w);
      }
      break;

#ifdef ADJUST_TEXTURE
    case GUI_TEX_S:
      {
 	pfMatrix mat, stmatrix;
  	int i, j;
	ViewState->texS = pfuGetWidgetValue(w);
       	
	pfCopyMat(mat, ViewState->fixmat);
        pfMakeTransMat(stmatrix, ViewState->texS, ViewState->texT, 0);
        pfPostMultMat(mat, stmatrix);

	for(i = 0; i < 4; i++)
            for(j =0; j< 4; j++)
		ViewState->mat[i*4+j]=mat[i][j];

    printf("changed S. matrix %f %f %f %f, %f %f %f %f, %f %f %f %f, %f %f %f %f\n",
        ViewState->mat[0*4+0], ViewState->mat[0*4+1], ViewState->mat[0*4+2], ViewState->mat[0*4+3],
        ViewState->mat[1*4+0], ViewState->mat[1*4+1], ViewState->mat[1*4+2], ViewState->mat[1*4+3],
        ViewState->mat[2*4+0], ViewState->mat[2*4+1], ViewState->mat[2*4+2], ViewState->mat[2*4+3],
        ViewState->mat[3*4+0], ViewState->mat[3*4+1], ViewState->mat[3*4+2], ViewState->mat[3*4+3]);


      }
      break;

    case GUI_TEX_T:
      {
        pfMatrix mat, stmatrix;
	int i, j;
        ViewState->texT = pfuGetWidgetValue(w);

	pfCopyMat(mat, ViewState->fixmat);
        pfMakeTransMat(stmatrix, ViewState->texS, ViewState->texT, 0);
        pfPostMultMat(mat, stmatrix);

        for(i = 0; i < 4; i++)
            for(j =0; j< 4; j++)
                ViewState->mat[i*4+j]=mat[i][j];

    printf("changed T, matrix %f %f %f %f, %f %f %f %f, %f %f %f %f, %f %f %f %f\n",
        ViewState->mat[0*4+0], ViewState->mat[0*4+1], ViewState->mat[0*4+2], ViewState->mat[0*4+3],
        ViewState->mat[1*4+0], ViewState->mat[1*4+1], ViewState->mat[1*4+2], ViewState->mat[1*4+3],
        ViewState->mat[2*4+0], ViewState->mat[2*4+1], ViewState->mat[2*4+2], ViewState->mat[2*4+3],
        ViewState->mat[3*4+0], ViewState->mat[3*4+1], ViewState->mat[3*4+2], ViewState->mat[3*4+3]);

      }
      break;
#endif

    default:
      break;

  }
}

void
resetlod( pfASDLODRange *lods, int numlods, double swin )
{
  int i;
  double trans, nextswin;
  pfASDLODRange *tmplod;

  tmplod = &lods[0];
  tmplod->morph = 0;
  tmplod->switchin = swin;

  if (ViewState->evalMethod == PFASD_DIST)
  {
      for (i=1; i<numlods; i++)
      {
          nextswin = swin/2;
          trans = nextswin + ((swin - nextswin) * 0.25);

          tmplod = &lods[i];

          tmplod->switchin = swin;
          /*tmplod->morph    = tmplod->switchin - (trans*trans);*/
          tmplod->morph    = 0.75*swin;
          swin = nextswin;
      }
  }
  else
  {
      for (i=1; i<numlods; i++)
      {
          nextswin = swin/2;

          tmplod = &lods[i];

          tmplod->switchin = swin;
          tmplod->morph    = 0.6*swin;
          swin = nextswin;
      }
  }
}


void
updateASD(void)
{
    pfASD *asd;
    int i, numasd, num;
    pfASDLODRange *adjlods; 
    int methodSwitched = 0;
    static int computedEvalData = 0;
    static pfASDLODRange *newlods = NULL;
    int bind;

    if((numasd=pfGetNum(ViewState->asdlist))==0) 
        return;

    if(ViewState->morphattrs == NULL)
/* give it an overall normal and an overall color */
    {
	ASDAttrs *attrs;

        ViewState->morphattrs = (float *)pfMalloc(14*sizeof(float), pfGetSharedArena());

        ViewState->morphattrs[0] = 0;
        ViewState->morphattrs[1] = 0;
        ViewState->morphattrs[2] = 1;
        ViewState->morphattrs[3] = 0;
        ViewState->morphattrs[4] = 0;
        ViewState->morphattrs[5] = 0;

        ViewState->morphattrs[6] = 0.7;
        ViewState->morphattrs[7] = 0.7;
        ViewState->morphattrs[8] = 0.7;
        ViewState->morphattrs[9] = 1.0;
        ViewState->morphattrs[10] = 0.0;
        ViewState->morphattrs[11] = -0.7;
        ViewState->morphattrs[12] = 0.0;
        ViewState->morphattrs[13] = 0.0;

        ViewState->bandattrs = (float *)pfMalloc(6*sizeof(float), pfGetSharedArena());

        ViewState->bandattrs[0] = 0;
        ViewState->bandattrs[1] = 0;
        ViewState->bandattrs[2] = 1;

        ViewState->bandattrs[3] = 0;
        ViewState->bandattrs[4] = 0;
        ViewState->bandattrs[5] = 0;

	for(i = 0; i < numasd; i++)
	{
	    asd = pfGet(ViewState->asdlist, i);
	    attrs = (ASDAttrs *)pfMalloc(sizeof(ASDAttrs), pfGetSharedArena());
            pfGetASDAttr(asd, PFASD_PER_VERTEX_ATTR, &attrs->bind, &attrs->numattrs, (void**)&attrs->attrs);
	    if(attrs->numattrs == 0)
	    {
	    	pfGetASDAttr(asd, PFASD_OVERALL_ATTR, &attrs->bind, &attrs->numattrs, (void**)&attrs->attrs);
		attrs->mode = PFASD_OVERALL_ATTR;
	    }
	    else
		attrs->mode = PFASD_PER_VERTEX_ATTR;
	    attrs->morphattrs = pfGetASDMorphAttrs(asd);
	    pfAdd(ViewState->origattrs, attrs);
	}
    }

    for(i = 0; i < numasd; i++)
    {
	ASDAttrs *attrs;
	asd = pfGet(ViewState->asdlist, i);
        if(ViewState->colorstyle != ViewState->precolorstyle)
        {
	    if(ViewState->colorstyle == PFASD_COLORMORPH)
	    {
		/* give it an overall normal and an overall color */
		pfASDAttr(asd, PFASD_OVERALL_ATTR, PFASD_NORMALS|PFASD_COLORS, 
		    1, ViewState->morphattrs);
		/* morph the color according to morph weights */
		pfASDMorphAttrs(asd, PFASD_COLORS);
		ViewState->lighting = LIGHTING_OFF;
		ViewState->texture = FALSE;
	    }
	    else if(ViewState->colorstyle == PFASD_COLORBAND)
	    {
		pfASDAttr(asd, PFASD_OVERALL_ATTR, PFASD_NORMALS, 
		    1, ViewState->bandattrs);
		pfASDMorphAttrs(asd, PFASD_NO_ATTR);
	       /* this is designed for .evt files since the color of terrain triangles are assigned by their LOD levels. 
		purely for demonstration purposes */
		attrs = pfGet(ViewState->origattrs, i);
		if(attrs->mode == PFASD_PER_VERTEX_ATTR && attrs->bind&PFASD_COLORS)
		{
		    attrs = pfGet(ViewState->origattrs, i);
		    pfASDAttr(asd, PFASD_PER_VERTEX_ATTR, PFASD_COLORS, 
			attrs->numattrs, attrs->attrs);
		    pfASDMorphAttrs(asd, PFASD_COLORS);
		}
		ViewState->lighting = LIGHTING_OFF;
		ViewState->texture = FALSE;
	    }
	    else
	    {
		/* restore the original attrs */
		attrs = pfGet(ViewState->origattrs, i);
		pfASDAttr(asd, attrs->mode, attrs->bind,
                        attrs->numattrs, attrs->attrs);
		pfASDMorphAttrs(asd, attrs->morphattrs);
		ViewState->lighting = LIGHTING_SUN;
		ViewState->texture = TRUE;
	    }
	}
	updateWidget(GUI_LIGHTING, (float)ViewState->lighting);
	updateWidget(GUI_TEXTURE, (float)ViewState->texture);
     
	if(pfGetASDEvalMethod(asd) != ViewState->evalMethod)
	{
	    if (ViewState->evalMethod == PFASD_DIST)
		pfASDEvalMethod(asd, PFASD_DIST);
	    else if (ViewState->evalMethod == PFASD_DIST_SQUARE)
		pfASDEvalMethod(asd, PFASD_DIST_SQUARE);
	    else if (ViewState->evalMethod == PFASD_CALLBACK)
	    {
		if (!computedEvalData)
		{
		    lodEvalFunc_init(asd);
		    computedEvalData = 1;
		}
		pfASDEvalFunc(asd, lodEvalFunc);
	    }

	    methodSwitched = 1;
	}

	/*
	 *    XXX This next line should really be called immediately before
	 *    each pass of terrain LOD evaluation, rather than at this
	 *    point in every frame.
	 */

	if (ViewState->evalMethod == PFASD_CALLBACK)
	    lodEvalFunc_pass();
	
	pfGetASDAttr(asd, PFASD_LODS, &bind, &num, (void **)(&adjlods));
	if((adjlods[0].switchin != ViewState->switchin) || methodSwitched) 
	{
	    if(newlods == NULL)
		newlods = (pfASDLODRange *)pfMalloc(num*sizeof(pfASDLODRange), 
			    pfGetSharedArena());
	    /* we over write the lods */
	    resetlod(newlods, num, ViewState->switchin); 
	    pfASDAttr(asd, PFASD_LODS, NULL, num, newlods);
	}

	{
	    int maxlod, m1;
	    float morphweightconstraint, w1;

	    /* pfASD morph weight is defined as 
	     *1 -- NOT morphed and 0 -- fully morphed.
	     * Therefore if 3.1 means only morphed a bit into level 3, 
	     * and 3.7 means morphed quite a bit into level 3, 
	     * then we have to do a 1-blah business
	     * to adjust to the right ASD morph definition.
	     */
	    pfGetASDMaxMorphDepth(asd, &maxlod, &morphweightconstraint);
	    m1 = (int)ViewState->morphconstraint;
	    w1 = 1.0-(ViewState->morphconstraint-(int)ViewState->morphconstraint);

	    /* if morphweightconstraint is 0, then, it means use that level, 
	     * but do not morph. so, we might as well use the lower level 
	     * and let it morph normally.
	     * it saves one more level of evaluation.
	     * for example, if the slider is 4.0, 
	     * we will pass in (3, 0.0), instead of (4,1.0).
	     */
	    if(w1 == 1.0) 
	    {
		m1 = PF_MAX2(0, m1-1);
		w1 = 0.0;
	    }

	    if((maxlod != m1) ||
		(morphweightconstraint != w1))
		pfASDMaxMorphDepth(asd, m1, w1 );

	}
    }
    ViewState->precolorstyle = ViewState->colorstyle;
/*
    {
        struct mallinfo     mall;
    	mall = mallinfo();
    	printMallInfo(&mall);

    	mall = amallinfo(pfGetSharedArena());
    	printAMallInfo(&mall);
    }
*/
}

