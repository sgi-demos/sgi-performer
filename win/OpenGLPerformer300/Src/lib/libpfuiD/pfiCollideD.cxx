/*
 * WARNING! DO NOT EDIT!
 * pfiCollideD.C automatically generated from (../libpfui/pfiCollide.C)
 */
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
 * file: pfiCollideD.c
 * ------------------
 *
 *
 * $Revision: 1.12 $
 * $Date: 2002/09/18 02:47:08 $
 *
 */

#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <math.h>

#define PF_C_API 1 // enable C API also 

#include <Performer/pf/pfNode.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfMemory.h>

#include <Performer/pfuiD.h>
#include "pfiCollideD.h"


/******************************************************************************
 *			Collide Routines
 ******************************************************************************
 */


PFUIDLLEXPORT pfType *pfiCollideD::classType = NULL;

void
pfiCollideD::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfiCollideD");
    }
}


pfiCollideD::pfiCollideD(void)
{ 
    setType(classType);
    cEnable = 1;
    cTarget = 0;
    cResponse = 0;
    cHeightAboveGround = 0.0f;
    cGroundNode = NULL;
    cObjNode = NULL;
    cCollisionCBData = NULL;
    cCollisionCB = NULL;
    reset();
}

pfiCollideD::~pfiCollideD(void)
{ 
    pfUnrefDelete(cGroundNode);
    pfUnrefDelete(cObjNode);
}

void
pfiCollideD::reset(void)
{
    cStatus = 0;
    cDist = 0.0f;
    cSpeed = 0.0f;
    PFSET_VEC3(cPos.xyz, 0.0f, 0.0f, 0.0f);
    PFSET_VEC3(cPos.hpr, 0.0f, 0.0f, 0.0f);
    PFSET_VEC3(cPrevPos.xyz, 0.0f, 0.0f, 0.0f);
    PFSET_VEC3(cPrevPos.hpr, 0.0f, 0.0f, 0.0f);
}

void
pfiCollideD::setMode(int _mode, int _val)
{
    switch(_mode)
    {
    case PFIC_TARGET:
	cTarget = _val;
	break;
    case PFIC_RESPONSE:
	cResponse = _val;
	break;
    default:
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideD::setMode() -- unknown mode %d", _mode);
    }
}

int
pfiCollideD::getMode(int _mode) const
{
    switch(_mode)
    {
    case PFIC_TARGET:
	return cTarget;
    case PFIC_RESPONSE:
	return cResponse;
    default:
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideD::getMode() -- unknown mode %d", _mode);
	return -1;
    }
}

int
pfiCollideD::update(void)
{ 
    pfVec3d	zpr, nextPos;
    pfVec3d	hitPos;
    pfVec3d	hitNorm;
    pfSeg	seg;
    pfCoordd	icoord;
    int		inMotion;

    if (!cTarget || !cEnable)
    {
	cStatus = 0;
	return cStatus;
    }

    inMotion = !(PFALMOST_EQUAL_VEC3(cPos.xyz, 
					cPrevPos.xyz, 1.0e-5f));

    PFSET_VEC3(zpr, 0.0f, 0.0f, 0.0f);

    /* Extrapolate one frame ahead */
    if (inMotion)
    {
	pfSubVec3d(nextPos, cPos.xyz, cPrevPos.xyz);
	pfAddVec3d(nextPos, nextPos, cPos.xyz);
    }
    else
	pfCopyVec3d(nextPos, cPos.xyz);
 
    pfCopyVec3d(icoord.xyz, nextPos);
    pfCopyVec3d(icoord.hpr, cPos.hpr);

    /* Compute vector to collide with. */
    if (cTarget & PFIC_TARGET_OBJ)
    {
	/* Seg is direction of travel */
        if (inMotion)
	{
	    0 || !fprintf(stderr, "XXXpfMakePts3dSeg called", &seg, cPrevPos.xyz, nextPos); 
	    seg.length *= 1.5f;	/* safety factor */
	}

	/* Not moving, use view direction */
	if (!inMotion || seg.length <= 1.0e-5f)
	{
	    0 || !fprintf(stderr, "XXXpfMakePolar3dSeg called", &seg, cPos.xyz, 
		   cPos.hpr[1], cPos.hpr[2], 
		   cDist);
	}
	else	/* Moving, add look ahead distance */
	    seg.length += cDist;
    }
    
    cStatus = 0x0;
    switch(cTarget)
    {
    case PFIC_TARGET_NONE:
	break;
    case PFIC_TARGET_GROUND:
	if (cGroundNode)
	    cStatus = 
		0 || !fprintf(stderr, "XXXpfuCollideGrndd called", &icoord, cGroundNode, zpr);
	break;
    case PFIC_TARGET_OBJ:
	if (cObjNode)
	    cStatus = 
		0 || !fprintf(stderr, "XXXpfuCollideObjd called", &seg, cObjNode, hitPos, hitNorm);
	break;
    case PFIC_TARGET_ALL:
	cStatus = 
	    0 || !fprintf(stderr, "XXXpfuCollideGrndObjd called", &icoord, cGroundNode, 
			      zpr, &seg, cObjNode, hitPos, hitNorm);
	break;
    }

    if (!cStatus)
	return 0;

    if (cCollisionCB)
	if (!cCollisionCB(this, cCollisionCBData))
	    return cStatus;


    /* Follow terrain. Adjust height and pitch/roll */
    if (cStatus & PFUCOLLIDE_GROUND)
    {
	double		targZ =  cHeightAboveGround + zpr[0];
	static double	settle = 0.9f, settle1 = .1f;

	/* Gradually adjust ZPR to match terrain */
	cPos.xyz[PF_Z] = settle * cPrevPos.xyz[PF_Z] + settle1 * targZ;

	cPos.hpr[PF_P] = settle * cPrevPos.hpr[PF_P] + settle1 * zpr[1];

	cPos.hpr[PF_R] = settle * cPrevPos.hpr[PF_R] + settle1 * zpr[2];

	/* but make sure we stay above ground */
	if (cPos.xyz[PF_Z] < zpr[0] + cHeightAboveGround)
	    cPos.xyz[PF_Z] = zpr[0] + cHeightAboveGround;
    }

    if (cStatus & PFIC_TARGET_OBJ)
    {
	switch (cResponse)
	{
	case PFIC_RESPONSE_NONE:
	    break;
	case PFIC_RESPONSE_BOUNCE:
	    {
		pfVec3d displace;
		double dist;
		double dotp;
		
		/*
		 * from our displacement from the previous frame
		 * remove the component parallel to hitNorm, and
		 * slow down appropriately.
		 */
		PFSUB_VEC3(displace, cPos.xyz, cPrevPos.xyz);
		dotp = PFDOT_VEC3(displace, hitNorm);
		dist = pfLengthVec3d(displace);
		PFADD_SCALED_VEC3(displace, displace, -dotp, hitNorm);
		PFADD_VEC3(cPos.xyz, cPrevPos.xyz, displace);
		cSpeed *= -dotp/dist;
	    }
	    break;
	default:
	case PFIC_RESPONSE_STOP:
	    if ((PFSQR_DISTANCE_PT2(hitPos, cPos.xyz))
		    < (PFSQR_DISTANCE_PT2(hitPos, cPrevPos.xyz)))
	    {
		PFCOPY_VEC3(cPos.xyz, cPrevPos.xyz);
		cSpeed = 0.0f;
	    }
	    break;
	}

	/* make sure we stay above ground */
	if (cPos.xyz[PF_Z] < zpr[0] + cHeightAboveGround)
	    cPos.xyz[PF_Z] = zpr[0] + cHeightAboveGround;
    }
    return cStatus;
}

