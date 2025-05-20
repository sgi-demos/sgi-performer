/*
 * Copyright 2000, Silicon Graphics, Inc.
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

#include <GL/glx.h>
#include <GL/gl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __linux__
#define fsqrt(x) sqrt(x)
#endif

/* To compile: cc -o simple simple.c -lGL -lX11 */

static int OpenGLInit(int size); /* sets up stuff to run opengl */

/* needed for X */
static Bool WaitForNotify(Display *d, XEvent *e, char *arg)
   { return (e->type == MapNotify) && (e->xmap.window == (Window)arg); }

typedef struct {
    char *str;
    GLenum format;
    GLenum type;
    int size;
} FileType;

FileType FileTypes[] = {
    {"888", GL_RGB, GL_UNSIGNED_BYTE, 3},
#ifdef GL_UNSIGNED_SHORT_5_5_5_1_EXT
    {"5551", GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1_EXT, 2},
#endif
#ifdef GL_UNSIGNED_SHORT_5_5_5_1
    {"5551", GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 2},
#endif

    {"L16", GL_LUMINANCE, GL_UNSIGNED_SHORT, 2},
    {"", 0, 0} /* error */
};

/* convert file format name to OpenGL format and type */
FileType *parseType(char *typename)
{
    FileType *filetype;
    for(filetype = FileTypes;*filetype->str;filetype++) {
	if(!strcmp(typename, filetype->str)) {
	    return filetype;
	}
    }
    return (FileType *)0; /* error */
}


main(int argc, char **argv)
{
   GLint val;
   GLint size; /* size of square image */
   GLubyte *image;
   FileType *dataInfo;
   FILE *fp = (FILE *)0;
   struct stat fileData;
   GLfloat offset; /* raster position */

   switch(argc) {
   case 1: /* give usage info */
       fprintf(stderr, "Usage: %s filename [888|5551|L16]\n"
	       "file is assumed square, and of filetype 888\n", argv[0]);
       return(-1);
   case 2: /* open file, assume it's 888, and figure out its size */
       if(!(fp = fopen(argv[1], "r"))) {
	   fprintf(stderr, "Couldn't open file %s\n", argv[1]);
	   return(-1);
       }
       dataInfo = parseType("888"); /* default to 888 */
       break;
   case 3: /* open file, get type, figure out its size */
       if(!(fp = fopen(argv[1], "r"))) {
	   fprintf(stderr, "Couldn't open file %s\n", argv[1]);
	   return(-1);
       }
       dataInfo = parseType(argv[2]);
       break;
   }
   /* find dimensions of file */
   fstat(fileno(fp), &fileData);
   size = fileData.st_size / dataInfo->size;
   size = (int)fsqrt((float)size);

   if(size < 128) { /* to make things look nice */
       OpenGLInit(128); /* opengl setup stuff */
       offset = -size/128.f;
       glRasterPos2f(offset, offset);
   } else
       OpenGLInit(size); /* opengl setup stuff */

   glClear(GL_COLOR_BUFFER_BIT); /* clear the screen to black */

   image = (GLubyte *)malloc(size * size * dataInfo->size);
   fread((void *)image, dataInfo->size, size * size, fp);

   glDrawPixels(size, size, dataInfo->format, dataInfo->type, (GLvoid *)image);
   glFlush();
   printf("press return to exit\n");
   (void)getchar();
}



static int OpenGLInit(int size) /* you can probably ignore this function */
{
   Colormap cmap;
   XSetWindowAttributes swa;
   XEvent event;
   int x1,y1,x2,y2;
   int arr[6];
   Display     *dpy;
   int         snu;
   XVisualInfo *vis;
   GLXContext  ctx;
   Window      rwi,win;

   /* get display */
   if (!(dpy = XOpenDisplay(0))) {
     fprintf(stderr,"ERROR: XOpenDisplay() failed.\n");
     return(0);
     }

   /* get screen number */
   snu = DefaultScreen(dpy);

   /* get visual */
   arr[0] = GLX_RGBA;
   arr[1] = None;
   if (!(vis = glXChooseVisual(dpy,snu,arr)))  {
     fprintf(stderr,"ERROR: glXChooseVisual() failed.\n");
     return(0);
     }

   /* get context */
   if (!(ctx = glXCreateContext(dpy,vis,0,GL_TRUE))) {
     fprintf(stderr,"ERROR: glXCreateContext() failed.\n");
     return(0);
     }

   /* root window */
   rwi = RootWindow(dpy,vis->screen);

   /* get RGBA colormap */
   if (!(cmap = XCreateColormap(dpy,rwi,vis->visual,AllocNone))) {
     fprintf(stderr,"ERROR: glXCreateColorMap() failed.\n");
     return(0);
     }

   x1 = 0;
   y1 = 0;
   x2 = size;
   y2 = size;

   /* get window */
   swa.colormap = cmap;
   swa.border_pixel = 0;
   swa.event_mask = StructureNotifyMask;
   if (!(win = XCreateWindow(dpy,rwi,x1,y1,x2,y2,0,vis->depth,
           InputOutput,vis->visual,CWBorderPixel|CWColormap|CWEventMask,&swa))) {
     fprintf(stderr,"ERROR: XCreateWindow() failed.\n");
     return(0);
     }

   XStoreName(dpy,win, "tile");
   XMapWindow(dpy,win);
   XFlush(dpy);

   XIfEvent(dpy,&event,WaitForNotify,(char*)win);

   glXMakeCurrent(dpy,win,ctx);
   return(1);
}

