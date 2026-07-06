//
//
// Copyright 1995, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfImageTile.h
//
// $Revision: 1.36 $
// $Date: 2002/03/22 03:19:02 $
//
//

#ifndef __PF_IMAGETILE_H__
#define __PF_IMAGETILE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>

extern "C" {     // EXPORT to CAPI
/* ----------------------- pfImageTile Tokens ----------------------- */

/* ------- pfImageTile Dirty Tokens ------ */
#define PFIMAGETILE_FILEORIGIN		0x1
#define PFIMAGETILE_SIZE		0x2
#define PFIMAGETILE_MEMFORMAT		0x4
#define PFIMAGETILE_NAME		0x8
#define PFIMAGETILE_FILENAME		0x10
#define PFIMAGETILE_FILEFORMAT		0x20
#define PFIMAGETILE_IMAGE		0x40
#define PFIMAGETILE_FILEREADFUNC	0x80
#define PFIMAGETILE_USER		0x100
extern DLLEXPORT pfQueue* pfGetGlobalReadQueue(void);
extern DLLEXPORT void pfDeleteGlobalReadQueue(void);
} // extern "C" END of C include export


#define PFIMAGETILE ((pfImageTile*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFIMAGETILEBUFFER ((pfImageTile*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfImageTile : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename ImageTile
    pfImageTile();
    virtual ~pfImageTile();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

/*----------------------------- pfImageTile --------------------------------*/
public:
    // sets and gets
    // CAPI:basename
    // CAPI:basename ImageTile
    void		setSize(int _w, int _h, int _d);
    void 		getSize(int *_w, int *_h,int *_d);
    void		setOrigin(int _s, int _t, int _r);
    void		getOrigin(int *_s, int *_t, int *_r);

    void		setMem(unsigned char *_img, int _nBytes);
    unsigned char*	getMem(void);

    void		setMemQueue(pfQueue *q);
    const pfQueue*	getMemQueue(void);

    int			getNumImageComponents(void);
    void		setMemImageFormat(int _format);
    int 		getMemImageFormat(void);
    void		setMemImageType(int _type);
    int			getMemImageType(void);
    int			getMemImageTexelSize(void);
    void		setMemInfo(int _psize, int _lock);
    void		getMemInfo(int *_psize, int *_lock);

    void		setName(const char *_fname);
    const char*		getName(void);
    void		setFileName(const char *_fname);
    const char*		getFileName(void);
    int			getFileImageTexelSize(void);
    void		setFileTile(int _tileS, int _tileT, int _tileR);
    void		getFileTile(int *_tileS, int *_tileT, int *_tileR);
    void		setNumFileTiles(int _nTilesS, int _nTilesT, int _nTilesR);
    void		getNumFileTiles(int *_nTilesS, int *_nTilesT, int *_nTilesR);
    void		setFileImageFormat(int _fileFmt);
    int			getFileImageFormat(void);
    void		setFileImageType(int _fileType);
    int			getFileImageType(void);

    unsigned char*	getSubTile(int _s, int _t, int _r);
    unsigned char*	getValidSubTile(int _s, int _t, int _r);

    void		setReadQueue(pfQueue *q);
    pfQueue*		getReadQueue(void);

    int			getTotalBytes(void);
    int			getValidBytes(void);

    int			getValidTexels(void);
    void		setValidTexels(int _nTexels);

    int			isValid(void);
    int			isLoading(void);
    int			isDirty(void);

    void			setReadFunc(pfReadImageTileFuncType _func);
    pfReadImageTileFuncType	getReadFunc(void);

    void		setDefaultTile(pfImageTile *_default);
    pfImageTile*	getDefaultTile(void);

    void		setDefaultTileMode(int _useDefault);
    int			getDefaultTileMode(void);

    void                setHeaderOffset(int hdr);
    int                 getHeaderOffset(void);

    void		setPriority(int priority);
    int			getPriority(void);

    void                setImageCache(pfImageCache *_ic);
    pfImageCache*       getImageCache(void);

    void                setTileIndex(int _s, int _t, int _r);
    void                getTileIndex(int *_s, int *_t, int *_r);

    void                setFileNameFunc(pfTileFileNameFuncType fnFunc);
    pfTileFileNameFuncType getFileNameFunc(void);
public:
    // Methods
    // CAPI:verb LoadImageTile
    int			load(void);
    // CAPI:verb LoadPartialImageTile
    int			load(int nTexels);
    // CAPI:verb LoadImageTileFile
    int			load(const char *_fname);
    // CAPI:verb LoadPartialImageTileFile
    int			load(const char *_fname, int _nTexels);
    // CAPI:verb FreeImageTileMem
    void		freeMem(void);

    static int 		ReadDirect(pfImageTile *itile, int ntexels);
    static int 		ReadNormal(pfImageTile *itile, int ntexels);

    static int ProcessOneRead(void *_data);

    //CAPI:verb ImageTileSortFunc
    static void sortFunc(pfQueueSortFuncData *_data);

    // CAPI:basename ImageTile
    unsigned char*	getUnalignedMem(void) const { return img; }
    short		getUseMemQueue(void) const { return useMemQueue; }
    void		setUseMemQueue(short useMemQueue);

protected:
    int			w,h,d;		// Tile Size
    int			memFmt;		// Image Format PFTEX_IMAGE_FORMAT
    int			memType;	// Image Type PFTEX_EXTERNAL_FORMAT
    unsigned char 	*img;		// Image Data
    unsigned char	*realImg;	// Image Data page aligned
    int			psize;		// mem alignment size
    int			pin;		// should mem be pinned
    short		pinned;		// is mem pinned
    short		useMemQueue;	// Use Mem Queue?
    int			imgBytes;	// img holds nbytes data
    int			pid;		// Holds PID of parallel load proc
    volatile int	loading;	// Texel bytes to load (parallel)
    volatile int	loaded;		// Loaded Texel bytes
    volatile int	valid;		// valid when loaded = ALL
    pfQueue*		queue;		// Queue to perform read
    pfQueue*		memQueue;	// Queue of memory blocks
    volatile pfImageTile* defaultTile;	// fallback tile in case this tile isnt valid
    int 	       	dirty;		// Has Image Changed
    int			s,t,r;		// Tile Origin (Offset on disk)
    int			fileFmt;	// on disk storage fmt (RGB/IA/I/RGBA)
    int			fileType;	// on disk data type (per comp/texel)
    int			fTileS,fTileT,fTileR; // subtile of img file
    int			nFTilesS, nFTilesT, nFTilesR; // Number of individual tiles in this file
    // Placeholders for future functionality:
    //     FILENUM_S = ((S + XXX_S) % YYY_S) / nFTilesS
    int			XXX_S, XXX_T, XXX_R;
    uint		YYY_S, YYY_T, YYY_R;

    char		fileName[PF_MAXSTRING];	// Name of file 
    char		name[PF_MAXSTRING];	// String Image Name
    pfReadImageTileFuncType	readFunc; // Function that will perform reads
    int                 hdrBytes;       // size of app's file header to skip
    volatile int	useDefault;	// use the fallback tile in case if tile not valid
    volatile int	priority;	// use this to determine loading order
    uint32_t		pr_clean_in_progress;
    pfImageCache        *ic; // point to non-slave image cache for data
    pfTileFileNameFuncType fnFunc; //compute file name
    int indexS, indexT, indexR; /* for generating file names */
private:
    static pfType *classType;
public:
    void *_pfImageTileExtraData;
};

#endif // !__PF_IMAGETILE_H__




