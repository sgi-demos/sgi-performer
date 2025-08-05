/*
 * Copyright 1995, Silicon Graphics, Inc.
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
 *
 * file: xfont.c
 * -------------
 * 
 * $Revision: 1.1 $ $Date: 2000/11/21 21:39:36 $ 
 */

#include <stdio.h>
#include <stdlib.h>
#ifndef __linux__
#include <bstring.h>
#endif  /* __linux__ */
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>

/******************************************************************************
 *			    Global Variables
 ******************************************************************************
 */

/******************************************************************************
 *			    Exposed Routines
 ******************************************************************************
 */

pfuXFont	pfuCurXFont;
static char dftFontName[] ="-*-courier-bold-r-normal--12-*-*-*-m-70-iso8859-1";

void
pfuLoadXFont(char *fontName, pfuXFont *fnt)
{
    Display *xdisplay;

    xdisplay = pfGetCurWSConnection();
    if (!fontName)
	fontName = dftFontName;
    if (fnt)
    {
	fnt->info = XLoadQueryFont(xdisplay,fontName);
	if (fnt->info == NULL) {
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
		"pfuLoadXFont - couldn't find font %s at display %s.\n", 
			fontName, DisplayString(xdisplay));
	}
    }
    else
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfuLoadXFont - null pfuXFont*");
}

void
pfuMakeXFontBitmaps(pfuXFont *fnt)
{
    Font id;
    unsigned int first, last;
    
    if (fnt && fnt->info)
    {
	id = fnt->info->fid;
	first = 32 /* fnt->info->min_char_or_byte2 */;
	last = 128 /* fnt->info->max_char_or_byte2 */;
	if (!fnt->handle)
	{
	    fnt->handle = glGenLists((GLuint) last+1);
	    if (fnt->handle == 0) 
	    {
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
		    "pfuMakeXFontBitmaps - couldn't get %d lists.\n", 
		    last+1);
		return;
	    }
	}
	glXUseXFont(id, first, last-first+1, fnt->handle+first);
    /*    *height = fnt->info->ascent + fnt->info->descent;
	*width = fnt->info->max_bounds.width;  */
    
    }
    else
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfuMakeXFontBitmaps - null pfuXFont*");
}

void
pfuMakeRasterXFont(char *fontName, pfuXFont *fnt)
{
    if (fnt)
    {
	pfuLoadXFont(fontName, fnt);
	pfuCurXFont.info = fnt->info;
	pfuMakeXFontBitmaps(fnt);
	pfuCurXFont.handle = fnt->handle;
    }
    else
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfuMakeRasterXFont - null pfuXFont*");
}

int 
pfuGetXFontWidth(pfuXFont *f, const char *str)
{
    if (f && f->info)
    {
	int len = (int)strlen(str);
	/*  int w = (len * (f->info->max_bounds.rbearing - f->info->min_bounds.lbearing));  */
	int w = XTextWidth(f->info, str, len);
	return w;
    }
    else
	return 0;
}

int 
pfuGetXFontHeight(pfuXFont *f)
{
    if (f && f->info)
    {
	int h =  f->info->ascent + f->info->descent;
	return h;
    }
    else
	return 0;
}

void 
pfuGetCurXFont(pfuXFont *f)
{
    *f =  pfuCurXFont;
}

void 
pfuSetXFont(pfuXFont *f)
{
    if (f && f->info)
    {
	pfuCurXFont = *f;
	return;
    } else
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfuSetXFont - pfuXFont has null info pointer.");
}

void 
pfuCharPos(float x, float y, float z)
{
    glRasterPos3f(x,y,z);
}

void 
pfuDrawString(const char *s)
{
    if (!pfuCurXFont.handle)
    {
pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"pfuDrawString - bad font handle in pid %d", getpid());
	return;
    }
    glPushAttrib (GL_LIST_BIT);
    glListBase(pfuCurXFont.handle);
    glCallLists((int)strlen(s), GL_UNSIGNED_BYTE, (GLubyte *)s);
    glPopAttrib ();
}

void 
pfuDrawStringPos(const char *s,float x, float y, float z)
{
    if (!pfuCurXFont.handle)
    {
pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"pfuDrawStringPos - bad font handle in pid %d", getpid());
	return;
    }
    glRasterPos3f(x,y,z);
    glPushAttrib (GL_LIST_BIT);
    glListBase(pfuCurXFont.handle);
    glCallLists((int)strlen(s), GL_UNSIGNED_BYTE, (GLubyte *)s);
    glPopAttrib ();
}
