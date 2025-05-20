/*
 * Copyright 1996, Silicon Graphics, Inc.
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

/*
 * file: pfconv.c		$Revision: 1.9 $
 * --------------
 *
 * utility for converting Performer databases
 */

#include <stdlib.h>
#include <stdio.h>
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


/*
 *  optimizer modes
 */
#define OPT_MAKE_SHARE		0
#define OPT_COMBINE_LAYERS	1
#define OPT_FLATTEN		2
#define OPT_CLEAN_TREE		3
#define OPT_COMBINE_BILLBOARDS	4
#define OPT_FREEZE_TRANSFORMS	5
#define OPT_PARTITION		6
int opt_modes[] =
{
    1, /* OPT_MAKE_SHARE */
    1, /* OPT_COMBINE_LAYERS */
    1, /* OPT_FLATTEN */
    1, /* OPT_CLEAN_TREE */
    0, /* OPT_COMBINE_BILLBOARDS */
    0, /* OPT_FREEZE_TRANSFORMS */
    0, /* OPT_PARTITION */
};


/*
 *  local prototypes
 */
static pfNode *optimize(pfNode *node);
static int parse_args(int argc, char *argv[], char ***files);
static void usage(char *prog_name);


int
main(int argc, char **argv)
{
    pfNode *node;
    char **files;
    int num_files;
    int i;

    /*
     *  initialize OpenGL Performer
     */
    pfInit();
    pfInitArenas();
    pfNotifyLevel(PFNFY_INFO);
    pfFilePath("/usr/share/Performer/data:"
	       "/usr/demos/models:"
	       "/usr/demos/data/flt:"
	       "/usr/demos/data/textures");

    if ((num_files = parse_args(argc, argv, &files)) < 2)
	usage(argv[0]);

    /*
     *  initialize file read/write functions
     */
    for (i = 0; i < num_files; i++)
	if (!(pfdInitConverter(files[i])))
	    exit(1);

    /*
     *  start OpenGL Performer
     */
    pfMultiprocess(0);
    pfConfig();

    /*
     *  read input file(s)
     */
    if (num_files == 2)
    {
	node = pfdLoadFile(files[0]);
	if (node == NULL)
	    exit(2);
    }
    else
    {
	pfGroup *group = pfNewGroup();
	for (i = 0; i < num_files - 1; i++)
	{
	    node = pfdLoadFile(files[i]);
	    if (node == NULL)
		exit(2);
	    pfAddChild(group, node);
	}
	node = (pfNode *)group;
    }

    /*
     *  optimize input file (optional)
     */
    node = optimize(node);

    /*
     *  write output file
     */
    pfdStoreFile(node, files[num_files - 1]);

    /*
     *  indicate success
     */
    return 0;
}


/*
 * optimize -- apply various scene-graph rewrite rules
 */
static pfNode *
optimize(pfNode *node)
{
#if 0
    /* special: spatialize */
    node = pfdSpatialize( ...
#endif

    /*
     *  safe: optimize geostate sharing within scene graph
     */
    if (opt_modes[OPT_MAKE_SHARE])
	pfdMakeShared(node);

    /*
     *  safe: combine adjacent layer nodes
     */
    if (opt_modes[OPT_COMBINE_LAYERS])
	pfdCombineLayers(node);

    /*
     *  aggressive: combine adjacent billboard nodes
     */
    if (opt_modes[OPT_COMBINE_BILLBOARDS] > 0)
	pfdCombineBillboards(node, opt_modes[OPT_COMBINE_BILLBOARDS]);

    /*
     *  aggressive: freeze dynamic transforms (dcs -> scs)
     */
    if (opt_modes[OPT_FREEZE_TRANSFORMS])
	node = pfdFreezeTransforms(node, NULL);

    /*
     *  safe: flatten static transforms (apply scs transforms)
     */
    if (opt_modes[OPT_FLATTEN])
	pfFlatten(node, 0);

    /*
     *  safe: remove identity scs, singleton groups, empty geodes
     */
    if (opt_modes[OPT_CLEAN_TREE])
	node = pfdCleanTree(node, NULL);

    /*
     *  special: compute partition node for faster intersections
     */
    if (opt_modes[OPT_PARTITION])
    {
	pfPartition *partition = pfNewPart();
	pfAddChild(partition, node);
	pfBuildPart(partition);
	pfUpdatePart(partition);
	node = (pfNode *)partition;
    }

#if 0
    /* special: convert geosets to packed-attribute form */
    pfuTravCreatePackedAttrs(node, ...
#endif

#if 0
    /* special: convert geosets to display-list form */
    pfuTravSetDListMode(node, 1, PFU_ATTRS_NONE);
#endif

    return node;
}


/*
 *  parse commnand line arguments
 */
static int
parse_args(int argc, char *argv[], char ***files)
{
    int opt;

    /*
     *  process command-line arguments
     */
    while ((opt = getopt(argc, argv, "b:B:F:m:M:n:o:O:")) != -1)
    {
	switch (opt)
	{
	    case 'b':			/* builder modes */
	    case 'B':
		{
		    int mode;
		    int value;
		    int found;

		    found = sscanf(optarg, "%d,%d", &mode, &value);
		    if (found == 2)
			pfdBldrMode(mode, value);
		    else if (found == 1)
			pfdBldrMode(mode, (opt == 'b')? 0 : 1);
		    else
			usage(argv[0]);
		}
		break;

	    case 'F': /* Specify the file search path -- look here first */
		{
		    int oldLength = 0;
		    int newLength = 0;
		    int fullLength = 0;
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
	    
	    case 'm':			/* converter modes */
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

	    case 'n':			/* Set the notification (debug) level */
		pfNotifyLevel(atoi(optarg));
		break;

	    case 'o':			/* optimizer modes */
	    case 'O':
		{
		    int mode;
		    int value;
		    int found;

		    found = sscanf(optarg, "%d,%d", &mode, &value);
		    if (found == 2)
			opt_modes[mode] = value;
		    else if (found == 1)
			opt_modes[mode] = (opt == 'o')? 0 : 1;
		    else
			usage(argv[0]);
		}
		break;

	    default:
		usage(argv[0]);
	}
    }

    *files = &(argv[optind]);	/* return address of filename list */

    return(argc - optind);	/* number of files */
}



/*
 *  print usage
 */
static void
usage(char *prog_name)
{
    char *base_name;
    static const char *help_list =
	"usage:\n"
	"	%s [options] infile1 [infile2 ...] outfile\n"
	"\n"
	"options:\n"
	"    -b <mode>,<value>             -Set builder mode (default value is OFF)\n"
	"    -B <mode>,<value>             -Set builder mode (default value is ON)\n"
	"        0 -> PFDBLDR_MESH_ENABLE\n"
	"        1 -> PFDBLDR_MESH_SHOW_TSTRIPS\n"
	"        2 -> PFDBLDR_MESH_INDEXED\n"
	"        3 -> PFDBLDR_MESH_MAX_TRIS\n"
	"        4 -> PFDBLDR_MESH_RETESSELLATE\n"
	"        5 -> PFDBLDR_MESH_LOCAL_LIGHTING\n"
	"       10 -> PFDBLDR_AUTO_COLORS\n"
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
	"              0 - don't remove tcoords if no texture specified\n"
	"              1 - remove tcoords if no texture specified\n"
	"        16 -> PFDBLDR_AUTO_DISABLE_NCOORDS_BY_STATE\n"
	"              0 - don't remove ncoords if no material specified\n"
	"              1 - remove ncoords if no material specified\n"
	"        17 -> PFDBLDR_AUTO_LIGHTING_STATE_BY_NCOORDS\n"
	"              0 - don't automatically set lighting enable based on presence of ncoords\n"
	"              1 - automatically set lighting enable based on presence of ncoords\n"
	"        18 -> PFDBLDR_AUTO_LIGHTING_STATE_BY_MATERIALS\n"
	"              0 - don't automatically set lighting enable based on presence of material\n"
	"              1 - automatically set lighting enable based on presence of material\n"
	"        19 -> PFDBLDR_AUTO_TEXTURE_STATE_BY_TEXTURES\n"
	"              0 - don't automatically set texture enable based on presence of tcoords\n"
	"              1 - automatically set texture enable based on presence of tcoords\n"
	"        20 -> PFDBLDR_AUTO_TEXTURE_STATE_BY_TCOORDS\n"
	"              0 - don't automatically set texture enable based on presence of texture\n"
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
	"    -F <path>                     -Set file path\n"
	"    -m <ext>,<mode>,<value>       -Set converter mode (default value is 0)\n"
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
	"    -n <notify>                   -Debug level\n"
	"    -o <mode>,<value>             -Set optimizer mode (default value is OFF)\n"
	"    -O <mode>,<value>             -Set optimizer mode (default value is ON)\n"
	"        0 -> pfdMakeShare			defaults to ON\n"
	"        1 -> pfdCombineLayers			defaults to ON\n"
	"        2 -> pfFlatten				defaults to ON\n"
	"        3 -> pfdCleanTree			defaults to ON\n"
	"        4 -> pfdCombineBillboards,<size_limit>	defaults to OFF\n"
	"        5 -> pfdFreezeTransforms		defaults to OFF\n"
	"        6 -> pfPartition			defaults to OFF\n"
	"\n";

    /*
     *  just use program's base name: /usr/fred/base -> base
     */
    if (prog_name == NULL)
	base_name = "UNKNOWN";
    else if ((base_name = strchr(prog_name, '/')) == NULL)
	    base_name = prog_name;

    /*
     *  print list of command-line options and arguments
     */
    fprintf(stderr, help_list, base_name);
    fflush(stderr);
    exit(1);
}
