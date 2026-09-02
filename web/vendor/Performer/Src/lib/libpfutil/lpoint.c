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
/*
 * lpoint.c
 * --------------
 * $Revision: 1.11 $
 * $Date: 2002/05/15 00:31:09 $
 */

#include <string.h>
#ifndef WIN32
#include <values.h>
#else
#define powf pow
#endif
#include <math.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>

	/*------------------------------------------------------*/

/* Make reflection map which describes the lightpoint envelope */
PFUDLLEXPORT void
pfuMakeLPStateShapeTex(pfLPointState *lps, pfTexture *tex, int size)
{
    int                 i, j, shapeMode;
    char                *img, *image;
    pfMatrix            rollMat;
    pfVec3              norm;
    float               alpha;
    float		henv, venv, roll, falloff, ambient;
    float		horizParam, vertParam, ambientScale;
    static pfVec3       eye = {0.0f, 1.0f, 0.0f};
    static float	eta = 1.0e-15f;

    image = img = (char*)pfCalloc(1, sizeof(char) * size * size * 2, 
				  pfGetArena(tex));

    pfGetLPStateShape(lps, &henv, &venv, &roll, &falloff, &ambient);
    shapeMode = pfGetLPStateMode(lps, PFLPS_SHAPE_MODE);
    ambientScale = 1.0f - ambient;
    pfMakeRotMat(rollMat, roll, 0.0f, 1.0f, 0.0f);

    if (henv > 180.0f - eta && henv < 180.0f + eta)
        horizParam = 0.0f;
    else
    {
        horizParam = tanf(PF_DEG2RAD(henv / 2.0f));
        horizParam = 1.0f / (horizParam * horizParam);
    }

    if (venv > 180.0f - eta && venv < 180.0f + eta)
        vertParam = 0.0f;
    else
    {
        vertParam = tanf(PF_DEG2RAD(venv / 2.0f));
        vertParam = 1.0f / (vertParam * vertParam);
    }

    /* 'i' ranges over 't' texture coordinate which maps to Z */
    for (i=0; i<size; i++)
    {
        norm[PF_Z] = ((float)i / (size - 1.0f) - 0.5f) * 2.0f;

        /* 'j' ranges over 's' texture coordinate which maps to X */
        for (j=0; j<size; j++)
        {
	    float		sqlen;
	    pfMatrix		dirMat;
	    pfVec3		viewVec;
	    static pfVec3      	zaxis = {0.0f, 0.0f, 1.0f};

            /* Compute normal of unit sphere */
            norm[PF_X] = ((float)j / (size - 1.0f) - 0.5f) * 2.0f;
            norm[PF_Y] = 1.0f - (norm[PF_X] *  norm[PF_X] + 
				 norm[PF_Z] *  norm[PF_Z]);
            if (norm[PF_Y] < 0.0f)
            {
		*img++ = 0xff;	/* Intensity is ON */
                *img++ = ambient * 255.0f;
                continue;
            }
            norm[PF_Y] = pfSqrt(norm[PF_Y]);

	    /* Construct coordinate system of lightpoint with normal
	     * becoming the +Y axis and "up" being aligned with the 
	     * +Z axis in object coordinates.
	    */
	    pfCopyVec3(dirMat[PF_Y], norm);

	    /* Compute new X axis */
	    pfCrossVec3(dirMat[PF_X], norm, zaxis);	

	    /* Check for norm collinear with +Z axis */
	    if ((sqlen = pfDotVec3(dirMat[PF_X], dirMat[PF_X])) < 1e-15)
	    {
		if (pfDotVec3(dirMat[PF_Y], zaxis) > 0.0f)
		{
		    pfSetVec3(dirMat[PF_X], 1.0f,  0.0f, 0.0f);
		    pfSetVec3(dirMat[PF_Z], 0.0f, -1.0f, 0.0f);
		}
		else
		{
		    pfSetVec3(dirMat[PF_X], 1.0f, 0.0f, 0.0f);
		    pfSetVec3(dirMat[PF_Z], 0.0f, 1.0f, 0.0f);
		}
	    }
	    else
	    {
		/* Normalize new X axis */
		sqlen = 1.0f / sqlen;
		PFSCALE_VEC3(dirMat[PF_X], sqlen, dirMat[PF_X]);

		/* Compute new Z axis */
		pfCrossVec3(dirMat[PF_Z], dirMat[PF_X], dirMat[PF_Y]);  
	    }

	    /* Pre-roll new coordinate system */
	    if (roll)
		pfPreMultMat(dirMat, rollMat);

	    /* Find image of eye in new coordinate system */
	    viewVec[PF_X] = PFDOT_VEC3(eye, dirMat[PF_X]);
	    viewVec[PF_Y] = PFDOT_VEC3(eye, dirMat[PF_Y]);
	    viewVec[PF_Z] = PFDOT_VEC3(eye, dirMat[PF_Z]);

	    if (shapeMode == PFLPS_SHAPE_MODE_UNI && viewVec[PF_Y] < 0.0f)
		alpha = ambient;
	    else
	    {
		float   x, z, intens;

		if (PF_ABSLT(viewVec[PF_Y], eta))
		    x = z = PF_HUGEVAL;
		else
		{
		    float       iy;
		    
		    iy = 1.0f / viewVec[PF_Y];
		    x = viewVec[PF_X] * iy;
		    z = viewVec[PF_Z] * iy;
		}
		
		intens = 1.0f - ((x * x) * horizParam + (z * z) * vertParam);
		
		if (intens <= 0.0f)
		    alpha = ambient;
		else if (intens >= 1.0f)
		    alpha = 1.0f;
		else
		    alpha = powf(intens, falloff) * ambientScale + ambient;
	    }

	    *img++ = 0xff;	/* Intensity is ON */
            *img++ = alpha * 255.0f;
        }
    }
    pfTexImage(tex, (uint*)image, 2, size, size, 1);
}

#define NOMINAL_NEAR_PIX_DIST   1236.075f

/* 
 * Make texture map which descibes light point fade ramp and/or
 * fog ramp. 
*/
PFUDLLEXPORT void
pfuMakeLPStateRangeTex(pfLPointState *lps, pfTexture *tex, int size, pfFog *fog) 
{
    float       isize;
    int         i, j, dim, tsize;
    char        *img, *image;

    int		transpRangeMode, fogRangeMode;
    float	transpScale, actualSize, transpPixSize, transpClamp, transpExp;
    float	maxTranspRange, minTranspRange, deltaTranspRange;
    float	maxFogRange, minFogRange, deltaFogRange;

    transpRangeMode = pfGetLPStateMode(lps, PFLPS_TRANSP_MODE);
    fogRangeMode = pfGetLPStateMode(lps, PFLPS_FOG_MODE);
    actualSize = pfGetLPStateVal(lps, PFLPS_SIZE_ACTUAL);
    transpScale = pfGetLPStateVal(lps, PFLPS_TRANSP_SCALE);
    transpPixSize = pfGetLPStateVal(lps, PFLPS_TRANSP_PIXEL_SIZE);
    transpExp = pfGetLPStateVal(lps, PFLPS_TRANSP_EXPONENT);
    transpClamp = pfGetLPStateVal(lps, PFLPS_TRANSP_CLAMP);

    dim = (transpRangeMode == PFLPS_TRANSP_MODE_TEX) + 
          (fogRangeMode == PFLPS_FOG_MODE_TEX);

    if (dim == 0)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuMakeLPStateRangeTex() 0x%x "
                 "is not configured to use texturing for fog or "
                 "transparency.\n", lps);
        return;
    }
    
    if (dim == 1)
    {
        image = img = (char*)pfCalloc(1, sizeof(char) * size * 2, 
				      pfGetArena(tex));
    }
    else
    {
        image = img = (char*)pfCalloc(1, sizeof(char) * size * size * 2, 
				      pfGetArena(tex));
    }

    if (transpRangeMode == PFLPS_TRANSP_MODE_TEX)
    {
        minTranspRange = 
            transpScale * actualSize *
                NOMINAL_NEAR_PIX_DIST / 
                    (transpScale * transpPixSize);

        maxTranspRange = transpScale * actualSize *
            NOMINAL_NEAR_PIX_DIST / 
                (transpScale * transpPixSize - 
                 powf(1.0f - transpClamp, 1.0f / transpExp));
        
        deltaTranspRange = maxTranspRange - minTranspRange;
    }
    if (fogRangeMode == PFLPS_FOG_MODE_TEX)
    {
	float	onset, opaque, a, b;

	pfGetFogOffsets(fog, &onset, &opaque);
	pfGetFogRange(fog, &a, &b);
	
        minFogRange = onset + a;
        maxFogRange = opaque + b;
        deltaFogRange = maxFogRange - minFogRange;
    }

    tsize = (transpRangeMode == PFLPS_TRANSP_MODE_TEX) ? size : 1;
    isize = 1.0f / ((float)size - 1.0f);

    for (i=0; i<tsize; i++)     /* transp loop */
    {
        float   s, a, d, f;

        if (transpRangeMode == PFLPS_TRANSP_MODE_TEX)
        {
            d = minTranspRange + (float)i * isize * deltaTranspRange;
            s = actualSize * NOMINAL_NEAR_PIX_DIST / d;

            if (transpPixSize > s)
            {
                if (transpExp == 1.0f)
		    a = 1.0f - transpScale * (transpPixSize - s);
                else
                    a = 1.0f - transpScale * 
                        powf((transpPixSize - s), transpExp);

                if (a < transpClamp)
                    a = transpClamp;
            }
            else
                a = 1.0f;
        }
        else
            a = 1.0f;

        if (fogRangeMode == PFLPS_FOG_MODE_TEX)
        {
            for (j=0; j<size; j++)     /* fog loop */
            {
                d = minFogRange + (float)j * isize * deltaFogRange;
                f = 1.0f - pfGetFogDensity(fog, d);

                *img++ = 0xff;          /* intensity */
                *img++ = f * a * 0xff;  /* alpha */
            }
        }
        else
        {
	    *img++ = 0xff;	/* Intensity is ON */
            *img++ = a * 0xff;  /* alpha */
        }
    }

    pfTexRepeat(tex, PFTEX_WRAP, PFTEX_CLAMP);
    pfTexFilter(tex, PFTEX_MAGFILTER, PFTEX_BILINEAR);
    pfTexFilter(tex, PFTEX_MINFILTER, PFTEX_MIPMAP_TRILINEAR);
    pfTexFormat(tex, PFTEX_INTERNAL_FORMAT, PFTEX_IA_8);
    if (dim == 1)
        pfTexImage(tex, (uint*)image, 2, size, 1, 1);
    else
        pfTexImage(tex, (uint*)image, 2, size, size, 1);

    return;
}


