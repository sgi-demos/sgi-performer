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
 * gui.h
 *
 * $Revision: 1.1 $
 * $Date: 2000/11/21 21:39:43 $
 *
 */

#ifndef __GUI_H__
#define __GUI_H__

#if 1 /* CUSTOM */
#include "fakeclipguistate.h"
extern void setGuiExageration(int exag);
#endif /* CUSTOM */

enum GUIID 
{
#if 1 /* CUSTOM */
    GUI_FAKECLIP_MIN_LOD,
    GUI_FAKECLIP_INVALID_BORDER,
    GUI_FAKECLIP_IMAGE,
    GUI_FAKECLIP_VIRTUAL_SIZE,
    GUI_FAKECLIP_CLIP_SIZE,
    GUI_FAKECLIP_CLIP_CENTERING,
    GUI_FAKECLIP_CLIP_VISUALIZATION,
    GUI_FAKECLIP_MINFILTER,
    GUI_FAKECLIP_MAGFILTER,
#endif /* CUSTOM */
#if 0 /* PERFLY */
    GUI_AA,
#endif /* PERFLY */
    GUI_COLLIDE,
    GUI_CULL,
    GUI_ESKY,
    GUI_FAR,
    GUI_FILES,
    GUI_FOG,
#if 0 /* PERFLY */
    GUI_NEAR_FOG_RANGE,
    GUI_FAR_FOG_RANGE,
#endif /* PERFLY */
    GUI_FOV,
    GUI_FOV_CULL,
    GUI_GUI,
    GUI_HIGH_STYLE,
    GUI_LIGHTING,
#if 0 /* PERFLY */
    GUI_LOD_FADE,
    GUI_LOD_FADE_RANGE,
    GUI_LOD_SCALE,
#endif /* PERFLY */
    GUI_MSG,
    GUI_NEAR,
    GUI_PHASE,
#if 0 /* PERFLY */
    GUI_PIX_LIMIT,
#endif /* PERFLY */
    GUI_QUIT,
    GUI_REPOSITION,
#ifdef _PF_MULTI_VIEWSTATE
    GUI_CONFIG,
#endif
    GUI_RESET_ALL,
    GUI_STATS,
    GUI_STRESS,
#if 0 /* PERFLY */
    GUI_STRESS_HIGH,
    GUI_STRESS_LOW,
    GUI_STRESS_MAX,
    GUI_STRESS_SCALE,
#endif /* PERFLY */
    GUI_TEXTURE,
#if 0 /* PERFLY */
    GUI_TOD,
#endif /* PERFLY */
    GUI_DRAW_STYLE,
    GUI_TETHERVIEW, 
    GUI_VEHICLE, 
    GUI_XFORMER, 
    GUI_CENTER, 
    GUI_SNAP,
    GUI_TREE,
    GUI_PANX,
    GUI_PANY,
    GUI_SCALE,
    GUI_MIP_MIN_LOD,
    GUI_MIP_MAX_LOD,
    GUI_MIP_MAGFILTER,
    GUI_MIP_MINFILTER,
    GUI_MIP_LOD_BIAS,
    GUI_MIP_LOD_BIAS_S,
    GUI_MIP_LOD_BIAS_T,
    GUI_CLIPTEX_SELECT, 
    GUI_CLIP_DTR_TIME,
    GUI_CLIP_DTRMODE,
    GUI_CLIP_LOD_OFFSET,
    GUI_CLIP_EFFECTIVE_LEVELS,
    GUI_CLIP_INVALID_BORDER,
    GUI_CLIP_CENTERING,
    GUI_CLIP_VISUALIZATION,
    GUI_DEMO,
    GUI_PATH,
    GUI_CALLIGRAPHIC_DEFOCUS,
    GUI_RASTER_DEFOCUS,
    GUI_FILTER_SIZEX,
    GUI_FILTER_SIZEY,
    GUI_ZFOOT_PRINT
};

typedef enum GUIID GUIId;

extern void 	initGUI(pfChannel *);
extern void	updateGUI(int);
extern void	updateGUIViewMsg(void);
extern void	updateWidget(int, float);

#endif
