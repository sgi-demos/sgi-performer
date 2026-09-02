/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
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

/*
 * texman.C : Performer program to demonstrate the use of pfuTextureManager.
 */

#include <stdlib.h>
#include <math.h>
#include <fcntl.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <Performer/pr.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfTexture.h>

#include <Performer/pfutil/pfuTextureManager.h>

#define MAX_SCENES    25

static struct _shared {
    int             done;
    int             winSizeX, 
                    winSizeY;
    pfDCS           *dcs;
    pfNode          *node[MAX_SCENES];
    int             nodeCnt;
	int				currentScene;
	int				goToNextScene;
    pfCoord         view;

	pfuTextureManager *texMan;
    TexList         *texList[MAX_SCENES];
	int				loadTextures;
	int				unloadTextures;
    int             reportLevel;
}*shared;


static  int             winX = 0,
                        winY = 0;
static unsigned int     winXSize = 512,
                        winYSize = 512;


static Display *dpy;
static Window win;


static void openPipeWin(pfPipeWindow *pw)
{
    pw->open();

    pw->getSize( &shared->winSizeX, &shared->winSizeY );

    dpy = pfGetCurWSConnection();
    win = pw->getWSWindow();
#ifndef WIN32
    XSelectInput( dpy, win,
                            ExposureMask | PointerMotionMask |
                            KeyPressMask | ButtonPressMask | ButtonReleaseMask);
#endif
}


static void openWindow( char *name )
{
    pfPipe *p;
    pfPipeWindow    *pw;

    p = pfGetPipe( 0 );
    pw = new pfPipeWindow( p );

    pw->setWinType( PFPWIN_TYPE_X );
    pw->setName( name );
    pw->setOriginSize( winX, winY, winXSize, winYSize );

    pw->setConfigFunc( openPipeWin );
    pw->config();
}

static void processInput( void )
{
#ifndef WIN32
    XEvent ev;

    while( XPending( dpy ) )
    {
        XNextEvent( dpy, &ev );
        switch( ev.type )
        {
            case KeyPress :
                switch( (XKeycodeToKeysym( dpy, ev.xkey.keycode, 0 )) & 0xFF )
                {
                    case 'k' :
                        shared->goToNextScene = TRUE;
                        break;

					case 'l' :
						shared->loadTextures = shared->currentScene;
						break;

                    case 'm' :
                        shared->reportLevel = (++shared->reportLevel % 4);
                        puts( "\033[2J" );
                        break;

					case 'u' :
						shared->unloadTextures = shared->currentScene;
						break;

                    case 0x1B : /* ESC */
                        shared->done = 1;
                        break;
                }
                break;

        }
    }
#endif
}

static void setNode( pfChannel *chan, int n )
{
    pfSphere    bsphere;

    if( n < 0 || n >= MAX_SCENES ) return;

    if( shared->currentScene != -1 )
    {
		shared->unloadTextures = shared->currentScene;
		shared->loadTextures   = n;

        shared->dcs->replaceChild( shared->node[shared->currentScene], shared->node[n] );
        shared->currentScene = n;
    }
    else
    {
        shared->currentScene = n;
        shared->dcs->addChild( shared->node[shared->currentScene] );
		shared->loadTextures = shared->currentScene;
    }



    shared->node[shared->currentScene]->getBound( &bsphere );
    shared->view.xyz.set(      bsphere.center[0],
                    bsphere.center[1] - (2.5 * bsphere.radius ),
                    bsphere.center[2] + (bsphere.radius * 0.5) ); 
    shared->view.hpr.set(  0.0, -20.0, 0.0 );
}

static void app(pfChannel *chan, void *data)
{
    if( shared->goToNextScene == TRUE )
    {
        int cn = (shared->currentScene + 1) % shared->nodeCnt;
        setNode( chan, cn );
        shared->goToNextScene = FALSE;
    }
}

static void draw(pfChannel *chan, void *data)
{
    processInput();
    chan->clear();

    pfDraw();

//
// Note that loading and unloading textures should happen in the draw process.
//
	if( shared->unloadTextures != -1 )
	{
		shared->texMan->unload( shared->texList[shared->unloadTextures] );
		shared->unloadTextures = -1;
	}

	if( shared->loadTextures != -1 )
	{
		// textures may be loaded all at once ... 
/*
		if( shared->texList[shared->loadTextures] != (TexList *)0L )
			shared->texMan->load( shared->texList[shared->loadTextures] );
*/

		// ... or during each frame with a deadline
		float done = 0.0f;
		if( shared->texList[shared->loadTextures] != (TexList *)0L )
		{
			done = shared->texMan->load( shared->texList[shared->currentScene], 
																(float)(pfGetTime() + 0.1f) );
			if( done == 1.0 )
				shared->loadTextures = -1;
		}
		else
			shared->loadTextures = -1;
	}	

    if( shared->reportLevel )
	{
		printf( "\033[0;0H PrintLevel %d\n\n", shared->reportLevel );
        shared->texMan->Print( shared->reportLevel );
	}
}


static pfChannel *createChannel( pfScene *scene )
{
    pfChannel *chan;

    chan = new pfChannel( pfGetPipe( 0 ));

    chan->setFOV( 45.0f, 0.0f );
    chan->setNearFar( 0.1f, 750.0f );

    chan->setTravFunc(  PFTRAV_DRAW, draw );
    chan->setTravFunc(  PFTRAV_APP , app );


    chan->setScene( scene );

    return chan;
}

static void loadFiles( int nfiles, char **files )
{
	static unsigned char *image = (unsigned char *)pfMalloc( 
					sizeof( unsigned char ) * 16 * 3, pfGetSharedArena() );
	int i;

	unsigned char *cptr = image;
	for( i = 0; i < 16; i++ )
	{
		*(cptr++) = 0x00;
		*(cptr++) = 0xFF;
		*(cptr++) = 0x00;
	}
	pfTexture *defaultTexture = new pfTexture;
	defaultTexture->setImage( (uint *)image, 3, 4, 4, 1 ); 

    for( i = 0; i < nfiles; i++ )
    {
        pfNode *root;
        if( i >= MAX_SCENES )
            break;

        if( (root = pfdLoadFile( files[i] )) )
        {
            shared->node[shared->nodeCnt++] = root;
            shared->texList[i] = shared->texMan->addTextureList( shared->node[i], files[i], defaultTexture );
        }
    }
}

static pfScene *setUpSceneGraph( void )
{
    pfScene        *scene;
    pfGeoState     *gstate;
    pfGroup        *grp;

    scene = new pfScene;
    scene->addChild( new pfLightSource );

    gstate = new pfGeoState;
    scene->setGState( gstate );

    shared->dcs = new pfDCS;
    grp         = new pfGroup;
    grp->addChild( shared->dcs );
    scene->addChild( grp );

    return scene;
}


main( int argc, char **argv )
{
    void         *arena;
    pfScene      *scene;
	pfChannel 	 *chan;
	int i;

    if( argc < 2 )
        pfNotify( PFNFY_FATAL, PFNFY_PRINT, "%s <file.ext> ... \n", argv[0] );

    pfInit();

    pfMultiprocess( PFMP_DEFAULT );

    arena = pfGetSharedArena();
    shared = (struct _shared *)pfMalloc( sizeof( struct _shared ), arena );
    shared->done = 0;
	shared->goToNextScene = TRUE;
    shared->currentScene  = -1;
    shared->unloadTextures  = -1;

	shared->nodeCnt = 0;

    pfuInit();

	for( i = 1; i < argc; i++ )
    	pfdInitConverter( argv[i] );

    pfConfig();

    shared->texMan = new pfuTextureManager;

    pfFilePath("./:/usr/share/Performer/data/:/usr/share/Performer/data/town/");

    openWindow( argv[0] );

    scene = setUpSceneGraph( );
    chan = createChannel( scene );


    loadFiles( argc - 1, &argv[1] );

	puts( "\033[2J" );

    while( !shared->done )
    {
        static float a = 0.0;

        shared->dcs->setRot( a, 0.0f, 0.0f );
        a += 1.0f;

    	chan->setView( shared->view.xyz, shared->view.hpr );
        pfFrame();
    }

    pfExit();
    exit(0);
}
