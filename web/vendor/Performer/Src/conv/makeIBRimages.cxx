//
// Copyright 2001, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// This source code ("Source Code") was originally derived from a
// code base owned by Silicon Graphics, Inc. ("SGI")
// 
// LICENSE: SGI grants the user ("Licensee") permission to reproduce,
// distribute, and create derivative works from this Source Code,
// provided that: (1) the user reproduces this entire notice within
// both source and binary format redistributions and any accompanying
// materials such as documentation in printed or electronic format;
// (2) the Source Code is not to be used, or ported or modified for
// use, except in conjunction with OpenGL Performer; and (3) the
// names of Silicon Graphics, Inc.  and SGI may not be used in any
// advertising or publicity relating to the Source Code without the
// prior written permission of SGI.  No further license or permission
// may be inferred or deemed or construed to exist with regard to the
// Source Code or the code base of which it forms a part. All rights
// not expressly granted are reserved.
// 
// This Source Code is provided to Licensee AS IS, without any
// warranty of any kind, either express, implied, or statutory,
// including, but not limited to, any warranty that the Source Code
// will conform to specifications, any implied warranties of
// merchantability, fitness for a particular purpose, and freedom
// from infringement, and any warranty that the documentation will
// conform to the program, or any warranty that the Source Code will
// be error free.
// 
// IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
// LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
// ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
// SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
// OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
// PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
// OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
// USE, THE SOURCE CODE.
// 
// Contact information:  Silicon Graphics, Inc., 
// 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
// or:  http://www.sgi.com
//
//
// IBRnode.C: illustrates the use of pfIBRnode and pfIBRtexture
//



#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfSCS.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfDoubleSCS.h>
#include <Performer/pf/pfDoubleDCS.h>
#include <Performer/pf/pfLOD.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfIBRnode.h>

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

///////////////////////////////////////////////////////////////////////////
//
// structure that resides in shared memory so that the
// application, cull, and draw processes can access it.

typedef struct
{
    char       outputName[2048];
    char       fileName[2048];
    int        numTex[PFDD_MAX_RINGS];
    float      horAngle[PFDD_MAX_RINGS];
    int        numRings;
    int        uniformViews;
    pfDirData  *viewData;
    pfVec3     *viewDir;
    int        numDirs;
    pfTexture  *tex;
    char       *texImage;
    char       *oversampledImage;
    int        sizex;
    int        sizey;
    int        samples;
    int        skipTex;
    GLboolean  useMultisampling;
    GLboolean  finished;
} SharedData;

static SharedData *Shared;

// parameters that are not needed in draw process
// window origin
static int posx = 0;
static int posy = 0;

static float nodeScale = 1.0;
static float proxyScale = 0.0;

static float distance = 0.0;
static float aspect;

static GLboolean saveFile = 0;
static GLboolean addProxy = 0;
static char *proxyFile = NULL;
static pfVec2 texShift;

static char *infoFile = NULL;

static GLboolean skipDraw = 0;

#define MAX_LIGHTS 10

static int numLights = 0;
static pfVec4 lightPos[MAX_LIGHTS];
static pfVec3 lightColor[MAX_LIGHTS];
static pfSphere bsphere;
static pfSphere proxybsphere;

static pfVec4 *color;

static void DrawChannel(pfChannel *chan, void *data);

/***************************************************************************/
static void
OpenPipeWin(pfPipeWindow *pw)
{
    // Configure and open GL window
    int constraints[] = {
	//PFFB_DOUBLEBUFFER,
	PFFB_RGBA,
	PFFB_RED_SIZE, 8, 
	PFFB_GREEN_SIZE, 8, 
	PFFB_BLUE_SIZE, 8, 
	PFFB_ALPHA_SIZE, 8, 
#ifndef __linux__
	PFFB_DEPTH_SIZE, 15, 
	PFFB_SAMPLES, 0,     // DO NOT REMOVE! or change code below
#endif
	NULL};

#ifndef __linux__
    {
	int samples, i;

	if(Shared->useMultisampling)
	    pfQuerySys(PFQSYS_MAX_MS_SAMPLES, &samples);
	else 
	    samples = 0;
	
	i = 0;
	while(constraints[i++] != PFFB_SAMPLES);
	if(samples > 0)
	    constraints[i] = 1;
    }
#endif
    
    pw->chooseFBConfig(constraints);

    pw->open();

    int alpha_bits;
    pw->query(PFQWIN_ALPHA_BITS, &alpha_bits);
    printf("Alpha bits: %d\n", alpha_bits);
}

/***************************************************************************/
static void Usage(void)
{
    printf("USAGE: makeIBRimages file options\n"
	   "  -p    use the lowest LOD as a proxy\n"
	   "  -pf file  use specified file as a proxy\n"
	   "  -ps F  proxy scale (if not specified, the scale factor -s is used)\n"
	   "  -f S  name of the output files (with the path)\n"
	   "  -n N  number of images around the object (in 2D mode)\n"
	   "  -nv N number of uniformly distributed 3D views\n"
	   "  -rv S use rings of view specified by file S\n"
	   "  -s F  scale factor (helps to assure that the object fits in all images\n"
	   "  -o N  oversampling - use on machines with no antialiasing\n"
	   "  -l x y z w r g b  specify a light source (position and color)\n"
	   "                    Several sources can be entered.\n"
	   "  -pfb S  output pbf file S with a single pfIBRnode\n"
	   "  -W x y  texture size\n"
	   "  -O x y  window origin\n"
	   "  -nms    no multisampling - improves chances of getting visual with an alpha channel\n"
	   "  -ntex   skip image creation (e.g. when changing proxy)\n"
	   "\n"
	   "Example: \n  "
	   "makeIBRimages esprit.pfb -f /tmp/view -W 256 128 -n 32 -o 2 -s 1.4\n"
	   "\n"
	   "The program creates images (views) of the specified object from\n"
	   "a set of directions. The images are stored in a directory \n"
	   "specified using the option -f. \n\n"
	   "If a text file 'info' is present in the output directory a set \n"
	   "of 3D views is rendered. Each line of the info file contains two\n"
	   "values: the angle from the horizontal plane and how many views \n"
	   "are created for that angle. The images are then indexed by two\n"
	   "integer values that are appended to the name specified by the\n"
	   "option -f. The first value is the index of the ring of views and\n"
	   "the second one indexes the views within the ring.\n\n"
	   "If the info file is not present a set of N views (set by the\n"
	   "option -n) is computed around the object using horizontal\n"
	   "angle of 0. In this case only one index is appended to the image\n"
	   "name.\n\n"
	   "If option -pfb is specified the program outputs a pfb file in\n"
	   "the specified directory. The file contains a single pfIBRnode\n"
	   "that uses the created images. Note: before loading in perfly make\n"
	   "sure that PFPATH is set to the directory with all images.\n\n"
	   );   
}

/***************************************************************************/
static int
docmdline(int argc, char *argv[])
{
    int i, found;

    if(argc <= 1) {
	Usage();
	exit(-1);
    }

    i = 1;

    // count file names, if any
    while(i<argc && argv[i][0] != '-')
	i++;
    found = i-1;

    if(found == 0 && argc == 1) {
	Usage();
	exit(1);
    }

    /* process commmand line args */
    for (; i < argc; ++i) {
	if (!strcmp("-O", argv[i])) {
	    posx = atoi(argv[++i]);
	    posy = atoi(argv[++i]);
	}
	else if (!strcmp("-p", argv[i])) {
	    addProxy = 1;
	}
	else if (!strcmp("-nv", argv[i])) {
	    Shared->uniformViews = atoi(argv[++i]);
	}
	else if (!strcmp("-rv", argv[i])) {
	    infoFile = argv[++i];
	}
	else if (!strcmp("-pf", argv[i])) {
	    addProxy = 1;
	    proxyFile = argv[++i];
	}
	else if (!strcmp("-ps", argv[i])) {
	    proxyScale = atof(argv[++i]);
	}
	else if (!strcmp("-ts", argv[i])) {
	    addProxy = 1;
	    texShift[0] = atof(argv[++i]);
	    texShift[1] = atof(argv[++i]);
	}
	else if (!strcmp("-W", argv[i])) {
	    Shared->sizex = atoi(argv[++i]);
	    Shared->sizey = atoi(argv[++i]);
	}
	else if (!strcmp("-s", argv[i])) {
	    nodeScale = atof(argv[++i]);
	}
	else if (!strcmp("-n", argv[i])) {
	    Shared->numTex[0] = atoi(argv[++i]);
	}
	else if (!strcmp("-sk", argv[i])) {
	    Shared->skipTex = atoi(argv[++i]);
	}
	else if (!strcmp("-f", argv[i])) {
	    strcpy(Shared->outputName, argv[++i]);
	}
	else if (!strcmp("-o", argv[i])) {
	    Shared->samples = atoi(argv[++i]);
	}
	else if (!strcmp("-pfb", argv[i])) {
	    strcpy(Shared->fileName, argv[++i]);
	    saveFile = 1;
	}
	else if (!strcmp("-d", argv[i])) {
	    distance = atof(argv[++i]);
	}
	else if (!strcmp("-nms", argv[i])) {
	    Shared->useMultisampling = 0;;
	}
	else if (!strcmp("-ntex", argv[i])) {
	    skipDraw = 1;
	}
	else if (!strcmp("-l", argv[i])) {
	    if(numLights < MAX_LIGHTS)
	    {
		lightPos[numLights][0] = atof(argv[++i]);
		lightPos[numLights][1] = atof(argv[++i]);
		lightPos[numLights][2] = atof(argv[++i]);
		lightPos[numLights][3] = atof(argv[++i]);
		lightColor[numLights][0] = atof(argv[++i]);
		lightColor[numLights][1] = atof(argv[++i]);
		lightColor[numLights][2] = atof(argv[++i]);
		numLights++;
	    }
	    else
		i += 7;
	}
	else {
	    printf("\nUnknown option: %s\n\n", argv[i]);
 	    Usage();
	    exit(1);
	}
    }
	    
    return found;
}

static void parseGsets(pfNode *node, float scale,
		       pfIBRnode *IBRnode, pfGeoState *gstate)
{
    int i, j, vertcount;
    pfGeode *geode;
    pfVec3 *attrib;
    ushort *indexPtr;
    pfVec2 *tcoords;
    int *lengths;
    int type, nprims;
    pfMatrix    mat;
    pfMatrix4d  mat4d;
    int         createGeoSet = 1; // XXX user parameter?
    pfIBRtexture *IBRtex = IBRnode->getIBRtexture();

    // check for geode
    if ( node->isExactType(pfGeode::getClassType()) )
    {
	geode = (pfGeode *) node;
        int num_sets = geode->getNumGSets();

	for(i = 0; i < num_sets; i++)
        {
	    pfGeoSet *gset = geode->getGSet(i);
		
	    if (gset->isOfType(pfFlux::getClassType())) {
		gset = (pfGeoSet *)((pfFlux *) gset)->getCurData();
	    }

	    gset->getAttrLists(PFGS_COORD3, (void **)&attrib, &indexPtr);

	    if(indexPtr)  // don't bother to handle indexed data
            {
		//cout << "Index data ignored\n";
		continue;
	    }

	    // get number of vertices
	    type = gset->getPrimType();
	    nprims = gset->getNumPrims();

	    switch(type)
            {
	    case PFGS_TRIS:
		vertcount = 3 * nprims;
		// newgset is not needed, primitives are planar
		break;

	    case PFGS_QUADS:
		vertcount = 4 * nprims;
		// newgset is not needed, primitives are planar
		break;

	    case PFGS_TRISTRIPS:
	    case PFGS_TRIFANS:
		lengths = gset->getPrimLengths();

		vertcount = 0;
		for(j = 0; j<nprims; j++)
		    vertcount += lengths[j];
		
		if(createGeoSet)
		{
		    // we need to split the strips or fans into planar parts
		    int      newvertcount, v;
		    pfGeoSet *newgset = new pfGeoSet();
		    
		    newvertcount = 0;
		    for(j = 0; j<nprims; j++)
			// there are lengths[j] - 2 triangles
			newvertcount += 3 * (lengths[j] - 2);

		    pfVec3 *coords = (pfVec3 *)pfMalloc(sizeof(pfVec3)*newvertcount, _pfSharedArena);

		    if(type == PFGS_TRISTRIPS)
		    {
			pfVec3 *source, *dest;
			
			printf("Splitting a tristrip into planar parts\n");

			source = attrib;
			dest = coords;

			for(j = 0; j<nprims; j++)
			{
			    for(v = 0; v <= lengths[j]-4; v += 2)
			    {
				*(dest++) = source[0];
				*(dest++) = source[1];
				*(dest++) = source[2];

				*(dest++) = source[2];
				*(dest++) = source[1];
				*(dest++) = source[3];

				source += 2;
			    }

			    if(v <= lengths[j]-3)
			    {
				*(dest++) = source[0];
				*(dest++) = source[1];
				*(dest++) = source[2];
				source += 1;
			    }
			    source += 2;
			}
		    }
		    else
		    {
			// trifans
			pfVec3 *source, *dest;
			
			printf("Splitting a trifan into planar parts\n");

			source = attrib;
			dest = coords;

			for(j = 0; j<nprims; j++)
			{
			    for(v = 0; v < lengths[j]-2; v++)
			    {
				*(dest++) = source[0];
				*(dest++) = source[1];
				*(dest++) = source[2];

				source++;
			    }
			    source += 2;
			}
		    }

		    newgset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
		    newgset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, NULL, NULL);

		    //newgset->setPrimLengths(lengths);
		    newgset->setPrimType(PFGS_TRIS);
		    newgset->setNumPrims(newvertcount / 3);
		    
		    vertcount = newvertcount;
		    attrib = coords;
		    gset = newgset;
		}

		break;



	    case PFGS_FLAT_TRISTRIPS:
	    case PFGS_FLAT_TRIFANS:
	    case PFGS_POLYS:

		lengths = gset->getPrimLengths();

		vertcount = 0;
		for(j = 0; j<nprims; j++)
		    vertcount += lengths[j];
		// newgset is not needed, primitives are planar
		break;

	    case PFGS_POINTS:
	    case PFGS_LINES:
	    case PFGS_LINESTRIPS:
	    case PFGS_FLAT_LINESTRIPS:
	    default:
		//cout << "ignoring not facet data";
		break;
	    }


	    // scale each vertex 
	    if(scale != 1.0)
		for(j = 0; j<vertcount; j++)
		{
		    attrib[j][0] *= scale;
		    attrib[j][1] *= scale;
		    attrib[j][2] *= scale;
		}
	    
	    // add the geoset to IBRnode, set its geostate
	    gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, color, NULL);

	    // test if tex coords present
	    gset->getAttrLists(PFGS_TEXCOORD2, 
			       (void**)&tcoords, &indexPtr);
	    if(!tcoords) 
	    {
		tcoords = (pfVec2 *)pfMalloc(sizeof(pfVec2)*vertcount, _pfSharedArena);
		gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, 
			      tcoords, NULL);
	    }

	    if(IBRtex->getFlags(PFIBR_USE_REG_COMBINERS))
	    {
		// multi-texture coordinates don't have to be initialized, 
		// they have to be set to non-NULL

		gset->setMultiAttr(PFGS_TEXCOORD2, 1, PFGS_PER_VERTEX, 
				   tcoords, NULL);
		if(IBRtex->getFlags(PFIBR_3D_VIEWS))
		{
		    gset->setMultiAttr(PFGS_TEXCOORD2, 2, PFGS_PER_VERTEX, 
				       tcoords, NULL);
		    gset->setMultiAttr(PFGS_TEXCOORD2, 3, PFGS_PER_VERTEX, 
				       tcoords, NULL);
		}
	    }

	    gset->setGState(gstate);

	    IBRnode->addGSet(gset);
	}
    }
    else
    {   
	// XXX matrices not processed!!!
	if (node->isExactType (pfDCS::getClassType()) ||
	    node->isExactType (pfSCS::getClassType())) {

	    ((pfSCS *)node)->getMat(mat);
	 
	    printf("pfSCS or pfDCS in the proxy not supported!!!\n");
	}
	else 
	if (node->isExactType (pfDoubleDCS::getClassType()) ||
	    node->isExactType (pfDoubleSCS::getClassType())) {
	    pfMatrix4d mat4d;

	    ((pfDoubleSCS *)node)->getMat(mat4d);
	    PFCOPY_MAT(mat, mat4d);
	    
	    printf("pfDoubleSCS or pfDoubleDCS  in the proxy not supported!!!\n");
	}
	
	// descend if group
        if( node->isOfType(pfGroup::getClassType()) )
        {
            int num_children = ((pfGroup *)node)->getNumChildren();

	    if( node->isOfType(pfLOD::getClassType()) && num_children >= 2)
	    {
		// descend child num_children-1
		//printf("LODdown ");
		parseGsets(((pfGroup *)node)->getChild(num_children-1), scale, 
			   IBRnode, gstate);
	    }
	    else
		for(i = 0; i < num_children; i++)
		    parseGsets(((pfGroup *)node)->getChild(i), scale, 
			       IBRnode, gstate);
        }
    }
}

/**************************************************************************/
void AddGeoSets(pfIBRnode *node, pfIBRtexture *IBRtex, pfNode *root,
		float scale, pfGeoState *gstate)
{

    if(addProxy)
    {
	// parse 'root' and add all its geosets to 'node'
	// when encountering LOD, choose the coarsest one
	color = (pfVec4 *)pfMalloc(sizeof(pfVec4), _pfSharedArena);
	color->set(1.0, 1.0, 1.0, 1.0);

	parseGsets(root, scale, node, gstate);

	IBRtex->computeProxyTexCoords(node, &bsphere.center, 
				      aspect, 1.0 / bsphere.radius, &texShift);
    }
    else
    {
	// create a single geoset - a billboard
	pfGeoSet *gset = new pfGeoSet();
	
	pfVec3 *coords = (pfVec3 *)pfMalloc(sizeof(pfVec3)*4, _pfSharedArena);
	coords[0].set(-0.5, 0.0, 0);
	coords[1].set( 0.5, 0.0, 0);
	coords[2].set( 0.5, 0.0, aspect);
	coords[3].set(-0.5, 0.0, aspect);
	gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	
	pfVec4 *color = (pfVec4 *)pfMalloc(sizeof(pfVec4), _pfSharedArena);
	color->set(1.0, 1.0, 1.0, 1.0);
	gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, color, NULL);
	
	// fixed texture coordinates
	pfVec2 *tcoords = (pfVec2 *)pfMalloc(sizeof(pfVec2)*4, _pfSharedArena);
	tcoords[0].set( 0, 0);
	tcoords[1].set( 1, 0);
	tcoords[2].set( 1, 1);
	tcoords[3].set( 0, 1);
	gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
	if(IBRtex->getFlags(PFIBR_USE_REG_COMBINERS))
	{
	    gset->setMultiAttr(PFGS_TEXCOORD2, 1, PFGS_PER_VERTEX, tcoords, NULL);
	    if(IBRtex->getFlags(PFIBR_3D_VIEWS))
	    {
		gset->setMultiAttr(PFGS_TEXCOORD2, 2, PFGS_PER_VERTEX, tcoords, NULL);
		gset->setMultiAttr(PFGS_TEXCOORD2, 3, PFGS_PER_VERTEX, tcoords, NULL);
	    }
	}       
	
	int *lengths = (int *)new (sizeof(int)) pfMemory;
	lengths[0] = 4;
	gset->setPrimLengths(lengths);
	gset->setPrimType(PFGS_QUADS);
	gset->setNumPrims(1);
	
	gset->setGState(gstate);

	// add the geoset to pfIBRnode
	node->addGSet(gset);
    }
}

/***************************************************************************/
int main (int argc, char *argv[])
{
    int   found;
    char  name[2048];
    FILE  *fp;

    // Initialize Performer
    pfInit();
    
    // allocate shared before fork()'ing parallel processes 
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());

    // set default values
    strcpy(Shared->outputName, "./view");
    Shared->numTex[0] = 128;
    Shared->skipTex = 1;
    Shared->horAngle[0] = 0;
    Shared->numRings = 1;
    Shared->uniformViews = 0;

    texShift.set(0,0);

    Shared->samples = 1;
    
    // window and texture size
    Shared->sizex = 256;
    Shared->sizey = 256;

    Shared->useMultisampling = 0;;
	    
    found = docmdline(argc, argv);

    int x,y;
    pfGetScreenSize(-1, &x, &y);
    if(Shared->sizex > x) {
	Shared->sizex = x;
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		 "cannot use bigger texture than screen. "
		 "Reducing xsize to %d!\n", x);
    }
	
    if(Shared->sizey > y) {
	Shared->sizey = y;
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		 "cannot use bigger texture than screen. "
		 "Reducing ysize to %d!\n", y);
    }
    if(Shared->sizex *Shared->samples > x ||
       Shared->sizey *Shared->samples > y) {

	while(Shared->sizex *Shared->samples > x ||
	      Shared->sizey *Shared->samples > y)
	    Shared->samples--;

	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		 "cannot oversample to a bigger image than screen. "
		 "Reducing samples to %d!\n", Shared->samples);
    }

    // search for info file and set up rings
    strcpy(name, Shared->outputName);

    if(infoFile)
	strcpy(name, infoFile);
    else
    {
	// find the last /
	char *ptr = strrchr(name, '/');
	if(ptr == NULL)
	    strcpy(name, "info");
	else
	    strcpy(ptr+1, "info");
    }
  
    Shared->viewData = new pfDirData;

    if(Shared->uniformViews > 0)
    {
	if(Shared->uniformViews < 4)
	    Shared->uniformViews = 4;

	Shared->viewData->generateDirections(Shared->uniformViews, 
					     PFDD_3D_UNIFORM, NULL);
	Shared->viewData->getDirections(&Shared->numDirs, &Shared->viewDir);

	Shared->numRings = 0;

	Shared->numTex[0] = Shared->numDirs;
    }
    else if((fp = fopen(name, "r")) != NULL)
    {
	float ringData[PFDD_MAX_RINGS*2];

	// read information about the number of textures and horizontal
	// angle of each ring of views (from the info file present in the
	// image directory)
	int i = 0;
	while(!feof(fp)) 
	{
	    fscanf(fp,"%g %d\n", &Shared->horAngle[i], &Shared->numTex[i]);
	    ringData[i*2] = Shared->numTex[i];
	    ringData[i*2+1] = Shared->horAngle[i];
	    i++;
	}
	Shared->numRings = i;

	Shared->viewData->generateDirections(i, PFDD_RINGS_OF_VIEWS, ringData);
	Shared->viewData->getDirections(&Shared->numDirs, &Shared->viewDir);
    }
    else
    {
        printf("File %s specifying the rings of views does not exist!\n"
	       "Create the file or specify -nv NumViews.\n", name);
	exit(0);
    }
#if 0
    // find the last /
    char *ptr = strrchr(name, '/');
    if(ptr == NULL)
	strcpy(name, "info");
    else
	strcpy(ptr+1, "info");
  
    if((fp = fopen(name, "r")) != NULL)
    {
	// read information about the number of textures and horizontal
	// angle of each ring of views (from the info file present in the
	// image directory)
	int i = 0;
	while(!feof(fp)) 
	{
	    fscanf(fp,"%g %d\n", &Shared->horAngle[i], &Shared->numTex[i]);
	    i++;
	}
	Shared->numRings = i;
    }
#endif

    pfMultiprocess( PFMP_APPCULLDRAW );
    
     // Load all loader DSO's before pfConfig() forks 
    for (int arg = 1; arg <= found; arg++)
	pfdInitConverter(argv[arg]);

    if(saveFile)
	pfdInitConverter(Shared->fileName);
	
    if(proxyFile)
	pfdInitConverter(proxyFile);
	
    // initiate multi-processing mode set in pfMultiprocess call 
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    //
    pfConfig();			
    
    // Append to Performer search path, PFPATH, files in 
    //	    /usr/share/Performer/data */
    pfFilePath(".:/usr/share/Performer/data");
    
    // create a new pfScene
    pfScene *scene = new pfScene;

    pfSCS    *scaleSCS;

    if(nodeScale != 1.0) {
	pfMatrix  *scaleMat = new pfMatrix;
	
	scaleMat->makeScale(nodeScale, nodeScale, nodeScale);
	scaleSCS = new pfSCS(*scaleMat);
	scene->addChild(scaleSCS);
    }

    pfNode *root = NULL;
    pfNode *proxyNode = NULL;

    for (int i=1 ; i<=found; i++)
	if ((root = pfdLoadFile(argv[i])) != NULL)
	    if(nodeScale != 1.0)
		scaleSCS->addChild(root);
	    else 
		scene->addChild(root);

    if(proxyScale == 0.0)
	proxyScale = nodeScale;

    if(proxyFile)
    {
	pfNode *node;

	node = pfdLoadFile(proxyFile);

	// use the same scale as for the object
	proxyNode = node;
    }
	

    if(numLights <= 0)
	// Create a default pfLightSource and attach it to scene
	scene->addChild(new pfLightSource);
    else
    {
	// add light sources 
	for (int i = 0; i < numLights; i++)
	{
	    pfLightSource *light = new pfLightSource();

	    if(lightPos[i][3] == 0)
		// infinite light source
		lightPos[i].normalize();

	    light->setPos(lightPos[i][0],
			  lightPos[i][1],
			  lightPos[i][2],
			  lightPos[i][3]);

	    /* set light source color */
	    light->setColor(PFLT_DIFFUSE,
			    lightColor[i][0], 
			    lightColor[i][1],
			    lightColor[i][2]);
#if 0
	    light->setColor(PFLT_SPECULAR,
			    lightColor[i][0], 
			    lightColor[i][1],
			    lightColor[i][2]);

	    light->setColor(PFLT_AMBIENT,
			    lightColor[i][0]*0.2, 
			    lightColor[i][1]*0.2,
			    lightColor[i][2]*0.2);
#endif
	    /* add lamp to scene */
	    scene->addChild(light);
	}
    }
    
    
    // Configure and open GL window
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setName("OpenGL Performer");
    pw->setOriginSize(posx, posy, 
		      Shared->sizex * Shared->samples, 
		      Shared->sizey * Shared->samples);
    pw->setMode(PFWIN_NOBORDER, 1);
    pw->setConfigFunc(OpenPipeWin);
    pw->config();
    
    // Create and configure a pfChannel.
    pfChannel *chan = new pfChannel(p);

    // have to use callback, because the image can be captured only in draw
    // process
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);
    chan->setScene(scene);
    
    // determine extent of scene's geometry
    if(root)
	root->getBound(&bsphere); // may not be correct if more than one node set

    if(proxyNode)
	proxyNode->getBound(&proxybsphere); 
    else
	proxybsphere = bsphere;

    // set up the projection
    aspect = (float)Shared->sizey / (float)Shared->sizex;

    if(distance == 0)
    {
	// orthographic projection (the default)
	chan->makeOrtho(-bsphere.radius, bsphere.radius,
			-bsphere.radius * aspect, bsphere.radius * aspect);
	chan->setNearFar(-bsphere.radius * nodeScale, 
			 bsphere.radius * nodeScale);
    }
    else
    {
	// perspective projection

	// set so that frustum spans -radius,+radius at bsphere center
	chan->setNearFar(distance, distance + bsphere.radius);
	chan->makePersp(-bsphere.radius, bsphere.radius,
			-bsphere.radius * aspect, bsphere.radius * aspect);

	// now update the near plane
	if(distance - bsphere.radius < 0.1)
	    chan->setNearFar(0.1, distance + bsphere.radius);
	else
	    chan->setNearFar(distance - bsphere.radius,
			     distance + bsphere.radius);
    }

    // create the texture that is used to store the images
    Shared->tex = new pfTexture;
    Shared->tex->setFormat(PFTEX_IMAGE_FORMAT, GL_RGBA);
    Shared->tex->setFormat(PFTEX_INTERNAL_FORMAT, PFTEX_RGBA_8);
    Shared->tex->setFormat(PFTEX_EXTERNAL_FORMAT, GL_UNSIGNED_BYTE);

    Shared->texImage = (char *)new(Shared->sizex * Shared->sizey * 4) pfMemory;
    if(Shared->samples > 1)
	Shared->oversampledImage = 
	    (char *)new(Shared->sizex * Shared->sizey * 4 *
			Shared->samples * Shared->samples) pfMemory;
    
    Shared->tex->setImage((unsigned int*)Shared->texImage, 4, 
			  Shared->sizex, Shared->sizey, 1);
    

    float angle1, angle2, astep1;
    int   index, ring;

    angle1 = 0;
    angle2 = Shared->horAngle[0];

    astep1 = 360.0 * Shared->skipTex / (float)Shared->numTex[0];

    index = 0;
    ring = 0;

    Shared->finished = skipDraw || root == NULL;

    pfCoord view;

    if(distance == 0)
	view.xyz = bsphere.center;

    while (!Shared->finished)
    {
	// Go to sleep until next frame time.
	pfSync();		

	view.xyz = bsphere.center - distance * Shared->viewDir[index];

	// angle from horizon (assuming viewDir's are normalized)
	angle2 = asinf(-Shared->viewDir[index][2]) * 180.0 / M_PI;

	float len2D = fsqrt(Shared->viewDir[index][0]*Shared->viewDir[index][0] +
			    Shared->viewDir[index][1]*Shared->viewDir[index][1]);
	if(len2D < 0.000001)
	    angle1 = 0;
	else 
	{
	    angle1 = acosf(Shared->viewDir[index][1] / len2D) * 180.0 / M_PI;
	    
	    if(Shared->viewDir[index][0] > 0)
		angle1 = 360.0 - angle1; 
	}
#if 0
	if(distance > 0) {
	    float cp = cosf(PF_DEG2RAD(angle2));
	    view.xyz[PF_X] = bsphere.center[0] 
		+ distance * sinf(-PF_DEG2RAD(180+angle1)) * cp;
	    view.xyz[PF_Y] = bsphere.center[1] 
		+ distance * cosf(-PF_DEG2RAD(180+angle1)) * cp;
	    view.xyz[PF_Z] = bsphere.center[2]
		+ distance * sinf( PF_DEG2RAD(angle2));
	}
#endif

	view.hpr.set(angle1, -angle2, 0);

	chan->setView(view.xyz, view.hpr);
	
	// Initiate cull/draw for this frame.
	pfFrame();

	// advance the indices and check if not finished
	index++;
	if(index >= Shared->numTex[ring]) {
	    ring ++;

	    if(ring >= Shared->numRings)
		ring = 0;  // let draw finish
	}
    }

    if(saveFile)
    {
	////////////////////////////////////////////////////////////////////
	// after all images are saved create a single pfIBRnode that uses
	// them and save it into a file
	
	// create a new pfIBRtexture
	pfIBRtexture *IBRtex = new pfIBRtexture;
	
#if 1
	IBRtex->setFlags(PFIBR_USE_2D_TEXTURES, 1);
#else
	IBRtex->setFlags(PFIBR_MIPMAP_3DTEXTURES, 1);
#endif

	if(Shared->uniformViews > 0) 
	{
	    int i;
	    pfTexture **texs;

	    texs = (pfTexture **)pfMalloc(sizeof(pfTexture *) * 
					  Shared->numDirs,
					  _pfSharedArena);

	    for(i=0; i<Shared->numDirs; i++)
	    {
		sprintf(name, "%s_%03d.rgb", Shared->outputName, i);
		
		texs[i] = new pfTexture;
		texs[i]->setFormat(PFTEX_INTERNAL_FORMAT, PFTEX_RGBA_8);
		texs[i]->loadFile(name);
	    }

	    IBRtex->setIBRdirections(Shared->viewDir, Shared->numDirs);
	    IBRtex->setIBRTexture(texs, Shared->numDirs);

	    IBRtex->setFlags(PFIBR_3D_VIEWS, 1);
	}
	else if(Shared->numRings > 1)
	{
	    sprintf(name, "%s_%%02d_%%03d.rgb", Shared->outputName);
	    //printf("name=%s\n", name);
	    IBRtex->setFlags(PFIBR_3D_VIEWS, 1);
	    
	    IBRtex->loadIBRTexture(name, Shared->numTex[0], Shared->skipTex);
	}
	else {
	    sprintf(name, "%s_%%03d.rgb", Shared->outputName);
	    IBRtex->loadIBRTexture(name, Shared->numTex[0], Shared->skipTex);
	}

	if(addProxy)
	    IBRtex->setFlags(PFIBR_USE_PROXY, 1);
	
	// create a new pfIBRnode (a child of pfBillboard)
	pfIBRnode *node = new pfIBRnode;
	node->setIBRtexture(IBRtex);
	node->setAngles(0, 0, 0.0);
	
	//if(IBRtex->getFlags(PFIBR_3D_VIEWS))
	//    node->setMode(PFBB_ROT, PFBB_POINT_ROT_WORLD);
	//else
	    node->setMode(PFBB_ROT, PFBB_AXIAL_ROT);
	
	pfVec3 *pos = new pfVec3;

	if(addProxy)
	    *pos = proxybsphere.center;
	else
	    pos->set(0,0,0);
	node->setPos(0, *pos);

	// create geostate
	pfGeoState *gstate = new pfGeoState;
	gstate->setMode(PFSTATE_ENTEXTURE, PF_ON);
	
	if(IBRtex->getFlags(PFIBR_USE_REG_COMBINERS))
	{
	    gstate->setMultiMode(PFSTATE_ENTEXTURE, 1, PF_ON);
	    
	    if(IBRtex->getFlags(PFIBR_3D_VIEWS))
	    {
		gstate->setMultiMode(PFSTATE_ENTEXTURE, 2, PF_ON);
		gstate->setMultiMode(PFSTATE_ENTEXTURE, 3, PF_ON);
	    }
	}   
	gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
	gstate->setMode(PFSTATE_ALPHAFUNC, PFAF_GREATER);
	gstate->setVal(PFSTATE_ALPHAREF, 0.03);
	gstate->setMode(PFSTATE_TRANSPARENCY, PF_ON);

	if(!IBRtex->getFlags(PFIBR_USE_2D_TEXTURES))
	    gstate->setMode(PFSTATE_ENTEXMAT, PF_ON);
	
	gstate->setAttr(PFSTATE_TEXTURE, IBRtex->getDefaultTexture());
	
	
	pfTexEnv *tenv = new(_pfSharedArena) pfTexEnv;
	tenv->setMode(PFTE_MODULATE);
	gstate->setAttr(PFSTATE_TEXENV, (void *)tenv);

	if(proxyNode)
	    AddGeoSets(node, IBRtex, proxyNode, proxyScale, gstate);
	else
	    AddGeoSets(node, IBRtex, root, proxyScale, gstate);
		
	pfdStoreFile(node, Shared->fileName);
    }

    // Terminate parallel processes and exit
    pfExit();
    return 0;
}

// these indices are used by the draw callback
static int tex = 0;
static int ring = 0;
static int *colorBuf = NULL;

/*ARGSUSED*/
static void DrawChannel (pfChannel *channel, void *)
{
    char       name[2048];

    if(Shared->finished)
	return;

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pfDraw();

    // read the buffer, downsample if necessary
    if(Shared->samples == 1)
	glReadPixels(0,0, Shared->sizex, Shared->sizey, 
		     GL_RGBA, GL_UNSIGNED_BYTE, Shared->texImage);
    else {
	unsigned char *ptrSrc, *ptrDst;
	int           *ptrBuf;
	int           x, y, i, j;
	float         invSamples;

	glReadPixels(0,0, 
		     Shared->sizex * Shared->samples, 
		     Shared->sizey * Shared->samples, 
		     GL_RGBA, GL_UNSIGNED_BYTE,
		     Shared->oversampledImage);

	// use a line buffer into which the data is accumulated
	if(colorBuf == NULL)
	    colorBuf = (int *)new (sizeof(int)*4*Shared->sizex) pfMemory;

	// downsample into Shared->texImage

	ptrBuf = colorBuf;

	// initialize the buffer
	for(x=0; x<Shared->sizex * 4; x++)
	    *(ptrBuf++) = 0;

	ptrSrc = (unsigned char*)Shared->oversampledImage;
	ptrDst = (unsigned char*)Shared->texImage;
	j =0;
	invSamples = 1.0 / (Shared->samples * Shared->samples);

	for(y=0; y<Shared->sizey*Shared->samples; y++)
	{
	    ptrBuf = colorBuf;
	
	    // set the value to the buffer
	    for(x=0; x<Shared->sizex; x++) 
	    {
		for(i = 0; i<Shared->samples; i++)
		{
		    ptrBuf[0] += (int)*(ptrSrc++);
		    ptrBuf[1] += (int)*(ptrSrc++);
		    ptrBuf[2] += (int)*(ptrSrc++);
		    ptrBuf[3] += (int)*(ptrSrc++);
		}

		ptrBuf += 4;
	    }

	    j++;
	    if(j == Shared->samples) {
		// divide the values in colorBuf by samples*samples and
		// copy to texImage
		ptrBuf = colorBuf;

		for(x=Shared->sizex * 4; x > 0; x--) 
		{
		   *(ptrDst++) = (unsigned char)((float)*ptrBuf * invSamples);

		   // reinitialize
		   *(ptrBuf++) = 0;
		}

		j = 0;
	    }
	}
    }

    // advance indices
    if(Shared->numRings > 1) {
	sprintf(name, "%s_%02d_%03d.rgb", Shared->outputName, ring, tex);
	tex++;
	if(tex >= Shared->numTex[ring]) {
	    tex = 0;
	    ring ++;
	    
	    if(ring >= Shared->numRings)
		Shared->finished = 1;
	}
    }
    else {
	sprintf(name, "%s_%03d.rgb", Shared->outputName, tex);
	tex++;
	
	if(tex >= Shared->numTex[0])
	    Shared->finished = 1;
    }

    printf("Saving image %s\n", name);

    Shared->tex->saveFile(name);

    return;
}
