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
// pfGeoMath.h
//
// $Revision: 1.67 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_GEOMATH_H__
#define __PF_GEOMATH_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>

//CAPI:struct
struct DLLEXPORT pfSeg {
public:
    pfVec3	pos;
    pfVec3	dir;
    float	length;

public:
    //CAPI:private
    pfSeg() {}

    // Construct a pfSeg from two points
    // CAPI:private
    pfSeg(const pfVec3& p1, const pfVec3& p2) { makePts(p1, p2); }
    // Construct a pfSeg from polar coordinates
    // CAPI:private
    pfSeg(const pfVec3& _pos, float azi, float elev, float len) { makePolar(_pos, azi, elev, len); }
    // Construct a pfSeg from a position, direction and length
    // CAPI:private
    pfSeg(const pfVec3& _pos, const pfVec3& _dir, float _length) 
      { PFCOPY_VEC3(pos, _pos); PFCOPY_VEC3(dir, _dir); length = _length; }

public:
    //CAPI:verb
    void  makePts(const pfVec3& p1, const pfVec3& p2);
    void  makePolar(const pfVec3& _pos, float azi, float elev, float len);

    void  clip(const pfSeg *seg, float d1, float d2);

    int   closestPtsOn(const pfSeg *seg, pfVec3& dst1, pfVec3& dst2) const;

    void  fromSegd(const pfSegd *segd);
};

//CAPI:struct
struct DLLEXPORT pfSegd {
    //CAPI:basename Segd
public:
    pfVec3d	pos;
    pfVec3d	dir;
    double	length;

public:
    //CAPI:private
    pfSegd() {}

    // Construct a pfSegd from two points
    // CAPI:private
    pfSegd(const pfVec3& p1, const pfVec3& p2) 
			{ makePts(p1, p2); }
    // Construct a pfSegd from two points
    // CAPI:private
    pfSegd(const pfVec3d& p1, const pfVec3d& p2) 
			{ makePts(p1, p2); }
    // Construct a pfSeg from polar coordinates
    // CAPI:private
    pfSegd(const pfVec3& _pos, float azi, float elev, float len) 
		{ makePolar(_pos, azi, elev, len); }

    // Construct a pfSeg from polar coordinates
    // CAPI:private
    pfSegd(const pfVec3d& _pos, double azi, double elev, double len) 
		{ makePolar(_pos, azi, elev, len); }

    // Construct a pfSeg from a position, direction and length
    // CAPI:private
    pfSegd(const pfVec3& _pos, const pfVec3& _dir, float _length) 
      { PFCOPY_VEC3(pos, _pos); PFCOPY_VEC3(dir, _dir); length = _length; }

    // Construct a pfSeg from a position, direction and length
    // CAPI:private
    pfSegd(const pfVec3d& _pos, const pfVec3d& _dir, double _length) 
      { PFCOPY_VEC3(pos, _pos); PFCOPY_VEC3(dir, _dir); length = _length; }

public:
    //CAPI:verb MakePts
    void  makePts(const pfVec3& p1, const pfVec3& p2);

    //CAPI:verb MakePtsd
    void  makePts(const pfVec3d& p1, const pfVec3d& p2);

    //CAPI:verb MakePolar
    void  makePolar(const pfVec3& _pos, float azi, float elev, float len);

    //CAPI:verb MakePolard
    void  makePolar(const pfVec3d& _pos, float azi, float elev, float len);
    // CAPI:verb

    void  clip(const pfSegd *seg, float d1, float d2);

    int   closestPtsOn(const pfSegd *seg, pfVec3& dst1, pfVec3& dst2) const;

    //CAPI:verb ClosestPtsOnd
    int   closestPtsOn(const pfSegd *seg, pfVec3d& dst1, pfVec3d& dst2) const;
    // CAPI:verb

    void  fromSeg(const pfSeg *seg);
};

//CAPI:struct
struct DLLEXPORT pfPlane {
public:
    pfVec3	normal;
    float	offset;		/* pt dot normal = offset */
 
public:
    //CAPI:private
    pfPlane() {}
    // Construct a plane given 3 points.
    // CAPI:private
    pfPlane(const pfVec3& p1, const pfVec3& p2, const pfVec3& p3) { makePts(p1, p2, p3); }
    // Construct a plane given normal and a point to pass through
    // CAPI:private
    pfPlane(const pfVec3& _norm, const pfVec3& _pos) { makeNormPt(_norm, _pos); }
    // Construct a plane given normal and distance from origin along normal.
    // CAPI:private
    pfPlane(const pfVec3& _norm, float _offset) { PFCOPY_VEC3(normal, _norm); offset = _offset; }

public:
    //CAPI:verb
    void  makePts(const pfVec3& p1, const pfVec3& p2, const pfVec3& p3);
    void  makeNormPt(const pfVec3& _norm, const pfVec3& _pos);

    void  displace(float _d);

    //CAPI:verb HalfSpaceContainsBox
    int   contains(const pfBox *box) const;
    //CAPI:verb HalfSpaceContainsSphere
    int   contains(const pfSphere *sph) const;
    //CAPI:verb HalfSpaceContainsCyl
    int   contains(const pfCylinder *cyl) const;
    //CAPI:verb HalfSpaceContainsPt
    int   contains(const pfVec3& pt) const;

    void  orthoXform(const pfPlane *pln, const pfMatrix& m);

    void  closestPtOn(const pfVec3& pt, pfVec3& dst) const;
    //CAPI:verb PlaneIsectSeg
    int   isect(const pfSeg *seg, float *d) const;
    //CAPI:verb PlaneIsectSegd
    int   isect(const pfSegd *seg, double *d) const;
    //CAPI:verb HalfSpaceIsectSeg
    int   isect(const pfSeg *seg, float *d1, float *d2) const;
    //CAPI:verb HalfSpaceIsectSegd
    int   isect(const pfSegd *seg, double *d1, double *d2) const;

    int   clipConvexPolygon(int nVerts, pfVec3 verts[], int texdim, float texcoords[]);

    //CAPI:private
    int   clipConvexPolygon(int nVerts, pfVec3 verts[], pfVec2 texcoords[/*nVerts+1*/]) {
	return clipConvexPolygon(nVerts, verts, 2, &texcoords[0][0]);
    }
    int   clipConvexPolygon(int nVerts, pfVec3 verts[], pfVec3 texcoords[/*nVerts+1*/]) {
	return clipConvexPolygon(nVerts, verts, 3, &texcoords[0][0]);
    }

};

//CAPI:struct
struct DLLEXPORT pfSphere {
public:
    pfVec3	center;
    float	radius;

public:
    //CAPI:private
    pfSphere() {}
    // Construct a sphere from a given center and radius
    // CAPI:private
    pfSphere(const pfVec3& _center, float _radius) 
      { PFCOPY_VEC3(center, _center); radius = _radius; }

public:
    //CAPI:verb
    void  makeEmpty();

    //CAPI:verb SphereContainsPt
    int   contains(const pfVec3& pt) const;
    //CAPI:verb SphereContainsSphere
    int   contains(const pfSphere *_sph) const;
    //CAPI:verb SphereContainsCyl
    int   contains(const pfCylinder *cyl) const;
    //CAPI:verb SphereContainsCyld
    int   contains(const pfCylinderd *cyl) const;
    //CAPI:verb SphereContainsBox
    int   contains(const pfBox *box) const;


    //CAPI:verb SphereAroundPts
    void  around(const pfVec3* pts, int npt);
    //CAPI:verb SphereAroundSpheres
    void  around(const pfSphere **sphs, int nsph);
    //CAPI:verb SphereAroundBoxes
    void  around(const pfBox **boxes, int nbox);
    //CAPI:verb SphereAroundCyls
    void  around(const pfCylinder **cyls, int ncyl);

    //CAPI:verb SphereExtendByPt
    void  extendBy(const pfVec3& pt);
    //CAPI:verb SphereExtendBySphere
    void  extendBy(const pfSphere *sph);
    //CAPI:verb SphereExtendByCyl
    void  extendBy(const pfCylinder *cyl);

    void  orthoXform(const pfSphere *sph, const pfMatrix& m);

    //CAPI:verb SphereIsectSeg
    int   isect(const pfSeg *seg, float *d1, float *d2) const;
};

// Sphere with double-precision center and single-precision radius
//CAPI:struct
struct DLLEXPORT pfSpheredf {
public:
    pfVec3d	center;
    float	radius;

public:
    //CAPI:private
    pfSpheredf() {}
    // Construct a sphere from a given center and radius
    // CAPI:private
    pfSpheredf(const pfVec3d& _center, float _radius) 
      { PFCOPY_VEC3(center, _center); radius = _radius; }

public:
    //CAPI:verb
    void  makeEmpty();

    //CAPI:verb SpheredfContainsPt
    int   contains(const pfVec3& pt) const;
    //CAPI:verb SpheredfContainsSpheredf
    int   contains(const pfSpheredf *_sph) const;
    //CAPI:verb SpheredfContainsCyl
    int   contains(const pfCylinder *cyl) const;
    //CAPI:verb SpheredfContainsCyld
    int   contains(const pfCylinderd *cyl) const;
    //CAPI:verb SpheredfContainsBox
    int   contains(const pfBox *box) const;


    //CAPI:verb SpheredfAroundPts
    void  around(const pfVec3* pts, int npt);
    //CAPI:verb SpheredfAroundSpheredfs
    void  around(const pfSpheredf **sphs, int nsph);
    //CAPI:verb SpheredfAroundBoxes
    void  around(const pfBox **boxes, int nbox);
    //CAPI:verb SpheredfAroundCyls
    void  around(const pfCylinder **cyls, int ncyl);

    //CAPI:verb SpheredfExtendByPt
    void  extendBy(const pfVec3& pt);
    //CAPI:verb SpheredfExtendBySpheredf
    void  extendBy(const pfSpheredf *sph);
    //CAPI:verb SpheredfExtendByCyl
    void  extendBy(const pfCylinder *cyl);

    void  orthoXform(const pfSpheredf *sph, const pfMatrix4d& m);

    // isect() not implemented for pfSpheredf
};

// Sphere with double-precision center and radius
//CAPI:struct
struct DLLEXPORT pfSpheredd {
public:
    pfVec3d	center;
    double	radius;

public:
    //CAPI:private
    pfSpheredd() {}
    // Construct a sphere from a given center and radius
    // CAPI:private
    pfSpheredd(const pfVec3d& _center, double _radius) 
      { PFCOPY_VEC3(center, _center); radius = _radius; }

public:
    //CAPI:verb
    void  makeEmpty();

    //CAPI:verb SphereddContainsPt
    int   contains(const pfVec3& pt) const;
    //CAPI:verb SphereddContainsSpheredd
    int   contains(const pfSpheredd *_sph) const;
    //CAPI:verb SphereddContainsCyl
    int   contains(const pfCylinder *cyl) const;
    //CAPI:verb SphereddContainsCyld
    int   contains(const pfCylinderd *cyl) const;
    //CAPI:verb SphereddContainsBox
    int   contains(const pfBox *box) const;


    //CAPI:verb SphereddAroundPts
    void  around(const pfVec3* pts, int npt);
    //CAPI:verb SphereddAroundSpheredds
    void  around(const pfSpheredd **sphs, int nsph);
    //CAPI:verb SphereddAroundBoxes
    void  around(const pfBox **boxes, int nbox);
    //CAPI:verb SphereddAroundCyls
    void  around(const pfCylinder **cyls, int ncyl);

    //CAPI:verb SphereddExtendByPt
    void  extendBy(const pfVec3& pt);
    //CAPI:verb SphereddExtendBySpheredd
    void  extendBy(const pfSpheredd *sph);
    //CAPI:verb SphereddExtendByCyl
    void  extendBy(const pfCylinder *cyl);

    void  orthoXform(const pfSpheredd *sph, const pfMatrix4d& m);

    // isect() not implemented for pfSpheredd
};



//CAPI:struct
struct DLLEXPORT pfCylinder {
    //CAPI:basename Cyl
public:
    pfVec3	center;
    float	radius;
    pfVec3	axis;
    float	halfLength;

public:
    //CAPI:private
    pfCylinder() {}

    // Construct a pfCylinder with a given center, radius, axis and halfLength
    // CAPI:private
    pfCylinder(const pfVec3& _center, float _radius, const pfVec3& _axis, float _halfLength)
      { PFCOPY_VEC3(center, _center); radius = _radius; 
        PFCOPY_VEC3(axis, _axis); halfLength = _halfLength; }

public:
    //CAPI:verb
    void  makeEmpty();

    //CAPI:verb CylContainsPt
    int   contains(const pfVec3& pt) const;

    void  orthoXform(const pfCylinder *cyl, const pfMatrix& m);

    //CAPI:verb CylAroundPts
    void  around(const pfVec3 *pts, int npt);
    //CAPI:verb CylAroundSegs
    void  around(const pfSeg **segs, int nseg);
    //CAPI:verb CylAroundSpheres
    void  around(const pfSphere **sphs, int nsph);
    //CAPI:verb CylAroundBoxes
    void  around(const pfBox **boxes, int nbox);

    //CAPI:verb CylExtendBySphere
    void  extendBy(const pfSphere *sph);
    //CAPI:verb CylExtendByCyl
    void  extendBy(const pfCylinder *cyl);
    //CAPI:verb CylExtendByBox
    void  extendBy(const pfVec3& pt);

    //CAPI:verb CylIsectSeg
    int   isect(const pfSeg *seg, float *d1, float *d2) const;

    //CAPI:verb 
    void fromCylinderd(pfCylinderd *cyl);

};

//CAPI:struct
struct DLLEXPORT pfCylinderd {
    //CAPI:basename Cyld
public:
    pfVec3d	center;
    double	radius;
    pfVec3d	axis;
    double	halfLength;

public:
    //CAPI:private
    pfCylinderd() {}

    // Construct a pfCylinderd with a given center, radius, 
    // axis and halfLength
    // CAPI:private
    pfCylinderd(const pfVec3d& _center, double _radius, const pfVec3d& _axis, double _halfLength)
      { PFCOPY_VEC3(center, _center); radius = _radius; 
        PFCOPY_VEC3(axis, _axis); halfLength = _halfLength; }

    pfCylinderd(const pfVec3& _center, float _radius, const pfVec3& _axis, float _halfLength)
      { PFCOPY_VEC3(center, _center); radius = _radius; 
        PFCOPY_VEC3(axis, _axis); halfLength = _halfLength; }

public:
    //CAPI:verb
    void  makeEmpty();

    //CAPI:verb CyldContainsPt
    int   contains(const pfVec3& pt) const;

    //CAPI:verb CyldContainsPtd
    int   contains(const pfVec3d& pt) const;

    //CAPI:verb 
    void  orthoXform(const pfCylinderd *cyl, const pfMatrix4d& m);

    //CAPI:verb CyldAroundPts
    void  around(const pfVec3 *pts, int npt);

    //CAPI:verb CyldAroundSegs
    void  around(const pfSeg **segs, int nseg);

    //CAPI:verb CyldAroundSegds
    void  around(const pfSegd **segs, int nseg);

    //CAPI:verb CyldAroundSpheres
    void  around(const pfSphere **sphs, int nsph);

    //CAPI:verb CyldAroundSpheresdd
    void  around(const pfSpheredd **sphs, int nsph);

    //CAPI:verb CyldAroundBoxes
    void  around(const pfBox **boxes, int nbox);

    //CAPI:verb CyldExtendBySphere
    void  extendBy(const pfSphere *sph);

    //CAPI:verb CyldExtendBySpheredd
    void  extendBy(const pfSpheredd *sph);

    //CAPI:verb CyldExtendByCyl
    void  extendBy(const pfCylinder *cyl);

    //CAPI:verb CyldExtendByCyld
    void  extendBy(const pfCylinderd *cyl);

    //CAPI:verb CyldExtendByBox
    void  extendBy(const pfVec3& pt);

    //CAPI:verb CyldExtendByBoxd
    void  extendBy(const pfVec3d& pt);

    //CAPI:verb CyldIsectSeg
    int   isect(const pfSeg *seg, float *d1, float *d2) const;
    //CAPI:verb CyldIsectSegd
    int   isect(const pfSegd *seg, double *d1, double *d2) const;

    //CAPI:verb 
    void fromCylinder(pfCylinder *cyl);

};

//CAPI:struct
struct DLLEXPORT pfBox {
public:
    pfVec3	min;
    pfVec3	max;

public:
    //CAPI:private
    pfBox() {}

    // Construct a box given min and max
    // CAPI:private
    pfBox(const pfVec3& _min, const pfVec3& _max)
      { PFCOPY_VEC3(min, _min); PFCOPY_VEC3(max, _max); }

public:
    //CAPI:verb
    void  makeEmpty();

    //CAPI:verb BoxContainsPt
    int   contains(const pfVec3& pt) const;
    //CAPI:verb BoxContainsBox
    int   contains(const pfBox *_inbox);
    //CAPI:verb BoxContainsSphere
    int   contains(const pfSphere *sphere) const;


    void  xform(const pfBox *box, const pfMatrix& xform);

    //CAPI:verb BoxAroundPts
    void  around(const pfVec3 *pts, int npt);
    //CAPI:verb BoxAroundSpheres
    void  around(const pfSphere **sphs, int nsph);
    //CAPI:verb BoxAroundBoxes
    void  around(const pfBox **boxes, int nbox);
    //CAPI:verb BoxAroundCyls
    void  around(const pfCylinder **cyls, int ncyl);

    //CAPI:verb BoxExtendByPt
    void  extendBy(const pfVec3& pt);
    //CAPI:verb BoxExtendByBox
    void  extendBy(const pfBox *box);

    //CAPI:verb BoxIsectSeg
    int   isect(const pfSeg *seg, float *d1, float *d2) const;

};
#define PFIS_MAX_SEGS		32 /* maximum number of segments per request */

struct DLLEXPORT pfSegSet
{
    int			mode;
    void*		userData;
    pfSeg		segs[PFIS_MAX_SEGS];
    unsigned int	activeMask;
    unsigned int	isectMask;
    void*		bound;
    int 		(*discFunc)(pfHit*);
public:
    void 		fromSegSetd(pfSegSetd *ssetd);
};

struct DLLEXPORT pfSegSetd
{
    int			mode;
    void*		userData;
    pfSegd		segs[PFIS_MAX_SEGS];
    unsigned int	activeMask;
    unsigned int	isectMask;
    void*		bound;
    int 		(*discFunc)(pfHit*);
public:
    void 		fromSegSet(pfSegSet *sset);
};


/*
 * plane definitions for frustum
 *
 * unusual ordering for efficiency in computing isections
 */

#define PFFRUST_LEFT	0
#define PFFRUST_RIGHT	1
#define PFFRUST_NEAR	2
#define PFFRUST_FAR	3
#define PFFRUST_BOTTOM	4    
#define PFFRUST_TOP	5

#define PFPOLYTOPE ((pfPolytope*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFPOLYTOPEBUFFER ((pfPolytope*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfPolytope : public pfObject
{
// CAPI:basename Ptope
// CAPI:argname  ptp
public:
    // Constructors, destructors
    pfPolytope();
    // CAPI:private
    pfPolytope(int numFacets);
    virtual ~pfPolytope();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

public:
    // sets and gets
    int   getNumFacets() const { return numFacets; }
    // CAPI:err FALSE
    int   setFacet(int _i, const pfPlane *_p);
    int   getFacet(int _i, pfPlane *_p) const;

public:
    // exposed functions
    // CAPI:err FALSE
    // CAPI:verb RemovePtopeFacet
    int   removeFacet(int _i);
    // CAPI:verb
    void  orthoXform(const pfPolytope *_src, const pfMatrix& _mat);
    // CAPI:verb PtopeContainsPt
    int   contains(const pfVec3& pt) const;

    // CAPI:verb PtopeContainsSphere
    int   contains(const pfSphere *sphere) const;
    // CAPI:verb PtopeContainsBox
    int   contains(const pfBox *box) const;
    // CAPI:verb PtopeContainsCyl
    int   contains(const pfCylinder *cyl) const;
    // CAPI:verb PtopeContainsPtope
    int   contains(const pfPolytope *ptope) const;


protected:
    int		availFacets;
    int		numFacets;
    pfPlane	*facets;	/* plane normals directed outward */

private:
    static pfType *classType;
};


/*------------------------------ pfFrustum ----------------------------------*/

extern "C" {     // EXPORT to CAPI
/* ----------------------- pfFrustum Tokens ----------------------- */

#define PFFRUST_SIMPLE		0
#define PFFRUST_ORTHOGONAL	1
#define PFFRUST_PERSPECTIVE	2

#define PFFRUST_CALC_NONE	0
#define PFFRUST_CALC_HORIZ	1
#define PFFRUST_CALC_VERT	2


} // extern "C" END of C include export

#define PFFRUSTUM ((pfFrustum*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFFRUSTUMBUFFER ((pfFrustum*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfFrustum : public pfPolytope
{
// CAPI:basename Frust
// CAPI:argname  fr
public:
    // Constructors, destructors
    pfFrustum();
    virtual ~pfFrustum();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

public:
    // sets and gets
    // CAPI:verb GetFrustType
    int		getFrustType() const { return frustType; }
    // CAPI:noverb
    void	setAspect(int _which, float _widthHeightRatio) ;
    float	getAspect() const { return xyaspect; }
    void	getFOV(float* _fovh, float* _fovv) const { 
			*_fovh = fovh ; *_fovv = fovv; }
    void	setNearFar(float _nearDist, float _farDist);

    void	getNearFar(float* _nearDist, float* _farDist) const {
			*_nearDist = nearDist ; *_farDist = farDist; }

    void 	getNear(pfVec3& _ll, pfVec3& _lr, pfVec3& _ul, pfVec3& _ur) const;
    void	getFar(pfVec3& _ll, pfVec3& _lr, pfVec3& _ul, pfVec3& _ur) const;
    void	getPtope(pfPolytope *dst) const { dst->copy((pfPolytope*)this); }
    void   	getGLProjMat(pfMatrix &mat) const;
    void	getLeftRightBottomTop(float *l, float *r, float *b, float *t) const;

    // CAPI:err FALSE
    int		getEye(pfVec3& _eye) const;

public:
    // exposed functions
    // CAPI:verb
    void  makePersp(float _left, float _right, float _bot, float _top);
    void  makeOrtho(float _left, float _right, float _bot, float _top);
    void  makeSimple(float _fov);
    void  orthoXform(const pfFrustum* _fr2, const pfMatrix& _mat);

    // CAPI:verb RecommendNearFrustSphere
    int  recommendNear(const pfSphere *sph, pfVec3 &result_point, int use_cone) const;
    // CAPI:verb RecommendNearFrustPolygon
    int  recommendNear(int nVerts, pfVec3 convexPolygonVerts[], pfVec3 &recommended, pfVec2 convexPolygonTexCoords[], pfVec2 &recommendedTextureCoord) const;

    // CAPI:verb FrustContainsPt
    int   contains(const pfVec3& pt) const;

    // CAPI:verb FrustContainsSphere
    int   contains(const pfSphere *sphere) const;
    // CAPI:verb FrustContainsBox
    int   contains(const pfBox *box) const;
    // CAPI:verb FrustContainsCyl
    int   contains(const pfCylinder *cyl) const;

    void  apply() const;


protected:
    int		frustType;
    pfVec3	eye;
    float	nearDist, farDist;
    float	xcenter, ycenter;
    float	fov, fovh, fovv;
    float	xyaspect;
    pfVec3	corners[8];

private:
    static pfType *classType;
};

extern "C" { // EXPORT to CAPI

/*------------------------- Triangle Intersection ---------------------------*/

extern int DLLEXPORT pfTriIsectSeg(const PFVEC3 _pt1, const PFVEC3 _pt2, const PFVEC3 _pt3, const pfSeg* _seg, float* _d);

/*
 * Isection 
 */
#define PFBOX_CONTAINS_PT(_box, _pt) \
   ((_pt)[0] >= (_box)->min[0] && \
    (_pt)[1] >= (_box)->min[1] && \
    (_pt)[2] >= (_box)->min[2] && \
    (_pt)[0] <= (_box)->max[0] && \
    (_pt)[1] <= (_box)->max[1] && \
    (_pt)[2] <= (_box)->max[2])

#define PFHALF_SPACE_CONTAINS_PT(_pln, _pt) \
	(PFDOT_VEC3((_pt), (_pln)->normal) <= (_pln)->offset)

#define PFSPHERE_CONTAINS_PT(_sp, _pt) \
	(PFSQR_DISTANCE_PT3((_sp)->center, (_pt)) <= PF_SQUARE((_sp)->radius))

/*
 * Boxes
 */

#define PFBOX_EXTENDBY_BOX(_dst, _box) \
    ((_box)->min[0] < (_dst)->min[0] ? (_dst)->min[0] = (_box)->min[0] : 0, \
     (_box)->min[1] < (_dst)->min[1] ? (_dst)->min[1] = (_box)->min[1] : 0, \
     (_box)->min[2] < (_dst)->min[2] ? (_dst)->min[2] = (_box)->min[2] : 0, \
     (_box)->max[0] > (_dst)->max[0] ? (_dst)->max[0] = (_box)->max[0] : 0, \
     (_box)->max[1] > (_dst)->max[1] ? (_dst)->max[1] = (_box)->max[1] : 0, \
     (_box)->max[2] > (_dst)->max[2] ? (_dst)->max[2] = (_box)->max[2] : 0)

/* 
 * Always testing against both bounds is not redundant because
 * box may be "empty" in one or more dimensions.
 */
#define PFBOX_EXTENDBY_PT(_dst, _pt) \
    ((_pt)[0] < (_dst)->min[0] ? (_dst)->min[0] = (_pt)[0] : 0, \
     (_pt)[1] < (_dst)->min[1] ? (_dst)->min[1] = (_pt)[1] : 0, \
     (_pt)[2] < (_dst)->min[2] ? (_dst)->min[2] = (_pt)[2] : 0, \
     (_pt)[0] > (_dst)->max[0] ? (_dst)->max[0] = (_pt)[0] : 0, \
     (_pt)[1] > (_dst)->max[1] ? (_dst)->max[1] = (_pt)[1] : 0, \
     (_pt)[2] > (_dst)->max[2] ? (_dst)->max[2] = (_pt)[2] : 0)

#define PFNONEMPTY_BOX_EXTENDBY_PT(_dst, _pt) \
    (((_pt)[0] < (_dst)->min[0] ? (_dst)->min[0] = (_pt)[0] : \
      ((_pt)[0] > (_dst)->max[0] ? (_dst)->max[0] = (_pt)[0] : 0)), \
     ((_pt)[1] < (_dst)->min[1] ? (_dst)->min[1] = (_pt)[1] : \
      ((_pt)[1] > (_dst)->max[1] ? (_dst)->max[1] = (_pt)[1] : 0)), \
     ((_pt)[2] < (_dst)->min[2] ? (_dst)->min[2] = (_pt)[2] : \
      ((_pt)[2] > (_dst)->max[2] ? (_dst)->max[2] = (_pt)[2] : 0)))

#define PFORTHO_XFORM_SPHERE(_dst, _sph, _m) \
	((_dst)->center.xformPt((sph)->center, (_m)), \
	 (_dst)->radius = ((_sph)->radius * \
			   pfSqrt(PF_SQUARE((_m)[0][0]) + \
				  PF_SQUARE((_m)[0][1]) + \
				  PF_SQUARE((_m)[0][2]))))
	    
} // extern "C" END of C include export


#endif	// !__PF_GEOMATH_H__
