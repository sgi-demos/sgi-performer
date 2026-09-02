#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#include <limits.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pf.h>

/* 
 * int LoadTexFile(pfTexture *tex, char *filename);
 *     - Loads the image stored with pfdStoreFile_h() from 'filename'
 */
int LoadTexFile(pfTexture *tex, char *filename)
{
	FILE *perffp=NULL;
	int comp, ns, nt, nr = 0;
	unsigned int *myimage=NULL;
	int i=0;

	perffp = fopen(filename, "r");
	if(perffp == NULL)
	{
		return 0;
	}
	fscanf(perffp, "static int myimage_ns = %d;\n", &ns);
	fscanf(perffp, "static int myimage_nt = %d;\n", &nt);
	fscanf(perffp, "static int myimage_nr = %d;\n", &nr);
	fscanf(perffp, "static int myimage_comp = %d;\n", &comp);
	fscanf(perffp, "static unsigned int myimage[] = {\n");

	myimage = (unsigned int *)pfMalloc(comp*ns*nt*nr, pfGetSharedArena());

	for(i=0; i<comp*ns*nt*nr; i++)
	{
	    fscanf(perffp, "0x%x,\n", &myimage[i]);
	}
	fprintf(perffp, "};\n");
	fclose(perffp);

	pfTexImage(tex, myimage, comp, ns, nt, nr);
	return 1;
}

/* 
 * pfNode *pfdLoadFile_h(char *filename);
 *      - Loads the header file named 'filename' and returns a
 *        pfGeode that contains a textured quad with nodeName 'myimage geode'.
 *        This loader was only designed to load one image into a scene.
 *        This restriction was based on laziness.  The purpose of this
 *        loader was so that it could be used in conjunction with pfconv.
 */
PFPFB_DLLEXPORT pfNode *pfdLoadFile_h(char *filename)
{
	pfGeode *geode=NULL;
	pfGeoSet *gset=NULL;
	pfGeoState *gstate=NULL;
	pfTexture *tex=NULL;
	pfTexEnv *tev=NULL;
	static pfVec2 texcoords[]={{0.0f, 0.0f},
                                {1.0f, 0.0f},
                                {1.0f, 1.0f},
                                {0.0f, 1.0f} };
	static ushort texlist[] = { 0, 1, 2, 3 };
	static pfVec3 coords[] ={{-1.0f, 0.0f, -1.0f },
                                { 1.0f, 0.0f, -1.0f },
                                { 1.0f, 0.0f, 1.0f },
                                {-1.0f, 0.0f, 1.0f } };
	static ushort vertexlist[] = { 0, 1, 2, 3 };
	static pfVec4 colors[] ={{1.0f, 1.0f, 1.0f, 1.0f},
                                {1.0f, 1.0f, 1.0f, 1.0f},
                                {1.0f, 1.0f, 1.0f, 1.0f},
                                {1.0f, 1.0f, 1.0f, 1.0f} };
	static ushort colorlist[] = { 0, 1, 2, 3 };


	geode = pfNewGeode();
	gset  = pfNewGSet(pfGetSharedArena());
	tex   = pfNewTex(pfGetSharedArena());
        if (LoadTexFile (tex, filename))
        {
            uint *i;
            int nc, sx, sy, sz;
            gstate = pfNewGState (pfGetSharedArena());
            pfGetTexImage(tex, &i, &nc, &sx, &sy, &sz);
            /* if have alpha channel, enable transparency */
            if (nc != 3)
                pfGStateMode (gstate, PFSTATE_TRANSPARENCY, PFTR_FAST);
            /* set alpha function to block pixels of 0 alpha for 
               transparent textures */
            pfGStateMode (gstate, PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
            pfGStateVal (gstate, PFSTATE_ALPHAREF, 0.0f);
            pfGStateAttr (gstate, PFSTATE_TEXTURE, tex);
            tev = pfNewTEnv (pfGetSharedArena());
            pfGStateAttr (gstate, PFSTATE_TEXENV, tev);
            pfGStateMode (gstate, PFSTATE_ENTEXTURE, 1);
            pfGStateMode (gstate, PFSTATE_ENLIGHTING, 0);
            pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
        }
	else
	{
            pfNotify(PFNFY_WARN, PFNFY_SYSERR, "failed to load: %s\n", filename);
            return NULL;
	}

	pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, vertexlist);
	pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, texlist);
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, colors, colorlist);
	pfGSetPrimType(gset, PFGS_QUADS);
	pfGSetNumPrims(gset, 1);
	pfGSetGState (gset, gstate);
	geode = pfNewGeode();
	pfAddGSet(geode, gset);
	pfNodeName(geode, "myimage geode");
	return (pfNode *)geode;
}

/*
 * int pfdStoreFile_h(pfNode *root, cont char *filename);
 *     - Searches the scene graph from 'root' for a pfGeode named 
 *       'myimage geode', then retrieves and stores the associated pfTexture
 *       as a textfile with C code containing four static variables and a
 *       static array containing the actual image data.  The purpose of
 *       this textfile is that it could be #included from a C/C++ file and
 *       the information can be used directly with pfTexture::setImage().
 *       For example:
 *       tex->setImage((uint *)myimage, myimage_comp, myimage_ns, myimage_nt, myimage_nr);

 */
int pfdStoreFile_h(pfNode *root, const char *filename)
{
	pfGeode *geode = NULL;
	pfGeoSet *gset = NULL;
	pfGeoState *gstate = NULL;
	pfTexture *tex = NULL;
	int comp, ns, nt, nr = 0;
	unsigned int *myimage=NULL;
	int i;
	FILE *perffp=NULL;

	geode = (pfGeode *) pfFindNode(root, "myimage geode", pfGetGeodeClassType());
	if(geode == NULL)
	{
                pfNotify(PFNFY_WARN, PFNFY_SYSERR, "Unable to find node named 'myimage geode'\n");
		return FALSE;
	}

	gset = pfGetGSet((pfGeode*)geode, 0);
	if(gset == NULL)
	{
                pfNotify(PFNFY_WARN, PFNFY_SYSERR, "Unable to retrieve gset from geode\n");
		return FALSE;
	}

	gstate = pfGetGSetGState(gset);
	if(gstate == NULL)
	{
                pfNotify(PFNFY_WARN, PFNFY_SYSERR, "Unable to retrieve gstate from geode\n");
		return FALSE;
	}

	tex = pfGetGStateAttr(gstate, PFSTATE_TEXTURE);
	if(tex == NULL)
	{
                pfNotify(PFNFY_WARN, PFNFY_SYSERR, "Unable to retrieve pfTexture from gstate\n");
		return FALSE;
	}

	pfGetTexImage(tex, &myimage, &comp, &ns, &nt, &nr);
	perffp=fopen(filename, "w");
	if(perffp == NULL)
	{
                pfNotify(PFNFY_WARN, PFNFY_SYSERR, "Unable to open %s for writing\n", filename);
		return FALSE;
	}

	fprintf(perffp, "static int myimage_ns = %d;\n", ns);
	fprintf(perffp, "static int myimage_nt = %d;\n", nt);
	fprintf(perffp, "static int myimage_nr = %d;\n", nr);
	fprintf(perffp, "static int myimage_comp = %d;\n", comp);
	fprintf(perffp, "static unsigned int myimage[] = {\n");
	for(i=0; i<comp*ns*nt*nr; i++)
	{
	    fprintf(perffp, "0x%x,\n", myimage[i]);
	}
	fprintf(perffp, "};\n");
	fclose(perffp);

	return TRUE;
}


