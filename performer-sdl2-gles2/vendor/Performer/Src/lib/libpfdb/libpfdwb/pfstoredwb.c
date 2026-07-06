/*
 *
 * COPYRIGHT NOTICE:
 *
 * Copyright (C) 1989-1992 Coryphaeus Software,985 University Ave.,Suite 31,
 * Los Gatos, CA  95030.
 *
 */

static char rcsid[] =
 "$Id: pfstoredwb.c,v 1.19 2002/09/22 03:06:30 naaman Exp $";

#include <stdio.h>
#ifdef sgi
#include <bstring.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <Performer/pfdb/pfdwb.h>

#ifdef __linux__
#include <asm/byteorder.h>
#endif

#define PFTODWB_VERSION "4.0"

static char pftodwb_banner[] = 
"pf2todwb " PFTODWB_VERSION " - " __DATE__ "\n\
Copyright (c) 1993-1997 Coryphaeus Software, Inc.\n\
Los Gatos, California, USA.  All Rights Reserved.\n\
\n\
";

static int 	 	displayMode = 0;
static int             verbose = 0;
static int        	level;
static FILE        	*outFile;
static int		pCount=1, gCount=1, LODCount=1;
static int             swCount = 1;
static int             seqCount = 1;
static int             decalCount = 1;
static int             billCount = 1;
static int             lpntCount = 1;
static int             tris=0, quads=0, tristrips=0, flattristrips=0;
static int             cPerVertex = 0;
static char            winTitle[512];

/*
 * Hash Tables for material, color and texture lookup
 */

static pfuHashTable *htM, *htC;

typedef struct _texTable
{
  pfTexture *texture;
  int ref;
  int detail;
} texTable;

static texTable *textureTable;
static int  numTextures;
static int  availTextures;

static pfMatStack *mstack;

    static int colorTable[][3] =
    {
    /* shaded: 0 - 29 */
        { 243, 243, 243 }, { 243, 0, 0 },     { 243, 150, 87 },
        { 243, 243, 0 },   { 0, 243, 0 },     { 128, 243, 104 },
        { 0, 0, 243 },     { 0, 193, 243 },   { 196, 0, 243 },
        { 243, 243, 72 },  { 243, 175, 141 }, { 243, 102, 57 },
        { 221, 159, 243 }, { 141, 243, 69 },  { 141, 217, 81 },
        { 11, 11, 11 },    { 196, 195, 243 }, { 243, 104, 124 },
        { 119, 127, 243 }, { 176, 243, 243 }, { 69, 242, 242 },
        { 243, 188, 184 }, { 187, 82, 50 },   { 242, 121, 20 },
        { 237, 104, 197 }, { 123, 213, 167 }, { 0, 188, 232 },
        { 242, 242, 157 }, { 243, 243, 243 }, { 243, 243, 243 },
    /* fixed: 31 - 94 */
        { 17, 17, 17 },    { 135, 190, 255 }, { 83, 107, 3 },
        { 240, 240, 240 }, { 175, 175, 175 }, { 116, 116, 116 },
        { 56, 56, 56 },    { 255, 0, 0 },     { 0, 255, 255 },
        { 130, 255, 0 },   { 246, 189, 143 }, { 255, 200, 0 },
        { 0, 56, 0 },      { 255, 0, 0 },     { 0, 0, 0 },
        { 0, 200, 255 },   { 0, 1, 130 },     { 254, 0, 0 },
        { 0, 246, 0 },     { 0, 0, 0 },       { 135, 190, 255 },
        { 60, 60, 255 },   { 255, 166, 0 },   { 250, 155, 0 },
        { 139, 75, 3 },    { 255, 255, 255 }, { 15, 142, 17 },
        { 112, 163, 150 }, { 98, 40, 139 },   { 95, 112, 133 },
        { 95, 130, 16 },   { 255, 147, 5 },   { 0, 0, 0 },
        { 37, 0, 115 },    { 0, 0, 0 },       { 195, 195, 195 },
        { 139, 139, 139 }, { 85, 85, 85 },    { 25, 25, 25 },
        { 80, 0, 0 },      { 0, 25, 80 },     { 0, 80, 0 },
        { 219, 126, 64 },  { 105, 50, 0 },    { 0, 3, 0 },
        { 255, 0, 255 },   { 255, 0, 0 },     { 15, 142, 17 },
        { 0, 0, 112 },     { 163, 0, 150 },   { 0, 98, 0 },
        { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 },
        { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 },
        { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 },
        { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 },
        { 255, 255, 255 },
    /* overlay */
        { 254, 255, 254 }, { 0, 0, 0 },       { 254, 0, 0 },
        { 0, 0, 255 },     { 0, 255, 0 },     { 254, 255, 0 },
        { 161, 161, 161 }, { 254, 255, 255 }, { 255, 255, 255 },
        { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 },
        { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 },
        { 255, 255, 255 },
    /* underlay */
        { 0, 0, 0 },       { 255, 255, 255 }, { 80, 80, 80 },
        { 161, 161, 161 }, { 0, 255, 0 },     { 254, 254, 0 },
        { 254, 0, 0 },     { 254, 166, 0 },   { 127, 138, 199 },
        { 60, 60, 60 },    { 74, 43, 3 },     { 0, 0, 60 },
        { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 },
        { 255, 255, 255 }
    };

static int shaded[30][128][3];

static void handleColor (pfGeoSet *gset, int  primType);
static void handleMaterial (pfGeoSet *gset);
static void handleTexture (pfGeoSet *gset);
static void buildColMatTables (pfNode *root);

static int writeDwbHeader (pfNode *, const char *);
static int writeDwbColorTable (void);
static int writeDwbTextureTable (void);
static int writeDwbMaterialTable (void);
static int writeDwbOpcode (int, int);
static int writeDwbVariableLengthRecord (int , const char *);

static void extractGeometry (pfNode *);
static dwbGroup *buildBasicDwbGroup (void);
static dwbNewFace *buildBasicDwbFace (void);
static void handlePFTYPE_GROUP (pfNode *);
static void handlePFTYPE_LAYER (pfNode *);
static void handlePFTYPE_SEQUENCE (pfNode *);
static void handlePFTYPE_BILLBOARD (pfNode *);
static void handleGEOSET (pfGeoSet *);
static int  buildTRISandQUADS (pfGeoSet *, int , pfList *);
static int  buildTRISTRIPS (pfGeoSet *, int , pfList *);
static void handlePrimitives (pfGeoSet *);

static char *getCurrentDateTime (void);
static void dwbPush (void);
static void dwbPop (void);




#ifdef __LITTLE_ENDIAN
    
    /* little endian */
extern short get16_le(void *ptr);
extern int get32_le(void *ptr);
extern void get16v_le(void *dstp, void *srcp, int n);
extern void get32v_le(void *dstp, void *srcp, int n);
extern void get64v_le(void *dstp, void *srcp, int n);

#define get16 get16_le
#define get32 get32_le
#define get16v get16v_le
#define get32v get32v_le
#define get64v get64v_le

#else
    /* big endian */
extern int get16_be(void *ptr);
extern int get32_be(void *ptr);
extern void get16v_be(void *dstp, void *srcp, int n);
extern void get32v_be(void *dstp, void *srcp, int n);
extern void get64v_be(void *dstp, void *srcp, int n);

   
#define get16(p) (*(short *)(p))
#define get32(p) (*(int *)(p))
#define get16v(q,p,n) memcpy(q,p,2*(n))
#define get32v(q,p,n) memcpy(q,p,4*(n))
#define get64v(q,p,n) memcpy(q,p,8*(n))

#endif

extern void swapDwbLModelInfo(dwbLModelInfo *ptr);
extern void swapDwbLSourceInfo(dwbLSourceInfo *ptr);
extern void swapDwbInstanceDef(dwbInstanceDef *ptr);
extern void swapDwbInstanceRef(dwbInstanceRef *ptr);
extern void swapDwbLineStyle(dwbLineStyle *ptr);
extern void swapDwbMatrix(dwbMatrix *ptr);
extern void swapDwbHeader(dwbHeader *ptr);
extern void swapDwbExtTexInfo(dwbExtTexInfo *ptr);
extern void swapDwbTexInfo(dwbTexInfo *ptr);
extern void swapDwbOpcodeRec(dwbOpcodeRec *ptr);
extern void swapDwbMaterial(dwbMaterial *ptr);
extern void swapDwbMatInfo(dwbMatInfo *ptr);
extern void swapDwbColorTable(dwbColorTable *ptr);
extern void swapDwbGroup(dwbGroup *ptr);
extern void swapDwbSwitch(dwbSwitch *ptr);
extern void swapDwbNewFace(dwbNewFace *ptr);
extern void swapDwbPackedVertex(dwbPackedVertex *ptr);
extern void swapDwbVertex(dwbVertex *ptr);
extern void swapDwbLightPt(dwbLightPt *ptr);
extern void swapDwbBSpline(dwbBSpline *ptr);
extern void swapDwbArc(dwbArc *ptr);
extern void swapDwbPage(dwbPage *ptr);


static void initFuncWithWindow (pfPipeWindow *pw)
{
  pfOpenPWin (pw);
  pfInitGfx ();

  pfLightPos (pfNewLight (NULL), -0.3f, -0.3f, 1.0f, 0.0f);
  pfApplyMtl (pfNewMtl (pfGetSharedArena()));
  pfApplyLModel (pfNewLModel (pfGetSharedArena()));

  pfEnable (PFEN_TEXTURE);
  pfEnable (PFEN_LIGHTING);
}


/*
 * Used by the quick sort function
 */

static int comp (const void *a, const void *b)
{
pfuHashElt **x, **y;

  x = (pfuHashElt **) a;
  y = (pfuHashElt **) b;
  return ((*y)->id - (*x)->id);
}


/*
 * Depending upon the type of the primitive, load the vertex colors into
 * the hash table for future use.
 */

void handleColor (pfGeoSet *gset, int  pType)
{
int  numPrims = pfGetGSetNumPrims (gset);
int  cbind = pfGetGSetAttrBind (gset, PFGS_COLOR4);
pfVec4 *alist;
unsigned short *ilist;
pfuHashElt new, *old;
int i,j;
int a, c;
int z = 0, zf = 0;

  pfGetGSetAttrLists (gset, PFGS_COLOR4, (void **) &alist, &ilist);
  switch (cbind)
  {
    case PFGS_OFF: c = 0; a = 0; break;

    case PFGS_OVERALL: c = 1; a = 1; break;

    case PFGS_PER_PRIM: c = numPrims; a = 1; break;

    case PFGS_PER_VERTEX:
    {
      cPerVertex++;

      switch (pType)
      {
	case PFGS_TRIS: a = 3; break;

	case PFGS_QUADS: a = 4; break;

	case PFGS_TRISTRIPS:
	case PFGS_FLAT_TRISTRIPS:
	  a = -1;
        break;
      }

      c = numPrims;
    }
    break;
  }

    for (i=0; i<c; ++i)
    {
    int len = pfGetGSetPrimLength(gset, i);
    int b = (a == -1) ? len : a;

      for (j=0; j<b; ++j)
      {
      int k[3];
      int index;

	if (a == -1)
	{
	  if (pType == PFGS_TRISTRIPS)
            if (ilist) index = ilist[z+j]; else index = z+j;
          else
	    if (ilist) index = ilist[zf+j]; else index = zf+j;
        }
	else
	{
          if (ilist) index = ilist[a*i+j]; else index = a*i+j;
        }

      /*
       * float to int cannot be converted by casting it, but
       * double to int works fine. Check man floor(2)
       *
       * Make sure that if the index list is nonNULL, the colors are
       * indexed properly.
       */

        k[0] = (int) ((double) alist[index][0]*255);
        k[1] = (int) ((double) alist[index][1]*255);
        k[2] = (int) ((double) alist[index][2]*255);
        new.data = (void *) k;

        if (! pfuFindHash (htC, &new)) {
  	  new.id = 1;
          pfuEnterHash (htC, &new);
        }
        else {
          old = pfuEnterHash (htC, &new);
	  old->id++;
        }
      }

      if (a == -1)
      {
	z += len;
        zf += len-2;
      }
    }

  return;
}


/*
 * Load the material used by the geoset into the hash table for future
 * look ups.
 */

void handleMaterial (pfGeoSet *gset)
{
pfGeoState *gstate = pfGetGSetGState (gset);
pfMaterial *front, *back;

  if (htM->listCount == 63) {
    fprintf (stderr, "pftodwb: More than 64 materials! ignoring material\n");
    return;
  }

  front = (pfMaterial *) pfGetGStateAttr (gstate, PFSTATE_FRONTMTL);
  back  = (pfMaterial *) pfGetGStateAttr (gstate, PFSTATE_BACKMTL);

  if (front == NULL && back == NULL) return;

  if (front) {
  pfuHashElt new, *old;
  float m[14];

    pfGetMtlColor (front, PFMTL_DIFFUSE, &m[0],&m[1],&m[2]);
    pfGetMtlColor (front, PFMTL_AMBIENT, &m[3],&m[4],&m[5]);
    m[6] = pfGetMtlShininess (front);
    pfGetMtlColor (front, PFMTL_SPECULAR, &m[7],&m[8],&m[9]);
    pfGetMtlColor (front, PFMTL_EMISSION, &m[10],&m[11],&m[12]);
    m[13] = pfGetMtlAlpha (front);

    new.data = (void *) m;
    if (! pfuFindHash (htM, &new))
    {
      new.id = 1;
      pfuEnterHash (htM, &new);
    }
    else
    {
      old = pfuEnterHash (htM, &new);
      old->id++;
    }
  }


  if (back) {
  pfuHashElt new, *old;
  float m[14];

    pfGetMtlColor (back, PFMTL_DIFFUSE, &m[0],&m[1],&m[2]);
    pfGetMtlColor (back, PFMTL_AMBIENT, &m[3],&m[4],&m[5]);
    m[6] = pfGetMtlShininess (back);
    pfGetMtlColor (back, PFMTL_SPECULAR, &m[7],&m[8],&m[9]);
    pfGetMtlColor (back, PFMTL_EMISSION, &m[10],&m[11],&m[12]);
    m[13] = pfGetMtlAlpha (back);

    new.data = (void *) m;
    if (! pfuFindHash (htM, &new))
    {
      new.id = 1;
      pfuEnterHash (htM, &new);
    }
    else
    {
      old = pfuEnterHash (htM, &new);
      old->id++;
    }
  }

  return;
}


static int getTextureIndex (pfTexture *tex)
{
int i;

  for (i=0; i<numTextures; ++i) {
    if (tex == textureTable[i].texture) return i;
  }

  return -1;
}

static int addTexture (texTable *t)
{
  if (getTextureIndex (t->texture) != -1) return 0;

  if (numTextures == availTextures)
  {
    textureTable = pfRealloc (textureTable, availTextures+64);
    availTextures += 64;
  }

  textureTable[numTextures].texture = t->texture;
  textureTable[numTextures].ref = t->ref;
  textureTable[numTextures].detail = t->detail;

  numTextures++;

  return 1;
}


/*
 * Make a list of textures and detail textures used in this scene graph.
 */

void handleTexture (pfGeoSet *gset)
{
pfGeoState *gstate = pfGetGSetGState (gset);
pfTexture *tex, *detail;
int i;

  tex = (pfTexture *) pfGetGStateAttr (gstate, PFSTATE_TEXTURE);
  if (tex == NULL) return;

  if ((i = getTextureIndex (tex)) == -1)
  {
  texTable t;

    t.texture = tex;
    t.ref = 1;
    t.detail = 0;
    addTexture (&t);
  }
  else
  {
    textureTable[i].ref++;
  }

  {
  int level;

    pfGetTexDetail (tex, &level, &detail);
  }

  if (detail)
  {
    if ((i = getTextureIndex (detail)) == -1)
    {
    texTable t;

      t.texture = detail;
      t.ref = 1;
      t.detail = 1;
      addTexture (&t);
    }
    else
    {
      textureTable[i].ref++;
    }
  }

  return;
}

/*
 * This is the first scene graph traversal. This looks for colors,
 * materials and textures used by the geosets and builds tables out of
 * them.
 */

static void recurseSceneGraph (pfNode *node)
{
int i;

  if (pfIsOfType(node, pfGetGeodeClassType()))
  {
      int  nc = pfGetNumGSets (node);
      pfGeoSet *gset;
      int  ptype;

    for (i=0; i<nc; ++i)
    {
    gset = pfGetGSet (node, i);

      ptype = pfGetGSetPrimType (gset);
      /* XXX - these are not handled yet */
      if (ptype == PFGS_POINTS || 
	  ptype == PFGS_LINES || 
	  ptype == PFGS_LINESTRIPS || 
	  ptype == PFGS_FLAT_LINESTRIPS || 
	  ptype == PFGS_POLYS)
	  return;
      handleColor (gset, ptype);
      handleMaterial (gset);
      handleTexture (gset);
    }

    return;
  }

  if (pfIsOfType (node, pfGetGroupClassType()))
  {
  int  nc = pfGetNumChildren (node);

    for (i=0; i<nc; ++i)
    {
      recurseSceneGraph (pfGetChild (node, i));
    }
  }

  return;
}


/*
 * Build a basic DWB group with defaults and return the pointer to the
 * structure.
 */

static dwbGroup *buildBasicDwbGroup (void)
{
static dwbGroup outGroup;

  outGroup.layer = 0;
  outGroup.color = 127;

  outGroup.drawstyle = DS_SOLID;
  outGroup.shademodel = SM_GOURAUD_NON_LIT;

  outGroup.linestyle = 0;
  outGroup.linewidth = 1;

  outGroup.fill_pattern = 0;
  outGroup.texture = -1;
  outGroup.transparency = 1.0f;

  outGroup.material = -1;
  outGroup.use_group_color = FALSE;
  outGroup.use_group_dstyle = FALSE;

  outGroup.surfaceType = -1;

  pfSetVec3 (outGroup.bbox, -1.0f, -1.0f, -1.0f);
  pfSetVec3 (outGroup.bbox+3, 1.0f, 1.0f, 1.0f);

  outGroup.grp_flags.perspective = 0;
  outGroup.grp_flags.region_def = 0;
  outGroup.grp_flags.clip_region = 0;
  outGroup.grp_flags.pages = 0;
  outGroup.grp_flags.renderGrp = 0;
  outGroup.grp_flags.decal = 0;
  outGroup.grp_flags.billboard = 0;
  outGroup.grp_flags.sequence = 0;

  outGroup.gfx_flags.zbuffer = TRUE;
  outGroup.gfx_flags.backface = TRUE;
  outGroup.gfx_flags.texture = FALSE;
  outGroup.gfx_flags.aa_lines = FALSE;
  outGroup.gfx_flags.aa_polys = FALSE;
  outGroup.gfx_flags.concave = FALSE;
  outGroup.gfx_flags.displist = FALSE;

  outGroup.ig_flags.day_object = 0;
  outGroup.ig_flags.night_object = 0;
  outGroup.ig_flags.dusk_object = 0;
  outGroup.ig_flags.shadow_object = 0;
  outGroup.ig_flags.terrain_object = 0;
  outGroup.ig_flags.road_object = 0;

  pfSetVec3 (outGroup.ll, 0.0f, 0.0f, 0.0f);
  pfSetVec3 (outGroup.ur, 0.0f, 0.0f, 0.0f);
  pfSetVec3 (outGroup.center, 0.0f, 0.0f, 0.0f);

  outGroup.udata.ifields[0] = outGroup.udata.ifields[1] = 0;
  outGroup.udata.ffields[0] = outGroup.udata.ffields[1] = 0.0f;

  outGroup.material_binding = MB_PER_GROUP;

  outGroup.decal_type = PFDECAL_BASE_HIGH_QUALITY;

  outGroup.packed_color = 0;

  outGroup.collapsed = 0;

  outGroup.seq_mode = 0;
  outGroup.seq_time = 0.1f;
  outGroup.seq_speed = 1.0f;
  outGroup.seq_nreps = -1;
  outGroup.seq_bgn = 0;
  outGroup.seq_end = -1;
  outGroup.seq_random = 0;

  return &outGroup;
}


/*
 * Build a basic DWB face initialized with defaults and return the pointer
 * to the structure.
 */

static dwbNewFace *buildBasicDwbFace (void)
{
static dwbNewFace outFace;

  outFace.layer = 0;
  outFace.color = 0;

  outFace.numverts = 0;
  outFace.ptype = PT_POLY;          /* poly type is always POLY */

  outFace.linewidth = 1;
  outFace.drawstyle = DS_SOLID;
  outFace.linestyle = 0;
  outFace.back_color = 0;

  outFace.texture = -1;
  outFace.pntsize = 1;

  outFace.ir = 0.0f;

  outFace.flags.vertColors = 0;
  outFace.flags.intersection = 0;

  outFace.udata.ifields[0] = outFace.udata.ifields[1] = 0;
  outFace.udata.ffields[0] = outFace.udata.ffields[1] = 0.0f;

  outFace.packed_color = 0;

  outFace.ig_flags.road = 0;
  outFace.ig_flags.intersection = 0;

  outFace.colapsed = 0;
  outFace.detail_tex = -1;

  outFace.surface_material_code = 0;
  outFace.feature_id = 0;

  outFace.alt_texture = -1;
  outFace.material = -1;

  return &outFace;
}


/*
 * Take care of nodes with type PFTYPE_GROUP
 */

static void handlePFTYPE_GROUP (pfNode *node)
{
dwbGroup *outGroup = buildBasicDwbGroup();
char *name, gname[256];

  writeDwbOpcode (DB_GROUP, sizeof(dwbGroup));
  swapDwbGroup(outGroup);
  fwrite (outGroup, sizeof(dwbGroup), 1, outFile);

  name = (char *) pfGetNodeName (node);
  if (name)
    writeDwbVariableLengthRecord (DB_NAME_STR, name);
  else
  {
    sprintf (gname, "Group%d", gCount);
    writeDwbVariableLengthRecord (DB_NAME_STR, gname);
  }

  gCount++;
  return;
}


/*
 * Take care of nodes with type PFTYPE_DCS
 */

static void handlePFTYPE_DCS (pfNode *node)
{
const pfMatrix *m;

  handlePFTYPE_GROUP (node);
  m = pfGetDCSMatPtr ((pfDCS *) node);

  writeDwbOpcode (DB_MATRIX, sizeof (float)*16);
  swapDwbMatrix(m);
  fwrite (m, sizeof (float), 16, outFile);

  return;
}


/*
 * Take care of nodes with type PFTYPE_SCS
 */

static void handlePFTYPE_SCS (pfNode *node)
{
const pfMatrix *m;

  handlePFTYPE_GROUP (node);
  m = pfGetSCSMatPtr ((pfSCS *) node);

  writeDwbOpcode (DB_MATRIX, sizeof (float)*16);
  swapDwbMatrix(m);
  fwrite (m, sizeof (float), 16, outFile);

  return;
}


/*
 * Take care of nodes with type PFTYPE_LAYER
 * Since the writer writes out a DWB 3.0 file, all the children are groups
 * and the first group is treated as the BASE and all the others are
 * treated as LAYERs.
 */

static void handlePFTYPE_LAYER (pfNode *node)
{
dwbGroup *outGroup = buildBasicDwbGroup();
char *name, gname[256];

  outGroup->grp_flags.decal = 1;
  switch (pfGetLayerMode ((pfLayer *) node))
  {
    case PFDECAL_BASE_FAST: outGroup->decal_type = DWB_DECAL_BASE_FAST; break;
    case PFDECAL_BASE_HIGH_QUALITY: outGroup->decal_type =
DWB_DECAL_BASE_HIGH_QUALITY; break;
    case PFDECAL_BASE_STENCIL: outGroup->decal_type = DWB_DECAL_BASE_STENCIL;
break;
    case PFDECAL_BASE_DISPLACE: outGroup->decal_type = DWB_DECAL_BASE_DISPLACE;
break;
  }

  writeDwbOpcode (DB_GROUP, sizeof (dwbGroup));
  swapDwbGroup(outGroup);
  fwrite (outGroup, sizeof (dwbGroup), 1, outFile);

  name = (char *) pfGetNodeName (node);
  if (name)
    writeDwbVariableLengthRecord (DB_NAME_STR, name);
  else
  {
    sprintf (gname, "Decal%d", decalCount);
    writeDwbVariableLengthRecord (DB_NAME_STR, gname);
  }

  decalCount++;
  return;
}


/*
 * This takes care of nodes with type PFTYPE_SEQUENCE
 * Since DWB doesn't maintain frame time information for each of the
 * children nodes, just use the first child's frametime for all the nodes.
 */

static void handlePFTYPE_SEQUENCE (pfNode *node)
{
dwbGroup *outGroup = buildBasicDwbGroup();
char *name, gname[256];
int  mode, begin, end;
float speed;
int  nReps;

  outGroup->grp_flags.sequence = 1;

  /*
   * Use the first child's frame time to be the frame time for all the
   * other children.
   */

  outGroup->seq_time = (float) pfGetSeqTime ((pfSequence *) node, 0);
  pfGetSeqInterval ((pfSequence *) node, &mode, &begin, &end);
  pfGetSeqDuration ((pfSequence *) node, &speed, &nReps);

  outGroup->seq_mode = mode;
  outGroup->seq_bgn = begin;
  outGroup->seq_end = end;
  outGroup->seq_speed = speed;
  outGroup->seq_nreps = nReps;
  outGroup->seq_random = 0;

  writeDwbOpcode (DB_GROUP, sizeof (dwbGroup));
  swapDwbGroup(outGroup);
  fwrite (outGroup, sizeof (dwbGroup), 1, outFile);

  name = (char *) pfGetNodeName (node);
  if (name)
    writeDwbVariableLengthRecord (DB_NAME_STR, name);
  else
  {
    sprintf (gname, "Sequence%d", seqCount);
    writeDwbVariableLengthRecord (DB_NAME_STR, gname);
  }

  seqCount++;
  return;
}


/*
 * Take care of nodes with type PFTYPE_BILLBOARD
 */

static void handlePFTYPE_BILLBOARD (pfNode *node)
{
dwbGroup *outGroup = buildBasicDwbGroup();
char *name, gname[256];

  outGroup->grp_flags.billboard = 1;

  writeDwbOpcode (DB_GROUP, sizeof (dwbGroup));
  swapDwbGroup(outGroup);
  fwrite (outGroup, sizeof (dwbGroup), 1, outFile);

  name = (char *) pfGetNodeName (node);
  if (name)
    writeDwbVariableLengthRecord (DB_NAME_STR, name);
  else
  {
    sprintf (gname, "Billboard%d", billCount);
    writeDwbVariableLengthRecord (DB_NAME_STR, gname);
  }

  billCount++;
  return;
}


/*
 * Take care of nodes with type PFTYPE_LOD
 * Since there's a conceptual difference between Performer LODs and DWB
 * Switches, this creates a dwbSwitch node for each of the LOD node's
 * children and sets the switchin and switchout ranges using Performer's
 * LODrange values.
 */

static void handlePFTYPE_LOD (pfNode *node, int i)
{
dwbSwitch outSwitch;
char *name, gname[256];
int j;

  outSwitch.layer = 0;
  outSwitch.spare = 0;

  outSwitch.switchtype = ST_DISTANCE;
  outSwitch.threshold = 0;

  outSwitch.switchin = pfGetLODRange ((pfLOD *) node, i+1);
  outSwitch.switchout = pfGetLODRange ((pfLOD *) node, i);

  pfGetLODCenter ((pfLOD *) node, outSwitch.center);

  outSwitch.udata.ifields[0] = outSwitch.udata.ifields[1] = 0;
  outSwitch.udata.ffields[0] = outSwitch.udata.ffields[1] = 0.0f;

  for (j=0; j<12; ++j) outSwitch.spare2[j] = 0;

  writeDwbOpcode (DB_SWITCH, sizeof (dwbSwitch));
  swapDwbSwitch(&outSwitch);
  fwrite (&outSwitch, sizeof (dwbSwitch), 1, outFile);

  name = (char *) pfGetNodeName (node);
  if (name)
    writeDwbVariableLengthRecord (DB_NAME_STR, name);
  else
  {
    sprintf (gname, "LOD%d_%d", LODCount, i);
    writeDwbVariableLengthRecord (DB_NAME_STR, gname);
  }

  LODCount++;
  return;
}


static void handlePFTYPE_LIGHTPOINT (pfNode *node)
{
dwbNewFace *outFace = buildBasicDwbFace ();
dwbLightPt lp;
char *name, gname[256];
int i;
int  dir;
float henv, venv, falloff;

  outFace->numverts = pfGetNumLPoints ((pfLightPoint *) node);
  outFace->ptype = 4; /* light point */
  outFace->color = 127;

  writeDwbOpcode (DB_FACE, sizeof (dwbNewFace));
  swapDwbNewFace(outFace);
  fwrite (outFace, sizeof (dwbNewFace), 1, outFile);
  swapDwbNewFace(outFace); /* swap back, this get referenced later */

  name = (char *) pfGetNodeName (node);
  if (name)
    writeDwbVariableLengthRecord (DB_NAME_STR, name);
  else
  {
    sprintf (gname, "lp%d", lpntCount);
    writeDwbVariableLengthRecord (DB_NAME_STR, gname);
  }

  lpntCount++;

  pfGetLPointShape ((pfLightPoint *) node, &dir, &henv, &venv, &falloff);

  lp.ltype = 1;    /* RUNWAY */
  switch (dir)
  {
    case PFLP_OMNIDIRECTIONAL:
      lp.directionality = DB_OMNIDIRECTIONAL;
    break;

    case PFLP_UNIDIRECTIONAL:
      lp.directionality = DB_UNIDIRECTIONAL;
    break;

    case PFLP_BIDIRECTIONAL:
      lp.directionality = DB_BIDIRECTIONAL;
    break;
  }

  lp.shape = 1;    /* single point */
  lp.suppress_last_light = 0;
  lp.lwidth = henv;
  lp.lheight = venv;
  lp.diam = pfGetLPointSize ((pfLightPoint *) node);

  {
  pfMatrix m;
  float h, p, r;
  pfVec3 v;

    pfSetVec3 (v, 0.0f, 1.0f, 0.0f);
    pfGetLPointRot ((pfLightPoint *) node, &h, &p, &r);
    pfMakeEulerMat (m, h, p, r);
    pfXformVec3 (v, v, m);
    lp.dir[0] = v[0];
    lp.dir[1] = v[1];
    lp.dir[2] = v[2];
  }

  lp.intensity = 1.0;
  lp.intensity_variance = 0.0;
  lp.calligraphic_priority = 0.0f;

  writeDwbOpcode (DB_LIGHT_PT, sizeof (dwbLightPt));
  swapDwbLightPt(&lp);
  fwrite (&lp, sizeof (dwbLightPt), 1, outFile);
  swapDwbLightPt(&lp); /* swap back, gets referenced later */

  dwbPush();
  writeDwbOpcode (DB_VERTEX, sizeof (dwbVertex)*outFace->numverts);

    for (i=0; i<outFace->numverts; ++i)
    {
    dwbVertex v;
    pfVec4 clr;
    pfVec3 c;
    int k[3];
    pfuHashElt n, *o;

      pfGetLPointColor ((pfLightPoint *) node, i, clr);
      k[0] = (int) ((double) clr[0]*255);
      k[1] = (int) ((double) clr[1]*255);
      k[2] = (int) ((double) clr[2]*255);

      n.data = (void *) k;
      o = pfuEnterHash (htC, &n);
      if (o)
	v.color = (short) o->id;
      else
      {
	o = pfuEnterHash (htC, &n);
	v.color = o->id;
      }

      pfGetLPointPos ((pfLightPoint *) node, i, c);
      v.coords[0] = c[0];
      v.coords[1] = c[2];
      v.coords[2] = -c[1];

      pfCopyVec3 (v.normal, lp.dir);
      pfSetVec4 (v.tex_coords, 0.0f, 0.0f, 0.0f, 0.0f);
		swapDwbVertex(&v);
      fwrite (&v, sizeof (dwbVertex), 1, outFile);
    }
  dwbPop();

  return;
}


static int getMaterialIndex (pfMaterial *mat)
{
float m[14];
pfuHashElt new, *old;
int mindex;

    if (mat == NULL || htM->listCount == 0) return -1;

    pfGetMtlColor (mat, PFMTL_DIFFUSE, &m[0],&m[1],&m[2]);
    pfGetMtlColor (mat, PFMTL_AMBIENT, &m[3],&m[4],&m[5]);
    m[6] = pfGetMtlShininess (mat);
    pfGetMtlColor (mat, PFMTL_SPECULAR, &m[7],&m[8],&m[9]);
    pfGetMtlColor (mat, PFMTL_EMISSION, &m[10],&m[11],&m[12]);
    m[13] = pfGetMtlAlpha (mat);

    new.data = (void *) m;
    old = pfuEnterHash (htM, &new);
    if (old)
    {
      mindex = old->id;
    }
    else
    {
      fprintf (stderr, "Couldn't find material in HT\n");
      mindex = -1;
    }

  return mindex;
}


/*
 * Takes care of Performer Geosets.
 * Since all the primitives under a geoset share the same material, this
 * sets the group level material by looking the material Hash Table.
 *
 */

static void handleGEOSET (pfGeoSet *gset)
{
dwbGroup *outGroup = buildBasicDwbGroup();
char gname[256];
pfGeoState *gstate = pfGetGSetGState (gset);
pfMaterial *m = pfGetGStateAttr (gstate, PFSTATE_FRONTMTL);

  outGroup->grp_flags.renderGrp = 1;
  outGroup->collapsed = 1;
  outGroup->material = getMaterialIndex (m);

  writeDwbOpcode (DB_GROUP, sizeof (dwbGroup));
  swapDwbGroup(outGroup);
  fwrite (outGroup, sizeof (dwbGroup), 1, outFile);
  sprintf (gname, "Group%d", gCount);
  writeDwbVariableLengthRecord (DB_NAME_STR, gname);

  dwbPush();
  handlePrimitives (gset);
  dwbPop();

  gCount++;
  return;
}


/*
 * returns the number of vertices required to draw the primitive as
 * triangles or polygons
 */

static int  getVertexCount (int  pType)
{
int  vCount = 0;

  switch (pType)
  {
    case PFGS_TRIS:
    case PFGS_TRISTRIPS:
    case PFGS_FLAT_TRISTRIPS:
      vCount = 3;
    break;

    case PFGS_QUADS:
      vCount = 4;
    break;
  }

  return vCount;
}


static unsigned int  packRGB (float fr, float fg, float fb)
{
short i,r,g,b,a;
unsigned int  pcolor = 0;

  r = (short)(fr * 255.0);
  g = (short)(fg * 255.0);
  b = (short)(fb * 255.0);
  a = 255;
  pcolor =  ( (a << 24) + ( b << 16 ) + ( g << 8 ) + r );
  return (pcolor);
}


static unsigned int
getFaceColor (pfVec4 *calist, unsigned short *cilist, int  cbind, int i)
{
int a,c;
unsigned int  cindex;

  switch (cbind)
  {
    case PFGS_OFF: c = -2; a = 0; break;

    case PFGS_OVERALL: c = 1; a = 0; break;

    case PFGS_PER_PRIM: c = i; a = 1; break;

    case PFGS_PER_VERTEX: c = -1; break;
  }

  if (c >= 0)
  {
  pfuHashElt n, *o;
  int k[3];
  int index;

    if (cilist) index = cilist[a*c];
    else index = a*c;

    k[0] = (int) ((double) calist[index][0]*255);
    k[1] = (int) ((double) calist[index][1]*255);
    k[2] = (int) ((double) calist[index][2]*255);

    n.data = (void *) k;
    o = pfuEnterHash (htC, &n);
    if (o)
    {
      cindex = o->id;
    }
    else {
      o = pfuEnterHash (htC, &n);
      cindex = o->id;
    }
  }
  else
  {
    cindex = 127;
  }

  return cindex;
}


/*
 * Builds a list of dwbVertex structures, using the geoset and primitive
 * type. This handles PFGS_TRIS and PFGS_QUADS.
 * This returns the number of faces that makes up each primitive.
 *   - 1 for both TRIS and QUADS.
 */

static int buildTRISandQUADS (pfGeoSet *gset, int  pType, pfList *list)
{
int  num = pfGetGSetNumPrims (gset);
pfVec3 *valist, *nalist;
pfVec2 *talist;
pfVec4 *calist;
unsigned short *vilist, *nilist, *tilist, *cilist;
int  numVerts = (pType == PFGS_TRIS) ? 3 : 4;
int  cbind, tbind, nbind;
pfuHashElt n, *o;
int i,j;

  pfGetGSetAttrLists (gset, PFGS_COORD3, (void **) &valist, &vilist);
  pfGetGSetAttrLists (gset, PFGS_NORMAL3, (void **) &nalist, &nilist);
  pfGetGSetAttrLists (gset, PFGS_TEXCOORD2, (void **) &talist, &tilist);
  pfGetGSetAttrLists (gset, PFGS_COLOR4, (void **) &calist, &cilist);

  cbind = pfGetGSetAttrBind (gset, PFGS_COLOR4);
  nbind = pfGetGSetAttrBind (gset, PFGS_NORMAL3);
  tbind = pfGetGSetAttrBind (gset, PFGS_TEXCOORD2);

  if (pType == PFGS_TRIS) tris += num; else quads += num;

  for (i=0; i<num; ++i)
  {
    for (j=0; j<numVerts; ++j)
    {
    dwbVertex *v;
    int index;

      v = pfCalloc (1, sizeof (dwbVertex), pfGetSharedArena());

      index = (vilist) ? vilist[numVerts*i+j] : numVerts*i+j;

      PFCOPY_VEC3 (v->coords, valist[index]);

      switch (cbind)
      {
	case PFGS_PER_VERTEX:
	{
	int k[3];

	  index = (cilist) ? cilist[numVerts*i+j] : numVerts*i+j;

          k[0] = (int) ((double) calist[index][0]*255);
          k[1] = (int) ((double) calist[index][1]*255);
          k[2] = (int) ((double) calist[index][2]*255);

          n.data = (void *) k;
          o = pfuEnterHash (htC, &n);
          if (o) v->color = o->id;
          else {
	    o = pfuEnterHash (htC, &n);
	    v->color = o->id;
	  }
	}
	break;

	default:
	  v->color = getFaceColor (calist,cilist,cbind,i);
        break;
      }

      switch (tbind)
      {
	case PFGS_PER_VERTEX:
	  index = (tilist) ? tilist[numVerts*i+j] : numVerts*i+j;
	  v->tex_coords[0] = talist[index][0];
	  v->tex_coords[1] = talist[index][1];
        break;
      }

      switch (nbind)
      {
	case PFGS_PER_VERTEX:
	  index = (nilist) ? nilist[numVerts*i+j] : numVerts*i+j;
	  PFCOPY_VEC3 (v->normal, nalist[index]);
        break;

	case PFGS_PER_PRIM:
	  index = (nilist) ? nilist[i] : i;

	  PFCOPY_VEC3 (v->normal, nalist[index]);
	break;

	case PFGS_OVERALL:
	  index = (nilist) ? nilist[0] : 0;

	  PFCOPY_VEC3 (v->normal, nalist[index]);
	break;
      }

      pfAdd (list, (void *) v);
    }
  }

  return 1;
}


/*
 * This builds a list of dwbVertex using the geoset and the primitive type
 * of the geoset. This handles PFGS_TRISTRIPS and PFGS_FLAT_TRISTRIPS.
 * This returns the total number of faces (triangles) required to build
 * the tristrip.
 * Since DWB doesn't support strip primitives, this triangulates the strip
 * and returns the invidual vertices required to build the strip.
 */

static int buildTRISTRIPS (pfGeoSet *gset, int  pType, pfList *list)
{
int  numPrims = pfGetGSetNumPrims (gset);
pfVec3 *valist, *nalist;
pfVec2 *talist;
pfVec4 *calist;
unsigned short *vilist, *nilist, *tilist, *cilist;
int  numVerts, numTris;
int  cbind, tbind, nbind;
pfuHashElt n, *o;
int i,tri, ver;
int result;

  pfGetGSetAttrLists (gset, PFGS_COORD3, (void **) &valist, &vilist);
  pfGetGSetAttrLists (gset, PFGS_NORMAL3, (void **) &nalist, &nilist);
  pfGetGSetAttrLists (gset, PFGS_TEXCOORD2, (void **) &talist, &tilist);
  pfGetGSetAttrLists (gset, PFGS_COLOR4, (void **) &calist, &cilist);

  cbind = pfGetGSetAttrBind (gset, PFGS_COLOR4);
  nbind = pfGetGSetAttrBind (gset, PFGS_NORMAL3);
  tbind = pfGetGSetAttrBind (gset, PFGS_TEXCOORD2);


  if (pType == PFGS_TRISTRIPS)
  {
    tristrips += numPrims;
  }
  else
  {
    flattristrips += numPrims;
  }

  result = numVerts = 0;

  for (i=0; i<numPrims; ++i)
  {
  int flip = 0;
  int len = pfGetGSetPrimLength(gset, i);

    numTris = len - 2;
    result += numTris;

    for (tri=0; tri<numTris; ++tri)
    {
    int lkup[3] = {1, 0, 2};

      for (ver=0; ver<3; ver++)
      {
      int w = (flip) ? lkup[ver] : ver;
      dwbVertex *v;
      int index;

        v = pfCalloc (1, sizeof (dwbVertex), pfGetSharedArena());

        index = (vilist) ? vilist[numVerts+tri+w] : numVerts+tri+w;

	PFCOPY_VEC3 (v->coords, valist[index]);

        switch (cbind)
        {
	  case PFGS_PER_VERTEX:
	  {
	  int k[3];

	    if (pType == PFGS_TRISTRIPS)
	      index = (cilist) ? cilist[numVerts+tri+w] : numVerts+tri+w;
            else
	      index = (cilist) ? cilist[result-numTris+tri]
			       : result-numTris+tri;

            k[0] = (int) ((double) calist[index][0]*255);
            k[1] = (int) ((double) calist[index][1]*255);
            k[2] = (int) ((double) calist[index][2]*255);

            n.data = (void *) k;
            o = pfuEnterHash (htC, &n);
            if (o) v->color = o->id;
            else {
	      o = pfuEnterHash (htC, &n);
	      v->color = o->id;
	    }
	  }
	  break;

  	  default:
	    v->color = getFaceColor (calist,cilist,cbind,i);
          break;
        }

        switch (tbind)
        {
	  case PFGS_PER_VERTEX:
	    index = (tilist) ? tilist[numVerts+tri+w] : numVerts+tri+w;
	    v->tex_coords[0] = talist[index][0];
	    v->tex_coords[1] = talist[index][1];
          break;
        }

        switch (nbind)
        {
	  case PFGS_PER_VERTEX:
	    if (pType == PFGS_TRISTRIPS)
	      index = (nilist) ? nilist[numVerts+tri+w] : numVerts+tri+w;
	    else
	      index = (nilist) ? nilist[result-numTris+tri]
			       : result-numTris+tri;

	    PFCOPY_VEC3 (v->normal, nalist[index]);
          break;

	  case PFGS_PER_PRIM:
	    index = (nilist) ? nilist[i] : i;

	    PFCOPY_VEC3 (v->normal, nalist[index]);
	  break;

	  case PFGS_OVERALL:
	    index = (nilist) ? nilist[0] : 0;

	    PFCOPY_VEC3 (v->normal, nalist[index]);
	  break;
        }

        pfAdd (list, (void *) v);
      }

      flip = !flip;
    }

    numVerts += len;
  }

  return 1;
}


static int getFaceCount (pfGeoSet *gset, int i, int  pType)
{
int faceCount = 0;

  switch (pType)
  {
    case PFGS_TRIS:
    case PFGS_QUADS:
      faceCount = 1;
    break;

    case PFGS_TRISTRIPS:
    case PFGS_FLAT_TRISTRIPS:
      faceCount = pfGetGSetPrimLength(gset, i) - 2;
    break;
  }

  return faceCount;
}


/*
 * For each geoset that is passed, this builds a list of vertices that
 * would make up all the primitives in the geoset.
 * Since all the primitives in the geoset use the same texture, this also
 * sets the dwbFace.texture and dwbFace.detail_tex indices.
 *
 * First this builds a list of dwbVertices.
 * Then
 * for each primitive
 * {
 *   for the number of dwbFace's that would make up the primitive
 *   {
 *     dwbPush();
 *       write out the vertices
 *     dwbPop();
 *   }
 * }
 */

static void handlePrimitives (pfGeoSet *gset)
{
int  numPrims = pfGetGSetNumPrims (gset);
int  pType = pfGetGSetPrimType (gset);
int  cbind = pfGetGSetAttrBind (gset, PFGS_COLOR4);
pfVec4 *calist;
unsigned short *cilist;
int i,j,k, totalVerts;
pfGeoState *gstate = pfGetGSetGState (gset);
pfTexture *detail, *tex = pfGetGStateAttr (gstate, PFSTATE_TEXTURE);
int  tindex, dindex;
char pname[256];
pfList *list;
int ok=0;
int numFaces = 0;
pfMatrix *xform = NULL;
int depth = pfGetMStackDepth (mstack);

  if (depth > 1)
  {
    xform = pfGetMStackTop (mstack);
  }

  pfGetGSetAttrLists (gset, PFGS_COLOR4, (void **) &calist, &cilist);

  if (tex)
  {
  int level;

    tindex = getTextureIndex (tex);

    pfGetTexDetail (tex, &level, &detail);
    if (detail)
      dindex = getTextureIndex (detail);
    else
      dindex = -1;
  }
  else
    tindex = -1;

  list = pfNewList (sizeof (dwbVertex *), 100, pfGetSharedArena());
  switch (pType)
  {
    case PFGS_TRIS:
    case PFGS_QUADS:
      ok = buildTRISandQUADS (gset, pType, list);
    break;

    case PFGS_TRISTRIPS:
    case PFGS_FLAT_TRISTRIPS:
      ok = buildTRISTRIPS (gset, pType, list);
    break;

    default:
      ok = 0;
    break;
  }

  totalVerts = 0;

  if (ok)
  {
    for (i=0; i<numPrims; ++i)
    {
    dwbNewFace *outFace;
    unsigned int  cindex;

      numFaces = getFaceCount (gset, i, pType);

      for (k=0; k<numFaces; ++k)
      {
        outFace = buildBasicDwbFace();
        outFace->numverts = (short) getVertexCount (pType);

        cindex = getFaceColor (calist, cilist, cbind, i);
        outFace->color = cindex;
        outFace->back_color = cindex;

        outFace->texture = tindex;
        outFace->detail_tex = dindex;

        writeDwbOpcode (DB_FACE, sizeof(dwbNewFace));
		swapDwbNewFace(outFace);
        fwrite (outFace, sizeof(dwbNewFace), 1, outFile);
		swapDwbNewFace(outFace); /* swap back, gets referenced later */
        sprintf (pname, "p%d", pCount++);
        writeDwbVariableLengthRecord (DB_NAME_STR, pname);

        dwbPush();

	/*
	 * Somehow DWB doesn't understand a mixture of DB_VERTEX and
	 * DB_PACKED_VERTEX under a DB_FACE level.
	 * So just write 'numverts' vertices (with color index) after
	 * the DB_VERTEX opcode record.
	 */

          writeDwbOpcode (DB_VERTEX, sizeof (dwbVertex)*outFace->numverts);

          for (j=totalVerts; j<totalVerts+outFace->numverts; ++j)
          {
          dwbVertex *v = pfGet (list, j);
			if(v == NULL) continue;
	    /*
	     * If the matrix stack has more than one entry, then take the
	     * matrix on the top of the stack and apply that to the vertex
	     * before writing it out.
	     */

	    if (depth > 1)
	    {
	      pfXformPt3 (v->coords, v->coords, *xform);
	      pfXformVec3 (v->normal, v->normal, *xform);
	    }
		swapDwbVertex(v); 
	    fwrite (v, sizeof (dwbVertex), 1, outFile);
          }

        dwbPop();

        totalVerts += outFace->numverts;
      }
    }

    for (i=0; i<pfGetNum (list); ++i) pfFree (pfGet (list, i));
  }
  pfDelete (list);
}


/*
 * Second recursion through the scene graph to extract the geometry in the
 * Performer geosets.
 */

static void extractGeometry (pfNode *node)
{
int i;
pfMatrix mat;

#if 0
  if (
	pfIsOfType(node, pfGetGroupClassType())
     )
  {
    handlePFTYPE_GROUP (node);
  }
#endif

  /*
   * Handle the special case where the node has a matrix associated with
   * it. In that case, add that matrix to the matrix stack and use the
   * matrix to transform the vertices
   */

  if (pfIsOfType(node, pfGetDCSClassType()))
  {
    handlePFTYPE_GROUP (node);

    pfGetDCSMat ((pfDCS *) node, mat);
    pfPushMStack (mstack);
    pfPreMultMStack (mstack, mat);
  }
  else if (pfIsOfType(node, pfGetSCSClassType()))
  {
    handlePFTYPE_GROUP (node);

    pfGetSCSMat ((pfSCS *) node, mat);
    pfPushMStack (mstack);
    pfPreMultMStack (mstack, mat);
  }
  else if (pfIsOfType(node, pfGetLayerClassType())) handlePFTYPE_LAYER (node);
  else if (pfIsOfType(node, pfGetSeqClassType())) handlePFTYPE_SEQUENCE (node);
  else if (pfIsOfType(node, pfGetBboardClassType()))
  {
  int  nc = pfGetNumGSets (node);

    handlePFTYPE_BILLBOARD (node);
    dwbPush();
      for (i=0; i<nc; ++i)
	handleGEOSET (pfGetGSet (node, i));
    dwbPop();
  }
  else if (pfIsOfType(node, pfGetGeodeClassType()))
  {
  int  nc = pfGetNumGSets (node);

    for (i=0; i<nc; ++i)
      handleGEOSET (pfGetGSet (node, i));
  }
  else if (pfIsOfType(node, pfGetLPointClassType())) handlePFTYPE_LIGHTPOINT (node);
  else if (pfIsOfType (node, pfGetGroupClassType()))
  {
    handlePFTYPE_GROUP(node);
  }


  if (pfIsOfType (node, pfGetGroupClassType()))
  {
  int  nc = pfGetNumChildren (node);

    if (!pfIsOfType(node, pfGetLODClassType())) dwbPush ();

    for (i=0; i<nc; ++i)
    {
      if (pfIsOfType(node, pfGetLODClassType()))
      {
	handlePFTYPE_LOD (node, i);
	dwbPush();
      }

      extractGeometry (pfGetChild (node, i));

      if (pfIsOfType(node, pfGetLODClassType())) dwbPop();
    }

    if (!pfIsOfType(node, pfGetLODClassType()))
    {
      dwbPop();
      if (pfIsOfType(node, pfGetSCSClassType()))
      {
	pfPopMStack (mstack);
      }
    }
  }

  return;
}


static void buildColMatTexTables (pfNode *root)
{
int i, j, k;

  if (verbose)
    fprintf (stderr, "Building color, material and texture Tables\n");

    htM = pfuNewHTable (100, 14*sizeof (float), pfGetSharedArena());
    htC = pfuNewHTable (100, 3*sizeof (int), pfGetSharedArena());

    textureTable = pfCalloc (64, sizeof (texTable), pfGetSharedArena());
    availTextures = 64;
    numTextures = 0;

    if (htM == NULL || htC == NULL || textureTable == NULL)
    {
      fprintf (stderr, "Couldn't create hash table\n"); exit (1);
    }

    recurseSceneGraph (root);

    if (verbose)
      fprintf (stderr, "  Found [%d] unique colors\n", htC->listCount);

    if (htC->listCount < 64) j = htC->listCount;
    else
    {
      if (verbose) fprintf (stderr, "    Using 64 most used colors\n");
      j = 64;
    }

    /*
     * sort them in decreasing order of most-used colors
     * and load them in the fixed part of the colorTable
     */

    qsort ((void *) htC->list, htC->listCount, sizeof (pfuHashElt *), comp);

    for (i=0; i<j; ++i)
    {
    int p[3];

      bcopy ((int *) htC->list[i]->data, p, 3*sizeof(int));
      colorTable[31+i][0] = p[0];
      colorTable[31+i][1] = p[1];
      colorTable[31+i][2] = p[2];
    }

    pfuDelHTable (htC);
    htC = pfuNewHTable (64+30*128, 3*sizeof (int), pfGetSharedArena());

    /*
     * Load the new colors in the hash table for future references.
     * First generate the shaded colors using the ramp function used by
     * DWB to fill up the hashTable. Try to override the fixed colors
     * with these guys, so that DWB can calculate color shading for polygons.
     */

    for (i=0; i<30; ++i)
    {
    pfuHashElt x;
    int intensity;

      for (j=0; j<128; ++j)
      {
	intensity = 127 - j;
	shaded[i][j][0] = (colorTable[i][0]*j)/127;
	shaded[i][j][1] = (colorTable[i][1]*j)/127;
	shaded[i][j][2] = (colorTable[i][2]*j)/127;

        x.id = i*128+j;
	x.data = (void *) shaded[i][j];
	pfuEnterHash (htC, &x);
      }
    }

    for (i=0; i<64; ++i)
    {
    pfuHashElt x;

      /*
       * The start of the fixed part of the color table is 4096 in DWB
       */

      x.id = 4096 + i;
      x.data = (void *) colorTable[31+i];
      pfuEnterHash (htC, &x);
    }

    if (verbose)
      fprintf (stderr, "  Found [%d] unique materials\n", htM->listCount);
    k = 0;
    for (i=0; i<numTextures; ++i)
    {
    texTable *t = textureTable;

      if (verbose)
        fprintf (stderr, "  [%d]:%s\n", t[i].detail,
			 pfGetTexName (t[i].texture));
      if (t[i].detail) ++k;
    }
    if (verbose) {
      fprintf (stderr, "  Found [%d] unique textures\n", numTextures-k);
      fprintf (stderr, "  Found [%d] unique detail textures\n", k);
      fprintf (stderr, "Done\n");
    }
}

#ifdef MAIN
static void Usage (void)
{
  fprintf (stderr,
    "Usage: pftodwb [-dvh] inFile.someKnownFormat outFile.dwb...\n");
  fprintf (stderr, "  -d: display the loaded file\n");
  fprintf (stderr, "  -v: verbose mode\n");
  fprintf (stderr, "  -h: display Usage\n");
  exit(1);
}


static int  processOptions (int argc, char *argv[])
{
int  opt;
extern int optind;

  while ((opt = getopt (argc, argv, "dvh")) != EOF)
  {
    switch (opt)
    {
      case 'd': displayMode = 1; break;
      case 'h': Usage(); exit (1); break;
      case 'v': verbose = 1; break;
    }
  }

  return optind;
}

char *getTail (char *a)
{
int i,l;

  l = strlen (a);
  for (i=l-1; i>=0; --i)
    if (a[i] == '/') return a+i+1;
  return a;
}

int main (int argc, char *argv[])
{
    pfNode	*root;
    pfPipe      *p;
    pfScene     *scene;
    pfChannel   *chan;
    int         arg;

    fprintf (stderr, "%s", pftodwb_banner);

    arg = processOptions (argc, argv);
    if (argc - arg < 2) Usage();

    if (! displayMode)
    {
      pfInit();
      pfConfig();
      pfNewScene ();
      pfNewChan(pfGetPipe(0));
    }
    else
    {
    pfPipeWindow *win;
    GLint temp[2];

      pfInit();
      pfMultiprocess (PFMP_APPCULLDRAW);
      pfConfig();

      scene = pfNewScene();

      sprintf (winTitle, "Writer: [%s] -> [%s]", getTail(argv[arg]),
			    getTail(argv[arg+1]));
      p = pfGetPipe (0);

      win = pfNewPWin(p);
      pfPWinName (win, winTitle);
      pfPWinType (win, PFWIN_TYPE_X);

      {
      int x,y, s;
      Display *d = pfGetCurWSConnection ();

	s = DefaultScreen (d);
	x = DisplayWidth (d, s);
	y = DisplayHeight (d, s);
        pfPWinOriginSize (win, x/4, y/4, 2*x/4, 2*y/4);
      }

      pfPWinConfigFunc (win, initFuncWithWindow);
      pfConfigPWin (win);

      chan = pfNewChan (p);
      pfChanScene (chan, scene);
      pfChanNearFar (chan, 1.0f, 10000.0f);
      pfChanFOV (chan, 45.0f, 0.0f);
    }

    if ((root = pfdLoadFile (argv[arg++])) == NULL)
    {
      pfExit();
      exit(-1);
    }

    if ((outFile = fopen (argv[arg++], "w")) == NULL)
    {
      fprintf (stderr, "Error opening %s for writing\n", argv[arg-1]);
      pfExit ();
      exit (-1);
    }
#else
PFDWB_DLLEXPORT int
pfdStoreFile_dwb (pfNode *root, const char *fileName)
{
    displayMode = 0;
    verbose = 0;
    level;
    *outFile;
    pCount=1, gCount=1, LODCount=1;
    swCount = 1;
    seqCount = 1;
    decalCount = 1;
    billCount = 1;
    lpntCount = 1;
    tris=0, quads=0, tristrips=0, flattristrips=0;
    cPerVertex = 0;

  outFile = fopen (fileName, "w");

  if (! outFile)
  {
    pfNotify (
      PFNFY_WARN, PFNFY_RESOURCE,
      "Error opening %s for writing\n", fileName
    );
    return -1;
  }
#endif

    cPerVertex = 0;
    buildColMatTexTables ((pfNode *) root);

#ifdef MAIN
    writeDwbHeader ((pfNode *) root, argv[2]);
#else
    writeDwbHeader ((pfNode *) root, fileName);
#endif

    if (htC->listCount)
      writeDwbColorTable ();

    if (numTextures)
      writeDwbTextureTable ();

    if (htM->listCount)
      writeDwbMaterialTable();

#ifdef MAIN
    if (verbose) fprintf (stderr, "Extracting geometry...");
#else
    pfNotify (
      PFNFY_DEBUG, PFNFY_PRINT,
      "pfdStore_dwb: Extracting geometry..."
    );
#endif

      mstack = pfNewMStack (32, pfGetSharedArena());

      dwbPush();
      extractGeometry ((pfNode *) root);
      dwbPop();

      pfDelete (mstack);

#ifdef MAIN
    if (verbose) fprintf (stderr, "Done\n");

    if (verbose)
    {
      fprintf (stderr, "Found %d Groups\n", gCount-1);
      fprintf (stderr, "Found %d Decals\n", decalCount-1);
      fprintf (stderr, "Found %d Billboards\n", billCount-1);
      fprintf (stderr, "Found %d Sequences\n", seqCount-1);
      fprintf (stderr, "Found %d LODs\n", LODCount-1);
      fprintf (stderr, "Fount %d LightPoints\n", lpntCount-1);

      fprintf (stderr, "\n      %d TRIS\n", tris);
      fprintf (stderr, "      %d QUADS\n", quads);
      fprintf (stderr, "      %d TRISTRIPS\n", tristrips);
      fprintf (stderr, "      %d FLATTRISTRIPS\n", flattristrips);
    }

#else
    pfNotify (
      PFNFY_DEBUG, PFNFY_PRINT,
      "pfdStore_dwb: Extraction complete\n"
    );

    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Found %d Groups\n", gCount-1);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Found %d Decals\n", decalCount-1);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Found %d Billboards\n", billCount-1);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Found %d Sequences\n", seqCount-1);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Found %d LODs\n", LODCount-1);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Found %d LightPoints\n", lpntCount-1);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "      %d TRIS\n", tris);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "      %d QUADS\n", quads);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "      %d TRISTRIPS\n", tristrips);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "      %d FLATTRISTRIPS\n",
flattristrips);
#endif

   fclose (outFile);

#ifdef MAIN
    if (displayMode)
    {
    float t = 0.0f;
    pfSphere sphere;
    float r;
    pfLightSource *l;
    pfBox bbox;
    pfCoord view;
    pfDCS *dcs = pfNewDCS();
    pfMatrix m;

      pfAddChild (scene, dcs);
      pfAddChild (dcs, root);

      pfuTravCalcBBox(scene, &bbox);

      /* find max dimension */
      r = bbox.max[PF_X] - bbox.min[PF_X];
      r = PF_MAX2(r, bbox.max[PF_Y] - bbox.min[PF_Y]);
      r = PF_MAX2(r, bbox.max[PF_Z] - bbox.min[PF_Z]);

      pfAddVec3 (view.xyz, bbox.min, bbox.max);
      pfScaleVec3 (view.xyz, 0.5f, view.xyz);
      pfMakeTransMat (m, -view.xyz[PF_X], -view.xyz[PF_Y], -view.xyz[PF_Z]);
      pfDCSMat (dcs, m);

      view.xyz[PF_Y] -= r;
      view.xyz[PF_Z] += 0.25f*r;
      pfSetVec3 (view.hpr,
	  0.0f,
	  90.0f*fatan (view.xyz[PF_Z]/view.xyz[PF_Y])/3.14159,
	  0.0f
      );

      l = pfNewLSource();
      pfAddChild (scene, l);
      pfLSourceColor (l, 0, 1.0f, 1.0f, 0.8f);
      pfLSourcePos (l, 0.0f, 1.0f, 0.0f, 0.0f);

      pfChanView (chan, view.xyz, view.hpr);
      pfChanNearFar (chan, 1.0f, 2*r);

      pfInitClock (0.0f);
      while (t < 20.0f)
      {
      float s,c;
      pfCoord view;

	pfSync();

	t = pfGetTime();
	pfSinCos (45.0f*t, &s, &c);
	pfDCSRot (dcs, 45.0f*t, 0.0f, 0.0f);
	pfFrame();
      }
    }
    pfExit();
    return 0;
#else
  return 1;
#endif
}


static int writeDwbHeader (pfNode *node, const char *filename)
{
int x;
dwbHeader outHeader;

  if (verbose) fprintf (stderr, "Writing DWB Header...");
  outHeader.units = UT_METERS;
  outHeader.version  = 3.0f;
  strcpy (outHeader.lastrev, getCurrentDateTime());
  for ( x=0; x<6; x++ ) outHeader.next_node_ids[x] = 99;

  for ( x=0; x<3; x++ ) outHeader.bbox[x] = 1.0;
  for ( x=3; x<6; x++ ) outHeader.bbox[x] = -1.0;

  outHeader.drawstyle   = DS_SOLID;
  outHeader.movestyle   = DS_SOLID;

  /*
   * If we had found materials in the scene graph, then set the shademodel
   * to illuminated, otherwise just gouraud would suffice.
   */

  if (htM->listCount)
    outHeader.shademodel  = SM_ILLUMINATED;
  else
    outHeader.shademodel  = SM_GOURAUD_NON_LIT;

  outHeader.zbuffer     = TS_GLOBAL_ON;
  outHeader.backface    = TS_GLOBAL_ON;
  outHeader.concave     = TS_GLOBAL_OFF;

  outHeader.flags.invalid_view_hint = 1;

  for (x=0; x<2; x++) {
    outHeader.udata.ifields[x] = 0;
    outHeader.udata.ffields[x] = 0.0f;
  }

  outHeader.color    = 0;

  if (cPerVertex)
    outHeader.material_binding = MB_PER_VERTEX;
  else
    outHeader.material_binding = MB_PER_FACE;

  outHeader.transparency     = TS_GLOBAL_OFF;
  outHeader.texture          = TS_GLOBAL_OFF;
  outHeader.up_axis          = Z_UP;

  writeDwbOpcode (DB_HEADER, sizeof(dwbHeader));
  swapDwbHeader(&outHeader);
  fwrite (&outHeader, sizeof(dwbHeader), 1, outFile);
  writeDwbVariableLengthRecord (DB_NAME_STR, filename);
  if (verbose) fprintf (stderr, "Done\n");
}


static int writeDwbColorTable (void)
{
dwbColorTable outColorT;
int i;

  if (verbose) fprintf (stderr, "Writing Color Table...");
  for (i=0; i<31; ++i)
  {
    outColorT.shaded[i][0] = (short) colorTable[i][0];
    outColorT.shaded[i][1] = (short) colorTable[i][1];
    outColorT.shaded[i][2] = (short) colorTable[i][2];
  }

  for (i=0; i<64; ++i)
  {
    outColorT.fixed[i][0] = (short) colorTable[31+i][0];
    outColorT.fixed[i][1] = (short) colorTable[31+i][1];
    outColorT.fixed[i][2] = (short) colorTable[31+i][2];
  }

  for (i=0; i<16; ++i)
  {
    outColorT.overlay[i][0] = (short) colorTable[31+64+i][0];
    outColorT.overlay[i][1] = (short) colorTable[31+64+i][1];
    outColorT.overlay[i][2] = (short) colorTable[31+64+i][2];
  }

  for (i=0; i<16; ++i)
  {
    outColorT.underlay[i][0] = (short) colorTable[31+64+16+i][0];
    outColorT.underlay[i][1] = (short) colorTable[31+64+16+i][1];
    outColorT.underlay[i][2] = (short) colorTable[31+64+16+i][2];
  }

  writeDwbOpcode (DB_COLOR_TABLE, sizeof (dwbColorTable));
  swapDwbColorTable(&outColorT);
  fwrite (&outColorT, sizeof (dwbColorTable), 1, outFile);
  if (verbose) fprintf (stderr, "Done\n");
}


static int writeDwbTextureTable (void)
{
int i;
dwbExtTexInfo outTex;
uint        *image;         /* pointer to texture image */
int         components;     /* 1=I, 2=IA, 3=RGB, 4=RGBA */
int         ns;             /* texture size in 's' dimension */
int         nt;             /* texture size in 't' dimension */
int         nr;             /* texture size in 'r' dimension (ignored) */
int         splineType;

pfTexEnv    *tenv = NULL;

  if (verbose) fprintf (stderr, "Writing Texture Table...");
  for (i=0; i<numTextures; ++i)
  {
  pfTexture *t = (pfTexture *) textureTable[i].texture;
  char *name = (char *) pfGetTexName (t);

    pfGetTexImage (t, &image, &components, &ns, &nt, &nr);

    outTex.xsize = ns;
    outTex.ysize = nt;
    outTex.zsize = components;
    outTex.type  = 2;                /* verbatim */

    switch (pfGetTexFilter (t, PFTEX_MINFILTER))
    {
      case PFTEX_POINT: outTex.minfilter = DWB_TX_POINT; break;
      case PFTEX_BILINEAR: outTex.minfilter = DWB_TX_BILINEAR; break;


      case PFTEX_BICUBIC: outTex.minfilter = DWB_TX_BICUBIC; break;
      case PFTEX_MIPMAP_POINT: outTex.minfilter = DWB_TX_MIPMAP_POINT; break;
      case PFTEX_MIPMAP_LINEAR: outTex.minfilter = DWB_TX_MIPMAP_LINEAR; break;
      case PFTEX_MIPMAP_BILINEAR: outTex.minfilter = DWB_TX_MIPMAP_BILINEAR;
break;
      case PFTEX_MIPMAP_TRILINEAR: outTex.minfilter = DWB_TX_MIPMAP_TRILINEAR;
break;
      default:
	outTex.minfilter = DWB_TX_POINT;
      break;
    }

    switch (pfGetTexFilter (t, PFTEX_MAGFILTER))
    {
      case PFTEX_POINT: outTex.magfilter = DWB_TX_POINT; break;
      case PFTEX_BILINEAR: outTex.magfilter = DWB_TX_BILINEAR; break;


      case PFTEX_BICUBIC: outTex.magfilter = DWB_TX_BICUBIC; break;
      case PFTEX_SHARPEN: outTex.magfilter = DWB_TX_SHARPEN; break;
      case PFTEX_ADD_DETAIL: outTex.magfilter = DWB_TX_ADD_DETAIL; break;
      case PFTEX_MODULATE_DETAIL: outTex.magfilter = DWB_TX_MODULATE_DETAIL;
break;
      default:
	outTex.magfilter = DWB_TX_POINT;
      break;
    }

    switch (pfGetTexRepeat (t, PFTEX_WRAP))
    {
      case PFTEX_REPEAT: outTex.wrap = DWB_TX_REPEAT; break;
      case PFTEX_CLAMP: outTex.wrap = DWB_TX_CLAMP; break;
      case PFTEX_SELECT: outTex.wrap = DWB_TX_SELECT; break;
      default:
	fprintf (stderr, "pftodwb: Unknown texture wrap for %s\n", name);
	outTex.wrap = DWB_TX_REPEAT;
      break;
    }

    switch (pfGetTexRepeat (t, PFTEX_WRAP_S))
    {
      case PFTEX_REPEAT: outTex.wraps = DWB_TX_REPEAT; break;
      case PFTEX_CLAMP: outTex.wraps = DWB_TX_CLAMP; break;
      case PFTEX_SELECT: outTex.wraps = DWB_TX_SELECT; break;
      default:
	fprintf (stderr, "pftodwb: Unknown texture wrapS for %s\n", name);
	outTex.wraps = DWB_TX_REPEAT;
      break;
    }

    switch (pfGetTexRepeat (t, PFTEX_WRAP_T))
    {
      case PFTEX_REPEAT: outTex.wrapt = DWB_TX_REPEAT; break;
      case PFTEX_CLAMP: outTex.wrapt = DWB_TX_CLAMP; break;
      case PFTEX_SELECT: outTex.wrapt = DWB_TX_SELECT; break;
      default:
	fprintf (stderr, "pftodwb: Unknown texture wrapT for %s\n", name);
	outTex.wrapt = DWB_TX_REPEAT;
      break;
    }

    outTex.envtype   = DWB_TV_MODULATE;

    outTex.version   = 3.0f;

    if (tenv)
    {
    float c[4];

      switch (pfGetTEnvComponent (tenv))
      {
	case PFTE_COMP_I_GETS_R: outTex.component_select = DWB_TV_I_GETS_R;
break;
        case PFTE_COMP_I_GETS_G: outTex.component_select = DWB_TV_I_GETS_G;
break;
        case PFTE_COMP_I_GETS_B: outTex.component_select = DWB_TV_I_GETS_B;
break;

#ifdef PERFORMER_BUG
   /*
    * XXX Performer 2.0 alpha header file "opengl.h" defines this as
    * PFTE_COMP_I_GETS_B which makes the compiler barf about a duplicate case
    * statement.
    *
    * Hope they fix it soon.
    */

        case PFTE_COMP_I_GETS_A: outTex.component_select = DWB_TV_I_GETS_A;
break;
#endif


        case PFTE_COMP_I_GETS_I: outTex.component_select = DWB_TV_I_GETS_I;
break;
	default:
	  fprintf (stderr, "pftodwb: Unknown tenv component for %s\n", name);
	  outTex.component_select = DWB_TV_I_GETS_R;
	break;
      }

      pfGetTEnvBlendColor (tenv, &c[0], &c[1], &c[2], &c[3]);
      pfCopyVec4 (outTex.envcolor, c);
    }

    outTex.magfilter_alpha = 0;
    switch (pfGetTexFilter (t, PFTEX_MAGFILTER_ALPHA))
    {
      case PFTEX_POINT: outTex.magfilter_alpha = DWB_TX_POINT; break;
      case PFTEX_BILINEAR: outTex.magfilter_alpha = DWB_TX_BILINEAR; break;


      case PFTEX_BICUBIC: outTex.magfilter_alpha = DWB_TX_BICUBIC; break;
      case PFTEX_SHARPEN: outTex.magfilter_alpha = DWB_TX_SHARPEN; break;
      case PFTEX_ADD_DETAIL: outTex.magfilter_alpha = DWB_TX_ADD_DETAIL; break;
      case PFTEX_MODULATE_DETAIL: outTex.magfilter_alpha =
DWB_TX_MODULATE_DETAIL; break;
    }

    outTex.magfilter_color = 0;
    switch (pfGetTexFilter (t, PFTEX_MAGFILTER_COLOR))
    {
      case PFTEX_POINT: outTex.magfilter_color = DWB_TX_POINT; break;
      case PFTEX_BILINEAR: outTex.magfilter_color = DWB_TX_BILINEAR; break;


      case PFTEX_BICUBIC: outTex.magfilter_color = DWB_TX_BICUBIC; break;
      case PFTEX_SHARPEN: outTex.magfilter_color = DWB_TX_SHARPEN; break;
      case PFTEX_ADD_DETAIL: outTex.magfilter_color = DWB_TX_ADD_DETAIL; break;
      case PFTEX_MODULATE_DETAIL: outTex.magfilter_color =
DWB_TX_MODULATE_DETAIL; break;
    }

    bzero (outTex.bicubic_filter, sizeof (float)*2);
    bzero (outTex.mipmap_filter, sizeof (float)*8);

    outTex.internal              = 0;
    outTex.external              = 0;

    bzero (outTex.control_points, 4*2*sizeof (float));
    outTex.control_clamp = 0.0f;

    if (textureTable[i].detail)
    {
    int  k[5];

      pfGetDetailTexTile (t, &k[0], &k[1], &k[2], &k[3], &k[4]);
      outTex.detail[0] = k[0];
      outTex.detail[1] = k[1];
      outTex.detail[2] = k[2];
      outTex.detail[3] = k[3];
      outTex.detail[4] = k[4];
    }

    bzero (outTex.tile, sizeof (float)*4);

    outTex.flags.enabled         = 1;
    outTex.flags.magfilter_split = 0;
    outTex.flags.wrap_split      = (outTex.wrap != outTex.wraps);
    outTex.flags.internal        = 0;
    outTex.flags.external        = 0;
    outTex.flags.mipmap_filter   = 0;
    outTex.flags.bicubic_filter  = 0;
    outTex.flags.control_points  = 0;
    outTex.flags.tile            = 0;
    outTex.flags.detail          = textureTable[i].detail;
    outTex.flags.fast_define     = 0;
    outTex.component_select      = 0;

    writeDwbOpcode (DB_TEXTURE_TABLE, sizeof(dwbExtTexInfo));
	swapDwbExtTexInfo(&outTex);
    fwrite (&outTex, sizeof(dwbExtTexInfo), 1, outFile);
    writeDwbVariableLengthRecord (DB_TEX_FILE_STR, name);
  }
  if (verbose) fprintf (stderr, "Done\n");
}


static int writeDwbMaterialTable (void)
{
dwbLModelInfo lm;
dwbLSourceInfo ls;
dwbMatInfo outMaterial;
int i;

  if (verbose) fprintf (stderr, "Writing Material Table...");
  /*
   * Default values for the light model. These values are the ones
   * used by DWB while initializing.
   */

  lm.defn[0] = 0.2f;
  lm.defn[1] = 0.2f;
  lm.defn[2] = 0.2f;
  lm.defn[3] = 1.0f;
  lm.defn[4] = 0.0f;
  lm.defn[5] = 0.0f;
  lm.defn[6] = 0.0f;
  lm.defn[7] = 0.0f;

  writeDwbOpcode (DB_LMODEL, sizeof (dwbLModelInfo));
	swapDwbLModelInfo(&lm);
  fwrite (&lm, sizeof (dwbLModelInfo), 1, outFile);
	swapDwbLModelInfo(&lm); /* swap back */

  /*
   * Default values for the light source. These values are the ones
   * used by DWB while initializing.
   */

  ls.defn[0] = 0.0f;
  ls.defn[1] = 0.0f;
  ls.defn[2] = 0.0f;
  ls.defn[3] = 1.0f;
  ls.defn[4] = 1.0f;
  ls.defn[5] = 1.0f;
  ls.defn[6] = 0.0f;
  ls.defn[7] = 1.0f;
  ls.defn[8] = 0.0f;
  ls.defn[9] = 0.0f;
  ls.defn[10] = 0.0f;
  ls.defn[11] = 180.0f;
  ls.defn[12] = 0.0f;
  ls.defn[13] = 0.0f;
  ls.defn[14] = -1.0f;

  writeDwbOpcode (DB_LSOURCE, sizeof (dwbLSourceInfo));
	swapDwbLSourceInfo(&lm);
  fwrite (&lm, sizeof (dwbLSourceInfo), 1, outFile);
	swapDwbLSourceInfo(&lm); /* swap back */

  /*
   * write out the material table.
   */

  for (i=0; i<htM->listCount; ++i)
  {
  float *m = (float *) htM->list[i]->data;

    pfSetVec3 (outMaterial.mat.diffuse, m[0], m[1], m[2]);
    pfSetVec3 (outMaterial.mat.ambient, m[3], m[4], m[5]);

    outMaterial.mat.shininess = m[6];

    pfSetVec3 (outMaterial.mat.specular, m[7], m[8], m[9]);
    pfSetVec3 (outMaterial.mat.emissive, m[10], m[11], m[12]);

    outMaterial.mat.alpha = m[13];

    writeDwbOpcode (DB_MATERIAL_TABLE, sizeof (dwbMatInfo));
	swapDwbMatInfo(&outMaterial);
    fwrite (&outMaterial, sizeof (dwbMatInfo), 1, outFile);
	swapDwbMatInfo(&outMaterial); /* swap back */

    /*
     * Set the index for each material so that when the hashTable is
     * looked up for a material, 'id' will have the material index.
     */

    htM->list[i]->id = i;
  }

  if (verbose) fprintf (stderr, "Done\n");
  fprintf (stderr, "Done\n");
  return 1;
}


static char *getCurrentDateTime (void)
{
char dateString[32];
int  len;
time_t tsecs;

  tsecs = time(&tsecs);
  strcpy (dateString, ctime(&tsecs));
  len = strlen (dateString);
  dateString[len-1] = '\0';

  if ( (len-1) > 31 )
    dateString[len-2] = '\0';

  return (dateString);
}

static void dwbPush ( void )
{
  level++;
  writeDwbOpcode(DB_PUSH,0);
}

static void dwbPop ( void )
{
  writeDwbOpcode(DB_POP,0);
  level--;
}

static int writeDwbOpcode (int op,int rec) {
  unsigned short status,opcode,recsize=0;
  
  opcode  = op;
  recsize = rec;
  
  opcode=get16(&opcode);
  recsize=get16(&recsize);
  
  status = fwrite ( &opcode,2,1,outFile);
  if ( status != 1 ) { perror("pfdStoreFile_dwb"); return ( 0 );}
  status = fwrite ( &recsize,2,1,outFile);
  if ( status != 1 ) { perror("pfdStoreFile_dwb"); return ( 0 );}
  
  return ( 1 );
}

static int writeDwbVariableLengthRecord ( int opcode,const char *name )
{
    int len,ival;
    char tname[256];
    float fval,rmdr;

  if ( name ) {
    len = strlen(name);

    ival = len / 4;
    fval = (float)len / 4.0;
    rmdr = ((float)ival) + 1.0  - fval;
    if ( rmdr == 1.0 )
      sprintf( tname,"%s",name );
    else if ( rmdr == 0.75 ) {
      sprintf( tname,"%s%c%c%c",name,'\0','\0','\0' );
      len += 3;
    }
    else if ( rmdr == 0.5 ) {
      sprintf( tname,"%s%c%c",name,'\0','\0' );
      len += 2;
    }
    else if ( rmdr == 0.25 ) {
      sprintf( tname,"%s%c",name,'\0' );
      len ++;
    }

    if ( writeDwbOpcode(opcode,len) ) {
      if ( fwrite (tname,len,1,outFile) ) return ( 1 );
    }
  }

  return ( 0 );
}

