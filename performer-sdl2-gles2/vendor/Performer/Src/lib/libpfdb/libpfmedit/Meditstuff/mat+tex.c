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
Materials and textures...
************************************************************************/

/************************************************************************
Add a material to the medit file (if it doesn't already exist)
************************************************************************/
static MMaterialPtr AddMaterial(MMaterialPtr mat)
{
    int i;
    MMaterialPtr new, search;

    search = CurrentMeditFile->FirstMaterial;
    while (search) {
	if ((mat IS search) OR (CompareMaterials(search, mat))) {
	    return search;
	}
	else {
	    float diff = 0.0;
	    for (i=0; i<3; i++) {
		diff += fabs(search->Ambient[i] - mat->Ambient[i]);
		diff += fabs(search->Diffuse[i] - mat->Diffuse[i]);
		diff += fabs(search->Specular[i] - mat->Specular[i]);
	    }
	}
	search = search->Next;
    }
    MaterialsCreated++;
    new = malloc(sizeof(MMaterial));
    for (i=0; i<3; i++) {
	new->Ambient[i] = mat->Ambient[i];
	new->Diffuse[i] = mat->Diffuse[i];
	new->Specular[i] = mat->Specular[i];
	new->Emissive[i] = mat->Emissive[i];
    }
    new->Alpha = mat->Alpha;
    new->Shine = mat->Shine;
    new->Name = strdup("Unnamed");
    new->Next = CurrentMeditFile->FirstMaterial;
    CurrentMeditFile->FirstMaterial = new;
    return new;
}
/************************************************************************
Turn a pfMaterial into a Medit material
************************************************************************/
static void GetMaterialComponent(pfMaterial *m, pflong component, float *c)
{
    pfGetMtlColor(m, component, c, c+1, c+2);
}
static void GenerateMaterial(MMaterialPtr mat, pfMaterial *m)
{
    int i;
    if (m) {
	DEBUG"Grabbing material\n");
	GetMaterialComponent(m, PFMTL_AMBIENT, mat->Ambient);
	GetMaterialComponent(m, PFMTL_DIFFUSE, mat->Diffuse);
	GetMaterialComponent(m, PFMTL_SPECULAR, mat->Specular);
	GetMaterialComponent(m, PFMTL_EMISSION, mat->Emissive);
	mat->Alpha = pfGetMtlAlpha(m);
	mat->Shine = pfGetMtlShininess(m);
    }
    else {
	for (i=0; i<3; i++) {
	    mat->Ambient[i] = 0.4;
	    mat->Diffuse[i] = 0.8;
	    mat->Specular[i] = 0.0;
	    mat->Emissive[i] = 0.0;
	}
	mat->Shine = 64;
	mat->Alpha = 1.0;
    }
}
/************************************************************************
Turn a color into a material
************************************************************************/
#define Tolerance 0.15
#define Ambratio 0.3
static boolean FloatsAreClose(reg float a, reg float b)
{
    register float d;
    if (b IS 0.0) {
	d = a;
    }
    else {
	d = (a/b)-1.0;
    }
    return (fabsf(d) < Tolerance);
}
static boolean RatiosAreClose(reg float rratio, reg float gratio, reg float bratio)
{
    return (FloatsAreClose(rratio, gratio) AND FloatsAreClose(rratio, bratio));
}
#define CalcRatio(var, Component, col, index)			\
if (col < (Tolerance/5)) {					\
    var = (search->Component[index] < Tolerance) ? 0.0: 0.0;	\
}								\
else {								\
    var = search->Component[index]/col;				\
}
static MMaterialPtr MixMaterial(reg MMaterialPtr mat, reg float *colour)
{
    reg int i;
    static MMaterial mix;
    reg MMaterialPtr search;
    reg float red, green, blue, rratio, gratio, bratio;

    red = colour[0];
    green = colour[1];
    blue = colour[2];
    for (i=0; i<3; i++) {
	mix.Ambient[i] = mat->Ambient[i] * colour[i];
	mix.Diffuse[i] = mat->Diffuse[i] * colour[i];
	mix.Specular[i] = mat->Specular[i];
	mix.Emissive[i] = mat->Emissive[i];
    }
    mix.Alpha = colour[3];
    mix.Shine = mat->Shine;

    search = CurrentMeditFile->FirstMaterial;
    while (search) {
	CalcRatio(rratio, Diffuse, red,   0);
	CalcRatio(gratio, Diffuse, green, 1);
	CalcRatio(bratio, Diffuse, blue,  2);
	/*DEBUG"rratio, gratio, bratio = %f, %f, %f\n", rratio, gratio, bratio);*/
	if (RatiosAreClose(rratio, gratio, bratio)) {
	    CalcRatio(rratio, Ambient, red,   0);
	    CalcRatio(gratio, Ambient, green, 1);
	    CalcRatio(bratio, Ambient, blue,  2);
	    if (RatiosAreClose(rratio, gratio, bratio)) {
		return search;
	    }
	}
	search = search->Next;
    }
    return &mix;
}
/************************************************************************
Turn a pftexture into a Medit texture
************************************************************************/
static MTexturePtr MakeTexture(pfTexture *t)
{
    MTexturePtr new, search;
    if (pfGetTexName(t)) {
	new = malloc(sizeof(MTexture));
	new->File = strdup(pfGetTexName(t));
	new->MinFilter = MT_NULL;
	new->MagFilter = MT_NULL;
	switch (pfGetTexRepeat(t, PFTEX_WRAP_S)) {
	    case PFTEX_CLAMP:	new->ClampX = TRUE;
				break;
	    default:		new->ClampX = FALSE;
	}
	switch (pfGetTexRepeat(t, PFTEX_WRAP_T)) {
	    case PFTEX_CLAMP:	new->ClampY = TRUE;
				break;
	    default:		new->ClampY = FALSE;
	}
	new->Fast = FALSE;

	search = CurrentMeditFile->FirstTexture;
	while (search) {
	    if (CompareTextures(search, new)) {
		free(new);
		return search;
	    }
	    search = search->Next;
	}
	new->Next = CurrentMeditFile->FirstTexture;
	CurrentMeditFile->FirstTexture = new;
	return new;
    }
    else {
	return NULL;
    }
}
