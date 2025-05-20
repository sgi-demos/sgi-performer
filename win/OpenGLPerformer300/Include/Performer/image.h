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
 *	image.h
 *
 *	$Revision: 1.9 $
 *	$Date: 2002/04/11 23:44:51 $
 */

#ifndef __GL_IMAGE__
#define __GL_IMAGE__

/*
 *	Defines for image files . . . .
 *
 *  			Paul Haeberli - 1984
 *
 *	Brutally hacked into a c++ wrapper by Dave Springer, 1988.
 *      reworked for C jimh 1991.
 *
 */

#include <stdio.h>

#ifdef __LOCAL_BUILD__
#include "pfDLL.h"
#else
#include <Performer/pfDLL.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned short	imagic;		/* stuff saved on disk . . */
    unsigned short 	type;
    unsigned short 	dim;
    unsigned short 	xsize;
    unsigned short 	ysize;
    unsigned short 	zsize;
    unsigned long 	min;
    unsigned long 	max;
    unsigned long	wastebytes;	
    char 		name[80];
    unsigned long	colormap;

    long 		file;		/* stuff used in core only */
    unsigned short 	flags;
    short		dorev;
    short		x;
    short		y;
    short		z;
    short		cnt;
    unsigned short	*ptr;
    unsigned short	*base;
    unsigned short	*tmpbuf;
    unsigned long	offset;
    unsigned long	rleend;		/* for rle images */
    unsigned long	*rowstart;	/* for rle images */
    long		*rowsize;	/* for rle images */
} IMAGE;


typedef struct {
	unsigned short 	type;
    	unsigned long colormap;
	int xsize, ysize;
	unsigned long *rowstart;
	long	*rowsize;
	short *pixels;     
} MEMIMAGE ;

#define IMAGIC 	0732

/* type of the image */
#define CM_NORMAL		0
#define CM_DITHERED		1
#define CM_SCREEN		2
#define CM_COLORMAP		3

#define TYPEMASK	0xff00
#define BPPMASK		0x00ff
#define ITYPE_VERBATIM	0x0000
#define ITYPE_RLE	0x0100
#define ISRLE(type)		(((type) & 0xff00) == ITYPE_RLE)
#define ISVERBATIM(type)	(((type) & 0xff00) == ITYPE_VERBATIM)
#define BPP(type)		((type) & BPPMASK)
#define RLE(bpp)		(ITYPE_RLE | (bpp))
#define VERBATIM(bpp)		(ITYPE_VERBATIM | (bpp))
#define	IBUFSIZE(pixels)	((pixels+(pixels>>6))<<2)
#define	RLE_NOP		0x00

#define	ierror(p)	(((p)->flags&_IOERR)!=0)
#define	ifileno(p)	((p)->file)

#ifdef PIXMACROS
#define	getpix(p)	(--(p)->cnt>=0? *(p)->ptr++:ifilbuf(p))
#define putpix(p,x) (--(p)->cnt>=0? ((int)(*(p)->ptr++=(unsigned)(x))):iflsbuf(p,(unsigned)(x)))
#endif

extern DLLEXPORT int	iclose(IMAGE* image);
extern DLLEXPORT int	iflush(IMAGE* image);
extern DLLEXPORT int	ifilbuf(IMAGE* image);
extern DLLEXPORT int	iflsbuf(IMAGE* image, unsigned long c);
extern DLLEXPORT MEMIMAGE	*newimage(int xsize, int ysize, int colormap);
extern DLLEXPORT void	freeimage(MEMIMAGE* mimage);
extern DLLEXPORT void	drawimage(int xorg, int yorg, MEMIMAGE* mimage);
extern DLLEXPORT MEMIMAGE	*readimage(char* name);
extern DLLEXPORT MEMIMAGE	*readrleimage(char* name);
extern DLLEXPORT void	img_transtoscreen(unsigned short* buf, int n);
extern DLLEXPORT void	img_setpixelortho(void);
extern DLLEXPORT void	img_makexmap(int colormap);
extern DLLEXPORT void	isetname(IMAGE* image, char* name);
extern DLLEXPORT void	isetcolormap(IMAGE* image, int colormap);
extern DLLEXPORT IMAGE	*iopen(char* file, char* mode, ...);
/*
extern IMAGE	*iopen(char* file, char* mode, unsigned int type,
		       unsigned int dim, unsigned int xsize,
		       unsigned int ysize, unsigned int zsize);
*/
extern DLLEXPORT IMAGE	*fiopen(int f, char* mode, unsigned int type,
			unsigned int dim, unsigned int xsize,
			unsigned int ysize, unsigned int zsize);
extern DLLEXPORT IMAGE	*imgopen(int f, char* file, char* mode, unsigned int type,
			 unsigned int dim, unsigned int xsize,
			 unsigned int ysize, unsigned int zsize);
extern DLLEXPORT unsigned	short* ibufalloc(IMAGE* image);
extern DLLEXPORT int	reverse(unsigned long lwrd);
extern DLLEXPORT void	cvtshorts(unsigned short* buffer, long n);
extern DLLEXPORT void	cvtlongs(long* buffer, long n);
extern DLLEXPORT void	cvtimage(long* buffer);
extern DLLEXPORT int	getpix(IMAGE* image);
extern DLLEXPORT int	putpix(register IMAGE* image, unsigned long pix);
extern DLLEXPORT void	img_seek(IMAGE* image, unsigned int y, unsigned int z);
extern DLLEXPORT int	img_write(IMAGE* image, char* buffer, long count);
extern DLLEXPORT int	img_read(IMAGE* image, char* buffer, long count);
extern DLLEXPORT unsigned	long img_optseek(IMAGE* image, unsigned long offset);
extern DLLEXPORT int	img_getrowsize(register IMAGE* image);
extern DLLEXPORT void	img_setrowsize(IMAGE* image, long cnt, long y, long z);
extern DLLEXPORT int	img_rle_compact(unsigned short* expbuf, int ibpp,
				unsigned short* rlebuf, int obpp, int cnt);
extern DLLEXPORT void	img_rle_expand(unsigned short* rlebuf, int ibpp,
			       unsigned short* expbuf, int obpp);
extern DLLEXPORT int	putrow(IMAGE* image, unsigned short* buffer,
		       unsigned int y, unsigned int z);
extern DLLEXPORT int	getrow(IMAGE* image, unsigned short* buffer, unsigned int y,
		       unsigned int z);

#ifdef __cplusplus
}
#endif

#endif /* _GL_IMAGE */
