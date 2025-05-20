
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <wglext.h>
/* WGL_ARB_extensions_string */
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;

/* WGL_ARB_pixel_format */
PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB = NULL;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC wglGetPixelFormatAttribfvARB = NULL;
PFNWGLCHOOSEPIXELFORMATARBPROC      wglChoosePixelFormatARB = NULL;

/* Emulated WGL_ARB_extensions_string */
const char * __stdcall wglGetExtensionsStringARB_emu(HDC dc) {
  static char *pfWGLExtensions = "";
  return pfWGLExtensions;
}


/* Emulated WGL_ARB_pixel_format functions */
BOOL  __stdcall wglGetPixelFormatAttribivARB_emu (HDC hdc, 
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
	printf("Requesting number of pixel formats, will return %d\n", ret); fflush(stdout);
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
	piValues[idx] = 0;
	return FALSE;
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
	return FALSE;
      }
    } //for
  }
  return TRUE;
}

BOOL   __stdcall
wglGetPixelFormatAttribfvARB_emu (HDC hdc, 
				    int iPixelFormat, 
				    int iLayerPlane, 
				    UINT nAttributes, 
				    const int *piAttributes, 
				    FLOAT *pfValues) {
  int *iValues = new int[nAttributes];
  int ret = wglGetPixelFormatAttribivARB_emu(hdc,
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

HWND hWnd = 0;
HGLRC gCxt;

void InitWGLPixelFormat() {
  /* Make a new context */
  HDC windowHDC =  GetDC(hWnd); 
  
  /* This is rather nasty, but it's necessary to create a throw-away context in order to get a
     pointer to wglGetExtensionsStringARB */
  PIXELFORMATDESCRIPTOR pfd, *ppfd; 
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
      fprintf(stderr, "InitWGLPixelFormat : Could not get pixel format\n");
      fflush(stderr);
      exit(-1);
    } 
  
  if (SetPixelFormat(windowHDC, pixelformat, ppfd) == FALSE) 
    { 
      fprintf(stderr, "InitWGLPixelFormat : Could not set pixel format\n");
      fflush(stderr);
       exit(-1);
    } 
  
  gCxt = wglCreateContext(windowHDC); 
  
  if(gCxt == NULL) {
    fprintf(stderr, "InitWGLPixelFormat : Could not create OpenGL context\n");
    fflush(stderr);
     exit(-1);
  }
  
  if(wglMakeCurrent(windowHDC, gCxt) == NULL) {
    fprintf(stderr, "pfInitWGLPixelFormat : Could not make current\n");
    fflush(stderr);
    exit(-1);
  }
  
  
  
  wglGetExtensionsStringARB = 
    (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
  
  int havePixelFormat = 0;

  if(wglGetExtensionsStringARB != NULL) {
    const char *extensions = (*wglGetExtensionsStringARB)(windowHDC);
    if(strstr(extensions, "WGL_ARB_pixel_format"))
      havePixelFormat = 1;
    //printf("WGL Extensions are: %s\n", extensions);
  }

  if(havePixelFormat) {
    /* Pixel format extension is present, initialize pointers */
    wglGetPixelFormatAttribivARB = 
      (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
    
    wglGetPixelFormatAttribfvARB = 
      (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");

    wglChoosePixelFormatARB = 
      (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

    if((!wglGetPixelFormatAttribfvARB) || (!wglGetPixelFormatAttribivARB))
      havePixelFormat = 0;
  }
  
  if(getenv("_EMULATE_PFD")) havePixelFormat = 0;

  if(!havePixelFormat) {
    printf("Emulating WGL_ARB_pixel_format\n");
    /* Pixel format extension missing, emulate */
    wglGetPixelFormatAttribivARB = wglGetPixelFormatAttribivARB_emu;
    wglGetPixelFormatAttribfvARB = wglGetPixelFormatAttribfvARB_emu;
    wglGetExtensionsStringARB = wglGetExtensionsStringARB_emu;
  }
  else {
    printf("Using driver supplied WGL_ARB_pixel_format\n");
  }
}

void printHeader() {
  printf("\n");
  printf("Vis  Vis Driver     Render  buff lev render DB ste color buf   aux dep ste accum buf   MS  MS\n");
  printf("ID   Dep Type       Type    size els type      reo r  g  b  a  buf th  ncl r  g  b  a  num bufs\n");
  printf("-----------------------------------------------------------------------------------------------\n");
  fflush(stderr);
}

void printExtensionList(const char *str) {
  
  if(!str) {
    printf("    None\n");
    return;
  }
  char separators[] = {' ', //space
		       0 //end of string
  };
  char *wglext = strdup(str);
  char *token = strtok(wglext, separators);
  printf("    ");
  int column = 4;
  while(token) {
    int width = strlen(token);
    if((column + width) > 80) {
      printf("\n    ");
      column = width+4;
    }
    else 
      column += width;
    printf("%s", token);
    token = strtok(NULL, separators);
    if(token)
      printf(", ");
    else
      printf("\n");
  }
  
  free(wglext);
}

#define NUM_ATTR 23

int main(int argc, char **argv) {
  HINSTANCE hInstance = GetModuleHandle(NULL);

  WNDCLASS wc;
  wc.style         = CS_OWNDC; 
  wc.lpfnWndProc   = (WNDPROC)DefWindowProc;
  wc.cbClsExtra    = 0; 
  wc.cbWndExtra    = 0; 
  wc.hInstance     = hInstance;
  wc.hIcon         = NULL;
  wc.hCursor       = NULL;
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); 
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = "wglinfoWNDCLASS";
  
  if (!RegisterClass (&wc) ) {
    fprintf(stderr, "Could not register WNDCLASS, exiting\n");
    fflush(stderr);
    return -1;
  }


  hWnd = CreateWindowEx(WS_EX_TRANSPARENT,
			"wglinfoWNDCLASS",
			"wglinfo",
			WS_POPUP,
			0, 0, 1, 1,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL);
  if(!hWnd) {
    fprintf(stderr, "Can not create a window to query visuals\n");
    fflush(stderr);
    return -1;
  }
  
  InitWGLPixelFormat();
  
  GLint numFormatIattr[] = {WGL_NUMBER_PIXEL_FORMATS_ARB};
  GLint nFormats = 0;


  GLint queryAttr[NUM_ATTR] = {
    WGL_COLOR_BITS_ARB, //0
    WGL_NUMBER_OVERLAYS_ARB, //1
    WGL_PIXEL_TYPE_ARB, //2
    WGL_DOUBLE_BUFFER_ARB, //3
    WGL_STEREO_ARB, //4
    WGL_RED_BITS_ARB, //5
    WGL_GREEN_BITS_ARB, //6
    WGL_BLUE_BITS_ARB, //7
    WGL_ALPHA_BITS_ARB, //8
    WGL_AUX_BUFFERS_ARB, //9 
    WGL_DEPTH_BITS_ARB, //10
    WGL_STENCIL_BITS_ARB, //11
    WGL_ACCUM_RED_BITS_ARB, //12
    WGL_ACCUM_GREEN_BITS_ARB, //13
    WGL_ACCUM_BLUE_BITS_ARB, //14
    WGL_ACCUM_ALPHA_BITS_ARB, //15
    WGL_ACCELERATION_ARB, //16
    WGL_SUPPORT_OPENGL_ARB, //17
    WGL_DRAW_TO_WINDOW_ARB, //18
    WGL_DRAW_TO_BITMAP_ARB, //19
    WGL_DRAW_TO_PBUFFER_ARB,//20
    WGL_SAMPLES_ARB, //21
    WGL_SAMPLE_BUFFERS_ARB, //22
  };
  
  GLint r[NUM_ATTR];
  (*wglGetPixelFormatAttribivARB)(GetDC(hWnd), 0, 0, 1, numFormatIattr, &nFormats);
  
  /* Print the WGL extension string */
  printf("WGL Extensions:\n");
  const char *wglext = (*wglGetExtensionsStringARB)(GetDC(hWnd));
  printExtensionList(wglext);
  
  printf("OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
  printf("OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
  printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
  printf("OpenGL Extensions:\n");
  printExtensionList((char *)glGetString(GL_EXTENSIONS));
  
  const char *ms1str = "WGL_ARB_multisample";
  const char *ms2str = "WGL_EXT_multisample";

  int nQuery = NUM_ATTR;
  
  /* if multisample is not there, can't ask about it */
  if((!wglext) || 
     (!strstr(wglext, ms1str)) && (!strstr(wglext, ms2str)))
    nQuery -= 2;

  int lines = 0;
  for(int i=1; i < (nFormats+1); i++) {
    //clean out desitnation
    for(int j=0; j < NUM_ATTR; j++) r[j] = 0;
    if(lines % 25 == 0) printHeader();
    BOOL status = (*wglGetPixelFormatAttribivARB)(GetDC(hWnd), i, 0, nQuery, queryAttr, r);
    
    if((r[17] != GL_TRUE)) continue; //OpenGL not supported 
    lines++;
    
    char *accel;
    if(r[16] == WGL_FULL_ACCELERATION_ARB)
      accel = "Hardware";
    else if(r[16] == WGL_GENERIC_ACCELERATION_ARB)
      accel = "Generic ";
    else
      accel = "Software";
    
    
    if(status == TRUE) {
      //printf("0x%2x  %2d %s        %1d   %2d   %1d   %s  %1d   %1d %2d %2d %2d %2d %3d %3d %3d %2d %2d %2d %2d %3d %3d\n", 
      
      //print visual
      printf("0x%2x ", i);
      
      //print color bits
      printf("%3d", r[0]);
      
      //print acceleration type
      printf(" %s   ", accel);
      
      // print draw type 
      if(r[18] == GL_TRUE)
	printf("W");
      else
	printf(".");
      
      if(r[19] == GL_TRUE)
	printf("B");
      else
	printf(".");
      
      if(r[20] == GL_TRUE)
	printf("P");
      else
	printf(".");
      
      // print number of color bits
      printf("     %3d ", r[0]);
      
      //print number of overlay
      printf(" %2d   ", r[1]);

      //print pixel type
      if(r[2] == WGL_TYPE_RGBA_ARB)
	printf("rgba");
      else
	printf("ci  ");
      
      // print double buffer
      if(r[3] == GL_TRUE)
	printf("   y");
      else
	printf("   .");
      
      //print stereo
      if(r[4] == GL_TRUE)
	printf("  y");
      else
	printf("  .");

      // print R
      if(r[5] > 0)
	printf(" %2d", r[5]);
      else
	printf("  .");

      // print G
      if(r[6] > 0)
	printf(" %2d", r[6]);
      else
	printf("  .");

      // print B
      if(r[7] > 0)
	printf(" %2d", r[7]);
      else
	printf("  .");
      
      // print A
      if(r[8] > 0)
	printf(" %2d", r[8]);
      else
	printf("  .");
      
      // print aux buffers
      if(r[9] > 0)
	printf("  %2d", r[9]);
      else
	printf("   .");
      
      // print depth
      if(r[10] > 0)
	printf("   %2d", r[10]);
      else
	printf("    .");
      
      // print stencil
      if(r[11] > 0)
	printf(" %2d ", r[11]);
      else
	printf("  . ");
      
       // print accum R
      if(r[12] > 0)
	printf(" %2d", r[12]);
      else
	printf("  .");

      // print accum G
      if(r[13] > 0)
	printf(" %2d", r[13]);
      else
	printf("  .");

      // print accum B
      if(r[14] > 0)
	printf(" %2d", r[14]);
      else
	printf("  .");
      
      // print accum A
      if(r[15] > 0)
	printf(" %2d", r[15]);
      else
	printf("  .");
      
      //print MS
      if(r[21] > 0)
	printf(" %2d", r[21]);
      else
	printf("  .");
      
      //print MS buf
      if(r[22] > 0)
	printf("  %2d", r[22]);
      else
	printf("   .");
      
      printf("\n");
    }
    else {
      printf("0x%2x ..failed to query visual\n", i);
    }
    fflush(stdout);
  }
  
  //wglMakeCurrent(GetDC(hWnd), NULL);
  //wglDeleteContext(gCxt);
  DestroyWindow(hWnd);

  return 1;
}
