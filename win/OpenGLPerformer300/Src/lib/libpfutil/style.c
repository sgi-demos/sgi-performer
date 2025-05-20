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

#include <Performer/pf.h>
#include <Performer/pfutil.h>

#define	STYLE_POINT_SIZE	2
#define	STYLE_LINE_WIDTH	1

/***
 ***	Utility to produce fancy drawing styles (such has wire-frame
 ***	with hidden lines removed) in IRIS Performer applications.
 ***/

/* Force scene to be rendered with flat-white material */
static void
forceColorOn(pfVec4 rgba)
{
    pfVec4 newColor;
    static pfMaterial	*forceMtl = NULL;
    static pfLightModel	*forceLM = NULL;
    static pfVec4 forceRgba;

    if (rgba == NULL)
	pfSetVec4(newColor, 0.0f, 0.0f, 0.0f, 1.0f);
    else
	pfCopyVec4(newColor, rgba);

    if (forceMtl == NULL)
    {
	forceMtl = pfNewMtl(pfGetSharedArena());
	forceLM = pfNewLModel(pfGetSharedArena());

	pfCopyVec4(forceRgba, newColor);
	pfMtlColor(forceMtl, PFMTL_AMBIENT, 
	    forceRgba[0], forceRgba[1], forceRgba[2]);
	pfMtlAlpha(forceMtl, forceRgba[3]);

	pfMtlColor(forceMtl, PFMTL_DIFFUSE,  0.0f, 0.0f, 0.0f);
	pfMtlColor(forceMtl, PFMTL_EMISSION, 0.0f, 0.0f, 0.0f);
	pfMtlColor(forceMtl, PFMTL_SPECULAR, 0.0f, 0.0f, 0.0f);
	pfMtlSide(forceMtl, PFMTL_BOTH);
	pfMtlColorMode(forceMtl, PFMTL_BOTH, PFMTL_CMODE_NULL);

	pfLModelAmbient(forceLM, 1.0f, 1.0f, 1.0f);
    }
    else
    if (!pfAlmostEqualVec4(newColor, forceRgba, 0.0001))
    {
	pfCopyVec4(forceRgba, newColor);
	pfMtlColor(forceMtl, PFMTL_AMBIENT, 
	    forceRgba[0], forceRgba[1], forceRgba[2]);
	pfMtlAlpha(forceMtl, forceRgba[3]);
    }

    pfOverride(
	PFSTATE_ENLIGHTING | 
	PFSTATE_ENTEXTURE | 
	PFSTATE_LIGHTMODEL | 
	PFSTATE_FRONTMTL | 
	PFSTATE_BACKMTL, 0);

    pfApplyMtl(forceMtl);
    pfApplyLModel(forceLM);

    pfEnable(PFEN_LIGHTING);
    pfDisable(PFEN_TEXTURE);

    pfOverride(
	PFSTATE_ENLIGHTING | 
	PFSTATE_ENTEXTURE | 
	PFSTATE_LIGHTMODEL | 
	PFSTATE_FRONTMTL | 
	PFSTATE_BACKMTL, 1);
}

static void
forceHighlightingOff(void)
{
    pfOverride(PFSTATE_ENHIGHLIGHTING, 0);
    pfDisable(PFEN_HIGHLIGHTING);
    pfOverride(PFSTATE_ENHIGHLIGHTING, 1);
}

static void
forceWireframeOn(void)
{
    pfOverride(PFSTATE_ENWIREFRAME, 0);
    pfEnable(PFEN_WIREFRAME);
    if (pfGetAntialias() == PFAA_OFF)
	pfAntialias(PFAA_SMOOTH);
    pfOverride(PFSTATE_ENWIREFRAME, 1);
}

/*
 *	pfuPreDrawStyle -- call just before pfDraw to obtain one of 
 *	    several special draw styles. Also call pfuPostDrawStyle 
 *	    after calling pfDraw if you use this function.
 */
PFUDLLEXPORT void
pfuPreDrawStyle (int style, pfVec4 scribeColor)
{
    /* this is the default and must be fast */
    if (style == PFUSTYLE_FILLED)
	return;

    pfPushState();

    /* handle draw style */
    switch (style)
    {
    case PFUSTYLE_POINTS:
	pfCullFace(PFCF_OFF);
	pfOverride(PFSTATE_CULLFACE, 1);
        if (pfGetAntialias() == PFAA_OFF)
	    pfAntialias(PFAA_SMOOTH);
	glPointSize((float)(STYLE_POINT_SIZE));
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	break;

    case PFUSTYLE_LINES:
	glLineWidth((float)(STYLE_LINE_WIDTH));
	forceWireframeOn();
	break;

    case PFUSTYLE_DASHED:
	pfCullFace(PFCF_FRONT);
	pfOverride(PFSTATE_CULLFACE, 1);
        if (pfGetAntialias() == PFAA_OFF)
	    pfAntialias(PFAA_SMOOTH);
	glLineWidth((float)(STYLE_LINE_WIDTH));
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineStipple(1, 0x8888);
	glEnable(GL_LINE_STIPPLE);
	pfDraw();
	pfOverride(PFSTATE_CULLFACE, 0);
	pfCullFace(PFCF_BACK);
	pfOverride(PFSTATE_CULLFACE, 1);
	glDisable(GL_LINE_STIPPLE);
	break;

    case PFUSTYLE_HALOED:
	forceWireframeOn();
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glLineWidth((float)(3*STYLE_LINE_WIDTH));
	pfDecal(PFDECAL_LAYER_DISPLACE_AWAY);
	pfOverride(PFSTATE_DECAL, 1);
	pfDraw();
	pfOverride(PFSTATE_DECAL, 0);
	pfDecal(PFDECAL_OFF);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glLineWidth((float)(STYLE_LINE_WIDTH));
	break;

    case PFUSTYLE_HIDDEN:
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	pfDecal(PFDECAL_LAYER_DISPLACE_AWAY);
	pfOverride(PFSTATE_DECAL, 1);
	pfDraw();
	pfOverride(PFSTATE_DECAL, 0);
	pfDecal(PFDECAL_OFF);
	forceWireframeOn();
	glLineWidth((float)(STYLE_LINE_WIDTH));
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	break;

    case PFUSTYLE_SCRIBED:
	pfDecal(PFDECAL_LAYER_DISPLACE_AWAY);
	pfOverride(PFSTATE_DECAL, 1);
	pfDraw();
	pfOverride(PFSTATE_DECAL, 0);
	pfDecal(PFDECAL_OFF);
	forceColorOn(scribeColor);
	forceWireframeOn();
	forceHighlightingOff();
	glLineWidth((float)(STYLE_LINE_WIDTH));
	break;

    default:
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	    "pfuPreDrawStyle: unknown draw style");
    }
}

/*
 *	pfuPostDrawStyle -- call just after pfDraw to obtain one of 
 *	    several special draw styles. You must have also called 
 *	    pfuPreDrawStyle before calling pfDraw if you use this 
 *	    function.
 */
PFUDLLEXPORT void
pfuPostDrawStyle (int style)
{
    /* this is the default and must be fast */
    if (style == PFUSTYLE_FILLED)
	return;

    /* handle draw style */
    switch (style)
    {
    case PFUSTYLE_LINES:
    case PFUSTYLE_HALOED:
    case PFUSTYLE_HIDDEN:
    case PFUSTYLE_SCRIBED:
	/* should restore line width here */
	break;

    case PFUSTYLE_POINTS:
    case PFUSTYLE_DASHED:
	pfOverride(PFSTATE_CULLFACE, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	break;

    default:
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	    "pfuPostDrawStyle: unknown draw style");
    }

    pfPopState();
}
