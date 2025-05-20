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
#include <sys/types.h>
#include <malloc.h>
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

void print_arena_info (void);
void draw_function(pfChannel* _chan, void* _userData);
void delete_textures (void);
void app_delete_model(void);

typedef struct
{
	int		done_delete_texture;
	pfNode		*model;
	pfList		*tex_list;
} 	delete_texture_command;

delete_texture_command	*command;

char			**model_filename;
int			nof_models;
int			model_index;

pfGroup			*group;

/*
 *	Usage() -- print usage advice and exit. 
 */
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, 
		"Usage: deleteGraph [-s|-m] file1.ext [file2.ext ...]\n");
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
    int		    frame_number;
    int		    isMP;

    if (argc < 2)
	Usage();

    /*
     *	=============================================
     *	Extract model names from command line 
     *	=============================================
     */
    nof_models = 0;
    model_index = 0;

    model_filename = (char **) malloc (argc * sizeof (char *));

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
	    model_filename[nof_models] = strdup (argv[i]);
	    fprintf(stderr, "ModelFile[%d]: <%s>\n", 
			nof_models, model_filename[nof_models]);
	    nof_models ++;
	}
    }

    if (nof_models == 0)
	Usage();
	
    /* 
     *	=============================================
     *	Initialize Performer 
     *	=============================================
     */
    pfInit();	

    /* 
     *	Run default/single-process/multi-process according to user 
     *	request.
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
     *	Load all loader DSO's before pfConfig() forks 
     */
    for (i = 0 ; i < nof_models ; i ++)
	pfdInitConverter(model_filename[i]);

    /*
     *	Create data structure for passing commands from APP to DRAW.
     */
    command = pfMalloc (sizeof (delete_texture_command), pfGetSharedArena());
    command -> done_delete_texture = 1;
    command -> model = NULL;
    command -> tex_list = NULL;

    /* 
     *	initiate multi-processing mode set in pfMultiprocess call 
     * 	FORKs for Performer processes,  CULL and DRAW, etc. happen here.
     */
    pfConfig();			

    /* 
     *	Append to Performer search path, PFPATH, files in 
     *	    /usr/share/Performer/data 
     */
    pfFilePath(".:/usr/share/Performer/data");

    /* Attach loaded file to a new pfScene. */
    scene = pfNewScene();
    group = pfNewGroup();

    pfAddChild(scene, group);

#if 0

    /*
     * 	Load all the model files as an initial setup.
     */
    for (i = 0 ; i < nof_models ; i ++)
    {
	/* 
	 *	Read a single file, of any known type. 
	 */
	if ((root = pfdLoadFile(model_filename[i])) == NULL) 
	{
	    pfExit();
	    exit(-1);
	}

	pfAddChild(group, root);
    }
#else
    /*
     *	Load the first file as an initial setup.
     */
    if ((root = pfdLoadFile(model_filename[0])) == NULL) 
    {
	pfExit();
	exit(-1);
    }

    pfAddChild(group, root);
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

    frame_number = 0;

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
        pfSetVec3(view.xyz, 2.0f * bsphere.radius * s,
                -2.0f * bsphere.radius *c,
                 0.5f * bsphere.radius);
        pfChanView(chan, view.xyz, view.hpr);

#if 0
	frame_number ++;
	if (frame_number > 90)
	{
	    pfExit();
	    exit(0);
	}
#endif
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
    struct mallinfo     mem_info;

    mem_info = amallinfo( pfGetSharedArena() );
#endif

    fprintf( stderr, "\n\n=================================================\n");

    used = pfGetMemoryArenaBytesUsed();

    if (used < 0)
	fprintf(stderr, 
	"### Warning: pfGetMemoryArenaBytesUsed works in the DEBUG"
	" library ONLY.\n");

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
		printf ("Delete texture GL handle.\n");
		pfDeleteGLHandle (tex); 
	    }
	}
    }

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
	// Get rid of model and texture list. The OpenGL textures in this
	// model are already gone (deleted in DRAW).
	// ================================================================ */

	if (command -> model)
	{
	    printf ("pfDelete model\n");

	    if (pfDelete (command -> model) != TRUE)
		fprintf (stderr, "\n### pfDelete model failed\n\n");

	    command -> model = NULL;
	}

	if (command -> tex_list)
	{
	    if (pfDelete( command -> tex_list ) != TRUE)
		fprintf (stderr, "\n### pfDelete tex_list failed\n\n");

	    command -> tex_list = NULL;
	}

	/* ===================================================================
	// Load new model onto the scene graph
	// ================================================================ */

	model_index = (model_index + 1) % nof_models;
	if ((model = pfdLoadFile(model_filename[model_index])) == NULL)
	{
	    pfExit();
	    exit(-1);
	}
	else
	{
	    printf ("Load new model: %s.\n", model_filename[model_index]);
	}

	pfAddChild(group, model);

	/* ===================================================================
	// Remove child # 0 from the scene graph and send it to DRAW for 
	// texture-deletion
	// ================================================================ */

	command -> model = pfGetChild (group, 0);
	pfRemoveChild (group, command -> model);
	
	command -> tex_list = pfuMakeTexList(command -> model);
	command -> done_delete_texture = 0;
    }
}

