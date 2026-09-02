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
 * $Revision: 1.6 $ $Date: 2002/09/15 03:51:02 $ 
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

void print_arena_info (void);
void draw_function(pfChannel* _chan, void* _userData);
void delete_textures (void);
void app_delete_model(void);
pfNode *build_box(void);
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
    int		    isMP;

    isMP = (-1);
    for (i = 1 ; i < argc ; i ++)
    {
	if (strcmp (argv[i], "-m") == 0)
	    isMP = 1;
	else
	if (strcmp (argv[i], "-s") == 0)
	    isMP = 0;
	else
	{
	    fprintf(stderr, "Unknown option %s\n", argv[i]);
	    exit(1);
	}
    }
	
    /* 
     *	=============================================
     *	Initialize Performer 
     *	=============================================
     */
    pfInit();	

    /* 
     *	=============================================================
     *	Run default/single-process/multi-process according to user 
     *	request.
     *	=============================================================
     */

    if (isMP < 0)
    {
	pfMultiprocess( PFMP_DEFAULT );			
	pfNotify (PFNFY_WARN, PFNFY_PRINT, "Running PFMP_DEFAULT\n");
    }
    else
    if (isMP == 0)
    {
	pfMultiprocess( PFMP_APPCULLDRAW );			
	pfNotify (PFNFY_WARN, PFNFY_PRINT, "Running PFMP_APPCULLDRAW\n");
    }
    else
    {
	pfMultiprocess( PFMP_APP_CULL_DRAW );			
	pfNotify (PFNFY_WARN, PFNFY_PRINT, "Running PFMP_APP_CULL_DRAW\n");
    }

    /* 
     *	=============================================
     *	Create data structure for communicating 
     *	between APP and DRAW.
     *	=============================================
     */

    command = pfMalloc (sizeof (delete_texture_command), pfGetSharedArena());
    command -> done_delete_texture = 1;
    command -> model = NULL;
    command -> tex_list = NULL;

    /* 
     *	====================================================================
     *	initiate multi-processing mode set in pfMultiprocess call 
     * 	FORKs for Performer processes,  CULL and DRAW, etc. happen here.
     *	====================================================================
     */
    pfConfig();			

    /* Attach loaded file to a new pfScene. */
    scene = pfNewScene();

    group = pfNewGroup();
    pfAddChild(scene, group);

    for (i = 0 ; i < 4 ; i ++)
    {
	root = build_box();
	pfAddChild(group, root);
    }

#if 0
    pfdStoreFile (build_box() , "box.pfb");
#endif

    /* Create a pfLightSource and attach it to scene. */
    pfAddChild(scene, pfNewLSource());
    
    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinType(pw, PFPWIN_TYPE_X);
    pfPWinName(pw, "OpenGL Performer");
    pfPWinOriginSize(pw, 0, 0, 500, 500);
    /* Open and configure the GL window. */
    pfOpenPWin(pw);
    
    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanFOV(chan, 45.0f, 0.0f);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (root, &bsphere);
    pfChanNearFar(chan, 1.0f, 10.0f * bsphere.radius);

    pfChanTravFunc (chan, PFTRAV_DRAW, draw_function);

    while (1)
    {
        pfCoord    view;
        float      s, c;

	print_arena_info();

	/*
	 *  Prepare next model textures for DRAW.
	 *  Delete previous model.
	 *  Load new model and hang on scene graph.
	 */
	app_delete_model();

        /* Go to sleep until next frame time. */
        pfSync();

        /* Initiate cull/draw for this frame. */
        pfFrame();

        /* Compute new view position. */
        t = pfGetTime();
        pfSinCos(45.0f*t, &s, &c);
        pfSetVec3(view.hpr, 45.0f*t, -10.0f, 0);
        pfSetVec3(view.xyz, 2.0f * 2.0 * bsphere.radius * s,
                -2.0f * 2.0 * bsphere.radius *c,
                 0.5f * 2.0 * bsphere.radius);
        pfChanView(chan, view.xyz, view.hpr);
    }
}

/* ========================================================================= */
void draw_function(pfChannel* _chan, void* _userData)
/* ========================================================================= */
{
    pfClearChan (_chan);
    pfDraw();
    if (! command -> done_delete_texture)
	delete_textures ();
}


/* ========================================================================= */
void print_arena_info (void)
/* ========================================================================= */
{
    static int	max_used = 0;
    int		used;

#if 0
    struct mallinfo mem_info;

    mem_info = amallinfo( pfGetSharedArena() );
#endif

    fprintf( stderr, "\n\n=================================================\n");

    used = pfGetMemoryArenaBytesUsed();
    if (used > max_used)
	max_used = used;

    fprintf(stderr, "Arena Bytes Used: %d, Max: %d\n", used, max_used);

#if 0
    fprintf(stderr, "Arena space: %d  in use: small: %d  ordinary: %d\n",
            mem_info.arena, mem_info.usmblks, mem_info.uordblks );

    mem_info = mallinfo( );
    fprintf(stderr, "Regular space: %d  in use: small: %d  ordinary: %d\n",
            mem_info.arena, mem_info.usmblks, mem_info.uordblks );
#endif

    fprintf( stderr, "=================================================\n\n");

    fflush (stderr);
}

/* ========================================================================= */
void delete_textures (void)
/* ========================================================================= */
{
    int		i;
    pfList 	*tex_list;

    tex_list = command -> tex_list;

    /* ===============================================================
    // Delete GLHandles for all textures
    // ============================================================ */

    if ( ! command -> tex_list ) 
	return;

    /* ===============================================================
    // Don't do this if we don't have a valid graphics context.  The
    // following does really bad things in that case.
    // ============================================================ */
#ifndef WIN32
    if ( glXGetCurrentDrawable( ) && glXGetCurrentContext( ) ) 
#else
    if ( wglGetCurrentContext( ) )
#endif
    { 
	int 		num_texs;

 	num_texs = pfGetNum (tex_list);

	for (i = 0 ; i < num_texs ; i++) 
	{
	    pfTexture *tex = (pfTexture *) pfGet (tex_list, i);

	    if ( tex ) 
	    {
		fprintf (stderr, "Delete texture GL handle.\n");
		pfDeleteGLHandle (tex); 
	    }
	}
    }
    else
	fprintf (stderr, 
		"### No Graphic context. Skipping texture deletion.\n");

    fflush (stderr);
    command -> done_delete_texture = 1;
}


/* ========================================================================= */
void    app_delete_model(void)
/* ========================================================================= */
{
    pfNode	*model;

    if (command -> done_delete_texture)
    {

	/* ===================================================================
	// Finish up processing from previous frame.
	//
	// In the first round, command -> model and command -> tex_list are
	// both NULL so we skip this stage.
	//
	// Get rid of model and texture list. The OpenGL textures in this
	// model are already gone (deleted in DRAW).
	// ================================================================= */

	if (command -> model)
	{
	    printf ("pfDelete model\n");

	    if (pfDelete (command -> model) != TRUE)
		fprintf (stderr, "\n### pfDelete model failed\n\n");

	    command -> model = NULL;
	}

	if (command -> tex_list)
	{
	    printf ("pfDelete tex_list\n");

	    if (pfDelete( command -> tex_list ) != TRUE)
		fprintf (stderr, "\n### pfDelete tex_list failed\n\n");

	    command -> tex_list = NULL;
	}

	/* ===================================================================
	// Build new model into shared arena.
	// ================================================================ */

	model = build_box();
        pfAddChild(group, model);

        /* ===================================================================
        // Remove child # 0 from the scene graph and send it to DRAW for
        // texture-deletion
	// ================================================================ */

        command -> model = pfGetChild (group, 0);

	if (model == command -> model)
	    fprintf (stderr, "Removing incorrect model\n");

        pfRemoveChild (group, command -> model);


	/* ===================================================================
	// Setup parameters for DRAW process texture deletion.
	// ================================================================ */

	command -> tex_list = pfuMakeTexList(command -> model);

	fflush (stderr);
	command -> done_delete_texture = 0;
    }
}


static float	box[6][4*3] =
	{
		{
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0, 1.0, -1.0,
		-1.0, 1.0, -1.0
		},
		{
		-1.0, -1.0, 1.0,
		1.0, -1.0, 1.0,
		1.0, 1.0, 1.0,
		-1.0, 1.0, 1.0
		},
		{
		-1.0, -1.0, -1.0,
		-1.0, 1.0, -1.0,
		-1.0, 1.0, 1.0,
		-1.0, -1.0, 1.0
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
		1.0, 1.0, -1.0,
		1.0, 1.0, 1.0,
		-1.0, 1.0, 1.0
		},
	};

static float	box_colors[6][4] =
	{
	0.0, 0.0, 1.0, 1.0,
	0.0, 1.0, 0.0, 1.0,
	0.0, 1.0, 1.0, 1.0,
	1.0, 0.0, 0.0, 1.0,
	1.0, 0.0, 1.0, 1.0,
	1.0, 1.0, 1.0, 1.0
	};

static  char    *texture_name[] =
        {
        "truckWheel.rgb",
        "truckSide.rgb",
        "truckBack.rgb",
        "tux.rgb",
        "tux2.rgb",
        "wood.rgb",
        "road.rgb",
        "planks.rgb",
        "iris.rgb",
        "hubcap.rgb",
        "marble.rgb",
        "iris.rgb",
        "flowers.rgb",
        "desert.rgb",
        "coke.rgb",
        "cafe.rgb",
        "brick.rgb",
        "bark.rgb"
        };




/*==========================================================================*/
pfNode *build_box(void)
/*==========================================================================*/
{
int		i, face;
float		*vlist, *tlist, *clist;
float		*vp, *tp, *cp;
pfGeoSet	*gset;
pfGeoState	*gstate;
pfGeode		*geode;
pfTexture	*tex;
pfTexEnv	*tev;
pfSCS		*scs;
pfMatrix	mat0, mat1;
float		r, rx, ry, rz;
static		tex_index = 0;

	gstate = pfNewGState (pfGetSharedArena());
	pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
	pfGStateMode (gstate, PFSTATE_ENLIGHTING, PF_OFF);

	pfGStateAttr (gstate, PFSTATE_FRONTMTL, defaultMaterial());
	pfGStateAttr (gstate, PFSTATE_BACKMTL, defaultMaterial());

	tex = pfNewTex (pfGetSharedArena());

	tex_index = (tex_index + 1) % 17;
	pfLoadTexFile (tex, texture_name[tex_index]);

	tev = pfNewTEnv (pfGetSharedArena());

	pfGStateMode (gstate, PFSTATE_ENTEXTURE, 1);
	pfGStateAttr (gstate, PFSTATE_TEXTURE, tex);
	pfGStateAttr (gstate, PFSTATE_TEXENV, tev);


	vlist = pfMalloc (24 * 3 * sizeof (float), pfGetSharedArena());
	tlist = pfMalloc (24 * 2 * sizeof (float), pfGetSharedArena());
	clist = pfMalloc (6 * 4 * sizeof (float), pfGetSharedArena());

	vp = vlist;
	tp = tlist;
	cp = clist;

	for (face = 0; face < 6; face ++)
		{
		r = 0.5 + ((float) (random() % 1000)) / 2000.0;

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

	pfGSetAttr (gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, (void *)tlist, NULL);
	pfGSetAttr (gset, PFGS_COLOR4, PFGS_PER_PRIM, (void *) clist, NULL);
	pfGSetAttr (gset, PFGS_NORMAL3, PFGS_OFF, NULL, NULL);

	pfGSetNumPrims (gset, 6);

	pfGSetGState (gset, gstate);

	geode = pfNewGeode();
	pfAddGSet (geode, gset);

	pfNodeName (geode, "Box Geode");

	pfMakeScaleMat (mat0, 5.1, 5.1, 5.1);

	rx = 5.0 * (((float) (random() % 1000)) / 500.0 - 1.0);
	ry = 5.0 * (((float) (random() % 1000)) / 500.0 - 1.0);
	rz = 5.0 * (((float) (random() % 1000)) / 500.0 - 1.0);
	pfPostTransMat (mat1, mat0, rx, ry, rz);

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

