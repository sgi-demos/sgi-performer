/*
 * pfdLoadImage.c
 *
 * $Revision: 1.122 $
 * $Date: 2002/12/09 04:17:56 $
 *
 * Load a database file into an IRIS Performer in-memory data 
 * structure using the filename's extension to intuit the file 
 * format. The file is sought in the directories named in the
 * active performer file search path (see pfFilePath for more
 * details). 
 *
 *
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
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <dlfcn.h>
#endif
#include <assert.h>

#ifndef WIN32
#include <sys/unistd.h>
#endif
#include <sys/types.h>
#ifndef WIN32
#include <sys/uio.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>

#ifdef WIN32
#define READ_MODE "rb"
#define WRITE_MODE "wb"
#else
#define READ_MODE "r"
#define WRITE_MODE "w"
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>



/*
** Read Normal() and ReadDirect() are prototypes for users to create
** custom icache readers. Performer has its own versions of these
** loaders.
*/

static int ReadNormal(pfImageTile *itile, int nTexels)
{
   int          read;
   int          os,ot,or,w,h,d;
   FILE         *fp;
   int          diskHeader = pfGetImageTileHeaderOffset(itile);
   int          diskTexelSize = pfGetImageTileFileImageTexelSize(itile);
   int          prev = pfGetImageTileValidTexels(itile);
   int          s, t, r; /* which tile in this tile file */
   int		nS, nT, nR; /* how many tiles in this tile file */
   int          fileoffset;
   pfTileFileNameFuncType fnfunc;
        
   pfGetImageTileOrigin(itile,&os,&ot,&or);
   pfGetImageTileSize(itile,&w,&h,&d);
   pfGetImageTileFileTile(itile, &s, &t, &r);
   pfGetImageTileNumFileTiles(itile, &nS, &nT, &nR);
   
   fnfunc = pfGetImageTileFileNameFunc(itile);
   if(fnfunc)
       (*fnfunc)(itile);

   fp = fopen(pfGetImageTileFileName(itile),READ_MODE); 

                 /* offset to proper tile in file */
   fileoffset = (s + nS * (t + nT * r)) * diskTexelSize * w * h * d
                + prev * diskTexelSize /* previous reads */
                + diskHeader; /* application-specific file headers */
   if(fileoffset)
       fseek(fp, fileoffset, SEEK_SET);
   read = fread(pfGetImageTileMem(itile), diskTexelSize, nTexels, fp);
   if (read < nTexels) {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "Normal Read of file %s failed",
		 pfGetImageTileFileName(itile));
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "Only Read %d of %d Texels",
		 read, nTexels);
        /* use default tile if it's available */
        pfImageTileDefaultTileMode(itile, 1);
   }
   fclose(fp);
   return read;
}  

static int ReadDirect(pfImageTile *itile, int nTexels)
{
#if defined(__linux__has_direct_file_io) || !defined(__linux__) && !(defined(WIN32))
   int          bytesRead;
   int          os,ot,or,w,h,d,op;
   char         namebuf[512];
   int          diskTexelSize = pfGetImageTileFileImageTexelSize(itile);
   int          diskHeader = pfGetImageTileHeaderOffset(itile);
   int          prev = pfGetImageTileValidTexels(itile);
   struct dioattr DIO;
   int          s, t, r; /* which tile in this tile file */
   int          nS, nT, nR; /* how many tiles in this tile file */
   int          fileoffset;
   int          psize;
   pfTileFileNameFuncType fnfunc;

   pfGetImageTileOrigin(itile,&os,&ot,&or);
   pfGetImageTileSize(itile,&w,&h,&d);
   pfGetImageTileFileTile(itile, &s, &t, &r);
   pfGetImageTileNumFileTiles(itile, &nS, &nT, &nR);

   pfGetImageTileMemInfo(itile, &psize, NULL);
   if(!psize)
       return ReadNormal(itile, nTexels);

   fnfunc = pfGetImageTileFileNameFunc(itile);
   if(fnfunc)
       (*fnfunc)(itile);

   op = open(pfGetImageTileFileName(itile), O_RDONLY|O_DIRECT);
   if(op == -1)
      return ReadNormal(itile, nTexels);

   fcntl(op,F_SETFL,FDIRECT);
   fcntl(op,F_DIOINFO,&DIO);
                 /* offset to proper tile in file */
   fileoffset = (s + nS * (t + nT * r)) * diskTexelSize * w * h * d
                + prev * diskTexelSize /* previous reads */
                + diskHeader; /* application-specific file headers */
   if(fileoffset)
       lseek(op, fileoffset, SEEK_SET);
 
   bytesRead = read(op,pfGetImageTileMem(itile), diskTexelSize*nTexels);
   if (bytesRead/diskTexelSize < nTexels)
   {
	pfNotify(PFNFY_WARN,PFNFY_PRINT,
		 "Only Read %d texels (%d bytes) of %d in %s",
		 bytesRead/diskTexelSize,bytesRead,nTexels,
		 pfGetImageTileFileName(itile));
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"Direct IO Read of File %s failed",
		 pfGetImageTileFileName(itile));
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"Trying normal fread for File %s",
		 pfGetImageTileFileName(itile));
	close(op);
	return ReadNormal(itile,nTexels);
   }
   if (bytesRead == -1)
	fprintf(stderr,"error on read %d\n",errno);
   close(op);
   return bytesRead/diskTexelSize;
#endif
}

#if 0
/* needs to be altered to a generic C function */
/* Computes the file name to read to load the image tile */
void
autoConfigFileName(pfImageCache *ic, pfImageTile *tile)
{
    static char NULLSTRING[] = "";

    char *dname = NULLSTRING;
    void* argTable[MAX_TILE_FILENAME_UNIQUE_ARGS];
    pfQueue* qs[3];
    int dim[3];
    int dimCnt[3];
    int min, cnt = 0;
    int S, T, R;
    qs[0] = qs[1] = qs[2] = NULL;

    /* MAXIMUM NUMBER OF ARGS for fname == 16
       To increase add args to sprintf below */
    void* args[PFIMAGECACHE_MAX_TILE_FILENAME_ARGS];

    pfGetImageTileTileIndex(&S, &T, &R);

    if (ic->readQueues[0] != NULL)
	if ((qs[cnt] = (pfQueue*)
	     ic->readQueues[0]->get(S % ic->readQueues[0]->getNum())) != NULL)
	{
	    dim[cnt] = 0;
	    dimCnt[cnt] = S % ic->readQueues[0]->getNum();
	    cnt++;
	}
    if (ic->readQueues[1] != NULL)
	if ((qs[cnt] = (pfQueue*)
	     ic->readQueues[1]->get(T % ic->readQueues[1]->getNum())) != NULL)
	{
	    dim[cnt] = 1;
	    dimCnt[cnt] = T % ic->readQueues[1]->getNum();
	    cnt++;
	}
    if (ic->readQueues[2] != NULL)
	if ((qs[cnt] = (pfQueue*)
	     ic->readQueues[2]->get(R % ic->readQueues[2]->getNum())) != NULL)
	{
	    dim[cnt] = 2;
	    dimCnt[cnt] = R % ic->readQueues[2]->getNum();
	    cnt++;
	}
    if (cnt > 0)
    {
	min = qs[0]->getNum();
	int which = 0;
	int ii;
	for(ii = 1; ii < cnt; ii++)
	    if (qs[ii]->getNum() < min)
	    {
		which = ii;
		min = qs[ii]->getNum();
	    }
	if (ic->autoSetQueue && (qs[which] != NULL))
	{
	    tile->setReadQueue(qs[which]);
	}
	if (ic->readNames[dim[which]] != NULL)
	    dname = (char *)
		ic->readNames[dim[which]]->get(dimCnt[which]);
	else
	    dname = NULLSTRING;
    }
    else
	dname = NULLSTRING;

    argTable[PFIMAGECACHE_TILE_FILENAMEARG_STREAMSERVERNAME] =
	(void *)(dname);

    argTable[PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_S] = 
	(void*)(long)ic->imageSize.S;
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_T] =
	(void*)(long)ic->imageSize.T;
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_R] = 
	(void*)(long)ic->imageSize.R;
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_CACHENAME] =
	(void*)ic->name;
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_S] = 
	(void*)(long)S;
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_T] = 
	(void*)(long)T;
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_R] = 
	(void*)(long)R;
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_S] = 
	(void*)(long)tile->s;
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_T] = 
	(void*)(long)tile->t;
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_R] = 
	(void*)(long)tile->r;
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_S] = 
	(void*)((long)(S/tile->nFTilesS));
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_T] = 
	(void*)((long)(T/tile->nFTilesT));
    argTable[PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_R] = 
	(void*)((long)(R/tile->nFTilesR));
    int ii;
    for(ii=0; ii< ic->nArgs; ii++)
	args[ii] = argTable[ic->argList[ii]];
    for(; ii<PFIMAGECACHE_MAX_TILE_FILENAME_ARGS; ii++)
	args[ii]=NULL; /* avoid UMRs */
	
    /* Add args here to increase MAX args... */
    char buf[PF_MAXSTRING + 256];
    sprintf(buf,ic->fNameFmt,
	    args[0], args[1], args[2], args[3],
	    args[4], args[5], args[6], args[7],
	    args[8], args[9], args[10], args[11],
	    args[12], args[13], args[14], args[15]);
    pfImageTileFileName(tile, buf);
}
#endif


/*
** Spec for finding config and tile files:
** 
** 0. base_name: make obsolecent; semantics for null or missing base names are 
**            too confusing. Make missing token ==  missing arg == "" arg ->
**            ignore basename
** 1. expand environment variables in filename
** 2. Add basenames
** 2. relative path: add calling config file path
**      icache config file ->  tiles: add .ic config file path
**      cliptexture config file -> icaches and tiles: add .ct config file path
** 3. apply pfFindFile to resolve files that may in the performer search path
** 4. for save the path for tile files so findfile doesn't have to be
**    re-searched
*/


/* Helper funcs */


typedef struct {
    char *string;
    int enumval;
} FormatInfo;

/* Equivalent to OpenGL type */
static FormatInfo ExternalFormats[] = {
       {"PFTEX_PACK_8", PFTEX_PACK_8},
       {"PFTEX_PACK_16", PFTEX_PACK_16},
       {"PFTEX_PACK_USHORT_5_6_5", PFTEX_PACK_USHORT_5_6_5},
       {"PFTEX_PACK_USHORT_4_4_4_4", PFTEX_PACK_USHORT_4_4_4_4},
       {"PFTEX_UNSIGNED_SHORT_5_5_5_1", PFTEX_UNSIGNED_SHORT_5_5_5_1},
       {"PFTEX_BYTE", PFTEX_BYTE},
       {"PFTEX_UNSIGNED_BYTE", PFTEX_UNSIGNED_BYTE},
       {"PFTEX_SHORT", PFTEX_SHORT},
       {"PFTEX_UNSIGNED_SHORT", PFTEX_UNSIGNED_SHORT},
       {"PFTEX_INT", PFTEX_INT},
       {"PFTEX_UNSIGNED_INT", PFTEX_UNSIGNED_INT},
       {"PFTEX_FLOAT", PFTEX_FLOAT},
       {"GL_UNSIGNED_BYTE", GL_UNSIGNED_BYTE},
       {"GL_BYTE", GL_BYTE},
       {"GL_BITMAP", GL_BITMAP},
       {"GL_UNSIGNED_SHORT", GL_UNSIGNED_SHORT},
       {"GL_SHORT", GL_SHORT},
       {"GL_UNSIGNED_INT", GL_UNSIGNED_INT},
       {"GL_INT", GL_INT},
       {"GL_FLOAT", GL_FLOAT},
#ifdef GL_UNSIGNED_BYTE_3_3_2_EXT
       {"GL_UNSIGNED_BYTE_3_3_2_EXT", GL_UNSIGNED_BYTE_3_3_2_EXT},
#endif
#ifdef GL_UNSIGNED_SHORT_4_4_4_4_EXT
       {"GL_UNSIGNED_SHORT_4_4_4_4_EXT", GL_UNSIGNED_SHORT_4_4_4_4_EXT},
#endif
#ifdef GL_UNSIGNED_SHORT_5_5_5_1_EXT
       {"GL_UNSIGNED_SHORT_5_5_5_1_EXT", GL_UNSIGNED_SHORT_5_5_5_1_EXT},
#endif
#ifdef GL_UNSIGNED_INT_8_8_8_8_EXT
       {"GL_UNSIGNED_INT_8_8_8_8_EXT", GL_UNSIGNED_INT_8_8_8_8_EXT},
#endif
#ifdef GL_UNSIGNED_INT_10_10_10_2_EXT
       {"GL_UNSIGNED_INT_10_10_10_2_EXT", GL_UNSIGNED_INT_10_10_10_2_EXT},
#endif
       {"", -1}
};

/* Equivalent to OpenGL components */
static FormatInfo InternalFormats[] = {
 	{"PFTEX_I_8", PFTEX_I_8},
 	{"PFTEX_I_16", PFTEX_I_16},
 	{"PFTEX_IA_8", PFTEX_IA_8},
 	{"PFTEX_I_12A_4", PFTEX_I_12A_4},
 	{"PFTEX_IA_12", PFTEX_IA_12},
 	{"PFTEX_RGB_4", PFTEX_RGB_4},
 	{"PFTEX_RGB_5", PFTEX_RGB_5},
 	{"PFTEX_RGB_8", PFTEX_RGB_8},
 	{"PFTEX_RGB_12", PFTEX_RGB_12},
 	{"PFTEX_RGBA_8", PFTEX_RGBA_8},
 	{"PFTEX_RGBA_12", PFTEX_RGBA_12},
 	{"PFTEX_RGB5_A1", PFTEX_RGB5_A1},
 	{"PFTEX_RGBA_4", PFTEX_RGBA_4},
 	{"PFTEX_DEFAULT", PFTEX_DEFAULT},
	{"GL_ALPHA", GL_ALPHA},
	{"GL_ALPHA4_EXT", GL_ALPHA4_EXT},
	{"GL_ALPHA8_EXT", GL_ALPHA8_EXT},
	{"GL_ALPHA12_EXT", GL_ALPHA12_EXT},
	{"GL_ALPHA16_EXT", GL_ALPHA16_EXT},
	{"GL_LUMINANCE", GL_LUMINANCE},
	{"GL_LUMINANCE4_EXT", GL_LUMINANCE4_EXT},
	{"GL_LUMINANCE8_EXT", GL_LUMINANCE8_EXT},
	{"GL_LUMINANCE12_EXT", GL_LUMINANCE12_EXT},
	{"GL_LUMINANCE16_EXT", GL_LUMINANCE16_EXT},
	{"GL_LUMINANCE_ALPHA", GL_LUMINANCE_ALPHA},
	{"GL_LUMINANCE4_ALPHA4_EXT", GL_LUMINANCE4_ALPHA4_EXT},
	{"GL_LUMINANCE6_ALPHA2_EXT", GL_LUMINANCE6_ALPHA2_EXT},
	{"GL_LUMINANCE8_ALPHA8_EXT", GL_LUMINANCE8_ALPHA8_EXT},
	{"GL_LUMINANCE12_ALPHA4_EXT", GL_LUMINANCE12_ALPHA4_EXT},
	{"GL_LUMINANCE12_ALPHA12_EXT", GL_LUMINANCE12_ALPHA12_EXT},
	{"GL_LUMINANCE16_ALPHA16_EXT", GL_LUMINANCE16_ALPHA16_EXT},
	{"GL_INTENSITY_EXT", GL_INTENSITY_EXT},
	{"GL_INTENSITY4_EXT", GL_INTENSITY4_EXT},
	{"GL_INTENSITY8_EXT", GL_INTENSITY8_EXT},
	{"GL_INTENSITY12_EXT", GL_INTENSITY12_EXT},
	{"GL_INTENSITY16_EXT", GL_INTENSITY16_EXT},
	{"GL_RGB", GL_RGB},
	{"GL_RGB2_EXT", GL_RGB2_EXT},
	{"GL_RGB4_EXT", GL_RGB4_EXT},
	{"GL_RGB5_EXT", GL_RGB5_EXT},
	{"GL_RGB8_EXT", GL_RGB8_EXT},
	{"GL_RGB10_EXT", GL_RGB10_EXT},
	{"GL_RGB12_EXT", GL_RGB12_EXT},
	{"GL_RGB16_EXT", GL_RGB16_EXT},
	{"GL_RGBA2_EXT", GL_RGBA2_EXT},
	{"GL_RGBA4_EXT", GL_RGBA4_EXT},
	{"GL_RGB5_A1_EXT", GL_RGB5_A1_EXT},
	{"GL_RGBA", GL_RGBA},
	{"GL_RGBA8_EXT", GL_RGBA8_EXT},
	{"GL_RGB10_A2_EXT", GL_RGB10_A2_EXT},
	{"GL_RGBA12_EXT", GL_RGBA12_EXT},
	{"GL_RGBA16_EXT", GL_RGBA16_EXT},
 	{"", -1}
};



typedef struct {
    char *string;
    int format;
    int components; /* how many components */
} ImageFormatInfo;

/* Equivalent to OpenGL formats */
static ImageFormatInfo ImageFormats[] = {
      {"PFTEX_RGB", PFTEX_RGB, 3},
      {"PFTEX_LUMINANCE_ALPHA", PFTEX_LUMINANCE_ALPHA, 2},
      {"PFTEX_LUMINANCE", PFTEX_LUMINANCE, 1},
      {"PFTEX_RGBA", PFTEX_RGBA, 4},
      {"GL_RED" , GL_RED, 1},
      {"GL_GREEN", GL_GREEN, 1},
      {"GL_BLUE", GL_BLUE, 1},
      {"GL_ALPHA", GL_ALPHA, 1},
      {"GL_LUMINANCE", GL_LUMINANCE, 1},
      {"GL_LUMINANCE_ALPHA", GL_LUMINANCE_ALPHA, 2},
      {"GL_RGB", GL_RGB, 3},
      {"GL_RGBA", GL_RGBA, 4},
#ifndef WIN32 // XXX Alex
      {"GL_ABGR_EXT", GL_ABGR_EXT, 4},
#endif
      {"", -1, -1}
};

static ImageFormatInfo *_pfImageFormatStrToEnum(const char *function,
						const char *str)
{
    ImageFormatInfo *format;
    for(format = ImageFormats;*format->string;format++) {
	if(!strcmp(str,format->string))
	    return format;
    }
    pfNotify(PFNFY_WARN, PFNFY_PRINT, 
	     "%s: Invalid Image Format %s", function, str);
    return NULL;
}



int _pfExternalFormatStrToEnum(const char *function, const char *str)
{
    FormatInfo *format;
    int val;
    for(format = ExternalFormats;*format->string;format++) {
	if(!strcmp(str, format->string))
	    return format->enumval;
    }
    if(sscanf(str,"%d", &val)) /* if they just give a number, use it */
	return val;
    else {
	pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		 "%s: Invalid External Format %s", function, str);
	return format->enumval; /* return invalid value */
    }
}


int _pfInternalFormatStrToEnum(const char *function, const char *str)
{
    FormatInfo *format;
    int val;
    for(format = InternalFormats;*format->string;format++) {
	if(!strcmp(str, format->string))
	    return format->enumval;
    }
    if(sscanf(str,"%d", &val)) /* if they just give a number, use it */
	return val;
    else {
	pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		 "%s: Invalid Internal Format %s", function, str);
	return format->enumval; /* return invalid value */
    }
}

typedef struct {
    char *string;
    int val;
} FnameInfo;

static FnameInfo Fnames[] = {
        {"PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_S", 
	     PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_S},
	{"PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_T",
	     PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_T},
	{"PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_R",
	    PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_R},
	{"PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_S",
	     PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_S},
	{"PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_T",
	     PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_T},
	{"PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_R",
	    PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_R},
	{"PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_S",
	    PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_S},
	{"PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_T",
	    PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_T},
	{"PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_R",
	    PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_R},
	{"PFIMAGECACHE_TILE_FILENAMEARG_STREAMSERVERNAME",
	    PFIMAGECACHE_TILE_FILENAMEARG_STREAMSERVERNAME},
	{"PFIMAGECACHE_TILE_FILENAMEARG_CACHENAME",
	    PFIMAGECACHE_TILE_FILENAMEARG_CACHENAME},
	{"PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_S",
	    PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_S},
	{"PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_T",
	    PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_T},
	{"PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_R",
	    PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_R},
	{"", -1}
};

static int _pfFnameArgToNum(const char *str) 
{
    FnameInfo *fnamearg;
    for(fnamearg = Fnames; *fnamearg->string;fnamearg++) {
	if(!strcmp(str, fnamearg->string))
	    return fnamearg->val;
    }
    pfNotify(PFNFY_WARN, PFNFY_PRINT,
             "pfdLoadImageCache: invalid file parameter name; %s\n",
             str);
    return fnamearg->val;
}

typedef struct {
    char *string;
    int token;
} pfCTFnameConvert;

/*
** XXX This is a disgusting merger of two name spaces
** it should be re-architected. It makes it possible to
** use image cache arguments in the the tile_format string of cliptextures
** so that image cache config files can be optional
*/
pfCTFnameConvert CTFnameTokens[] = {
{"PFCLIPTEX_FNAMEARG_LEVEL",
     PFCLIPTEX_FNAMEARG_LEVEL},
{"PFCLIPTEX_FNAMEARG_IMAGE_CACHE_BASE", 
     PFCLIPTEX_FNAMEARG_IMAGE_CACHE_BASE},
{"PFCLIPTEX_FNAMEARG_LEVEL_SIZE",
     PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_S},
{"PFCLIPTEX_FNAMEARG_TILE_BASE", 
     PFIMAGECACHE_TILE_FILENAMEARG_CACHENAME},
{"PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_S", 
     PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_S},
{"PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_T",
     PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_T},
{"PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_R",
     PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_R},
{"PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_S",
     PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_S},
{"PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_T",
     PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_T},
{"PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_R",
     PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_R},
{"PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_S",
     PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_S},
{"PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_T",
     PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_T},
{"PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_R",
     PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_R},
{"PFIMAGECACHE_TILE_FILENAMEARG_STREAMSERVERNAME",
     PFIMAGECACHE_TILE_FILENAMEARG_STREAMSERVERNAME},
{"PFIMAGECACHE_TILE_FILENAMEARG_CACHENAME",
     PFIMAGECACHE_TILE_FILENAMEARG_CACHENAME},
{"PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_S",
     PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_S},
{"PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_T",
     PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_T},
{"PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_R",
     PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_R},
{"", PFCLIPTEX_FNAMEARG_INVALID}
};

/* convert a token string to an integer */
int _pfCTFnameArgToNum(const char *buf)
{
    pfCTFnameConvert *tokenlist;
    for(tokenlist = CTFnameTokens;*tokenlist->string;tokenlist++) {
	if(!strcmp(tokenlist->string, buf))
	    return tokenlist->token;
    }
    pfNotify(PFNFY_WARN, PFNFY_PRINT, 
	     "pfdLoadClipTexture: Invalid Argument Token Name: %s\n", 
	     buf);
    return PFCLIPTEX_FNAMEARG_INVALID;
}

/* Support for Tokens in Config Files */

/* XXX TODO: split tokens for icache and cliptexture */

typedef enum {
    START_OF_FILE = -2,
    BAD_TOKEN, /* -1 */
    CLIP_TEX_VERSION, /* 0 */
    IMAGE_CACHE_VERSION,
    COMMENT,
    EXT_FORMAT,
    INT_FORMAT,
    IMG_FORMAT,
    VSIZE,
    CLIPSIZE,
    INVALID_BORDER,
    EFFECTIVE_LEVELS,
    ALLOCATED_LEVELS,
    LAST_ICACHE_SIZE,
    ICACHE_BASE,
    TILE_BASE,
    ICACHE_FORMAT,
    NUM_ICACHE_PARAMS,
    ICACHE_PARAMS,
    TILE_FORMAT,
    NUM_TILE_PARAMS,
    TILE_PARAMS,
    ICACHE_FILES,
    TILE_FILES,
    CACHE_SIZE,
    VALID_REGION,
    VALID_REGION_DST,
    HEADER_OFFSET,
    TILES_IN_FILE,
    TILE_SIZE,
    NUM_STREAM_SERVERS,
    S_STREAM_SERVERS,
    T_STREAM_SERVERS,
    R_STREAM_SERVERS,
    DEFAULT_TILE,
    LEVEL_PHASE_SHIFT,
    LEVEL_PHASE_MARGIN,
    TEX_REGION,
    MEM_REGION,
    PAGE_SIZE, /* size for reading from disk with direct i/o */
    READ_FUNC, /* user-defined read function */
    LOOKAHEAD, /* mem region border in tiles */
    END_OF_FILE /* run out of file to read */
} Token;

/* Matching a token string to a token */
typedef struct {
    char *str;
    Token token;
} TokStr;

static TokStr TokenStrings[] = {
    {"ct_version1.0", CLIP_TEX_VERSION},
    {"ct_version2.0", CLIP_TEX_VERSION},
    {"ic_version1.0", IMAGE_CACHE_VERSION},
    {"ic_version2.0", IMAGE_CACHE_VERSION}, /* support new icache api */
    {"#", COMMENT}, /* shell-style comment */
    {"//", COMMENT}, /* C++-style comment */
    {";", COMMENT}, /* LISP-style comment */
    {"rem", COMMENT}, /* BASIC-style comment */
    {"comment", COMMENT}, /* ALGOL-style comment */
    {"num_streams", COMMENT}, /* no longer needed */
    {"ext_format", EXT_FORMAT},
    {"int_format", INT_FORMAT},
    {"img_format", IMG_FORMAT},
    {"virt_size", VSIZE}, /* old api for icache -> icache_size */
    {"icache_size", VSIZE}, /* new api */
    {"cache_size", CACHE_SIZE}, /* old api -> mem region size */
    {"valid_region", VALID_REGION}, /* old api -> tex_region_size */
    {"valid_region_dst", VALID_REGION_DST}, /* old api -> tex_size */
    {"tex_size", VALID_REGION_DST}, /* new api */
    {"header_offset", HEADER_OFFSET},
    {"tiles_in_file", TILES_IN_FILE},
    {"tile_size", TILE_SIZE},
    {"tile_base", TILE_BASE},
    {"tile_format", TILE_FORMAT},
    {"num_tile_params", NUM_TILE_PARAMS},
    {"tile_params", TILE_PARAMS},
    {"tile_files", TILE_FILES},
    {"s_stream", S_STREAM_SERVERS}, 
    {"t_stream", T_STREAM_SERVERS}, 
    {"r_stream", R_STREAM_SERVERS}, 
    {"s_streams", S_STREAM_SERVERS},  /* deal with typo in man pages */
    {"t_streams", T_STREAM_SERVERS}, 
    {"r_streams", R_STREAM_SERVERS}, 
    {"default_tile", DEFAULT_TILE},
    {"clip_size", CLIPSIZE},
    {"invalid_border", INVALID_BORDER},
    {"effective_levels", EFFECTIVE_LEVELS},
    {"allocated_levels", ALLOCATED_LEVELS},
    {"smallest_icache", LAST_ICACHE_SIZE},
    {"icache_base", ICACHE_BASE},
    {"icache_format", ICACHE_FORMAT},
    {"num_icache_params", NUM_ICACHE_PARAMS},
    {"icache_params", ICACHE_PARAMS},
    {"icache_files", ICACHE_FILES},
    {"phase_shift", LEVEL_PHASE_SHIFT},
    {"phase_margin", LEVEL_PHASE_MARGIN},
    {"tex_region_size", TEX_REGION}, /* new api */
    {"mem_region_size", MEM_REGION}, /* new api */
    {"page_size", PAGE_SIZE}, /* new api; page size to use for direct i/o */
    {"read_func", READ_FUNC}, /* user-defined read function */
    {"lookahead", LOOKAHEAD}, /* mem region tile border */
    {"start of file", START_OF_FILE}, /* default token values */
    {"", BAD_TOKEN}
};

/*
** Find and return the next token
** Current limitiation: tokens must be surrounded by whitespace
** XXX TODO: remove following whitespace restriction.
*/

static Token isToken(char *str, FILE *fp)
{
    TokStr *token;
    int length;
    char *ptr;

    for(token = TokenStrings;token->token != BAD_TOKEN;token++) {
	if(ptr = strstr(str, token->str)) {
	    if(ptr == str)
	    {
		/* string is more than just token */
		length = strlen(str) - strlen(token->str);
		if(length)
		{
		    /* subtract extra */
		    fseek(fp, -length, SEEK_CUR);
		}
		return token->token;
	    }
	}
    }
    /* couldn't find a match */
    return BAD_TOKEN;
}

static Token nextToken(FILE *fp)
{
    int read;
    Token token;
    char tokstr[256];

    switch(fscanf(fp, "%s", tokstr)) {
    case 0:
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "nextToken(); failed trying to parse string %s\n",
		 tokstr);
	return BAD_TOKEN;
    case EOF:
	return END_OF_FILE;
    default:
	token = isToken(tokstr, fp);
	if(token == BAD_TOKEN)
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "Encountered invalid token \"%s\"\n",
		     tokstr);
	return token;
    }
}

/*
** Find the string associated with the token
** for error reporting
*/
static char *tokenString(Token token)
{
    TokStr *tokens;
    for(tokens = TokenStrings;*tokens->str;tokens++) {
	if(tokens->token == token)
	    return tokens->str;
    }
    return "BAD_TOKEN";
}

#define VERSION_ENFORCE(str)                            \
 if(!version_enforce) {                                 \
    pfNotify(PFNFY_WARN, PFNFY_PRINT, 			\
	     "%s: version token doesn't start file!\n",	\
	     str);					\
    version_enforce = 1;				\
 }


/*
** Parse tile_base or icache_base strings to expand any environment
** variable name at the beginning.
** Assuming that str must be at least a null string
*/
void _pfExpandBase(const char *fname, char *str)
{
    char var[256]; /* variable names */
    char *offset; /* skip over variable name */
    char *value; /* environment variable value */
    char tmp[512]; /* temporary string */
    char *defaultvalue; /* if not defined in environment */

    if(*str != '$')
	return; /* no varible, no expansion */

    *var = 0;
    switch(str[1]) { /* how to parse variable */
    case '(':
    case '{':
	sscanf(&str[2],"%256[^)}]", var);
	break;
    default:
	sscanf(&str[1],"%256[^./]", var);
	break;
    }
    if(*var) {
	*tmp = 0;
	defaultvalue = NULL;
	if ((defaultvalue = strstr(var, ":-")) != NULL)
	{
	    *defaultvalue = '\0';
	    defaultvalue += 2;
	}

	if((value = getenv(var)) != NULL)
	    strcpy(tmp, value); /* if defined */
	else if (defaultvalue != NULL)
	    strcpy(tmp, defaultvalue);
	else { /* if not defined */
	    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT,
		     "%s: variable %s undefined\n", fname, var);
	    *tmp = 0; /* use empty string if variable undefined */
	}
#ifdef WIN32
	if(strchr(tmp, ';')) {
#else
	if(strchr(tmp, ':')) {
#endif
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "%s: Invalid value %s (contains '%c') for variable %s\n",
#ifdef WIN32
		     fname, tmp, ';', var);
#else
		     fname, tmp, ':', var);
#endif
	    return;
	}
	/* skip over variable name part of original string */
	if(offset = strpbrk(str, ")}")) /* $(NAME) or ${NAME} */
	{
	    offset++;
	    strcat(tmp, offset);
	}
	else if(offset = strpbrk(str, "./")) /* $NAME/ or $NAME.[.] */
	    strcat(tmp, offset);
	    
	pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "%s: expanding %s to %s\n",
		 fname, str, tmp);
	strcpy(str, tmp);
	    
	return;
    }
}

void
_pfExpandRelative(const char *funname, const char *path, char *fname)
{
    char *start;
    int count;
    char *end;
    char *tmp;

    if(*fname == '/')
	return; /* not a relative filepath */

    /* remove last filename in path */
    end = strrchr(path, '/');

    if(!end)
	return; /* fname is relative, and path has no slashes */
	
    count = end - path + 1;
    tmp = (char *)malloc(strlen(fname) + count + 1);

    if(!tmp)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "%s _pfExpandRelative malloc failed; no expansion done",
		 funname);
	return;
    }

    strncpy(tmp, path, count);
    tmp[count] = '\0'; /* add the null so it's a string */

    strcat(tmp, fname);

    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "%s: expanding %s to %s\n",
	     funname, fname, tmp);

    strcpy(fname, tmp);
    free(tmp);
}

enum {NONE, FILELIST, FORMAT};

typedef struct {
    int type; /* list of files or format string and args */
    char *base;      /* image cache file base name */
    char *format;    /* image cache name format string */
    int numFnameArgs; /* number of arguments in filename string */
    int fnameTokens[PFCLIPTEX_MAX_NUM_FORMAT_ARGS];
    pfList *files; /* list of user supplied config filenames */
} pfdClipTexData;

void initClipTexData(pfdClipTexData *config)
{
    config->type = NONE; /* a pseudo-union */

    config->base = NULL;

    /* if format string and arguments */
    config->format = NULL;
    config->numFnameArgs = 0;
    config->fnameTokens[0] = -1;

    /* if list of files */
    config->files = NULL;
}

void freeClipTexData(pfdClipTexData *config)
{
    int i;

    /* function should work whether format or files */
    if(config->base)
	free(config->base);
    if(config->format)
	free(config->format);
    if(config->files)
    {
	for(i = 0; i < pfGetNum(config->files); i++)
	    free(pfGet(config->files, i));
	pfDelete(config->files);
    }
}

#define MAX_STR_BUF 2048

typedef char Strbuf[MAX_STR_BUF];


/* Hack to expand tokens into actual parameters values. Used as scanf args */
#define PFCLIPTEX_EVAL_FORMAT_ARGS(TOKENS, NUMARGS, ARGS, STR) \
/* stay below PFCLIPTEX_MAX_NUM_FORMAT_ARGS */            \
for(i=0;i < NUMARGS;i++) {                                \
    switch(TOKENS[i]) {                                   \
    case PFCLIPTEX_FNAMEARG_INVALID:                      \
	pfNotify(PFNFY_WARN, PFNFY_PRINT,                 \
		 "pfuMakeClipTexture: invalid token"      \
		 "value %d found while parsing %s"        \
		 "pfuMakeClipTexture: %s probably wasn't" \
		 "initialized",                           \
		 TOKENS[i], STR, STR);                    \
        return NULL; /* bail out; config file unusable */ \
    case PFCLIPTEX_FNAMEARG_LEVEL:                        \
	ARGS[i] = (void *)level;                          \
	break;                                            \
    case PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_S:           \
	ARGS[i] = (void *)levelSizeS;                     \
	break;                                            \
    case PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_T:           \
	ARGS[i] = (void *)levelSizeT;                     \
	break;                                            \
    case PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_R:           \
	ARGS[i] = (void *)1;                              \
	break;                                            \
    case PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_S:         \
    case PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_T:         \
    case PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_R:         \
    case PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_S:         \
    case PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_T:         \
    case PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_R:         \
    case PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_S:         \
    case PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_T:         \
    case PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_R:         \
        ARGS[i] = (void *)0;	                          \
        break;	                                          \
    case PFIMAGECACHE_TILE_FILENAMEARG_STREAMSERVERNAME:  \
	pfNotify(PFNFY_WARN, PFNFY_PRINT,                 \
		 "pfuMakeClipTexture: sorry, streams"     \
		 "not supported yet for image cache"      \
		 "cliptexture config files");             \
        return NULL; /* bail out; config file unusable */ \
    case PFCLIPTEX_FNAMEARG_IMAGE_CACHE_BASE:             \
	ARGS[i] = (void *)icData->base;                   \
	break;                                            \
    case PFIMAGECACHE_TILE_FILENAMEARG_CACHENAME:         \
	ARGS[i] = (void *)tileData->base;                 \
	break;                                            \
    default:                                              \
	pfNotify(PFNFY_WARN, PFNFY_PRINT,                 \
		 "pfuMakeClipTexture: unknown token"      \
		 "value %d found while parsing %s",       \
		 TOKENS[i], STR);                         \
        return NULL; /* bail out; config file unusable */ \
    }                                                     \
}							  \
for (; i < PFCLIPTEX_MAX_NUM_FORMAT_ARGS; ++i)		  \
    ARGS[i] = NULL;


PFDUDLLEXPORT
pfImageTile *pfdLoadImageTileFormat(pfClipTexture *ct, int level, 
				    struct _pfuClipTexConfig *config)
{
    int i;
    int levelSizeS, levelSizeT;
    pfImageTile *it;
    struct stat dummy; /* for checking the existence of files */
    Strbuf fname;
    pfdClipTexData *tileData, *icData = NULL; /* for macro */
    void *FnameArgs[PFCLIPTEX_MAX_NUM_FORMAT_ARGS];
    void *arena = pfGetSharedArena();
    char itFilePath[2048];

    tileData = (pfdClipTexData *)config->tileData;

    levelSizeS = config->imgSize[PF_S] >> level;
    levelSizeT = config->imgSize[PF_T] >> level;

    PFCLIPTEX_EVAL_FORMAT_ARGS(tileData->fnameTokens,
			       tileData->numFnameArgs,
			       FnameArgs,
			       "image tile tokens");

    PFASSERTALWAYS(PFCLIPTEX_MAX_NUM_FORMAT_ARGS == 6);
    sprintf(fname, tileData->format,
	    FnameArgs[0], FnameArgs[1], FnameArgs[2],
	    FnameArgs[3], FnameArgs[4], FnameArgs[5]);

    if(!pfFindFile(fname, itFilePath, R_OK))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadImageTileFormat: "
		 "couldn't find pyramid image tile file %s, level %d",
		 fname, level);
	return NULL;
    }

    /* check for the existence of all pyramid tile files */
    if(stat(itFilePath, &dummy))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadImageTileFormat: "
		 "couldn't open pyramid image tile %s, level %d ",
		 itFilePath, level);
	return NULL;
    }
    it = pfNewImageTile(arena);
    pfImageTileFileName(it, itFilePath);
    return it;
}

/* zero will give erroneus results */
int powof2(int val)
{
    int i = 0;

    while(val)
    {
	i++;
	val >>= 1;
    }
    return i;
}

PFDUDLLEXPORT
pfImageTile *pfdLoadImageTileFiles(pfClipTexture *ct, int level,
				    struct _pfuClipTexConfig *config)
{
    pfImageTile *it;
    int i;
    int levelSizeS;
    struct stat dummy; /* for checking the existence of files */
    char *fname;
    pfdClipTexData *tileData;
    char itFilePath[2048];
    void *arena = pfGetSharedArena();

    tileData = (pfdClipTexData *)config->tileData;
    levelSizeS = config->imgSize[PF_S] >> level;

    i = powof2(config->minICache[PF_S]) - powof2(levelSizeS) - 1;
    fname = pfGet(tileData->files, i);

    if(!pfFindFile(fname, itFilePath, R_OK))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadImageTileFiles: "
		 "couldn't find pyramid image tile file %s, level %d",
		 fname, level);
	return NULL;
    }

    /* check for the existence of all pyramid tile files */
    if(stat(itFilePath, &dummy))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadImageTileFiles: pyramid image tile %s, level %d "
		 "doesn't exist", itFilePath, level);
	return NULL;
    }
    it = pfNewImageTile(arena);
    pfImageTileFileName(it, itFilePath);
    return it;
}

/* XXX TODO: Document the circumstances when this function would be used */
/* use default names */
PFDUDLLEXPORT 
pfImageTile *pfdLoadImageTileDefault(pfClipTexture *ct, int level,
				     struct _pfuClipTexConfig *config)
{
    void *arena = pfGetSharedArena();
    return  pfNewImageTile(arena);
}


PFDUDLLEXPORT
pfImageCache *pfdLoadImageCacheFormat(pfClipTexture *ct, int level,
				    struct _pfuClipTexConfig *config)
{
    int i;
    int levelSizeS, levelSizeT;
    pfdClipTexData *icData, *tileData = NULL; /* for macro */
    Strbuf fname;
    pfImageCache *ic;
    pfuImgCacheConfig icConfig;
    void *FnameArgs[PFCLIPTEX_MAX_NUM_FORMAT_ARGS];

    icData = (pfdClipTexData *)config->icData;

    levelSizeS = config->imgSize[PF_S] >> level;
    levelSizeT = config->imgSize[PF_T] >> level;

    PFCLIPTEX_EVAL_FORMAT_ARGS(icData->fnameTokens, 
			       icData->numFnameArgs, 
			       FnameArgs,
			       "image cache tokens");

    PFASSERTALWAYS(PFCLIPTEX_MAX_NUM_FORMAT_ARGS == 6);
    sprintf(fname, icData->format,
	    FnameArgs[0], FnameArgs[1], FnameArgs[2],
	    FnameArgs[3], FnameArgs[4], FnameArgs[5]);

    pfuInitImgCacheConfig(&icConfig);

    /* pass parameters from cliptex config to icache config as needed */
    icConfig.pageSize = config->pageSize;
    icConfig.readFunc = config->readFunc;
    icConfig.lookahead = config->lookahead;
    icConfig.clipSize = config->clipSize;

    return pfdLoadImageCacheState(fname, (pfTexture *)ct, level, &icConfig);
}

PFDUDLLEXPORT
pfImageCache *pfdLoadImageCacheFiles(pfClipTexture *ct, int level,
				    struct _pfuClipTexConfig *config)
{
    char *fname;
    pfdClipTexData *icData;
    pfuImgCacheConfig icConfig;
    icData = (pfdClipTexData *)config->icData;

    fname = pfGet(icData->files, level);

    pfuInitImgCacheConfig(&icConfig);

    /* pass parameters from cliptex config to icache config as needed */
    icConfig.pageSize = config->pageSize;
    icConfig.readFunc = config->readFunc;
    icConfig.lookahead = config->lookahead;
    icConfig.clipSize = config->clipSize;

    return pfdLoadImageCacheState(fname, (pfTexture *)ct, level, &icConfig);
}

PFDUDLLEXPORT
pfImageCache *pfdLoadImageCacheAuto(pfClipTexture *ct, int level, 
				    struct _pfuClipTexConfig *config)
{
    int i;
    pfImageCache *ic;
    pfuImgCacheConfig icConfig;
    pfdClipTexData *icData;
    pfdClipTexData *tileData;
    int levelSizeS, levelSizeT;

    levelSizeS = config->imgSize[PF_S] >> level;
    levelSizeT = config->imgSize[PF_T] >> level;
    
    icData = (pfdClipTexData *)config->icData;
    tileData = (pfdClipTexData *)config->tileData;

    pfuInitImgCacheConfig(&icConfig);

    icConfig.extFormat  = config->extFormat;
    icConfig.intFormat  = config->intFormat;
    icConfig.imgFormat  = config->imgFormat;
    icConfig.components = config->components;

    icConfig.size[PF_S] = levelSizeS;
    icConfig.size[PF_T] = levelSizeT;
    icConfig.size[PF_R] = 1;


    /* paolo: if header_offset was specified in .ct file    */
    /* we want this value to be valid for each imgeTile	    */
    /* created by each imageCache..			    */
    /*							    */
    /* This was not here before pf2.5 so I guess it might   */
    /* cause problems if behavior changes now..		    */
    /* set env. variable:				    */
    /*    PF_CLIPTEX_DONT_USE_CT_HEADEROFFSET_FOR_ICACHES   */
    /* if you need the 'old' behavior			    */
    
    if(!getenv("PF_CLIPTEX_DONT_USE_CT_HEADEROFFSET_FOR_ICACHES"))
    {
	icConfig.header = config->hdrOffset;
    }








    /* tilesize */
    
    if(config->tileSize[PF_S] < 0 ||
       config->tileSize[PF_T] < 0 ||
       config->tileSize[PF_R] < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: you must provide image "
		 "tile sizes if no image caches are specified");
    }

    icConfig.tileSize[PF_S] = 
	PF_MIN2(config->tileSize[PF_S], levelSizeS);
    icConfig.tileSize[PF_T] = 
	PF_MIN2(config->tileSize[PF_T], levelSizeT);
    icConfig.tileSize[PF_R] = config->tileSize[PF_R];

    icConfig.texRegSize[PF_S] = PF_MIN2(config->clipSize, levelSizeS);
    icConfig.texRegSize[PF_T] = PF_MIN2(config->clipSize, levelSizeT);
    icConfig.texRegSize[PF_R] = 1;

    icConfig.texSize[PF_S] = PF_MIN2(config->clipSize, levelSizeS);
    icConfig.texSize[PF_T] = PF_MIN2(config->clipSize, levelSizeT);
    icConfig.texSize[PF_R] = 1;

    if(!tileData->format)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: must provide tile file format "
		 "if no image caches supplied");
    }

    /*
    ** Semantics brain-dead for no.ic files with tile_base defined
    ** we'll de-support tile_base and icace_base over time (not needed
    ** anymore anyway).
    **
    ** tileData->base can probably never equal "" anymore; (empty
    ** string comes from undefined environment variable expansion)
    ** relative pathname expansion will replace it with a path, but
    ** changing it just before 2.2 release is too dangerous; I'll
    ** just add the check for null pointer. Enviroment variable
    ** expansion should probably be overhauled. -tomcat
    */
    if(tileData->base)
	if(*tileData->base == '\0') /* if empty string */
	    icConfig.base = "./"; /* guess that base should be this directory */
	else
	    icConfig.base = tileData->base;

    icConfig.format = tileData->format;
    icConfig.numParams = tileData->numFnameArgs;
    for(i = 0; i < icConfig.numParams; i++)
	icConfig.args[i] = tileData->fnameTokens[i];

    icConfig.pageSize = config->pageSize;
    icConfig.readFunc = config->readFunc;
    icConfig.lookahead = config->lookahead;
    icConfig.clipSize = config->clipSize;


    /* default tile (use tile base) */

    ic = pfuMakeImageCache((pfTexture *)ct, level, &icConfig);
    /* don't free these; they're shared */
    icConfig.base = NULL;
    icConfig.format = NULL;
    pfuFreeImgCacheConfig(&icConfig);

    if (config->lookahead != 1 && level == 0) /* only do this once per cliptex*/
    {
	pfNotify(PFNFY_DEBUG, PFNFY_MORE,  " ");
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "WARNING: pfdLoadClipTexture():");
	pfNotify(PFNFY_DEBUG, PFNFY_MORE,  " no-ic config file \"%s\"",
					   config->name);
	pfNotify(PFNFY_DEBUG, PFNFY_MORE,  " contains the 'lookahead' token;");
	pfNotify(PFNFY_DEBUG, PFNFY_MORE,  " this was broken in Performer2.2 and 2.2.1");
	pfNotify(PFNFY_DEBUG, PFNFY_MORE,  " and is now fixed, so your program may");
	pfNotify(PFNFY_DEBUG, PFNFY_MORE,  " use more memory than before; please tune your");
	pfNotify(PFNFY_DEBUG, PFNFY_MORE,  " configuration file and application accordingly.");
	pfNotify(PFNFY_DEBUG, PFNFY_MORE,  " ");
    }

    return ic;
}




/*
** Read in Configuration information to Load a Clip Texture
**
** Arguments: 
**
** configuration file name
**
** Configuration File Clip Texture Format:
**
** comments: start "#" "comment" or "//" until end of line. May go anywhere
**           a token is expected.
**
** Treat tokens as if order dependent. Some tokens and values can be
** omitted to use defaults. Don't use token without a value.
**
** ct_version1.0 must be at top of file
** ext_format <format string> (external format of stored texels)
** int_format <format string> (internal format used by graphics hw)
** img_format <format string> (image format of stored texels)
** virt_size  <int> <int> <int> (size of entire texture)
** clip_size  <int> (size of square of texture levels in hardware)
** invalid_border  <int> ()
** smallest_icache <int> (dimensions of lowest icache level in clip texture)

** phase_shift <int list> (list of phase shift pairs (s,t) for each level)
** phase_margin <int list> (list of phase margins for each level)
**
** icache_base   <file path string> (root of icache config filenames)
** icache_format <scanf string> (icache config fnames  no string: list files)
** num_icache_params <int> (number of format arguments to follow)
** icache_params <string list> (format tokens in order)
**
** header_offset <int> (byte offset to skip user's tile file headers)
** tile_base   <file path string> (root of pyramid tile filenames)
** tile_format <scanf string> (pyramid tile fnames no string; list tile files)
** num_tile_params <int> (number of format arguments to follow)
** tile_params   <string list> (format tokens in order)
**
** icache_files <list of filenames> (only if icache_format is default )
** tile_files <list of filenames> (pyramid part; only if tile_format default)
**
*/

PFDUDLLEXPORT pfClipTexture*
pfdLoadClipTexture(const char *fileName)
{
    return pfdLoadClipTextureState(fileName, NULL);
}


PFDUDLLEXPORT pfClipTexture *
pfdLoadClipTextureState(const char *fileName, pfuClipTexConfig *state)
{
    Strbuf buf;
    FILE *fp;
    pfClipTexture *ct;
    pfuClipTexConfig stateInfo;
    int i, j, len, cnt;
    int done = FALSE; /* done parsing file? */
    int failed = FALSE; /* to find multiple errors */
    Token lastToken = START_OF_FILE; /* for analyzing parse failures */
    Token curToken =  START_OF_FILE;
    int version_enforce = 0;
    int iCacheType = 0; /* how icache file is specified */
    char ctFilePath[2048]; /* get this file's filepath for relative pathnames*/
    pfdClipTexData icData, tileData; 
    pfdClipTexData *data[2];
    char **strptr, *str;

    initClipTexData(&icData);
    initClipTexData(&tileData);

    /* pfdOpenFile should be using this */
    if(!pfFindFile(fileName, ctFilePath, R_OK))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadClipTexure: couldn't find clip texture config file %s",
		 fileName);
	return NULL;
    }

    if ((fp = pfdOpenFile(ctFilePath)) == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadClipTexure: couldn't open clip texture config file %s",
		 fileName);
	return NULL;
    }

    if(!state)
    {
	state = &stateInfo;
	pfuInitClipTexConfig(state);
    }

    /*
    ** XXX in the future, the user can override name by 
    ** providing a name token 
    */
    state->name = ctFilePath; /* give this cliptexture a default name */

    while(!done) {
	switch(curToken = nextToken(fp)) {
	case CLIP_TEX_VERSION:
	    if(lastToken != START_OF_FILE) {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture: "
			 "version token doesn't start file!\n");
	    }
	    version_enforce = 1;
	    break;
	case COMMENT:
	    /* Comment from token to end of current line */
	    while(fscanf(fp, "%*[^\n]"));
	    lastToken = COMMENT;
	    break;
	case EXT_FORMAT:
	    /* Read in External Format of Image Tile */
	    fscanf(fp,"%s",buf); 
	    state->extFormat = 
		_pfExternalFormatStrToEnum("pfdLoadClipTexture", buf);
	    if(state->extFormat < 0)
		failed = TRUE;
	    lastToken = EXT_FORMAT;
	    break;
	case INT_FORMAT:
	    /* Read in Internal Format of Image Tile */
	    fscanf(fp,"%s",buf); 
	    state->intFormat = 
		_pfInternalFormatStrToEnum("pfdLoadClipTexture", buf);
	    if(state->intFormat < 0)
		failed = TRUE;
	    lastToken = INT_FORMAT;
	    break;
	case IMG_FORMAT:
	{
	    /* Read in Image Format of Image Tile */
	    ImageFormatInfo *imageFormat;
	    fscanf(fp, "%s", buf);
	    imageFormat = _pfImageFormatStrToEnum("pfdLoadClipTexture", buf);
	    if(!imageFormat)
		 failed = TRUE;
	    else
	    {
		state->imgFormat = imageFormat->format;
		state->components = imageFormat->components;
	    }
	    lastToken = IMG_FORMAT;
	    break;
	}
	case VSIZE:
	    /* Read Image (Virtual) Size */
	    fscanf(fp, "%d %d %d", 
		   &state->imgSize[PF_S],
		   &state->imgSize[PF_T],
		   &state->imgSize[PF_R]);
	    lastToken = VSIZE;
	    VERSION_ENFORCE("pfdLoadClipTexture");
	    break;
	case CLIPSIZE:
	    /* Read Clip Size */
	    fscanf(fp, "%d", &state->clipSize);
	    lastToken = CLIPSIZE;
	    break;
	case INVALID_BORDER:
	    /* Read Invalid Border Value */
	    fscanf(fp, "%d", &state->invBorder);
	    lastToken = INVALID_BORDER;
	    break;
	case EFFECTIVE_LEVELS:
	    /* Read Effective Levels Value */
	    fscanf(fp, "%d", &state->effLevels);
	    lastToken = EFFECTIVE_LEVELS;
	    break;
	case ALLOCATED_LEVELS:
	    /* Read Allocated Levels Value */
	    fscanf(fp, "%d", &state->allocLevels);
	    lastToken = ALLOCATED_LEVELS;
	    break;
	case TILE_SIZE:
	    /* Read in Tile Width,Height,Depth specified in texels */
	    /* Only used if no image caches specified */
	    fscanf(fp,"%d %d %d",
		   &state->tileSize[PF_S],
		   &state->tileSize[PF_T],
		   &state->tileSize[PF_R]);
	    lastToken = TILE_SIZE;
	    break;
	case LAST_ICACHE_SIZE:
	    fscanf(fp, "%d %d %d", 
		   &state->minICache[PF_S],
		   &state->minICache[PF_T],
		   &state->minICache[PF_R]);
	    lastToken = LAST_ICACHE_SIZE;
	    break;
	case LEVEL_PHASE_SHIFT: 
	    /* obsolete; ignore */
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "pfdLoadClipTexture: phase shift is no longer "
		     "supported. Arguments ignored");
	    while(1) /* consume arguments */
	    {
		int status;
		status = fscanf(fp, "%s", buf);
		if(status == 0 || status == EOF)
		    break;
		if(isToken(buf, fp) != BAD_TOKEN) /* done reading params */
		{
		    fseek(fp, -strlen(buf), SEEK_CUR);
		    break;
		}
	    }
	    break;
	case LEVEL_PHASE_MARGIN:
	    /* obsolete; ignore */
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "pfdLoadClipTexture: phase margin is no longer "
		     "supported. Arguments ignored");
	    while(1) /* consume arguments */
	    {
		int status;
		status = fscanf(fp, "%s", buf);
		if(status == 0 || status == EOF)
		    break;
		if(isToken(buf, fp) != BAD_TOKEN) /* done reading params */
		{
		    fseek(fp, -strlen(buf), SEEK_CUR);
		    break;
		}
	    }
	    break;
	case ICACHE_BASE:
	    /* get image cache fname base */
	{
	    int status;
	    status = fscanf(fp, "%s", buf); 
	    if(isToken(buf, fp) != BAD_TOKEN || /* no usable argument */
		status == EOF || status == 0) 
	    {
		/* unread next token */
		if(status != 0 && status != EOF)
		    fseek(fp, -strlen(buf), SEEK_CUR);
		buf[0] = '\0'; /* use empty string */
	    }
	    _pfExpandBase("pfdLoadClipTexture", buf); /* expand env var */
	    if(icData.base)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture: image cache filename base "
			 "can only be set once");
		failed = TRUE;
	    }
	    else
	    {
		icData.base = (char *)malloc(strlen(buf) + 1);
		strcpy(icData.base, buf);
	    }
	    lastToken = ICACHE_BASE;
	}
	    break;
	case TILE_BASE:
	    /* get tile fname base */
	{
	    int status;
	    status = fscanf(fp, "%s", buf); 
	    if(isToken(buf, fp) != BAD_TOKEN || /* no usable argument */
		status == EOF || status == 0) 
	    {
		/* unread next token */
		if(status != 0 && status != EOF)
		    fseek(fp, -strlen(buf), SEEK_CUR);
		buf[0] = '\0'; /* use empty string */
	    }
	    _pfExpandBase("pfdLoadClipTexture", buf); /* expand env var */
	    if(tileData.base)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture: image tile filename base "
			 "can only be set once");
		failed = TRUE;
	    }
	    else
	    {
		tileData.base = (char *)malloc(strlen(buf) + 1);
		strcpy(tileData.base, buf);
	    }
	    lastToken = TILE_BASE;
	}
	    break;
	case ICACHE_FORMAT:
	    /* get image cache file name format */
	    fscanf(fp, "%s", buf);
	    if(icData.format)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture: image icache filename format "
			 "can only be set once");
		failed = TRUE;
	    }
	    else
	    {
		_pfExpandBase("pfdLoadClipTexture", buf); /* expand env var */
		icData.type |= FORMAT;
		icData.format = (char *)malloc(strlen(buf) + 1);
		strcpy(icData.format, buf);
	    }
	    lastToken = ICACHE_FORMAT;
	    break;
	case NUM_ICACHE_PARAMS:
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "pfdLoadClipTexture: num_icache_params obsolete. "
		     "Argument ignored");
	    fscanf(fp, "%*d");
	    lastToken = NUM_ICACHE_PARAMS;
	    break;
	case ICACHE_PARAMS:
	    icData.type |= FORMAT;
	    for(i = 0; 1; i++) 
	    {
		int status;
		status = fscanf(fp, "%s", buf);
		if(status == 0 || status == EOF)
		    break;
		if(isToken(buf, fp) == BAD_TOKEN) /* still reading params */
		{
		    int arg;
		    arg = _pfCTFnameArgToNum(buf);
		    if(arg == PFCLIPTEX_FNAMEARG_INVALID)
		    {
			pfNotify(PFNFY_WARN, PFNFY_PRINT,
				 "pfdLoadClipTexture: invalid image cache "
				 "parameter token");
			failed = TRUE;
		    }
		    if(i < PFCLIPTEX_MAX_NUM_FORMAT_ARGS)
			icData.fnameTokens[i] = arg;
		}
		else /* unread last fscanf so nextToken() call works */
		{
		    fseek(fp, -strlen(buf), SEEK_CUR);
		    break;
		}
	    }
	    if(i > PFCLIPTEX_MAX_NUM_FORMAT_ARGS)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture: number of image cache filename "
			 "arguments (%d) exceeds maximum (%d)", 
			 i, PFCLIPTEX_MAX_NUM_FORMAT_ARGS);
		failed = TRUE;
	    }	    
	    icData.numFnameArgs = i;
	    lastToken = ICACHE_PARAMS;
	    break;
	case HEADER_OFFSET:
	    /* Read in file header offset */
	    fscanf(fp, "%d", &state->hdrOffset);
	    lastToken = HEADER_OFFSET;
	    break;	    
	case TILE_FORMAT:
	    /* get tile file name format */
	    fscanf(fp, "%s", buf);
	    if(tileData.format)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture: image tile filename format "
			 "can only be set once");
		failed = TRUE;
	    }
	    else
	    {
		_pfExpandBase("pfdLoadClipTexture", buf); /* expand env var */
		tileData.type |= FORMAT;
		tileData.format = (char *)malloc(strlen(buf) + 1);
		strcpy(tileData.format, buf);
	    }
	    lastToken = TILE_FORMAT;
	    break;
	case NUM_TILE_PARAMS:
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "pfdLoadClipTexture: num_tile_params obsolete. "
		     "Argument ignored");
	    fscanf(fp, "%*d");
	    lastToken = NUM_TILE_PARAMS;
	    break;
	case TILE_PARAMS:
	    tileData.type |= FORMAT;
	    for(i = 0;1;i++) /* read everything until the next valid token */
	    {
		int status;
		status = fscanf(fp, "%s", buf);
		if(status == 0 || status == EOF)
		    break;
		if(isToken(buf, fp) == BAD_TOKEN) /* still reading params */
		{
		    int arg;
		    arg = _pfCTFnameArgToNum(buf);
		    if(arg == PFCLIPTEX_FNAMEARG_INVALID)
		    {
			pfNotify(PFNFY_WARN, PFNFY_PRINT,
				 "pfdLoadClipTexture: invalid tile "
				 "parameter token");
			failed = TRUE;
		    }
		    if(i < PFCLIPTEX_MAX_NUM_FORMAT_ARGS)
			tileData.fnameTokens[i] = arg;
		}
		else /* unread last fscanf so nextToken() call works */
		{
		    fseek(fp, -strlen(buf), SEEK_CUR);
		    break;
		}
	    }
	    if(i > PFCLIPTEX_MAX_NUM_FORMAT_ARGS)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture: number of tile filename arguments"
			 " (%d) exceeds maximum (%d)", 
			 i, PFCLIPTEX_MAX_NUM_FORMAT_ARGS);
		failed = TRUE;
	    }	    
	    tileData.numFnameArgs = i;
	    lastToken = TILE_PARAMS;
	    break;
	case ICACHE_FILES:
	    if(icData.files)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture: image icache filename format "
			 "can only be set once");
		failed = TRUE;
	    }
	    else
	    {
		icData.type |= FILELIST;
		icData.files = pfNewList(sizeof(char *), 10, NULL);
		for(i = 0;1; i++) {
		    int status;
		    status = fscanf(fp, "%s", buf);
		    if(status == 0 || status == EOF)
			break; /* invalid token or end of file */
		    if(isToken(buf, fp) == BAD_TOKEN) /* still reading streams */
		    {
			int length;
			char *icache;
			_pfExpandBase("pfdLoadClipTexture", 
				      buf); /* expand env var */
			length = strlen(buf) + 1;
			if(icData.base)
			    length += strlen(icData.base);

			icache = (char *)malloc(length);
			if(icData.base)
			{
			    strcpy(icache, icData.base);
			    strcat(icache, buf);
			}
			else
			    strcpy(icache, buf);

			pfAdd(icData.files, icache);
		    }
		    else /* "unread" last fscanf so nextToken() call works */
		    {
			fseek(fp, -strlen(buf), SEEK_CUR);
			break;
		    }
		}
	    }
	    lastToken = ICACHE_FILES;
	    break;
	case TILE_FILES:
	    if(tileData.files)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture: image icache filename format "
			 "can only be set once");
		failed = TRUE;
	    }
	    else
	    {
		tileData.type |= FILELIST;
		tileData.files = pfNewList(sizeof(char *), 10, NULL);
		for(i = 0;1; i++) {
		    int status;
		    status = fscanf(fp, "%s", buf);
		    if(status == 0 || status == EOF)
			break; /* invalid token or end of file */
		    if(isToken(buf, fp) == BAD_TOKEN) /* still reading streams */
		    {
			int length;
			char *tile;
			_pfExpandBase("pfdLoadClipTexture", 
				      buf); /* expand env var */
			length = strlen(buf) + 1;
			if(tileData.base)
			    length += strlen(tileData.base);

			tile = (char *)malloc(length);
			if(tileData.base)
			{
			    strcpy(tile, tileData.base);
			    strcat(tile, buf);
			}
			else
			    strcpy(tile, buf);
			pfAdd(tileData.files, tile);
		    }
		    else /* "unread" last fscanf so nextToken() call works */
		    {
			fseek(fp, -strlen(buf), SEEK_CUR);
			break;
		    }
		}
	    }
	    lastToken = TILE_FILES;
	    break;
	case END_OF_FILE:
	    done = 1;
	    lastToken = END_OF_FILE;
	    break;
	default:
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "Tried to use image cache config file token");
	    pfNotify(PFNFY_WARN, PFNFY_MORE,
		     "%s in clip texture config file. Failed ",
		     tokenString(curToken));
	case BAD_TOKEN:
	    pfNotify(PFNFY_WARN, PFNFY_MORE, 
		     "in pfdLoadClipTexture after parsing token %s\n",
		     tokenString(lastToken));
	    failed = TRUE;
	case PAGE_SIZE:
	    /* Read in page size for direct i/o; info passed to icaches */
	    if(fscanf(fp, "%d", &state->pageSize) != 1)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture(): couldn't read page_size "
			 "argument (should be positive small integer). "
			 "using default value.");
		state->pageSize = -1;
	    }
	    lastToken = PAGE_SIZE;
	    break;
	case READ_FUNC: 
	{ /* 1 arg: function name (from executable). 2 args: dso funcname */
	    int status;
	    char *dsoname;
	    char *readname;
	    void *dso;
	    status = fscanf(fp, "%s", buf);
	    if(isToken(buf, fp) != BAD_TOKEN || /* no usable argument */
		status == EOF || status == 0) 
	    {
		/* unread next token */
		if(status != 0 && status != EOF)
		    fseek(fp, -strlen(buf), SEEK_CUR);
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture: read function "
			 "requires 1 or 2 arguments; "
			 "using default read function");
		lastToken = READ_FUNC;
		break;
	    }
	    
	    /* this may turn out to be fname (no dso arg) */
	    dsoname = (char *)malloc(sizeof(buf) + 1);
	    strcpy(dsoname, buf);

	    status = fscanf(fp, "%s", buf);
	    if(isToken(buf, fp) != BAD_TOKEN || /* no usable argument */
		status == EOF || status == 0) 
	    {
		/* unread next token */
		if(status != 0 && status != EOF)
		    fseek(fp, -strlen(buf), SEEK_CUR);
		readname = dsoname;
		dsoname = NULL;
	    }
	    else
	    {
		readname = (char *)malloc(sizeof(buf) + 1);
		strcpy(readname, buf);
	    }

#ifndef WIN32
	    dso = dlopen(dsoname, RTLD_LAZY);
#else
	    dso = LoadLibrary(dsoname);
#endif
	    if(!dso && dsoname)
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
			 "pfuMakeClipTexture(): can't find DSO %s for "
			 "read function; looking for %s in executable",
			 dsoname, readname);

	    if(!dso)
#ifndef WIN32
		dso = dlopen(NULL,  RTLD_LAZY);
#else
 	        dso = GetModuleHandle(NULL);
#endif

	    if(!dso)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture(): could open read function dso; "
			 "using default read function");
		lastToken = READ_FUNC;
		break;
	    }
#ifndef WIN32
	    state->readFunc = (pfReadImageTileFuncType)dlsym(dso, readname);
#else
	    state->readFunc = (pfReadImageTileFuncType)GetProcAddress(dso, readname);
#endif

	    if(!state->readFunc)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
			 "pfdLoadClipTexture(): couldn't find %s symbol; "
			 "using default read function",
			 readname);
		
	    }
	    lastToken = READ_FUNC;
	    break;
	} /* READ_FUNC */
	case LOOKAHEAD:
	    if(fscanf(fp,"%d", &state->lookahead) != 1)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture(): couldn't read lookahead "
			 "argument (should be positive small integer). "
			 "using default border of 1. ");
		state->lookahead = 1;
	    }
	    lastToken = LOOKAHEAD;
	    break;
	} /* switch statement */
    } /* while loop */




    /* make sure clipSize is small enough to fit in texture memory */
    {
	int texelbytes;
	
	switch( state->intFormat )
	{
	    case PFTEX_I_8:
	    case PFTEX_RGB_4: /* in IRISGL, RGB_4 is RGB_5 */
	    case PFTEX_IA_8:
	    case PFTEX_I_12A_4:
	    case PFTEX_RGB_5:
	    case PFTEX_RGB5_A1:
	    case PFTEX_RGBA_4:
	    case PFTEX_I_16:
		texelbytes = 2;
		break;
    
	    case PFTEX_RGB_12: /* 4.5 bytes, padded to 6 */
	    case PFTEX_RGBA_12:
		texelbytes = 6;
		break;
		
	    case PFTEX_RGBA_8:
	    default:
		texelbytes = 4;
		break;	    
	}
    
	if( state->clipSize > pfGetClipTextureMaxClipSize(texelbytes) )
	{
	    state->clipSize = pfGetClipTextureMaxClipSize(texelbytes);
	    pfNotify( PFNFY_NOTICE, PFNFY_PRINT, 
		"pfdLoadclipTexture: shrinking clipSize to %d in order to fit"
		" in texture memory", 	state->clipSize );
	} 
	
    }










    /* expand relative pathnames. Use old method (expand base name)
    ** if tileData.base is set, then it is expanded, otherwise expand
    ** tileData.format or tileData.files, whichever was set
    ** if icData.base is set, then it is expanded otherwise expand
    ** icData.format or icData.files, whichever was set
    */
    data[0] = &icData;
    data[1] = &tileData;

    
    for(i = 0; i < 2; i++)
    {
	cnt = 0; /* for noic files: no image cache base, format, or files */
	strptr = NULL;
	if(data[i]->base) /*expand the base */
	{
	    strptr = &data[i]->base;
	    cnt = 1;
	}
	else
	{
	    if(data[i]->type == FORMAT) /* expand format string */
	    {
		strptr = &data[i]->format;
		cnt = 1;
	    }
	    if(data[i]->type ==  FILELIST) /* expand list of files */
	    {
		strptr = NULL;
		cnt = pfGetNum(data[i]->files);
	    }
	}

	if(strptr || cnt) /* avoid core dump if nothing set */
	    for(j = 0; j < cnt; j++)
	    {
		if(data[i]->type == FILELIST)
		{
		    str = (char *)pfGet(data[i]->files, j);
		    strptr = &str;
		}

		len = strlen(*strptr);
		strcpy(buf, *strptr);
		_pfExpandRelative("pfdLoadClipTexture", ctFilePath, buf);
		if(strlen(buf) > len)
		    *strptr = (char *)realloc(*strptr, strlen(buf) + 1);
		strcpy(*strptr, buf);
		if(data[i]->type == FILELIST)
		    pfSet(data[i]->files, j, *strptr);
	    }	
    }
	

    /* set the functions needed to configure data */
    state->icData = &icData;
    state->tileData = &tileData;

    switch(icData.type) {
    case FORMAT:
	state->icFunc = pfdLoadImageCacheFormat;
	break;
    case FILELIST:
	state->icFunc = pfdLoadImageCacheFiles;
	break;
    case NONE: /* XXX TODO: semantic conflict! need another way to do auto */
	state->icFunc = pfdLoadImageCacheAuto;
	break;
    default: /* FORMAT | FILELIST */
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuLoadClipTexture: both format string and file list method "
		 "used to configure image caches");
        failed = TRUE;
	break;
    }

    switch(tileData.type) {
    case FORMAT:
	state->tileFunc = pfdLoadImageTileFormat;
	break;
    case FILELIST:
	state->tileFunc = pfdLoadImageTileFiles;
	break;
    case NONE:
	state->tileFunc = pfdLoadImageTileDefault;
	break;
    default: /* FORMAT|FILELIST  or NONE*/
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuLoadClipTexture: either format string or file list method "
		 "must be used to configure image tile");
        failed = TRUE;
	break;
    }

    if(failed)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		 "pfdLoadClipTexture: "
		 "parse of clip texture config file %s failed",
		 fileName);
	freeClipTexData(&icData);
	freeClipTexData(&tileData);
	pfuFreeClipTexConfig(state);
	return NULL;
    }
    ct = pfuMakeClipTexture(state);
    freeClipTexData(&icData);
    freeClipTexData(&tileData);
    pfuFreeClipTexConfig(state);
    return ct; 
}



PFDUDLLEXPORT pfMPClipTexture *
pfdLoadMPClipTexture(const char *_fileName)
{
    pfMPClipTexture *mpct = pfNewMPClipTexture();
    pfClipTexture *ct = pfdLoadClipTexture(_fileName);
    if(ct == NULL)
    {
	pfDelete(mpct);
	return NULL;
    }
    pfMPClipTextureClipTexture(mpct, ct);
    return mpct;
}




/*
** Read in Configuration Information to Load an Image Cache.
**
** Arguments:
**
** configuration file name
** destination texture
** destination texture level
**
** Configuration File Cache Format:
**
** comments: start "#" "comment" or "//" until end of line. May go anywhere
**           a token is expected.
**
** Treat tokens as if order dependent. Some tokens and values can be
** omitted to use defaults. Don't use token without a value.
**
** (old)ic_version1.0 must be at top of file
** ic_version2.0 must be at top of file
** ext_format <string> (external format of stored texels)
** int_format <string> (internal format used by graphics hw)
** img_format <string> (image format of stored texels)
**
** (old)virt_size <int> <int> <int> (area in texels of entire tex)
** icache_size <int> <int> <int> (texel area of entire image cache)
** (old)cache_size <int> <int> (number of rows and columns of tiles in memory)
** mem_region_size <int> <int> (tile area downloaded to image cache memory)
** (old)valid_region <int> <int> <int> (texel area downloaded to texture meme)
** tex_region_size <int> <int> <int> (texel area downloaded to texture meme)
**
** header_offset <int> (byte offset to skip user's file header)
** tiles_in_file <int> <int> <int> number of image tiles in each image file
** tile_size <int> <int> <int>  (width, height, depth in texels of each tile)
**
** tile_base <file path string> (root of tile file names)
**
** tile_format <scanf string> (tile file name format; null string=default)
** num_tile_params <int> (number of parameter token arguments)
** tile_params <string list> (format parameter tokens in order)
**
** (ignored)num_streams <int> <int> <int> (number of s, t, and r stream servers)
** s_streams <string list> (list of S stream servers)
** t_streams <string list> (list of T stream servers)
** r_streams <string list> (list of R stream servers)
** default_tile <file path string> (name of tile to use if a tile isn't found)
*/

PFDUDLLEXPORT pfImageCache *
pfdLoadImageCache(const char *fileName, pfTexture *texture, int level)
{
    return pfdLoadImageCacheState(fileName, texture, level, NULL);
}

PFDUDLLEXPORT pfImageCache *
pfdLoadImageCacheState(const char *fileName, pfTexture *texture, 
				     int level, pfuImgCacheConfig *state)
{
    Strbuf buf;
    FILE *fp;
    pfImageCache *ic;
    pfuImgCacheConfig stateInfo;
    int i;
    int done = FALSE; /* done parsing file? */
    int failed = FALSE;
    Token lastToken = START_OF_FILE; /* for analyzing parse failures */
    Token curToken =  START_OF_FILE;
    char icFilePath[2048]; /* get this file's filepath for relative pathnames*/
    int version_enforce = 0;
    char **strptr;


    /* pfdOpenFile should be using this */
    if(!pfFindFile(fileName, icFilePath, R_OK))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadClipTexure: couldn't find image cache config file %s",
		 fileName);
	return NULL;
    }
    if ((fp = pfdOpenFile(icFilePath)) == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadClipTexure: couldn't open image cache config file %s",
		 fileName);
        return NULL;
    }

    if(!state) /* no state arg, point to struct and initialize */
    {
	state = &stateInfo;
	pfuInitImgCacheConfig(state);
    }

    /*
    ** In the future, this will be settable by the user; path is the
    ** default name.
    */
    state->name = icFilePath;

    while(!done) {
	switch(curToken = nextToken(fp)) {
	case IMAGE_CACHE_VERSION:
	    if(lastToken != START_OF_FILE) {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadImageCache: "
			 "version token doesn't start file!\n");
	    }
	    version_enforce = 1;
	    break;
	case COMMENT:
	    /* Comment from token to end of current line */
	    while(fscanf(fp, "%*[^\n]"));
	    lastToken = COMMENT;
	    break;
	case EXT_FORMAT:
	    /* Read in External Format of Image Tile */
	    fscanf(fp,"%s", buf); 
	    state->extFormat = _pfExternalFormatStrToEnum("pfdLoadImageCache",
							 buf);
	    if(state->extFormat < 0)
		failed = TRUE;
	    lastToken = EXT_FORMAT;
	    break;
	case INT_FORMAT:
	    /* Read in Internal Format of Image Tile */
	    fscanf(fp,"%s",buf); 
	    state->intFormat = _pfInternalFormatStrToEnum("pfdLoadImageCache",
							 buf);
	    if(state->intFormat < 0)
		failed = TRUE;
	    lastToken = INT_FORMAT;
	    break;
	case IMG_FORMAT:
	{
	    /* Read in Image Format of Image Tile */
	    ImageFormatInfo *imageFormat;
	    fscanf(fp, "%s", buf);
	    imageFormat = _pfImageFormatStrToEnum("pfdLoadImageCache", buf);
	    if(!imageFormat)
		failed = TRUE;
	    else
	    {
		state->imgFormat = imageFormat->format;
		state->components = imageFormat->components;
	    }
	    lastToken = IMG_FORMAT;
	    break;
	}
	case VSIZE:
	    /* Read Virtual Size */
	    fscanf(fp, "%d %d %d", &state->size[PF_S],
		   &state->size[PF_T], &state->size[PF_R]);
	    lastToken = VSIZE;
	    VERSION_ENFORCE("pfdLoadImageCache");
	    break;
	case TEX_REGION:
	    fscanf(fp,"%d %d %d",
		   &state->texRegSize[PF_S],
		   &state->texRegSize[PF_T],
		   &state->texRegSize[PF_R]);
	    break;
	case MEM_REGION:
	    fscanf(fp,"%d %d %d",
		   &state->memRegSize[PF_S],
		   &state->memRegSize[PF_T],
		   &state->memRegSize[PF_R]);
	    break;
	case CACHE_SIZE:
	    /* Read in Cache size in mem specified in tiles */
	    fscanf(fp,"%d %d %d",
		   &state->memRegSize[PF_S],
		   &state->memRegSize[PF_T],
		   &state->memRegSize[PF_R]);
	    lastToken = CACHE_SIZE;
	    break;
	case VALID_REGION:
	    /* Read in Valid Region Size - specified in texels */
	    fscanf(fp,"%d %d %d",
		   &state->texRegSize[PF_S],
		   &state->texRegSize[PF_T],
		   &state->texRegSize[PF_R]);
	    lastToken = VALID_REGION;
	    break;
	case VALID_REGION_DST:
	    /* Read in Valid Region Dst Size - specified in texels */
	    fscanf(fp,"%d %d %d",
		   &state->texSize[PF_S],
		   &state->texSize[PF_T],
		   &state->texSize[PF_R]);
	    lastToken = VALID_REGION_DST;
	    break;
	case HEADER_OFFSET:
	    /* Read in file header offset */
	    fscanf(fp, "%d", &state->header);
	    lastToken = HEADER_OFFSET;
	    break;
	case TILES_IN_FILE:
	    /* Read in number of S, T, & R Tiles in each tile file */
	    fscanf(fp,"%d %d %d",
		   &state->tilesInFile[PF_S],
		   &state->tilesInFile[PF_T],
		   &state->tilesInFile[PF_R]);
	    lastToken = TILES_IN_FILE;
	    break;
	case TILE_SIZE:
	    /* Read in Tile Width,Height,Depth specified in texels */
	    fscanf(fp,"%d %d %d",
		   &state->tileSize[PF_S],
		   &state->tileSize[PF_T],
		   &state->tileSize[PF_R]);
	    lastToken = TILE_SIZE;
	    break;
	case TILE_BASE:
	    /* get image cache fname base */
	{
	    int status;
	    status = fscanf(fp, "%s", buf); 
	    if(isToken(buf, fp) != BAD_TOKEN || /* no usable argument */
		status == EOF || status == 0) 
	    {
		/* unread next token */
		if(status != 0 && status != EOF)
		    fseek(fp, -strlen(buf), SEEK_CUR);
		buf[0] = '\0'; /* use empty string */
	    }
	    _pfExpandBase("pfdLoadImageCache", buf);/* expand env var */
	    if(state->base)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadImageCache: image cache filename base "
			 "can only be set once");
		failed = TRUE;
	    }
	    else
	    {
		state->base = (char *)malloc(strlen(buf) + 1);
		strcpy(state->base, buf);
	    }
	    lastToken = TILE_BASE;
	}
	    break;
	case TILE_FORMAT:
	    /* get image cache file name format */
	    fscanf(fp, "%s", buf);
	    _pfExpandBase("pfdLoadImageCache", buf);/* expand env var */
	    state->format = (char *)malloc(strlen(buf) + 1);
	    strcpy(state->format, buf);
	    lastToken = TILE_FORMAT;
	    break;
	case NUM_TILE_PARAMS:
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "pfdLoadImageCache: num_tile_params obsolete. "
		     "Argument ignored");
	    fscanf(fp, "%*d");
	    lastToken = NUM_TILE_PARAMS;
	    break;
	case TILE_PARAMS:
	    for(i = 0;1; i++) 
	    {
		int status;
		status = fscanf(fp, "%s", buf);
		if(status == 0 || status == EOF)
		    break;
		if(isToken(buf, fp) == BAD_TOKEN) /* still reading params */
		{
		    int arg;
		    arg = _pfFnameArgToNum(buf);
		    if(arg == -1)
		    {
			pfNotify(PFNFY_WARN, PFNFY_PRINT,
				 "pfdLoadImageCache: invalid tile "
				 "parameter token");
			failed = TRUE;
		    }
		    if(i < PFIMAGECACHE_MAX_TILE_FILENAME_ARGS)
			state->args[i] = arg;
		}
		else /* unread last fscanf so nextToken() call works */
		{
		    fseek(fp, -strlen(buf), SEEK_CUR);
		    break;
		}
	    }
	    if(i > PFIMAGECACHE_MAX_TILE_FILENAME_ARGS)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadImageCache: number of filename arguments"
			 "exceeds maximum (%d)", 
			 PFIMAGECACHE_MAX_TILE_FILENAME_ARGS);
		failed = TRUE;
	    }	    
	    state->numParams = i;
	    lastToken = TILE_PARAMS;
	    break;
	case NUM_STREAM_SERVERS:
	    /* Obsolete Token: no longer used */
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "pfdLoadImageCache: num_streams servers obsolete."
		     "Arguments ignored");
	    (void)fscanf(fp, "%*d %*d %*d"); /* consume arguments */
	    break;
	case S_STREAM_SERVERS:
	    state->sStreams = pfNewList(sizeof(char*), 5, NULL);
	    for(i = 0;1; i++) {
		int status;
		status = fscanf(fp, "%s", buf);
		if(status == 0 || status == EOF)
		    break; /* invalid token or end of file */
		if(isToken(buf, fp) == BAD_TOKEN) /* still reading streams */
		{
		    char *path = (char *)malloc(strlen(buf) + 1);
		    strcpy(path, buf);
		    pfAdd(state->sStreams, path);
		}
		else /* "unread" last fscanf so nextToken() call works */
		{
		    fseek(fp, -strlen(buf), SEEK_CUR);
		    break;
		}
	    }
	    break;
	case T_STREAM_SERVERS:
	    state->tStreams = pfNewList(sizeof(char*), 5, NULL);
	    for(i = 0;1; i++) {
		int status;
		status = fscanf(fp, "%s", buf);
		if(status == 0 || status == EOF)
		    break; /* invalid token or end of file */
		if(isToken(buf, fp) == BAD_TOKEN) /* not a token */
		{
		    char *path = (char *)malloc(strlen(buf) + 1);
		    strcpy(path, buf);
		    pfAdd(state->tStreams, path);
		}
		else /* "unread" last fscanf so nextToken() call works */
		{
		    fseek(fp, -strlen(buf), SEEK_CUR);
		    break;
		}
	    }
	    break;
	case R_STREAM_SERVERS:
	    state->rStreams = pfNewList(sizeof(char*), 5, NULL);
	    for(i = 0;1; i++) {
		int status;
		status = fscanf(fp, "%s", buf);
		if(status == 0 || status == EOF)
		    break; /* invalid token or end of file */
		if(isToken(buf, fp) == BAD_TOKEN) /* not a token */
		{
		    char *path = (char *)malloc(strlen(buf) + 1);
		    strcpy(path, buf);
		    pfAdd(state->rStreams, path);
		}
		else /* "unread" last fscanf so nextToken() call works */
		{
		    fseek(fp, -strlen(buf), SEEK_CUR);
		    break;
		}
	    }
	    break;
	case DEFAULT_TILE:
	    fscanf(fp, "%s", buf); /* default tile file name */
	    state->defaultTile = (char *)malloc(strlen(buf) + 1);
	    strcpy(state->defaultTile, buf);
	    lastToken = DEFAULT_TILE;
	    break;
	case PAGE_SIZE:
	    /* Will override pagesize set in clip texture */
	    if(state->pageSize != -1)
	    {
		pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT,
			 "pfdLoadImageCache(): page_size value overrides "
			 "value set in cliptexture config file.");
	    }
	    if(fscanf(fp, "%d", &state->pageSize) != 1)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture(): couldn't read page_size "
			 "argument (should be positive small integer). "
			 "using default value.");
		state->pageSize = -1;
	    }
	    lastToken = PAGE_SIZE;
	    break;
	case READ_FUNC: 
	{ /* 1 arg: function name (from executable). 2 args: dso funcname */
	    int status;
	    char *dsoname;
	    char *readname;
	    void *dso;
	    pfReadImageTileFuncType readfunc;
	    status = fscanf(fp, "%s", buf);
	    if(isToken(buf, fp) != BAD_TOKEN || /* no usable argument */
		status == EOF || status == 0) 
	    {
		/* unread next token */
		if(status != 0 && status != EOF)
		    fseek(fp, -strlen(buf), SEEK_CUR);
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadImageCache(): read function "
			 "requires 1 or 2 arguments; "
			 "using default read function");
		lastToken = READ_FUNC;
		break;
	    }
	    
	    /* this may turn out to be fname (no dso arg) */
	    dsoname = (char *)malloc(sizeof(buf) + 1);
	    strcpy(dsoname, buf);

	    status = fscanf(fp, "%s", buf);
	    if(isToken(buf, fp) != BAD_TOKEN || /* no usable argument */
		status == EOF || status == 0) 
	    {
		/* unread next token */
		if(status != 0 && status != EOF)
		    fseek(fp, -strlen(buf), SEEK_CUR);
		readname = dsoname;
		dsoname = NULL;
	    }
	    else
	    {
		readname = (char *)malloc(sizeof(buf) + 1);
		strcpy(readname, buf);
	    }

#ifndef WIN32
	    dso = dlopen(dsoname, RTLD_LAZY);
#else
	    dso = LoadLibrary(dsoname);
#endif
	    if(!dso && dsoname)
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
			 "pfuMakeClipTexture(): can't find DSO %s for "
			 "read function; looking for %s in executable",
			 dsoname, readname);

	    if(!dso)
#ifndef WIN32
		dso = dlopen(NULL,  RTLD_LAZY);
#else
	        dso = GetModuleHandle(NULL);
#endif

	    if(!dso)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadClipTexture(): could open read function dso; "
			 "using default read function");
		lastToken = READ_FUNC;
		break;
	    }
#ifndef WIN32
	    readfunc = (pfReadImageTileFuncType)dlsym(dso, readname);
#else
	    readfunc = (pfReadImageTileFuncType)GetProcAddress(dso, readname);
#endif

	    if(!readfunc)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
			 "pfdLoadImageCache(): couldn't find %s symbol; "
			 "read function unchanged",
			 readname);
	    }
	    else
		state->readFunc = readfunc;

	    lastToken = READ_FUNC;
	    break;
	} /* READ_FUNC */
	case LOOKAHEAD:
	    if(state->lookahead != 1)
	    {
		pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT,
			 "pfdLoadImageCache(): overriding lookahead value "
			 "%d set in clip texture config file.",
			 state->lookahead);
	    }
	    if(fscanf(fp,"%d", &state->lookahead) != 1)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfdLoadImageCache(): couldn't read lookahead "
			 "argument (should be positive small integer). "
			 "using default border of 1. ");
		state->lookahead = 1;
	    }
	    lastToken = LOOKAHEAD;
	    break;
	case END_OF_FILE:
	    done = 1;
	    lastToken = END_OF_FILE;
	    break;
	default:
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "Tried to used image cache config file token");
	    pfNotify(PFNFY_WARN, PFNFY_MORE,
		     "%s in clip texture config file. Failed ",
		     tokenString(curToken));
	case BAD_TOKEN:
	    pfNotify(PFNFY_WARN, PFNFY_MORE, 
		     "in pfdLoadImageCache after parsing token %s\n",
		     tokenString(lastToken));
	    failed = TRUE;
	}
    }
    /* Create and Configure Image Cache */
    if(failed)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadImageCache: "
		 "parse of image cache configuration file %s failed",
		 fileName);
	return NULL;
    }
    /* 
    ** do relative pathname expansion. Use old method (expand base name)
    ** if state->base is set, otherwise expand state->format
    ** semantics are unclear if base or format is passed in through struct and
    ** not in config file.
    */
    if(state->base)
	strptr = &state->base;
    else
	strptr = &state->format;
    if(*strptr) /* neither set; an error, but test prevents core dump */
    {
	i = strlen(*strptr);
	strcpy(buf, *strptr);
	_pfExpandRelative("pfdLoadImageCache", icFilePath, buf);
	if(strlen(buf) > i)
	    *strptr = realloc(*strptr, strlen(buf) + 1);
	strcpy(*strptr, buf);
    }
    ic = pfuMakeImageCache(texture, level, state);
    pfuFreeImgCacheConfig(state);
    return ic;
}

PFDUDLLEXPORT int
pfdClipTextureNodeUpdate(pfTraverser *trav, void *userData)
{
    static int pcS,pcT,pcR;
    static int LSIZE;
    int i,n,ds,dt,vS,vT,vR,rS,rT,tS,tT,tR;
    int nCacheCS,nCacheCT,csizeS,csizeT,csizeR;
    pfMPClipTexture *mpClip;
    pfClipTexture *clip;
    pfImageCache *ic;
    pfImageTile *proto;
    static int first = 1;
    pfChannel *chan = pfGetTravChan(trav);
    pfNode *nd = pfGetTravNode(trav);
    pfList *lst = pfGetUserData(nd);

    if (first)
    {
	pcS = 0;
	pcT = 0;
	LSIZE = atoi(getenv("LOADSIZE"));
	if (LSIZE < 8)
	    LSIZE = 8;
	pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT,"Using load size %d",LSIZE);
	first = 0;
    }
		
    if (lst == NULL)
	return 0;
    n = pfGetNum(lst);
    for(i=0;i<n;i++)
    {
        mpClip = (pfMPClipTexture*)pfGet(lst,i);
	clip = pfGetMPClipTextureClipTexture(mpClip);
        pfGetClipTextureVirtualSize(clip,&vS,&vT,&vR);
	rS = rT = pfGetClipTextureClipSize(clip);
	pfGetMPClipTextureCenter(mpClip,&pcS,&pcT,&pcR);

	{
	    static int signS = 1;
	    static int signT = 1;

	pcS += LSIZE * signS;
	pcT += LSIZE * signT;
	if (pcS >= vS - rS)
	{
	    signS = -1;
	    pcS = vS-rS;
	}
	if (pcS < 0)
	{
	    signS = 1;
	    pcS = 0;
	}
	if (pcT >= vT - rT)
	{
	    signT = -1;
	    pcT = vT-rT;
	}
	if (pcT < 0)
	{
	    signT = 1;
	    pcT = 0;
	}
	}
	if (first)
	{
	    first--;
	    pcS = pcT = vS/2;
	}
	pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT,
		"Frame %d - setting center of clip 0x%x - %d, %d",
		pfGetFrameCount(),mpClip,pcS,pcT);
	pfMPClipTextureCenter(mpClip,pcS,pcT,0);
    }
    return PFTRAV_CONT;
}
