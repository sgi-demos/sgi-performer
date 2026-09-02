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

#ifdef WIN32
#include <image.h>
#else
#include <gl/image.h>
#endif

unsigned short rbuf[8192]; 
unsigned short gbuf[8192]; 
unsigned short bbuf[8192]; 

unsigned short packed[8192];


static int
reverse_and_spread(int x, int n)     /* n is # of bits in x, half # in result */
{
    int i, X = 0;
    for (i = 0; i < n; ++i)
	X |= ((x >> (n-1-i))&1) << (2*i);
    return X;
}

/*
   Magic routine to fill a dither table of a given size.
   The first few are:

    logwidth=0	0

    logwidth=1  0 2
		3 1

    logwidth=2	 0  8  2 10
		12  4 14  6
		 3 11  1  9
		15  7 13  5
 */
static void
fill_dithertable(int logwidth, unsigned int *table)
{
    int i, j, width = 1 << logwidth;

    for (i = 0; i < width; ++i)
	for (j = 0; j < width; ++j)
	    table[i*width+j] = reverse_and_spread(i, logwidth)
		            | (reverse_and_spread(i^j, logwidth)<<1);
}

/* Use a 16x16 dither table */
#define LOG_DITHERWIDTH 4
#define DITHERWIDTH (1<<LOG_DITHERWIDTH)
#define DITHERSIZE (DITHERWIDTH*DITHERWIDTH)
#define DITHERMASK (DITHERWIDTH-1)

main(argc,argv)
int argc;
char **argv;
{
    IMAGE *image;
    FILE *output;
    int x, y, xsize, ysize;
    int z, zsize;
    int xoff = 0, yoff = 0; /* where to start in the dither table */
    unsigned int dithertable[DITHERWIDTH][DITHERWIDTH];

    if( argc != 3 && argc != 5 ) {
	fprintf(stderr,"usage: to5551 inimage.rgb outimage [<ditherStartX> <ditherStartY>]\n");
	exit(1);
    } 
    if( (image=iopen(argv[1],"r")) == NULL ) {
	fprintf(stderr,"to5551: can't open input file %s\n",argv[1]);
	exit(1);
    }
    if ((output = fopen(argv[2], "w")) == NULL) {
	fprintf(stderr,"to5551: can't open output file %s\n",argv[2]);
	exit(2);
    }
    if (argc > 3) xoff = atoi(argv[3]);
    if (argc > 3) yoff = atoi(argv[4]);
    xsize = image->xsize;
    ysize = image->ysize;
    zsize = image->zsize;
    if(zsize<3) {
	fprintf(stderr,"to5551: this is not an RGB image file\n");
	exit(1);
    }

    fill_dithertable(LOG_DITHERWIDTH, *dithertable);

    for(y=0; y<ysize; y++) {
	short r, g, b, a, i;

	getrow(image,rbuf,y,0);
	getrow(image,gbuf,y,1);
	getrow(image,bbuf,y,2);

	for (x=0; x<xsize; x++) {
	    i = dithertable[(y+yoff)&DITHERMASK][(x+xoff)&DITHERMASK];
            /*
             * reverse sense of dither for green...
             * this makes it so that green pixels will tend to alternate
             * with red pixels, producing a more uniform brightness
             * (less noticable checkerboard pattern).
             */
	    r = (int)(rbuf[x]*31/255.0f +   (float)(2*i+1)/(2*DITHERSIZE));
	    g = (int)(gbuf[x]*31/255.0f + 1-(float)(2*i+1)/(2*DITHERSIZE));
	    b = (int)(bbuf[x]*31/255.0f +   (float)(2*i+1)/(2*DITHERSIZE));

	    /* opaque */
	    a = 1; 

	    /* [r:15..11][g:10..6][b:5..1][a:0] */
	    packed[x] = (r << 11) | (g << 6) | (b << 1) | a;
	}
	fwrite(packed, sizeof(short), xsize, output);
    }
    fclose(output);
    exit(0);
}
