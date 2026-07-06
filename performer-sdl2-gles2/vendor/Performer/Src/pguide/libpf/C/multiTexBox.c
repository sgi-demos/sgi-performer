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
 *
 * delete.c: 
 *
 * $Revision: 1.7 $ $Date: 2002/09/15 03:51:02 $ 
 *
 */

#include <stdlib.h>
#include <malloc.h>
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#ifdef WIN32
#define random rand
#endif

#define MP_MASK    (PFMP_APP_CULL_DRAW)			
/* #define MP_MASK    (PFMP_APPCULLDRAW) */
/* #define MP_MASK    (PFMP_DEFAULT) */

#define TABLE_SIZE	4

void draw_function(pfChannel* _chan, void* _userData);
pfNode *build_box(
		int 	num_textures, 
		float 	x_trans, 
		float 	y_trans, 
		float 	z_trans);
pfMaterial *defaultMaterial (void);

typedef struct
{
	int		done_delete_texture;
	pfNode		*model;
	pfList		*tex_list;
} 	delete_texture_command;

delete_texture_command	*command;

pfGroup			*group;

/*
 *	Usage() -- print usage advice and exit. This
 *      procedure is executed in the application process.
 */
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: delete file.ext ...\n");
    exit(1);
}


int	nof_textures[] = { 2, 2, 2, 2, 0, 1, 2, 1, 0, 2, 0, 2, 2, 1, 1, 0, 0 };

int	nof_textures_array_size = (sizeof (nof_textures) / sizeof (int));

int
main (int argc, char *argv[])
{
    float	    t = 0.0f;
    pfNode	    *root, *model;
    pfPipe	    *p;
    pfPipeWindow    *pw;
    pfChannel	    *chan;
    pfSphere	    bsphere;
    pfScene  	    *scene;
    int		    i;
    float	    base, step;
    int	     	    x, y;
    int		    t_count;
    float	    use_radius;

    /* 
     *	=============================================
     *	Initialize Performer 
     *	=============================================
     */
    pfInit();	

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess( MP_MASK );			

    command = pfMalloc (sizeof (delete_texture_command), pfGetSharedArena());
    command -> done_delete_texture = 1;
    command -> model = NULL;
    command -> tex_list = NULL;

    /* initiate multi-processing mode set in pfMultiprocess call 
     * FORKs for Performer processes,  CULL and DRAW, etc. happen here.
     */
    pfConfig();			

    /* Append to Performer search path, PFPATH, files in 
     *	    /usr/share/Performer/data */
    pfFilePath(".:/usr/share/Performer/data");

    /* Attach loaded file to a new pfScene. */
    scene = pfNewScene();

    group = pfNewGroup();
    pfAddChild(scene, group);

    /* Add the boxes. */

    step = 1.2;
    base = (-1.0) * step * ((float) (TABLE_SIZE - 1) / 2.0);

    t_count = 0;

    for (x = 0 ; x < TABLE_SIZE ; x ++)
	for (y = 0 ; y < TABLE_SIZE ; y ++ )
	{
	    root = build_box(nof_textures[t_count], 
				base + ((float) x) * step,
				0.0, 
				((float) y) * step - 1.0);

	    pfAddChild(group, root);

	    t_count = (t_count + 1) % nof_textures_array_size;
	}


#if 1
    pfdStoreFile (group, "multiTexBox.pfb");
#endif

    /* Create a pfLightSource and attach it to scene. */
    pfAddChild(scene, pfNewLSource());
    
    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinType(pw, PFPWIN_TYPE_X);
    pfPWinName(pw, "OpenGL Performer");
    pfPWinOriginSize(pw, 0, 0, 800, 800);
    /* Open and configure the GL window. */
    pfOpenPWin(pw);
    
    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanFOV(chan, 60.0f, 0.0f);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (root, &bsphere);
    pfChanNearFar(chan, 1.0f, 100.0f * bsphere.radius);

    pfChanTravFunc (chan, PFTRAV_DRAW, draw_function);

    while (1)
    {
        pfCoord    view;
        float      s, c;

        /* Go to sleep until next frame time. */
        pfSync();

        /* Initiate cull/draw for this frame. */
        pfFrame();

        /* Compute new view position. */
        t = pfGetTime();
        pfSinCos(45.0f*t, &s, &c);

	use_radius = 3.0 * bsphere.radius;

        pfSetVec3(view.hpr, 45.0f*t, -10.0f, 0);
        pfSetVec3(view.xyz, 
		 2.0f * use_radius * s,
                -2.0f * use_radius * c,
                 0.5f * use_radius);
        pfChanView(chan, view.xyz, view.hpr);
    }
}

/* ========================================================================= */
void draw_function(pfChannel* _chan, void* _userData)
/* ========================================================================= */
{
    pfClearChan (_chan);
    pfDraw();
}


static float	box[6][4*3] =
	{
		/* 	Bottom 		*/
		{
		-1.0, -1.0, -1.0,
		-1.0, 1.0, -1.0,
		1.0, 1.0, -1.0,
		1.0, -1.0, -1.0
		},
		/*	Top 		*/
		{
		-1.0, -1.0, 1.0,
		1.0, -1.0, 1.0,
		1.0, 1.0, 1.0,
		-1.0, 1.0, 1.0
		},
		{
		-1.0, -1.0, -1.0,
		-1.0, -1.0, 1.0,
		-1.0, 1.0, 1.0,
		-1.0, 1.0, -1.0
		},
		{
		1.0, -1.0, -1.0,
		1.0, 1.0, -1.0,
		1.0, 1.0, 1.0,
		1.0, -1.0, 1.0
		},
		{
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0, -1.0, 1.0,
		-1.0, -1.0, 1.0
		},
		{
		-1.0, 1.0, -1.0,
		-1.0, 1.0, 1.0,
		1.0, 1.0, 1.0,
		1.0, 1.0, -1.0
		},
	};

static float	box_colors[6][4] =
	{
	0.5, 0.5, 1.0, 1.0,
	0.5, 1.0, 0.5, 1.0,
	0.5, 1.0, 1.0, 1.0,
	1.0, 0.5, 0.5, 1.0,
	1.0, 0.5, 1.0, 1.0,
	1.0, 1.0, 1.0, 1.0
	};

/* Use a random collection of textures that ship with OpenGL-Performer. */

static  char    *texture_name[] =
        {
        "truckWheel.rgb",
        "truckSide.rgb",

        "hubcap.rgb",
        "road.rgb",

        "cafe.rgb",
        "floor5.rgb",

        "pfgroup.rgb",
        "marble.rgb",

        "license.rgb",
        "logo.128.rgb",

        "parquet.rgb",
        "prchmnt.rgb",

        "desert.rgb",
        "truckBack.rgb",

        "iris.rgb",
        "bark.rgb"
        };



/*==========================================================================*/
pfNode *build_box(int num_textures, float x_trans, float y_trans, float z_trans)
/*==========================================================================*/
{
int		i, face;
float		*vlist, *tlist, *clist;
float		*vp, *tp, *cp;
pfGeoSet	*gset;
pfGeoState	*gstate;
pfGeode		*geode;
pfTexture	*tex_0, *tex_1;
pfTexEnv	*tev_0, *tev_1;
pfSCS		*scs;
pfMatrix	mat0, mat1;
float		r, rx, ry, rz;
static		tex_index = 0;

	pfNotify (PFNFY_WARN, PFNFY_PRINT, ">>>>> build_box <<<<<<\n");

	gstate = pfNewGState (pfGetSharedArena());
	pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
	pfGStateMode (gstate, PFSTATE_ENLIGHTING, PF_OFF);

	pfGStateAttr (gstate, PFSTATE_FRONTMTL, defaultMaterial());
	pfGStateAttr (gstate, PFSTATE_BACKMTL, defaultMaterial());

	if (num_textures > 0)
	{
	    tex_0 = pfNewTex (pfGetSharedArena());
	    pfLoadTexFile (tex_0, texture_name[tex_index]);
	    tex_index = (tex_index + 1) % 16;
	}

	if (num_textures > 1)
	{
	    tex_1 = pfNewTex (pfGetSharedArena());
	    pfLoadTexFile (tex_1, texture_name[tex_index]);
	    tex_index = (tex_index + 1) % 16;
	}

	if (num_textures > 0)
	{
	    tev_0 = pfNewTEnv (pfGetSharedArena());
	    pfGStateMultiMode (gstate, PFSTATE_ENTEXTURE, 0, 1);
	    pfGStateMultiAttr (gstate, PFSTATE_TEXTURE, 0, tex_0);
	    pfGStateMultiAttr (gstate, PFSTATE_TEXENV, 0, tev_0);
	}

	if (num_textures > 1)
	{
	    tev_1 = pfNewTEnv (pfGetSharedArena());
	    pfGStateMultiMode (gstate, PFSTATE_ENTEXTURE, 1, 1);
	    pfGStateMultiAttr (gstate, PFSTATE_TEXTURE, 1, tex_1);
	    pfGStateMultiAttr (gstate, PFSTATE_TEXENV, 1, tev_1);
	}

	vlist = pfMalloc (24 * 3 * sizeof (float), pfGetSharedArena());
	tlist = pfMalloc (24 * 2 * sizeof (float), pfGetSharedArena());
	clist = pfMalloc (6 * 4 * sizeof (float), pfGetSharedArena());

	vp = vlist;
	tp = tlist;
	cp = clist;

	for (face = 0; face < 6; face ++)
		{
#if 0
		r = 0.5 + ((float) (random() % 1000)) / 2000.0;
#else
		r = 1.0;
#endif

		*cp ++ = box_colors[face][0] * r;
		*cp ++ = box_colors[face][1] * r;
		*cp ++ = box_colors[face][2] * r;
		*cp ++ = box_colors[face][3] * r;

		for (i = 0; i < 4 * 3; i ++)
			*vp ++ = box[face][i];

		tp[0] = 0.0;    tp[1] = 0.0;    tp += 2;
                tp[0] = 1.0;    tp[1] = 0.0;    tp += 2;
                tp[0] = 1.0;    tp[1] = 1.0;    tp += 2;
                tp[0] = 0.0;    tp[1] = 1.0;    tp += 2;
		}

	gset = pfNewGSet (pfGetSharedArena());
	pfGSetPrimType (gset, PFGS_QUADS);
	pfGSetAttr (gset, PFGS_COORD3, PFGS_PER_VERTEX, (void *) vlist, NULL);

	if (num_textures > 0)
	    pfGSetMultiAttr (gset, PFGS_TEXCOORD2, 0, PFGS_PER_VERTEX, 
						(void *)tlist, NULL);
	else
	    pfGSetMultiAttr (gset, PFGS_TEXCOORD2, 0, PFGS_OFF, NULL, NULL);

	if (num_textures > 1)
	    pfGSetMultiAttr (gset, PFGS_TEXCOORD2, 1, PFGS_PER_VERTEX, 
						(void *)tlist, NULL);
	else
	    pfGSetMultiAttr (gset, PFGS_TEXCOORD2, 1, PFGS_OFF, NULL, NULL);


	pfGSetAttr (gset, PFGS_COLOR4, PFGS_PER_PRIM, (void *) clist, NULL);
	pfGSetAttr (gset, PFGS_NORMAL3, PFGS_OFF, NULL, NULL);

	pfGSetNumPrims (gset, 6);

	pfGSetGState (gset, gstate);

	geode = pfNewGeode();
	pfAddGSet (geode, gset);

	pfNodeName (geode, "Box Geode");

	pfMakeScaleMat (mat0, 0.5, 0.5, 0.5);

	pfPostTransMat (mat1, mat0, x_trans, y_trans, z_trans);

	scs = pfNewSCS(mat1);
	pfAddChild (scs, geode);

	return ((pfNode *) scs);
}

/*==========================================================================*/
pfMaterial *defaultMaterial (void)
/*==========================================================================*/
{
    pfMaterial   *material;

    material = pfNewMtl(pfGetSharedArena());
    pfMtlColor(material, PFMTL_AMBIENT,  0.8f, 0.8f, 0.8f);
    pfMtlColor(material, PFMTL_DIFFUSE,  0.8f, 0.8f, 0.8f);
    pfMtlColor(material, PFMTL_SPECULAR, 0.0f, 0.0f, 0.0f);
    pfMtlShininess(material, 0.0f);

    pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_OFF);

    /* return pointer to default material */
    return material;
}

