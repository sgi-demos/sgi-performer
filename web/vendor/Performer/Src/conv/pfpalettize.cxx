/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that (i) the above copyright notices and this
 * permission notice appear in all copies of the software and related
 * documentation, and (ii) the name of Silicon Graphics may not be
 * used in any advertising or publicity relating to the software
 * without the specific, prior written permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL,
 * INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY
 * THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h>
#endif

#include <Performer/pf.h>

#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pfdb/pfpfb.h>

#include <Performer/pfutil/pfuPalettizer.h>

static char *database      = (char *)0L;
static int  paletteS       = 512,
            paletteT       = 512;
static char *texturePrefix = "palette";
static char *outputFile    = "out.pfb";
static int mipmap = 0;
static unsigned int margin = 0;
static float    r          = 0.0f,
                g          = 0.0f,
                b          = 0.0f,
                a          = 1.0f;


static void usage( char *p )
{
    static char *helpList =
    "usage:\n"
    "    %s [options] <file>.ext ...\n"
    "\n"
    "options:\n"
    "    -h                         - This \"usage\" message\n"
    "    -b <r,g,b,a>               - background color\n"
    "    -m <margin>                - Enable MIPmapping and Specify margin surrounding tiles in palettes\n"
    "    -M <ext>,<mode>,<value>       -Set converter mode (default value is 1)\n"
    "        pfb modes\n"
    "        1 -> PFPFB_SAVE_TEXTURE_IMAGE\n"
    "              0 - save only file names, not images (default)\n"
    "              1 - save texture images in pfb file\n"
    "        2 -> PFPFB_SAVE_TEXTURE_PATH\n"
    "              0 - save only texture image file name (default)\n"
    "              1 - save full path of texture image file\n"
    "        3 -> PFPFB_SHARE_GS_OBJECTS\n"
    "              0 - don't share graphics state objects\n"
    "              1 - share graphics state objects (default)\n"
    "        4 -> PFPFB_COMPILE_GL\n"
    "              0 - don't make compiled gl objects (default)\n"
    "              1 - always make compiled gl objects\n"
    "              2 - make compiled gl objects as saved\n"
    "        5 -> PFPFB_SAVE_TEXTURE_PFI\n"
    "              0 - don't convert .rgb texture images to .pfi (default)\n"
    "              1 - convert .rgb texture images to .pfi\n"
    "    -o <file.ext>              - Write scene in .ext format\n"
    "    -p <prefix>                - Textures will be saved to \"<prefix>###.rgba\"\n"
    "    -w <sSize,tSize>           - Maximum palette size\n"
    ;

    fprintf(stderr, helpList, p);

    exit(-1);
}

static void doargs( int argc, char **argv )
{
    static char optionStr[] = "b:iw:p:o:m:M:h";

    extern char *optarg;
    extern int  optind;

    int     opt;

    while ((opt = getopt(argc, argv, optionStr)) != -1)
    {
        switch (opt)
        {
            case 'b' :
                if( sscanf( optarg,"%f,%f,%f,%f", &r, &g, &b, &a ) != 4 )
                    usage( argv[0] );
                break;

            case 'h' :
                usage( argv[0] );
                break;

            case 'm' :
                unsigned int u1;
		mipmap = 1;
                if( sscanf( optarg,"%d", &u1 ) == 1 )
                    margin = u1;
                else
                    usage( argv[0] );
                break;

	    case 'M':
		{
		    char *ext, *s;
		    int mode;
		    int value;
		    int found;

		    ext = optarg;
		    if (s = strchr(optarg, ','))
		    {
		        found = sscanf(s, ",%d,%d", &mode, &value);
			s[0] = '\0';
		    }
		    else
			found = 0;

		    if (found == 2)
			pfdConverterMode(ext, mode, value);
		    else if (found == 1)
			pfdConverterMode(ext, mode, (opt == 'm')? 0 : 1);
		    else
			usage(argv[0]);
		}
		break;

            case 'p' :
                texturePrefix = optarg;
                break;
                
            case 'w' :
                int i1, i2;
                if( sscanf( optarg, "%d,%d", &i1, &i2 ) == 2 )
                {
                    paletteS = i1;
                    paletteT = i2;
                }
                else
                    usage( argv[0] );
                break;

            case 'o' :
                outputFile = optarg;
                break;

            default : usage( argv[0] );
        }
    }


    if( optind < argc )
        database = argv[optind];


    if( database == (char *)0L )
        usage( argv[0] );

    if( texturePrefix == (char *)0L )
        usage( argv[0] );

    if( outputFile == (char *)0L )
        usage( argv[0] );
}

static void gatherGeoStates( pfNode *node, pfList *gl )
{
    int i;

    if (node->isOfType(pfGroup::getClassType()))
    {
        pfGroup *group = (pfGroup *) node;

        for(i = 0; i < group->getNumChildren(); i++)
            gatherGeoStates( group->getChild(i), gl );

    }
    else if( node->isOfType( pfGeode::getClassType() ))
    {
        pfGeode *geode = (pfGeode *)node;

        for( i = 0; i < geode->getNumGSets(); i++ )
        {
            pfGeoSet    *gset   = geode->getGSet(i);
            pfGeoState *gstate = gset->getGState();
            if( gl->search( gstate ) == -1 )
                gl->add( gstate );
        }
    }
    
}

static int countgstates( pfNode *node )
{
    pfList *gl;
    int ret;


    gl = new pfList();

    gatherGeoStates( node, gl );

    ret = gl->getNum();

    pfDelete( gl );

    return ret;
}

main( int argc, char **argv )
{
    pfNode *node;
    pfuPalettizer *p;
    int ngstates1, ngstates2;

    doargs( argc, argv );

    pfInit();
    pfMultiprocess( 0 );
    pfdInitConverter( database );
    pfdInitConverter( outputFile );

    pfConfig();

    pfFilePath( ".:/usr/share/Performer/data/:/usr/share/Performer/data/town" );


    if( (node = pfdLoadFile( database )) == (pfNode *)0L )
        exit(1);

    ngstates1 = countgstates( node );

    p = new pfuPalettizer;

    p->setTexturePrefix( texturePrefix );
    p->setMargin( margin );
    p->setMode( PFUPAL_MODE_MIPMAP, mipmap );
    p->setST( paletteS, paletteT );
    p->setBackground( r, g, b, a );
    p->palettize( node );
    pfList *texlist = p->getPaletteList();

    pfdMakeShared((pfNode *)node);

    ngstates2 = countgstates( node );

    pfNotify( PFNFY_NOTICE, PFNFY_PRINT, "%s : pfGeoStates reduced from %d to %d\n", argv[0], ngstates1, ngstates2 );

    for( int i = 0; i < texlist->getNum(); i++ )
    {
        pfTexture *tex = (pfTexture *)texlist->get( i );
        tex->saveFile( tex->getName() );
    }


    int n;
    if( ((n = strlen( outputFile )) >= 3) && (strncmp( &outputFile[n-3], "pfb", 3 ) == 0 ) )
    {
	    pfdConverterMode( outputFile,  PFPFB_SAVE_TEXTURE_PATH, 1 );
    }
    pfdStoreFile( node, outputFile );

    exit(0);
}
