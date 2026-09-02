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
 * texmem.C : Simple Performer program to demonstrate the use of pfuTextureMemory 
 * utilities pfuShowTextureMemory() and pfuClearTextureMemory().
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


static struct _shared {
	int 		done;
	pfList		*texList;
	int 		snapTexture;
	int 		clearTexture;
}*shared;

static  int             winX = 0,
                        winY = 0;
static unsigned int     winXSize = 1280,
                        winYSize = 1024;

static Display *dpy;
static Window 	win;
static pfDCS *dcs;


static void openPipeWin(pfPipeWindow *pw)
{
	pw->open();

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
					case 'c'  :
						shared->clearTexture = 1;
						break;

					case 'k'  : 
						shared->snapTexture = 1;
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


static void draw(pfChannel *chan, void *data)
{
	static int flag = 0;

int formats[] = {
           PFTEX_RGB_5   ,
           PFTEX_RGB_4   ,
           PFTEX_RGB5_A1 ,
           PFTEX_RGBA_4  ,
           PFTEX_IA_8    ,
           PFTEX_I_12A_4 ,
           PFTEX_I_8     ,
           PFTEX_I_16    ,
           PFTEX_IA_12   ,
           PFTEX_RGBA_8  ,
           PFTEX_RGB_12  ,
           PFTEX_RGBA_12 ,
};

	static int which = 0;

#ifndef WIN32
	if( flag )
 	{
		XEvent ev; 
		do  XNextEvent( dpy, &ev ); while( ev.type != KeyPress );  
		flag = 0;
	}
#endif

    processInput();
    chan->clear();
    pfDraw();

	if( shared->snapTexture )
	{
		which = (which + 1) % 12;
		pfuShowTextureMemory( chan->getPWin(), shared->texList, formats[which] );
		shared->snapTexture = 0;
		flag = 1;
	}

	if( shared->clearTexture )
	{
		pfuClearTextureMemory();
		shared->clearTexture = 0;
	}
}



static pfChannel *createChannel( void )
{
	pfChannel *chan;

    chan = new pfChannel( pfGetPipe( 0 ));

    chan->setFOV( 45.0f, 0.0f );
    chan->setNearFar( 0.1f, 1e6 );

    chan->setTravFunc(  PFTRAV_DRAW, draw );
	return chan;
}



static pfScene *setUpSceneGraph( int nfiles, char **files )
{
	pfScene *scene;
	pfGeoState *gstate;
	int i;

	scene = new pfScene;
	scene->addChild( new pfLightSource );

    gstate = new pfGeoState;
    scene->setGState( gstate );

	dcs = new pfDCS;
	scene->addChild( dcs );

	pfGroup *grp = new pfGroup;
	dcs->addChild( grp );

	for( i = 0; i < nfiles; i++ )
		grp->addChild( pfdLoadFile( files[i] ));

	return scene;
}

static void setView( pfChannel *chan, pfNode *node )
{
	pfSphere 	bs;
	pfCoord		view;

	node->getBound( &bs );

    view.xyz.set(  	bs.center[0], 
					bs.center[1] - bs.radius * 2, 
					bs.center[2] + bs.radius/2 );
    view.hpr.set(  0.0, -20.0, 0.0 );

    chan->setView( view.xyz, view.hpr );
}

main( int argc, char **argv )
{
	void 	*arena;
	pfChannel *chan;
	pfScene *scene;

	if( argc < 2 )
		pfNotify( PFNFY_FATAL, PFNFY_USAGE, "Usage : %s <file.ext> ...", argv[0] );


	pfInit();

	pfMultiprocess( PFMP_DEFAULT );

	arena = pfGetSharedArena();
	shared = (struct _shared *)pfMalloc( sizeof( struct _shared ), arena );
	shared->done = 0;
	shared->snapTexture = 0;
	shared->clearTexture = 0;

    pfuInit();

	for( int i = 1; i < argc; i++ )
		pfdInitConverter( argv[i] );

	pfConfig();

	pfFilePath( ".:/usr/share/Performer/data/" );

	openWindow( argv[0] );

	chan  = createChannel();
	scene = setUpSceneGraph( argc - 1, &argv[1] );
	chan->setScene( scene );
	setView( chan, (pfNode *)scene );

	shared->texList = pfuMakeSceneTexList( scene );

	while( !shared->done )
	{
		static float a = 0.0;
		dcs->setRot( a, 0.0, 0.0 );
		a ++;
		pfFrame();
	}

	pfExit();
	exit(0);
}

