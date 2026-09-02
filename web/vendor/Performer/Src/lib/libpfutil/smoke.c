/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 */

#include <math.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>

#define MAX_SMOKE	16
#define MAX_PUFFS	32
#define MAX_RAND	256	/* Must be power of 2 */

#define RAND_VEC3	RandVec3[pfuRandomLong() & (MAX_RAND - 1)]

#define MAXSMOKES	256

typedef struct _pfuPuff
{
    float	t;
    float	transp;
    double	startTime;

    float	radius;
    pfVec3	origin;

    pfMatrix	rotMat;

    pfVec3	coords[4];
    pfVec3	tcoords[4];

    float	speed;
    pfVec3	direction;	/* Direction of travel with random factor */

} pfuPuff;

struct _pfuSmoke
{
    pfVec3	bgnColor, deltaColor;

    float	dissipation;	/* Dissipation rate in seconds */
    float	density;
    float	puffInterval;
    float	duration;

    double	startTime;
    double	prevTime;
    double	lastPuffTime;	

    int	type;
    int	mode;
    float	frequency;

    pfVec3	direction;	/* Direction of travel */
    float	speed;
    float	turbulence;

    pfVec3	origin;
    float	radius;
    float	expansion;
    float	offset;		
    
    pfTexture	*tex;

    pfuPuff	*puffs[MAX_PUFFS];
    int	startPuff;
    int	numPuffs;

};

static void	initPuff(pfuPuff *puff, pfuSmoke *smoke);
#if 0
static void	getRandVec3(pfVec3 vec);
#endif
static void	drawPuff(pfuPuff *puff, pfuSmoke *smoke, pfVec3 eye);
static void	drawSmoke(pfuSmoke *smoke, pfVec3 eye);

static int	InitRand = 0;
static pfVec3	RandVec3[MAX_RAND], smokeDir;

static int	*smokeCount;
static pfuSmoke	**smokeList;

static pfTexture *smokeTex, *fireTex;
static pfTexEnv  *smokeTEnv;


	/*----------------------------------------------------------*/

#if 0
#define NSQR 	512		/* must be a power of 2 */
#define NSQR1F	511.0f		/* float version of NSQR-1 */

#define SQR_LOOKUP(_x)  	(sqrArr[((int)(_x))&(NSQR-1)])

static float sqrArr[NSQR];

static void
initSqrTable(void)
{
    int 	i;
    float 	delta ;

    delta = 1.0f / NSQR;
    for (i = 0 ; i < NSQR ; i++)
        sqrArr[i] = pfSqrt((i+0.5f)*delta);
}
#endif

#if 0
static void
getRandVec3(pfVec3 vec)
{
    int	i = pfuRandomLong() & (MAX_RAND - 1);

    PFCOPY_VEC3(vec, RandVec3[i]);
}
#endif

static void
initPuff(pfuPuff *puff, pfuSmoke *smoke)
{
    float 	*rv, r;
    pfVec3	offset;
    pfMatrix	mat;
    int	i;

    if(InitRand == 0)
    {
	for(i=0; i<MAX_RAND; i++)
	{
	    RandVec3[i][0] = 2.0f*pfuRandomFloat() - 1.0f;
	    RandVec3[i][1] = 2.0f*pfuRandomFloat() - 1.0f;
	    RandVec3[i][2] = 2.0f*pfuRandomFloat() - 1.0f;

	    pfNormalizeVec3(RandVec3[i]);
	}

	InitRand = 1;
    }

    puff->radius = smoke->radius;
    pfCopyVec3(puff->origin, smoke->origin);
    puff->transp = 0.0f;
    puff->startTime = smoke->lastPuffTime;

    /* Add random velocity modifier */
    if(smoke->type != PFUSMOKE_MISSLE)
    {
        rv = RAND_VEC3;
    	pfAddScaledVec3(puff->direction, smoke->direction, .15f, rv);
	pfScaleVec3(offset, smoke->expansion * smoke->radius * .05f, rv);
	PFADD_VEC3(puff->origin, puff->origin, offset);
    }
    else
    	PFCOPY_VEC3(puff->direction, smoke->direction);

    r = (pfuRandomFloat() - .5f) * 4.0f * smoke->turbulence;

    pfMakeTransMat(puff->rotMat, -0.5f, -0.5f, 0.0f);
    pfMakeRotMat(mat, r, 0.0f, 0.0f, 1.0f);
    pfPostMultMat(puff->rotMat, mat);
    pfMakeTransMat(mat, 0.5f, 0.5f, 0.0f);
    pfPostMultMat(puff->rotMat, mat);

    r = 1.0f + (pfuRandomFloat() - 0.5f) * .1f;

    puff->speed = r * smoke->speed;
}

PFUDLLEXPORT void
pfuInitSmokes(void)
{
    smokeList = (pfuSmoke**)pfCalloc(1, sizeof(pfuSmoke*) * MAXSMOKES, pfGetSharedArena());
    smokeCount = (int*)pfCalloc(1, sizeof(int), pfGetSharedArena());

    smokeDir[0] = 0.1f;
    smokeDir[1] = 0.3f;
    smokeDir[2] = 1.0f;

    pfNormalizeVec3(smokeDir);

    smokeTex = pfNewTex(pfGetSharedArena());
    pfLoadTexFile(smokeTex, "smoke.tex");
    pfTexRepeat(smokeTex, PFTEX_WRAP, PFTEX_CLAMP);
    fireTex = pfNewTex(pfGetSharedArena());
    pfLoadTexFile(fireTex, "fire.tex");
    pfTexRepeat(fireTex, PFTEX_WRAP, PFTEX_CLAMP);

    smokeTEnv = pfNewTEnv(pfGetSharedArena());
}


PFUDLLEXPORT pfuSmoke*
pfuNewSmoke(void)
{
    void	*arena = pfGetSharedArena();
    int	i;
    pfuSmoke 	*smoke;

    smoke = (pfuSmoke*)pfMalloc(sizeof(pfuSmoke), arena);

    smoke->radius = 1.0f;
    pfSetVec3(smoke->origin, 0.0f, 0.0f, 0.0f);
    pfCopyVec3(smoke->direction, smokeDir);
    smoke->speed = .1f;	
    smoke->turbulence = 1.0f;
    smoke->mode = PFUSMOKE_STOP;
    smoke->type = PFUSMOKE_SMOKE;
    smoke->startPuff = smoke->numPuffs = 0;
    smoke->prevTime = -1.0f;
    smoke->startTime = -1.0f;
    smoke->tex = NULL;
    smoke->duration = -1.0f;
    smoke->lastPuffTime = -1.;

    pfuSmokeType(smoke, smoke->type);

    for(i=0; i<MAX_PUFFS; i++)
    {
	int	j, r;

	static pfVec2 tc[4][4] = {
		{{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},
		{0.0f, 1.0f}},

		{{1.0f, 0.0f},
		{0.0f, 0.0f},
		{0.0f, 1.0f},
		{1.0f, 1.0f}},

		{{0.0f, 1.0f},
		{1.0f, 1.0f},
		{1.0f, 0.0f},
		{0.0f, 0.0f}},

		{{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},
		{0.0f, 1.0f}}
	}; 

	initPuff(smoke->puffs[i] = (pfuPuff*)pfMalloc(sizeof(pfuPuff), arena), smoke);

	r = (int)(pfuRandomLong() & 0x3);

	for(j=0; j<4; j++)
	{
	    pfCopyVec2(smoke->puffs[i]->tcoords[j], tc[r][j]);
	    smoke->puffs[i]->tcoords[j][2] = 0.0f;
	}
    }
    if(*smokeCount >= MAXSMOKES)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, "Too many smokes");
	return smokeList[*smokeCount-1];
    }
    else
    {
	smokeList[*smokeCount] = smoke;
	*smokeCount = *smokeCount + 1;
    }

    return smoke;
}

PFUDLLEXPORT void
pfuSmokeType(pfuSmoke *smoke, int type)
{
    static pfVec3	wht = {1.0f, 1.0f, 1.0f};
    static pfVec3	blk = {.01f, .01f, .01f};
    static pfVec3	brn = {.2f, .1f, .05f};

    smoke->type = type;

    switch(type) {
	case PFUSMOKE_MISSLE:
	    smoke->speed = 32.3f;
    	    pfuSmokeDensity(smoke, 0.52f, .05f, .17f);
    	    pfuSmokeColor(smoke, wht, blk);
    	    pfuSmokeTex(smoke, fireTex);
	    break;
	case PFUSMOKE_EXPLOSION:
	    smoke->speed = 0.0f;
    	    pfuSmokeDensity(smoke, 1.0f, .2f, 4.0f);
    	    pfuSmokeDuration(smoke, .2f);
    	    pfuSmokeColor(smoke, wht, wht);
    	    pfuSmokeTex(smoke, fireTex);
	    break;
	case PFUSMOKE_FIRE:
	    smoke->speed = 1.73f;
    	    pfuSmokeDensity(smoke, 0.11f, .32f, 1.13f);
    	    pfuSmokeColor(smoke, wht, blk);
    	    pfuSmokeTex(smoke, fireTex);
	    break;
	case PFUSMOKE_SMOKE:
	    smoke->speed = 1.73f;
    	    pfuSmokeDensity(smoke, 0.33f, 2.42f, 1.83f);
    	    pfuSmokeColor(smoke, wht, blk);
    	    pfuSmokeTex(smoke, smokeTex);
	    break;
	case PFUSMOKE_DUST:
	    smoke->speed = .8f;
    	    pfuSmokeDensity(smoke, 0.1f, 2.0f, 2.0f);
    	    pfuSmokeColor(smoke, brn, brn);
    	    pfuSmokeTex(smoke, smokeTex);
	    break;
	default:
	    smoke->speed = 1.73f;
    	    pfuSmokeDensity(smoke, 0.33f, 2.42f, 1.83f);
    	    pfuSmokeColor(smoke, wht, blk);
    	    pfuSmokeTex(smoke, smokeTex);
	    break;
    }
}

PFUDLLEXPORT void
pfuSmokeOrigin(pfuSmoke *smoke, pfVec3 origin, float radius)
{
    pfCopyVec3(smoke->origin, origin);
    smoke->radius = radius;
}

PFUDLLEXPORT void
pfuSmokeMode(pfuSmoke *smoke, int mode)
{
    smoke->mode = mode;
    if (smoke->mode == PFUSMOKE_START)
    {
	smoke->startTime = -1.;
	smoke->lastPuffTime = -1.;
    }
}

PFUDLLEXPORT void
pfuSmokeDuration(pfuSmoke *smoke, float duration)
{
    smoke->duration = duration;
}    

PFUDLLEXPORT void
pfuSmokeDir(pfuSmoke *smoke, pfVec3 dir) 
{
    pfCopyVec3(smoke->direction, dir);
}
    
PFUDLLEXPORT void
pfuSmokeVelocity(pfuSmoke *smoke, float turbulence, float speed)
{
    smoke->speed = speed;
    smoke->turbulence = turbulence;
}

PFUDLLEXPORT void
pfuGetSmokeVelocity(pfuSmoke *smoke, float *turbulence, float *speed)
{
    *speed = smoke->speed;
    *turbulence = smoke->turbulence;
}

PFUDLLEXPORT void
pfuSmokeColor(pfuSmoke *smoke, pfVec3 bgn, pfVec3 end)
{
    pfCopyVec3(smoke->bgnColor, bgn);
    pfSubVec3(smoke->deltaColor, end, smoke->bgnColor);
}

PFUDLLEXPORT void
pfuSmokeTex(pfuSmoke *smoke, pfTexture *tex)
{
    smoke->tex = tex;
}


PFUDLLEXPORT void
pfuSmokeDensity(pfuSmoke *smoke, float dens, float diss, float expansion)
{
    if (dens <= 0.0f || diss <= 0.0f || expansion <= 0.0f)
	return;
    smoke->density = dens;
    smoke->dissipation = diss;
    smoke->puffInterval = diss / (float)MAX_PUFFS / dens; 
    smoke->expansion = expansion;
}    

PFUDLLEXPORT void
pfuGetSmokeDensity(pfuSmoke *smoke, float *dens, float *diss, float *expansion)
{
    *dens = smoke->density;
    *diss = smoke->dissipation;
    *expansion = smoke->expansion;
}    


PFUDLLEXPORT void
pfuDrawSmokes(pfVec3 eye)
{
    int	i, n;

    pfPushState();
    pfBasicState();
    pfEnable(PFEN_TEXTURE); 
    pfApplyTEnv(smokeTEnv);
    pfAlphaFunc((4.0f/255.0f), PFAF_GREATER);
    pfTransparency(PFTR_HIGH_QUALITY | PFTR_NO_OCCLUDE);
/*      zwritemask(0);
  blendfunction(BF_SA, BF_MSA);*/

    n = *smokeCount;
    for(i=0; i<n; i++)
        if(smokeList[i]->mode != PFUSMOKE_STOP)
	    drawSmoke(smokeList[i], eye);

/*    blendfunction(BF_ONE, BF_ZERO);
zwritemask(0xffffffff);*/

    pfPopState();
    
}

static void
drawSmoke(pfuSmoke *smoke, pfVec3 eye)
{
    double 	now = pfGetTime();
    double	deltaTime, age = 1.0f;
    float	speed, *tmp;
    float	puffInterval, puffDiss;
    pfVec3	dist;
    int	i, n, startPuff;

    /* Compute time elapsed from previous pfDrawSmoke */
    if(smoke->prevTime < 0.0f)
	deltaTime = 0.0f;
    else
    	deltaTime = now - smoke->prevTime;

    smoke->prevTime = now;

    if(smoke->startTime < 0.0f)
	smoke->startTime = now;

    if(smoke->duration > 0.0f)
    {
    	age = 1.0 - (now - smoke->startTime) / smoke->duration;

    	if(age <= 0.0)
    	{
	    smoke->mode = PFUSMOKE_STOP;
	    smoke->startTime = -1.;
	    smoke->lastPuffTime = -1.;
	    return;
        }

	age += .01f;
    }

    if (smoke->type != PFUSMOKE_EXPLOSION)
    {
	puffInterval = smoke->puffInterval / age;
	puffDiss = smoke->dissipation * age;
    }
    else
    {
	puffInterval = smoke->puffInterval;
	puffDiss = smoke->dissipation;
    }
   
    /* Initialize new puff of smoke if necessary */
    if(now - smoke->lastPuffTime >= puffInterval)
    {
	pfuPuff	*puff;

	smoke->lastPuffTime = now;
	puff = smoke->puffs[(smoke->startPuff + smoke->numPuffs) % MAX_PUFFS];
						
	initPuff(puff, smoke);

	smoke->numPuffs++;
    }

    if(smoke->tex)
    	pfApplyTex(smoke->tex);

    n = smoke->numPuffs;
    startPuff = smoke->startPuff;

    for(i=n-1; i>=0; i--)
    {
	pfuPuff *puff = smoke->puffs[(i+startPuff) % MAX_PUFFS];

	/* 
	 * Dissipate puff based on dissipation rate 
	*/
	puff->t = (now - puff->startTime) / puffDiss;

	/* Get rid of puff if completely transparent */
	if(puff->t >= 1.0f)
	{	    
	    smoke->startPuff = (smoke->startPuff + 1) % MAX_PUFFS;
	    smoke->numPuffs--;
	    continue;
	}

	/* 
	 * Transparency is cubed so it quickly dissipates as dissipation
	 * time is neared.
	*/
	puff->transp = puff->t * puff->t * puff->t * puff->t;

	if(smoke->type == PFUSMOKE_EXPLOSION)
	{
	    /* Radius is linear interpolation between 0 and expansion */
	    puff->radius = (now - puff->startTime) / puffDiss * 
				smoke->expansion * smoke->radius;
	}
	else
	{
	    /* Radius is linear interpolation between radius and expansion */
	    puff->radius = (1.0f - puff->transp) * smoke->radius + 
			puff->transp * smoke->expansion * smoke->radius;
	}

    	speed = puff->speed * deltaTime;
   	PFSCALE_VEC3(dist, speed, puff->direction);

	/* Translate puff based on velocity vector */
	{
 	    tmp = puff->origin;
	    PFADD_VEC3(tmp, tmp, dist);
	}

	/* Fade in brand new puff */
	if(i == smoke->numPuffs - 1 
	   && smoke->type != PFUSMOKE_EXPLOSION && smoke->type != PFUSMOKE_FIRE)
	{
	    puff->transp = 1.0f - (now - puff->startTime) / puffInterval;
	}

	drawPuff(puff, smoke, eye);
    }
}

#define SQRT3_2	.86602540378443864676f

static void
drawPuff(pfuPuff *puff, pfuSmoke *smoke, pfVec3 eye)
{
    pfVec4    	opaque;
    int	i;
#if 0
    float	r = puff->radius, *org = puff->origin;
    pfVec3    	vert;
#endif

    static const pfVec3 quad[] = {
	{-1.0f, 0.0f, -1.0f},
	{1.0f, 0.0f, -1.0f},
	{1.0f, 0.0f, 1.0f},
	{-1.0f, 0.0f, 1.0f}
    }; 

    pfVec3 		dquad[4], toEye;
    pfMatrix		mat;
    static pfVec3	vec = {0.0f, -1.0f, 0.0f};

    /* Fade fire to black smoke */
    if (smoke->type == PFUSMOKE_FIRE)
    {
   	 pfAddScaledVec3(opaque, smoke->bgnColor, 
			  puff->transp, smoke->deltaColor);
    	opaque[3] = 1.0f;
    }
    else
    {
   	 pfAddScaledVec3(opaque, smoke->bgnColor, 
			  puff->t, smoke->deltaColor);

    	opaque[3] = 1.0f - puff->transp;
    }

    /* 
     * Compute matrix which rotates puff to face the eyepoint 
    */
    PFSUB_VEC3(toEye, eye, puff->origin);

    pfNormalizeVec3(toEye);
    pfMakeVecRotVecMat(mat, vec, toEye);

    glColor4fv(opaque);
    glBegin(GL_QUADS);
    for(i=0; i<4; i++)
    {
	pfVec3		squad;

	/* Rotate texture coordinates to simulate turbulence */
	pfXformPt3(puff->tcoords[i], puff->tcoords[i], puff->rotMat);
	glTexCoord2fv(puff->tcoords[i]);

	/* Scale quad to current radius */
	PFSCALE_VEC3(squad, puff->radius, quad[i]);

	/* Offset puff based on radius */
	squad[1] -= smoke->radius;

	/* Rotate puff to follow viewer */
	pfXformPt3(dquad[i], squad, mat);

	/* Translate puff to origin */
    	PFADD_VEC3(dquad[i], dquad[i], puff->origin);
	glVertex3fv(dquad[i]);
    }
    glEnd();
}

#if 0
    bgnpolygon();
    vert[PF_Y] = puff->origin[PF_Y];
    vert[PF_X] = puff->origin[PF_X] - puff->radius;
    vert[PF_Z] = puff->origin[PF_Z] - puff->radius;
    t2f(tc[0]);
    v3f(vert);
    vert[PF_X] = puff->origin[PF_X] + puff->radius;
    t2f(tc[1]);
    v3f(vert);
    vert[PF_Z] = puff->origin[PF_Z] + puff->radius;
    t2f(tc[2]);
    v3f(vert);
    vert[PF_X] = puff->origin[PF_X] - puff->radius;
    t2f(tc[3]);
    v3f(vert);
    endpolygon();


	float	*org = puff->origin;
	float	*dh = dquad[i], *h = quad[i];
	float	sqLen, sqToView, ch, sh, cp, sp;

	PFSUB_VEC3(toView, eye, org);

	sqToView[0] = toView[0] * toView[0];
	sqToView[1] = toView[1] * toView[1];
	sqToView[2] = toView[2] * toView[2];
	
	sqLen = (NSQR1F)/(sqToView[0] + sqToView[1] + sqToView[2]);
	
	ch = SQR_LOOKUP(sqLen / sqToView[1]);
	if (sqToView[1] > 0.0f)
	    ch = -ch;
	
	sh = -SQR_LOOKUP(lensq1 * sqToView[0]);
	if (sqToView[0] < 0.0f)
	    sh = -sh;

	cp = SQR_LOOKUP(sqLen / sqToView[1]);
	if (sqToView[1] > 0.0f)
	    cp = -cp;
	
	sp = -SQR_LOOKUP(lensq1 * sqToView[0]);
	if (sqToView[0] < 0.0f)
	    sp = -sp;

    	/* prefetch values (as have regs for) to avoid repeated loads */
    	org0 = org[0];
    	org1 = org[1];

	s00 = quad[0][0] * r; 
        dquad[0][0] = c * s00 + org0;
        dquad[0][1] = org1 - s * s00;
        dquad[0][2] = org1 - s * s00;

	s10 = quad[1][0] * r;
        dquad[1][0] = c * s10 + org0;
        dquad[1][1] = org1 - s * s10;

	s20 = quad[2][0] * r; 
        dquad[2][0] = c * s20 + org0; 
        dquad[2][1] = org1 - s * s20;

	s30 = quad[3][0] * r;
        dquad[3][0] = c * s30 + org0; 
        dquad[3][1] = org1 - s * s30;
#endif


