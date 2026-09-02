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
 * shadows.c: Performer program to demonstrate use of pfLightSource shadowing
 *		and projected texturing.
 *        Based on simple.c
 *
 * $Revision: 1.25 $ $Date: 2002/11/14 00:02:43 $ 
 *
 */

static int Floor= 0;
static Ambient = 0;
static int SpotOne=0;
static int SpotTwo=0;
static int Whites=0;
static int Shadow=0;
static int SpotTexture = 1;
static int enable_shadows = 1; /* disable shadows in HW does not handle it */

#include <stdlib.h>
#include <math.h>
#ifndef WIN32
#include <unistd.h>
#define None 0
#endif
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#endif


/*------------------------------------------------------------------*/


static int FBAttrs[] = {
    PFFB_RGBA,
    PFFB_DOUBLEBUFFER,
    PFFB_DEPTH_SIZE, 1,
    NULL,
};

char ProgName[PF_MAXSTRING];

void 
OpenPipeWin (pfPipeWindow *pw)
{
    pfLightModel	*lm;

    pfOpenPWin(pw);

    lm = pfNewLModel(NULL);
    pfLModelLocal(lm, 1);
    pfApplyLModel(lm);

    pfCullFace(PFCF_BACK);
}

/* ARGSUSED */
void 
DrawChannel (pfChannel *chan, void *data)
{
    pfVec4	clr;
    pfSetVec4(clr, .2f, .2f, .2f, 1.0f); 
    pfClear(PFCL_COLOR|PFCL_DEPTH, clr);
    pfDraw();
}

/*
 *	Usage() -- print usage advice and exit. This
 *      procedure is executed in the application process.
 */
static void
Usage (void)
{
    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, "Usage: shadows [-1 -2 -a -s -f -w -t] file.ext ...\n");
    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, "       -1 : enable projected texture 1 \n");
    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, "       -2 : enable projected texture 2 \n");
    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, "       -a : enable regular light\n");
    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, "       -s : enable shadow \n");
    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, "       -f : use the texture floor5.rgb on the floor\n");
    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, "       -w : use white spots lights \n");
    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, "       -t : use texture myspot.tex for the projected textures\n");
    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, "Default: -1 -2 -a -s -f\n");

    exit(1);
}

static void
Default(void)
{
    SpotOne = SpotTwo = Shadow = Ambient = Floor = 1;
}

/*
*	docmdline() -- use getopt to get command-line arguments, 
*	executed at the start of the application process.
*/

static int
docmdline(int argc, char *argv[])
{
    int	    opt;

    strcpy(ProgName, argv[0]);
    
    /* process command-line arguments */
    while ((opt = getopt(argc, argv, "12asfwt")) != -1)
    {
	switch (opt)
	{
	case '1': 
	    SpotOne = 1;
	    break;
	case '2': 
	    SpotTwo = 1;
	    break;
	case 's':
	    Shadow = 1;
	    break;
	case 'f': 
	    Floor = 1;
	    break;
	case 't': 
	    SpotTexture = 1;
	    break;
	case 'a': 
	    Ambient = 1;
	    break;
	case 'w':
	    Whites = 1;
	    break;
	case 'h': 
	    Usage();
	}
    }
    return optind;
}

static pfVec3	zaxis = {0.0f, 0.0f, -1.0f};	
/* Compute matrix so +Y axis of DCS is pointing at 'at' */
static void
lookAt(pfDCS *dcs, pfVec3 at)
{
    pfVec3		v, pos;
    pfMatrix		mat;

    /* Light sources point down -z axis by default, so we want to rotate the
       dcs with the light source under it so that its -z axis points at the
       specified point
       */
    
    
    /* v is origin of dcs */
    pfGetMatRowVec3(*pfGetDCSMatPtr(dcs), 3, pos); 
    
    /* v is direction vector from dcs to 'at' */
    pfSubVec3(v, at, pos);
    pfNormalizeVec3(v);

    /* Rotate y-axis onto v */
    pfMakeVecRotVecMat(mat, zaxis, v);
    pfSetMatRowVec3(mat, 3, pos); /* restore dcs origin */
    pfDCSMat(dcs, mat);
}

#define SPOT_SIZE	256
#define SPOT_RADIUS	64

/*
 * Create circular image for projected spotlight texture.
*/
static pfTexture*
initSpotTex(void)
{
    long            	i, j;
    pfTexture		*tex;
    unsigned short 	*image;

    image = pfMalloc(sizeof(unsigned short) * SPOT_SIZE * SPOT_SIZE,
		     pfGetSharedArena());

    for (i = 0; i < SPOT_SIZE / 2; i++)
    {
	for (j = 0; j < SPOT_SIZE / 2; j++)
	{
	    unsigned long   val;

	    val = 0;

	    /* See if corners of texel are within circle. This antialiases
	     * the edge of the circle somewhat. */
	    if ((i * i + j * j) <= SPOT_RADIUS * SPOT_RADIUS)
		val += 0xffff;
	    if (((i + 1) * (i + 1) + j * j) <= SPOT_RADIUS * SPOT_RADIUS)
		val += 0xffff;
	    if (((i + 1) * (i + 1) + (j + 1) * (j + 1)) <= SPOT_RADIUS * SPOT_RADIUS)
		val += 0xffff;
	    if ((i * i + (j + 1) * (j + 1)) <= SPOT_RADIUS * SPOT_RADIUS)
		val += 0xffff;

	    val >>= 2;
	    /* set alpha to 0 for openGL */
	    val &=  0xfff0;

	    /* Set all 4 quadrants to val */
	    image[(SPOT_SIZE / 2 - i) * SPOT_SIZE + (SPOT_SIZE / 2 - j)] =
		image[(SPOT_SIZE / 2 - i) * SPOT_SIZE + (SPOT_SIZE / 2 + j)] =
		image[(SPOT_SIZE / 2 + i) * SPOT_SIZE + (SPOT_SIZE / 2 + j)] =
		image[(SPOT_SIZE / 2 + i) * SPOT_SIZE + (SPOT_SIZE / 2 - j)] =
		(unsigned short) val;

	}
    }

    tex = pfNewTex(pfGetSharedArena());
    pfTexImage(tex, (uint*) image, 2, SPOT_SIZE, SPOT_SIZE, 0);
    pfTexFormat(tex, PFTEX_INTERNAL_FORMAT, PFTEX_IA_8);
    pfTexRepeat(tex, PFTEX_WRAP, PFTEX_CLAMP);

    /*
	pfSaveTexFile(tex, "myspot.tex");
    */
    return tex;
}

#define FOV		45.0f
#define NEAR_		1.0f
#define SQRT2INV	.7071067811865475244

static pfDCS 		*shadDCS, *projDCS, *proj2DCS;
static pfLightSource 	*shad, *proj, *proj2;

static pfGroup*
initLSources(pfSphere *bsphere)
{
    pfFrustum 		*shadFrust;
    pfTexture 		*tex;
    pfGroup		*group;
    float		d;
    pfMatrix		mat;
    void		*arena = pfGetSharedArena();

    /* Create and configure shadow frustum. Fit frustum tightly to scene. */
    shadFrust = pfNewFrust(arena);
    
    pfMakeSimpleFrust(shadFrust, FOV);

    d = bsphere->radius / sinf(PF_DEG2RAD(FOV/2.0f));
    d = PF_MAX2(d, NEAR_ + bsphere->radius);

    pfFrustNearFar(shadFrust, NEAR_, d + 1.1f * bsphere->radius);

    if (enable_shadows)
    {
    	/* Create and configure white shadow casting light source */
    	shad = pfNewLSource();
    	pfLSourceMode(shad, PFLS_SHADOW_ENABLE, 1);
    	pfLSourceAttr(shad, PFLS_PROJ_FRUST, shadFrust);
    	pfLSourceColor(shad, PFLT_DIFFUSE, 1.0f, 1.0f, 1.0f);
    	pfLSourceVal(shad, PFLS_INTENSITY, .2f);   /* shadow not complety dark */
    	pfLSourcePos(shad, 0.0f, 0.0f, 0.0f, 1.0f);	/* Make light local */
	pfLSourceVal(shad, PFLS_SHADOW_SIZE, SPOT_SIZE);
	pfSpotLSourceDir(shad, 0, 0, -1);
    }

    /* Create and configure blue spotlight */
    proj = pfNewLSource();
    if (SpotTexture)
        tex = initSpotTex();
    else
    {
       tex = pfNewTex(arena);
       pfLoadTexFile(tex, "myspot.tex");
       pfTexRepeat(tex, PFTEX_WRAP, PFTEX_CLAMP);
    }

    pfLSourceMode(proj, PFLS_PROJTEX_ENABLE, 1);
    pfLSourceAttr(proj, PFLS_PROJ_FRUST, shadFrust);
    pfLSourceAttr(proj, PFLS_PROJ_TEX, tex);
    if (Whites) {
      pfLSourceColor(proj, PFLT_DIFFUSE, 1.0f, 1.0f, 1.0f);
      pfLSourceColor(proj, PFLT_SPECULAR, 1.0f, 1.0f, 1.0f);
    }
    else {
      pfLSourceColor(proj, PFLT_DIFFUSE, 0.0f, 0.0f, 1.0f);
      pfLSourceColor(proj, PFLT_SPECULAR, 0.0f, 0.0f, 1.0f);
    }

    pfLSourceColor(proj, PFLT_AMBIENT, 0.0f, 0.0f, 0.0f);
    pfLSourceVal(proj, PFLS_INTENSITY, 0.5f);
    pfLSourcePos(proj, 0.0f, 0.0f, 0.0f, 1.0f);	/* Make light local */
    pfSpotLSourceDir(proj, 0, 0, -1);

    group = pfNewGroup();

    /* Make DCSes to move lights around */
    if (enable_shadows)
    {

    	shadDCS = pfNewDCS();

    	pfMakeTransMat(mat, d*SQRT2INV, 0.0f, d*SQRT2INV);
    	pfDCSMat(shadDCS, mat);
    	lookAt(shadDCS, bsphere->center);

    	pfAddChild(shadDCS, shad);
	if (Shadow)
    	    pfAddChild(group, shadDCS);
    }

    projDCS = pfNewDCS();

    pfMakeTransMat(mat, -d*SQRT2INV, 0.0f, d*SQRT2INV);
    pfDCSMat(projDCS, mat);
    lookAt(projDCS, bsphere->center);

    pfAddChild(projDCS, proj);
    if (SpotOne)
        pfAddChild(group, projDCS);
    if (SpotTwo)
    {
	/* Create and configure red spotlight */
	proj2 = pfNewLSource();
	pfLSourceMode(proj2, PFLS_PROJTEX_ENABLE, 1);
	pfLSourceAttr(proj2, PFLS_PROJ_FRUST, shadFrust);
	pfLSourceAttr(proj2, PFLS_PROJ_TEX, tex);
	pfSpotLSourceDir(proj2, 0, 0, -1);
	if (Whites) {
	  pfLSourceColor(proj2, PFLT_DIFFUSE, 1.0f, 1.0f, 1.0f);
	  pfLSourceColor(proj2, PFLT_SPECULAR, 1.0f, 1.0f, 1.0f);
	}
	else {
	  pfLSourceColor(proj2, PFLT_DIFFUSE, 1.0f, 0.0f, 0.0f);
	  pfLSourceColor(proj2, PFLT_SPECULAR, 1.0f, 0.0f, 0.0f);
	}
	pfLSourceColor(proj2, PFLT_AMBIENT, 0.0f, 0.0f, 0.0f);
	pfLSourceVal(proj2, PFLS_INTENSITY, .5f);
	pfLSourcePos(proj2, 0.0f, 0.0f, 0.0f, 1.0f); /* Make light local */

	proj2DCS = pfNewDCS();
	pfMakeTransMat(mat, d*SQRT2INV, 0.0f , d*SQRT2INV);
	pfDCSMat(proj2DCS, mat);
	lookAt(proj2DCS, bsphere->center);
	pfAddChild(proj2DCS, proj2);
	pfAddChild(group,proj2DCS);
    }
    /* Add a regular white light */
    if (Ambient)
        pfAddChild(group, pfNewLSource());

    return group;
}


static pfGeode*
initFloor(pfSphere *bsphere)
{
    pfGeoSet	*gset;
    pfGeoState	*gstate;
    pfGeode	*geode;
    pfVec4 *c;
    pfVec3	*v;
    pfMaterial	*mtl;
    float	d;
    void	*arena = pfGetSharedArena();
    pfVec2 *t;
    pfTexture *tex;
    pfTexEnv  *tev;

    gset = pfNewGSet(arena);
    gstate = pfNewGState(arena);
    geode = pfNewGeode();
    
    v = (pfVec3*)pfMalloc(sizeof(pfVec3) * 4, arena);
    d = 1.7f * bsphere->radius; 
    

    pfSetVec3(v[0], -d, -d, -d/3.0f);
    pfSetVec3(v[1],  d, -d, -d/3.0f);
    pfSetVec3(v[2],  d,  d, -d/3.0f);
    pfSetVec3(v[3], -d,  d, -d/3.0f);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, v, NULL);

    v = (pfVec3*)pfMalloc(sizeof(pfVec3), arena);
    pfSetVec3(v[0], 0.0f, 0.0f, 1.0f);
    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, v, NULL);

    if (Floor)
    {
      	c = (pfVec4*) pfMalloc(sizeof(pfVec4) , pfGetSharedArena());
 
	c[0][0] = 0.5;
	c[0][1] = 0.5;
	c[0][2] = 0.5;
	c[0][3] = 1.;
 
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, c, NULL);

	t = (pfVec2*) pfMalloc(sizeof(pfVec2) *4, pfGetSharedArena());
	t[0][0] = 0.; t[0][1] = 0.;
	t[1][0] = 1.; t[1][1] = 0.;
	t[2][0] = 1.; t[2][1] = 1.;
	t[3][0] = 0.; t[3][1] = 1.;
	
	pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, t, NULL);
    }
     else
    {

	c = (pfVec4*) pfMalloc(sizeof(pfVec4) , pfGetSharedArena());
 
	c[0][0] = 0.5;
	c[0][1] = 0.5;
	c[0][2] = 0.5;
	c[0][3] = 1.;
 
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, c, NULL);
    }


    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 1);

    if (Floor)
    {
	tex = pfNewTex(pfGetSharedArena());
	pfLoadTexFile(tex,"floor5.rgb");

	pfTexFormat(tex,PFTEX_INTERNAL_FORMAT, PFTEX_RGB_8);
	pfTexRepeat(tex,PFTEX_WRAP, PFTEX_REPEAT);
       
	pfGStateAttr(gstate, PFSTATE_TEXTURE, tex);
	pfGStateMode(gstate, PFSTATE_ENTEXTURE, 1);

	tev = pfNewTEnv(pfGetSharedArena());

	pfTEnvMode(tev, PFTE_DECAL);
	pfGStateAttr(gstate,PFSTATE_TEXENV, tev);

	mtl = pfNewMtl(arena);    
	pfMtlColorMode(mtl, PFMTL_BOTH, PFMTL_CMODE_OFF);
	pfMtlColor(mtl,PFMTL_AMBIENT, 0.0, 0.0, 0.0);
	pfGStateAttr(gstate, PFSTATE_FRONTMTL, mtl);
	pfGStateMode(gstate, PFSTATE_ENLIGHTING, 1);
    }

    pfGSetGState(gset, gstate);
    pfAddGSet(geode, gset);

    return geode;
}

int
main (int argc, char *argv[])
{
    int loop;
    
    pfScene     *scene;
    pfNode	*root;
    pfDCS	*dcs;
    pfPipe      *p;
    pfPipeWindow *pw;
    pfChannel   *chan;
    pfSphere 	bsphere;
    pfCoord	view;
    pfMatrix	mat, orbit1, orbit2, orbit3;
    pfList	*texList;
    int		ret;
    int 	arg = 1;
    int		found;
    pfGeoState  *gstate;
    pfLightModel *lm;

    if (argc < 2)
	Usage();
    if (argc == 2)
	Default();
    else
	arg = docmdline(argc,argv);


    /* Initialize Performer */
    pfInit();	


    pfQueryFeature(PFQFTR_TEXTURE_PROJECTIVE, &ret);
    if (!(ret == PFQFTR_FAST))
    {
	printf("Sorry, Projected Light Sources are not supported \n");
	return 0;
    }

    if (Shadow)
    {
	pfQueryFeature(PFQFTR_TEXTURE_SHADOW, &ret);
	if (!(ret == PFQFTR_FAST))
	{
	    printf("Sorry, Shadow are not supported !\n");
	    enable_shadows = 0;
	}
    } else
    enable_shadows = 0;

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess(PFMP_DEFAULT); 

    /* Load all loader DSO's before pfConfig() forks */
    for (found = arg; found < argc; found++)
        pfdInitConverter(argv[found]);

    /* Configure multiprocessing mode and start parallel
     * processes.
     */
    pfConfig();			

    /* Append to PFPATH additional standard directories where 
     * geometry and textures exist 
     */
    pfFilePath(".:/usr/share/Performer/data");

    /* Do not use FLAT_ primitives because they look bad 
     * with local lighting.
    */
    pfdBldrMode(PFDBLDR_MESH_LOCAL_LIGHTING, 1);

    /* load files named by command line arguments */
    scene = pfNewScene();
    dcs = pfNewDCS();
    pfMakeRotMat(mat, 30.0f, 1.0f, 0.0f, 0.0f);
    pfDCSMat(dcs, mat);
    pfAddChild(scene, dcs);

    for (found = 0; arg < argc; arg++)
    {
        if ((root = pfdLoadFile(argv[arg])) != NULL)
        {
            pfAddChild(dcs, root);
            found++;
        }
    }

    if (!found)
    {
	pfNotify(PFNFY_FATAL,PFNFY_USAGE,"Error loading database files");
	exit(-1);
    }


    /* determine extent of scene's geometry */
    pfGetNodeBSphere (scene, &bsphere);

    /* Create pfLightSources and add to scene */
    pfAddChild(scene, initLSources(&bsphere));

    /* Create "floor" for shadows */
    pfAddChild(dcs, initFloor(&bsphere));

    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinName(pw, argv[0]);
    pfPWinConfigFunc(pw, OpenPipeWin);	
    pfPWinOriginSize(pw, 0, 0, 500, 500);
    pfPWinType(pw, PFPWIN_TYPE_X);
    pfPWinFBConfigAttrs(pw, FBAttrs);
    pfConfigPWin(pw);

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 5.0f * bsphere.radius);
    pfChanFOV(chan, 45.0f, 0.0f);
    pfCopyVec3(view.xyz, bsphere.center);
    view.xyz[PF_Y] -= 3.0f * bsphere.radius;
    pfSetVec3(view.hpr, 0.0f, 0.0f, 0.0f); 
    pfChanView(chan, view.xyz, view.hpr);
    pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);
    
    gstate = pfNewGState(pfGetSharedArena());
    lm = pfNewLModel(pfGetSharedArena());
    
    pfLModelAmbient(lm, 0, 0, 0);
    pfGStateMode(gstate, PFSTATE_ENLIGHTING, 1);
    pfGStateAttr(gstate, PFSTATE_LIGHTMODEL, lm);
    pfChanGState(chan, gstate);

    /* If scene is non-textured then we can save an entire rendering pass */
    texList = pfuMakeSceneTexList(scene);
    if (pfGetNum(texList) == 0)
    {
	pfChanTravMode(chan, PFTRAV_MULTIPASS, 
		       PFMPASS_NONTEX_SCENE | 
		       PFMPASS_EMISSIVE_PASS |
		       PFMPASS_BLACK_PASS);
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
		 "Scene is non-textured! Using PFMPASS_NONTEX_SCENE "
		 "optimization");
    }
    else {
      pfChanTravMode(chan, PFTRAV_MULTIPASS, 
		     PFMPASS_EMISSIVE_PASS | PFMPASS_BLACK_PASS);
    }

    pfMakeRotMat(orbit1, 1.3f, 0.0f, 0.0f, 1.0f);
    pfMakeRotMat(orbit2, 0.7f, 0.0f, 0.0f, 1.0f);
    pfMakeRotMat(orbit3, -1.2f, 0.f, 0.0f, 1.0f);

    /* Simulate for ever ! */
    loop = 0;
    while (1)
    {
	loop ++;
	/* Spin light sources around object */
	if (SpotOne)
	{
	    pfMultMat(mat, *pfGetDCSMatPtr(projDCS), orbit1);
	    pfDCSMat(projDCS, mat);
	    lookAt(projDCS, bsphere.center);
	}
	if (SpotTwo)
	{
	    pfMultMat(mat, *pfGetDCSMatPtr(proj2DCS), orbit2);
	    pfDCSMat(proj2DCS, mat);
	    lookAt(proj2DCS, bsphere.center);
	}
	if (enable_shadows)
	{
		pfMultMat(mat, *pfGetDCSMatPtr(shadDCS), orbit3);
		pfDCSMat(shadDCS, mat);
		lookAt(shadDCS, bsphere.center);
	}

	/* Initiate cull/draw for this frame. */
	pfFrame();		
   {
   int err;
   if ((err = glGetError()) != GL_NO_ERROR)
        pfNotify(PFNFY_NOTICE,PFNFY_USAGE,"OpenGL Error 0x%x - %s",err, gluErrorString(err));
   }

    }

    /*  press Ctrl-C to exit ! */
}

