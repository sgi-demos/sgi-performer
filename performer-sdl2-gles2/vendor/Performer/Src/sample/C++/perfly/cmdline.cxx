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
 * cmdline.C
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifndef WIN32
#include <getopt.h>
#else
/* Getopt is defined in pfutil on windows */
#include <Performer/pfutil/getopt.h>
#endif

#include <Performer/pf/pfEarthSky.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pfdu.h>
#include <Performer/pfui.h>

#include "perfly.h"
#include "cmdline.h"

/* **************************************************************************
 *		Process the command line options
 ****************************************************************************/

static void
usage(char *progName)
{
    char	*baseName;
    static char *helpList =
	"usage:\n"
	"	%s [options] model.ext ...\n"
	"\n"
	"options:\n"
	"    -A <string>                   -welcome text\n"
	"    -a <string>                   -overlay text\n"
	"    -b r,g,b[,a]                  -Earth/Sky clear color\n"
	"    -c <v,numChannels>            -Number of channels\n"
	"	 v -> optional request for mapping to video channels\n"
	"    -C a,b,c,...                  -Pipes for channels (L-to-R)\n"
	"    -d                            -Drive\n"
	"    -D                            -Lock down draw process\n"
	"    -e <h,p,r>                    -Initial view angles\n"
	"    -E <string>                   -Earth sky mode\n"
	"        tag      -> tag clear\n"
	"        clear    -> regular clear\n"
	"        sky      -> sky only\n"
	"        skygrnd  -> sky and ground\n"
	"        skyclear -> sky and clear\n"
	"    -f                            -Fly\n"
	"    -F <path>                     -Set file path\n"
	"    -g <format>                   -GUI format\n"
	"        0 -> GUI OFF\n"
	"        1 -> GUI_VERTICAL\n"
	"        2 -> GUI_HORIZONTAL\n"
	"    -G                            -Use GANGDRAW with Multipipe\n"
	"    -h                            -Print command list\n"
	"    -H <horiz_fov>,<vert_fov>,<offset> -Set Custom Multipipe Field of View\n"
	"    -i <count>                    -Set file loading repeat count\n"
	"    -I <count>                    -Exit after printing stats for count frames\n"
	"    -j <font_type>                -Set font type\n"
	"        0 -> PFDOBJFNT_TEXTURED\n"
	"        1 -> PFDOBJFNT_OUTLINED\n"
	"        2 -> PFDOBJFNT_FILLED\n"
	"        3 -> PFDOBJFNT_EXTRUDED\n"
	"    -J <font_name>                -Set Font to Named Font\n"
	"    -k r,g,b[,a]                  -Scribed-style draw color\n"
	"    -K <count>                    -Combine billboards up to size count\n"
	"    -l <style>                    -Set lighting enable\n"
	"        0 -> lighting off\n"
	"        1 -> lighting eye\n"
	"        2 -> lighting sky\n"
	"    -l <x,y,z,r,g,b>              -Add a light source to the scene\n"
	"    -L                            -Free then Lock down processes (requires root ID)\n"
	"    -m <mode>                     -Multiprocessing mode\n"
	"       -1 -> PFMP_DEFAULT\n"
	"        0 -> PFMP_APPCULLDRAW\n"
	"        2 -> PFMP_APP_CULLDRAW\n"
	"        4 -> PFMP_APPCULL_DRAW\n"
	"        6 -> PFMP_APP_CULL_DRAW\n"
	"	 131072 -> PFMP_APPCULL_DL_DRAW\n"
	"	 131074 -> PFMP_APP_CULL_DL_DRAW\n"
	"	 65540 -> PFMP_APPCULLoDRAW\n"
	"	 65542 -> PFMP_APP_CULLoDRAW\n"
	"    -M <mode>                     -Multipipe mode\n"
	"        0 -> single pipe mode\n"
	"        1 -> multipipe mode\n"
	"        2 -> hyperpipe mode\n"
	"    -n <notify>                   -Debug level\n";
    static char *helpList2 =
	"    -N                            -Assign non-degrading priorities to Performer processes\n"
	"    -o <mode>,<value>             -Set builder mode (default value is OFF)\n"
	"    -O <mode>,<value>             -Set builder mode (default value is ON)\n"
	"        0 -> PFDBLDR_MESH_ENABLE\n"
	"        1 -> PFDBLDR_MESH_SHOW_TSTRIPS\n"
	"        2 -> PFDBLDR_MESH_INDEXED\n"
	"        3 -> PFDBLDR_MESH_MAX_TRIS\n"
	"        4 -> PFDBLDR_MESH_RETESSELLATE\n"
	"         5 -> PFDBLDR_MESH_LOCAL_LIGHTING\n"
	"        10 -> PFDBLDR_AUTO_COLORS\n"
	"             0 -> PFDBLDR_COLORS_PRESERVE  + leave colors alone\n"
	"             1 -> PFDBLDR_COLORS_MISSING   - make missing colors\n"
	"             2 -> PFDBLDR_COLORS_GENERATE  - make all colors\n"
	"             3 -> PFDBLDR_COLORS_DISCARD   - toss existing colors\n"
	"       11 -> PFDBLDR_AUTO_NORMALS\n"
	"             0 -> PFDBLDR_NORMALS_PRESERVE - leave normals alone\n"
	"             1 -> PFDBLDR_NORMALS_MISSING  + make missing normals\n"
	"             2 -> PFDBLDR_NORMALS_GENERATE - make all normals\n"
	"             3 -> PFDBLDR_NORMALS_DISCARD  - toss existing normals\n"
	"       12 -> PFDBLDR_AUTO_TEXTURE\n"
	"             0 -> PFDBLDR_TEXTURE_PRESERVE + leave texture coord alone\n"
	"             1 -> PFDBLDR_TEXTURE_MISSING  - make missing texture coord\n"
	"             2 -> PFDBLDR_TEXTURE_GENERATE - make all texture coord\n"
	"             3 -> PFDBLDR_TEXTURE_DISCARD  - toss existing texture coord\n"
	"       13 -> PFDBLDR_AUTO_ORIENT\n"
	"             0 -> PFDBLDR_ORIENT_PRESERVE  - leave normal and order alone\n"
	"             1 -> PFDBLDR_ORIENT_NORMALS   - make normal match vertex order\n"
	"             2 -> PFDBLDR_ORIENT_VERTICES  + make vertex order match normal\n"
	"        15 -> PFDBLDR_AUTO_DISABLE_TCOORDS_BY_STATE\n"
	"              0 - dont remove tcoords if no texture specified\n"
	"              1 - remove tcoords if no texture specified\n"
	"        16 -> PFDBLDR_AUTO_DISABLE_NCOORDS_BY_STATE\n"
	"              0 - dont remove ncoords if no material specified\n"
	"              1 - remove ncoords if no material specified\n"
	"        17 -> PFDBLDR_AUTO_LIGHTING_STATE_BY_NCOORDS\n"
	"              0 - dont automatically set lighting enable based on presence of ncoords\n"
	"              1 - automatically set lighting enable based on presence of ncoords\n"
	"        18 -> PFDBLDR_AUTO_LIGHTING_STATE_BY_MATERIALS\n"
	"              0 - dont automatically set lighting enable based on presence of material\n"
	"              1 - automatically set lighting enable based on presence of material\n"
	"        19 -> PFDBLDR_AUTO_TEXTURE_STATE_BY_TEXTURES\n"
	"              0 - dont automatically set texture enable based on presence of tcoords\n"
	"              1 - automatically set texture enable based on presence of tcoords\n"
	"        20 -> PFDBLDR_AUTO_TEXTURE_STATE_BY_TCOORDS\n"
	"              0 - dont automatically set texture enable based on presence of texture\n"
	"              1 - automatically set texture enable based on presence of texture\n"
	"       30 -> PFDBLDR_BREAKUP\n"
	"       31 -> PFDBLDR_BREAKUP_SIZE\n"
	"       32 -> PFDBLDR_BREAKUP_BRANCH\n"
	"       33 -> PFDBLDR_BREAKUP_STRIP_LENGTH\n"
	"       34 -> PFDBLDR_SHARE_MASK\n"
	"       35 -> PFDBLDR_ATTACH_NODE_NAMES\n"
	"       36 -> PFDBLDR_DESTROY_DATA_UPON_BUILD\n"
	"       37 -> PFDBLDR_PF12_STATE_COMPATIBLE\n"
	"       38 -> PFDBLDR_BUILD_LIMIT  -maximum number of tris/strip\n"
	"    -p <x,y,z>                    -Initial view position\n"
	"    -P <phase>                    -Phase\n"
	"        0 -> PFPHASE_FLOAT\n"
	"        1 -> PFPHASE_LOCK\n"
	"        2 -> PFPHASE_FREE_RUN\n"
	"        3 -> PFPHASE_LIMIT\n"
	"    -q <optimize>                 -Optimize Scene GStates into single Scene pfGeoState\n"
	"    -Q <optimize>                 -Flatten tree and remove empty nodes\n"
	"        0 -> Off, 1 -> On\n"
	"    -r <frameRate>                -Target frame rate in Hertz\n"
	"    -R <mode>                     -Retessellate input geometry\n"
	"        0 -> Off, 1 -> On\n"
	"    -s <LODscale>                 -Set LOD scale factor\n"
	"    -S <mode>                     -Show triangle strips via color coding\n"
	"        0 -> Off, 1 -> On\n"
        "    -t <0xVisID0>,<0xVisID1>,... -specify GLX Visual ID per pipe has Hex Integer(OpenGL Only)\n" 
	"    -T <mode>                     -use gl display lists\n"
	"        0 -> Off, 1 -> On\n"
	"    -u                            -Inflate input files\n"
	"    -U                            -Start Calligraphic boards and LPoint process"
	"    -v <type>                     -Use vertex arrays of specified type\n"
	"	 0 -> Off \n"
	"	 1 -> PFGS_PA_C4UBN3ST2FV3F - all attrs packed in arrays\n"
	"	 2 -> PFGS_PA_C4UBN3ST2F - all attrs but verts packed in arrays\n"
	"    -V <mode>			   -Set Dynamic Resolution (DVR) mode\n"
	"    	<mode,xsize,ysize>	   - Set DVR mode and initial size.\n"
	"        0 -> PFPVC_DVR_OFF \n"
	"        1 -> PFPVC_DVR_MANUAL \n"
	"        2 -> PFPVC_DVR_AUTOMATIC \n"
	"    -w <file>.out                 -Write scene in ASCII\n"
	"    -w <file.ext>                 -Write scene in .ext format\n"
	"    -W <Size>                     -Window size\n"
	"    -W <xSize,ySize>              -Window size\n"
	"    -x <fork>                     -Set use of GLX input handling, forked != 0-> asynch\n"
	"	0 -> input handling is not forked.\n"
	"	1 -> forked X input handling is used.\n"
	"	2 -> forked X input allowing multi-pipe (the default).\n"
	"    -X <radius>                   -Explode input files\n"
	"    -y ext,mode,value             -Set loader mode for file extension ext to value\n"
	"    -Y ext,alias                  -Substitute file extension alias for file extension ext\n"
	"    -z near,far                   -Set near/far clip ranges\n"
	"    -Z                            -Don't initially free CPUs before lock\n"
        "    -3				   -use default Performer Z\n"
        "    -4				   -minimal multisample visual (4 samples)\n"
	"    -7				   -Trackball (default, undoes effect of -f, -d)\n"
        "    -1 demo_path_file		   -Demo path file name\n"
	"\n";
    
    /* just use program's base name: /usr/fred/base -> base */
    if (progName == NULL)
        baseName = "UNKNOWN";
    else if ((baseName = strchr(progName, '/')) == NULL)
	    baseName = progName;
    
    /* print list of command-line options and arguments */
    fprintf(stderr, helpList, baseName);
    fprintf(stderr, helpList2);
    fflush(stderr);
    exit(1);
}

char optionStr[] = 
    "A:a:b:B:c:C:dD:e:E:fF:g:GhH:i:I:j:J:k:K:l:Lm:M:"
    "o:O:Nn:p:P:q:Q:r:R:s:S:t:T:uUv:V:W:w:x:X:y:Y:z:Z23478:1:?";

int
processCmdLine(int argc, char *argv[], char ***files)
{
    extern char *optarg;
    extern int        optind;
    /* save and restore optarg&optind so we can be re-entrant */
    char * 	       saved_optarg = optarg;
    int 	       saved_optind = optind;
    int                opt;
    int                guiFormatSet = 0;
    int		       returnval;
    
    /* process command-line arguments */
    while ((opt = getopt(argc, argv, optionStr)) != -1)
    {
        switch (opt)
        {
	    /* Specify introductory 3D welcome text */
	case 'A':	
	    strcpy(ViewState->welcomeText, optarg);
	    break;
	    
	    /* Specify overlay-plane attribution text */
	case 'a':	
	    strcpy(ViewState->overlayText, optarg);
	    break;
	    
	    /* Specify mapping of processes to processors for locked CPUs */
	case 'B': 
	    {
		int i;
		int count;
		int num = 0;
		char	*start = optarg;

		while ((sscanf(start, "%d%n", &i, &count) == 1) && (num <= 4))
		{
		    if (*(start += count) != '\0')
			++start;
		    num ++;
		    switch (num)
		    {
			case 1:
			    AppCPU = i;
			    break;
			case 2:
			    CullCPU = i;
			    break;
			case 3:
			    DrawCPU = i;
			    break;
			case 4:
			    LPointCPU = i;
			    break;
		    }
		}

		if (num == 0)
		    usage(argv[0]);
	    }
	    break;
	    
	    /* Specify the Earth/Sky clear color */
	case 'b':
	    if (sscanf(optarg, "%f,%f,%f,%f",
		       &ViewState->earthSkyColor[0],
		       &ViewState->earthSkyColor[1],
		       &ViewState->earthSkyColor[2],
		       &ViewState->earthSkyColor[3]) != 4 &&
		sscanf(optarg, "%f,%f,%f",
		       &ViewState->earthSkyColor[0],
		       &ViewState->earthSkyColor[1],
		       &ViewState->earthSkyColor[2]) != 3)
		usage(argv[0]);
	    break;
	    
	    /* Specify number of channels to use */
	case 'c':	
	    /* accept v,%d - v is optional */
	    if (sscanf(optarg, "v,%d", &NumChans) != 1)
	    {
		ViewState->MCO = 0;
		if (sscanf(optarg, "%d", &NumChans) != 1)
		    usage(argv[0]);
	    } 
	    else
		ViewState->MCO = 1;
	    if (NumChans < 1)
		usage(argv[0]);
	    if (!guiFormatSet && (NumChans > 1))
		ViewState->guiFormat = GUI_HORIZONTAL;
	    break;
	    
	    /* Specify the pipe to use for each channel */
	case 'C':	
	    {
		int	 	index = 0;
		int	 	count = 0;
		char	*start = optarg;
		
		while (sscanf(start, "%d%n", &ChanOrder[index++], &count) == 1)
		    if (*(start += count) != '\0')
			++start;
	    }
	    break;
	    
	    /* Set pfuXformer default to Drive */
	case 'd':
	    ViewState->xformerModel = PFITDF_DRIVE;
	    break;
	    
	    /* lock down processor for just draw process */
	case 'D':
	    ViewState->procLock |= PFMP_FORK_DRAW;
	    break;
	    
	    /* Set the initial view Euler angles (h p r) */
	case 'e':
	    if (sscanf(optarg, "%f,%f,%f",
		       &ViewState->initViews[0].hpr[PF_H],
		       &ViewState->initViews[0].hpr[PF_P],
		       &ViewState->initViews[0].hpr[PF_R]) == 3)
		InitHPR = TRUE;
	    else
		usage(argv[0]);
	    break;
	    
	    /* Set Earth Sky mode */
	case 'E':
	    if (strcmp(optarg, "tag") == 0)
		if (ViewState->haveTagClear)
		    ViewState->earthSkyMode = PFES_TAG;
		else
		    ViewState->earthSkyMode = PFES_FAST;
	    else if (strcmp(optarg, "sky") == 0)
		ViewState->earthSkyMode = PFES_SKY;
	    else if (strcmp(optarg, "skygrnd") == 0)
		ViewState->earthSkyMode = PFES_SKY_GRND;
	    else if (strcmp(optarg, "skyclear") == 0)
		ViewState->earthSkyMode = PFES_SKY_CLEAR;
	    else if (strcmp(optarg, "clear") == 0)
		ViewState->earthSkyMode = PFES_FAST;
	    else 
		usage(argv[0]);
	    break;
	    
	    /* Set pfuXformer default to Fly */
	case 'f':
	    ViewState->xformerModel = PFITDF_FLY;
	    break;
	    
	    /* Specify the file search path -- look here first */
	case 'F':
	    {
	    size_t oldLength = 0;
	    size_t newLength = 0;
	    size_t fullLength = 0;
	    const char *oldPath = pfGetFilePath();
	    char *newPath = optarg;
	    char *fullPath = NULL;

	    if (oldPath != NULL)
		oldLength = strlen(oldPath);
	    if (newPath != NULL)
		newLength = strlen(newPath);
	    fullLength = oldLength + newLength;

	    if (fullLength > 0)
	    {
		/* allocate space for old, ":", new, and ZERO */
		fullPath = (char *)pfMalloc(fullLength + 2, NULL);
		fullPath[0] = '\0';
		if (oldPath != NULL)
		    strcat(fullPath, oldPath);
		if (oldPath != NULL && newPath != NULL)
		    strcat(fullPath, ":");
		if (newPath != NULL)
		    strcat(fullPath, newPath);
		pfFilePath(fullPath);
		pfFree(fullPath);
	    }
	    }
	    break;
	    
	    /* Specify the GUI format */
	case 'g':
	    ViewState->gui = ((ViewState->guiFormat = atoi(optarg)) ? 1 : 0);
	    if( ViewState->guiFormat != GUI_HORIZONTAL )
		ViewState->guiFormat = GUI_VERTICAL;
	    guiFormatSet = 1;
	    break;
	    
	    /* Initialize with GUI Panel disabled */
	case 'G':
	    GangDraw ^= 1;
	    break;
	    
	    /* set repeat count for file loading */
	case 'H':
	    if(sscanf(optarg, "%f,%f,%f", 
		      &ViewState->fov[0],
		      &ViewState->fov[1],
		      &ViewState->fov[2]) == 3)
		InitFOV = TRUE;
	    else
		usage(argv[0]);
	    break;
	case 'i':
	    ViewState->iterate = atoi(optarg);
	    break;
	    
	    /* set repeat count for file loading */
	case 'I':
	    ViewState->printStats = atoi(optarg);
	    ViewState->exitCount = atoi(optarg);
	    break;
	    
	case 'j':
	    ViewState->objFontType = atoi(optarg);
	    break;

	case 'J':
	    ViewState->objFontName = strdup(optarg);
	    break;
	    
	    /* Specify the scribed-style draw color */
	case 'k':
	    if (sscanf(optarg, "%f,%f,%f,%f",
		       &ViewState->scribeColor[0],
		       &ViewState->scribeColor[1],
		       &ViewState->scribeColor[2],
		       &ViewState->scribeColor[3]) != 4 &&
		sscanf(optarg, "%f,%f,%f",
		       &ViewState->scribeColor[0],
		       &ViewState->scribeColor[1],
		       &ViewState->scribeColor[2]) != 3)
		usage(argv[0]);
	    break;
	    
	case 'K':
	    ViewState->combineBillboards = atoi(optarg);
	    break;

	    
	    /* remember light source definition */
	case 'l':
	    {
		float lxyz[3], lrgb[3], l;
		if (sscanf(optarg, "%f,%f,%f,%f,%f,%f", 
			   &lxyz[0], &lxyz[1], &lxyz[2], &lrgb[0], &lrgb[1], &lrgb[2]) == 6)
		{
		    if (ViewState->lamps < (MAX_LAMPS - 1))
		    {
			/* set default default values */
			ViewState->lampXYZ[ViewState->lamps].set(0.0f,0.0f,0.0f);
			ViewState->lampRGB[ViewState->lamps].set(1.0f,1.0f,1.0f);
			
			/* read specific values */
			sscanf(optarg, "%f,%f,%f,%f,%f,%f",
			       &ViewState->lampXYZ[ViewState->lamps][0],
			       &ViewState->lampXYZ[ViewState->lamps][1],
			       &ViewState->lampXYZ[ViewState->lamps][2],
			       &ViewState->lampRGB[ViewState->lamps][0],
			       &ViewState->lampRGB[ViewState->lamps][1], 
			       &ViewState->lampRGB[ViewState->lamps][2]);
			
			/* advance lamp-definition count */
			ViewState->lamps += 1;
		    }
		}
		else if (sscanf(optarg, "%f", &l) == 1)
		    ViewState->lighting = l;
		else
		    usage(argv[0]);
	    }
	    break;
	    
	    /* lock down processors for app, cull, and draw */
	case 'L':
	    ViewState->procLock = ((uint)~0);
	    break;
	    
	    /* Specify multiprocessing mode */
	case 'm':
	    ProcSplit = atoi(optarg);
	    break;
	    
	    /* Enable multipipe mode */
	case 'M':
	    Multipipe = atoi(optarg);
	    break;
	    
	    /* Set the notification (debug) level */
	case 'n':
	    NotifyLevel = atoi(optarg);
	    break;
	    
	    /* Assign non-degrading priorities to Performer processes. */
	case 'N':
	    PrioritizeProcs = 1;
	    break;
	    
        case 'o':
            {
                int mode;
                int value;
		int found;
		
                found = sscanf(optarg, "%d,%d", &mode, &value);
                if (found == 2)
		    pfdBldrMode(mode, value);
		else if (found == 1)
		    pfdBldrMode(mode, 0);
		else
		    usage(argv[0]);
            }
            break;
	    
	case 'O':
            {
                int mode;
                int value;
		int found;
		
                found = sscanf(optarg, "%d,%d", &mode, &value);
                if (found == 2)
		    pfdBldrMode(mode, value);
		else if (found == 1)
		    pfdBldrMode(mode, 1);
		else
		    usage(argv[0]);
            }
	    break;
	    
	    /* Set the initial view position (x y z)*/
	case 'p':
	    if (sscanf(optarg, "%f,%f,%f",
		       &ViewState->initViews[0].xyz[PF_X],
		       &ViewState->initViews[0].xyz[PF_Y],
		       &ViewState->initViews[0].xyz[PF_Z]) == 3)
		InitXYZ = TRUE;
	    else
		usage(argv[0]);
	    break;
	    
	    /* Set the frame phase */
	case 'P':
	    ViewState->phase = atoi(optarg);
	    break;
	    
	    /* Whether to optimize into a scene geostate */
	case 'q':
	    ViewState->optimizeGStates = atoi(optarg);
	    break;
	    
	    /* Scene cleaning */
	case 'Q':
	    ViewState->optimizeTree = atoi(optarg);
	    break;
	    
	    /* Set the frame rate */
	case 'r':
	    ViewState->frameRate = (float)atof(optarg);
	    if( ViewState->frameRate <= 0.0000001f)
		ViewState->frameRate = 30.0f;
	    break;
	    
	    /* Enable retessellation of input geometry */
	case 'R':
	    pfdBldrMode(PFDBLDR_MESH_RETESSELLATE, atoi(optarg));
	    break;
	    
	    /* set LOD scale factor */
	case 's':
	    ViewState->LODscale = atof(optarg);
	    break;
	    
	    /* Enable color-coding of individual triangle strips */
	case 'S':
	    pfdBldrMode(PFDBLDR_MESH_SHOW_TSTRIPS, atoi(optarg));
	    break;

	    /* Visual ids for up to 8 pipes, hex (start with 0x) or decimal */
	case 't':
	    {
	        int i, nscanned = sscanf(optarg,"%i,%i,%i,%i,%i,%i,%i,%i",
					 &ViewState->visualIDs[0],
					 &ViewState->visualIDs[1],
					 &ViewState->visualIDs[2],
					 &ViewState->visualIDs[3],
					 &ViewState->visualIDs[4],
					 &ViewState->visualIDs[5],
					 &ViewState->visualIDs[6],
					 &ViewState->visualIDs[7]);
		if (nscanned < 1)
		{
		    nscanned = 1;
		    ViewState->visualIDs[0] = -1;
		}
		/* For all remaining pipes, use the last number specified */
		for (i = nscanned; i < MAX_PIPES; ++i)
		    ViewState->visualIDs[i] = ViewState->visualIDs[nscanned-1];
	    }
	    break;

	/* set display-list mode */
	case 'T':
	    if (atoi(optarg))
		ViewState->drawMode = DLIST_GEOSET;
	    else
		ViewState->drawMode = DLIST_OFF;
	    break;

        /* inflate files */
	case 'u':
	    ViewState->explode = -1;
	    break;

	/* start calligraphics */

	case 'U':
	    ViewState->startCallig = 1;
	    break;
	
	    /* set pfGeoSet packed attribute format for vertex arrays */
	case 'v':
	    PackedAttrFormat = atoi(optarg);
	    break;
	    /* change DVR mode */
	case 'V':
	    if (sscanf(optarg, "%d,%d,%d", &ViewState->doDVR, &ViewState->dvrXSize, &ViewState->dvrYSize) != 3)
		ViewState->doDVR = atoi(optarg);
	    break;
	    /* Write scene to a file */
	case 'w':
	    if (optarg[0] == '-')
		usage(argv[0]);
	    WriteFileDbg = strdup(optarg);
	    break;
	    
	    /* Set the window size */
	case 'W':
	    if (sscanf(optarg, "%d,%d", &WinSizeX, &WinSizeY) != 2)
		if (sscanf(optarg, "%d", &WinSizeX) == 1)
		    WinSizeY = WinSizeX;
		else
		    usage(argv[0]);
	    break;
	    
	case 'x':
	    {
		int i = atoi(optarg);
		switch(i)
		{ 
		case 0: 
		    ViewState->input = PFUINPUT_NOFORK_X; 
		    MultiPipeInput = 0;
		    break;
		case 1:
		    ViewState->input = PFUINPUT_X; 
		    MultiPipeInput = 0;
		    break;
		case 2:
		    ViewState->input = PFUINPUT_X;
		    MultiPipeInput = 1;
		    break;
		}
	    }
	    break;
	    
	    /* explode files */
	case 'X':
	    ViewState->explode = (float)atof(optarg);
	    break;
	    /* Set specific loader mode */
        case 'y':
            {
                int mode;
                float val;
                char buf[64], *c;
		if ((c=strchr(optarg, ',')) != NULL) *c = ' ';
	    	if (sscanf(optarg,"%s %d,%f",buf,&mode,&val) == 3)
		    pfdConverterMode(buf,mode,val);
		else 
		    usage(argv[0]);
	    }
	    break;
	    /* Add an file extension alias */
	case 'Y':
	    {
		char buf[64],buf2[64], *c;
		
		if ((c=strchr(optarg, ',')) != NULL) *c = ' ';
	    	if (sscanf(optarg,"%s %s",buf,buf2) == 2) 
		    pfdAddExtAlias(buf,buf2);
		else
		    usage(argv[0]);
	    }
	    break;

	case 'z':
	    {
		float nearPlane, farPlane;
		if (sscanf(optarg, "%f,%f", &nearPlane, &farPlane) == 2)
		{
		    ViewState->nearPlane = nearPlane;
		    ViewState->farPlane = farPlane;
		    ViewState->nearFogRange = ViewState->nearPlane;
		    ViewState->farFogRange = ViewState->farPlane;
		}
		else
		    usage(argv[0]);
	    }
	    
	    break;
	    
	case 'Z':
	    FreeInitCPUs = 0;
	    break;
	    
	case '1':
	    strcpy(ViewState->demoPathFileName, optarg);
	    ViewState->doDemoPath = 1;
	    break;

	case '3':
	    ZBits = 23;
	    break;
	case '4':
	    Multisamples = 4;
	    break;
	case '7':
	    ViewState->xformerModel = PFITDF_TRACKBALL;
	    break;
        case '2':
            ViewState->xformerModel = PFITDF_TETHER;
            break;
	    
	    /* Unknown option is no option at all */
	case '?':
	case 'h':
	default:
	    usage(argv[0]);
	}
    }

    /* preprocess results from cmd line */
    /************************************/

    /* force lpoint process if cmdline ask for Calligraphics (-U) */
    if (ViewState->startCallig)
	if (ProcSplit == PFMP_DEFAULT)
	    	ProcSplit = PFMP_FORK_LPOINT | PFMP_FORK_CULL | PFMP_FORK_DRAW;
	else
	    ProcSplit |= PFMP_FORK_LPOINT;

    /* prepare returned results */
    /****************************/

    /* return address of database filename list through "files" argument */
    *files = &(argv[optind]);

    /* function return value is number of database files to be read */
    returnval = argc - optind;

    /* restore original global vars for re-entrancy */
    optind = saved_optind;
    optarg = saved_optarg;

    return returnval;
}
