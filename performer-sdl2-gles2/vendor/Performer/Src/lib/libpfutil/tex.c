/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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
 * tex.c
 * --------------
 * $Revision: 1.98 $
 * $Date: 2002/08/30 00:22:06 $
 *
 *	This is a special tex.c with minor changes for tracking IMPACT
 *	memory usage and getting improved memory usage on IMPACT.
 *	Minor changes have been made to pfuMakeSceneTexList() and
 *	pfuMakeTexList() that will affect the order of textures in the
 *	returned list.
 *	It is highly recommended that this be used in place of tex.c on IMPACT
 *	and it should have no effect on other graphics patforms.
 */

#include <string.h>
#ifndef WIN32
#include <values.h>
#endif
#include <stdlib.h>
#include <ctype.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>

#define DIM_1D 		0
#define DIM_PROXY_1D	1
#define DIM_2D		2
#define DIM_PROXY_2D	3
#define DIM_3D		4
#define DIM_PROXY_3D	5
#define DIM_4D		6
#define DIM_PROXY_4D	7
#define MAXLEVEL    14

/* texture level identification */
typedef struct __TexLevIDRec
	{
    GLsizei width, height, depth, extent; /* sizes declared for level */
    GLint brd_data;		/* border data declared by app */
    GLint numcomp;		/* #components in internal format [1:4]=>[0:3] */
    GLint compdepth;		/* component depth (bits) internal fmt [4/8/12]=>[0:2]*/
    GLint hx2;				/* Horizontal/vertical x2 mode */
    GLint db_possible;	/* is texture-double-buffering possible */
    GLenum internalformat;	/* internal format assigned by TM */
    } __TexLevID;

/* texture level descriptor */
typedef struct __TexLevInfoRec
	{
    struct __TexLevInfoRec *packed;	/* common block for packed case */
    __TexLevID id; 						/* identification data */
    GLsizei numpage, totpage;			/* #mipmap pages and log */
    GLsizei brd_page;					/* offset to border page(s) (-1=>none) */
    GLint lod;								/* LOD level number */
    GLint prohibited;					/* prohibited level */
    } __TexLevInfo;

/* texture descriptor */
typedef struct __TextureInfoRec
	{
    __TexLevInfo lev[MAXLEVEL];	/* 14 possible levels */
    __TexLevInfo *packed;			/* common block for packed case */
    __TexLevID id; 					/* identification data */
    GLsizei numpage;					/* total # tram pages for texture */
    GLint dim;							/* texture dimension [1D:4D]=>[0:3] */
    GLint vx2;							/* Horizontal/vertical x2 mode */
    GLint mm_enab; 					/* enable mipmapping */
    } __TextureInfo;

static GLenum format_enums[4][3] = {
{GL_LUMINANCE4_EXT, 			GL_LUMINANCE8_EXT, 		 GL_LUMINANCE12_EXT},
{GL_LUMINANCE4_ALPHA4_EXT, GL_LUMINANCE8_ALPHA8_EXT,GL_LUMINANCE12_ALPHA12_EXT},
{GL_RGB4_EXT, 					GL_RGB8_EXT, 				 GL_RGB12_EXT},
{GL_RGBA4_EXT, 				GL_RGBA8_EXT, 				 GL_RGBA12_EXT}
};

static int CalculateNumPages( pfTexture*, int, int, int, int );
static void total_pages( __TextureInfo* );
static void pages_per_level( __TextureInfo*, int );
static void set_internalformat( __TextureInfo*, int );

static pfList		**texList = NULL;

	/*------------------------------------------------------*/

PFUDLLEXPORT pfTexture*
pfuNewSharedTex(const char *filename, void *arena)
{
    int	i;
    static pfTexture *ntex=NULL;    
    pfTexture	*tex;

    if (texList == NULL)
    {
    	if (!(texList = (pfList **) pfuFindUtilDPData(PFU_TEXLIST_DATA_DPID)))
    	{
	    pfNotify(PFNFY_FATAL, PFNFY_SYSERR, 
	    	"pfuNewSharedTex() Could not attach to libpfutil pfDataPool. "
		"Call pfuInitUtil() first.");
	    return NULL;
    	}
    	*texList = pfNewList(sizeof(pfTexture*), 256, pfGetSharedArena());
    }

    for (i=0; i<pfGetNum(*texList); i++)
    {
	const char *name;
	tex = pfGet(*texList, i);
	name = pfGetTexName(tex);

	if (name && strstr(name, filename))
	    return tex;
    }

    if (!ntex)
	ntex = pfNewTex(arena);
#if 0
    if (pfLoadTexFile(ntex, (char *)filename) > 0)
    {
	tex = pfNewTex(arena);
	pfCopy(tex, ntex);
	pfAdd(*texList, tex);
	pfRef (tex);
	return tex;
    }
#else
    if (pfLoadTexFile(ntex, (char *)filename) > 0)
    {
	tex = ntex;
	pfAdd(*texList, tex);
	pfRef(tex);

	ntex = NULL;
	return tex;
    }
#endif
	
    return NULL;
}

PFUDLLEXPORT pfList*
pfuGetSharedTexList(void)
{
    if (texList == NULL)
    {
    	if (!(texList = (pfList **) pfuFindUtilDPData(PFU_TEXLIST_DATA_DPID)))
    	{
	    pfNotify(PFNFY_FATAL, PFNFY_SYSERR, 
	    	"pfuGetSharedTexList() Could not attach to libpfutil pfDataPool. "
		"Call pfuInitUtil() first.");
	    return NULL;
    	}
    	*texList = pfNewList(sizeof(pfTexture*), 256, pfGetSharedArena());
    }

    return *texList;
}

/*
 *	processGeoSet() -- add textures referenced by geoset to list
 */

static void
processGeoSet (pfGeoSet *geoset, pfList *list)
{
    pfGeoState	*geostate;
    pfTexture	*mainTex;

    /* validate argument */
    if (geoset == NULL || (geostate = pfGetGSetGState(geoset)) == NULL)
	return;

    /* add main texture (if defined) to list */
    if (((mainTex = pfGetGStateAttr(geostate, PFSTATE_TEXTURE)) != NULL)&&
	((pfGetGStateInherit(geostate)&PFSTATE_TEXTURE) == 0))
	if (pfSearch(list, mainTex) < 0)
	    pfAdd(list, mainTex);
}

/*
 *	makeTexListAux() -- recursively locate pfTextures in scene graph
 */

static void
makeTexListAux(pfNode *node, pfList *list)
{
    int		numChildren;
    int		numGeoSets;
    int		child;

    /* validate arguments */
    if (node == NULL)
	return;

    /* perform class-dependent node processing */
	
    if (pfIsOfType(node, pfGetGeodeClassType()))
    {
	/* process geode-class nodes by inspection */
	numGeoSets = pfGetNumGSets((pfGeode *)node);
	for (child = 0; child < numGeoSets; child++)
	    processGeoSet(pfGetGSet((pfGeode *)node, child), list);
    }
    if (pfIsOfType(node, pfGetIBRnodeClassType()))
    {
	/* process IBRtexture associated with this IBRnode */
	int          i, n;
	pfTexture    **textures;
	pfIBRtexture *IBRtex = pfGetIBRnodeIBRtexture((pfIBRnode *)node);

	pfGetIBRtextureIBRTextures(IBRtex, &textures, &n);
	
	for (i = 0; i < n; i++)
	    if (pfSearch(list, textures[i]) < 0)
		pfAdd(list, textures[i]);
    }
    else if (pfIsOfType(node, pfGetGroupClassType()))
    {
	/* process group-class nodes by recursion */
	numChildren = pfGetNumChildren((pfGroup *)node);
	for (child = 0; child < numChildren; child++)
	    makeTexListAux(pfGetChild((pfGroup *)node, child), list);
    }
    /* NOTE: lightpoint and lightsource nodes are skipped */
}

void
sortTextureList(
	pfList *list,
	pfList *new_list,
	int size,
	int this_num_components )
{
	int i;
	unsigned int *image;     /* pointer to texture image */
	int num_components; /* 1=I, 2=IA, 3=RGB, 4=RGBA */
	int ns = 0;     /* texture size in 's' dimension */
	int nt = 0;     /* texture size in 't' dimension */
	int nr = 0;     /* texture size in 'r' dimension */

	for ( i=0; i<size; i++ )
	  {
		pfTexture* tex = pfGet( list, i );

/* get details of texture's in-memory representation */
		pfGetTexImage(tex, &image, &num_components, &ns, &nt, &nr);

		if ( num_components == this_num_components )
			pfAdd( new_list, tex );
	  }
}

PFUDLLEXPORT pfList*
pfuMakeSceneTexList(pfScene *scene)
{
    pfList	*list	= NULL;
    pfGeoState	*state	= NULL;
    pfTexture	*tex	= NULL;
    pfList *nlist;
    int num_tex;

    /* validate arguments */
    if (scene == NULL)
	return NULL;

    /* initialize an empty texture list */
    list = pfNewList(sizeof(pfTexture*), 64, pfGetSharedArena());

    /* do the actual work */
    makeTexListAux((pfNode*)scene, list);

    /* add scene geostate's texture to list */
    if ((state = pfGetSceneGState(scene)) != NULL)
    {
	tex = pfGetGStateAttr(state, PFSTATE_TEXTURE);

	if ((tex != NULL) && (pfSearch(list, tex) < 0))
	    pfAdd(list, tex);
    }

    /* initialize an empty texture list */
    num_tex = pfGetNum( list );
    nlist = pfNewList(sizeof(pfTexture*), num_tex, pfGetSharedArena());

    sortTextureList( list, nlist, num_tex, 4 );
    sortTextureList( list, nlist, num_tex, 3 );
    sortTextureList( list, nlist, num_tex, 2 );
    sortTextureList( list, nlist, num_tex, 1 );

    pfDelete(list);

    return nlist;
}

/*
 *	pfuMakeTexList() -- find all textures in a scene graph
 */

PFUDLLEXPORT pfList*
pfuMakeTexList (pfNode *node)
{
	pfList *list;
	pfList *nlist;
	int num_tex;

	/* validate arguments */
	if (node == NULL)
		return NULL;

   /* initialize an empty texture list */
	list = pfNewList(sizeof(pfTexture*), 64, pfGetSharedArena());

   /* do the actual work */
	makeTexListAux(node, list);

   /* initialize an empty texture list */
	num_tex = pfGetNum( list );
	nlist = pfNewList(sizeof(pfTexture*), num_tex, pfGetSharedArena());

	sortTextureList( list, nlist, num_tex, 4 );
	sortTextureList( list, nlist, num_tex, 3 );
	sortTextureList( list, nlist, num_tex, 2 );
	sortTextureList( list, nlist, num_tex, 1 );

	pfDelete(list);

	return nlist;
}


/*
 *	showTex() -- display a texture in the center of the screen
 *
 *	  NOTE: You'd better be calling this from a draw-process!
 */

static void
showTex(pfWindow *w, pfTexture *texture, int ns, int nt)
{
    int	 xs;		/*  window size in  x  dimension */
    int	 ys;		/*  window size in  y  dimension */
    float	 xs2,  ys2, ns2, nt2;
    pfVec2	 v;
    static pfVec4 clr = {0.0f, 0.0f, 0.0f, 0.0f};

    static	const pfVec2 t[] =
	{{0.0, 0.0,},
	 {1.0, 0.0,},
	 {1.0, 1.0,},
	 {0.0, 1.0,}};

    pfPushState();
    pfBasicState();
    pfEnable(PFEN_TEXTURE);

    pfGetWinSize(w, &xs, &ys);

    /* clamp texture display size to window size */
    if (ns > xs)
	ns = 0.9*xs;
    if (nt > ys)
	nt = 0.9*ys;

    xs2 = xs/2.0f;
    ys2 = ys/2.0f;
    ns2 = ns/2.0f;
    nt2 = nt/2.0f;
    
    pfPushIdentMatrix();
    pfApplyTex(texture);

    /* background will be black */
    {
    GLint mm;
    glGetIntegerv(GL_MATRIX_MODE, &mm);
    if (mm != GL_PROJECTION)
	glMatrixMode(GL_PROJECTION);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glLoadIdentity(); 
    glOrtho(0.0f, xs, 0.0f, ys, -1.0f, 1.0f);
    glViewport(0, 0, xs, ys);
    pfClear(PFCL_COLOR, clr);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
    glTexCoord2fv(t[0]);
    PFSET_VEC2(v, xs2 - ns2, ys2 - nt2);
    glVertex2fv(v);
    glTexCoord2fv(t[1]);
    PFSET_VEC2(v, xs2 + ns2, ys2 - nt2);
    glVertex2fv(v);
    glTexCoord2fv(t[2]);
    PFSET_VEC2(v, xs2 + ns2, ys2 + nt2);
    glVertex2fv(v);
    glTexCoord2fv(t[3]);
    PFSET_VEC2(v, xs2 - ns2, ys2 + nt2);
    glVertex2fv(v);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    if (mm != GL_PROJECTION)
            glMatrixMode(mm);
    }

    pfSwapWinBuffers(w);

    pfPopMatrix();
    pfPopState();
}

/*
 *	powerOfTwo -- return least power of 2 >= argument
 */

static int
powerOfTwo (int arg)
{
    int	pot = 1;

    while (pot < arg)
	pot = pot << 1;

    return pot;
}

/*
 *	pfuGetTexSize() -- return memory requirements of a texture (in bytes)
 */

PFUDLLEXPORT int
pfuGetTexSize (pfTexture *texture)
{
    unsigned int 	*image;		/* pointer to texture image */
    int	components;	/* 1=I, 2=IA, 3=RGB, 4=RGBA */
    int	ns = 0;		/* texture size in 's' dimension */
    int	nt = 0;		/* texture size in 't' dimension */
    int	nr = 0;		/* texture size in 'r' dimension */
    int	format;
    int	bytesPerTexel;
    int	filter;
    int	size;

    /* validate argument */
    if (texture == NULL)
	return 0;

    /* get details of texture's in-memory representation */
    pfGetTexImage(texture, &image, &components, &ns, &nt, &nr);
    if (nr < 1) nr = 1;

    /* decode storage "depth" from internal format */
    format = pfGetTexFormat(texture, PFTEX_INTERNAL_FORMAT);
    switch (format) {
    case PFTEX_I_8:
    case PFTEX_I_12A_4:
    case PFTEX_IA_8:
    case PFTEX_RGB_5:
    case PFTEX_RGB5_A1:
    case PFTEX_RGBA_4:
    case PFTEX_I_16:
        bytesPerTexel = 2;
        break;
    case PFTEX_RGB_12:
	bytesPerTexel = 4.5;
	break;
    case PFTEX_RGBA_12:
	bytesPerTexel = 6;
	break;
    case PFTEX_IA_12:
    case PFTEX_RGBA_8:
    default:
        bytesPerTexel = 4;
        break;
    }

    if (ns == 0 && nt == 0)
	size = 0;
    else
    {
	/* adjust dimensions to reflect power-of-two requirement */
	ns = powerOfTwo(ns);
	nt = powerOfTwo(nt);

	/* MIP-map textures have an extra storage requirement */
	filter = pfGetTexFilter(texture, PFTEX_MINFILTER);
	if (filter == PFTEX_MIPMAP          || 
	    filter == PFTEX_MIPMAP_LINEAR   ||
	    filter == PFTEX_MIPMAP_BILINEAR || 
	    filter == PFTEX_MIPMAP_TRILINEAR )
	    size = (ns*nt*nr) * bytesPerTexel * 4.0f/3.0f + 4;
	else
	    size = (ns*nt*nr) * bytesPerTexel;
    }

    return size;
}

/*
 *	pfuDownloadTexList() -- download each texture in list
 *
 *	  NOTE: You'd better be calling this from a draw-process!
 */

PFUDLLEXPORT void
pfuDownloadTexList (pfList *list, int style)
{
    pfWindow	*w;
    unsigned int 	*image;		/* pointer to texture image */
    int		components;	/* 1=I, 2=IA, 3=RGB, 4=RGBA */
    int		ns = 0;		/* texture size in 's' dimension */
    int		nt = 0;		/* texture size in 't' dimension */
    int		nr = 0;		/* texture size in 'r' dimension (ignored) */
    int		i;
    int		n;
    double	listStartTime;
    double	listElapsedTime;
    int		textureSize = -1;
	 const GLubyte *machine;
	 int impact=0;
	 int numtram=0;

    /* validate argument */
    if (list == NULL)
	return;

    pfQuerySys(PFQSYS_TEXTURE_MEMORY_BYTES, &textureSize);

	machine = glGetString( GL_RENDERER );
	impact = !strncmp( machine, "IMPACT", 6 );

	if ( impact )
		numtram = atoi( &machine[11] );
    
    pfNotify(PFNFY_INFO, PFNFY_PRINT, "pfuDownloadTexList texture processing");

    /* process each texture in list */
    listStartTime = pfGetTime();
    for (i = 0, n = pfGetNum(list); i < n; i++)
    {
	pfTexture	*texture	= pfGet(list, i);
	pfTexture	*detail;
	int		 bytes;
	int		 ss;
	int		 st;
	double		 startTime;
	double		 elapsedTime;
	static int	 total		= 0;

	/* get details of texture's in-memory representation */
	pfGetTexImage(texture, &image, &components, &ns, &nt, &nr);
	if (nr < 1) nr = 1;

	/* print statistics */
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "  Loading texture:");
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    File name           = %s", pfGetTexName(texture));
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Image size (SxTxR)  = %dx%dx%d", ns, nt, nr);
	ss = powerOfTwo(ns);
	st = powerOfTwo(nt);
	if (ns != ss || nt != st)
	    pfNotify(PFNFY_INFO, PFNFY_MORE, 
		"    Scaled up size (SxTxR) = %dx%dx%d", ss, st, nr);

	/* accumulate total texture size defined during execution */
	bytes	= pfuGetTexSize(texture);
	total += bytes;

	if ( (detail=pfGetTexDetailTex( texture )) )
	{
	    pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Texture is detailed with %s", pfGetTexName(detail));
	    pfGetTexImage(detail, &image, &components, &ns, &nt, &nr);
	    ss = powerOfTwo(ns);
	    st = powerOfTwo(nt);
	    total += pfuGetTexSize(detail);
	}

	if ( impact )
		if(CalculateNumPages( texture, numtram, components, ns, nt ))
		{
		    pfNotify(PFNFY_WARN, PFNFY_PRINT,
			     "pfuDownloadTexList(): CalculateNumPages failed "
			     "for texture %d of %d; skipping", i, n);
		    continue;
		}
	 else
	    pfNotify(PFNFY_INFO, PFNFY_MORE, 
		"    Download size       = %8d bytes", bytes);

	/* download texture to graphics hardware */
	startTime = pfGetTime();
	switch (style)
	{
	case PFUTEX_DEFINE:
		 pfFormatTex(texture);
		 break;

	case PFUTEX_SHOW:
		 w = pfGetCurWin();
		 showTex(w, texture, ns, nt);
		 break;

	/*case PFUTEX_APPLY:*/
	default:
		 pfApplyTex(texture);
		 break;
	}
	elapsedTime = pfGetTime() - startTime;

	if (elapsedTime > 0.0f)
	{
	    pfNotify(PFNFY_INFO, PFNFY_MORE, 
		"    Download time       =%8.3f ms", 1000.0f*elapsedTime);
	    pfNotify(PFNFY_INFO, PFNFY_MORE, 
		"    Download rate       =%8.3f MB/sec", bytes/(1024*1024*elapsedTime));
	}

	pfNotify(PFNFY_INFO, PFNFY_MORE, 
		 "    Total texture use   =%8.3f MB",
		 total * 1.0f/(1024.0f*1024.0f));
	if (textureSize > 0)
	{
	    pfNotify(PFNFY_INFO, PFNFY_MORE, 
		     "    This texture uses   =%8.3f%% of %dMB memory",
		     bytes*100.0f/textureSize,
		     textureSize/(1024*1024));
	    pfNotify(PFNFY_INFO, PFNFY_MORE, 
		     "    Total texture use   =%8.3f%% of %dMB memory", 
		     total*100.0f/textureSize,
		     textureSize/(1024*1024));
	}

    }
    listElapsedTime = pfGetTime() - listStartTime;

    pfNotify(PFNFY_INFO, PFNFY_MORE, "  Download totals");
    pfNotify(PFNFY_INFO, PFNFY_MORE, "    Textures: %d", n);
    pfNotify(PFNFY_INFO, PFNFY_MORE, "    Elapsed time: %.3f sec", listElapsedTime);
    pfNotify(PFNFY_INFO, PFNFY_MORE, NULL);
}

PFUDLLEXPORT void	
pfuLoadDetailTextures(pfList *list, pfuDetailInfo *di, int ndi)
{
    pfTexture *dtex;
    int num, i, n;
    void *arena = pfGetSharedArena();
    int comp, sx, sy, sz;
    uint *image;
    
    if (!list || !(num = pfGetNum(list)))
	return;

    for (i=0; i < num; i++)
    {
	const char *name;
	pfTexture *tex=0;

	tex = (pfTexture *)pfGet(list, i);
	pfGetTexImage(tex, &image, &comp, &sx, &sy, &sz);
	name = pfGetTexName(tex);
	if (name == NULL)
	    continue;

	if (pfGetTexDetailTex(tex) || 
		(pfGetTexFilter(tex, PFTEX_MAGFILTER) == PFTEX_SHARPEN))
		continue;

	for (n=0; n < ndi; n++) {
	    if (strcmp(name, di[n].texname)) continue;
	    
	    /* load it if it's not already loaded */
	    if (!di[n].detailtex) {
		dtex = di[n].detailtex = pfNewTex(arena);
		if (pfLoadTexFile(dtex, di[n].detailname) <= 0) {
		    pfDelete(dtex);
		    di[n].detailtex = 0;
		    break;
		} else {
		    pfGetTexImage(dtex, &image, &comp, &sx, &sy, &sz);
		    switch (comp) {
		      case 1:
			pfTexFormat(dtex, PFTEX_INTERNAL_FORMAT,
				    PFTEX_I_8);
			break;
		      case 2:
			pfTexFormat(dtex, PFTEX_INTERNAL_FORMAT,
				    PFTEX_IA_8);
			break;
		      case 3:
			pfTexFormat(dtex, PFTEX_INTERNAL_FORMAT,
				    PFTEX_RGB_5);
			break;
		    }
		    pfInsert(list, 0, dtex);
		}
	    }
	    pfTexDetail(tex, di[n].level, di[n].detailtex);
	    pfTexFilter(tex, PFTEX_MAGFILTER_COLOR, PFTEX_ADD_DETAIL);
	    if (di[n].setSpline)
		pfTexSpline(tex, PFTEX_DETAIL_SPLINE, di[n].spline, 1.0);
	    break;
	}
    }
}

/******************************************************************************
 *			Dynamic Texture Sequencing Utilities
 ******************************************************************************
 */

/**** pfuNewTexList() - put a new list on a pfTexture */
PFUDLLEXPORT void
pfuNewTexList(pfTexture *tex)
{
    pfTexList(tex, pfNewList(sizeof(pfTexture*), 16, pfGetArena(tex)));
}

/****
 * Assorted routines to load in movie files.
 */
 
/**** pfuLoadTexList() - load a give list of file names 
 *	If the provided pfList* is NULL, a new one is created.
 *	Otherwise, new files are appended to the existing list.
 */
PFUDLLEXPORT pfList *
pfuLoadTexListFiles(pfList *movieTexList, char nameList[][PF_MAXSTRING], int len)
{
    pfTexture *ftex;
    void *arena;
    int i;
    
    
    if (nameList == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuLoadTexSeq() - Null movie name list!");
	return NULL;
    }
    
    
    if (!movieTexList)
    {
	arena = pfGetSharedArena();
	movieTexList = pfNewList(sizeof(pfTexture*), 16, arena);
    } 
    else
    {
	arena = pfGetArena(movieTexList);
    }
    for (i=0; i < len; i++)  
    {
	if ((nameList[i]) && (nameList[i][0] != '0'))
	{
	    /*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		"pfuLoadTexListFiles() - new tex file: \"%s\"", nameList[i]);
	    */
	    if (ftex = pfuNewSharedTex(nameList[i], arena))
	    {
		pfTexName(ftex, (char *) nameList[i]);
		pfAdd(movieTexList, ftex);
	    }
	}
    }
    return movieTexList;
}

/**** pfuLoadTexListFmt() - load range of movie frames into list.
 *	file name format determined by fmtStr with current frame number argument.
 *	If the provided list is NULL,  a new list is created.
 */
PFUDLLEXPORT pfList *
pfuLoadTexListFmt(pfList *movieTexList, const char *fmtStr, int start, int end)
{
    pfTexture *ftex;
    void *arena;
    int i;
    char fname[PF_MAXSTRING], tstr[PF_MAXSTRING];
    
    if (fmtStr == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuLoadTexMovie() - Null fmtStr!");
	return NULL;
    }
    
    sprintf(fname, fmtStr, start);
    sprintf(tstr, fmtStr, end);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
	"pfuLoadTexMovie: Loading movie frames [%s - %s]", fname, tstr);
	
    if (!movieTexList)
    {
	arena = pfGetSharedArena();
	movieTexList = pfNewList(sizeof(pfTexture*), 16, arena);
    } 
    else
    {
	arena = pfGetArena(movieTexList);
    }
    
    for (i=start; i <= end; i++)
    {
	sprintf(fname, fmtStr, i);
	if (ftex = pfuNewSharedTex(fname, arena))
	{
	    pfTexName(ftex, fname);
	    pfAdd(movieTexList, ftex);
	}
    }
    return movieTexList;
}

/****
 * The movie machinery: a pfSequence node, user data, and pre-draw callback.
 */

/**** pfuNewProjector - make a new pfSequnce with userData pointing to a 
 *	shared pfuProjectorData structure.
 *	If the provided texture handle is NULL, a new one is created.
 */
PFUDLLEXPORT pfSequence *
pfuNewProjector(pfTexture *handle)
{
    pfSequence *seq;
    pfuProjectorData	*pdata;
    pfList	*frames;
    
    if (handle == NULL)
    {
	handle = pfNewTex(pfGetSharedArena());
    }
    
    /* create new pfSequence node for the movie */
    seq = pfNewSeq();
    /* create the UserData area */
    pdata = (pfuProjectorData*) pfCalloc(1, sizeof(pfuProjectorData), pfGetSharedArena());
    /* assign handle as both the default base texture and the initial screen */
    pdata->handle = handle;
    if (frames = pfGetTexList(handle))
	pfuProjectorMovie(seq, frames);
    pfUserData(seq, pdata);
    /* make movie re-play continuously at default speed */
    pfSeqDuration(seq, 1, -1);
    
    /* The movie starts out with the given texture as a screen */
    pdata->screens = pfNewList(sizeof(pfTexture*), 8, pfGetSharedArena());
    pfAdd(pdata->screens, handle);
    pdata->numScreens = 1;
    
    /* Turn off culling - this node should always be drawn */
    pfNodeTravMask(seq, PFTRAV_CULL, 0x0, 
			PFTRAV_SELF | PFTRAV_DESCEND, PF_SET);
    return (seq);
}

/**** pfuProjectorPreDrawCB() - update each screen showing the movie each frame
 *				in the draw traversal.
 */
PFUDLLEXPORT int
pfuProjectorPreDrawCB(pfTraverser *trav, void *travData)
{
    pfuProjectorData *pdata = (pfuProjectorData*)travData;
    int childNum = pfGetTravIndex(trav);
    pfList *sl = pdata->screens;
    pfTexture *screen;
    int i, n;
    
    n = pdata->numScreens;
    for (i=0; i < n; i++)
    {
	screen = pfGet(sl, i);
	pfTexFrame(screen, childNum);
	/*
	{
	    pfTexture *tex, *t2;
	    pfList *tl;
	    tex = pfGet(pdata->frames,childNum);
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		"Setting frame %d=\"%s\"  on screen \"%s\"",
		childNum, pfGetTexName(tex), pfGetTexName(screen));
	    tl = pfGetTexList(screen);
	    if (tl != pdata->frames)
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
		    "Bad frames on screen  \"%s\"", pfGetTexName(screen));
	    t2 = pfGet(tl,childNum);
	    if (t2 != tex)
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
		    "Bad tex %d in frames list of screen  \"%s\"", 
		    childNum, pfGetTexName(screen));
	
	}
	*/
    }
    
    return PFTRAV_CONT;
}

/****
 * pfuProjectorMovie - Replaces handle texture list
 *	Sets the frame to -1.
 */
PFUDLLEXPORT void 
pfuProjectorMovie(pfSequence *proj, pfList *movie)
{
    pfuProjectorData *pdata;
    pfGroup *g;
    pfTexture *screen;
    int len, i;
    
    if (!proj)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuProjectorMovie() - Null pfSequence* proj!");
	return;
    }
    
    pdata = (pfuProjectorData*)pfGetUserData(proj);
    if (!pdata->handle)
	pdata->handle = pfNewTex(pfGetSharedArena());
    
    /* create children for each frame of the movie (all same child) 
     * 
     */
    if (pfGetNumChildren(proj) > 0)
	g = (pfGroup*) pfGetChild(proj, 0);
    else
    {
	g = pfNewGroup();
	pfNodeTravFuncs(g, PFTRAV_DRAW, pfuProjectorPreDrawCB, NULL);
	pfNodeTravData(g, PFTRAV_DRAW, pdata);
    }
    if (movie)
    {
	len = pfGetNum(movie);
	if (pdata->frames)
	    len -= pfGetNum(pdata->frames);
	for (i=len; i > 0; i--)
	    pfAddChild(proj, g);
    }	
    pdata->frames = movie;
    
    /* for every screen, make the movie the texture list of that screen */
    for (i=0; i < pdata->numScreens; i++)
    {
	screen = pfGet(pdata->screens, i);
	pfTexList(screen, pdata->frames);
	pfTexFrame(pdata->handle, -1);
    }
}

PFUDLLEXPORT pfTexture*
pfuGetProjectorHandle(pfSequence *proj)
{
    pfuProjectorData *pdata;
    if (!proj)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuGetProjectorHandle() - Null pfSequence* proj!");
	return NULL;
    }
    pdata = (pfuProjectorData*)pfGetUserData(proj);
    return (pdata->handle);
}

/****
 * pfuProjectorHandle - Replace base texture and list
 *	Sets the frame to -1.
 */
PFUDLLEXPORT void 
pfuProjectorHandle(pfSequence *proj, pfTexture *new)
{
    pfuProjectorData *pdata;
    pfList *frames;
    
    if (!proj)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuReplaceProjectorHandle() - Null pfSequence* proj!");
	return;
    }
    pdata = (pfuProjectorData*)pfGetUserData(proj);
    pdata->handle = new; 
    if (frames = pfGetTexList(new))
	pfuProjectorMovie(proj, frames);
    else
	pfTexFrame(pdata->handle, -1);
}


PFUDLLEXPORT pfList *pfuGetProjectorScreenList(pfSequence *proj)
{
    pfuProjectorData *pdata;
    if (!proj)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuGetProjectorScreenList() - Null pfSequence* proj!");
	return NULL;
    }
    pdata = (pfuProjectorData*)pfGetUserData(proj);
    return (pdata->screens);
}

/****
 * Put a movie on a screen: each movie gets a list of screens,  but each
 *	screen should only be showing one movie!
 *	The original base texture of the movie is kept around
 *	so that it is never "screen-less".
 * Also note that the screen will inherit the texture management modes from 
 * 	the projector handle.
 */
PFUDLLEXPORT void 
pfuAddProjectorScreen(pfSequence *proj, pfTexture *screen)
{
    pfuProjectorData *pdata;
    int repeat;
    
    if (!proj)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuAddProjectorScreen() - Null pfSequence* proj!");
	return;
    }
    if (!screen)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuAddProjectorScreen() - Null pfTexture* screen!");
	return;
    }
    
    /*
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
	    "Putting movie \"%s\"  on screen \"%s\"",
	    pfGetNodeName(proj), pfGetTexName(screen));
    */ 
    
    /* Put the screen ptr in the movie's user-data screens */
    pdata = (pfuProjectorData*)pfGetUserData(proj);
    pfAdd(pdata->screens, screen); 
    pdata->numScreens++;
    /* apply the movie frames to the screen texture */
    pfTexList(screen, pdata->frames);
    /* set texture management  modes */
    pfTexLoadMode(screen, PFTEX_LOAD_LIST, 
	    pfGetTexLoadMode(pdata->handle, PFTEX_LOAD_LIST));
    /* set the texture to the current frame */
    pfTexFrame(screen, pfGetSeqFrame(proj, &repeat));
}


/****
 * pfuRemoveProjectorScreen - Remove a screen from the movie.
 *	Sets the screen's texture list to NULL and frame to -1.
 *	Reset texture management modes.
 */
PFUDLLEXPORT void 
pfuRemoveProjectorScreen(pfSequence *proj, pfTexture *screen)
{
    pfuProjectorData *pdata;
    
    if (!proj)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuRemoveProjectorScreen() - Null pfSequence* proj!");
	return;
    }
    if (!screen)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuRemoveProjectorScreen() - Null pfTexture* screen!");
	return;
    }
    pdata = (pfuProjectorData*)pfGetUserData(proj);
    pfRemove(pdata->screens, screen); 
    pfTexList(screen, NULL);
    pfTexFrame(screen, -1);
    pdata->numScreens--;
}

/****
 * pfuReplaceProjectorScreen - replace one screen with another.
 *	Sets the removed screen's texture list to NULL and frame to -1.
 */
PFUDLLEXPORT void 
pfuReplaceProjectorScreen(pfSequence *proj, pfTexture *old, pfTexture *new)
{
    pfuProjectorData *pdata;
    
    if (!proj)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuRemoveProjectorScreen() - Null pfSequence* proj!");
	return;
    }
    if (!old || !new)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuReplaceProjectorScreen() - bad screen.");
	return;
    }
    pdata = (pfuProjectorData*)pfGetUserData(proj);
    pfReplace(pdata->screens, old, new); 
    pfTexList(old, NULL);
    pfTexFrame(old, -1);
}

/* 0 ok; nonzero error */
static int CalculateNumPages(
	pfTexture *texture,
	int numtram,
	int components,
	int ns,
	int nt )
{
	static int tot_pages = 0;
	static int free_alpha = 0;
	static int free_blue = 0;
	static int free_green = 0;
	static pfList *detailL=NULL;
	static int download_detail=0;
	__TextureInfo tex;
	pfTexture *detail;
	int numbits;
	int format;

/* calculate the # of TRAM pages for IMPACT */
	memset( &tex, 0, sizeof(__TextureInfo) );
	tex.id.numcomp = components;
	tex.id.width = ns;
	if ( download_detail )
		tex.id.width *= 2;
	tex.id.height = nt;
	if ( !download_detail )
		tex.mm_enab = 1;

	format = pfGetTexFormat( texture, PFTEX_INTERNAL_FORMAT );
	switch (format)
	  {
		case PFTEX_I_16:
			numbits = 16;
			break;
		case PFTEX_RGBA_12:
		case PFTEX_RGB_12:
			if ( numtram == 1 )
				numbits = 4;
			 else
				numbits = 12;
			break;
		case PFTEX_IA_12:
		case PFTEX_I_12A_4:
			if ( numtram == 1 )
				numbits = 8;
			 else
				numbits = 12;
			break;
	        case PFTEX_RGB_8:
		case PFTEX_RGBA_8:
		case PFTEX_RGB_5:
			if ( numtram == 1 )
				numbits = 4;
			 else
				numbits = 8;
			break;
		case PFTEX_IA_8:
		case GL_LUMINANCE8_EXT:
			numbits = 8;
			break;
		case PFTEX_RGBA_4:
		case PFTEX_RGB_4:
			numbits = 4;
			break;
		default:
			pfNotify(PFNFY_INFO, PFNFY_MORE, 
				 "    Unsupported internal download texture format" );
			return -1; /* can't survive this error */
	  }

	tex.id.compdepth = numbits / 4 - 1;
	tex.id.internalformat = format_enums[tex.id.numcomp-1][tex.id.compdepth];
	set_internalformat( &tex, numtram );
	tex.vx2 = tex.id.db_possible;
	total_pages( &tex );

	if ( numtram > 1 )
		switch( components )
		  {
			case 1:
				if ( tot_pages+tex.numpage < 256 )
				  {
					free_green += tex.numpage;
					free_blue += tex.numpage;
					free_alpha += tex.numpage;
					tot_pages += tex.numpage;
				  }
				 else if ( free_alpha >= tex.numpage )
					free_alpha -= tex.numpage;
				 else if ( free_blue >= tex.numpage )
					free_blue -= tex.numpage;
				 else if ( free_green >= tex.numpage )
					free_green -= tex.numpage;
				 else
				  {
					free_green += tex.numpage;
					free_blue += tex.numpage;
					free_alpha += tex.numpage;
					tot_pages += tex.numpage;
				  }
				break;
			case 2:
				if ( free_alpha >= tex.numpage && free_blue >= tex.numpage )
				  {
					free_alpha -= tex.numpage;
					free_blue -= tex.numpage;
				  }
				 else
				  {
					tot_pages += tex.numpage;
					free_alpha += tex.numpage;
					free_blue += tex.numpage;
				  }
				break;
			case 3:
				tot_pages += tex.numpage;
				free_alpha += tex.numpage;
				break;
			case 4:
				tot_pages += tex.numpage;
				break;
		  }
		 else if ( numtram == 1 )
			tot_pages += tex.numpage;
	
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
		"    Download size       = %4d pages", tex.numpage);
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
		 "    Total hardware texture use   =%4d Pages", tot_pages );

	if ( (detail=pfGetTexDetailTex( texture )) )
	  {
		if ( !detailL )
		  {
			detailL = pfNewList(sizeof(pfTexture*), 4, pfGetSharedArena());
			pfAdd( detailL, detail );
			download_detail = 1;
		  }
		 else if ( pfSearch( detailL, detail ) == -1 )
		  {
			pfAdd( detailL, detail );
			download_detail = 1;
		  }
	  }
	 else
		download_detail = 0;

	if ( download_detail )
	  {
		unsigned int *im;
		int c;
		int s, t, r;
		int ss, st;

/* get details of texture's in-memory representation */
		pfGetTexImage(detail, &im, &c, &s, &t, &r);
		if (r < 1) r = 1;

		/* print statistics */
		pfNotify(PFNFY_INFO, PFNFY_MORE, 
		"  Loading detail texture:");
		pfNotify(PFNFY_INFO, PFNFY_MORE, 
		"    File name           = %s", pfGetTexName(detail));
		pfNotify(PFNFY_INFO, PFNFY_MORE, 
		"    Image size (SxTxR)  = %dx%dx%d", s, t, r);
		ss = powerOfTwo(s);
		st = powerOfTwo(t);
		if (s != ss || t != st)
		pfNotify(PFNFY_INFO, PFNFY_MORE, 
		"    Scaled size (SxTxR) = %dx%dx%d", ss, st, r);

		if(CalculateNumPages( detail, numtram, c, s, t ))
		    return -1;
	  }
	return 0;
}

static void
total_pages(
	__TextureInfo *tex )
{
	long lod = 0;
	__TexLevInfo *lev;

   if ( tex->id.extent != 0 )
	  {
		tex->dim = 6;

		if ( tex->id.width==0 || tex->id.height==0 || tex->id.depth==0 )
			 return;

		if ( tex->id.brd_data )
			 return;

		if ( tex->mm_enab )
			 return;

		memcpy( &tex->lev[lod].id, &tex->id, sizeof(__TexLevID) );
		pages_per_level( tex, lod );
	  }
    else if ( tex->id.depth != 0 )
	  {
		tex->dim = 4;

		if ( tex->id.width==0 || tex->id.height==0 )
			return;

		if( tex->id.brd_data )
	   	return;

		if ( tex->mm_enab )
	   	return;

		memcpy( &tex->lev[lod].id, &tex->id, sizeof(__TexLevID) );
    	pages_per_level( tex, lod );
	  }
    else if ( tex->id.height != 0 )
	  {
		tex->dim = 2;

		if ( tex->id.width==0 )
			return;

		lev = &tex->lev[lod];
		memcpy( &lev->id, &tex->id, sizeof(__TexLevID) );
    	pages_per_level( tex, lod );

    	if ( tex->mm_enab )
		  {
			lev = &tex->lev[lod];
			while( !( lev->id.width==1 && lev->id.height==1 ) )
			  {
				memcpy( &tex->lev[++lod].id, &lev->id, sizeof(__TexLevID) );

				lev = &tex->lev[lod];
				if( (lev->id.width >>= 1) == 0 )
					lev->id.width = 1;
				if( (lev->id.height >>= 1) == 0 )
					lev->id.height = 1;
				pages_per_level(tex, lod );
			  }
		  }
	  }
    else if ( tex->id.width != 0 )
	  {
		tex->dim = 0;
		lev = &tex->lev[lod];
		memcpy(&lev->id, &tex->id, sizeof(__TexLevID));
    	pages_per_level(tex, lod );

    	if ( tex->mm_enab )
		  {
			lev = &tex->lev[lod];
			while( lev->id.width !=1 )
			  {
				memcpy( &tex->lev[++lod].id, &lev->id, sizeof(__TexLevID) );
				lev = &tex->lev[lod];

				if((lev->id.width >>= 1) == 0)
					lev->id.width = 1;

				pages_per_level(tex, lod );
			  }
		  }
	  }
}

static void
pages_per_level(
	__TextureInfo *tex,
	int lod)
{
	long tm_ssize, tm_tsize, border_tb, border_lr, corner, overlap; 
	long numpage, numspg, numtpg, brd_numpage, thickness;
	__TexLevInfo *lev = &tex->lev[lod], *levtmp;

/* tm_ssize is the original width/2 for 1D and 2D, width for 3D and 4D;
   divided by 2 if we have horizontal x2 */
	tm_ssize = (tex->dim<DIM_3D ? lev->id.width>>1 : lev->id.width)>>tex->id.hx2;
	tm_tsize = lev->id.height >> tex->vx2;

/* default is no border pages */
	lev->brd_page = -1;
	brd_numpage = 0;

/* packed case */
	if ( tm_tsize <= 16 && tm_ssize <= 8 && tex->dim < DIM_3D )
	  {
		lev->numpage = brd_numpage = overlap = 0;
		if ( tex->id.brd_data || tex->mm_enab )
			lev->brd_page = 0;
		if ( tex->packed )
		  {
	   	if ( lod > 0 )
			  {
				levtmp = &tex->lev[lod-1];
				if ( (levtmp->id.width>>1>>tex->id.hx2)+(levtmp->id.width>>tex->vx2)
						< 1)
				  {
					lev->prohibited = 1;
					return;
				  }
			  }
			lev->packed = tex->packed;
		  }
		 else
		  {
			lev->numpage = lev->totpage = 1;
			tex->numpage++;
			tex->packed = lev;
		  }
	  }
    else
/* not the packed case */
	  {
    	if( (numspg = tm_ssize >> 5) == 0 )
			numspg = 1;
    	if( (numtpg = tm_tsize >> 5) == 0 )
			numtpg = 1;
		lev->numpage = 0;

/* for 1D, image data is in the border pages */
    	if ( tex->dim > DIM_PROXY_1D )
	   	lev->numpage = numspg * numtpg;

/* for 3D and 4D, calc total pages */
		if ( tex->dim > DIM_PROXY_2D )
		  {
			if ( (thickness = lev->id.depth>>1) == 0 )
				thickness = 1;
			lev->numpage *= thickness * (lev->id.extent == 0 ? 1 : lev->id.extent);
			if ( lev->numpage == 0 )
				lev->numpage = 1;
		  }

/* Default setting */
    	border_tb = border_lr = 0;

/* if border data supplied or 1D */
    	if ( tex->id.brd_data || tex->dim < DIM_2D )
		  {

/* 8 or 16 rows @ 32 or 64 texels per row */
			border_tb = tm_ssize >> 8 >> tex->vx2;

/* partial page gets 1 page */
			if ( border_tb == 0 )
				border_tb = 1;
		  }

		overlap = 0;
    	if ( tex->dim > DIM_PROXY_1D && (tex->id.brd_data || tex->mm_enab) )
		  {

/* 8 columns @ 32 or 64 texels per column */
			border_lr = tm_tsize >> 8;
			if ( border_lr == 0 )
				border_lr = 1;

			if ( lev->id.brd_data )
			  {
				if ( tm_tsize <= 16 )
					overlap = 1;
			  }
			 else if ( tm_ssize <= 16 )
				overlap = 1;
		  }

/* assignment statement intended */
		if ( corner = border_tb )
			corner = corner &&
						(tex->dim > DIM_PROXY_1D || tex->id.brd_data || tex->mm_enab);

		brd_numpage = border_tb + border_lr + corner;

/* put border pages at end of mipmap pages */
		if ( brd_numpage )
			lev->brd_page = lev->numpage - overlap;

		numpage = lev->numpage + brd_numpage - overlap;
		lev->totpage = numpage;
		tex->numpage += numpage;
	  }
}

/* The next step is to establish the internal format as one of
* the 12 formats implemented in the TRAMs, taking into account
* the number of TRAMs. We have to set the numcomp, compdepth,
* hx2, code, and db_possible parameters for each internal format.
*/
static void
set_internalformat(
	__TextureInfo *tex,
	int numtram)
{
	tex->id.hx2 = tex->id.db_possible = 0;

	switch ( tex->id.internalformat )
	  {
		case GL_LUMINANCE4_EXT:
			 tex->id.hx2 = 1;
			 tex->id.db_possible = 1;
			 break;
		case GL_LUMINANCE8_EXT:
			 tex->id.db_possible = 1;
			 break;
		case GL_LUMINANCE12_EXT:
			 break;
		case GL_LUMINANCE4_ALPHA4_EXT:
			if ( numtram == 4 )
				tex->id.hx2 = 1;
			tex->id.db_possible = 1;
			break;
		case GL_LUMINANCE8_ALPHA8_EXT:
			if ( numtram == 4 )
				tex->id.db_possible = 1;
			break;
		case GL_LUMINANCE12_ALPHA12_EXT:
			if ( numtram == 1 )
			  {
				tex->id.compdepth = 1;
				tex->id.internalformat = GL_LUMINANCE8_ALPHA8_EXT;
			  }
			break;
		case GL_RGB4_EXT:
			if ( numtram == 4 )
			  {
				tex->id.db_possible = 1;
				tex->id.hx2 = 1;
			  }
	   	break;
		case GL_RGB8_EXT:
			if ( numtram == 4 )
				tex->id.db_possible = 1;
			 else
/* redefine component depth */
			  {
				tex->id.compdepth = 0;
				tex->id.internalformat = GL_RGB4_EXT;
			  }
			break;
		case GL_RGB12_EXT:

/* redefine component depth */
			if ( numtram == 1 )
			  {
				tex->id.compdepth = 0;
				tex->id.internalformat = GL_RGB4_EXT;
			  }
			break;
		case GL_RGBA4_EXT:
			if ( numtram == 4 )
			  {
				tex->id.db_possible = 1;
				tex->id.hx2 = 1;
			  }
			break;
		case GL_RGBA8_EXT:
			if ( numtram == 4 )
				tex->id.db_possible = 1;
			 else
/* redefine component depth */
			  {
				tex->id.compdepth = 0;
				tex->id.internalformat = GL_RGBA4_EXT;
			  }
			break;
		case GL_RGBA12_EXT:

/* redefine component depth */
			if ( numtram == 1 )
			  {
				tex->id.internalformat = GL_RGBA4_EXT;
				tex->id.compdepth = 0;
			  }
			break;
		default:
			break;
	  }
}
