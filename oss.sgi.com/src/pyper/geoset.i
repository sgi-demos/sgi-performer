
// This file contains the public interface from the Performer
// header file pf/pfGeoSet.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{

#include <Performer/pr/pfGeoSet.h>

%}

#define PFGS_POINTS	    	0
#define PFGS_LINES	    	1
#define PFGS_LINESTRIPS	    	2	
#define PFGS_TRIS	    	3
#define PFGS_QUADS	    	4
#define PFGS_TRISTRIPS	    	5
#define PFGS_FLAT_LINESTRIPS	6
#define PFGS_FLAT_TRISTRIPS	7
#define PFGS_POLYS		8
#define PFGS_TRIFANS		9
#define PFGS_FLAT_TRIFANS	10
#define PFGS_NUM_PRIMS		11

/* pfGSetAttr(), attribute types */
#define PFGS_COLOR4	    0
#define PFGS_NORMAL3	    1
#define PFGS_TEXCOORD2	    2
#define PFGS_COORD3	    3
#define PFGS_PACKED_ATTRS   5 /* needed for pfGSetDrawMode too */ 
#define PFGS_TEXCOORD3	    6 /* for vert arrrays only */

/* pfGSetAttr(), binding types */
#define PFGS_OFF	    0
#define PFGS_OVERALL	    1
#define PFGS_PER_PRIM	    2
#define PFGS_PER_VERTEX	    3

/* pfGSetDrawMode() */
#define PFGS_FLATSHADE	    	2
#define PFGS_WIREFRAME	    	3
#define PFGS_COMPILE_GL     	4
/* #define PFGS_PACKED_ATTRS  5 */
#define PFGS_DRAW_GLOBJ     	6

/* packed attr formats for pfGSetAttr() with PACKED_ATTRS */
	/* these are the fast path formats */
#define PFGS_PA_OFF		0
#define PFGS_PA_C4UBN3ST2FV3F	1
#define PFGS_PA_C4UBN3ST2F	2
#define PFGS_PA_C4UBT2F		3
#define PFGS_PA_T2S_FORMATS	4
	/* these are additional useful formats */
#define PFGS_PA_GEN_FORMATS	4
#define PFGS_PA_C4UBN3ST2SV3F	4
#define PFGS_PA_C4UBN3ST2S	5
#define PFGS_PA_C4UBT2S		6
#define PFGS_PA_T3_FORMATS	7
#define PFGS_PA_C4UBN3ST3FV3F	7
#define PFGS_PA_C4UBN3ST3F	8
#define PFGS_PA_C4UBT3F		9
#define PFGS_PA_T3S_FORMATS	10
#define PFGS_PA_C4UBN3ST3SV3F	10
#define PFGS_PA_C4UBN3ST3S	11
#define PFGS_PA_C4UBT3S		12
#define PFGS_PA_T2D_FORMATS	13
#define PFGS_PA_C4UBN3ST2DV3F	13
#define PFGS_PA_C4UBN3ST2D	14

/* pfQueryGSet() */
#define PFQGSET_NUM_TRIS	1
#define PFQGSET_NUM_VERTS	2
#define PFQGSET_NUM_QUERIES	2

/* pfGSetIsectMask() */
#define PFIS_ALL_MASK		0xffffffff /* all active mask */
#define PFIS_PICK_MASK		0x80000000 /* reserved for picking */
#define PFIS_SET_PICK		0x80000000 /* picking intersection set flag */ 
    
/* pfGSetPassFilter() */
#define PFGS_TEX_GSET            0x1
#define PFGS_NONTEX_GSET         0x2
#define PFGS_EMISSIVE_GSET       0x4
#define PFGS_NONEMISSIVE_GSET    0x8
#define PFGS_LAYER_GSET          0x10
#define PFGS_NONLAYER_GSET       0x20
#define PFGS_GSET_MASK           0xfff

/* pfGSetBBox() */
#define PFBOUND_STATIC		1
#define PFBOUND_DYNAMIC		2


#define PFGS_SPRITE	0x01
#define PFGS_COLORTABLE	0x02
#define PFGS_HIGHLIGHT	0x04
#define PFGS_FILTER	0x08
#define PFGS_DECALPLANE	0x10

class pfGeoSet
{
public:

    %addmethods
    {
      char *__str__()
      {
        static char temp[256];
        char *t;
        switch(self->getPrimType())
        {
          case PFGS_POINTS: t="points"; break;
          case PFGS_LINES: t="lines"; break;
          case PFGS_LINESTRIPS: t="linestrips"; break;
          case PFGS_FLAT_LINESTRIPS: t="flat linestrips"; break;
          case PFGS_TRIS: t="triangles"; break;
          case PFGS_QUADS: t="quads"; break;
          case PFGS_TRISTRIPS: t="triangle strips"; break;
          case PFGS_FLAT_TRISTRIPS: t="flat triangle strips"; break;
          case PFGS_TRIFANS: t="triangle fans"; break;
          case PFGS_FLAT_TRIFANS: t="flat triangle fans"; break;
          case PFGS_POLYS: t="polygons"; break;
          default: t="unknown primitives";
        }
        sprintf(temp, "pfGeoSet with %d %s", self->getNumPrims(), t);
        return temp;
      }
    }

    pfGeoSet();

    virtual ~pfGeoSet();

    static pfType* getClassType();

    void	setNumPrims(int _n);

    int		getNumPrims() const;

    void	setPrimType(int _type);

    int		getPrimType() const;

    void	setPrimLengths(int *_lengths);

    int*	getPrimLengths() const;

    int		getPrimLength(int i) const;

    void	setAttr(int _attr, int _bind, void* _alist, unsigned short* _ilist);

    int		getAttrBind(int _attr) const;

    void	getAttrLists(int _attr, void** _alist, unsigned short** _ilist) const;    

    int		getAttrRange(int _attr, int *_min, int *_max) const;

    void	setDrawMode(int _mode, int _val);

    int		getDrawMode(int _mode) const;

    void	setGState(pfGeoState *_gstate);

    pfGeoState*	getGState() const; 

    void	setGStateIndex(int _id);

    int		getGStateIndex() const;

    void	setHlight(pfHighlight *_hlight);

    pfHighlight*getHlight() const;

    void	setDecalPlane(pfPlane *_plane);

    pfPlane 	*getDecalPlane() const;

    void	setLineWidth(float _width);

    float	getLineWidth() const;

    void	setPntSize(float _s);

    float	getPntSize() const;

    void	setIsectMask(unsigned int _mask, int _setMode, int _bitOp);

    unsigned int getIsectMask() const;

    void	setDrawBin(short bin);

    int		getDrawBin(void) const;

    void	setDrawOrder(unsigned int order);

    unsigned int getDrawOrder(void) const;

    void	setBound(pfBox* _box, int _mode);

    int		getBound(pfBox* _box);

    void	setBoundFlux(pfFlux* _flux);

    pfFlux*	getBoundFlux(void);

    void	hideStripPrim(int i);

    void	unhideStripPrim(int i);

    int		isStripPrimHidden(int i);

    void	draw();

    void	compile();

    int		query(unsigned int _which, void *_dst) const;

    int		mQuery(unsigned int *_which, void *_dst) const;

    int		isect(pfSegSet *segSet, pfHit **hits[]);

    void	drawHlightOnly();

    static void	setPassFilter(uint _mask);

    static uint	getPassFilter();
};


class pfHit
{
public:

    pfHit();

    virtual ~pfHit();

    static pfType* getClassType();

    int		query(unsigned int _which, void *_dst) const; 

    int		mQuery(unsigned int *_which, void *_dst) const;

};


/* ------------------ pfHit Tokens --------------------- */

#define PFQHIT_FLAGS		1
#define PFQHIT_SEGNUM		2
#define PFQHIT_SEG		3
#define PFQHIT_POINT		4
#define PFQHIT_NORM		5
#define PFQHIT_VERTS		6
#define PFQHIT_TRI		7
#define PFQHIT_PRIM		8
#define PFQHIT_GSET		9
#define PFQHIT_STRING		10
#define PFQHIT_CHARINDEX	11
#define PFQHIT_FACEINDEX	12


/* pfHit flag values */
#define PFHIT_NONE		0x00 /* no intersection */
#define PFHIT_ISECT		0x01 /* intersection */
#define PFHIT_POINT		0x02 /* point info valid */
#define PFHIT_NORM		0x04 /* normal info valid */
#define PFHIT_TRI		0x08 /* triangle index valid */
#define PFHIT_PRIM		0x10 /* primitive index valid */
#define PFHIT_VERTS		0x20 /* triangle vertex info valid */

