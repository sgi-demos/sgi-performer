/*--------------------------Message start-------------------------------

This software is copyright (C) 1994-1995 by Medit Productions


This software is provided under the following terms, if you do not
agree to all of these terms, delete this software now. Any use of
this software indicates the acceptance of these terms by the person
making said usage ("The User").

1) Medit Productions provides this software "as is" and without
warranty of any kind. All consequences of the use of this software
fall completely on The User.

2) This software may be copied without restriction, provided that an
unaltered and clearly visible copy of this message is placed in any
copies of, or derivatives of, this software.


---------------------------Message end--------------------------------*/




/************************************************************************
Data structures
************************************************************************/
typedef struct pstruct {
struct pstruct	*Next;
MMaterialPtr	mat;
MTexturePtr	tex;
int		NoVerts;
int		lighting;
float		Pos[4][XYZ];
float		Tex[4][XY];
} myPolygon;
typedef myPolygon *PolygonPtr;

typedef struct geom {
struct geom	*Next;
PolygonPtr	FirstPolygon;
flag		Billboard;
float		Centre[XYZ];
flag		HasSwitch;
float		SwitchIn;
float		SwitchOut;
} Geometry;
typedef Geometry *GeometryPtr;

typedef struct geometry_list {
MBranchPtr	Parent;
char		*Name;
GeometryPtr	FirstGeom;
GeometryPtr	FirstBill;
float		Transformation[4][4];
} GList;
typedef GList *GListPtr;

#define LoopThroughChildren(n)	children = pfGetNumChildren(n);	\
				for (loop=0; loop<children; loop++)
#define LoopChild pfGetChild(node, loop)
#define CopyXYZ(d, s) { d[0]=s[0]; d[1]=s[1]; d[2]=s[2]; }

static int MaterialsCreated, ObjectsCreated, ObjectsReplicated, PolygonsCreated;

/************************************************************************
Return the name of a node
************************************************************************/
static char *NodeName(pfNode *node)
{
    char *n;
    if (pfGetNodeName(node)) {
	n = strdup(pfGetNodeName(node));
    }
    else {
	n = strdup("Unnamed");
    }
    return n;
}
static void InitGList(GListPtr g, char *name)
{
    g->Name = strdup(name);
    g->FirstGeom = NULL;
    g->FirstBill = NULL;
}


/************************************************************************
Compare two Materials to see if they are the same
************************************************************************/
#define CompareMat(prop) ( 		\
(m1->prop[0] IS m2->prop[0]) AND	\
(m1->prop[1] IS m2->prop[1]) AND	\
(m1->prop[2] IS m2->prop[2]))

static boolean CompareMaterials(register MMaterialPtr m1, register MMaterialPtr m2)
{
    reg int i;
    reg float diff = 0.0;
    for (i=0; i<3; i++) {
	diff += fabsf(m1->Ambient[i] - m2->Ambient[i]);
	diff += fabsf(m1->Diffuse[i] - m2->Diffuse[i]);
	diff += fabsf(m1->Specular[i] - m2->Specular[i]);
	diff += fabsf(m1->Emissive[i] - m2->Emissive[i]);
    }
    diff += fabsf(m1->Shine - m2->Shine);
    diff += fabsf(m1->Alpha - m2->Alpha);

    return diff < 0.1;
}
#undef CompareMat

/************************************************************************
Compare two textures to see if they are the same
************************************************************************/
static boolean CompareTextures(register MTexturePtr t1, register MTexturePtr t2)
{
    if (strcmp(t1->File,t2->File) ISNT 0) return FALSE;
    if (t1->MinFilter ISNT t2->MinFilter) return FALSE;
    if (t1->MagFilter ISNT t2->MagFilter) return FALSE;
    if (t1->ClampX ISNT t2->ClampX) return FALSE;
    if (t1->ClampY ISNT t2->ClampY) return FALSE;
    if (t1->Fast ISNT t2->Fast) return FALSE;

    return TRUE;
}

/************************************************************************
Node types
************************************************************************/
static boolean ItsAGroup(pfNode *node)
{
#ifdef PERFORMER2
    return (pfIsOfType(node, pfGetGroupClassType()));
#else
    return (pfGetType(node) IS PFTYPE_GROUP);
#endif
}
static boolean ItsALod(pfNode *node)
{
#ifdef PERFORMER2
    return (pfIsOfType(node, pfGetLODClassType()));
#else
    return (pfGetType(node) IS PFTYPE_LOD);
#endif
}
static boolean ItsATransform(pfNode *node)
{
#ifdef PERFORMER2
    return (pfIsOfType(node, pfGetLODClassType())
	OR  pfIsOfType(node, pfGetLODClassType()));
#else
    return ((pfGetType(node) IS PFTYPE_SCS)
	OR  (pfGetType(node) IS PFTYPE_DCS));
#endif
}

static boolean ItsABillboard(pfNode *node)
{
#ifdef PERFORMER2
    return (pfIsOfType(node, pfGetBboardClassType()));
#else
    return (pfGetType(node) IS PFTYPE_BILLBOARD);
#endif
}
static boolean ItsAGeode(pfNode *node)
{
    if (ItsABillboard(node)) {
	return TRUE;
    }
    else {
#ifdef PERFORMER2
	return (pfIsOfType(node, pfGetGeodeClassType()));
#else
	return (pfGetType(node) IS PFTYPE_GEODE);
#endif
    }
}


