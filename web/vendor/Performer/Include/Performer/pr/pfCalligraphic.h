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
// pfCalligraphic.h - Calligraphic Board / Channel 
//
// $Revision: 1.28 $
// $Date: 2002/09/23 04:25:06 $
//
//

#ifndef __PF_CALLIGRAPHIC_H__
#define __PF_CALLIGRAPHIC_H__


#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pr/pfMemory.h>
#include <Performer/pr/pfList.h>
#include <Performer/pr/pfStats.h>


/////////////////// defines common to all LPB //////////////////////////
//typedef void * pfCallBuffer;

extern "C" {     // EXPORT to CAPI

extern void 	pfSelectCallig(pfCalligraphic *_calligraphic);
extern DLLEXPORT pfCalligraphic * pfGetCurCallig(void);

} // extern "C" END of C include export





#define PFCALLIGRAPHIC ((pfCalligraphic*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFCALLIGRAPHICBUFFER ((pfCalligraphic*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfCalligraphic : public pfObject
{
// CAPI:basename Callig
public:
    // constructors and destructors
    pfCalligraphic();
    virtual ~pfCalligraphic();

public:
    // per class functions;
    static void	init();
    static pfType* getClassType() { return classType; }
    static int initBoard(int _numPipe);
    static unsigned int queryBoard(int _numPipe);
    static int closeBoard(int _numPipe);
    static int isBoardInited(int _numPipe);
    static int getDeviceId(int _numPipe);
    static int partition(int _board, size_t *_allocate, int _n);
    static int waitForVME(int _board);
    static int waitForVISI(int _board);
    static void	swapVME(int _board);
    static int getBoardMemSize(int _board);
public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

public:
    // sets and gets
    void setZFootPrintSize(float _size);
    float getZFootPrintSize(void);
    void setDrawTime(float _time);
    float getDrawTime(void);
    void setFilterSize(unsigned int _sizeX, unsigned int _sizeY);
    void getFilterSize(unsigned int *_sizeX, unsigned int *_sizeY);
    void setDefocus(float _defocus);
    float getDefocus(void);
    void setRasterDefocus(float _defocus);
    float getRasterDefocus(void);
    void setStress(float _stress);
    float getStress(void);
    int setChannel(int _pipe, int _channel);
    int getChannel(int *_pipe, int *_channel);
    void setWin(float _xmin, float _ymin, float _width, float _height);
    void getWin(float *_xmin, float *_ymin, float *_width, float *_height);
    void setMultisample(int _n);
    int getMultisample(void);
    int downLoadSlewTable(pfCalligSlewTableEnum offset, pfCalligSlewTable Slew);
    int upLoadSlewTable(pfCalligSlewTableEnum offset, pfCalligSlewTable Slew);
    int downLoadGammaTable(pfCalligGammaTableEnum offset, pfCalligGammaTable Gamma);
    int upLoadGammaTable(pfCalligGammaTableEnum offset, pfCalligGammaTable Gamma);
    void setExposureRatio(float ratio) { exposureRatio = ratio; }
    float getExposureRatio(void) { return exposureRatio; }
    void setXYSwap(int flag) { xyswap = flag; }
    int getXYSwap(void) { return xyswap; }
    void setProjMatrix(pfMatrix *projMat);
    void getProjMatrix(pfMatrix *projMat);

    // CAPI:verb CollectCalligStats
    void collectStats(pfStats *stats);

public:
    int		isInited(void);		// return 1 if initialized


PFINTERNAL: // XXX - should make protected???
public:
    float   stress;  		// Normalized, 1 = all calligraphic 
private:
    // pipe and channel
    int	pipe;
    int channel;
    // the viewport
    float xmin;
    float ymin;
    float width;
    float height;
    int xyswap;

    // VME driving
    unsigned int ahead;	// Number of allowed DMA buffers ahead of swapbuffer
    float   zFootPrint; 	// size of zbuffer testing area
    int	    nb_msample;		// nb multisample in the selected visual
    float   drawTime;		// set draw time in nano seconds
    uint    filterSizeX; 	// size of filter for debunching 
    uint    filterSizeY; 	// size of filter for debunching
    float   defocus; 		// 0 = normal 
    float   rasterDefocus;	// Only for specific projectors 
    float   exposureRatio;      // division to projector unit

private:
    static pfType *classType;
public:
    void *_pfCalligraphicExtraData;
}; 

#ifndef __BUILDCAPI__
extern "C" {     // EXPORT to CAPI
/* ------------------ pfCalligraphic Macros --------------------- */
#if defined(__STDC__) || defined(__cplusplus)

#endif /* defined(__STDC__) || defined(__cplusplus) */
}
#endif // !__BUILDCAPI__

#endif // !__PF_CALLIGRAPHICS_H__
