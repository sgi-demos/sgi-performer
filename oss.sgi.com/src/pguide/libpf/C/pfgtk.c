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
 */
/*
 * pfgtk.c
 * 
 * OpenGL Performer example using Gtk+.
 *
 * Scott A. Friedman
 * Urban Simulation Team
 * Univeristy of California at Los Angeles - UCLA
 *
 * Based on SGI OpenGL Performer example program motif.c
 * 
 * Run-time controls:
 *       ESC-key: exits
 *        F1-key: profile
 *    Left-mouse: advance
 *  Middle-mouse: stop
 *   Right-mouse: retreat
 * pfgtk is a straight port of Performer's motif.c to use the Gtk user
 * interface libraries. If you are interested in developing on both IRIX
 * and Linux using Performer (like me) this may be of interest to you.
 * Basically, you are only going to want DoGtk for the magic to hook
 * Performer to the window. For you linux only people this shows you how
 * to start wrapping your cool Performer apps with Gtk/Gnome so they look
 * like all the other programs out there.
 *
 * One GUI source for both IRIX and Linux!  Okay, more or less ;-)
 *
 * This has been compiled and run with the latest libraries for both IRIX
 * and Linux as of 4/7/2001. That means IRIX 6.5.11 and Redhat 7.0 with
 * no fancy add-ons or patches. Using the SGI compiler gives some weird
 * warnings about mixing enumerated types - ignore it, probably a bug.
 * Using gcc under IRIX complains about finding more than one libm.so -
 * again, you can ignore but it's easy to fix.
 *
 * To Compile:
 *
 * gcc pfgtk.c -o pfgtk -lpf -lpfdu -lpfutil `gtk-config --cflags --libs` -lgtkgl
 *
 * To Test:
 * pfgtk esprit.pfb
 *
 * The End
 *
 * Scott Friedman
 * Urban Simulation Team at UCLA
 * friedman@ucla.edu
 *  
 */

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <getopt.h>		    /* for cmdline handler */
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gtkgl/gtkglarea.h>
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#define HISPEED		0.1f
#define LOSPEED		0.001f
#define	FARCLIP 	10000.0f


/*
 *  Structure that resides in shared memory so that the
 *  application, cull, and draw processes can access it.
 */
typedef struct
{
    pfPipeWindow    *pw;
    pfChannel	    *chan;
    pfScene	    *scene;
    pfNode	    *model;
    int		    exitFlag;
    int    	    inWindow; 
    int		    reset;
    float	    mouseX;
    float	    mouseY;
    int		    winSizeX, winSizeY;
    int		    mouseButtons;
    pfCoord	    view;
    pfCoord	    viewOrig;
    float	    sceneSize;
    int		    drawStats;
    int		    loadNewModel;
    char	    loadFileName[PF_MAXSTRING];
    char	    loadFilePath[PF_MAXSTRING];
} SharedData;

typedef struct GtkWinInfo_t 
{
    Window  xWin;
    Window  gfXWin;
    int	   *config;
    guint   work_id;
} GtkWinInfo_t;

/* 
 *  Inter-process communication structures 
 */
static SharedData *Shared;
GtkWinInfo_t *GtkWinInfo;

/*
 *  For configuring multi-process 
 */
static int ProcSplit = PFMP_APPCULLDRAW;
static int WriteScene = 0; /* use pfDebugPrint to write out scene upon read */
static int ForkGtk = 0; /* put Motif in a forked process */
static int WinOrgX=0, WinOrgY=0, WinSizeX=300, WinSizeY=300;
char ProgName[PF_MAXSTRING];
   
static void CullChannel( pfChannel *, void * );
static void DrawChannel( pfChannel *, void * );
static void OpenPipeWin( pfPipeWindow *pw );
static void InitPipe( void );
static gint SimFrame( gpointer );
static void UpdateView( void );
static void LoadModel( char * );
static void Usage( void );

/* 
 *  Gtk process routines 
 */
static void DoGtk( int, char **, int );
static int WorkProc( void );
static gint InitGtkGlWidget( GtkWidget *, GdkEventConfigure *, gpointer );
static gint KeyPressEvent( GtkWidget *, GdkEventKey * );
static gint KeyReleaseEvent( GtkWidget *, GdkEventKey * );
static gint ButtonPressEvent( GtkWidget *, GdkEventButton * );
static gint ButtonReleaseEvent( GtkWidget *, GdkEventButton * );
static gint PointerMotionEvent( GtkWidget *, GdkEventMotion * );
static gint FocusChangeEvent( GtkWidget *, GdkEventFocus * );
static gint EnterCrossingEvent( GtkWidget *, GdkEventCrossing * );
static gint LeaveCrossingEvent( GtkWidget *, GdkEventCrossing * );
static gint GetTopInput( GtkWidget *, GdkEvent *, gpointer );
static void OpenMenuHandler( GtkMenuItem *, gpointer );
static void QuitMenuHandler( GtkMenuItem *, gpointer );
static void FileDialogOK( GtkButton *, gpointer );
static void FileDialogCancel( GtkButton *, gpointer );

/*
 *  Usage() -- print usage advice and exit. This procedure
 *  is executed in the application process.
 */
static void Usage( void )
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE,
	"Usage: %s [-f][-p procSplit][-w][-W ox,oy,sx,sy] [file.ext]\n", ProgName);
    exit( 0 );
}

/*
 *  docmdline() -- use getopt to get command-line arguments, 
 *  executed at the start of the application process.
 */
static int docmdline( int argc, char *argv[] )
{
    int	    opt;

    strcpy( ProgName, argv[0] );
    
    /*
     *	Process command-line arguments 
     */
    while ( ( opt = getopt( argc, argv, "Fm:p:wW:?" ) ) != -1 )
    {
	switch ( opt )
	{
	    case 'F':
		ForkGtk ^= 1;
		break;
    
	    case 'm':
	    case 'p':
		ProcSplit = atoi( optarg );
		break;
    
	    case 'w': 
		WriteScene = 1;
		break;
    
	    /*
	     *  Set the window size 
	     */
	    case 'W':
		if ( sscanf( optarg, "%d,%d,%d,%d", 
			&WinOrgX, &WinOrgY, &WinSizeX, &WinSizeY ) != 4 )
		{
		    if ( sscanf( optarg, "%d,%d", &WinSizeX, &WinSizeY ) != 2 )
		    {
			if ( sscanf( optarg, "%d", &WinSizeX ) == 1 )
			{
			    WinSizeY = WinSizeX;
			}
		    }
		}
		break;
	    
	    case '?': 
		Usage( );
	}
    }
    return optind;
}

/*
 *  main() -- program entry point. this procedure
 *  is executed in the application process.
 */

int main ( int argc, char **argv )
{
/*    int		    i;*/
    int		    arg;
/*    pfNode	   *root;*/
    pfPipe	   *p;
    pfEarthSky	   *eSky;
    pfLightSource  *sun;
    pfVec3	    where;
    pid_t	    fpid = 0;
    
    /*
     *	Process command line arguments 
     */
    arg = docmdline( argc, argv );
    
    /*
     *	Performer Initialization 
     */
    pfInit( );
    
    /*
     *	Allocate Shared before fork()'ing Gtk so that all 
     *	processes see the Shared pointer.
     */
    Shared = (SharedData *)pfCalloc( 1, sizeof( SharedData ), pfGetSharedArena( ) );

    Shared->inWindow = 0;
    Shared->reset = 1;
    Shared->exitFlag = 0;
    Shared->drawStats = 0;
    Shared->loadNewModel = 0;
    pfSetVec3( Shared->view.xyz, 0.0f, -100.0f, 50.0f );
    pfSetVec3( Shared->view.hpr, 0.0f, 0.0f, 0.0f );
    
   /*
    *	Set global application configuration parameters
    */
    
    /*
     *	Configure multi-process selection 
     */
    pfMultiprocess( ProcSplit );

    /*
     *	Initialize the file-loader DSO before forking in pfConfig() 
     */
    if ( arg < argc )
    {
	pfdInitConverter( argv[arg] );
    }
    else
    {
	/*
	 *  Make a guess
	 */
	pfdInitConverter( "*.flt" );
    }
    
    /*
     *	Initiate multi-processing 
     */
    pfConfig( );	
    
    /* 
     *	Set off Gtk AFTER pfConfig so that we will have query 
     *	access to Performer objects in shared memory
     */
    GtkWinInfo = (GtkWinInfo_t *)pfCalloc( 1, sizeof( GtkWinInfo_t ), pfGetSharedArena( ) );
    if ( ForkGtk )
    {
	if ( ( fpid = fork( ) ) < 0 )
	{
	    pfNotify( PFNFY_FATAL, PFNFY_SYSERR, 
		"Fork of Gtk process failed." );
	}
	else if ( fpid ) 
	{
	    pfNotify( PFNFY_NOTICE, PFNFY_PRINT,
		"Motif running in forked process %d", fpid );
	}
	if ( !fpid )
	{
	    DoGtk( argc, argv, ForkGtk );
	    exit( 0 );
	}
    }
    	   
    pfFrameRate( 60.0f );
    pfPhase( PFPHASE_FREE_RUN );
   
    /*
     * Load the initial database
     */
    
    /*
     *	Specify directories where geometry and textures exist 
     */
    if ( !( getenv( "PFPATH" ) ) )
    {
        pfFilePath(
            "."
            ":./data"
            ":../data"
            ":../../data"
            ":/usr/share/Performer/data"
            );
    }
    pfNotify( PFNFY_INFO, PFNFY_PRINT, "FilePath: %s", pfGetFilePath( ) );
    
    /*
     *	Initialize the scene graph 
     */
    Shared->scene = pfNewScene( );

    /*
     *	Add infinite light source to scene 
     */
    sun = pfNewLSource( );
    pfSetVec3( where, -1.0f, -1.0f, 2.0f );
    pfNormalizeVec3( where );
    pfLSourcePos( sun, where[0], where[1], where[2], 0.0f );
    pfLSourceColor( sun, PFLT_DIFFUSE, 1.0f, 1.0f, 1.0f );
    pfAddChild(Shared->scene, sun);

    /* 
     *	Add place holder "model" to scene 
     */
    Shared->model = (pfNode *)pfNewGroup( );
    pfAddChild( Shared->scene, Shared->model );

    /* 
     *	Load file named on command line 
     */
    if ( arg < argc )
    {
	LoadModel( argv[arg] );
    }

    /*
     *	Write out nodes in scene (for debugging) 
     */
    if ( WriteScene )
    {
	FILE *fp;
	if (fp = fopen( "scene.out", "w" ) )
	{
	    pfPrint( Shared->scene, PFTRAV_SELF|PFTRAV_DESCEND, PFPRINT_VB_DEBUG, fp );
	    fclose( fp );
	}
	else
	{
	    pfNotify( PFNFY_WARN, PFNFY_RESOURCE,
		"Could not open scene.out for debug printing." );
	}
    }
 
   /*
    *	Create Pipes, Windows, and Channels
    */
    
    /*
     *	Create Performer Window 
     */
    p = pfGetPipe( 0 );
    Shared->pw = pfNewPWin( p );
    pfPWinType( Shared->pw, PFPWIN_TYPE_X );
    pfPWinName( Shared->pw, "IRIS Performer ");

    /*
     *	Set the window initialization callback 
     */
    pfPWinConfigFunc( Shared->pw, OpenPipeWin );

    /*
     *	Set the request for the configuration of the window 
     */
    pfConfigPWin( Shared->pw );
    
    Shared->chan = pfNewChan( p );
    pfAddChan( Shared->pw, Shared->chan );
    pfChanTravMode( Shared->chan, PFTRAV_CULL, PFCULL_ALL );
    pfChanTravFunc( Shared->chan, PFTRAV_CULL, CullChannel );
    pfChanTravFunc( Shared->chan, PFTRAV_DRAW, DrawChannel );
    pfChanScene( Shared->chan, Shared->scene );
    pfChanNearFar( Shared->chan, 0.1f, FARCLIP );
    
    /*
     *	Create an earth/sky model that draws sky/ground/horizon 
     */
    eSky = pfNewESky( );
    pfESkyMode( eSky, PFES_BUFFER_CLEAR, PFES_SKY_GRND );
    pfESkyAttr( eSky, PFES_GRND_HT, -100.0f );
    pfChanESky( Shared->chan, eSky );

    /*
     *	Horizontal FOV is matched to window aspect ratio 
     */
    pfChanFOV( Shared->chan, 0.0f, 45.0f );
    
    /*
     *	Initialize viewpoint 
     */
    pfChanView( Shared->chan, Shared->view.xyz, Shared->view.hpr );
    PFCOPY_VEC3( Shared->viewOrig.xyz, Shared->view.xyz );
    PFCOPY_VEC3( Shared->viewOrig.hpr, Shared->view.hpr );
    
    /*
     *	Main simulation loop
     */
    
    /*
     *	If Gtk is forked, we control the app main loop 
     */
    if ( ForkGtk )
    {
	/*
	 *  Wait for Gtk process to initialize the pfPipeWindow parameters 
	 */
	while ( !GtkWinInfo->xWin )
	{
	    sginap( 1 );
	}

	InitPipe( );
	
	/*
	 *  main simulation loop 
	 */
	while ( !Shared->exitFlag )
	{
	    SimFrame( NULL );
	}
    }
    else
    {
	DoGtk( argc, argv, ForkGtk );
    }

    /*
     *	Terminate cull and draw processes (if they exist) 
     */
    pfExit( );
    
    /*
     *	Exit to operating system 
     */
    exit( 0 );
}

/*
 *  LoadModel() -- Load in new databases -
 *  is executed in the application process.
 */
static void LoadModel( char *filename )
{
    int		    found=0;
    pfBox           bbox;
    
    pfNotify( PFNFY_NOTICE, PFNFY_PRINT,
	"LoadModel - loading file '%s'", filename );

    if ( pfFindFile( filename, Shared->loadFilePath, R_OK ) )
    {
	pfNode	*node;
	char	*c;

	/* load named file */
	if ((node = pfdLoadFile(Shared->loadFilePath)) != NULL)
	{
	    /* install new model into scene */
	    pfReplaceChild(Shared->scene, Shared->model, node);
	    pfDelete(Shared->model);
	    Shared->model = node;
	    found = 1;

	    /* remove unsed objects from pfdBuilder share cache */
	    pfdCleanBldrShare();

	    /* optimize model (optional) */
#define OPTIMIZE_MODEL
#ifdef  OPTIMIZE_MODEL
	    /* optimize sharing of geostate structures within scene */
	    pfdMakeShared(Shared->model);

	    /* factor common geostate elements to scene geostate */
	    pfdMakeSharedScene(Shared->scene);

	    /* optimize pfLayer nodes via "DISPLACE POLYGON" */
	    pfdCombineLayers(Shared->model);

	    /* optimize pfBillboard nodes via */
	    pfdCombineBillboards(Shared->model, 32);
#endif

	    /* update file path */
	    strcpy(Shared->loadFileName, filename);
	    c = strrchr(Shared->loadFilePath, '/');
	    if (c)
		*c = '\0';
	}
    }

    /* set appropriate viewing position and near/far clipplanes */
    if (found) 
    {
	float sceneSize;
	
	/* Set initial view to be "in front" of model */
	/* determine extent of scene's geometry */
	pfuTravCalcBBox(Shared->model, &bbox);
    
	/* place view point at center of bbox */
	pfAddVec3(Shared->view.xyz, bbox.min, bbox.max);
	pfScaleVec3(Shared->view.xyz, 0.5f, Shared->view.xyz);
	
	/* find max dimension */
	sceneSize =                    bbox.max[PF_X] - bbox.min[PF_X];
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Y] - bbox.min[PF_Y]);
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Z] - bbox.min[PF_Z]);
	sceneSize = PF_MIN2(sceneSize, 0.5f * FARCLIP);
	Shared->sceneSize = sceneSize;
	
	/* offset so all is visible */
	Shared->view.xyz[PF_Y] -= 1.50f*sceneSize;
	Shared->view.xyz[PF_Z] += 0.25f*sceneSize;	
    }
#ifdef  RESET_VIEWPOINT_ON_FAILED_LOAD
    else
    {

	pfSetVec3(Shared->view.xyz, 0.0f, -100.0f, 50.0f);
	PFSET_VEC3(bbox.min, -5000.0, -5000.0, -5000.0);
	PFSET_VEC3(bbox.max,  5000.0,  5000.0,  5000.0);
	Shared->sceneSize = 10000.0f;
    }
#endif
}

/******************************************************************************
 *			    Gtk Routines
 ******************************************************************************
 */

void DoGtk(int argc, char **argv, int forked)
{
    GtkWidget *mainWindow;
    GtkWidget *glArea;
    Display *dsp;
    GtkWidget *table;
    GtkWidget *menuBar;
    GtkWidget *item;
    GtkWidget *fileMenuItem;
    GtkWidget *fileMenu;

#ifdef __linux__
    prctl(PR_SET_PDEATHSIG, SIGHUP, 0, 0, 0);
#else
    prctl( PR_TERMCHILD );        /* Exit when parent does */
    sigset( SIGHUP, SIG_DFL );    /* Exit when sent SIGHUP by TERMCHILD */
#endif

    /*
     *	initialize gtk 
     */
    gtk_init( &argc, &argv );

    /*
     *	Check if OpenGL is supported. 
     */
    if ( gdk_gl_query( ) == FALSE ) 
    {
	g_print( "OpenGL not supported\n" );
	return;
    }
    /*
     *	Create new top level window. 
     */
    mainWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW( mainWindow ), "pfGtk" );

    /*
     *	Quit from main if got delete event 
     */
    gtk_signal_connect( GTK_OBJECT( mainWindow ), "delete_event",
			    GTK_SIGNAL_FUNC( gtk_main_quit ), NULL );
    /*
     *	Create the boxes that will hold our interface
     */
    table = gtk_table_new( 2, 1, FALSE );
    /*
     *	Create the menu bar
     */
    menuBar = gtk_menu_bar_new( );
    /*
     *	Create file menu
     */
    fileMenuItem = gtk_menu_item_new_with_label( "File" );
    gtk_menu_bar_append( GTK_MENU_BAR( menuBar ), fileMenuItem );
    /*
     *	Create the file menu itself
     */
    fileMenu = gtk_menu_new( );
    /*
     *	Create the menu items and add them to the menu
     */
    item = gtk_menu_item_new_with_label( "Open" );
    gtk_menu_append( GTK_MENU( fileMenu ), item );
    gtk_signal_connect( GTK_OBJECT( item ), "activate", 
    		    GTK_SIGNAL_FUNC( OpenMenuHandler ), NULL );
    item = gtk_menu_item_new_with_label( "Quit" );
    gtk_menu_append( GTK_MENU( fileMenu ), item );
    gtk_signal_connect( GTK_OBJECT( item ), "activate", 
    		    GTK_SIGNAL_FUNC( QuitMenuHandler ), NULL );
    /*
     *	Attach the menu to the menu bar
     */
    gtk_menu_item_set_submenu( GTK_MENU_ITEM( fileMenuItem ), fileMenu );
    /*
     *	You should always delete gtk_gl_area widgets before exit or else
     *	GLX contexts are left undeleted, this may cause problems
     *	(=core dump) in some systems.
     *	Destroy method of objects is not automatically called on exit.
     *	You need to manually enable this feature. Do gtk_quit_add_destroy()
     *	for all your top level windows unless you are certain that they get
     *	destroy signal by other means.
     */
    gtk_quit_add_destroy( 1, GTK_OBJECT( mainWindow ) );
    /*
     *	Create rendering widget 
     *	visual chosen by Performer
     */
    dsp = GDK_DISPLAY( );
    pfChooseFBConfigData( (void**)&(GtkWinInfo->config), 
			    dsp, -1, NULL, pfGetSharedArena( ) );
    glArea = GTK_WIDGET( gtk_gl_area_new( GtkWinInfo->config ) );
    /*
     *	Events for widget must be set before X Window is created 
     */
    gtk_widget_set_events( GTK_WIDGET( mainWindow ), GDK_STRUCTURE_MASK );
    gtk_widget_set_events( GTK_WIDGET( glArea ), GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
						 GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
						 GDK_POINTER_MOTION_HINT_MASK | GDK_FOCUS_CHANGE |
						 GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK );
    /*
     *	Catch window mapping. 
     */
    gtk_signal_connect( GTK_OBJECT( mainWindow ), "map_event",
		     GTK_SIGNAL_FUNC( GetTopInput ), NULL );
    gtk_signal_connect( GTK_OBJECT( mainWindow ), "unmap_event",
		     GTK_SIGNAL_FUNC( GetTopInput ), NULL );
    /*
     *	Collect all those events. 
     */
    gtk_signal_connect( GTK_OBJECT( glArea ), "configure_event",
		     GTK_SIGNAL_FUNC( InitGtkGlWidget ), NULL);
    gtk_signal_connect( GTK_OBJECT( glArea ), "key_press_event",
		     GTK_SIGNAL_FUNC( KeyPressEvent ), NULL );
    gtk_signal_connect( GTK_OBJECT( glArea ), "key_release_event",
		     GTK_SIGNAL_FUNC( KeyReleaseEvent ), NULL );
    gtk_signal_connect( GTK_OBJECT( glArea ), "button_press_event",
		     GTK_SIGNAL_FUNC( ButtonPressEvent ), NULL );
    gtk_signal_connect( GTK_OBJECT( glArea ), "button_release_event",
		     GTK_SIGNAL_FUNC( ButtonReleaseEvent ), NULL );
    gtk_signal_connect( GTK_OBJECT( glArea ), "motion_notify_event",
		     GTK_SIGNAL_FUNC( PointerMotionEvent ), NULL );
    gtk_signal_connect( GTK_OBJECT( glArea ), "focus_in_event",
		     GTK_SIGNAL_FUNC( FocusChangeEvent ), NULL );
    gtk_signal_connect( GTK_OBJECT( glArea ), "focus_out_event",
		     GTK_SIGNAL_FUNC( FocusChangeEvent ), NULL );
    gtk_signal_connect( GTK_OBJECT( glArea ), "enter_notify_event",
		     GTK_SIGNAL_FUNC( EnterCrossingEvent ), NULL );
    gtk_signal_connect( GTK_OBJECT( glArea ), "leave_notify_event",
		     GTK_SIGNAL_FUNC( LeaveCrossingEvent ), NULL );
    /*
     *	set minimum size 
     */
    gtk_widget_set_usize( GTK_WIDGET( glArea ), WinSizeX, WinSizeY );
    /* 
     *	put glarea into window and show it all 
     */
    gtk_table_attach( GTK_TABLE( table ), menuBar, 
			0, 1, 0, 1, 
			GTK_FILL | GTK_EXPAND | GTK_SHRINK, 
			GTK_FILL, 
			0, 0 );
    gtk_table_attach( GTK_TABLE( table ), glArea, 
			0, 1, 1, 2, 
			GTK_EXPAND | GTK_SHRINK, 
			GTK_FILL | GTK_EXPAND | GTK_SHRINK, 
			0, 0 );

    gtk_container_add( GTK_CONTAINER( mainWindow ), GTK_WIDGET( table ) );

    gtk_widget_show_all( GTK_WIDGET( mainWindow ) );

    GTK_WIDGET_SET_FLAGS( glArea, GTK_CAN_FOCUS );
    gtk_widget_grab_focus( GTK_WIDGET( glArea ) );

    /*
     *	set PipeWindow X parameters from the widget 
     */
    GtkWinInfo->gfXWin = GDK_WINDOW_XWINDOW( glArea->window );

    if ( !GtkWinInfo->gfXWin )
	pfNotify( PFNFY_WARN, PFNFY_PRINT, "Gtk - No GFX Window!!" );
    /*
     *	this will release the waiting application process 
     */
    GtkWinInfo->xWin = GDK_WINDOW_XWINDOW( glArea->window ); 
    if ( !forked )
    {
	GtkWinInfo->work_id = gtk_idle_add( SimFrame, NULL );
	InitPipe( );
    }
    /*
     *	Here we go...
     */
    gtk_main( );
}


static void OpenMenuHandler( GtkMenuItem *menuItem, gpointer p )
{
    GtkWidget *dialog = gtk_file_selection_new( "Select model to open" );
    
    gtk_signal_connect( GTK_OBJECT( GTK_FILE_SELECTION( dialog )->ok_button ), 
			"clicked", 
			GTK_SIGNAL_FUNC( FileDialogOK ), 
			dialog );
    gtk_signal_connect( GTK_OBJECT( GTK_FILE_SELECTION( dialog )->cancel_button ), 
			"clicked", 
			GTK_SIGNAL_FUNC( FileDialogCancel ), 
			dialog );
			
    gtk_file_selection_complete( GTK_FILE_SELECTION( dialog ), "*.flt" );
    
    gtk_file_selection_hide_fileop_buttons( GTK_FILE_SELECTION( dialog ) );
    
    gtk_window_set_modal( GTK_WINDOW( dialog ), TRUE );
    
    gtk_widget_show( dialog );
}

static void FileDialogOK( GtkButton *button, gpointer data )
{
    gchar *selectedFile = gtk_file_selection_get_filename( GTK_FILE_SELECTION( data ) );
    strcpy( Shared->loadFileName, selectedFile );
    Shared->loadNewModel = 1;
    gtk_widget_destroy( GTK_WIDGET( data ) );
}

static void FileDialogCancel( GtkButton *button, gpointer data )
{
    gtk_widget_destroy( GTK_WIDGET( data ) );
}

static void QuitMenuHandler( GtkMenuItem *menuItem, gpointer p )
{
    Shared->exitFlag = 1;
    gtk_main_quit( );
}


static gint InitGtkGlWidget( GtkWidget *widget, GdkEventConfigure *event, gpointer userData )
{
    return FALSE;
}


static gint GetTopInput( GtkWidget *widget, GdkEvent *event, gpointer userData )
{
    static pid_t draw_pid = 0;

    switch ( event->type ) 
    {
	case GDK_MAP:
	    if ( GtkWinInfo->work_id )
	    {
		GtkWinInfo->work_id = gtk_idle_add( SimFrame, NULL );
	    }
	    /*
	     *	Resume any stopped draw process (see below) 
	     */
	    if ( draw_pid > 0 )
	    {
		kill( draw_pid, SIGCONT );
	    }
	    break;
    
	case GDK_UNMAP:
	    /*
	     *	If there is a separate draw process, 
	     *	it must be temporarily stopped, 
	     *	otherwise it busy-waits.
	     */
	    if ( GtkWinInfo->work_id )
	    {
		gtk_idle_remove( GtkWinInfo->work_id );
	    }
	    if ( draw_pid == 0 )
	    {
		int mode = pfGetMultiprocess( );
		if ( mode & PFMP_FORK_DRAW )
		{
		    draw_pid = pfGetPID( 0, PFPROC_DRAW );
		}
		else
		{
		    draw_pid = -1;
		}
	    }
	    if ( draw_pid > 0 )
	    {
		kill( draw_pid, SIGSTOP );
	    }
	    break;
    }
    return FALSE;
}

static gint KeyPressEvent( GtkWidget *widget, GdkEventKey *event )
{
    switch( event->keyval ) 
    {
	case GDK_Escape: 
	    Shared->exitFlag = 1;
	    gtk_main_quit( );
	    break;
	case GDK_space:
	    Shared->reset = 1;
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Reset");
	    break;
	case GDK_g:
	case GDK_F1:
	    Shared->drawStats = !Shared->drawStats;
	    break;
    }
    gtk_signal_emit_stop_by_name( GTK_OBJECT( widget ), "key_press_event" );
    return FALSE;
}

static gint KeyReleaseEvent( GtkWidget *widget, GdkEventKey *event )
{
    gtk_signal_emit_stop_by_name( GTK_OBJECT( widget ), "key_release_event" );
    return FALSE;
}

static gint ButtonPressEvent( GtkWidget *widget, GdkEventButton *event )
{
    gint x = (gint)event->x;
    gint y = Shared->winSizeY - (gint)event->y;
    Shared->inWindow = 1;
    switch ( event->button ) 
    {
	case 1:
	    Shared->mouseButtons |= GDK_BUTTON1_MASK;
	    break;
	case 2:
	    Shared->mouseButtons |= GDK_BUTTON2_MASK;
	    break;
	case 3:
	    Shared->mouseButtons |= GDK_BUTTON3_MASK;
	    break;
    }
    /*
     *	update cursor virtual position when cursor inside window 
     */
    Shared->mouseX = x;
    Shared->mouseY = y;
    return FALSE;
}

static gint ButtonReleaseEvent( GtkWidget *widget, GdkEventButton *event )
{
    switch ( event->button ) 
    {
	case 1:
	    Shared->mouseButtons &= ~GDK_BUTTON1_MASK;
	    break;
	case 2:
	    Shared->mouseButtons &= ~GDK_BUTTON2_MASK;
	    break;
	case 3:
	    Shared->mouseButtons &= ~GDK_BUTTON3_MASK;
	    break;
    }
    return FALSE;
}

static gint PointerMotionEvent( GtkWidget *widget, GdkEventMotion *event )
{
    if ( Shared->mouseButtons )
    {
	if ( event->is_hint )
	{
	    gint x, y;
	    gdk_window_get_pointer( event->window, &x, &y, NULL );
	    y = Shared->winSizeY - (gint)y;
	    /*
	     *	update cursor virtual position when cursor inside window 
	     */
	    Shared->mouseX = x;
	    Shared->mouseY = y;
	}
    }
    return FALSE;
}

static gint FocusChangeEvent( GtkWidget *widget, GdkEventFocus *event )
{
    return FALSE;
}

static gint EnterCrossingEvent( GtkWidget *widget, GdkEventCrossing *event )
{
    Shared->inWindow = 1;
    return FALSE;
}

static gint LeaveCrossingEvent( GtkWidget *widget, GdkEventCrossing *event )
{
    Shared->inWindow = 0;
    return FALSE;
}

/******************************************************************************
 *			    Application Routines
 ******************************************************************************
 */

static void InitPipe( void )
{
    /* 
     *	Set pfPipeWindow parameters from the Gtk widget 
     */
    pfPWinWSDrawable( Shared->pw, NULL, GtkWinInfo->gfXWin );
    pfPWinWSWindow( Shared->pw, NULL, GtkWinInfo->xWin );
}

static gint SimFrame( gpointer userData )
{
    /*
     *	wait until next frame boundary 
     */
    pfSync( );

    /*
     *	Set view parameters. 
     */
    UpdateView( );
    pfChanView( Shared->chan, Shared->view.xyz, Shared->view.hpr );

    /*
     *	Initiate traversal using current state 
     */
    pfFrame( );
    
    if ( Shared->loadNewModel ) 
    {
	/*
	 *  Load named database in place of current one 
	 */
	LoadModel( Shared->loadFileName );
	Shared->loadNewModel = 0;
    }
	    
    /*
     *	Read window origin and size (it may have changed)
     */
    pfGetPWinSize( pfGetChanPWin( Shared->chan ), 
			&Shared->winSizeX, &Shared->winSizeY );
		    
    return TRUE; /* Gtk: don't remove this callback */
}

/* 
 *	UpdateView() updates the eyepoint based on the information
 *	placed in shared memory by GetInput().
 */
static void UpdateView( void )
{
    static float speed = 0.0f;
    pfCoord *view = &Shared->view;
    float mx, my;
    float cp;

    if (!Shared->inWindow || Shared->reset)
    {
	speed = 0;
	if (Shared->reset)
	{
	    PFCOPY_VEC3(Shared->view.xyz, Shared->viewOrig.xyz);
	    PFCOPY_VEC3(Shared->view.hpr, Shared->viewOrig.hpr);
	    Shared->reset = 0;
	    Shared->mouseButtons = 0x0;
	    Shared->mouseX = Shared->mouseY = 0;
	}
	return;
    }
  
    switch (Shared->mouseButtons)
    {
    case GDK_BUTTON1_MASK: /* LEFTMOUSE: faster forward or slower backward*/
	if (speed > 0.0f)
	    speed *= 1.2f;
	else
	    speed /= 1.2f;
	
	if (PF_ABSLT(speed, LOSPEED * Shared->sceneSize))
	    speed = LOSPEED * Shared->sceneSize;
	else if (speed >=  HISPEED * Shared->sceneSize)
	    speed = HISPEED * Shared->sceneSize;
	break;

    case GDK_BUTTON2_MASK:	/* MIDDLEMOUSE: stop moving */
	speed = 0.0f;
	break;

    case GDK_BUTTON3_MASK: /* RIGHTMOUSE: faster backward or slower foreward*/
	if (speed < 0.0f)
	    speed *= 1.2f;
	else
	    speed /= 1.2f;
	
	if (PF_ABSLT(speed, LOSPEED * Shared->sceneSize))
	    speed = -LOSPEED * Shared->sceneSize;
	else if (speed <=  -HISPEED * Shared->sceneSize)
	    speed = -HISPEED * Shared->sceneSize;
	break;
    }
    
    if (Shared->mouseButtons)
    {
	mx = 2.0f * (Shared->mouseX / (float)Shared->winSizeX) - 1.0f;
	my = 2.0f * (Shared->mouseY / (float)Shared->winSizeY) - 1.0f;
	    
	/* update view direction */
	view->hpr[PF_H] -= mx * PF_ABS(mx);
	view->hpr[PF_P]  = my * PF_ABS(my) * 90.0f;
	view->hpr[PF_R]  = 0.0f;
    
	/* update view position */
	cp = cosf(PF_DEG2RAD(view->hpr[PF_P]));
	view->xyz[PF_X] += speed*sinf(-PF_DEG2RAD(view->hpr[PF_H])*cp);
	view->xyz[PF_Y] += speed*cosf(-PF_DEG2RAD(view->hpr[PF_H])*cp);
	view->xyz[PF_Z] += speed*sinf( PF_DEG2RAD(view->hpr[PF_P]));
    }
}

/******************************************************************************
 *			    Draw Process Routines
 ******************************************************************************
 */

/*
 *	OpenPipeWin() -- create a win: setup the GL and IRIS Performer.
 *	This procedure is executed in the draw process 
 *	(when there is a separate draw process).
 */

Display *Dsp=NULL;

static void
OpenPipeWin(pfPipeWindow *pw)
{
/*    pfPipe *p;*/
/*    Window w;*/

    pfOpenPWin(pw);
    Dsp = pfGetCurWSConnection();
}

/******************************************************************************
 *			    Cull Process Routines
 ******************************************************************************
 */

/*
 *	CullChannel() -- traverse the scene graph and generate a
 * 	display list for the draw process.  This procedure is 
 *	executed in the cull process.
 */

static void
CullChannel(pfChannel *channel, void *data)
{
    /* 
     * pfDrawGeoSet or other display listable Performer routines
     * could be invoked before or after pfCull()
     */
    
    pfCull();
}

/*
 *	DrawChannel() -- draw a channel and read input queue. this
 *	procedure is executed in the draw process (when there is a
 *	separate draw process).
 */
static void
DrawChannel (pfChannel *channel, void *data)
{
    /* erase framebuffer and draw Earth-Sky model */
    pfClearChan(channel);
    
    /* invoke Performer draw-processing for this frame */
    pfDraw();

    /* draw Performer throughput statistics */
    if (Shared->drawStats)
	pfDrawChanStats(channel);
}
