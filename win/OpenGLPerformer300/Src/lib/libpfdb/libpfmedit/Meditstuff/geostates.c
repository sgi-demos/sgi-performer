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
This file deals with Performer goestates. It generates geostates as
needed, and maintains lists of existing geostates
************************************************************************/

/************************************************************************
Storage for our geostate list...
************************************************************************/
/* Structure to remember geostate definitions */
typedef struct geostate_list {
struct geostate_list	*Next;			/* For linked list			*/
pfGeoState		*Geostate;		/* Pointer to the geostate		*/
flag			Lit;			/* Flag - geostate has lighting enabled	*/
flag			Textured;		/* Flag - geostate has texturing enabled*/
flag			EnvMapped;		/* Flag - geostate is environment mapped*/
MMaterial		Mat;			/* The material used in this geostate	*/
pfMaterial		*Matdef;		/* The performer copy of this material	*/
MTexture		Tex;			/* The texture used in this geostate	*/
pfTexture		*Texdef;		/* The performer copy of this texture	*/
} GeoStateList;
typedef GeoStateList *GeoStateListPtr;
static GeoStateListPtr FirstGeoState = NULL;

/************************************************************************
Compare two Materials to see if they are the same
************************************************************************/
#define CompareMat(prop) ( 			\
(m1->prop[Red]   IS m2->prop[Red]) AND		\
(m1->prop[Green] IS m2->prop[Green]) AND	\
(m1->prop[Blue]  IS m2->prop[Blue]))

static boolean CompareMaterials(register MMaterialPtr m1, register MMaterialPtr m2)
{
    if (FastLighting) {		/* All materials are equal in this mode */
	return TRUE;
    }
    if (!CompareMat(Ambient)) return FALSE;
    if (!CompareMat(Diffuse)) return FALSE;
    if (!CompareMat(Specular)) return FALSE;
    if (!CompareMat(Emissive)) return FALSE;
    if (m1->Shine ISNT m2->Shine) return FALSE;
    if (m1->Alpha ISNT m2->Alpha) return FALSE;

    return TRUE;
}
#undef CompareMat
/************************************************************************
Copy a material
************************************************************************/
static void CopyMaterial(MMaterialPtr dest, MMaterialPtr source)
{
    CopyXYZ(dest->Ambient,source->Ambient);
    CopyXYZ(dest->Diffuse,source->Diffuse);
    CopyXYZ(dest->Specular,source->Specular);
    CopyXYZ(dest->Emissive,source->Emissive);
    dest->Shine = source->Shine;
    dest->Alpha = source->Alpha;
}
/************************************************************************
Turn a Medit material into a Performer material
************************************************************************/
#define RGBOf(m) m[0],m[1],m[2]
#define RGBSumOf(m) m[0]+m[1]+m[2]
static pfMaterial *WhiteMat = NULL;
static pfMaterial *DefineMaterial(MMaterialPtr mat)
{
    pfMaterial *new;
    GeoStateListPtr g;

    if (FastLighting) {
	if (!WhiteMat) {
	    WhiteMat = pfNewMtl(MeditArena);
	    pfMtlColor(WhiteMat, PFMTL_AMBIENT, 0.5, 0.5, 0.5);
	    pfMtlShininess(WhiteMat,0);
	    pfMtlAlpha(WhiteMat,1.0);
	    pfMtlSide(WhiteMat,PFMTL_BOTH);
	    pfMtlColorMode(WhiteMat, PFMTL_BOTH, PFMTL_CMODE_AD);
	}
	return WhiteMat;
    }
    else {
	g = FirstGeoState;	/* First see if we already have this material */
	while (g) {
	    if (CompareMaterials(mat, &(g->Mat))) {
		return g->Matdef;	/* Yes, share the definition */
	    }
	    g = g->Next;
	}
	new = pfNewMtl(MeditArena);
	pfMtlColor(new, PFMTL_AMBIENT, RGBOf(mat->Ambient));
	pfMtlColor(new, PFMTL_DIFFUSE, RGBOf(mat->Diffuse));
	if (RGBSumOf(mat->Specular) ISNT 0.0) {
	    pfMtlColor(new, PFMTL_SPECULAR, RGBOf(mat->Specular));
	}
	if (RGBSumOf(mat->Emissive) ISNT 0.0) {
	    pfMtlColor(new, PFMTL_EMISSION, RGBOf(mat->Emissive));
	}
	pfMtlSide(new,PFMTL_BOTH);
	pfMtlShininess(new,mat->Shine);
	pfMtlAlpha(new,mat->Alpha);
	if (mat->Alpha < 0.95) {
	    pfMtlColorMode(new, PFMTL_BOTH, PFMTL_CMODE_COLOR);
	}
	return new;
    }
}
#undef RGBOf
#undef RGBSumOf





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
Copy a texture
************************************************************************/
static void CopyTexture(MTexturePtr dest, MTexturePtr source)
{
    dest->File = malloc(strlen(source->File)+1);
    strcpy(dest->File,source->File);

    dest->MinFilter = source->MinFilter;
    dest->MagFilter = source->MagFilter;
    dest->ClampX = source->ClampX;
    dest->ClampY = source->ClampY;
    dest->Fast = source->Fast;
}
/************************************************************************
Read/define a texture
************************************************************************/
static char *ChopOffPath(char *filename)
{
    char *search = filename, *find;
    while (find = strchr(search,'/')) {
	search = ++find;
    }
    return search;
}
static pfTexture *LoadTexture(MTexturePtr tex, int *components)
{
    flag do_it;
    pfTexture *new;
    GeoStateListPtr g;
    int filter, s,t,r;
    unsigned int *image;

    g = FirstGeoState;	/* First see if we already have this texture... */
    while (g) {
	if (g->Textured AND CompareTextures(tex, &(g->Tex))) {
	    return g->Texdef;	/* yes, share the definition */
	}
	g = g->Next;
    }

    new = pfNewTex(MeditArena);
    if (pfLoadTexFile(new, tex->File)) {
	pfGetTexImage(new, &image, components, &s, &t, &r);

	/* Set minification filter */
	do_it = TRUE;
	switch ((int)tex->MinFilter) {
	    case MT_POINT:		filter = PFTEX_POINT;		break;
	    case MT_BILINEAR:		filter = PFTEX_BILINEAR;	break;
	    case MT_BICUBIC:		filter = PFTEX_BICUBIC;
					do_it = ItsARealityEngine;	break;
	    case MT_MIPMAP_POINT:	filter = PFTEX_MIPMAP_POINT;	break;
	    case MT_MIPMAP_LINEAR:	filter = PFTEX_MIPMAP_LINEAR;	break;
	    case MT_MIPMAP_BILINEAR:	filter = PFTEX_MIPMAP_BILINEAR;	break;
	    case MT_MIPMAP_TRILINEAR:	filter = PFTEX_MIPMAP_TRILINEAR;
					do_it = ItsARealityEngine;	break;
	    default:			do_it = FALSE;
	}
	if (do_it) pfTexFilter(new,PFTEX_MINFILTER,filter);


	/* Set magnification filter */
	do_it = TRUE;
	switch ((int)tex->MagFilter) {
	    case MT_POINT:		filter = PFTEX_POINT;		break;
	    case MT_BILINEAR:		filter = PFTEX_BILINEAR;	break;
	    case MT_BICUBIC:		filter = PFTEX_BICUBIC;
					do_it = ItsARealityEngine;	break;
	    case MT_SHARPEN:		filter = PFTEX_SHARPEN;
					do_it = ItsARealityEngine;	break;
	    default:			do_it = FALSE;
	}
	if (do_it) pfTexFilter(new,PFTEX_MAGFILTER,filter);

	if (tex->Fast AND ItsARealityEngine) {
	    if (*components IS 3) {
		pfTexFormat(new, PFTEX_INTERNAL_FORMAT, PFTEX_RGB_5);
	    }
	    else if (*components IS 4) {
		pfTexFormat(new, PFTEX_INTERNAL_FORMAT, PFTEX_RGBA_4);
	    }
	}
	if (tex->ClampX) {
	    pfTexRepeat(new, PFTEX_WRAP_S, PFTEX_CLAMP);
	}
	if (tex->ClampY) {
	    pfTexRepeat(new, PFTEX_WRAP_T, PFTEX_CLAMP);
	}
    }
    return new;
}
/************************************************************************
Generate a geostate (if it doesn't exist)
************************************************************************/
static pfTexEnv *ModulateEnv = NULL;
static pfTexGen *EnvironmentGen = NULL;
static pfGeoState *MakeGeostate(flag coloured, MMaterialPtr mat, MTexturePtr tex, flag EnvMap)
{
    pfTexture *t;
    int components;
    register flag found;
    register pfGeoState *geostate;
    register GeoStateListPtr g = FirstGeoState;;

    /* First see if geostate already exists... */
    while (g) {
	found = TRUE;
	if ((g->Lit IS coloured) OR !CompareMaterials(mat,&(g->Mat))) {
	    found = FALSE;
	}
	if (found) {
	    if (tex) {
		found = (g->Textured AND CompareTextures(tex,&(g->Tex)));
	    }
	    else {
		found = !g->Textured;
	    }
	}
	if (found) {
	    found = EnvMap IS g->EnvMapped;
	}
	if (found) {
	    /* Geostate exists, use the existing definition */
	    return g->Geostate;
	}
	g = g->Next;
    }

    /* Geostate is not defined, generate a new one... */
    g = pfMalloc(sizeof(GeoStateList), MeditArena);
    geostate = g->Geostate = pfNewGState(MeditArena);
    if (coloured) {
	g->Lit = FALSE;
	pfGStateMode(geostate, PFSTATE_ENLIGHTING, PF_OFF);
    }
    else {
	g->Lit = TRUE;
	CopyMaterial(&(g->Mat),mat);
	g->Matdef = DefineMaterial(mat);

	pfGStateMode(geostate, PFSTATE_ENLIGHTING, PF_ON);
	pfGStateAttr(geostate, PFSTATE_FRONTMTL, g->Matdef);
    }
    if (MultiSampling) {
	pfGStateMode(geostate, PFSTATE_TRANSPARENCY, PFTR_MS_ALPHA);
    }
    else {
	pfGStateMode(geostate, PFSTATE_TRANSPARENCY, PFTR_ON);
    }
    if (tex AND (t = LoadTexture(tex, &components))) {
	g->Textured = TRUE;
	CopyTexture(&(g->Tex),tex);
	g->Texdef = t;

	pfGStateMode(geostate, PFSTATE_ENTEXTURE, 1);
	pfGStateAttr(geostate, PFSTATE_TEXTURE, t);
	if (!ModulateEnv) {
	    ModulateEnv = pfNewTEnv(MeditArena);
	    pfTEnvMode(ModulateEnv,PFTE_MODULATE);
	}
	pfGStateAttr(geostate, PFSTATE_TEXENV, ModulateEnv);

	if ((components IS 2) OR (components IS 4)) {		/* ie. Texure has alpha */
	    pfGStateMode(geostate, PFSTATE_TRANSPARENCY, PFTR_OFF);
	    if (AfunctionAvailable) {
		if (!MultiSampling) {
		    pfGStateMode(geostate, PFSTATE_TRANSPARENCY, PFTR_OFF);
		}
		if (GoodAFunction) {
		    pfGStateMode(geostate, PFSTATE_ALPHAFUNC, PFAF_GREATER);
		    pfGStateVal( geostate, PFSTATE_ALPHAREF, (64.0f/255.0f));
		}
		else {
		    pfGStateMode(geostate, PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
		    pfGStateVal( geostate, PFSTATE_ALPHAREF, 0.0f);
		}
	    }
	}
    }
    else {
	g->Textured = FALSE;
	pfGStateMode(geostate, PFSTATE_ENTEXTURE, 0);
    }
    if (g->EnvMapped = EnvMap) {
	if (!EnvironmentGen) {
	    EnvironmentGen = pfNewTGen(MeditArena);
	    pfTGenMode(EnvironmentGen, PF_S, PFTG_SPHERE_MAP);
	    pfTGenMode(EnvironmentGen, PF_T, PFTG_SPHERE_MAP);
	}
	pfGStateMode(geostate, PFSTATE_ENTEXGEN, PF_ON);
	pfGStateAttr(geostate, PFSTATE_TEXGEN, EnvironmentGen);
    }

    g->Next = FirstGeoState;	/* Add it to our list of defined geostates */
    FirstGeoState = g;

    return geostate;
}
