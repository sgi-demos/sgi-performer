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

#include <windows.h>
#include <stdio.h>
#include <Performer/pr/pfMemory.h>
#include "pfuPixelFormat.h"


/* WGL_ARB_extensions_string */
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;

/* WGL_ARB_pixel_format */
PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB = NULL;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC wglGetPixelFormatAttribfvARB = NULL;
PFNWGLCHOOSEPIXELFORMATARBPROC      wglChoosePixelFormatARB = NULL;


/* Emulated WGL_ARB_extensions_string */
const char * __stdcall wglGetExtensionsStringARB_pfuemu(HDC dc) {
  static char *pfWGLExtensions = "";
  return pfWGLExtensions;
}


/* Emulated WGL_ARB_pixel_format functions */
BOOL  __stdcall wglGetPixelFormatAttribivARB_pfuemu (HDC hdc, 
				    int iPixelFormat, 
				    int iLayerPlane, 
				    UINT nAttributes, 
				    const int *piAttributes, 
				    int *piValues) {
  
  /* Normally, wglGetPixelFormatAttribivARB is not invoked with 0 for the iPixelFormat, but
     that is the way to query the number of visuals, so have that happen in a special case */
  if( (iPixelFormat == 0) && (piAttributes[0] == WGL_NUMBER_PIXEL_FORMATS_ARB)) {
    piValues[0] = DescribePixelFormat(hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), NULL);
    return TRUE;
  }
  /* 0 is not a queryable format */
  else if( (iPixelFormat == 0)) 
    return FALSE;
  
  /* Describe main plane */
  if(iLayerPlane == 0) {
    PIXELFORMATDESCRIPTOR pfd;
    pfd.nVersion = 1;
    pfd.nSize = sizeof(pfd);
    
    int ret = DescribePixelFormat(hdc, iPixelFormat, sizeof(pfd), &pfd);
    if(ret == 0) {
      return FALSE;
    }
    int idx = 0;
    for(idx = 0; idx < nAttributes; idx++) {
      switch(piAttributes[idx]) {
	/* These can not be emulated for the main plane, pick safe values */
      case WGL_SHARE_DEPTH_ARB:	 
      case WGL_SHARE_STENCIL_ARB:
      case WGL_SHARE_ACCUM_ARB:	 
      case WGL_TRANSPARENT_ARB:
      case WGL_DRAW_TO_PBUFFER_ARB:
      case WGL_SAMPLE_BUFFERS_ARB:
	piValues[idx] = GL_FALSE;
	break;
      case WGL_TRANSPARENT_RED_VALUE_ARB:
      case WGL_TRANSPARENT_GREEN_VALUE_ARB:
      case WGL_TRANSPARENT_BLUE_VALUE_ARB:
      case WGL_TRANSPARENT_ALPHA_VALUE_ARB:
      case WGL_TRANSPARENT_INDEX_VALUE_ARB:
      case WGL_MAX_PBUFFER_PIXELS_ARB:	  
      case WGL_MAX_PBUFFER_WIDTH_ARB:		  
      case WGL_MAX_PBUFFER_HEIGHT_ARB:	  
      case WGL_SAMPLES_ARB:
	piValues[idx] = 0;
	break;
	/* The queries below can be emulated */
      case WGL_DRAW_TO_WINDOW_ARB:	  
	if(pfd.dwFlags & PFD_DRAW_TO_WINDOW)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_DRAW_TO_BITMAP_ARB:	  
	if(pfd.dwFlags & PFD_DRAW_TO_BITMAP)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_ACCELERATION_ARB:		  
	if( (pfd.dwFlags & PFD_GENERIC_FORMAT) == PFD_GENERIC_FORMAT)
	  if( (pfd.dwFlags & PFD_GENERIC_ACCELERATED)  == PFD_GENERIC_ACCELERATED)
	    piValues[idx] = WGL_GENERIC_ACCELERATION_ARB;
	  else
	    piValues[idx] = WGL_NO_ACCELERATION_ARB;
	else
	  piValues[idx] = WGL_FULL_ACCELERATION_ARB;
	break;
      case WGL_SWAP_METHOD_ARB:	  	  
	if((pfd.dwFlags & PFD_SWAP_EXCHANGE))
	  piValues[idx] = WGL_SWAP_EXCHANGE_ARB;
	else if((pfd.dwFlags & PFD_SWAP_COPY))
	  piValues[idx] = WGL_SWAP_COPY_ARB;
	else
	  piValues[idx] = WGL_SWAP_UNDEFINED_ARB;
	break;
      case WGL_NEED_PALETTE_ARB:
	if(pfd.dwFlags & PFD_NEED_PALETTE)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_NEED_SYSTEM_PALETTE_ARB:
	if(pfd.dwFlags & PFD_NEED_SYSTEM_PALETTE)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_SWAP_LAYER_BUFFERS_ARB:  
	if(pfd.dwFlags & PFD_SWAP_LAYER_BUFFERS)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_NUMBER_OVERLAYS_ARB:	  
	piValues[idx] = ((pfd.bReserved) & 0x0f);
	break;
      case WGL_NUMBER_PIXEL_FORMATS_ARB:
	piValues[idx] = ret;
	break;
      case WGL_NUMBER_UNDERLAYS_ARB:	  
	piValues[idx] = ((pfd.bReserved) & 0xf0) >> 4;
	break;
      case WGL_SUPPORT_GDI_ARB:	  
	if(pfd.dwFlags & PFD_SUPPORT_GDI)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_PIXEL_TYPE_ARB:	  
	if(pfd.iPixelType == PFD_TYPE_COLORINDEX)
	  piValues[idx] = WGL_TYPE_COLORINDEX_ARB;
	else
	  piValues[idx] = WGL_TYPE_RGBA_ARB;
	break;
      case WGL_SUPPORT_OPENGL_ARB:
	if(pfd.dwFlags & PFD_SUPPORT_OPENGL)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_DOUBLE_BUFFER_ARB:	 	  
	if(pfd.dwFlags & PFD_DOUBLEBUFFER) 
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_STEREO_ARB:	 
	if(pfd.dwFlags & PFD_STEREO) 
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_ACCUM_BITS_ARB:	  
	piValues[idx] = pfd.cAccumBits;
	break;
      case WGL_COLOR_BITS_ARB:	  
	piValues[idx] = pfd.cColorBits;
	break;
      case WGL_RED_BITS_ARB:	  
	piValues[idx] = pfd.cRedBits;
	break;
      case WGL_GREEN_BITS_ARB:	 
	piValues[idx] = pfd.cGreenBits;
	break;
      case WGL_BLUE_BITS_ARB:	  
	piValues[idx] = pfd.cBlueBits;
	break;
      case WGL_ALPHA_BITS_ARB:	  
	piValues[idx] = pfd.cAlphaBits;
	break;
      case WGL_ACCUM_RED_BITS_ARB:	  
	piValues[idx] = pfd.cAccumRedBits;
	break;
      case WGL_ACCUM_GREEN_BITS_ARB:
	piValues[idx] = pfd.cAccumGreenBits;
	break;
      case WGL_ACCUM_BLUE_BITS_ARB:	  
	piValues[idx] = pfd.cAccumBlueBits;
	break;
      case WGL_ACCUM_ALPHA_BITS_ARB:	  
	piValues[idx] = pfd.cAccumAlphaBits;
	break;
      case WGL_DEPTH_BITS_ARB:	  
	piValues[idx] = pfd.cDepthBits;
	break;
      case WGL_STENCIL_BITS_ARB:		  
	piValues[idx] = pfd.cStencilBits;
	break;
      case WGL_AUX_BUFFERS_ARB:	  
	piValues[idx] = pfd.cAuxBuffers;
	break;
      default:
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "Encountered unknown attribute (0x%x) "
		 "in WGL_ARB_pixel_format emulation at position %d", piAttributes[idx], idx);
	piValues[idx] = 0;
      }
    } //for
  }
  else {
    //describe layer plane
    LAYERPLANEDESCRIPTOR lpd;
    lpd.nVersion = 1;
    lpd.nSize = sizeof(lpd);
    int ret = wglDescribeLayerPlane(hdc, 
				    iPixelFormat, 
				    iLayerPlane, 
				    sizeof(LAYERPLANEDESCRIPTOR),
				    &lpd);
    if(ret == 0) 
      return FALSE;
    
    int idx = 0;
    for(idx = 0; idx < nAttributes; idx++) {
      switch(piAttributes[idx]) {
	/* These can not be emulated for the layer plane, pick safe values */
      case WGL_DRAW_TO_PBUFFER_ARB:
      case WGL_SAMPLE_BUFFERS_ARB:
      case WGL_NEED_PALETTE_ARB:
      case WGL_NEED_SYSTEM_PALETTE_ARB:
      case WGL_SWAP_LAYER_BUFFERS_ARB:  
      case WGL_DRAW_TO_WINDOW_ARB:
      case WGL_DRAW_TO_BITMAP_ARB:
	piValues[idx] = GL_FALSE;
	break;
      case WGL_NUMBER_PIXEL_FORMATS_ARB:
      case WGL_NUMBER_OVERLAYS_ARB:	  
      case WGL_NUMBER_UNDERLAYS_ARB:	  
      case WGL_MAX_PBUFFER_PIXELS_ARB:	  
      case WGL_MAX_PBUFFER_WIDTH_ARB:		  
      case WGL_MAX_PBUFFER_HEIGHT_ARB:	  
      case WGL_SAMPLES_ARB:
	piValues[idx] = 0;
	break;
	/* The queries below can be emulated */
      case WGL_SHARE_DEPTH_ARB:	 
	if(lpd.dwFlags & LPD_SHARE_DEPTH)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_SHARE_STENCIL_ARB:
	if(lpd.dwFlags & LPD_SHARE_STENCIL)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_SHARE_ACCUM_ARB:	 
	if(lpd.dwFlags & LPD_SHARE_ACCUM)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_TRANSPARENT_ARB:
	if(lpd.dwFlags & LPD_TRANSPARENT)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_SWAP_METHOD_ARB:	  	  
	if((lpd.dwFlags & LPD_SWAP_EXCHANGE))
	  piValues[idx] = WGL_SWAP_EXCHANGE_ARB;
	else if((lpd.dwFlags & LPD_SWAP_COPY))
	  piValues[idx] = WGL_SWAP_COPY_ARB;
	else
	  piValues[idx] = WGL_SWAP_UNDEFINED_ARB;
	break;
      case WGL_SUPPORT_GDI_ARB:	  
	if(lpd.dwFlags & LPD_SUPPORT_GDI)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_PIXEL_TYPE_ARB:	  
	if(lpd.iPixelType == LPD_TYPE_COLORINDEX)
	  piValues[idx] = WGL_TYPE_COLORINDEX_ARB;
	else
	  piValues[idx] = WGL_TYPE_RGBA_ARB;
	break;
      case WGL_SUPPORT_OPENGL_ARB:
	if(lpd.dwFlags & LPD_SUPPORT_OPENGL)
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_DOUBLE_BUFFER_ARB:	 	  
	if(lpd.dwFlags & LPD_DOUBLEBUFFER) 
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_STEREO_ARB:	 
	if(lpd.dwFlags & LPD_STEREO) 
	  piValues[idx] = GL_TRUE;
	else
	  piValues[idx] = GL_FALSE;
	break;
      case WGL_ACCUM_BITS_ARB:	  
	piValues[idx] = lpd.cAccumBits;
	break;
      case WGL_COLOR_BITS_ARB:	  
	piValues[idx] = lpd.cColorBits;
	break;
      case WGL_RED_BITS_ARB:	  
	piValues[idx] = lpd.cRedBits;
	break;
      case WGL_GREEN_BITS_ARB:	 
	piValues[idx] = lpd.cGreenBits;
	break;
      case WGL_BLUE_BITS_ARB:	  
	piValues[idx] = lpd.cBlueBits;
	break;
      case WGL_ALPHA_BITS_ARB:	  
	piValues[idx] = lpd.cAlphaBits;
	break;
      case WGL_ACCUM_RED_BITS_ARB:	  
	piValues[idx] = lpd.cAccumRedBits;
	break;
      case WGL_ACCUM_GREEN_BITS_ARB:
	piValues[idx] = lpd.cAccumGreenBits;
	break;
      case WGL_ACCUM_BLUE_BITS_ARB:	  
	piValues[idx] = lpd.cAccumBlueBits;
	break;
      case WGL_ACCUM_ALPHA_BITS_ARB:	  
	piValues[idx] = lpd.cAccumAlphaBits;
	break;
      case WGL_DEPTH_BITS_ARB:	  
	piValues[idx] = lpd.cDepthBits;
	break;
      case WGL_STENCIL_BITS_ARB:		  
	piValues[idx] = lpd.cStencilBits;
	break;
      case WGL_AUX_BUFFERS_ARB:	  
	piValues[idx] = lpd.cAuxBuffers;
	break;
      case WGL_TRANSPARENT_RED_VALUE_ARB:
      case WGL_TRANSPARENT_GREEN_VALUE_ARB:
      case WGL_TRANSPARENT_BLUE_VALUE_ARB:
      case WGL_TRANSPARENT_ALPHA_VALUE_ARB:
      case WGL_TRANSPARENT_INDEX_VALUE_ARB:
	/* This is not guaranteed to work for RGBA, but in practice it will :),
	   since this is normally = 0 
	*/
	piValues[idx] = lpd.crTransparent;
	break;
      default:
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "Encountered unknown attribute (0x%x) in WGL_ARB_pixel_format"
		 "emulation for layer %d", piAttributes[idx], iLayerPlane);
	piValues[idx] = 0;
      }
    } //for

  }
  return TRUE;
}

BOOL   __stdcall
wglGetPixelFormatAttribfvARB_pfuemu (HDC hdc, 
				    int iPixelFormat, 
				    int iLayerPlane, 
				    UINT nAttributes, 
				    const int *piAttributes, 
				    FLOAT *pfValues) {
  int *iValues = new int[nAttributes];
  int ret = wglGetPixelFormatAttribivARB_pfuemu(hdc,
					       iPixelFormat,
					       iLayerPlane,
					       nAttributes,
					       piAttributes,
					       iValues);
  if(ret) {
    for(int i=0; i < nAttributes; i++)
      pfValues[i] = float(iValues[i]);
  }
  
  delete iValues;
  return ret;
}


union IntFloat {
  float f;
  int i;
};


BOOL   __stdcall
wglChoosePixelFormatARB_pfuemu(HDC hdc, 
			      const int *piAttribIList, 
			      const FLOAT *pfAttribFList, 
			      UINT nMaxFormats, 
			      int *piFormats, 
			      UINT *nNumFormats) {

  if(!piFormats) 
    return FALSE;
  
  if(!nNumFormats)
    return FALSE;
      

  int fmtsFound = 0;
  
  int count = 0;
  if(piAttribIList)
    while(piAttribIList[count]) count+=2;
  /* generate an int attrib list to query */
  int intCount = count;
  
  count = 0;
  if(pfAttribFList)
    while(pfAttribFList[count]) count+=2;
  /* generate float query array */
  int floatCount = count;
  
  /* If both the int list and float list are 0 length, return all visuals. This is a special case */
  if( (intCount == 0) && (floatCount == 0)) {
    int numFormats = DescribePixelFormat(hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), NULL);
    for(int i=0; (i < nMaxFormats) && (i < (numFormats-1)); i++)
      piFormats[i] = i + 1;
    *nNumFormats = i;
  }

  IntFloat *userVals = 0;
  IntFloat *vals = 0;
  int *attribs = 0;
  
  userVals = (IntFloat *)alloca(sizeof(IntFloat) * ((intCount + floatCount) / 2));
  vals = (IntFloat *)alloca(sizeof(IntFloat) * ((intCount + floatCount) / 2));
  attribs = (int *)alloca(sizeof(int) * ((intCount + floatCount) / 2));
  
  /* Copy all int attributes into one attrib array */
  count = 0;
  if(piAttribIList)
    while(piAttribIList[count * 2]) {
      attribs[count] = piAttribIList[2*count];
      userVals[count].i = piAttribIList[2*count+1];
      count++;
    }
  /* add the floats to the same array */
  int idx=0;
  if(pfAttribFList)
    while(pfAttribFList[idx]) {
      attribs[count] = pfAttribFList[idx];
      userVals[count].f = pfAttribFList[idx+1];
      count++;
      idx+=2;
    }
  
  //get number of pixel formats
  int numFormats = DescribePixelFormat(hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), NULL);
  if(!numFormats) {
    pfNotify(PFNFY_WARN, PFNFY_PRINT,"wglChoosePixelFormatARB_pfuemu unable to query number "
	     "of pixel formats");
    return FALSE;
  }
  
  
  /* loop through all formats, checking for matches */
  for(int i = 1; (i < numFormats + 1) && (fmtsFound < nMaxFormats); i++) {
    int ret;
    if(intCount) {
      ret = wglGetPixelFormatAttribivARB_pfuemu(hdc,
					       i,
					       0,
					       intCount/2,
					       attribs,
					       &(vals[0].i));
      
      if(ret == 0) {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "wglGetPixelFormatAttribivARB_pfuemu failed");
	continue;
      }
    }

    if(floatCount) {
      ret = wglGetPixelFormatAttribfvARB_pfuemu(hdc,
					       i,
					       0,
					       floatCount/2,
					       &attribs[intCount/2],
					       &(vals[intCount/2].f));
      
      if(ret == 0) {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "wglGetPixelFormatAttribfvARB_pfuemu failed");
	continue;
      }
    }
    
    int success = 1;
    
    /* Loop through all integer attributes to see if any 
       violates our criteria. Only need to check for ones we can fake */
    
    for(int j=0; j < (floatCount + intCount) / 2; j++) {
      switch(attribs[j]) {
	/* These can not be emulated. Asking for a visual with these is not possible */
      case WGL_SAMPLES_ARB:
      case WGL_SAMPLE_BUFFERS_ARB:
	if(j < (intCount / 2)) {
	  /* Do int comparison */
	  if(userVals[j].i != 0) {
	    success = 0;
	  }
	}
	else {
	  /* Do float comparison */
	  if(userVals[j].f != 0) { 
	    success = 0;
	  }
	}
	break;
	/* Do exact match comparisons on these */
      case WGL_ACCELERATION_ARB:    /*	    enum	exact */
      case WGL_DRAW_TO_WINDOW_ARB:  /*	    boolean	exact */
      case WGL_DRAW_TO_BITMAP_ARB:  /*      boolean	exact */
      case WGL_NEED_PALETTE_ARB:    /*	    boolean	exact */
      case WGL_NEED_SYSTEM_PALETTE_ARB: /*  boolean	exact */
      case WGL_SWAP_LAYER_BUFFERS_ARB: /*   boolean	exact */
      case WGL_SWAP_METHOD_ARB: /*	    enum	exact */
      case WGL_SHARE_DEPTH_ARB: /*	    boolean	exact */
      case WGL_SHARE_STENCIL_ARB: /*	    boolean	exact */
      case WGL_SHARE_ACCUM_ARB: /*	    boolean	exact */
      case WGL_SUPPORT_GDI_ARB: /*	    boolean	exact */
      case WGL_SUPPORT_OPENGL_ARB: /*	    boolean	exact */
      case WGL_DOUBLE_BUFFER_ARB: /*	    boolean	exact */
      case WGL_STEREO_ARB: /*		    boolean	exact */
      case WGL_PIXEL_TYPE_ARB: /*	    enum	exact */
	if(j < (intCount / 2)) {
	  /* Do int comparison */
	  if(userVals[j].i != vals[j].i) {
	    success = 0;
	  }
	}
	else {
	  /* Do float comparison */
	  if(userVals[j].f != vals[j].f) { 
	    success = 0;
	  }
	}
	break;
	/* To greater-than comparisons on these */
      case WGL_NUMBER_OVERLAYS_ARB: /*	    integer	minimum */
      case WGL_NUMBER_UNDERLAYS_ARB: /*	    integer	minimum */
      case WGL_COLOR_BITS_ARB: /*	    integer	minimum */
      case WGL_RED_BITS_ARB: /*		    integer	minimum */
      case WGL_GREEN_BITS_ARB: /*	    integer	minimum */
      case WGL_BLUE_BITS_ARB: /*	    integer	minimum */
      case WGL_ALPHA_BITS_ARB: /*	    integer	minimum */
      case WGL_ACCUM_BITS_ARB: /*	    integer	minimum */
      case WGL_ACCUM_RED_BITS_ARB: /*	    integer	minimum */
      case WGL_ACCUM_GREEN_BITS_ARB: /*	    integer	minimum */
      case WGL_ACCUM_BLUE_BITS_ARB: /*	    integer	minimum */
      case WGL_ACCUM_ALPHA_BITS_ARB: /*	    integer	minimum */
      case WGL_DEPTH_BITS_ARB: /*	    integer	minimum */
      case WGL_STENCIL_BITS_ARB: /*	    integer	minimum */
      case WGL_AUX_BUFFERS_ARB: /*	    integer	minimum */
	if(j < (intCount / 2)) {
	  /* Do int comparison */
	  if(userVals[j].i > vals[j].i) {
	    success = 0;
	  }
	}
	else {
	  /* Do float comparison */
	  if(userVals[j].f > vals[j].f) {
	    success = 0;
	  }
	}
	break;
      default:
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "wglChoosePixelFormatARB_pfuemu: unknown attribute in comparator: 0x%x",
		 attribs[j]);
      } //switch
    } //loop over individual attributes
    
    if(success) {
      /* found a good visual */
      piFormats[fmtsFound] = i;
      fmtsFound++;
    }
  } //loop over formats
  
  *nNumFormats = fmtsFound;
  return TRUE;
}

/*
  This is a message handler function which makes sure to ignore any window
   quit messages which are sent to dummy windows. Performer's default behavior
   is to pfExit on any of the messages.
*/
LONG WINAPI _pfuDummyWndProc ( 
			      HWND    hWnd, 
			      UINT    uMsg, 
			      WPARAM  wParam, 
			      LPARAM  lParam) { 
  switch (uMsg) { 
  case WM_CREATE: 
    return 1;
  case WM_CLOSE:
  case WM_QUIT:
  case WM_DESTROY:
  case WM_NCDESTROY:
    return 1;
  default: 
    return DefWindowProc (hWnd, uMsg, wParam, lParam); 
  } 
}


/* These variables are used to back up the current hdc and context */
HDC backupHDC;
HGLRC backupContext;

#define WNDCLASS_NAME "pfuDummyQueryWindow"
pfuDummyQueryWindow *

pfuCreateDummyWGLWindow() {
  /* Back up current DC and context */
  backupHDC = wglGetCurrentDC();
  backupContext = wglGetCurrentContext();
  
  /* Register a class for this window */
  static int first = 1;
  if(first) {
    first = 0;
    HINSTANCE myHandle = GetModuleHandle(NULL);
    /* Register our window class once, so do it in here */
    WNDCLASS wndclass;
    wndclass.style         = CS_OWNDC; 
    wndclass.lpfnWndProc   = (WNDPROC)_pfuDummyWndProc;
    wndclass.cbClsExtra    = 0; 
    wndclass.cbWndExtra    = 0; 
    wndclass.hInstance     = myHandle;
    wndclass.hIcon         = LoadIcon(myHandle, IDI_APPLICATION); 
    wndclass.hCursor       = LoadCursor(myHandle, IDC_ARROW); 
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); 
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = WNDCLASS_NAME;
    
    if (!RegisterClass (&wndclass) ) {
      pfNotify(PFNFY_FATAL, PFNFY_PRINT, "Performer dummy WNDCLASS failed to register!");
    }
  }

  HWND hWnd = CreateWindow(WNDCLASS_NAME,
			   "Temporary window you should not see",
			   0,
			   0, 0,
			   1, 1, /* 1x1 pixel window to keep Windows happy */
			   NULL,
			   NULL,
			   GetModuleHandle(NULL),
			   NULL);
  
  
  if(!hWnd) {
    pfNotify(PFNFY_WARN, PFNFY_FATAL, "Could not create window in pfCreateDummyWindow");
  }
  
  /* Get the device context from the window */
  HDC windowHDC =  GetDC(hWnd); 
  
  /* Create a context using old-school WGL so that we can query if WGL extensions are
     present. On windows, it is necessary to have a context in order to call
     wglChoosePixelFormat and friends */
  PIXELFORMATDESCRIPTOR pfd;
  PIXELFORMATDESCRIPTOR* ppfd;
  int pixelformat; 
  DWORD flags;
  flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
  ppfd = &pfd; 
  ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR); 
  ppfd->nVersion = 1; 
  ppfd->dwFlags = flags; 
  ppfd->dwLayerMask = PFD_MAIN_PLANE; 
  ppfd->iPixelType = PFD_TYPE_RGBA; 
  ppfd->cColorBits = 24;
  ppfd->cDepthBits = 16; 
  ppfd->cStencilBits = 0;
  
  pixelformat = ChoosePixelFormat(windowHDC, ppfd); 
  
  if ( (pixelformat = ChoosePixelFormat(windowHDC, ppfd)) == 0 ) 
    { 
      pfNotify(PFNFY_FATAL, PFNFY_PRINT, "pfCreateDummyWindow : Could not get pixel format\n");
    } 
  
  if (SetPixelFormat(windowHDC, pixelformat, ppfd) == FALSE) 
    { 
      pfNotify(PFNFY_FATAL, PFNFY_PRINT, "pfCreateDummyWindow : Could not set pixel format\n");
    } 
  
  pfGLContext gCxt = wglCreateContext(windowHDC); 
  
  if(gCxt == NULL) {
    pfNotify(PFNFY_FATAL, PFNFY_PRINT, "pfCreateDummyWindow : Could not create OpenGL context\n");
  }
  
  if(wglMakeCurrent(windowHDC, gCxt) == NULL) {
    pfNotify(PFNFY_FATAL, PFNFY_PRINT, "pfCreateDummyWindow : Could not bind current context \n");
  }
  
  /* Query extensions */
  
  /* First, check for WGL_ARB_extensions_string, which is a prerequisite for all others */
  wglGetExtensionsStringARB = 
    (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
  
  if(wglGetExtensionsStringARB == NULL) {
    /* This extension is not present, use the emulated version */
    wglGetExtensionsStringARB = wglGetExtensionsStringARB_pfuemu;
  }
  
  /* Now, check to see if we have WGL_ARB_pixel_format */
  int havePixelFormat = 0;
  
  const char *extensions = (*wglGetExtensionsStringARB)(windowHDC);
  if(strstr(extensions, "WGL_ARB_pixel_format"))
    havePixelFormat = 1;
  
  /* Force WGL_ARB_pixel_format emulation if requested */
  if(getenv("__PF_ENABLE_PFD_EMULATION"))
    havePixelFormat = 0;
  
  /* Now, based on whether the extension is present, set up the function pointers correctly */
  if(havePixelFormat) {
    /* Pixel format extension is present, initialize pointers */
    wglGetPixelFormatAttribivARB = 
      (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
    
    wglGetPixelFormatAttribfvARB = 
      (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
    
    wglChoosePixelFormatARB = 
      (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    
    if((!wglGetPixelFormatAttribivARB) || (!wglGetPixelFormatAttribfvARB) || (!wglChoosePixelFormatARB)) {
      pfNotify(PFNFY_FATAL, PFNFY_PRINT, "pfCreateDummyWindow: WGL is malfunctioning. Can not continue");
    }
    
  }
  else {
    /* Pixel format extension missing, emulate */
    wglGetPixelFormatAttribivARB = wglGetPixelFormatAttribivARB_pfuemu;
    wglGetPixelFormatAttribfvARB = wglGetPixelFormatAttribfvARB_pfuemu;
    wglChoosePixelFormatARB = wglChoosePixelFormatARB_pfuemu;
  }
  
  /* Create pfuDummyQueryWindow structure and fill it in */
  pfuDummyQueryWindow *ret = (pfuDummyQueryWindow *)pfMemory::malloc(sizeof(pfuDummyQueryWindow),
								     pfGetSharedArena());
  ret->window_ = hWnd;
  ret->context_ = gCxt;
  ret->wglGetExtensionsStringARB    = wglGetExtensionsStringARB;
  ret->wglGetPixelFormatAttribivARB = wglGetPixelFormatAttribivARB;
  ret->wglGetPixelFormatAttribfvARB = wglGetPixelFormatAttribfvARB;
  ret->wglChoosePixelFormatARB      = wglChoosePixelFormatARB;
  
  return ret;
}

void 
pfuDestroyDummyWGLWindow(pfuDummyQueryWindow *dw) {
  wglMakeCurrent(GetDC(dw->window_), NULL);
  wglDeleteContext(dw->context_);
  DestroyWindow(dw->window_);
  pfMemory::free(dw);
  /* Restore any context we may have had */
  wglMakeCurrent(backupHDC, backupContext);
}
