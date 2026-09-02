/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
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
 * env.c	environment functions, ie. weather, time of day, etc
 *
 * $Revision: 1.7 $
 * $Date: 2000/10/06 21:26:03 $
 *
 */

#include <math.h>
/*
#include <Performer/pf.h>
#include <Performer/pfutil.h>
*/
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfEarthSky.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pr/pfFog.h>

#include "custom.h"

static pfFog	 *eskyFog;
static pfVec3	fogColor;

#define	TOTAL_HORIZ_ANGLE	7.0f

	/*-----------------------------------------------------*/

void
initEnvironment(void)
{
    pfEarthSky 	*es;

    /* Initialize sun */
    ViewState->sun = new pfLightSource();
    ViewState->sunDCS = new pfDCS();

    /* Point in direction of view for easier LIGHTING_EYE computation */
    ViewState->sun->setPos(0.0f, -1.0f, 0.0f, 0.0f);

    ViewState->scene->addChild(ViewState->sunDCS);
    ViewState->sunDCS->addChild(ViewState->sun);

    es = new pfEarthSky();
    es->setMode(PFES_BUFFER_CLEAR, ViewState->earthSkyMode); 
    es->setAttr(PFES_GRND_HT, -0.5f * ViewState->sceneSize);
    es->setColor(PFES_CLEAR, 
		 ViewState->earthSkyColor[0],
		 ViewState->earthSkyColor[1],
		 ViewState->earthSkyColor[2],
		 ViewState->earthSkyColor[3]);

    es->setAttr(PFES_HORIZ_ANGLE, TOTAL_HORIZ_ANGLE);

    /* setup the fog for general visibility */
    eskyFog = new pfFog();
    fogColor.set(.6f, .6f, .6f); 
    if(ViewState->fog != PFFOG_OFF)
    {
	eskyFog->setFogType(ViewState->fog);
	eskyFog->setColor(fogColor[0] * ViewState->timeOfDay, 
		      fogColor[1] * ViewState->timeOfDay, 
		      fogColor[2] * ViewState->timeOfDay); 
	eskyFog->setRange(ViewState->nearFogRange, ViewState->farFogRange);
    }
    ViewState->eSky = es;
}

void
updateEnvironment(void)
{
    pfEarthSky 		*es = ViewState->eSky;
    static int 	 	earthSkyMode = -1;
    static int 	 	fog = -1;
    static float 	nearFogRange = 0.0f, farFogRange = 0.0f;
    
    if ((earthSkyMode == ViewState->earthSkyMode) && 
	(fog == ViewState->fog) && 
	(nearFogRange == ViewState->nearFogRange) &&
	(farFogRange == ViewState->farFogRange))
	return;
 
    es->setMode(PFES_BUFFER_CLEAR, ViewState->earthSkyMode); 

    if (ViewState->fog != PFFOG_OFF)
    {
	eskyFog->setFogType(ViewState->fog);
	eskyFog->setRange(ViewState->nearFogRange, ViewState->farFogRange);
	es->setFog(PFES_GENERAL, eskyFog);
	eskyFog->setColor(fogColor[0] * ViewState->timeOfDay, 
			  fogColor[1] * ViewState->timeOfDay, 
			  fogColor[2] * ViewState->timeOfDay); 
	es->setColor(PFES_CLEAR, 
		     fogColor[0] * ViewState->timeOfDay, 
		     fogColor[1] * ViewState->timeOfDay, 
		     fogColor[2] * ViewState->timeOfDay,
		     ViewState->earthSkyColor[3]);
    }
    else
    {
	es->setFog(PFES_GENERAL, NULL);
	es->setColor(PFES_CLEAR, 
		     ViewState->earthSkyColor[0],
		     ViewState->earthSkyColor[1],
		     ViewState->earthSkyColor[2],
		     ViewState->earthSkyColor[3]);
    }

    /* update static values */
    earthSkyMode = ViewState->earthSkyMode;
    fog = ViewState->fog;
    nearFogRange = ViewState->nearFogRange;
    farFogRange = ViewState->farFogRange;
}


void
updateTimeOfDay(float newTod)
{
    float 		r, g, b;
    float 		s;
    pfVec3 		t;
    pfEarthSky		*es;
    static int		lighting = -1;
    static float  	tod = -1.0f;
    static pfVec3	noon(-.05f, -.1f, 1.0f);
    static pfVec3	night(-.15f, 1.0f, .2f);
    static pfVec3	prevViewDir(-1.0f, -1.0f, -1.0f);

    /* only do if necessary */
    if (ViewState->lighting == lighting && newTod == tod && 
	(ViewState->lighting == LIGHTING_SUN || 
	 PFEQUAL_VEC3(ViewState->viewMat[1], prevViewDir)))
        return;

    tod = newTod;
    lighting = ViewState->lighting;
    prevViewDir = *(pfVec3*)ViewState->viewMat.mat[1];

    es = ViewState->eSky;

    /* make horizon band angle directly proportional to time of day */
    es->setAttr(PFES_HORIZ_ANGLE, 1.0f + (TOTAL_HORIZ_ANGLE - 1.0f)*tod);

    /* scale it for the following ramp calcs */
    s = tod * 4.0f;	
    
    /* top of sky */
    r = (0.4f * s) - 1.6f;   r = PF_MAX2(0.0f, r);  r = PF_MIN2(0.7f, r);
    g = (0.4f * s) - 1.6f;   g = PF_MAX2(0.0f, g);  g = PF_MIN2(0.7f, g); 
    b = (0.8f * s) - 1.6f;   b = PF_MAX2(0.0f, b);  b = PF_MIN2(1.0f, b);
    es->setColor(PFES_SKY_TOP, r,g,b, 1.0f);
    
    /* bottom of sky */
    r = (0.6f * s) - 1.6f;   r = PF_MAX2(0.0f, r);  r = PF_MIN2(0.7f, r);
    g = (0.6f * s) - 1.6f;   g = PF_MAX2(0.0f, g);  g = PF_MIN2(0.7f, g); 
    b = (1.0f * s) - 1.6f;   b = PF_MAX2(0.0f, b);  b = PF_MIN2(1.0f, b);
    es->setColor(PFES_SKY_BOT, r,g,b, 1.0f);
	
    /* horizon */
    r = (1.45f * s) - 0.4f;   r = PF_MAX2(0.0f, r);  r = PF_MIN2(0.7f, r);
    g = (0.65f * s) - 1.4f;   g = PF_MAX2(0.0f, g);  g = PF_MIN2(0.7f, g); 
    b = (0.65f * s) - 1.6f;   b = PF_MAX2(0.0f, b);  b = PF_MIN2(1.0f, b);
    es->setColor(PFES_HORIZ, r,g,b, 1.0f);
    
    /* ground */
    r = (0.0181f * (s - 0.8f));   r = PF_MAX2(0.0f, r);  r = PF_MIN2(0.058f, r);
    g = (0.0840f * (s - 0.8f));   g = PF_MAX2(0.0f, g);  g = PF_MIN2(0.270f, g); 
    b = (0.0181f * (s - 0.8f));   b = PF_MAX2(0.0f, b);  b = PF_MIN2(0.058f, b); 
    es->setColor(PFES_GRND_NEAR, r ,g, b, 1.0f);
    es->setColor(PFES_GRND_FAR,  r ,g, b, 1.0f);

    /* Scale fog color based on time of day */
    if (ViewState->fog != PFFOG_OFF)
    {
	eskyFog->setColor(fogColor[0] * ViewState->timeOfDay, 
			  fogColor[1] * ViewState->timeOfDay, 
			  fogColor[2] * ViewState->timeOfDay); 
	es->setColor(PFES_CLEAR, 
		     fogColor[0] * ViewState->timeOfDay, 
		     fogColor[1] * ViewState->timeOfDay, 
		     fogColor[2] * ViewState->timeOfDay,
		     ViewState->earthSkyColor[3]);
    }
    
    /* Here we set the light position and color */
    t.combine(tod, noon, 1.0f - tod, night);
    t.normalize();
    if (ViewState->lighting == LIGHTING_SUN)
    {
	pfMatrix	mat;
	static pfVec3	nyaxis(0.0f, -1.0f, 0.0f);

	mat.makeVecRotVec(nyaxis, t);
    	ViewState->sunDCS->setMat(mat);
        ViewState->sun->setColor(PFLT_AMBIENT,  0.8f*tod, 0.8f*tod, 0.8f*tod); 
    }
    else /* LIGHTING_EYE */	
    {
        ViewState->sun->setColor(PFLT_AMBIENT,  0.0f, 0.0f, 0.0f); 
    	ViewState->sunDCS->setMat(ViewState->viewMat);
    }

    ViewState->sun->setColor(PFLT_DIFFUSE,  0.8f*tod, 0.8f*tod, 0.8f*tod); 
    ViewState->sun->setColor(PFLT_SPECULAR, 0.8f*tod, 0.8f*tod, 0.8f*tod); 
}
