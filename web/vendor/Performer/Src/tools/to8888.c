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

/*
 *	readimg - 
 *		Read an RGB image file but don't do anything with it!
 *
 *				Paul Haeberli - 1984
 *
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

main(argc,argv)
int argc;
char **argv;
{
    IMAGE *image;
    FILE *output;
    int x, y, xsize, ysize;
    int z, zsize;

    if( argc<3 ) {
	fprintf(stderr,"usage: to5551 inimage.rgb outimage\n");
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
    xsize = image->xsize;
    ysize = image->ysize;
    zsize = image->zsize;
    if(zsize<3) {
	fprintf(stderr,"to5551: this is not an RGB image file\n");
	exit(1);
    }
    for(y=0; y<ysize; y++) {
	short r, g, b, a;

	getrow(image,rbuf,y,0);
	getrow(image,gbuf,y,1);
	getrow(image,bbuf,y,2);

	for (x=0; x<xsize; x++) {
	    /* extract top 5 bits of each component */
	    packed[x*3+0] = (char)(rbuf[x] & 0xff);
	    packed[x*3+1] = (char)(gbuf[x] & 0xff);
	    packed[x*3+2] = (char)(bbuf[x] & 0xff);
	}
	fwrite(packed, 3, xsize, output);
    }
    fclose(output);
    exit(0);
}
