/*
 * Copyright 1996, Silicon Graphics, Inc.
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
 * file: gridify.c
 *
 * $Revision: 1.29 $
 * $Date: 2002/05/15 00:31:09 $
 *
 * -----------------
 *
 * Contains the function pfuGridify(), which turns
 * gridification of a clip texture on or off.
 *
 * When gridification of a clip texture is on,
 * a colored grid is overlayed on each disk tile of texel data.
 * The grid color is determined as follows, depending on the clipmap level:
            1x1      cyan
            2x2      blue
            4x4      magenta
            8x8      red
           16x16     yellow
           32x32     green
           64x64     cyan
          128x128    blue
          256x256    magenta
          512x512    red
         1024x1024   yellow
         2048x2048   green
         4096x4096   cyan
         8192x8192   blue
        16384x16384  magenta
        32768x32768  red
 *      
 * The grid consists of:
 *      -- A 2-pixel border around the entire tile
 *      -- A column of 4x8-pixel ticks, spaced 8 pixels apart,
 *         down the center of the tile vertically, and a similar row of them
 *         across the center horizontally
 *      -- Two columns of 2x4-pixel ticks, spaced 16 pixels apart,
 *         at the 1/4 and 3/4 position, and similar rows of them
 *
 * For example, this is the grid overlayed on a 128x128 tile
 * (only the upper-left quarter is shown):
 *
    ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    ::
    ::
    ::                                                          ::::::::
    ::                                                          ::::::::
    ::
    ::
    ::                            ....                          ::::::::
    ::                            """"                          ::::::::
    ::
    ::
    ::                                                          ::::::::
    ::                                                          ::::::::
    ::
    ::
    ::             ::             .::.             ::           ::::::::
    ::             ::             "::"             ::           ::::::::
    ::
    ::
    ::                                                          ::::::::
    ::                                                          ::::::::
    ::
    ::
    ::                            ....                          ::::::::
    ::                            """"                          ::::::::
    ::
    ::
    ::                                                          ::::::::
    ::                                                          ::::::::
    ::
    ::    ::::    ::::    ::::    ::::    ::::    ::::    ::::    ::::    ::
    ::    ::::    ::::    ::::    ::::    ::::    ::::    ::::  ::::::::  ::
    ::    ::::    ::::    ::::    ::::    ::::    ::::    ::::  ::::::::  ::
    ::    ::::    ::::    ::::    ::::    ::::    ::::    ::::    ::::    ::
    ::
    ::                                                          ::::::::

 *
 * A 64x64 tile:
 *
    ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    ::                                                            ::
    ::                                                            ::
    ::                          ::::::::                          ::
    ::                          ::::::::                          ::
    ::                                                            ::
    ::                                                            ::
    ::            .::.          ::::::::          .::.            ::
    ::            "::"          ::::::::          "::"            ::
    ::                                                            ::
    ::                                                            ::
    ::                          ::::::::                          ::
    ::                          ::::::::                          ::
    ::                                                            ::
    ::    ::::    ::::    ::::    ::::    ::::    ::::    ::::    ::
    ::    ::::    ::::    ::::  ::::::::  ::::    ::::    ::::    ::
    ::    ::::    ::::    ::::  ::::::::  ::::    ::::    ::::    ::
    ::    ::::    ::::    ::::    ::::    ::::    ::::    ::::    ::
    ::                                                            ::
    ::                          ::::::::                          ::
    ::                          ::::::::                          ::
    ::                                                            ::
    ::                                                            ::
    ::            .::.          ::::::::          .::.            ::
    ::            "::"          ::::::::          "::"            ::
    ::                                                            ::
    ::                                                            ::
    ::                          ::::::::                          ::
    ::                          ::::::::                          ::
    ::                                                            ::
    ::                                                            ::
    ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

 */

#include <Performer/pr.h>
#include <Performer/pr/pfImageTile.h>
#include <Performer/pr/pfImageCache.h>
#include <Performer/pr/pfClipTexture.h>
#include <Performer/pf/pfMPClipTexture.h>
#include <Performer/pfutil.h>
#include <stdlib.h>
#define FOR(i,n) for ((i) = 0; (i) < (n); ++(i))








/* paolo */
#ifndef P_32_SWAP
#define	P_32_SWAP(a) {					\
	uint _tmp = *(uint *)a;				\
	((char *)a)[0] = ((char *)&_tmp)[3];		\
	((char *)a)[1] = ((char *)&_tmp)[2];		\
	((char *)a)[2] = ((char *)&_tmp)[1];		\
	((char *)a)[3] = ((char *)&_tmp)[0];		\
}
#endif  /* P_32_SWAP */


#ifndef P_16_SWAP
#define	P_16_SWAP(a) {					\
	ushort _tmp = *(ushort *)a;			\
	((char *)a)[0] = ((char *)&_tmp)[1];		\
	((char *)a)[1] = ((char *)&_tmp)[0];		\
}
#endif  /* P_16_SWAP */













/*
 * General optimized loop for gridification.
 * It is essential that this function be inlined
 * so that many of the variables controlling the inner loops
 * will be known at compile time and therefore optimized out.
 * XXX The o32 compiler won't do this, but the n32/64 compiler will
 * (Ragnarok in many minutes, Mongoose in a few seconds).
 * XXX If the compiler says a parameter needs to be set to
 * get optimization, do what it says!
 */
static inline void
_zipper(unsigned char *ptr, int depth,
        const unsigned char *&setcolors, int setcolorstride,
	      unsigned char *&getcolors, int getcolorstride,
	int dim0, int incr0,
	int dim1, int incr1,
	int dim2, int incr2,
	int dim3, int incr3)
{
    int i3, i2, i1, i0, i;
    FOR (i3, dim3)
    {
	FOR (i2, dim2)
	{
	    FOR (i1, dim1)
	    {
		FOR (i0, dim0)	// this loop should get optimized out
		{
		    if (getcolors)
		    {
			FOR (i, depth)
			    getcolors[i] = ptr[i];
			getcolors += getcolorstride;
		    }
		    if (setcolors)
		    {
			FOR (i, depth)
			    ptr[i] = setcolors[i];
			setcolors += setcolorstride;
		    }
		    ptr += incr0;
		}
		ptr += incr1;
	    }
	    ptr += incr2;
	}
	ptr += incr3;
    }
}

/*
 * Draw a zipper in a 1-dimensional buffer
 */
static inline void
zipper1(unsigned char *buf, int bufsize, int depth, int dir,
        const unsigned char *&setcolors, int setcolorstride,
              unsigned char *&getcolors, int getcolorstride,
	int dim0, int stride0,
	int dim1, int stride1,
	int dim2, int stride2,
	int dim3, int stride3)
{
    bufsize *= depth;
    setcolorstride *= depth;
    getcolorstride *= depth;
    stride0 *= dir * depth;
    stride1 *= dir * depth;
    stride2 *= dir * depth;
    stride3 *= dir * depth;

    /* now everything is in bytes rather than pixels. */
    /* start at a position so that the zipper is centered... */
    _zipper(buf + (bufsize - ((dim3-1)*stride3 +
			      (dim2-1)*stride2 +
			      (dim1-1)*stride1 +
			      (dim0-1)*stride0 + depth)) /2,
	     depth,
	     setcolors, setcolorstride, getcolors, getcolorstride,
	     dim0, stride0,
	     dim1, stride1 - dim0*stride0,
	     dim2, stride2 - dim1*stride1,
	     dim3, stride3 - dim2*stride2);
}

/*
 * Draw a zipper in a 2-dimensional buffer.
 * Return the number of pixels processed.
 */
static inline int
zipper2(unsigned char *buf, int bufwidth, int bufheight, int bufdepth, int dir,
       const unsigned char *&setcolors, int setcolorstride,
             unsigned char *&getcolors, int getcolorstride,
       int dim0, int stride0x, int stride0y, /* to next pixel in fastest dir */
       int dim1, int stride1x, int stride1y, /* to next pixel in middle dir */
       int dim2, int stride2x, int stride2y, /* to next pixel in slowest dir */
       int dim3=1, int stride3x=0, int stride3y=0)
{
    if (dim0 <= 0 || dim1 <= 0 || dim2 <= 0 || dim3 <= 0)
	return 0;
    if (setcolors != NULL || getcolors != NULL)
    {
	zipper1(buf, bufwidth*bufheight, bufdepth, dir,
		setcolors, setcolorstride, getcolors, getcolorstride,
		dim0, stride0y*bufwidth + stride0x,
		dim1, stride1y*bufwidth + stride1x,
		dim2, stride2y*bufwidth + stride2x,
		dim3, stride3y*bufwidth + stride3x);
    }
    return dim0*dim1*dim2*dim3;
}

static int
gridifymem(unsigned char *buf, int width, int height, int depth, int dir,
	int borderwidth,
        const unsigned char *setcolors, int setcolorstride,
	      unsigned char *getcolors, int getcolorstride)
{
    if (setcolors != NULL && getcolors != NULL)
	PFASSERTALWAYS(setcolors != getcolors);

    if (width <= 2*borderwidth
     || height <= 2*borderwidth)
    {
	return zipper2(buf, width, height, depth, dir,
		       setcolors, setcolorstride, getcolors, getcolorstride,
		       width,1,0,  height,0,1,  1,0,0);
    }

    int npixels = 0;

    /*
     * Hack to put a single uniform grid
     * covering the whole thing...
     * XXX This is easy, we should have API for it
     */
    {
	static int been_here_before = 0;
	static char *e;
	if (!been_here_before)
	{
	    e = getenv("_CCT_GRIDIFY");
	    been_here_before = 1;
	}
	if (e != NULL)
	{
	    /*
	     * By default, 4x8 rectangles covering the whole dang thing,
	     * spaced 16x16 apart
	     */
	    int ticSizeX = 4;
	    int ticSizeY = 8;
	    int ticSpacingX = 16;
	    int ticSpacingY = 16;
	    (void)sscanf(e, "%d,%d,%d,%d", &ticSizeX, &ticSizeY,
					   &ticSpacingX, &ticSpacingY);
	    npixels += zipper2(buf, width, height, depth, dir,
			       setcolors, setcolorstride,
			       getcolors, getcolorstride,
			       ticSizeX,1,0,
			       ticSizeY,0,1,
			       width/ticSpacingX,ticSpacingX,0,
			       height/ticSpacingY,0,ticSpacingY);
	    return npixels;
	}
    }

    /*
     * Border around the whole thing...
     */
    npixels += zipper2(buf, width, height, depth, dir,
	    setcolors, setcolorstride, getcolors, getcolorstride,
	    width,1,0,  borderwidth,0,1,  2,0,height-borderwidth);
    npixels += zipper2(buf, width, height, depth, dir,
	    setcolors, setcolorstride, getcolors, getcolorstride,
	    borderwidth,1,0,  2,width-borderwidth,0,  height-2*borderwidth,0,1);

    /*
     * 4x8 rectangles along the midlines, 8 pixels apart...
     */
    if (width >= 8 && height >= 8)
    {
	npixels += zipper2(buf, width, height, depth, dir,
	        setcolors, setcolorstride, getcolors, getcolorstride,
		8,1,0,  4,0,1,  height/8-1,0,8);
	npixels += zipper2(buf, width, height, depth, dir,
	        setcolors, setcolorstride, getcolors, getcolorstride,
		4,1,0,  width/8-1,8,0,  8,0,1);
    }

    /*
     * And little 2x4 rectangles along the quarter lines, 16 pixels apart...
     */
    if (width >= 16 && height >= 16)
    {
	npixels += zipper2(buf, width, height, depth, dir,
	        setcolors, setcolorstride, getcolors, getcolorstride,
		4,1,0,  2,0,1,  height/16-1,0,16,  2,width/2,0);
	npixels += zipper2(buf, width, height, depth, dir,
	        setcolors, setcolorstride, getcolors, getcolorstride,
		2,1,0,  height/16-1,16,0,  4,0,1,  2,0,height/2);
    }

    return npixels;
}

/* return the number of bytes that will be saved by pfuGridify() */
extern PFUDLLEXPORT int
pfuGridifySaveSize(int width, int height, int depth,
		       int borderwidth)
{
    return depth * gridifymem(NULL, width, height, depth,
			           1, borderwidth,
		               NULL,0,NULL,0);
}

extern PFUDLLEXPORT void
pfuGridifyMem(unsigned char *buf, int width, int height, int depth,
	   int borderwidth,
	   const unsigned char color[/* depth */],
	   unsigned char savecolors[/* pfuGridifySaveSize() * depth */])
{
    /* XXX seems like we ought to be able to
       do the saving and setting in one call to gridifymem(),
       but it's awkward to keep track of the few pixels that
       get hit multiple times.
       If I don't figure out a way to do this,
       the "dir" parameter will probably never be used and can go away. */
    if (savecolors != NULL)
	(void)gridifymem(buf, width, height, depth, 1, borderwidth,
		          NULL,0,savecolors,1);
    (void)gridifymem(buf, width, height, depth, 1, borderwidth,
		  color,0,NULL,0);
}

extern PFUDLLEXPORT void
pfuUnGridifyMem(unsigned char *buf, int width, int height, int depth,
	    int borderwidth,
	    const unsigned char savedcolors[/* pfuGridifySaveSize() * depth */])
{
    (void)gridifymem(buf, width, height, depth, 1, borderwidth,
		  savedcolors,1,NULL,0);
}


//============================================================================
// File gridification...
//============================================================================

/* return 1 on success, 0 on failure */
extern PFUDLLEXPORT int
pfuGridifyFileP(const char *infilename_for_errors, FILE *infp,
		const char *outfilename_for_errors, FILE *outfp,
		int width, int height, int depth,
	        int borderwidth,
	        unsigned char color[/* depth */])
{
    int returnval = 1; // success, until we find out otherwise

    int bufsize = width*height*depth, nread;
    unsigned char *buf = (unsigned char *)malloc(bufsize);
    if (buf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuGridifyFile: Can't allocate %d (%dx%dx%d) bytes for file \"%s\"\n", bufsize, width, height, depth, infilename_for_errors);
	returnval = 0; // failure
    }
    else if ((nread = fread(buf, 1, bufsize, infp)) != bufsize)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuGridifyFile: Expected to read %d (%dx%dx%d) bytes from \"%s\", but only got %d:", bufsize, width, height, depth, infilename_for_errors, nread);
	if (ferror(infp))
	    pfNotify(PFNFY_WARN, PFNFY_MORE, "    %s", strerror(oserror()));
	else
	    pfNotify(PFNFY_WARN, PFNFY_MORE, "    Premature EOF");
	returnval = 0; // failure
    }
    else
    {
	/*
	 * Gridify the tile in memory.
	 */
// #define TEST_PFUUNGRIDIFYTILE
#ifdef TEST_PFUUNGRIDIFYTILE
	unsigned char *savebuf = (unsigned char *)
		malloc(pfuGridifySaveSize(width, height, depth, borderwidth));
	PFASSERTALWAYS(savebuf != NULL);
	pfuGridifyMem(buf, width, height, depth,
		       borderwidth, color, savebuf);
	/* pfuUnGridifyMem(buf, width, height, depth, borderwidth, savebuf); */
	free(savebuf);
#else
	pfuGridifyMem(buf, width, height, depth,
		       borderwidth, color, NULL);
#endif
	/*
	 * Write it back out and flush.
	 */
	if (infp == outfp)
	    rewind(outfp);
	int nwrote = fwrite(buf, sizeof(unsigned char), bufsize, outfp);
	if (nwrote != bufsize)
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfuGridifyFile: Could only write %d of %d bytes back to file \"%s\"", nwrote, bufsize, outfilename_for_errors);
	    pfNotify(PFNFY_WARN, PFNFY_MORE, "    %s", strerror(oserror()));
	    // don't return here; go ahead and close to avoid a leak
	    returnval = 0; // failure
	}
	if (fflush(outfp) == EOF)
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfuGridifyFile: Error flushing to file \"%s\" after writing %d bytes",
		     outfilename_for_errors, nwrote);
	    pfNotify(PFNFY_WARN, PFNFY_MORE, "    (file may be corrupt):");
	    pfNotify(PFNFY_WARN, PFNFY_MORE, "    %s", strerror(oserror()));
	    returnval = 0; // failure
	}
    }
    if (buf != NULL)
	free(buf);

    return returnval;
} /* end pfuGridifyFileP() */


/* return 1 on success, 0 on failure */
extern PFUDLLEXPORT int
pfuGridifyFile(const char *filename, int width, int height, int depth,
	   int borderwidth,
	   unsigned char color[/* depth */])
{
    FILE *fp = fopen(filename, "r+b");
    if (fp == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuGridifyFile: Can't open file \"%s\" for read-write:",
		 filename);
	pfNotify(PFNFY_WARN, PFNFY_MORE, "    %s", strerror(oserror()));
	return 0; // failure
    }

    int returnval = pfuGridifyFileP(filename, fp, filename, fp,
				    width, height, depth,
				    borderwidth, color);

    if (fclose(fp) == EOF)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "Error closing file \"%s\" after gridification",
		 filename);
	pfNotify(PFNFY_WARN, PFNFY_MORE, "    (file may be corrupt):");
	pfNotify(PFNFY_WARN, PFNFY_MORE, "    %s", strerror(oserror()));
	returnval = 0; // failure
    }

    return returnval;
} /* end pfuGridifyFile() */


//==========================================================================
// pfImageTile gridification...
//==========================================================================

struct ImageTileUserData
{
    pfReadImageTileFuncType origReadFunc;
    int w,h,d,pixeldepth,borderwidth;
    unsigned char color[4];
    unsigned char savedTexels[1];
};

static struct ImageTileUserData *
NewUserData(pfReadImageTileFuncType origReadFunc,
		 int w, int h, int d,
		 int pixeldepth,
		 int borderwidth,
		 const unsigned char color[4])
{
    int size = offsetof(struct ImageTileUserData, savedTexels)
	     + pfuGridifySaveSize(w,h,pixeldepth, borderwidth);
    struct ImageTileUserData *userdata =
	(struct ImageTileUserData *) pfMalloc(size, pfGetSharedArena());
    if (userdata == NULL)
    {
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
	   "pfuGridifyImageTile: can't allocate %d bytes to save texels",
	   size);
	return NULL;
    }
    userdata->origReadFunc = origReadFunc;
    userdata->w = w;
    userdata->h = h;
    userdata->d = d;
    userdata->pixeldepth = pixeldepth;
    userdata->borderwidth = borderwidth;
    int i;
    FOR (i, 4)
	userdata->color[i] = color[i];
    return userdata;
}


// Because pfObject::setUserData() unrefs but does not delete
// the old user data, which is NEVER what I want...
static void pfUnrefDeleteSetUserData(pfObject *obj, void *data)
{
    void *olddata = obj->getUserData();
    if (olddata != data)
    {
	pfMemory::ref(olddata);
	obj->setUserData(data);
	pfMemory::unrefDelete(olddata); // XXX not pfBuffer::unrefDelete()?
    }
}


/*
 * Read func that calls the real read func and gridifies...
 */
static int pfuGridifyReadFunc(pfImageTile *itile, int nTexels)
{
    /*
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "in pfuGridifyReadFunc");
    */
    struct ImageTileUserData *userdata =
	(struct ImageTileUserData *) itile->getUserData();
    if (userdata == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_INTERNAL,
		 "pfuGridifyReadFunc(itile=0x%p): no userdata?", itile);
	return 0; // failure
    }
    // The old userdata is a ref to the proto's.  Make
    // a new one using the relevant parameters.
    userdata = NewUserData(userdata->origReadFunc,
			   userdata->w,userdata->h,userdata->d,
			   userdata->pixeldepth,
			   userdata->borderwidth,
			   userdata->color);
    pfRef(userdata); // to protect us if itile gets put on the free list
		     // before we complete
    pfUnrefDeleteSetUserData(itile, userdata);
    if (userdata == NULL)
	return 0; // error handled already
    if (userdata->origReadFunc == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_INTERNAL,
		 "pfuGridifyReadFunc: origReadFunc is NULL");
	return 0;
    }
    int bytesRead = userdata->origReadFunc(itile, nTexels);
    if (bytesRead == userdata->w*userdata->h*userdata->d)
    {
	pfuGridifyMem(itile->getMem(),
		      userdata->w,
		      userdata->h,
		      userdata->pixeldepth,
		      userdata->borderwidth,
		      userdata->color,
		      userdata->savedTexels);
    }
    else
    {
	pfNotify(PFNFY_DEBUG, PFNFY_INTERNAL,
	    "pfuGridifyReadFunc(%s): orig read func returned %d out of %d, not gridifying tile\n",
	    itile->getFileName(), bytesRead, userdata->w*userdata->h*userdata->d);
	itile->setReadFunc(userdata->origReadFunc);
	pfUnrefDeleteSetUserData(itile, NULL);
    }
    pfMemory::unrefDelete(userdata); // XXX not pfBuffer::unrefDelete()?
    return bytesRead;
}

extern PFUDLLEXPORT int
pfuGridifyImageTile(pfImageTile *itile, int pixeldepth, const unsigned char color[], int isproto)
{
    static int borderwidth = -1;
    if (borderwidth == -1)
    {
	char *e = getenv("_PFUGRIDIFY_BORDERWIDTH");
	borderwidth = (e ? *e ? atoi(e) : 1 : 2);
    }

    int w,h,d;
    itile->getSize(&w,&h,&d);

    pfReadImageTileFuncType origReadFunc = itile->getReadFunc();
    if (origReadFunc == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_INTERNAL,
		 "pfuGridifyImageTile: origReadFunc is NULL");
	return 0; // failure
    }
    if (origReadFunc == pfuGridifyReadFunc)
    {
	pfNotify(PFNFY_WARN, PFNFY_INTERNAL,
		 "pfuGridifyImageTile: origReadFunc is already pfuGridifyReadFunc (%sproto tile 0x%p)", isproto ? "" : "non-", itile);
	return 0; // failure
    }
    struct ImageTileUserData *userdata = NewUserData(origReadFunc,
				w,h,d,pixeldepth,borderwidth,color);
    if (userdata == NULL)
	return 0; // error handled already

    if (!isproto)
	if (access(itile->getFileName(), F_OK) != -1)
	{
	    int orgS,orgT,orgR;
	    itile->getOrigin(&orgS,&orgT,&orgR);
	    unsigned char *mem = itile->getValidSubTile(orgS,orgT,orgR);
	    if (mem != NULL)
		pfuGridifyMem(mem,
			      userdata->w,
			      userdata->h,
			      userdata->pixeldepth,
			      userdata->borderwidth,
			      userdata->color,
			      userdata->savedTexels);
	    // XXX need to wait even if the file does not exist!
	    // XXX what to do if getValidSubTile returned NULL?
	}

    // this must be after the getValidSubTile to prevent race with i/o thread
    pfUnrefDeleteSetUserData(itile, userdata);
    itile->setReadFunc(pfuGridifyReadFunc);

    return 1; // success
}

extern PFUDLLEXPORT int
pfuUnGridifyImageTile(pfImageTile *itile, int isproto)
{
    int w,h,d;
    itile->getSize(&w,&h,&d);

    struct ImageTileUserData *userdata =
		(struct ImageTileUserData *) itile->getUserData();
    if (userdata == NULL)
	return 0; // the original gridify failed (probably the file did not exist)
    if (w != userdata->w
     || h != userdata->h
     || d != userdata->d
     || itile->getReadFunc() != pfuGridifyReadFunc
     || userdata->origReadFunc == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfuUnGridifyImageTile() called when tile is not gridified (%sproto tile 0x%p)", isproto ? "" : "non-", itile);
	if (userdata == NULL)
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "userdata == NULL");
	if ( w != userdata->w)
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "w != userdata->w");
	if ( h != userdata->h)
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "h != userdata->h");
	if ( d != userdata->d)
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "d != userdata->d");
	if ( itile->getReadFunc() != pfuGridifyReadFunc)
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "itile->getReadFunc() != pfuGridifyReadFunc");
	if ( userdata->origReadFunc == NULL)
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "userdata->origReadFunc == NULL");
	return 0; // failure
    }

    if (!isproto)
	if (access(itile->getFileName(), F_OK) != -1)
	{
	    int orgS,orgT,orgR;
	    itile->getOrigin(&orgS,&orgT,&orgR);
	    unsigned char *mem = itile->getValidSubTile(orgS,orgT,orgR);
	    if (mem != NULL)
		pfuUnGridifyMem(mem,
			      userdata->w,
			      userdata->h,
			      userdata->pixeldepth,
			      userdata->borderwidth,
			      userdata->savedTexels);
	    // XXX need to wait even if the file does not exist!
	    // XXX what to do if getValidSubTile returned NULL?
	}

    // this must be after the getValidSubTile to prevent race with i/o thread
    itile->setReadFunc(userdata->origReadFunc);
    pfUnrefDeleteSetUserData(itile, NULL);

    return 1; // success
}


//==========================================================================
// pfImageCache gridification...
//==========================================================================

extern PFUDLLEXPORT int
pfuGridifyImageCache(pfImageCache *icache, int pixeldepth, unsigned char color[])
{
    pfImageTile *itile;
    int orgS, orgT, orgR;
    int nTilesS, nTilesT, nTilesR;
    int s, t, r;
    icache->getCurMemRegionOrg(&orgS, &orgT, &orgR);
    icache->getCurMemRegionSize(&nTilesS, &nTilesT, &nTilesR);

    FOR (r, nTilesR)
    FOR (t, nTilesT)
    FOR (s, nTilesS)
    {
	itile = icache->getTile(orgS+s, orgT+t, orgR+r);
	if (itile == NULL)
	    return 0; // failure; error handled already
	pfuGridifyImageTile(itile, pixeldepth, color, 0);
    }

    //
    // Must do the proto after all the others
    // (after the i/o threads are idle), otherwise
    // a read func could be copying from the proto's user data
    //
    itile = icache->getProtoTile();
    pfuGridifyImageTile(itile, pixeldepth, color, 1);


    return 1; // success
}

extern PFUDLLEXPORT int
pfuUnGridifyImageCache(pfImageCache *icache)
{
    pfImageTile *itile;
    int orgS, orgT, orgR;
    int nTilesS, nTilesT, nTilesR;
    int s, t, r;
    icache->getCurMemRegionOrg(&orgS, &orgT, &orgR);
    icache->getCurMemRegionSize(&nTilesS, &nTilesT, &nTilesR);

    FOR (r, nTilesR)
    FOR (t, nTilesT)
    FOR (s, nTilesS)
    {
	itile = icache->getTile(orgS+s, orgT+t, orgR+r);
	if (itile == NULL)
	    return 0; // failure; error handled already
	pfuUnGridifyImageTile(itile, 0);
    }

    //
    // Must do the proto after all the others
    // (after the i/o threads are idle), otherwise
    // a read func could be copying from the proto's user data
    //
    itile = icache->getProtoTile();
    pfuUnGridifyImageTile(itile, 1);


    return 1; // success
}

//==========================================================================
// pfClipTexture gridification...
//==========================================================================

static int intlog2(int n) { return n == 1 ? 0 : 1 + intlog2(n/2); }

static void
hsv_to_rgb(double h, double s, double v, double *R, double *G, double *B)
{
    int segment;
    double *rgb[3], major, minor, middle, frac;

    rgb[0] = R;
    rgb[1] = G;
    rgb[2] = B;

    while (h < 0)
        h++;
    while (h >= 1)
        h--;

    segment = (int)(h*6);

    frac = (6*h)-segment;
    if (segment % 2)
        frac = 1 - frac;

    major = v;
    minor = (1-s)*v;
    middle = frac * major + (1-frac) * minor;

    *rgb[(segment+1)/2 % 3] = major;
    *rgb[(segment+4)/2 % 3] = minor;
    *rgb[(7-segment) % 3] = middle;
}

/* the color scheme described at the top of this file... */
static int   /* return bytes per pixel */
getColorOfLevel(int logsize, int clipsize, int internalformat, unsigned char color[4])
{
    // Can set this environment variable to make the top pyramid level
    // have a less saturated color...
    static double _PF_CLIPSAT = -1.;
    if (_PF_CLIPSAT < 0)
    {
	char *e = getenv("_PF_CLIPSAT");
	_PF_CLIPSAT = (e != NULL ? atof(e) : .9);
    }

    // Top pyramid level gets less saturated (barely)...
    double hue = (logsize-3) / 6.;
    double sat = (1<<logsize) == clipsize ? _PF_CLIPSAT : .9;
    double val = 1.;
    double red, green, blue;
    hsv_to_rgb(hue, sat, val, &red, &green, &blue);
    switch (internalformat)
    {
	case PFTEX_RGB_8:
	case PFTEX_RGBA_8:
	    color[0] = (int)(red*255 + .5);
	    color[1] = (int)(green*255 + .5);
	    color[2] = (int)(blue*255 + .5);
	    color[3] = 255;
	    
#ifdef __linux__
	    P_32_SWAP(color);
#endif
	    	    
	    return internalformat == PFTEX_RGBA_8 ? 4 : 3;
	case PFTEX_RGB5_A1:
	{
	    unsigned int red5 = (int)(red*31 + .5);
	    unsigned int green5 = (int)(green*31 + .5);
	    unsigned int blue5 = (int)(blue*31 + .5);
	    unsigned short temp;
	    temp = (red5 << 11) | (green5 << 6) | (blue5 << 1) | 1;
	    color[0] = (temp>>8)&255;
	    color[1] = temp&255;
	    color[2] = 0;
	    color[3] = 0;

#ifdef __linux__
	    P_16_SWAP(color);
#endif
	    
	    return 2;
	}

	case PFTEX_I_8:
	    // Make it black (even levels) or white (odd levels)
	    color[0] = (logsize&1) * 255;
	    color[1] = 0;
	    color[2] = 0;
	    color[3] = 0;
	    return 1;

	case PFTEX_IA_8:
	case GL_LUMINANCE:
	    // Make it black (even levels) or white (odd levels)
	    color[0] = (logsize&1) * 255;
	    color[1] = 255;
	    color[2] = 0;
	    color[3] = 0;

#ifdef __linux__
	    P_16_SWAP(color);
#endif

	    return 2;


	default:
	    pfNotify(PFNFY_FATAL, PFNFY_USAGE,
		     "pfuGridifyClipTexture: Don't know about internal format %#x\n", internalformat);
	    return 0;
    }
}

// can only be called in the cull process (the one controlling the
// image caches)
extern PFUDLLEXPORT int
pfuGridifyClipTexture(pfClipTexture *cliptex)
{
    int returnval = 1; // success; failure at a level will change it to failure
    int w,h,d, i;
    cliptex->getVirtualSize(&w,&h,&d);
    int clipsize = cliptex->getClipSize();
    int mindim = PF_MIN2(w, h);
    int nlevels = intlog2(mindim) + 1;
    int internalformat = cliptex->getFormat(PFTEX_INTERNAL_FORMAT);

    if(cliptex->getMaster())
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuGridifyClipTexture: cliptexture is a slave!");
	return 0;
    }

    FOR (i, nlevels)	/* counting up from 1x1 through VsizexVsize */
    {
	unsigned char color[4];
	int pixeldepth = getColorOfLevel(i, clipsize, internalformat, color);

	pfObject *level = cliptex->getLevel(nlevels-1 - i);
	if (level->isOfType(pfImageCache::getClassType()))
	    returnval &= pfuGridifyImageCache((pfImageCache *)level, pixeldepth, color);
	else
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Gridifying image tile of level %d...", i);
	    returnval &= pfuGridifyImageTile((pfImageTile *)level, pixeldepth, color, 0);
	    pfNotify(PFNFY_INFO, PFNFY_MORE, "done.");
	}

    }

    cliptex->invalidate();

    return returnval;
}

// can only be called in the cull process (the one controlling the
// image caches)
extern PFUDLLEXPORT int
pfuUnGridifyClipTexture(pfClipTexture *cliptex)
{
    int returnval = 1; // success; failure at a level will change it to failure
    int w,h,d, i;
    cliptex->getVirtualSize(&w,&h,&d);
    int mindim = PF_MIN2(w, h);
    int nlevels = intlog2(mindim) + 1;

    if(cliptex->getMaster())
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuGridifyClipTexture: cliptexture is a slave!");
	return 0;
    }

    FOR (i, nlevels)
    {
	pfObject *level = cliptex->getLevel(nlevels-1 - i);
	if (level->isOfType(pfImageCache::getClassType()))
	    returnval &= pfuUnGridifyImageCache((pfImageCache *)level);
	else
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Ungridifying image tile of level %d...", i);
	    returnval &= pfuUnGridifyImageTile((pfImageTile *)level, 0);
	    pfNotify(PFNFY_INFO, PFNFY_MORE, "done.");
	}
    }

    cliptex->invalidate();

    return returnval;
}


//==========================================================================
// pfMPClipTexture gridification...
//==========================================================================

extern PFUDLLEXPORT int
pfuGridifyMPClipTexture(pfMPClipTexture *)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT,
	     "XXX pfuGridifyClipTexture not implemented");
    return 0;
}

extern PFUDLLEXPORT int
pfuUnGridifyMPClipTexture(pfMPClipTexture *)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT,
	     "XXX pfuUnGridifyClipTexture not implemented");
    return 0;
}


//============================================================================
// Hack to gridify the first clip texture encountered in a scene...
//============================================================================

/* ARGSUSED */
static int isClipTexture(pfGeoState *gstate, pfTexture *tex, void *arg)
{
    return tex->isOfType(pfClipTexture::getClassType());
}

extern PFUDLLEXPORT int
pfuGridifyAnyClipTexture(pfNode *scene, int i)
{
    int travmode = PFUTRAV_SW_CUR | PFUTRAV_SEQ_CUR
		 | PFUTRAV_LOD_RANGE0 | PFUTRAV_LAYER_BOTH;
    /* XXX would really like PFUTRAV_LOD_CUR but it doesn't exist */
    pfClipTexture *cliptex = (pfClipTexture*)pfuFindTexture(scene,
					    i, isClipTexture, NULL, travmode);

    if (cliptex == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfuGridifyAnyClipTexture: no clip texture in scene");
	return 0;
    }
    return pfuGridifyClipTexture(cliptex);
}

extern PFUDLLEXPORT int
pfuUnGridifyAnyClipTexture(pfNode *scene, int i)
{
    int travmode = PFUTRAV_SW_CUR | PFUTRAV_SEQ_CUR
		 | PFUTRAV_LOD_RANGE0 | PFUTRAV_LAYER_BOTH;
    /* XXX would really like PFUTRAV_LOD_CUR but it doesn't exist */
    pfClipTexture *cliptex = (pfClipTexture*)pfuFindTexture(scene,
					    i, isClipTexture, NULL, travmode);
    if (cliptex == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfuUnGridifyAnyClipTexture: no clip texture in scene");
	return 0;
    }
    return pfuUnGridifyClipTexture(cliptex);
}


