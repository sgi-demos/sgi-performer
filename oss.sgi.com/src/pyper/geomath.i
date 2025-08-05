
// This file contains the public interface from the Performer
// header file pr/pfGeoMath.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pr/pfGeoMath.h>
%}


struct pfSeg 
{
public:
    pfVec3	pos;
    pfVec3	dir;
    float	length;

    pfSeg();

    void  makePts(const pfVec3& p1, const pfVec3& p2);
    void  makePolar(const pfVec3& _pos, float azi, float elev, float len);

    void  clip(const pfSeg *seg, float d1, float d2);

    int   closestPtsOn(const pfSeg *seg, pfVec3& dst1, pfVec3& dst2) const;
};


struct pfPlane 
{
public:
    pfVec3	normal;
    float	offset;		/* pt dot normal = offset */
 
    pfPlane();

    void  makePts(const pfVec3& p1, const pfVec3& p2, const pfVec3& p3);

    void  makeNormPt(const pfVec3& _norm, const pfVec3& _pos);

    void  displace(float _d);

    void  orthoXform(const pfPlane *pln, const pfMatrix& m);

    void  closestPtOn(const pfVec3& pt, pfVec3& dst) const;
};


struct pfSphere 
{
public:

    %addmethods
    {
      char *__str__()
      {
        static char temp[256];
        sprintf
        (
          temp, 
          "pfSphere at (%f,%f,%f), radius %f",
          self->center[0],
          self->center[1],
          self->center[2],
          self->radius
        );
        return temp;
      }
    }

    pfVec3	center;
    float	radius;

    pfSphere();

    void  makeEmpty();

    void  orthoXform(const pfSphere *sph, const pfMatrix& m);

    int   isect(const pfSeg *seg, float *d1, float *d2) const;
};


struct pfCylinder 
{
public:

    pfVec3	center;
    float	radius;
    pfVec3	axis;
    float	halfLength;

    pfCylinder();

    void  makeEmpty();

    void  orthoXform(const pfCylinder *cyl, const pfMatrix& m);

    int   isect(const pfSeg *seg, float *d1, float *d2) const;
};


struct pfBox 
{
public:

    pfVec3	min;
    pfVec3	max;

    pfBox();

    void  makeEmpty();

    void  xform(const pfBox *box, const pfMatrix& xform);

    int   isect(const pfSeg *seg, float *d1, float *d2) const;
};

#define PFIS_MAX_SEGS		32 /* maximum number of segments per request */


#define PFFRUST_LEFT	0
#define PFFRUST_RIGHT	1
#define PFFRUST_NEAR	2
#define PFFRUST_FAR	3
#define PFFRUST_BOTTOM	4    
#define PFFRUST_TOP	5

class pfPolytope 
{
public:
    // Constructors, destructors
    pfPolytope();
    virtual ~pfPolytope();

    // per class functions;
    static void	   init();
    static pfType* getClassType();

    int   getNumFacets() const;
    int   setFacet(int _i, const pfPlane *_p);
    int   getFacet(int _i, pfPlane *_p) const;

    int   removeFacet(int _i);
    void  orthoXform(const pfPolytope *_src, const pfMatrix& _mat);

};


#define PFFRUST_SIMPLE		0
#define PFFRUST_ORTHOGONAL	1
#define PFFRUST_PERSPECTIVE	2

#define PFFRUST_CALC_NONE	0
#define PFFRUST_CALC_HORIZ	1
#define PFFRUST_CALC_VERT	2


class pfFrustum : public pfPolytope
{
public:
    // Constructors, destructors
    pfFrustum();
    virtual ~pfFrustum();

    static void	   init();
    static pfType* getClassType();

    int		getFrustType() const;
    void	setAspect(int _which, float _widthHeightRatio) ;
    float	getAspect() const;
    void	getFOV(float* _fovh, float* _fovv) const;
    void	setNearFar(float _nearDist, float _farDist);

    void	getNearFar(float* _nearDist, float* _farDist) const;
    void 	getNear(pfVec3& _ll, pfVec3& _lr, pfVec3& _ul, pfVec3& _ur) const;
    void	getFar(pfVec3& _ll, pfVec3& _lr, pfVec3& _ul, pfVec3& _ur) const;
    void	getPtope(pfPolytope *dst) const;
    void   	getGLProjMat(pfMatrix &mat) const;

    int		getEye(pfVec3& _eye) const;

    void  makePersp(float _left, float _right, float _bot, float _top);
    void  makeOrtho(float _left, float _right, float _bot, float _top);
    void  makeSimple(float _fov);
    void  orthoXform(const pfFrustum* _fr2, const pfMatrix& _mat);

    void  apply() const;
};

