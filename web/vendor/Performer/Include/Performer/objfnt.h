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
 * objfnt.h
 *
 * $Revision: 1.8 $
 * $Date: 1995/12/02 00:23:09 $
 */

#ifndef OBJFNTDEF
#define OBJFNTDEF

typedef struct _texchardesc 
{
     float 	movex;         /* advance */
     int 	haveimage;
     float	llx, lly;      /* geometry box */
     float 	urx, ury;
     float 	tllx, tlly;    /* texture box */
     float 	turx, tury;
     float 	data[3*8];

} texchardesc;

typedef struct texfnt 
{
    short 	charmin, charmax;
    short 	nchars;
    float 	pixhigh;
    texchardesc *chars;
    short 	rasxsize,rasysize;
    ushort 	*rasdata;

} texfnt;

typedef struct chardesc 
{
    short 	movex, movey;	/* advance */
    short 	llx, lly;	/* bounding box */
    short 	urx, ury;
    short 	*data;		/* char data */
    int 	datalen;		

} chardesc;

typedef struct objfnt 
{
    struct objfnt *freeaddr;	/* if freeaddr != 0, objfnt is one chunck */
    short 	type;
    short 	charmin, charmax;
    short 	nchars;
    short 	scale;
    chardesc 	*chars;
} objfnt;

#ifdef N64
typedef struct
{
    uint	freeaddr;	/* if freeaddr != 0, objfnt is one chunck */
    short 	type;
    short 	charmin, charmax;
    short 	nchars;
    short 	scale;
    uint	chars;
} objfnt64;

typedef struct
{
    short 	movex, movey;	/* advance */
    short 	llx, lly;	/* bounding box */
    short 	urx, ury;
    uint 	data;		/* char data */
    int 	datalen;		

} chardesc64;
#endif

#define OFMAGIC		0x93339333

#define TM_TYPE		1
#define PO_TYPE		2
#define SP_TYPE		3

/* ops for tmesh characters */

#define	TM_BGNTMESH	(1)
#define	TM_SWAPTMESH	(2)
#define	TM_ENDBGNTMESH	(3)
#define	TM_RETENDTMESH	(4)
#define	TM_RET		(5)

/* ops for poly characters */

#define	PO_BGNLOOP	(1)
#define	PO_ENDBGNLOOP	(2)
#define	PO_RETENDLOOP	(3)
#define	PO_RET		(4)

/* ops for spline  characters */

#define	SP_MOVETO	(1)
#define	SP_LINETO	(2)
#define	SP_CURVETO	(3)
#define	SP_CLOSEPATH	(4)
#define	SP_RETCLOSEPATH	(5)
#define	SP_RET		(6)


#define MIN_ASCII 	' '
#define MAX_ASCII 	'~'
#define NASCII		(224)
#define NSYMBL		(224)
#define NZAPFD		(224)
#define NACCENT		(14)
#define ACCENTBASE	(400)

#define NOBBOX		(30000)

typedef struct pschar {
    char *name;
    int code;
    int prog;
} pschar;

extern pschar charlist[NASCII];
extern pschar scharlist[NSYMBL];
extern pschar zcharlist[NZAPFD];
extern pschar accentlist[NACCENT];

typedef struct mat2d {
    double a, b, c, d;
    double tx, ty;
} mat2d;

#ifdef __cplusplus
extern "C" {
#endif

extern void     fntpointmode(int size);
extern void     printobjfnt(objfnt* fnt);
extern short*   getcharprog(objfnt* fnt, int c);
extern chardesc*        getchardesc(objfnt* fnt, int c);
extern int      chartoindex(objfnt* fnt, int c);
extern int      printobjchar(objfnt* fnt, int c);
extern void     applytoobjfntverts(objfnt* fnt, void (*func)(short*));
extern void     applytocharverts(objfnt* fnt, int c, void (*func)(short*));
extern void     applytocharedges(objfnt* fnt, int c, void (*func)(short*,short*));
extern void     fontbbox(objfnt* fnt, int* llx, int* lly, int* urx, int* ury);
extern void     calcstringbbox(objfnt* fnt, unsigned char* str, int* llx, int* lly, int* urx, int* ury);
extern void     calccharbboxes(objfnt* fnt);
extern int      isobjfnt(char* name);
extern objfnt*  readobjfnt(char* name, void *arena);
extern void     writeobjfnt(char* name, objfnt* fnt);
extern objfnt*  newobjfnt(int type, int charmin, int charmax, int fscale);
extern void     freeobjfnt(objfnt* fnt);
extern void     addchardata(objfnt* fnt, int c, short* data, int nshorts);
extern void     addcharmetrics(objfnt* fnt, int c, int movex, int movey);
extern int      drawobjchar(objfnt* fnt, int c);
extern int      drawobjcharbbox(objfnt* fnt, int c);
extern int      getcharadvance(objfnt* fnt, int c, int* dx, int* dy);
extern void     fontsetsize(float size);
extern void     fontmoveto(float x, float y);
extern void     fontrmoveto(float x, float y);
extern void     fontptr(objfnt* fnt);
extern objfnt*  fontname(char* name);
extern void     fontpurge(char* name);
extern float    getstringwidth(objfnt* fnt, unsigned char* str);
extern float    fontstringwidth(unsigned char* str);
extern int      fontshow(unsigned char* str);
extern int      bboxshow(unsigned char* str);
extern char*    asciiname(int c);
extern int      hasvertdata(objfnt* fnt, int c);
extern void     fontcentershow(unsigned char* str);
extern void     fakechar(objfnt* fnt, int c, int width);
extern void     mergefont(objfnt* dest, objfnt* src);
extern int      charexists(objfnt* fnt, int c);

extern texfnt 	*readtexfnt(char *file, void *arena);
extern float 	texstrwidth(char *str);

extern objfnt 	*readtype1(char* infont, float beztolerance, int fullset);

#ifdef __cplusplus
}
#endif

#endif
