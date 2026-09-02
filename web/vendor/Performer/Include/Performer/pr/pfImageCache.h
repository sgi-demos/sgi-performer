
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
// pfImageCache.h
//
// $Revision: 1.66 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_IMAGECACHE_H__
#define __PF_IMAGECACHE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfImageTile.h>
#include <Performer/pr/pfTexLoad.h>
#include <Performer/pr/pfTexture.h>


//Cost table in use
extern pfTexSubloadCostTable *_pfTexSubloadCostTable;

//Machine specific cost tables
extern pfTexSubloadCostTable *_pfIrCostTable;
extern pfTexSubloadCostTable *_pfIr2CostTable;
extern pfTexSubloadCostTable *_pfMgrasCostTable;
extern pfTexSubloadCostTable *_pfReCostTable;
extern pfTexSubloadCostTable *_pfO2CostTable;
extern pfTexSubloadCostTable *_pfOdsyRevACostTable;
extern pfTexSubloadCostTable *_pfNV10CostTable;
extern pfTexSubloadCostTable *_pfNV20CostTable;

//User-defined cost table
extern pfTexSubloadCostTable *_pfUserCostTable;

struct _pfDim {
    public:
// CAPI: private
    void copy(_pfDim src) {
	S = src.S;
	T = src.T;
	R = src.R;
    }
    int compare(const _pfDim other) const {
	if(S == other.S && T == other.T && R == other.R)
	    return 0;
	else
	    return 1;
    }
    void set(int value) {
	S = value;
	T = value;
	R = value;
    }
    int prod(void) {
	return (S * T * R);
    }
    void print(FILE *file, char *prefix, char *str) {
	fprintf(file, "%s%s (%d, %d, %d)\n",
		prefix, str, S, T, R);
    }

    void print(FILE *file, char *prefix, char *str, char *suf) {
	fprintf(file, "%s%s%s (%d, %d, %d)\n",
		prefix, str, suf, S, T, R);
    }

    void print(FILE *file, char *prefix, char *str, char *suf1, char *suf2) {
	fprintf(file, "%s%s%s%s (%d, %d, %d)\n",
		prefix, str, suf1, suf2, S, T, R);
    }

    void operator=(int value) {
	set(value);
    }
    int S;
    int T;
    int R;
};


struct _pfReg {
    public:
// CAPI: private
    _pfReg& operator=(const _pfReg &src) {
	copy(src);
	return *this;
    }
    void copy(_pfReg src) {
	Org.copy(src.Org);
	Size.copy(src.Size);
    }
    int compare(const _pfReg other) const {
        if(Org.compare(other.Org) || Size.compare(other.Size))
	    return 1;
	else
	    return 0;
    }
    void set(int value) {
	Org.set(value);
	Size.set(value);
    }
    void print(FILE *file, char *prefix, char *str) {
	Org.print(file, prefix, str, ".Org");
	Size.print(file, prefix, str, ".Size");
    }

    void print(FILE *file, char *prefix, char *str, char *suf) {
	Org.print(file, prefix, str, suf, ".Org");
	Size.print(file, prefix, str, suf, ".Size");
    }
    void operator=(int value) {
	set(value);
    }
    _pfDim Org;
    _pfDim Size;
};


struct _pfState {
    public:
// CAPI: private
    void copy(_pfState src) {
	Shad.copy(src.Shad);
	Cur.copy(src.Cur);
	New.copy(src.New);
	DstSize.copy(src.DstSize);
    }
    // Don't compare shadow and new values for equality
    int compare(const _pfState other) const {
	if(Cur.compare(other.Cur) || DstSize.compare(other.DstSize))
	    return 1;
	else
	    return 0;
    }
    void update(void) {
	Cur.copy(New);
    }
    void set(int value) {
	Shad.set(value);
	Cur.set(value);
	New.set(value);
	DstSize.set(value);
    }
    void print(FILE *file, char *prefix, char *str) {
	Shad.print(file, prefix, str, ".Shad");
	Cur.print(file, prefix, str, ".Cur");
	New.print(file, prefix, str, ".New");
	New.print(file, prefix, str, ".DstSize");
    }

    void operator=(int value) {
	set(value);
    }
    _pfReg Shad;
    _pfReg Cur;
    _pfReg New;
    _pfDim DstSize; /* size of destination region */
};

extern "C" {     // EXPORT to CAPI

/* ------------------ pfImageCache Related Definitions --------------------- */
#define PF_DTR_TEXLOAD      0x01 /*check if tex region can be loaded in time */
#define PF_DTR_MEMLOAD      0x02 /*check if memory tiles where loaded in time */
#define PF_DTR_READSORT     0x04 /*sort memory tile read queue */
#define PF_DTR_TEXSIZE      0x08 /*shrink size of textured level (obsolete) */
/*
** PFIMAGECACHE_MAX_TILE_FILENAME_ARGS has been moved to pr.h so
** pfutil.h can use it.
*/

#define PFIMAGECACHE_AUTOCENTER				0x1

#define PFIMAGECACHE_AUTOCREATE_STREAMSERVER_QUEUES 	0x2

#define PFIMAGECACHE_AUTOSET_TILE_FILENAME		0x3

#define PFIMAGECACHE_AUTOSET_TILE_READQUEUE		0x4

#define PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_S		0
#define PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_T		1
#define PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_R		2
#define PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_S		3
#define PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_T		4
#define PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_R		5
#define PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_S		6
#define PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_T		7
#define PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_R		8
#define PFIMAGECACHE_TILE_FILENAMEARG_STREAMSERVERNAME	9
#define PFIMAGECACHE_TILE_FILENAMEARG_CACHENAME		10
#define PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_S		11
#define PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_T		12
#define PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_R		13
 
#define PFIMAGECACHE_S_DIMENSION			0x0
#define PFIMAGECACHE_T_DIMENSION			0x1
#define PFIMAGECACHE_R_DIMENSION			0x2

} // extern "C" END of C include export

/*
** PFIMAGECACHE_MAX_TILE_FILENAME_ARGS	moved to pr.h so 
** pfutil.h can use it.
*/

#define MAX_DIM 3
#define PFIMAGECACHE ((pfImageCache*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFIMAGECACHEBUFFER ((pfImageCache*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfImageCache : public pfObject
{
public:
    // constructors and destructors
    pfImageCache();
    virtual ~pfImageCache();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

/*----------------------------- pfImageCache --------------------------------*/

public:
    // sets and gets
    // CAPI:basename ImageCache
    void	setName(const char *_name);
    const char* getName(void);

    void	setProtoTile(pfImageTile *tile);
    pfImageTile *getProtoTile(void);

    // rename ValidRegionOrigin
    void 	setTexRegionOrg(int _originS, int _originT, int _originR);
    // rename ValidRegionSize
    void 	setTexRegionSize(int _sizeS, int _sizeT, int _sizeR);
    void 	getTexRegionOrg(int *_originS, int *_originT, int *_originR);
    void 	getTexRegionSize(int *_sizeS, int *_sizeT, int *_sizeR);
    void 	getCurTexRegionOrg(int *_originS, int *_originT, int *_originR);
    void 	getCurTexRegionSize(int *_sizeS, int *_sizeT, int *_sizeR);
    // rename Origin
    void 	setMemRegionOrg(int _tileS, int _tileT, int _tileR);
    // rename Size
    void 	setMemRegionSize(int _tileS, int _tileT, int _tileR);
    void 	getMemRegionOrg(int *_orgTileS, int *_orgTileT, int *_orgTileR);
    void 	getMemRegionSize(int *_sizeS, int *_sizeT, int *_sizeR);
    void 	getCurMemRegionOrg(int *_orgS, int *_orgT, int *_orgR);
    void 	getCurMemRegionSize(int *_sizeS, int *_sizeT, int *_sizeR);

    // rename ValidRegionDstSize
    void 	setTexSize(int _s, int _t, int _r);
    void	getTexSize(int *_s, int *_t, int *_r);

    // rename ValidRegionOffset
    void	getTexRegionOffset(int *_s, int *_t, int *_r);

    // rename ValidRegionDst
    void	setTex(void *_dst, int _lvl, int _type);
    void	getTex(void **_dst, int *_lvl, int *_type);

    // rename Virtual Size
    void     	setImageSize(int _sizeS, int _sizeT, int _sizeR); // in texels
    void     	getImageSize(int *_sizeS, int *_sizeT, int *_sizeR);

    void       	setReadQueueFunc(pfImageCacheReadQueueFuncType _func);
    void	setTileFileNameFunc(pfTileFileNameFuncType _func);
    pfImageCacheReadQueueFuncType	getReadQueueFunc(void);
    pfTileFileNameFuncType 	getTileFileNameFunc(void);

    void	setFileStreamServer(int _dim, int _which, const char *_device);
    const char*	getFileStreamServer(int _dim, int _which);
    void	setStreamServerQueue(int _dim, int _which, pfQueue *_q);
    pfQueue*	getStreamServerQueue(int _dim, int _which);
    pfQueue*    getStreamServerQueueByName(const char *_name);

    void	setTileFileNameFormat(const char *_fmtString, int _nArgs, int *_argList);
    void	getTileFileNameFormat(const char **_fmtString, int *_nArgs, const int **_argList);

    int		getNumStreamServers(int _dim) const {return readQueues[_dim] ? readQueues[_dim]->getNum() : 0; }

    void	setMode(int _mode, int _val);
    int		getMode(int _mode);

    void	setMaster(pfImageCache *_master);
    pfImageCache *getMaster(void) const {return master;};
    pfList       *getSlaves(void) const {return slaves;};

    void        setDTRMode(uint _mode);
    uint        getDTRMode(void);

public:
    // other non-explicit set/gets
    pfImageTile* getTile(int _s, int _t, int _r);

    const pfList* getLoadUpdates(void);

public:
    // other functions

    // CAPI:verb ImageCacheCalcTexRegion
    void        calcTexRegion(int *_orgS, int *_orgT, int *_orgR, int *_sizeS, int *_sizeT, int *_sizeR);
    // CAPI:verb ImageCacheCalcMemRegion
    void        calcMemRegion(int *_orgS, int *_orgT, int *_orgR, int *_sizeS, int *_sizeT, int *_sizeR);


    // CAPI:verb IsImageCacheValid
    int		isValid(int _s, int _t, int _r, int _sizeS, int _sizeT, int _sizeR);
    int		isValid(void);

    // CAPI:verb IsImageCacheTexRegChanged
    int         isTexRegChanged(void);
    
public:
    // methods
    // CAPI:verb DTRApplyImageCache
    float     	apply(float _msec); // apply with DTR
    // CAPI:verb
    void     	apply(void); //apply -- no DTR
    void	invalidate();
    static void        autoConfigFileName(pfImageTile *_itile);

private:
    // internal methods

PFINTERNAL: // XXX - should make protected???
    char*		name;			// Cache Name

    _pfState            memReg;                 // memRegion data
    _pfState            texReg;                 // texRegion data
    _pfDim              texRegOff;              // texregion offset
    _pfDim		imageSize; 	        // image cache imageSize(texels)
    // placeholder for future functionality...
    _pfDim		imageWrapSize; 	        // wrapping size
    _pfDim		arraySizeMask;	// for fast indexing assuming power of 2
    _pfDim		arraySizeShift;	// for fast indexing assuming power of 2
    _pfDim              texRegAlign; // texreg Align value (power of 2 minus 1)

    pfImageTile	    	*proto;
    pfImageTile**	tiles;	  	// List of valid pfImageTiles
    pfList*		tileHeap;	// List of avail pfImageTiles
    pfList*		freeTiles;	// List of Itiles to reuse
					// these might have reads pending,
					// so cannot be put on tileHeap yet
    pfQueue*		memHeap;	// List of proto size chunks of memory

    pfList*		updates;	// List of update pfTexLoads
    pfList*		loaderHeap;	// List of avail pfTexLoads
    pfList*		freeLoaders;	// List of loads to reuse

    int			centerMode;   	// Center Cache by region?
    int			autoDevQueues;  // Automatically create work queues for new file devices
    int			autoSetFileName;// Automatically set itile file name
    int			autoSetQueue;   // Automatically set itile read queue
	
    int			invalid;

    void*		dst;		// Destination of Cache
    int			dstLvl;		// Destination level of Cache
    int			dstType;	// Destination type of cache

    int 		dirty;		// changed field flag
    int 		applyMem;	// updateMemRegion on apply
    int			applyTex;	// updateTexRegion on apply

    float		*dwnldCost;	// 3d array of floats downloadtimes
    int			dwnldCostW;	// 3d array width
    int			dwnldCostH;	// 3d array height
    int			dwnldCostD;	// 3d array depth
    float		dwnldCostAlignS; // lookup offsetS
    float		dwnldCostAlignT; // lookup offsetT
    float		dwnldCostAlignR; // lookup offsetR
    int			priCenterS;	 // unclamped centerS for priority
    int			priCenterT;	 // unclamped centerT for priority
    // XXX shorts to avoid breaking 2.2 binary interface freeze...
    short		priCenterR;	 // unclamped centerR for priority
    short		priCenterSet;    // setPriorityCenter has been called

			// Tile (S,T,R) -> Queue[S%nS] OR Q[T%nT] OR Q[R%nR]
    pfList*		readQueues[MAX_DIM];
			// FileName(S,T,R) -> Names[S%nS],Names[T%nT],N[R%nR]
    pfList*		readNames[MAX_DIM];
    char*		fNameFmt;
    int*		argList;
    int			nArgs;
    pfImageCacheReadQueueFuncType	rqFunc;
    pfTileFileNameFuncType fnFunc;

    pfImageCache 	*master; // if != 0, this is a slave image cache
    pfList	 	*slaves; // if != 0, this is a master image cache

    int                 needAlignValues; //when to get the alignment values
    uint                DTRMode;
    int                 texRegChanged; //did DTR shrink the tex region?
    //XXX TODO make these settable?? (DTR code must change significantly)
    _pfDim              texRegShrinkSize; 

private:
    inline int setupLoadsNoWrap(int doLoads, float *loadTime,
				int oldS0, int oldT0,
				int oldS1, int oldT1,
				int newS0, int newT0,
				int newS1, int newT1,
				int tileW, int tileH,
				int srcToDstOffS, int srcToDstOffT,
				int dSS, int dST);
    void setupLoads(int doLoads, float *loadTime,
		    int oldS0, int oldT0,
		    int oldS1, int oldT1,
		    int newS0, int newT0,
		    int newS1, int newT1,
		    int tileW, int tileH,
		    int srcToDstOffS, int srcToDstOffT,
		    int dSS, int dST);

    inline void setupLoad(int doLoad, float *loadTime,
			  int indexS, int indexT, 
			  int left, int bot, int right, int top,
			  int offS, int offT, int dSS, int dST,
			  int *texelsloaded);

    static pfType *classType;
public:
    void *_pfImageCacheExtraData;
};

#endif // !__PF_IMAGECACHE_H__


