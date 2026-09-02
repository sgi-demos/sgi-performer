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
 * file: snapwin.c
 * ---------------
 * These routines are internal utility routines for use in test programs
 * 
 * $Revision: 1.44 $
 * $Date: 2002/05/15 00:31:09 $
 */

#include <stdlib.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>

#include <X11/X.h>
#endif

#include <Performer/image.h>
#include <Performer/pfutil.h>


PFUDLLEXPORT void
pfuCpackToRGB(unsigned int *l, unsigned short *r, unsigned short *g, unsigned short *b, int n)
{
    while (n >= 8) 
    {
	PFU_CPACKTORGB(l[0], r[0], g[0], b[0]);
	PFU_CPACKTORGB(l[1], r[1], g[1], b[1]);
	PFU_CPACKTORGB(l[2], r[2], g[2], b[2]);
	PFU_CPACKTORGB(l[3], r[3], g[3], b[3]);
	PFU_CPACKTORGB(l[4], r[4], g[4], b[4]);
	PFU_CPACKTORGB(l[5], r[5], g[5], b[5]);
	PFU_CPACKTORGB(l[6], r[6], g[6], b[6]);
	PFU_CPACKTORGB(l[7], r[7], g[7], b[7]);
	l += 8;
	r += 8;
	g += 8;
	b += 8;
	n -= 8;
    }

    while (n--) 
    {
	PFU_CPACKTORGB(l[0], r[0], g[0], b[0]);
	l++;
	r++;
	g++;
	b++;
    }
}

PFUDLLEXPORT void
pfuCpackToRGBA(unsigned int *l, unsigned short *r, unsigned short *g, unsigned short *b, unsigned short *a, int n)
{
    while (n >= 8) 
    {
	PFU_CPACKTORGBA(l[0], r[0], g[0], b[0], a[0]);
	PFU_CPACKTORGBA(l[1], r[1], g[1], b[1], a[1]);
	PFU_CPACKTORGBA(l[2], r[2], g[2], b[2], a[2]);
	PFU_CPACKTORGBA(l[3], r[3], g[3], b[3], a[3]);
	PFU_CPACKTORGBA(l[4], r[4], g[4], b[4], a[4]);
	PFU_CPACKTORGBA(l[5], r[5], g[5], b[5], a[5]);
	PFU_CPACKTORGBA(l[6], r[6], g[6], b[6], a[6]);
	PFU_CPACKTORGBA(l[7], r[7], g[7], b[7], a[7]);
	l += 8;
	r += 8;
	g += 8;
	b += 8;
	a += 8;
	n -= 8;
    }

    while (n--) 
    {
	PFU_CPACKTORGBA(l[0], r[0], g[0], b[0], a[0]);
	l++;
	r++;
	g++;
	b++;
	a++;
    }
}

/* the standard error handler calls exit(). we don't want to quit out of 
 * the application */
void
my_iopenerror(char *str)
{
    pfNotify(PFNFY_WARN, PFNFY_RESOURCE, "pfuSaveImage: %s", str);
}

extern void i_seterror(void (*)(char *)); /* from libimage.a */

PFUDLLEXPORT int
pfuSaveImage(char *name, int xorg, int yorg, int xsize, int ysize, int alpha)
{
    IMAGE *oimage;
    int writeerr, y;
    unsigned int *scrbuf, *ss;
    unsigned short *rs, *gs, *bs, *as;

    i_seterror(my_iopenerror);
    /* open the image file */
    oimage = iopen(name, "w", RLE(1), 3, xsize, ysize, alpha ? 4 : 3);

    if (!oimage) 
    {
	return -1;
    }

    writeerr = 0;
    
    /* malloc buffers */
    rs = (unsigned short *)pfMalloc((unsigned int)(xsize*sizeof(short)),  NULL);
    gs = (unsigned short *)pfMalloc((unsigned int)(xsize*sizeof(short)),  NULL);
    bs = (unsigned short *)pfMalloc((unsigned int)(xsize*sizeof(short)),  NULL);
    if (alpha)
	as = (unsigned short *)pfMalloc((unsigned int)(xsize*sizeof(short)),  NULL);

    scrbuf = (unsigned long *)pfMalloc((unsigned int)(xsize*ysize*sizeof(long)),  NULL);

    /* read from FRONT-buffer to get channel stats info */
    glReadBuffer(GL_FRONT);
    
    /* read the display */
    glReadPixels((short)xorg, (short)yorg, (short)xsize, (short)ysize,
	 GL_RGBA, GL_UNSIGNED_BYTE, scrbuf);	

    /* write the data to the image file */
    if (alpha == 0)
    {
	ss = scrbuf;
	for (y=0; y<ysize; y++) 
	{
	    pfuCpackToRGB(ss, rs, gs, bs, xsize);

	    if(putrow(oimage, rs, (unsigned int)y, 0)!=xsize) 
		writeerr = 1;
	    if(putrow(oimage, gs, (unsigned int)y, 1)!=xsize) 
		writeerr = 1;
	    if(putrow(oimage, bs, (unsigned int)y, 2)!=xsize) 
		writeerr = 1;

	    ss += xsize;
	}
    }
    else
    {
	ss = scrbuf;
	for (y=0; y<ysize; y++) 
	{
	    pfuCpackToRGBA(ss, rs, gs, bs, as, xsize);

	    if(putrow(oimage, rs, (unsigned int)y, 0)!=xsize) 
		writeerr = 1;
	    if(putrow(oimage, gs, (unsigned int)y, 1)!=xsize) 
		writeerr = 1;
	    if(putrow(oimage, bs, (unsigned int)y, 2)!=xsize) 
		writeerr = 1;
	    if(putrow(oimage, as, (unsigned int)y, 3)!=xsize) 
		writeerr = 1;

	    ss += xsize;
	}
    }
    
    /* free buffers */
    pfFree(rs);
    pfFree(gs);
    pfFree(bs);
    if (alpha)
	pfFree(as);
    pfFree(scrbuf);
    
    /* close the image file */
    if(iclose(oimage)<0 || writeerr)
	return(-1);
    else
	return(0);
}
