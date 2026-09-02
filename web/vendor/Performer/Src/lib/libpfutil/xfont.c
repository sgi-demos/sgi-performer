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
 * $Revision: 1.13 $ $Date: 2002/11/05 03:21:24 $ 
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef mips
#include <bstring.h>
#endif 
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include <wingdi.h>
#else
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

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
#ifdef WIN32
static LOGFONT defaultFont = {
  -12,
  0,
  0,
  0,
  FW_BOLD,
  FALSE,
  FALSE,
  FALSE,
  ANSI_CHARSET,
  OUT_TT_PRECIS, 
  CLIP_DEFAULT_PRECIS,
  ANTIALIASED_QUALITY,
  FF_DONTCARE|DEFAULT_PITCH,
  "Courier New"
};
static char dftFontName[] ="-*-Courier New-bold-r-normal--12-*-*-*-m-70-iso8859-1";
#else
static char dftFontName[] ="-*-courier-bold-r-normal--12-*-*-*-m-70-iso8859-1";
#endif

#ifdef WIN32
/* On windows, we need a function that parses an X Logical Font Description
   and produces a LOGFONT structure.
   
   An X LFD has the following fields:
   1 - FOUNDRY (will ignore)
   2 - FAMILY_NAME  (name of the font, eg courier)
   3 - WEIGHT_NAME  (eg, bold)
   4 - SLANT        R = roman(upright)
                    I = italic(clockwise slant)
                    O = oblique(clockwise slant)
                    RI = reverse italic
                    RO = reverse oblique
                    OT = other
                    <number> = polymorphic font code
   5 - SETWIDTH_NAME (narrow, condensed, normal, double wide, etc)
   6 - ADD_STYLE_NAME (serif, sans serif, informal, decorated, "[")
   7 - PIXEL_SIZE (size of font at a particular POINT_SIZE and RESOLUTION_Y, 0= scalable)
   8 - POINT_SIZE (body size font was designed for)
   9 - RESOLUTION_X
  10 - RESOLUTION_Y (dpi for font, 0 = scalable)
  11 - SPACING      P = proportional
                    M = monospaced
                    C = CharCell (typewriter model)
  12 - AVERAGE_WIDTH (tenths of pixels, negative if right-to-left, ~ denotes negative,
                      0 = scalable)
  13 - CHARSET_REGISTRY   (eg, iso8859) (ignore)
  14 - CHARSET_ENCODING   (eg, 1)       (ignore)

  return types:
  0 = failure
  1 = success (partial)


  For now, this function does a very basic interpretation. It is assumed that
  the PIXEL_SIZE field is used to choose a font of a particular pixel size, meaning
  that POINT_SIZE and RESOLUTION_* are ignored. 
  
  So, the X LFD will tell us the pixel size of a font. 

  Now, on windows, things get a little more interesting.  Point sizes
  are specified relative to a logical inch. There are 72 points in a
  logical inch. A logical inch can be any number of pixels, dependent on
  display. 

  Example: 
    12 point font on 96 dpi display: 
    12 / 72 * 96 = 16 pixel font (Normal fonts)

    12 point font on a 120 dpi display:
    12 / 72 * 120 = 20 pixel font (Large fonts in control panel)

    So, to get actual pixel size, we must do the following

    FontSize = 72 * (desired size) / (dots per logical inch)
*/                  

/* This function is like strtok except that it returns an empty string if there is an 
   instance of a double delimiter, like "--" in a font description */

char *customStrtok(char *string, const char delim) {
  static int index = 0;
  static char *lastString;
  char *retVal;
  
  if(string) {
    index = 0;
    lastString = string;
    
    /* strip off any leading tokens */
    while(lastString[index] == delim)
      index++;
  }
  
  /* at this point, laststring[index] points to first non-token char */
  retVal = &(lastString[index]);
  
  /* now, search through the string looking for a delimiter */
  while((lastString[index] != 0) && (lastString[index] != delim))
    index++;
  
  /* check to see if we ran off end, special case if 0 is delim */
  if(lastString[index] == 0)
    if(delim == 0)
      return retVal;
    else
      return 0;
  
  /* ok, index now points to delimiter, null it out */
  lastString[index] = 0;
  index++;
  
  /* return the null-terminated token */
  return retVal;
};

int pfuXLFDtoLOGFONT(const char *fontDesc, LOGFONT *destination) {
  char buffer[256]; /* max length of description */
  char *tmpName;
  char delimiter = '-';
  char *token;
  
  /* check for null font description */
  if(!fontDesc) return 0;
  
  /* initialize the logfont to reasonable values */
  destination->lfHeight = 0;
  destination->lfWidth = 0;
  destination->lfEscapement = 0;
  destination->lfOrientation = 0;
  destination->lfWeight = FW_DONTCARE;
  destination->lfItalic = FALSE;
  destination->lfUnderline = FALSE;
  destination->lfStrikeOut = FALSE;
  destination->lfCharSet = ANSI_CHARSET;
  destination->lfOutPrecision = OUT_TT_PRECIS;
  destination->lfClipPrecision = CLIP_DEFAULT_PRECIS;
  destination->lfQuality = PROOF_QUALITY;
  destination->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
  /*destination->lpFaceName will always be set below */

  tmpName = strdup(fontDesc);
  
  /* initialize customStrtok */
  token = customStrtok(tmpName, delimiter); /* waste first token, don't care about it */
  
  
  /* get the font name */
  token = customStrtok(NULL, delimiter);
  /*fprintf(stderr, "fontName is %s\n", token);*/
  /* copy face name into LOGFONT */
  strcpy(destination->lfFaceName, token);
  
  /* get the weight name */
  token = customStrtok(NULL, delimiter);
  /*fprintf(stderr, "weightName = %s\n", token);*/
  /* black, bold, book, demi, light, medium, regular */
  if(_stricmp(token, "black") == 0)
    destination->lfWeight = FW_BLACK;
  else if(_stricmp(token, "bold") == 0)
    destination->lfWeight = FW_BOLD;
  else if(_stricmp(token, "book") == 0)
    destination->lfWeight = FW_EXTRALIGHT;
  else if(_stricmp(token, "demi") == 0)
    destination->lfWeight = FW_DEMIBOLD;
  else if(_stricmp(token, "light") == 0)
    destination->lfWeight = FW_LIGHT;
  else if(_stricmp(token, "medium") == 0)
    destination->lfWeight = FW_MEDIUM;
  else if(_stricmp(token, "regular") == 0)
    destination->lfWeight = FW_REGULAR;
  else /* default to regular */
    destination->lfWeight = FW_REGULAR;
  
  /* get the slant name */
  token = customStrtok(NULL, delimiter);
  /*fprintf(stderr, "slantName = %s\n", token);*/
  if(_stricmp(token, "r") == 0)
    destination->lfItalic = FALSE;
  else if(_stricmp(token, "i") == 0)
    destination->lfItalic = TRUE;
  else if(_stricmp(token, "o") == 0)
    destination->lfItalic = TRUE;
  else /*default to regular */
    destination->lfItalic = FALSE;
  
  /* get the width name */
  token = customStrtok(NULL, delimiter);
  /*fprintf(stderr, "widthName = %s\n", token);*/
  /* ignored for now */
  destination->lfWidth = 0; /* use default */
  
  /* get the style name */
  token = customStrtok(NULL, delimiter);
  /* ignored for now */
  /*fprintf(stderr, "styleName = %s\n", token);*/

  /* get the pixel size */
  token = customStrtok(NULL, delimiter);
  /*fprintf(stderr, "pixelSize = %s\n", token);*/
  if(token[0] != '*')
    {
      int desiredPixelSize;
      int querySize;
      desiredPixelSize = atoi(token);
      
      querySize = (desiredPixelSize * 72) / GetDeviceCaps(GetDC(GetDesktopWindow()),
							  LOGPIXELSY);
      destination->lfHeight = -desiredPixelSize;
    }
  else {
    destination->lfHeight = 0;
  }
  
  /* get the point size */
  token = customStrtok(NULL, delimiter);
  /*fprintf(stderr, "pointSize = %s\n", token);*/
  /* ignored for now */
  
  /* get the resolution*/
  token = customStrtok(NULL, delimiter);
  /*fprintf(stderr, "res X = %s\n", token);*/
  token = customStrtok(NULL, delimiter);
  /*fprintf(stderr, "res Y = %s\n", token);*/
  /* resolution ignored for now */

  /* spacing */
  token = customStrtok(NULL, delimiter);
  /*fprintf(stderr, "spacing = %s\n", token);*/
  destination->lfPitchAndFamily = 0;
  if(_stricmp(token, "p") == 0)
    destination->lfPitchAndFamily |= VARIABLE_PITCH;
  else if(_stricmp(token, "m") == 0)
    destination->lfPitchAndFamily |= FIXED_PITCH;
  else if(_stricmp(token, "c") == 0)
    destination->lfPitchAndFamily |= FIXED_PITCH;
  else
    destination->lfPitchAndFamily |= DEFAULT_PITCH;
  destination->lfPitchAndFamily |= FF_DONTCARE;
  
  /* avg width */
  token = customStrtok(NULL, delimiter);
  /*fprintf(stderr, "avg width = %s\n", token);*/
  if(token[0] != '*')
    {
      int desiredPixelWidth;
      int querySize;
      desiredPixelWidth = atoi(token);
      
      querySize = desiredPixelWidth * 72 / GetDeviceCaps(GetDC(GetDesktopWindow()),
							 LOGPIXELSY);
      querySize /= 10; /* expressed in tenths in X */
      destination->lfWidth = desiredPixelWidth;
    }
  else {
    destination->lfWidth = 0;
  }

  
  /* registry and encoding, ignore */
  token = customStrtok(NULL, delimiter);
  /*fprintf(stderr, "registry = %s\n", token);*/
  token = customStrtok(NULL, 0);
  /*fprintf(stderr, "encoding = %s\n", token);*/
  free(tmpName);
  
  return 1;
}
#endif


#ifdef WIN32
PFUDLLEXPORT void pfuLoadWINFont(LOGFONT *lf, pfuXFont *fnt) {
  HFONT font;
  LOGFONT logicalFont;
  
  if(!lf)
    lf = &defaultFont;
  
  font = CreateFontIndirect(lf);
  
  if(!font) {
    pfNotify(PFNFY_WARN,PFNFY_USAGE,"pfuLoadXFont - null pfuXFont");
    fnt->info = NULL;
  } 
  else
    fnt->info = font;
}
#endif

/* 
 * XXX Alex -- provide some way of specifying height for fonts on win32
 * with X this is easy because it's a part of the font name but on windows ...
 * no such luck.
 *
 */
PFUDLLEXPORT void
pfuLoadXFont(char *fontName, pfuXFont *fnt)
{
#ifdef WIN32
    HFONT font;
    LOGFONT logicalFont;
#else
    Display *xdisplay;
#endif

    if (!fontName)
      fontName = dftFontName;

#ifdef WIN32
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
	     "pfuLoadXFont: On WIN32, you should really call pfuLoadWINFont");
    pfuXLFDtoLOGFONT(fontName, &logicalFont);
    pfuLoadWINFont(&logicalFont, fnt);
#else

    xdisplay = pfGetCurWSConnection();
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
#endif
}

PFUDLLEXPORT void
pfuMakeXFontBitmaps(pfuXFont *fnt)
{
#ifndef WIN32
  Font id;
#endif
  unsigned int first, last;
  
  if (fnt && fnt->info)
    {
#ifndef WIN32
      id = fnt->info->fid;
#endif
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
#ifdef WIN32
      first = 0;
      last = 128;
      SelectObject(wglGetCurrentDC(), fnt->info);
      wglUseFontBitmaps(wglGetCurrentDC(),first,last-first+1,fnt->handle);
#else
      glXUseXFont(id, first, last-first+1, fnt->handle+first);
#endif
      /*    *height = fnt->info->ascent + fnt->info->descent;
       *width = fnt->info->max_bounds.width;  */
    }
  else
    pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	     "pfuMakeXFontBitmaps - null pfuXFont*");
}

PFUDLLEXPORT void
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

#ifdef WIN32
HDC QueryDC = NULL;
#endif


PFUDLLEXPORT int 
pfuGetXFontWidth(pfuXFont *f, const char *str)
{
    if (f && f->info)
    {
	int len = (int)strlen(str);
	/*  int w = (len * (f->info->max_bounds.rbearing - f->info->min_bounds.lbearing));  */
#ifdef WIN32
	BOOL status;
	SIZE stringSize;
	HDC UseDC = 0;
	if(QueryDC == 0) {
	  QueryDC = CreateDC("DISPLAY", NULL, NULL, NULL);
	  if(!QueryDC) {
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		     "pfuGetXFontWidth: Unable to create a device context!");
	  }
	}
	
	if(QueryDC) {
	  HGDIOBJ retval;
	  UseDC = QueryDC;
	  retval = SelectObject(QueryDC, f->info);
	  if((retval == 0) || (retval == GDI_ERROR)) 
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		     "pfuGetXFontWidth: Failed to SelectObject font\n");
	}
	else
	  UseDC = GetDC(NULL); /* Use screen DC */
	
	status = GetTextExtentPoint32(UseDC,
				      str,
				      len,
				      &stringSize);
	if(!status)
	  pfNotify(PFNFY_WARN, PFNFY_PRINT,
		   "pfuGetXFontWidth: failed to query font width properly");
	
        return stringSize.cx;
#else
	int w = XTextWidth(f->info, str, len);
	return w;
#endif
    }
    else
      return 0;
}

PFUDLLEXPORT int 
pfuGetXFontHeight(pfuXFont *f)
{
    if (f && f->info)
    {
#ifdef WIN32
      BOOL status;
      SIZE stringSize;
      TEXTMETRIC tm;
      HDC UseDC = 0;
      if(QueryDC == 0) {
	QueryDC = CreateDC("DISPLAY", NULL, NULL, NULL);
	if(!QueryDC) {
	  pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		   "pfuGetXFontHeight: Unable to create a device context!");
	}
      }
      
      if(QueryDC) {
	HGDIOBJ retval;
	UseDC = QueryDC;
	retval = SelectObject(QueryDC, f->info);
	if((retval == 0) || (retval == GDI_ERROR)) 
	  pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		   "pfuGetXFontHeight: Failed to SelectObject font\n");
      }
      else
	UseDC = GetDC(NULL); /* Use screen DC */
      
      status = GetTextMetrics(UseDC, &tm);
      
      
      if(!status)
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuGetXFontHeight: failed to query font height properly");
      
      return (tm.tmHeight * GetDeviceCaps(UseDC, LOGPIXELSY)) / 72;
#else
      int h =  f->info->ascent + f->info->descent;
      return h;
#endif
    }
    else
      return 0;
}

PFUDLLEXPORT void 
pfuGetCurXFont(pfuXFont *f)
{
  *f =  pfuCurXFont;
}

PFUDLLEXPORT void 
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

PFUDLLEXPORT void 
pfuCharPos(float x, float y, float z)
{
    glRasterPos3f(x,y,z);
}

PFUDLLEXPORT void 
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

PFUDLLEXPORT void 
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
