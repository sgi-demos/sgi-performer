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
 * path.c -- multi-segment path DCS animation
 *
 * $Revision: 1.23 $
 * $Date: 2002/05/15 00:31:09 $
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#ifdef WIN32
#define fatan2 atan2
#else
#ifdef _POSIX_SOURCE
float fatan2(float y, float x);
#endif
#endif

#include <Performer/pf.h>
#include <Performer/prmath.h>

#include <Performer/pfutil.h>
static int computeFillet (pfVec3 a, pfVec3 b, pfVec3 c, float radius,
			  pfVec3 h, pfVec3 i, pfVec3 g, pfVec2 angles);

/*
 *	pfuNewPath -- make a new (and quite short) path
 */

PFUDLLEXPORT pfuPath *pfuNewPath(void)
{
    pfuPath *path;
    
    /* allocate new path structure */
    if ((path = (pfuPath *)malloc(sizeof(pfuPath))) == NULL)
    {
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, "pfuNewPath: "
	    "memory allocation failure");
	return NULL;
    }
    
    /* initialize new path structure */
    path->head = NULL;
    path->speed = 1.0f;
    path->pitch = 1.0f;
    path->roll = 0.0f;
    path->desired = 0.0f;
    path->rate = 0.0f;
    path->delay = 0.0f;
    path->position = 0.0f;
    path->here = NULL;
    
    /* return address of new path structure to caller */
    return path;
}

/*
 *	pfuSharePath -- make a new path that shares segments
 */

PFUDLLEXPORT pfuPath *pfuSharePath (pfuPath *share)
{
    pfuPath *path;
    
    /* make new path structure */
    if ((path = pfuNewPath()) == NULL)
	return NULL;
    
    /* share path segment chain with indicated path */
    path->head = share->head;
    
    /* return address of new path structure to caller */
    return path;
}

/*
 *	pfuCopyPath -- make a new path by copying
 */

PFUDLLEXPORT pfuPath *pfuCopyPath (pfuPath *copy)
{
    pfuPath *path;
    pfuPathSeg *segment;
    
    /* make new path structure */
    if ((path = pfuNewPath()) == NULL)
	return NULL;
    
    /* copy existing segments */
    for (segment = copy->head; segment != NULL; segment = segment->next)
    {
	switch (segment->type)
	{
	case PATYPE_ARC:
	    pfuAddArc(path, segment->center, segment->radius, segment->angles);
	    break;
	    
	case PATYPE_LINE:
	    pfuAddPath(path, segment->start, segment->final);
	    break;
	    
	case PATYPE_FILLET:
	    /* things are pretty weird if we get here */
	    pfuAddFillet(path, segment->radius);
	    break;
	    
	case PATYPE_SPEED:
	    pfuAddSpeed(path, segment->desired, segment->rate);
	    break;
	    
	case PATYPE_DELAY:
	    pfuAddDelay(path, segment->delay);
	    break;
	    
	default:
	    pfNotify(PFNFY_DEBUG, PFNFY_USAGE, "pfuCopyPath: "
		"bad segment type in path");
	    return NULL;
	}
    }
    
    /* return address of new path structure to caller */
    return path;
}

/*
 *	pfuClosePath -- connect first and last path segments
 */

PFUDLLEXPORT pfuPath *pfuClosePath (pfuPath *path)
{
    pfuPathSeg *old;
    
    /* find tail of segment chain */
    old = path->head;
    while (old != NULL && old->next != NULL)
	old = old->next;
    
    /* connect first and last path segments */
    path->head->prev = old;
    old->next = path->head;
    
    /* return address of new path structure to caller */
    return path;
}

/*
 *	pfuPrintPath -- print a path and its segments
 */

PFUDLLEXPORT int pfuPrintPath (pfuPath *path)
{
    pfuPathSeg *segment;
    
    /* print header information */
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "          head = 0x%p", path->head);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "          here = 0x%p", path->here);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "         speed = %g", path->speed);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "      position = %g", path->position);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "         delay = %g", path->delay);
    
    /* print segment information */
    for (segment = path->head; segment != NULL; segment = segment->next)
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, NULL);
	
	switch (segment->type)
	{
	case PATYPE_ARC:
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    arc center = %g %g %g",
		segment->center[PF_X],
		segment->center[PF_Y],
		segment->center[PF_Z]);
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    arc radius = %g", 
		segment->radius);
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    arc angles = %g %g",
		segment->angles[PF_X],
		segment->angles[PF_Y]);
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "        length = %g", 
		segment->length);
	    break;
	    
	case PATYPE_LINE:
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    line start = %g %g %g",
		segment->start[PF_X],
		segment->start[PF_Y],
		segment->start[PF_Z]);
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    line final = %g %g %g",
		segment->final[PF_X],
		segment->final[PF_Y],
		segment->final[PF_Z]);
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "        length = %g", 
		segment->length);
	    break;
	    
	case PATYPE_FILLET:
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "        radius = %g", 
		segment->radius);
	    break;
	    
	case PATYPE_SPEED:
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "       desired = %g", 
		segment->desired);
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "          rate = %g", 
		segment->rate);
	    break;
	    
	case PATYPE_DELAY:
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "         delay = %g", 
		segment->delay);
	    break;
	    
	default:
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, "bad segment type in path");
	    return NULL;
	}
    }
    
    /* indicate succes to caller */
    return 0;
}

/*
 *	pfuAddArc -- append an arc-segment definition to path
 */
PFUDLLEXPORT
int pfuAddArc (pfuPath *path, pfVec3 center, float radius, pfVec2 angles)
{
    pfuPathSeg *segment;
    pfuPathSeg *old;
    
    /* allocate a new segment */
    if ((segment = (pfuPathSeg *)malloc(sizeof(pfuPathSeg))) == NULL)
	return -1;
    
    /* initialize new segment */
    segment->type = PATYPE_ARC;
    segment->radius = radius;
    PFCOPY_VEC3(segment->center, center);
    PFCOPY_VEC2(segment->angles, angles);
    
    /* compute path segment length */
    segment->length = 2.0f*PF_PI*segment->radius*PF_ABS(segment->angles[1])/360.0f;
    
    /* find tail of segment chain */
    old = path->head;
    while (old != NULL && old->next != NULL)
	old = old->next;
    
    /* add segment as first item in chain */
    if (old == NULL)
    {
	path->head = segment;
	segment->next = NULL;
	segment->prev = NULL;
    }
    /* add segment after tail of chain */
    else
    {
	segment->next = NULL;
	segment->prev = old;
	old->next = segment;
    }
    
    /* indicate succes to caller */
    return 0;
}

/*
 *	pfuAddFillet -- append a fillet-segment definition to path
 */
PFUDLLEXPORT
int pfuAddFillet (pfuPath *path, float radius)
{
    pfuPathSeg *segment;
    pfuPathSeg *old;
    
    /* allocate a new segment */
    if ((segment = (pfuPathSeg *)malloc(sizeof(pfuPathSeg))) == NULL)
	return -1;
    
    /* initialize new segment */
    segment->type = PATYPE_FILLET;
    segment->radius = radius;
    PFSET_VEC3(segment->center, 0.0f, 0.0f, 0.0f);
    PFSET_VEC2(segment->angles, 0.0f, 0.0f);
    
    /* compute path segment length */
    segment->length = 0.0f;
    
    /* find tail of segment chain */
    old = path->head;
    while (old != NULL && old->next != NULL)
	old = old->next;
    
    /* add segment as first item in chain */
    if (old == NULL)
    {
	path->head = segment;
	segment->next = NULL;
	segment->prev = NULL;
    }
    /* add segment after tail of chain */
    else
    {
	segment->next = NULL;
	segment->prev = old;
	old->next = segment;
    }
    
    /* indicate succes to caller */
    return 0;
}

/*
 *	pfuAddPath -- append a line-segment definition to path
 */
PFUDLLEXPORT
int pfuAddPath (pfuPath *path, pfVec3 start, pfVec3 final)
{
    pfuPathSeg *segment;
    pfuPathSeg *old;
    pfVec3 delta;
    
    /* allocate a new segment */
    if ((segment = (pfuPathSeg *)malloc(sizeof(pfuPathSeg))) == NULL)
	return -1;
    
    /* initialize new segment */
    segment->type = PATYPE_LINE;
    PFCOPY_VEC3(segment->start, start);
    PFCOPY_VEC3(segment->final, final);
    
    /* precompute constant segment values */
    segment->length = PFDISTANCE_PT3(segment->start, segment->final);
    
    PFSUB_VEC3(delta, segment->final, segment->start);
    #ifndef __linux__
    segment->orient[PF_H] = PF_RAD2DEG(fatan2(delta[PF_Y], delta[PF_X]));
    segment->orient[PF_P] = PF_RAD2DEG(fatan2(delta[PF_Z], segment->length));
    #else
    segment->orient[PF_H] = PF_RAD2DEG(atan2(delta[PF_Y], delta[PF_X]));
    segment->orient[PF_P] = PF_RAD2DEG(atan2(delta[PF_Z], segment->length));
    #endif  /* __linux__ */
    segment->orient[PF_R] = 0.0f;
    
    if (segment->orient[PF_H] < 0.0f)
	segment->orient[PF_H] += 360.0f;
    if (segment->orient[PF_P] < 0.0f)
	segment->orient[PF_P] += 360.0f;
    
    /* find tail of segment chain */
    old = path->head;
    while (old != NULL && old->next != NULL)
	old = old->next;
    
    /* add segment as first item in chain */
    if (old == NULL)
    {
	path->head = segment;
	segment->next = NULL;
	segment->prev = NULL;
    }
    /* add segment after tail of chain */
    else
    {
	segment->next = NULL;
	segment->prev = old;
	old->next = segment;
    }
    
    /* is the creation of a fillet called for ? */
    if (segment->prev       != NULL && segment->prev->type       == PATYPE_FILLET &&
	segment->prev->prev != NULL && segment->prev->prev->type == PATYPE_LINE)
    {
	pfuPathSeg *ab;
	pfuPathSeg *fillet;
	pfuPathSeg *bc;
	
	pfVec3 newFinal;
	pfVec3 newStart;
	
	ab     = segment->prev->prev;
	fillet = segment->prev;
	bc     = segment;
	
	/* lines must connect */
	if (!PFEQUAL_VEC3(ab->final, bc->start))
	    return -1;
	
	/* determine fillet parameters */
	if (computeFillet(ab->start, ab->final, bc->final, fillet->radius,
			   newFinal, newStart, fillet->center, fillet->angles) < 0)
	    return -1;
	
	PFCOPY_VEC3(ab->final, newFinal);
	PFCOPY_VEC3(bc->start, newStart);
	
	/* horrible part -- recompute segment-specific stuff */
	ab->length = PFDISTANCE_PT3(ab->start, ab->final);
	fillet->length = 2.0f*PF_PI*fillet->radius*PF_ABS(fillet->angles[1])/360.0f;
	bc->length = PFDISTANCE_PT3(bc->start, bc->final);
	
	/* convert fillet to an arc */
	fillet->type = PATYPE_ARC;
    }
    
    /* indicate succes to caller */
    return 0;
}

/*
 *	pfuAddSpeed -- append a speed-segment definition to path
 */
PFUDLLEXPORT
int pfuAddSpeed (pfuPath *path, float desired, float rate)
{
    pfuPathSeg *segment;
    pfuPathSeg *old;
    
    /* allocate a new segment */
    if ((segment = (pfuPathSeg *)malloc(sizeof(pfuPathSeg))) == NULL)
	return -1;
    
    /* initialize new segment */
    segment->type = PATYPE_SPEED;
    segment->desired = desired;
    segment->rate = rate;
    
    /* precompute constant segment values */
    segment->length = 0.0f;
    
    segment->orient[PF_H] = 0.0f;
    segment->orient[PF_P] = 0.0f;
    segment->orient[PF_R] = 0.0f;
    
    /* find tail of segment chain */
    old = path->head;
    while (old != NULL && old->next != NULL)
	old = old->next;
    
    /* add segment as first item in chain */
    if (old == NULL)
    {
	path->head = segment;
	segment->next = NULL;
	segment->prev = NULL;
    }
    /* add segment after tail of chain */
    else
    {
	segment->next = NULL;
	segment->prev = old;
	old->next = segment;
    }
    
    /* indicate succes to caller */
    return 0;
}

/*
 *	pfuAddDelay -- append a delay-segment definition to path
 */
PFUDLLEXPORT
int pfuAddDelay (pfuPath *path, float delay)
{
    pfuPathSeg *segment;
    pfuPathSeg *old;
    
    /* allocate a new segment */
    if ((segment = (pfuPathSeg *)malloc(sizeof(pfuPathSeg))) == NULL)
	return -1;
    
    /* initialize new segment */
    segment->type = PATYPE_DELAY;
    segment->delay = delay;
    
    /* precompute constant segment values */
    segment->length = 0.0f;
    
    segment->orient[PF_H] = 0.0f;
    segment->orient[PF_P] = 0.0f;
    segment->orient[PF_R] = 0.0f;
    
    /* find tail of segment chain */
    old = path->head;
    while (old != NULL && old->next != NULL)
	old = old->next;
    
    /* add segment as first item in chain */
    if (old == NULL)
    {
	path->head = segment;
	segment->next = NULL;
	segment->prev = NULL;
    }
    /* add segment after tail of chain */
    else
    {
	segment->next = NULL;
	segment->prev = old;
	old->next = segment;
    }
    
    /* indicate succes to caller */
    return 0;
}

/*
 *	pfuAddFile -- add path segments from a file
 */
PFUDLLEXPORT
int pfuAddFile (pfuPath *path, char *name)
{
    char location[256];
    char buffer[256];
    FILE *fp;
    
    /* open indicated file */
    if (!pfFindFile(name, location, R_OK))
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, "pfuAddFile: "
	    "unable to find file \"%s\"", name);
	return -1;
    }
    if ((fp = fopen(location, "r")) == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, "pfuAddFile: "
	    "unable to open file \"%s\"", location);
	return -2;
    }
    
    /* read file contents */
    while (fgets(buffer, 256, fp) != NULL)
    {
	char *comment;
	char keyword[256];
	
	/* look for comment character */
	if ((comment = strchr(buffer, '#')) != NULL)
	    *comment = '\0';
	
	/* look for segment-type keyword */
	if (sscanf(buffer, "%s", keyword) != 1)
	    continue;
	
	/* process line-segments */
	if (strcmp(keyword, "line") == 0)
	{
	    pfVec3	start;
	    pfVec3	final;
	    
	    if (sscanf(buffer, "%*s %f %f %f %f %f %f",
		       &start[PF_X], &start[PF_Y], &start[PF_Z],
		       &final[PF_X], &final[PF_Y], &final[PF_Z]) != 6)
		pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuAddFile: "
		    "bad line definition in segment file");
	    else
		pfuAddPath(path, start, final);
	}
	/* process arc-segments */
	else
	if (strcmp(keyword, "arc") == 0)
	{
	    pfVec3	center;
	    float	radius;
	    pfVec2	angles;
	    
	    if (sscanf(buffer, "%*s %f %f %f %f %f %f",
		       &center[PF_X], &center[PF_Y], &center[PF_Z],
		       &radius, &angles[PF_X], &angles[PF_Y]) != 6)
		pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuAddFile: "
		    "bad arc definition in segment file");
	    else
		pfuAddArc(path, center, radius, angles);
	}
	/* process fillet-segments */
	else
	if (strcmp(keyword, "fillet") == 0)
	{
	    float	radius;
	    
	    if (sscanf(buffer, "%*s %f", &radius) != 1)
		pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuAddFile: "
		    "bad fillet definition in segment file");
	    else
		pfuAddFillet(path, radius);
	}
	/* process speed-segments */
	else
	if (strcmp(keyword, "speed") == 0)
	{
	    float	desired;
	    float	rate;
	    
	    if (sscanf(buffer, "%*s %f %f", &desired, &rate) != 2)
		pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuAddFile: "
		    "bad speed definition in segment file");
	    else
		pfuAddSpeed(path, desired, rate);
	}
	/* process delay-segments */
	else
	if (strcmp(keyword, "delay") == 0)
	{
	    float	delay;
	    
	    if (sscanf(buffer, "%*s %f", &delay) != 1)
		pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuAddFile: "
		    "bad delay definition in segment file");
	    else
		pfuAddDelay(path, delay);
	}
	/* process close commands */
	else
	if (strcmp(keyword, "close") == 0)
	    pfuClosePath(path);
	/* report failure */
	else
	    pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuAddFile: "
		"unknown keyword \"%s\" in segment file", keyword);
    }

    /* close file */
    fclose(fp);
    
    /* indicate succes to caller */
    return 0;
}

/*
 *	pfuFollowPath -- update position along path
 */

PFUDLLEXPORT int
pfuFollowPath(pfuPath *path, float seconds, pfVec3 where, pfVec3 orient)
{
    float distance;
    float t;
    float angle;
    float sine;
    float cosine;
    float way = 1.0f;
    float now;
    
    /* segment defaults to first if unspecified */
    if (path->here == NULL)
	path->here = path->head;
    
    /* NULL segment means nowhere to go! */
    if (path->here == NULL)
	return -1;
    
    /* are we in a pause condition ? */
    if (path->delay != 0.0f)
    {
	distance = 0;
	
	/* terminate pause mode */
	if (pfGetTime() >= path->delay)
	    path->delay = 0.0f;
    }
    else
    {
	/* update path speed */
	path->speed += path->rate;
	
	/* clamp speed at desired velocity */
	if ((path->rate > 0.0f && path->speed >= path->desired) ||
	    (path->rate < 0.0f && path->speed <= path->desired))
	{
	    path->rate = 0.0f;
	    path->speed = path->desired;
	}
	
	/* compute travel distance */
	distance = seconds*path->speed;
    }
    
    /* are we moving forward through segment-chain */
    if (distance > 0.0f)
    {
	/* advance through completely-spanned segments */
	while (distance > 0.0f && distance >= path->here->length - path->position)
	{
	    /* move along path */
	    distance -= path->here->length - path->position;
	    
	    /* perform segment actions */
	    switch (path->here->type)
	    {
	    case PATYPE_SPEED:
		path->desired = path->here->desired;
		path->rate = path->here->rate;
		break;
		
	    case PATYPE_DELAY:
		now = pfGetTime();
		path->delay = now + path->here->delay;
		distance = 0.0f;
		break;
		
	    default:
		break;
	    }
	    
	    /* advance to next segment if not at end */
	    if (path->here->next != NULL)
	    {
		path->here = path->here->next;
		path->position = 0.0f;
	    }
	    /* otherwise, just park at the end (shuttle? cycle?) */
	    else
	    {
		path->speed = 0.0f;
		path->position = path->here->length;
		distance = 0.0f;
	    }
	}
	
	/* remember new position */
	path->position += distance;
    }
    /* are we moving backward through segment-chain */
    else
	if (distance < 0.0f)
	{
	    /* now its just a distance, direction is assumed */
	    distance = -distance;
	    way = -1.0f;
	    
	    /* advance through completely-spanned segments */
	    while (distance > 0.0f && distance >= path->position)
	    {
		/* move along path */
		distance -= path->position;
		
		/* perform segment actions */
		switch (path->here->type)
		{
		case PATYPE_SPEED:
		    path->desired = path->here->desired;
		    path->rate = path->here->rate;
		    break;
		    
		case PATYPE_DELAY:
		    now = pfGetTime();
		    path->delay = now + path->here->delay;
		    distance = 0.0f;
		    break;
		    
		default:
		    break;
		}
		
		/* retreat to previous segment if not at start */
		if (path->here->prev != NULL)
		{
		    path->here = path->here->prev;
		    path->position = path->here->length;
		}
		/* otherwise, just park at the front (shuttle? cycle?) */
		else
		{
		    path->speed = 0.0f;
		    path->position = 0.0f;
		    distance = 0.0f;
		}
	    }
	    
	    /* remember new position */
	    path->position -= distance;
	}
    
    /* determine segment mediation parameter: 0=start 1=end */
    t = path->position/path->here->length;
    
    /* determine current position */
    switch (path->here->type)
    {
    case PATYPE_ARC:
        angle = path->here->angles[0] + t*path->here->angles[1];
        pfSinCos(angle, &sine, &cosine);
        where[PF_X] = path->here->center[PF_X] + path->here->radius*cosine;
        where[PF_Y] = path->here->center[PF_Y] + path->here->radius*sine;
        where[PF_Z] = path->here->center[PF_Z];

        orient[PF_H] = way*90.0f;
        if (path->here->angles[1] < 0.0f)
            orient[PF_H] = -orient[PF_H];
        orient[PF_H] += angle;

        if (orient[PF_H] < 0.0f)
            orient[PF_H] += 360.0f;
        if (orient[PF_H] > 360.0f)
            orient[PF_H] -= 360.0f;

        orient[PF_P] = 0.0f;
        orient[PF_R] = -way*90.0f;
        if (path->here->angles[1] < 0.0f)
            orient[PF_R] = -orient[PF_R];

	break;
	
    case PATYPE_LINE:
	PFCOMBINE_VEC3(where, 1.0f-t, path->here->start, t, path->here->final);
	PFCOPY_VEC3(orient, path->here->orient);
	if (way < 0.0f)
	{
	    orient[PF_H] += 180.0f;
	    orient[PF_P]  = -orient[PF_P];
	}
	break;
	
    case PATYPE_FILLET:
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuFollowPath: "
	    "encountered fillet segment in path");
	return -2;
	
    case PATYPE_SPEED:
    case PATYPE_DELAY:
	break;
	
    default:
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuFollowPath: "
	    "bad segment type in path");
	return -3;
    }
    
    /* scale pitch and roll parameters */
    orient[PF_P] *= path->pitch;
    orient[PF_R] *= path->roll;
    
    /* indicate succes to caller */
    return 0;
}

/*
 *	compute the arc segment joining a, b, and c
 *
 *	this is actually a neat derivation. there is a file named
 *	"path.ps" in this directory that is drawing that shows the
 *	meaning of the variables and steps below. the drawing is
 *	a little primitve, but is much better than nothing at all.
 */

static int
computeFillet (pfVec3 a, pfVec3 b, pfVec3 c, float radius,
	       pfVec3 h, pfVec3 i, pfVec3 g, pfVec2 angles)
{
    float lba;
    float lbc;
    float norm;
    pfVec3 bd;
    pfVec3 be;
    pfVec3 d;
    pfVec3 e;
    pfVec3 f;
    pfVec3 bf;
    float angle;
    float hyp;
    float leg;
    pfVec3 gh;
    pfVec3 gi;
    float xgh;
    float xgi;
    
    /* bd: unit length vector "from" point b toward point a */
    /* lba: length of vector ba */
    PFSUB_VEC3(bd, a, b);
    lba = PFLENGTH_VEC3(bd);
    norm = 1.0f/lba;
    PFSCALE_VEC3(bd, norm, bd);
    
    /* be: unit length vector "from" point b toward point c */
    /* lbc: length of vector bc */
    PFSUB_VEC3(be, c, b);
    lbc = PFLENGTH_VEC3(be);
    norm = 1.0f/lbc;
    PFSCALE_VEC3(be, norm, be);
    
    /* d: the point one unit from point b toward point a */
    PFADD_VEC3(d, b, bd);
    
    /* e: the point one unit from point b toward point c */
    PFADD_VEC3(e, b, be);
    
    /* f: midpoint of segment DE, it's on the bisector of angle ABC */
    PFCOMBINE_VEC3(f, 0.5f, d, 0.5f, e);
    
    /* bf: unit length vector "from" point b toward point f */
    PFSUB_VEC3(bf, f, b);
    norm = 1.0f/PFLENGTH_VEC3(bf);
    PFSCALE_VEC3(bf, norm, bf);
    
    /* angle ABF and FBC -- one half of angle ABC */
    angle = 0.5f*acosf(PFDOT_VEC3(bd, be));
    
    /* 
     * the sides of a right triangle whose base (LEG) is along BA and 
     * whose height is RADIUS.
     */
    hyp = radius/sinf(angle);
    leg = sqrtf(hyp*hyp - radius*radius);
    
    /* 
     * is the triangle's base too long to fit within AB and BC ? if so
     * then the arc's radius is too big 
     */
    if (leg > lba || leg > lbc)
	return -1;
    
    /* g: the center of the arc */
    PFCOMBINE_VEC3(g, 1.0f, b, hyp, bf);
    
    /* h: the point on AB tangent to the arc */
    PFCOMBINE_VEC3(h, 1.0f, b, leg, bd);
    
    /* i: the point on BC tangent to the arc */
    PFCOMBINE_VEC3(i, 1.0f, b, leg, be);
    
    /* gh: unit length vector "from" point g toward point h */
    PFSUB_VEC3(gh, h, g);
    norm = 1.0f/PFLENGTH_VEC3(gh);
    PFSCALE_VEC3(gh, norm, gh);
    
    /* gi: unit length vector "from" point g toward point i */
    PFSUB_VEC3(gi, i, g);
    norm = 1.0f/PFLENGTH_VEC3(gi);
    PFSCALE_VEC3(gi, norm, gi);
    
    /* the angle CCW from +X in the XY plane to GH */
    xgh = PF_RAD2DEG(atan2f(h[PF_Y] - g[PF_Y], h[PF_X] - g[PF_X]));
    if (xgh < 0.0f)
	xgh += 360.0f;
    
    /* the angle CCW from +X in the XY plane to GI */
    xgi = PF_RAD2DEG(atan2f(i[PF_Y] - g[PF_Y], i[PF_X] - g[PF_X]));
    if (xgi < 0.0f)
	xgi += 360.0f;
    
    /* set the starting angle */
    angles[0] = xgh;
    
    /* compute the arc's magnitude */
    angles[1] = xgi - xgh;
    
    /* compute the arc's sign */
    if (angles[1] >  180.0f)
	angles[1] -= 360.0f;
    else
	if (angles[1] < -180.0f)
	    angles[1] += 360.0f;
    
#ifdef	VERBOSE
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "computeFillet");
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "  input:");
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    a: %g %g %g", a[PF_X], a[PF_Y], a[PF_Z]);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    b: %g %g %g", b[PF_X], b[PF_Y], b[PF_Z]);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    c: %g %g %g", c[PF_X], c[PF_Y], c[PF_Z]);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "output:");
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    a: %g %g %g", a[PF_X], a[PF_Y], a[PF_Z]);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    h: %g %g %g", h[PF_X], h[PF_Y], h[PF_Z]);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    g: %g %g %g", g[PF_X], g[PF_Y], g[PF_Z]);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    radius = %g", radius);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    start  = %g", angles[0]);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    turn   = %g", angles[1]);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    i: %g %g %g", i[PF_X], i[PF_Y], i[PF_Z]);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "    c: %g %g %g", c[PF_X], c[PF_Y], c[PF_Z]);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, NULL);
#endif

    return 0;
}
