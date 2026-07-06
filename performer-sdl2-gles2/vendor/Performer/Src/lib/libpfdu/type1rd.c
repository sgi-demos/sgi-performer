/*
 * Copyright 1995, Silicon Graphics, Inc.
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
 *
 *	type1rd -
 *		Read an Adobe type 1 font into objfnt data structure.
 *
 *				Paul Haeberli - 1990
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <Performer/pr.h>
#endif
#include <Performer/objfnt.h>

#ifdef WIN32
#define READ_MODE "rb"
#define WRITE_MODE "wb"
#else
#define READ_MODE "r"
#define WRITE_MODE "w"
#endif

/*
 *	The rest of this file is could be put into a library!
 *
*/
static void runprog();

#define LINELEN 	(2048)
#define SKIP 		(4)
#define NOTHEX		(100)
#define MAXSUBRS 	(1000)
#define MAXCHARS 	(1000)
#define MAXTRIES 	(30)

#define mymalloc	malloc

static int debugtype1;

static void applymat(float mat[2][2], double* x, double* y);
static void poly_width(int x);
static void poly_beginchar(void);
static void poly_endchar(void);
static void poly_close(void);
static void poly_pnt(double x, double y);
static void spline_width(int x);
static void spline_beginchar(void);
static void spline_endchar(void);
static void spline_close(void);
static void spline_line(double x0, double y0, double x1, double y1);
static void spline_curveto(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3);
static void getmove(int* x, int* y);
static void getpos(int* x, int* y);
static void savestart(int x, int y);
static void sbpoint(int x, int y);
static void moveto(int x, int y);
static void rmoveto(int x, int y);
static void drawline(float x0, float y0, float x1, float y1, float dx0, float dy0, float dx1, float dy1);
static void rlineto(int x, int y);
static void closepath(void);
static void bezadapt(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float beztol);
static void drawbez(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);
static void rcurveto(int dx1, int dy1, int dx2, int dy2, int dx3, int dy3);
static void readfontmatrix(char* name, float mat[2][2]);
static void resetdecrypt(int n);
static void decryptdata(unsigned char* iptr, unsigned char* optr, int n);
static int decryptprogram(unsigned char* buf, int len);
static void decryptall(void);
static unsigned char *decodetype1(char* inname);
static void fakefopen(void);
static void fakegettoken(char* str);
static int fakefgets(char* buf, int max);
static unsigned char *fakefread(int n);
static void setcharlist(void);
static void initpcstack(void);
static void pushpc(unsigned char* pc);
static unsigned char *poppc(void);
static void initstack(void);
static void push(int val);
static int pop(void);
static void initretstack(void);
static void retpush(int val);
static int retpop(void);
static void subr0(void);
static void subr1(void);
static void subr2(void);
static void drawchar(int c);
static void drawchar_seac(int c, int accentflag);
static void seac(int asb, int adx, int ady, int bchar, int achar);
static int docommand(int cmd);
static void runprog(void);
static objfnt *genobjfont(int savesplines, int fullset);
static int sizeoffile(FILE *f);

/*------------------------------------------------------------------------*/

static int sizeoffile(FILE *f) 
{
    struct stat statbuf;
    
    if(fstat(fileno(f),&statbuf)) {
        fprintf(stderr,"sizeoffile: stat failed\n");
        return 0;
    }
    return statbuf.st_size;
}

void setdebugtype1(int val) 
{
    debugtype1 = val;
}


/* 
 *	font data globals
 *
 */
static char oneline[LINELEN];
static char fname[100];
static float beztol;
static unsigned char *subrs[MAXSUBRS];
static unsigned int sublen[MAXSUBRS];
static unsigned char *chars[MAXCHARS];
static unsigned int charlen[MAXCHARS];
static char *charname[MAXCHARS];
static int nsubrs, nchars;
static char tok[LINELEN];
static unsigned char hextab[256];
static int firsted;

static unsigned char *bindat;
static int datbytes;
static int fakepos;
static int fakemax;

/* 
 *	interpreter globals
 *
 */
static float mat[2][2];
static unsigned char *pcstack[100];
static unsigned char *pc;
static int pcsp;
static int coordpos;
static int coordsave[7][2];
static int incusp;
static int retstack[1000];
static int retsp;
static int stack[1000];
static int sp;

/*
 *	interpreter geometry
 *
 */
static int x_offset, y_offset;	/* offsets for accents */
static int thecharwidth;
static int nvertpos;
static int startx, starty;
static int curx, cury;
static int nextx, nexty;
static int delx, dely;
static int started;

/*
 * generic routines 
 *
 */
static void applymat(float mat[2][2], double *x, double *y)	
{ 					/* use doubles don't change!! */
    float tx, ty;

    tx = (*x)*mat[0][0]+(*y)*mat[0][1];
    ty = (*x)*mat[1][0]+(*y)*mat[1][1];
    *x = tx;
    *y = ty;
}

/*
 * poly output stuff 
 *
 */
static int npnts, nloops;
static short chardata[2000];
static int nshorts;

static void poly_width(int x)
{
    double fx, fy;

    fx = x;
    fy = 0.0;
    applymat(mat,&fx,&fy);
    thecharwidth = (int)(fx + 0.5);
}

static void poly_beginchar(void)
{
    npnts = 0;
    nloops = 0;
}

static void poly_endchar(void)
{
    if(nloops == 0)
	chardata[nshorts++] = PO_RET;
    else
	chardata[nshorts++] = PO_RETENDLOOP;
}

static void poly_close(void)
{
    if(npnts) {
	chardata[nvertpos] = npnts;
	npnts = 0;
    }
}

static void poly_pnt(double x, double y)
{
    int ix, iy;

    applymat(mat,&x,&y);
    ix = floor(x);
    iy = floor(y);
    ix += x_offset;
    iy += y_offset;
    if(npnts == 0) {
	if(nloops == 0)
	    chardata[nshorts++] = PO_BGNLOOP;
	else 
	    chardata[nshorts++] = PO_ENDBGNLOOP;
	nvertpos = nshorts;
	chardata[nshorts++] = 0;
	nloops++;
    }
    chardata[nshorts++] = ix;
    chardata[nshorts++] = iy;
    npnts++;
}

/*
 * spline output stuff 
 *
 */
static int sp_npnts, sp_nloops;
static short sp_chardata[2000];
static int sp_nshorts;

static void spline_width(int x)
{
    double fx, fy;

    fx = x;
    fy = 0.0;
    applymat(mat,&fx,&fy);
    thecharwidth = (int)(fx + 0.5);
}

static void spline_beginchar(void)
{
    sp_npnts = 0;
    sp_nloops = 0;
}

static void spline_endchar(void)
{
    if(sp_nloops == 0)
	sp_chardata[sp_nshorts++] = SP_RET;
    else
	sp_chardata[sp_nshorts++] = SP_RETCLOSEPATH;
}

static void spline_close(void)
{
    sp_chardata[sp_nshorts++] = SP_CLOSEPATH;
    sp_npnts = 0;
}

static void spline_line(double x0,double y0,double x1,double y1)
{
    applymat(mat,&x0,&y0);
    applymat(mat,&x1,&y1);
    if(sp_npnts == 0) {
	sp_chardata[sp_nshorts++] = SP_MOVETO;
	sp_chardata[sp_nshorts++] = floor(x0);
	sp_chardata[sp_nshorts++] = floor(y0);
	sp_npnts++;
	sp_nloops++;
    }
    sp_chardata[sp_nshorts++] = SP_LINETO;
    sp_chardata[sp_nshorts++] = floor(x1);
    sp_chardata[sp_nshorts++] = floor(y1);
    sp_npnts++;
}

static void spline_curveto(double x0,double y0,double x1,double y1,
				  double x2,double y2,double x3,double y3)
{
    applymat(mat,&x0,&y0);
    applymat(mat,&x1,&y1);
    applymat(mat,&x2,&y2);
    applymat(mat,&x3,&y3);
    if(sp_npnts == 0) {
	sp_chardata[sp_nshorts++] = SP_MOVETO;
	sp_chardata[sp_nshorts++] = floor(x0);
	sp_chardata[sp_nshorts++] = floor(y0);
	sp_npnts++;
	sp_nloops++;
    }
    sp_chardata[sp_nshorts++] = SP_CURVETO;
    sp_chardata[sp_nshorts++] = floor(x1);
    sp_chardata[sp_nshorts++] = floor(y1);
    sp_chardata[sp_nshorts++] = floor(x2);
    sp_chardata[sp_nshorts++] = floor(y2);
    sp_chardata[sp_nshorts++] = floor(x3);
    sp_chardata[sp_nshorts++] = floor(y3);
}

/* 
 *    graphics follows 
 *
 *
 */
static void getmove(int *x,int *y)
{
    *x = delx;
    *y = dely;
}

static void getpos(int *x,int *y)
{
    *x = curx;
    *y = cury;
}

static void savestart(int x,int y)
{
    startx = x;
    starty = y;
    started = 1;
}

static void sbpoint(int x,int y)
{
    curx = x;
    cury = y;
}

#ifdef NOTDEF
static void moveto(int x,int y)
{
    curx = x;
    cury = y;
    savestart(curx,cury);
}
#endif

static void rmoveto(int x,int y)
{
    if(incusp) {
	delx = x;
	dely = y;
    } else {
	curx += x;
	cury += y;
	savestart(curx,cury);
    }
}

static void drawline(float x0,float y0,float x1,float y1,
			   float dx0,float dy0,float dx1,float dy1)
{
    if(x0!=x1 || y0!=y1) 
	poly_pnt(x1,y1);
}

static void rlineto(int x,int y)
{
    float dx, dy;

    nextx = curx + x;
    nexty = cury + y;
    dx = nextx-curx;
    dy = nexty-cury;
    drawline((float)curx,(float)cury,(float)nextx,(float)nexty,dx,dy,dx,dy);
    spline_line((float)curx,(float)cury,(float)nextx,(float)nexty);
    curx = nextx;
    cury = nexty;
}

static void closepath(void)
{
    float dx, dy;

    if(started) {
	dx = startx-curx;
	dy = starty-cury;
	drawline((float)curx,(float)cury,(float)startx,(float)starty,dx,dy,dx,dy);
	poly_close();
	spline_close();
	started = 0;
    }
}

static void bezadapt(float x0,float y0,float x1,float y1,
			   float x2,float y2,float x3,float y3,float beztol)
{
    float ax0,ay0,ax1,ay1,ax2,ay2,ax3,ay3;
    float bx0,by0,bx1,by1,bx2,by2,bx3,by3;
    float midx, midy;
    float linx, liny, dx, dy, mag;
   
    midx = (x0+3*x1+3*x2+x3)/8.0;
    midy = (y0+3*y1+3*y2+y3)/8.0;
    linx = (x0+x3)/2.0;
    liny = (y0+y3)/2.0;
    dx = midx-linx;
    dy = midy-liny;
    mag = dx*dx+dy*dy;
    if(mag<(beztol*beztol))
	drawline(x0,y0,x3,y3,x1-x0,y1-y0,x3-x2,y3-y2);
    else {
	ax0 = x0;
	ay0 = y0;
	ax1 = (x0+x1)/2;
	ay1 = (y0+y1)/2;
	ax2 = (x0+2*x1+x2)/4;
	ay2 = (y0+2*y1+y2)/4;
	ax3 = midx;
	ay3 = midy;
	bezadapt(ax0,ay0,ax1,ay1,ax2,ay2,ax3,ay3,beztol);

	bx0 = midx;
	by0 = midy;
	bx1 = (x1+2*x2+x3)/4;
	by1 = (y1+2*y2+y3)/4;
	bx2 = (x2+x3)/2;
	by2 = (y2+y3)/2;
	bx3 = x3;
	by3 = y3;
	bezadapt(bx0,by0,bx1,by1,bx2,by2,bx3,by3,beztol);
    }
}

static void drawbez(float x0,float y0,float x1,float y1,
				  float x2,float y2,float x3,float y3)
{
    bezadapt(x0,y0,x1,y1,x2,y2,x3,y3,beztol);
}

static void rcurveto(int dx1,int dy1,int dx2,int dy2,int dx3,int dy3)
{
    int x0, y0;
    int x1, y1;
    int x2, y2;
    int x3, y3;

    x0 = curx;
    y0 = cury;
    x1 = curx+dx1;
    y1 = cury+dy1;
    x2 = curx+dx2;
    y2 = cury+dy2;
    x3 = curx+dx3;
    y3 = cury+dy3;
    drawbez((float)x0,(float)y0,(float)x1,(float)y1,(float)x2,(float)y2,(float)x3,(float)y3);
    spline_curveto((float)x0,(float)y0,(float)x1,(float)y1,
				       (float)x2,(float)y2,(float)x3,(float)y3);
    curx = x3;
    cury = y3;
}

/*
 *	read the font matrix out of the font file
 *	update global fname with the actual font name.
 *
 */
static void readfontmatrix(char *name, float mat[2][2])
{
    FILE *inf;
    char *cptr;
    float a, b, c, d, e, f;

/* open the input file */
    inf = fopen(name,READ_MODE);
    if(!inf) {
	fprintf(stderr,"fromtype1: can't open input file %s\n",name);
	exit(1);
    }

/* look for the FontName and FontMatrix def */
    while(1) {
	if(!fgets(oneline,LINELEN,inf)) {
	    fprintf(stderr,"fromtype1: no FontMatrix found\n");
	    exit(1);
	}
	cptr = strchr(oneline,'/');
	if(cptr) {
            if(strncmp(cptr,"/FontName",9) == 0) {
                cptr = strchr(cptr+1,'/');
                if(!cptr) {
                    fprintf(stderr,"fromtype1: bad FontName line\n");
                    exit(1);
                }
                sscanf(cptr+1,"%s\n", fname);
            } else {
	        if(strncmp(cptr,"/FontMatrix",11) == 0) {
		    cptr = strchr(cptr,'[');
		    if(!cptr) {
		        fprintf(stderr,"fromtype1: bad FontMatrix line\n");
		        exit(1);
		    }
		    sscanf(cptr+1,"%f %f %f %f %f %f\n",&a,&b,&c,&d,&e,&f);
#ifndef __linux__
                    if (*fname != NULL)
		        break;
#else
                    if (*fname != 0)
		        break;
#endif
	        }
            }
	}
    }
#ifndef __linux__
    if (*fname == NULL) {
        fprintf(stderr,"fromtype1: bad FontName\n");
        exit(1);
    }
#else
    if (*fname == 0) {
        fprintf(stderr,"fromtype1: bad FontName\n");
        exit(1);
    }
#endif
    fclose(inf);
    mat[0][0] = 1000.0*a; 
    mat[0][1] = 1000.0*c; 
    mat[1][0] = 1000.0*b; 
    mat[1][1] = 1000.0*d; 
}

/*
 *	Decryption support
 *
 */
static unsigned short mr;

static void resetdecrypt(int n)
{
    mr = n;
}

static void decryptdata(unsigned char *iptr,unsigned char *optr,int n)
{
    unsigned char in;
    unsigned short mymr;
    unsigned short c1;
    unsigned short c2;

    mymr = mr;
    c1 = 52845;
    c2 = 22719;
    while(n>8) {
	in = iptr[0];
	optr[0] = (in^(mymr>>8));
	mymr = (in+mymr)*c1 + c2;
	in = iptr[1];
	optr[1] = (in^(mymr>>8));
	mymr = (in+mymr)*c1 + c2;
	in = iptr[2];
	optr[2] = (in^(mymr>>8));
	mymr = (in+mymr)*c1 + c2;
	in = iptr[3];
	optr[3] = (in^(mymr>>8));
	mymr = (in+mymr)*c1 + c2;
	in = iptr[4];
	optr[4] = (in^(mymr>>8));
	mymr = (in+mymr)*c1 + c2;
	in = iptr[5];
	optr[5] = (in^(mymr>>8));
	mymr = (in+mymr)*c1 + c2;
	in = iptr[6];
	optr[6] = (in^(mymr>>8));
	mymr = (in+mymr)*c1 + c2;
	in = iptr[7];
	optr[7] = (in^(mymr>>8));
	mymr = (in+mymr)*c1 + c2;
	n-=8;
	optr+=8;
	iptr+=8;
    }
    while(n--) {
	in = iptr[0];
	optr[0] = (in^(mymr>>8));
	mymr = (in+mymr)*c1 + c2;
	optr++;
	iptr++;
    }
    mr = mymr;
}

static int decryptprogram(unsigned char *buf,int len)
{
    resetdecrypt(4330);
    if(len<4) {
	printf("bad char program\n");
	exit(1);
    }
    decryptdata(buf,buf,SKIP);
    decryptdata(buf+SKIP,buf,len-SKIP);
    return len-SKIP;
}

static void decryptall(void)
{
    int i;

    for(i=0; i<nsubrs; i++)
	sublen[i] = decryptprogram(subrs[i],sublen[i]);
    for(i=0; i<nchars; i++) 
	charlen[i] = decryptprogram(chars[i],charlen[i]);
#ifdef FINDIT
    for(i=0; i<nchars; i++) 
	printchardata(i);
#endif
}

/*
 *	decode the eexec part of the file
 *
 */
static unsigned char *decodetype1(char *inname)
{
    FILE *inf, *outf;
    unsigned char *hptr, *bptr;
    int i, totlen, hexbytes, c;
    unsigned char *hexdat;
    unsigned char *decdat;

/* make hex table */
    if(!firsted) {
	for(i=0; i<256; i++) {
	    if(i>='0' && i<='9')
		 hextab[i] = i-'0';
	    else if(i>='a' && i<='f')
		 hextab[i] = 10+i-'a';
	    else if(i>='A' && i<='F')
		 hextab[i] = 10+i-'A';
	    else
		 hextab[i] = NOTHEX;
	}
    }

/* open input file */
    inf = fopen(inname, READ_MODE);
    if(!inf) {
	fprintf(stderr,"fromtype1: can't open input file\n");
	exit(1);
    }

/* allocate buffers */
    totlen = sizeoffile(inf);
    hexdat = (unsigned char *)mymalloc(totlen);
    decdat = (unsigned char *)mymalloc(totlen/2);

/* look for eexec part of file */
    while(1) {
	if(!fgets(oneline,LINELEN,inf)) {
	    fprintf(stderr,"fromtype1: no currentfile eexec found\n");
	    exit(1);
	}
	oneline[16] = 0;
	if(strcmp(oneline,"currentfile eexe") == 0) 
	    break;
    }

/* read all the hex bytes into the hex buffer */
    hexbytes = 0;
    while(fgets(oneline,LINELEN,inf)) {
	hptr = (unsigned char *)oneline;
	while(*hptr) {
	    if(hextab[*hptr] != NOTHEX)
		hexdat[hexbytes++] = *hptr;
	    hptr++;
	}
    }
    fclose(inf);

/* check number of hex bytes */
    if(hexbytes & 1) 
	hexbytes--;
    datbytes = hexbytes/2;

/* translate hex data to binary */
    hptr = hexdat;
    bptr = decdat;
    c = datbytes;
    while(c--) {
	*bptr++  = (hextab[hptr[0]]<<4)+hextab[hptr[1]];
	hptr += 2;
    }

/* decrypt the data */
    resetdecrypt(55665);
    decryptdata(decdat,decdat,datbytes);

    if(debugtype1) {
	outf = fopen("debug.bin",WRITE_MODE);
	fwrite(decdat+4,datbytes-4,1,outf);
	fclose(outf);

    }
    free(hexdat);
    return decdat;
}

/* 
 *	fake file reading funcs
 *
 *
 */
static void fakefopen(void)
{
    fakepos = 0;
    fakemax = datbytes;
}

#define NL	(0x0a)
#define CR	(0x0d)

static void fakegettoken(char *str)
{
    int c;
    unsigned char *cptr;

    cptr = bindat+fakepos;
    c = *cptr++;
    fakepos++;
    if(c != NL && c != CR) {
	while(isspace(c)) {
	    c = *cptr++;
	    fakepos++;
	}
	while (fakepos<fakemax && !isspace(c)) {
	    *str++ = c;
	    c = *cptr++;
	    fakepos++;
	}
	if(c == NL || c == CR)
	    fakepos--;
    }
    *str = 0;
    if(fakepos>fakemax) {
	fprintf(stderr,"fromtype1: unexpected eof\n");
	exit(1);
    }
}

static int fakefgets(char *buf,int max)
{
    unsigned char *cptr;

    cptr = (unsigned char *)(bindat+fakepos);
    while(max--) {
	*buf++ = *cptr;
	fakepos++;
	if(*cptr == NL || *cptr == CR)
	    return 1;
	cptr++;
	if(fakepos>fakemax)
	    return 0;
    }
    return 0;
}

static unsigned char *fakefread(int n)
{
    fakepos += n;
    return bindat+fakepos-n;
}

static void setcharlist(void)
{
    char *name;
    int i, j;

    if(strncmp(fname,"Symbol",6) == 0) {
        for(i=0; i<NSYMBL; i++) 
    	scharlist[i].prog = -1;
        for(i=0; i<NSYMBL-1; i++) {
    	    name = scharlist[i].name;
    	    if(name) {
    	        for(j=0; j<nchars; j++) {
    		    if(charname[j]&&(strcmp(name,charname[j]) == 0))
    		        scharlist[i].prog = j;
    	        }
    	    }
        }
    } else {
        if(strncmp(fname,"ZapfDingbats",12) == 0) {
            for(i=0; i<NZAPFD; i++) 
        	zcharlist[i].prog = -1;
            for(i=0; i<NZAPFD-1; i++) {
                name = zcharlist[i].name;
                if(name) {
                    for(j=0; j<nchars; j++) {
                        if(charname[j]&&(strcmp(name,charname[j]) == 0))
                            zcharlist[i].prog = j;
                    }
                }
            }
        } else {
            for(i=0; i < ACCENTBASE + NACCENT; i++) 
        	charlist[i].prog = -1;
            for(i=0; i < NASCII; i++) {
                name = charlist[i].name;
                if(name) {
                    for(j=0; j<nchars; j++) {
                        if(charname[j]&&(strcmp(name,charname[j]) == 0))
			    charlist[i].prog = j;
                    }
                }
            }
            for(i=0; i < NACCENT; i++) {
                name = accentlist[i].name;
                if(name) {
                    for(j=0; j<nchars; j++) {
                        if(charname[j]&&(strcmp(name,charname[j]) == 0)) {
#ifdef DEBUGGING
			    fprintf(stderr, "Found accent %s, assiging id %d\n",
				name, i + ACCENTBASE);
#endif
			    charlist[i + ACCENTBASE].prog = j;
			}
                    }
                }
            }
        }
    }
}

/*
 *	pc stack support
 *
 */
static void initpcstack(void)
{
   pcsp = 0;
}

static void pushpc(unsigned char *pc)
{
   pcstack[pcsp] = pc;
   pcsp++;
}

static unsigned char *poppc(void)
{
   pcsp--;
   if(pcsp<0) {
     	fprintf(stderr,"\nYUCK: pc stack under flow\n"); 
	pcsp = 0;
	return 0;
   }
   return pcstack[pcsp];
}

/*
 *	Data stack support
 *
 */
static void initstack(void)
{
   sp = 0;
}

static void push(int val)
{
   stack[sp] = val;
   sp++;
}

static int pop(void)
{
   sp--;
   if(sp<0) {
     	fprintf(stderr,"\nYUCK: stack under flow\n"); 
	sp = 0;
	return 0;
   }
   return stack[sp];
}

/*
 *	call/return data stack
 *
 */
static void initretstack(void)
{
   retsp = 0;
}

static void retpush(int val)
{
   retstack[retsp] = val;
   retsp++;
}

static int retpop(void)
{
   retsp--;
   if(retsp<0) {
     	fprintf(stderr,"\nYUCK: ret stack under flow\n"); 
	retsp = 0;
	return 0;
   }
   return retstack[retsp];
}

/*
 * the three subrs
 *
 */
static void subr0(void)
{
    int x0, y0;
    int x1, y1;
    int x2, y2;
    int x3, y3;
    int xpos, ypos, noise;

    ypos = pop();
    xpos = pop();
    noise = pop();
    if(coordpos!=7) {
	fprintf(stderr,"subr0: bad poop\n");
	exit(1);
    }
    x0 =  coordsave[0][0];
    y0 =  coordsave[0][1];

    x1 =  coordsave[1][0]+x0;
    y1 =  coordsave[1][1]+y0;
    x2 =  coordsave[2][0];
    y2 =  coordsave[2][1];
    x3 =  coordsave[3][0];
    y3 =  coordsave[3][1];
    rcurveto(x1,y1,x1+x2,y1+y2,x1+x2+x3,y1+y2+y3);
    x1 =  coordsave[4][0];
    y1 =  coordsave[4][1];
    x2 =  coordsave[5][0];
    y2 =  coordsave[5][1];
    x3 =  coordsave[6][0];
    y3 =  coordsave[6][1];
    rcurveto(x1,y1,x1+x2,y1+y2,x1+x2+x3,y1+y2+y3);
    getpos(&x0,&y0);
    retpush(y0);
    retpush(x0);
    incusp = 0;
}

static void subr1(void)
{
    coordpos = 0;
    incusp = 1;
}

static void subr2(void)
{
    int x, y;

    getmove(&x,&y);
    if(coordpos>=7) {
	fprintf(stderr,"subr2: bad poop\n");
	exit(1);
    }
    coordsave[coordpos][0] = x;
    coordsave[coordpos][1] = y;
    coordpos++;
}

#ifndef __linux__
void puthexdata(outf,buf,n)
FILE *outf;
unsigned char *buf;
int n;
#else
void puthexdata(FILE *outf, unsigned char *buf, int n)
#endif
{
    while(n--) 
	 fprintf(outf,"%02x",*buf++);
     fprintf(outf,"\n");
}

/*
 * run the character program	
 *
 *
 */
static void drawchar(int c)
{
#ifdef DEBUGGING
fprintf(stderr,"\n\nDrawing char %d: name: [%s]\n",c,charname[c]);
fprintf(stderr,"N bytes %d\n",charlen[c]);
puthexdata(stderr,chars[c],charlen[c]);
fprintf(stderr,"\n\n");

#endif
    x_offset = y_offset = 0;
    poly_beginchar();
    spline_beginchar();
    initstack();
    initpcstack();
    initretstack();
    pc = chars[c];
    runprog();
    poly_endchar();
    spline_endchar();
}

static void drawchar_seac(int c, int accentflag)
{
    if(!accentflag) {
	poly_beginchar();
	spline_beginchar();
    }
    initstack();
    initpcstack();
    initretstack();
    pc = chars[c];
    runprog();
    if(accentflag) {
	poly_endchar();
	spline_endchar();
    }
}

static void seac(int asb, int adx, int ady, int bchar, int achar)
{
    int	i, j;

    for(i = 0; i < NACCENT; i++) {
	if(achar == accentlist[i].code) {
#ifdef DEBUGGING
	    fprintf(stderr, "\nin seac, drawing char %03o and %03o\n",
		bchar, achar);
#endif
	    x_offset = adx;
	    y_offset = ady;
	    drawchar_seac(charlist[ACCENTBASE + i].prog, 0);

	    x_offset = 0;
	    y_offset = 0;
	    for(j = 0; j < NACCENT; j++) {	/* look through accents first */
		if(bchar == accentlist[j].code) {
		    drawchar_seac(charlist[ACCENTBASE + j].prog, 1);
		    break;
		}
	    }

	    if(j == NACCENT) {
		for(j = 0; j < NASCII; j++) {
		    if(bchar == charlist[j].code) {
			drawchar_seac(charlist[j].prog, 1);
			break;
		    }
		}
	    }
	    break;
	}
    }
    if(i == NACCENT) {
#ifdef DEBUGGING
	fprintf(stderr, "\nin seac, drawing char %03o\n", bchar);
#endif
	drawchar(achar);
    }
}

/*
 *	execute the program:
 *
 *
 */
#define HSTEM		(1)
#define VSTEM		(3)
#define VMOVETO		(4)
#define RLINETO		(5)
#define HLINETO		(6)
#define VLINETO		(7)
#define RRCURVETO	(8)
#define CLOSEPATH	(9)
#define CALLSUBR	(10)
#define RETURN		(11)
#define HSBW		(13)
#define ENDCHAR		(14)
#define UNDOC15		(15)	/* undocumented, but used by Adobe */
#define RMOVETO		(21)
#define HMOVETO		(22)
#define VHCURVETO	(30)
#define HVCURVETO	(31)
#define DOTSECTION	(256+0)
#define VSTEM3		(256+1)
#define HSTEM3		(256+2)
#define SEAC		(256+6)
#define SBW		(256+7)
#define DIV		(256+12)
#define CALLOTHERSUBR	(256+16)
#define POP		(256+17)
#define SETCURRENTPOINT	(256+33)
#define WHAT0		(0)

static int docommand(int cmd)
{
    int x, y, w;
    int dx1, dy1;
    int dx2, dy2;
    int dx3, dy3;
    int i, sub, n;
    int	achar, bchar, adx, ady, asb;
    unsigned char *subpc;

    switch(cmd) {
	case WHAT0:
	    fprintf(stderr,"\nYUCK: WHAT0\n"); 
	    break;
	case HSTEM:
	    pop();
	    pop();
	    break;
	case VSTEM:		
	    pop();
	    pop();
	    break;
	case VMOVETO:
	    y = pop();
	    rmoveto(0,y);
	    break;
	case RLINETO:
	    y = pop();
	    x = pop();
	    rlineto(x,y);
	    break;
	case HLINETO:
	    x = pop();
	    rlineto(x,0);
	    break;
	case VLINETO:
	    y = pop();
	    rlineto(0,y);
	    break;
	case RRCURVETO:
	    dy3 = pop();
	    dx3 = pop();
	    dy2 = pop();
	    dx2 = pop();
	    dy1 = pop();
	    dx1 = pop();
	    rcurveto(dx1,dy1,dx1+dx2,dy1+dy2,dx1+dx2+dx3,dy1+dy2+dy3);
	    break;
	case CLOSEPATH:
	    closepath();
	    break;
	case CALLSUBR:
	    sub = pop();
	    subpc = subrs[sub];
	    if(!subpc) {
		fprintf(stderr,"\nYUCK no sub addr\n");
	    }
	    pushpc(pc);
	    pc = subpc;
	    break;
	case RETURN:
	    pc = poppc();
	    break;
	case HSBW:
	    w = pop();
	    x = pop();
	    poly_width(w);
	    spline_width(w);
	    sbpoint(x,0);
	    break;
	case ENDCHAR:
	    closepath();
	    break;
	case UNDOC15:
	    break;
	case RMOVETO:
	    y = pop();
	    x = pop();
	    rmoveto(x,y);
	    break;
	case HMOVETO:
	    x = pop();
	    rmoveto(x,0);
	    break;
	case VHCURVETO:
	    dy3 = 0;
	    dx3 = pop();
	    dy2 = pop();
	    dx2 = pop();
	    dy1 = pop();
	    dx1 = 0;
	    rcurveto(dx1,dy1,dx1+dx2,dy1+dy2,dx1+dx2+dx3,dy1+dy2+dy3);
	    break;
	case HVCURVETO:
	    dy3 = pop();
	    dx3 = 0;
	    dy2 = pop();
	    dx2 = pop();
	    dy1 = 0;
	    dx1 = pop();
	    rcurveto(dx1,dy1,dx1+dx2,dy1+dy2,dx1+dx2+dx3,dy1+dy2+dy3);
	    break;
	case DOTSECTION:
	    break;
	case VSTEM3:
	    pop();
	    pop();
	    pop();
	    pop();
	    pop();
	    pop();
	    break;
	case HSTEM3:
	    pop();
	    pop();
	    pop();
	    pop();
	    pop();
	    pop();
	    break;
	case SEAC:
	    achar = pop();
	    bchar = pop();
	    ady   = pop();
	    adx   = pop();
	    asb   = pop();
	    seac(asb, adx, ady, bchar, achar);
	    return 0;
	case SBW:
	    x = pop();
	    y = pop();
	    fprintf(stderr,"sbw: width: %d %d\n",x,y);
	    poly_width(x);
	    spline_width(x);
	    y = pop();
	    x = pop();
	    fprintf(stderr,"sbw: side: %d %d\n",x,y);
	    sbpoint(x,y);
	    break;
	case DIV:
	    x = pop();
	    y = pop();
	    push(x/y);
	    break;
	case CALLOTHERSUBR:
	    sub = pop();
	    n = pop();
	    if(sub == 0)
		subr0();
	    else if(sub == 1)
		subr1();
	    else if(sub == 2)
		subr2();
	    else {
		for(i=0; i<n; i++) {
		    retpush(pop());
		}
	    }
	    break;
	case POP:
	    push(retpop());
	    break;
	case SETCURRENTPOINT:
	    y = pop();
	    x = pop();
	    sbpoint(x,y);
	    break;
	default:
	    fprintf(stderr,"\nYUCK bad instruction %d\n",cmd);
	    break;
    }
    if(pc == 0 || cmd == ENDCHAR || cmd == WHAT0 || cmd == SEAC)
	return 0;
    else
	return 1;
}

/*
 *	Character interpreter
 *
 */
static void runprog()
{
    long v, w, num, cmd;

#ifdef PRINTDEBUG
    unsigned char	*printpc;
    long		printed;

    /* print program */
    printpc = pc;
    printed = 0;
    while(printpc && *printpc != WHAT0 && *printpc != ENDCHAR) {
	v  = *printpc++;
	if(v>=0 && v<=31) {
	    if(v == 12) {
		w  = *printpc++;
		cmd = 256+w;
	    } else 
		cmd = v;
	    switch(cmd) {
	    case HSTEM:    	fprintf(stderr, "HSTEM"); break;
	    case VSTEM:   	fprintf(stderr, "VSTEM"); break;
	    case VMOVETO: 	fprintf(stderr, "VMOVE"); break;
	    case RLINETO: 	fprintf(stderr, "RLINE"); break;
	    case HLINETO: 	fprintf(stderr, "HLINE"); break;
	    case VLINETO: 	fprintf(stderr, "VLINE"); break;
	    case RRCURVETO: 	fprintf(stderr, "RRCRV"); break;
	    case CLOSEPATH: 	fprintf(stderr, "CLSEP"); break;
	    case CALLSUBR: 	fprintf(stderr, "CLLSB"); break;
	    case RETURN: 	fprintf(stderr, "RETRN"); break;
	    case HSBW: 		fprintf(stderr, "HSBW "); break;
	    case ENDCHAR: 	fprintf(stderr, "ENDCH"); break;
	    case RMOVETO: 	fprintf(stderr, "RMOVE"); break;
	    case HMOVETO: 	fprintf(stderr, "HMOVE"); break;
	    case VHCURVETO: 	fprintf(stderr, "VHCRV"); break;
	    case HVCURVETO: 	fprintf(stderr, "HVCRV"); break;
	    case DOTSECTION: 	fprintf(stderr, "DTSCT"); break;
	    case VSTEM3: 	fprintf(stderr, "VSTM3"); break;
	    case HSTEM3: 	fprintf(stderr, "HSTM3"); break;
	    case SEAC: 		fprintf(stderr, "SEAC "); printpc = 0; break;
	    case SBW: 		fprintf(stderr, "SBW  "); break;
	    case DIV: 		fprintf(stderr, "DIV  "); break;
	    case CALLOTHERSUBR: fprintf(stderr, "COSBR"); break;
	    case POP: 		fprintf(stderr, "POP  "); break;
	    case SETCURRENTPOINT:fprintf(stderr,"SETPT"); break;
	    case WHAT0: 	fprintf(stderr, "WHAT0"); break;
	    default:		fprintf(stderr, "ERROR"); break;
	    }
	} else if(v>=32 && v<=246) {
	    num = v-139;
	    fprintf(stderr, "%5d", num);
	} else if(v>=247 && v<=250) {
	    w  = *printpc++;
	    num = (v-247)*256+w+108;
	    fprintf(stderr, "%5d", num);
	} else if(v>=251 && v<=254) {
	    w  = *printpc++;
	    num = -(v-251)*256-w-108;
	    fprintf(stderr, "%5d", num);
	} else if(v == 255) {
	    num  = *printpc++;
	    num <<= 8;
	    num |= *printpc++;
	    num <<= 8;
	    num |= *printpc++;
	    num <<= 8;
	    num |= *printpc++;
	    fprintf(stderr, "%5d", num);
	}
	if(printed++ % 12 == 11)
	    fprintf(stderr, "\n");
	else
	    fprintf(stderr, " ");
    }
    if(printpc == 0) {
	fprintf(stderr,"OK  pc is 0\n");
	exit(0);
    } else {
	if(*printpc == ENDCHAR) 
	    fprintf(stderr, "ENDCH");
     }
#endif

    while(1) {
	v  = *pc++;
	if(v>=0 && v<=31) {
	    if(v == 12) {
		w  = *pc++;
		cmd = 256+w;
	    } else 
		cmd = v;
	    if(!docommand(cmd)) {
		return;
	    }
	} else if(v>=32 && v<=246) {
	    num = v-139;
	    push(num);
	} else if(v>=247 && v<=250) {
	    w  = *pc++;
	    num = (v-247)*256+w+108;
	    push(num);
	} else if(v>=251 && v<=254) {
	    w  = *pc++;
	    num = -(v-251)*256-w-108;
	    push(num);
	} else if(v == 255) {
	    num  = *pc++;
	    num <<= 8;
	    num |= *pc++;
	    num <<= 8;
	    num |= *pc++;
	    num <<= 8;
	    num |= *pc++;
	    push(num);
	}
    }
}

/*
 *	genobjfont -
 *		generate an object font.
 *
 */
static objfnt *genobjfont(int savesplines,int fullset)
{
    int i, c;
    objfnt *fnt;
    int maxchars;

    if (strncmp(fname,"Symbol",6) == 0) {
        if(savesplines)
	    fnt = newobjfnt(SP_TYPE,32,32+NSYMBL-1,1000);
        else
            fnt = newobjfnt(PO_TYPE,32,32+NSYMBL-1,1000);
        for(i=0; i<NSYMBL; i++) {
            c = i+32;
            if(scharlist[i].prog>=0) {
                nshorts = 0;
                sp_nshorts = 0;
                drawchar(scharlist[i].prog);
                if(savesplines)
                    addchardata(fnt,c,sp_chardata,sp_nshorts);
                else
                    addchardata(fnt,c,chardata,nshorts);
                addcharmetrics(fnt,c,thecharwidth,0);
            } else 
                if(c == ' ') {
                    printf("faking space %d\n",i);
                    fakechar(fnt,' ',400);
                }
        }
    } else {
        if(strncmp(fname,"ZapfDingbats",12) == 0) {
            if(savesplines)
                fnt = newobjfnt(SP_TYPE,32,32+NZAPFD-1,1000);
            else
                fnt = newobjfnt(PO_TYPE,32,32+NZAPFD-1,1000);
            for(i=0; i<NZAPFD; i++) {
                c = i+32;
                if(zcharlist[i].prog>=0) {
                    nshorts = 0;
                    sp_nshorts = 0;
                    drawchar(zcharlist[i].prog);
                    if(savesplines)
                        addchardata(fnt,c,sp_chardata,sp_nshorts);
                    else
                        addchardata(fnt,c,chardata,nshorts);
                    addcharmetrics(fnt,c,thecharwidth,0);
                } else 
                    if(c == ' ') {
                        printf("faking space %d\n",i);
                        fakechar(fnt,' ',400);
                    }
            }
        } else {

	    maxchars = 0;
            for(i=0; i<NASCII; i++) {
                if(charlist[i].prog>=0)
		     maxchars = i;
	    }
	    maxchars++;

	    if(maxchars>NASCII)
		 maxchars = NASCII;
	    if(!fullset) {
		if(maxchars>95)
		     maxchars = 95;
	    }

            if(savesplines)
                fnt = newobjfnt(SP_TYPE,32,32+maxchars-1,1000);
            else
                fnt = newobjfnt(PO_TYPE,32,32+maxchars-1,1000);
            for(i=0; i<maxchars; i++) {
                c = i+32;
                if(charlist[i].prog>=0) {
                    nshorts = 0;
                    sp_nshorts = 0;
#ifdef DEBUGGGING
		    fprintf(stderr, "\n\ndrawing character '%c' (%03o %d)\n",
			c, c, c);
#endif
                    drawchar(charlist[i].prog);
                    if(savesplines)
                        addchardata(fnt,c,sp_chardata,sp_nshorts);
                    else
                        addchardata(fnt,c,chardata,nshorts);
                    addcharmetrics(fnt,c,thecharwidth,0);
                } else 
                    if(c == ' ') {
                        printf("faking space %d\n",i);
                        fakechar(fnt,' ',400);
                    }
            }
        } 
    }
    return fnt;
}

objfnt *readtype1(char *infont,float beztolerance,int fullset)
{
    int i, k, index;
    int nread, namelen, savesplines;
    char *cptr;
    objfnt *fnt;

    if(beztolerance<0.0001) {
	savesplines = 1;
	beztol = 100.0;
    } else  {
	savesplines = 0;
	beztol = beztolerance;
    }
 
/* read the font matrix from the font */
    *fname = NULL;
    readfontmatrix(infont,mat);

/* decode the font data */
    bindat = decodetype1(infont);

/* open the input file */
    fakefopen();

/* init the charname, charlen and sublen arrays */
    nchars = 0;
    for(i=0; i<MAXCHARS; i++) {
	charname[i] = 0;
	charlen[i] = 0;
    }
    for(i=0; i<MAXSUBRS; i++) 
	sublen[i] = 0;

/* look for the /Subrs def and get nsubrs */
    while(1) {
	if(!fakefgets(oneline,LINELEN)) {
	    fprintf(stderr,"fromtype1: no /Subrs found\n");
	    exit(1);
	}
	cptr = strchr(oneline,'/');
	if(cptr) {
	    if(strncmp(cptr,"/Subrs",6) == 0) {
		nsubrs = atoi(cptr+6);
		break;
	    }
	}
    }

/* read the Subrs in one by one */
    for(i=0; i<nsubrs; i++) 
	sublen[i] = 0;
    for(i=0; i<nsubrs; i++) {
	for(k=0; k<MAXTRIES; k++) {
	    fakegettoken(tok);
	    if(strcmp(tok,"dup") == 0)
		break;
	}
	if(k == MAXTRIES) {
	    /* fprintf(stderr,"dup for subr %d not found in range\n"); */
		/* don't know what the %d was supposed to be, don't care -don */
	    fprintf(stderr,"dup for subr not found in range\n");
	    exit(1);
	}

/* get the Subr index here */
	fakegettoken(tok);
	index = atoi(tok);

/* check to make sure it is in range */
	if(index<0 || index>nsubrs) {
	    fprintf(stderr,"bad Subr index %d\n",index);
	    exit(1);
	}

/* get the number of bytes to read */
	fakegettoken(tok);
	nread = atoi(tok);
	fakegettoken(tok);

/* read in the subroutine */
	sublen[index] = nread;
	subrs[index] = fakefread(nread);
	fakegettoken(tok);
    }

/* look for the CharStrings */
    while(1) {
	fakegettoken(tok);
	cptr = strchr(tok,'/');
	if(cptr && strcmp(cptr,"/CharStrings") == 0)
	    break;
    }

    fakegettoken(tok);	/* skip ncharscrings */
    fakegettoken(tok);	/* skip dict */
    fakegettoken(tok);	/* skip dup */
    fakegettoken(tok);	/* skip begin */
    fakegettoken(tok);	/* skip newline */

/* read the CharStrings one by one */
    for(i=0; i<MAXCHARS; i++) { 

/* check for end */
	fakegettoken(tok);
	if(strcmp(tok,"end") == 0) 
	    break;

/* get the char name and allocate space for it */
	namelen = strlen(tok);
	charname[i] = (char *)mymalloc(namelen+1);
	strcpy(charname[i],tok);

/* get the number of bytes to read */
	fakegettoken(tok);
	nread = atoi(tok);
	fakegettoken(tok);

/* read in the char description */
	charlen[i] = nread;
	chars[i] = fakefread(nread);

/* skip the end of line */
	fakegettoken(tok);
	fakegettoken(tok);
	nchars++;
    }

/* decrypt the character descriptions */
    decryptall();
    setcharlist();

/* make the obj font */
    fnt = genobjfont(savesplines,fullset);

/* calculate the bboxes */
    calccharbboxes(fnt);

/* free malloced stuff */
    free(bindat);
    for(i=0; i<MAXCHARS; i++) { 
	if(charname[i])
	    free(charname[i]);
    }
    return fnt;
}

#ifdef FINDIT
printchardata(i)
int i;
{
    int n;
    unsigned char *cptr;

    printf("name: [%s] proglen %d\n",charname[i],charlen[i]);
    n = charlen[i];
    cptr = chars[i];
    while(n--) {
	printf("%02x ",*cptr++);
	
    }
    printf("\n");
}
#endif
