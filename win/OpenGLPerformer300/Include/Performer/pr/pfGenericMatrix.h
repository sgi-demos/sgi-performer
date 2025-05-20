// XXX pfGenericMatrix.h
// XXX This should go in pfLinMath.h
// XXX or fixed up to stand alone and externally.
// XXX For now, interested internal source files
// XXX just include it after pfLinMath.h
// XXX (if we include it from pfLinMath.h explicitly,
// XXX thewhole world would get recompiled every time we edit this)

#ifndef __PF_GENERIC_MATRIX_H__
#define __PF_GENERIC_MATRIX_H__

#include <Performer/pr/pfLinMath.h>

enum pfMatrixType
{
    PF_MATRIXTYPE_SS=0,
    PF_MATRIXTYPE_SD=1,
    PF_MATRIXTYPE_DS=2,
    PF_MATRIXTYPE_DD=3
};

//CAPI:struct
struct pfGenericMatrix
{
    PFSTRUCT_DECLARE

private:
    friend class pfGenericMatStack; // XXX this sucks, try to get around it

    #ifdef LOCAL_BUILD
    // XXX The following should just be a union of pfMatrix and pfMatrix4d,
    // but since they  have contructors they are not allowed in unions.
    // C++ is crap.
    // This must be the first element of the structure
    // since we sometimes use pointers
    // to pfMatrix, pfMatrix4d, and pfGenericMatrix interchangeably.
    //

    // XXX - allan  Even worse,
    // Linux doesn't like the implicit conversion of the _s[4][4] 
    // and _d[4][4] union into a pfMatrix& in the s() and d() member 
    // funcs, complains there is no constructor with those args.  If 
    // you create a constructor with those args then it returns a ref 
    // to an out-of-scope matrix, bad bad bad.  C++ does suck.  
    //
    // Had initially made it a struct simply containing both elements
    // for pf2.3/2.3.1.  For pf2.4, realized that with Stupid Compiler
    // Tricks, could caste/dereference our way to total bliss.  Or is
    // it ignorance..?
    #endif

    union {
	float _s[4][4];  // single precision version
	double _d[4][4]; // double precision version
    } u;

    const pfMatrix   &s() const {
	PFASSERTDEBUG((char *)this > (char *)4096);
	/* PFASSERTDEBUG(matrixType == PF_MATRIXTYPE_SS); */
        #ifdef __linux__
	return (pfMatrix &) *(pfMatrix*)u._s;
        #else
	return (pfMatrix &)u._s;
        #endif
    }

    const pfMatrix4d &d() const {
	PFASSERTDEBUG((char *)this > (char *)4096);
	/* PFASSERTDEBUG(matrixType == PF_MATRIXTYPE_SD || matrixType == PF_MATRIXTYPE_DS || matrixType == PF_MATRIXTYPE_DD); */
        #ifdef __linux__
	return (const pfMatrix4d &) *(pfMatrix*)u._d;
        #else
	return (const pfMatrix4d &) u._d;
        #endif  /* __linux__ */
    }

    pfMatrix   &s() {
	PFASSERTDEBUG((char *)this > (char *)4096);
        /* PFASSERTDEBUG(matrixType == PF_MATRIXTYPE_SS); */
        #ifdef __linux__
	return (pfMatrix &) *(pfMatrix*)u._s;
        #else
	return (pfMatrix &) u._s;
        #endif  /* __linux__ */
    }

    pfMatrix4d &d() {
	PFASSERTDEBUG((char *)this > (char *)4096);
	/* PFASSERTDEBUG(matrixType == PF_MATRIXTYPE_SD || matrixType == PF_MATRIXTYPE_DS || matrixType == PF_MATRIXTYPE_DD); */
        #ifdef __linux__
	return (pfMatrix4d &) *(pfMatrix*)u._d;
        #else
	return (pfMatrix4d &) u._d;
        #endif
    }

    pfMatrixType matrixType;

    typedef void MultFunc(pfGenericMatrix &_result, pfMatrixType _t1, const pfGenericMatrix &_m1, pfMatrixType _t2, const pfGenericMatrix &_m2);
    static MultFunc *multFuncs[4][4];

public:
    int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

    // XXX This should be in the C API!!
    // XXX but CAPI generator is broken for returning enums...
    //CAPI:private
    enum pfMatrixType getMatrixType() const { return matrixType; }

    // XXX This is what is used when pfGenericMatStack is
    // XXX initialized, and it makes me nervous...
    // XXX do we really want it to always be initialized with SP
    // XXX without even having to say it explicitly?
    // XXX Probably not; I forgot about it after only a couple of days...
    void	makeIdent(pfMatrixType _matrixType = PF_MATRIXTYPE_SS)
    {
	matrixType = _matrixType;
	if (_matrixType == PF_MATRIXTYPE_SS)
	    s().makeIdent();
	else
	    d().makeIdent();
    }

    // XXX move the contents of these functions down below
    /* inline */ void copy(const pfMatrix& _m, pfMatrixType _t = PF_MATRIXTYPE_SS)
    {
	matrixType = _t;
	if (_t == PF_MATRIXTYPE_SS)
	    s().copy(_m);		 // mem copy
	else
	{
	    if (&_m == (pfMatrix *)&u._s) // in-place-- be careful not to clobber!
	    {
		pfMatrix temp = _m;      // mem copy
		PFCOPY_MAT(d(), temp);   // promote
	    }
	    else
		PFCOPY_MAT(d(), _m);	// promote
	}
    }
    /* inline */ void copy(const pfMatrix4d& _m, pfMatrixType _t = PF_MATRIXTYPE_DD)
    {
	matrixType = _t;
	if (_t == PF_MATRIXTYPE_SS)
	    PFCOPY_MAT(s(), _m);    // in-place demotion is safe
	else
	    d().copy(_m); 	       // mem copy
    }
    // XXX pick consistent order for matrix/type arguments!
    /* inline */ void copy(const pfGenericMatrix& _m, pfMatrixType _t)
    {
	if (_m.getMatrixType() == PF_MATRIXTYPE_SS)
	    copy(_m.s(), _t);
	else // the other three matrix types are stored as double
	    copy(_m.d(), _t);
    }
    /* inline */ void copy(const pfGenericMatrix& _m)
    {
	copy(_m, _m.getMatrixType());
    }

    // XXX this should never be called internally!
    // XXX comment it out for the internal build?
    /* inline */ void	get4f(pfMatrix& _m) const {
	if (getMatrixType() == PF_MATRIXTYPE_SS)
	    _m.copy(s());
	else
	    PFCOPY_MAT(_m, d());
    }
    /* inline */ void	get4d(pfMatrix4d& _m) const {
	if (getMatrixType() == PF_MATRIXTYPE_SS)
	    PFCOPY_MAT(_m, s());
	else
	    _m.copy(d());
    }

    //
    // The following are the only functions supported.
    // We do NOT add, subtract, or scale generic matrices,
    // but we can copy, multiply and invert every which way.
    //
    // XXX encapsulate these darn assertions so they aren't so obtrusive!
    // XXX the 4g stuff in the names is obnoxious...
    // XXX is there a nicer scheme?

    //CAPI: verb
    void mult(const pfGenericMatrix & _m1, const pfGenericMatrix & _m2)
    {
	pfMatrixType t1 = _m1.getMatrixType();
	pfMatrixType t2 = _m2.getMatrixType();
	PFASSERTDEBUG(t1 == PF_MATRIXTYPE_SS
		   || t1 == PF_MATRIXTYPE_SD
		   || t1 == PF_MATRIXTYPE_DS
		   || t1 == PF_MATRIXTYPE_DD);
	PFASSERTDEBUG(t2 == PF_MATRIXTYPE_SS
		   || t2 == PF_MATRIXTYPE_SD
		   || t2 == PF_MATRIXTYPE_DS
		   || t2 == PF_MATRIXTYPE_DD);
	multFuncs[t1][t2](*this, t1, _m1, t2, _m2);
    }

    void mult4f4g(const pfMatrix & _m1, const pfGenericMatrix & _m2)
    {
	pfMatrixType t1 = PF_MATRIXTYPE_SS;
	pfMatrixType t2 = _m2.getMatrixType();
	PFASSERTDEBUG(t1 == PF_MATRIXTYPE_SS
		   || t1 == PF_MATRIXTYPE_SD
		   || t1 == PF_MATRIXTYPE_DS
		   || t1 == PF_MATRIXTYPE_DD);
	PFASSERTDEBUG(t2 == PF_MATRIXTYPE_SS
		   || t2 == PF_MATRIXTYPE_SD
		   || t2 == PF_MATRIXTYPE_DS
		   || t2 == PF_MATRIXTYPE_DD);
	multFuncs[t1][t2](*this, t1, (const pfGenericMatrix &)_m1, t2, _m2);
    }
    void mult4g4f(const pfGenericMatrix & _m1, const pfMatrix & _m2)
    {
	pfMatrixType t1 = _m1.getMatrixType();
	pfMatrixType t2 = PF_MATRIXTYPE_SS;
	PFASSERTDEBUG(t1 == PF_MATRIXTYPE_SS
		   || t1 == PF_MATRIXTYPE_SD
		   || t1 == PF_MATRIXTYPE_DS
		   || t1 == PF_MATRIXTYPE_DD);
	PFASSERTDEBUG(t2 == PF_MATRIXTYPE_SS
		   || t2 == PF_MATRIXTYPE_SD
		   || t2 == PF_MATRIXTYPE_DS
		   || t2 == PF_MATRIXTYPE_DD);
	multFuncs[t1][t2](*this, t1, _m1, t2, (const pfGenericMatrix &)_m2);
    }

    void mult4d4g(const pfMatrix4d & _m1, const pfGenericMatrix & _m2)
    {
	pfMatrixType t1 = PF_MATRIXTYPE_DD;
	pfMatrixType t2 = _m2.getMatrixType();
	PFASSERTDEBUG(t1 == PF_MATRIXTYPE_SS
		   || t1 == PF_MATRIXTYPE_SD
		   || t1 == PF_MATRIXTYPE_DS
		   || t1 == PF_MATRIXTYPE_DD);
	PFASSERTDEBUG(t2 == PF_MATRIXTYPE_SS
		   || t2 == PF_MATRIXTYPE_SD
		   || t2 == PF_MATRIXTYPE_DS
		   || t2 == PF_MATRIXTYPE_DD);
	multFuncs[t1][t2](*this, t1, (const pfGenericMatrix &)_m1, t2, _m2);
    }
    void mult4g4d(const pfGenericMatrix & _m1, const pfMatrix4d & _m2)
    {
	pfMatrixType t1 = _m1.getMatrixType();
	pfMatrixType t2 = PF_MATRIXTYPE_DD;
	PFASSERTDEBUG(t1 == PF_MATRIXTYPE_SS
		   || t1 == PF_MATRIXTYPE_SD
		   || t1 == PF_MATRIXTYPE_DS
		   || t1 == PF_MATRIXTYPE_DD);
	PFASSERTDEBUG(t2 == PF_MATRIXTYPE_SS
		   || t2 == PF_MATRIXTYPE_SD
		   || t2 == PF_MATRIXTYPE_DS
		   || t2 == PF_MATRIXTYPE_DD);
	multFuncs[t1][t2](*this, t1, _m1, t2, (const pfGenericMatrix &)_m2);
    }

    void preMult(const pfGenericMatrix &_m)    { mult(_m, *this); }
    void postMult(const pfGenericMatrix &_m)   { mult(*this, _m); }
    void preMult4f(const pfMatrix &_m)         { mult4f4g(_m, *this); }
    void postMult4f(const pfMatrix &_m)        { mult4g4f(*this, _m); }
    void preMult4d(const pfMatrix4d &_m)       { mult4d4g(_m, *this); }
    void postMult4d(const pfMatrix4d &_m)      { mult4g4d(*this, _m); }

    //
    // Full suite of inverse functions...
    // XXX do we really want this? It's not really bloat since they
    // are all inlines...
    //
    /* inline */ void transpose(const pfGenericMatrix &_m);
    /* inline */ int invertFull(const pfGenericMatrix &_m);
    /* inline */ void invertAff(const pfGenericMatrix &_m);
    /* inline */ void invertOrtho(const pfGenericMatrix &_m);
    /* inline */ void invertOrthoN(const pfGenericMatrix &_m);
    /* inline */ void invertIdent(const pfGenericMatrix &_m);

    /* inline */ void transpose4f(const pfMatrix &_m);
    /* inline */ int invertFull4f(const pfMatrix &_m);
    /* inline */ void invertAff4f(const pfMatrix &_m);
    /* inline */ void invertOrtho4f(const pfMatrix &_m);
    /* inline */ void invertOrthoN4f(const pfMatrix &_m);
    /* inline */ void invertIdent4f(const pfMatrix &_m);

    /* inline */ void transpose4d(const pfMatrix4d &_m);
    /* inline */ int invertFull4d(const pfMatrix4d &_m);
    /* inline */ void invertAff4d(const pfMatrix4d &_m);
    /* inline */ void invertOrtho4d(const pfMatrix4d &_m);
    /* inline */ void invertOrthoN4d(const pfMatrix4d &_m);
    /* inline */ void invertIdent4d(const pfMatrix4d &_m);


};

#define PFGENERICMATSTACK ((pfGenericMatStack*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFGENERICMATSTACKBUFFER ((pfGenericMatStack*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfGenericMatStack : public pfObject
{
    //CAPI:basename GMStack
    //CAPI:argname  gmst
public:
    // Constructors, destructors
    pfGenericMatStack(int _size);

    //CAPI:private
    pfGenericMatStack();
    virtual ~pfGenericMatStack();

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
    void	get(pfGenericMatrix& _m) const {
	_m.copy(*sp);
    }
    // Returning pointers to internal structure is
    // an atrocity, but it's really really needed
    // for performance when examining the top matrix
    // at every node in the traversal...
    pfGenericMatrix *getTopGenericMatrix() const {
	return sp;
    }

    // XXX this should never be called internally!
    // XXX comment it out for the internal build?
    void	get4f(pfMatrix& _m)   const { sp->get4f(_m); }
    void	get4d(pfMatrix4d& _m) const { sp->get4d(_m); }

    // XXX getTop() should not exist!
    // XXX but I don't want to have to change everything at once,
    // XXX so I'm supporting it for now...
    // XXX should at least deprecate it by giving it a funky name
    // XXX to flag the questionable usages...
    pfMatrix*	getTop() const {
#ifdef POST_2_2_COMPAT
	pfNotify(PFNFY_FATAL, PFNFY_USAGE,
		 "pfGenericMatStack::getTop() not implemented!\n");
#else /* 2_2_COMPAT */
	return old_sp_for_2_2_compatibility;
#endif /* 2_2_COMPAT */
    }

    int		getDepth() const {
	return depth;
    }
    
    //CAPI:private
    int		getStackSize(int _size) const {
        return (int)sizeof(pfGenericMatrix) * _size;
    }
    void	setStack(void *stack, int _size);

public:	
    // other functions
    //CAPI:verb

    void	reset();
    //CAPI:err FALSE
    int		push();
    //CAPI:err FALSE
    int		pop();

    void	load(const pfGenericMatrix& _m) {
	sp->copy(_m);
#ifndef POST_2_2_COMPAT
	_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
    }
    void	preMult(const pfGenericMatrix& _m) {
	sp->preMult(_m);
#ifndef POST_2_2_COMPAT
	_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
    }
    void	postMult(const pfGenericMatrix& _m) {
	sp->postMult(_m);
#ifndef POST_2_2_COMPAT
	_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
    }
    void	load4f(const pfMatrix& _m) {
	sp->copy(_m);
#ifndef POST_2_2_COMPAT
	_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
    }
    void	preMult4f(const pfMatrix& _m) {
	sp->preMult4f(_m);
#ifndef POST_2_2_COMPAT
	_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
    }
    void	postMult4f(const pfMatrix& _m) {
	sp->postMult4f(_m);
#ifndef POST_2_2_COMPAT
	_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
    }
    void	load4d(const pfMatrix4d& _m) {
	sp->copy(_m);
#ifndef POST_2_2_COMPAT
	_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
    }
    void	preMult4d(const pfMatrix4d& _m) {
	sp->preMult4d(_m);
#ifndef POST_2_2_COMPAT
	_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
    }
    void	postMult4d(const pfMatrix4d& _m) {
	sp->postMult4d(_m);
#ifndef POST_2_2_COMPAT
	_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
    }

    // XXX these should really be pfGenericMatrix functions...
    // XXX This is kind of big and messy... should
    // XXX do something analogous to the mult() functions?
    void	preTransf(float _x, float _y, float _z) {
	// The translation is implicitly SS.
	switch(sp->getMatrixType())
	{
	    case PF_MATRIXTYPE_DS:
		sp->copy(sp->d(), PF_MATRIXTYPE_SS); // demote matrix to float
#ifndef POST_2_2_COMPAT
		_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
		// fall through
	    case PF_MATRIXTYPE_SS:
		// Result type is SS (same as before).
		sp->s().preTrans(_x, _y, _z, sp->s());
		break;

	    case PF_MATRIXTYPE_DD:
		sp->matrixType = PF_MATRIXTYPE_SD;
		// Fall through...
	    case PF_MATRIXTYPE_SD:
		// Result type is SD (same as before).
		sp->d().preTrans((double)_x, (double)_y, (double)_z, sp->d());
#ifndef POST_2_2_COMPAT
		_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
		break;
	    default:
		PFASSERTDEBUG(0); // XXX make this a better assert that matches the ones I'm going to put in all the mults to keep from bogusly indexing
	}
    }
    void	preTransd(double _x, double _y, double _z) {
	// The translation is implicitly DD.
	switch(sp->getMatrixType())
	{
	    case PF_MATRIXTYPE_SS:
                #ifdef __linux__
		sp->copy((const pfMatrix&)(*(pfMatrix*)sp->u._s), 
                    PF_MATRIXTYPE_DS); // promote matrix to DS
                #else
		sp->copy((const pfMatrix&)sp->u._s, 
                    PF_MATRIXTYPE_DS); // promote matrix to DS
                #endif
		break;
	    case PF_MATRIXTYPE_SD:
		sp->matrixType = PF_MATRIXTYPE_DD;
		break;
	    case PF_MATRIXTYPE_DS:
	    case PF_MATRIXTYPE_DD:
		// Result matrix type is same as before-- do nothing here.
		break;
	    default:
		PFASSERTDEBUG(0); // XXX make this a better assert that matches the ones I'm going to put in all the mults to keep from bogusly indexing
	}
	// In all cases, the translation gets done in double precision.
	sp->d().preTrans(_x, _y, _z, sp->d());
#ifndef POST_2_2_COMPAT
	_fix_2_2_compatibility_sp();
#endif /* 2_2_COMPAT */
    }


#ifdef NOTYET // XXX do we really need these? if so, need both SP and DP
    void	preTrans(float _x, float _y, float _z) {
	sp->preTrans(_x, _y, _z, *sp);
    }

    void	postTrans(float _x, float _y, float _z) {
	sp->postTrans(*sp, _x, _y, _z);
    }

    void	preRot(float _degrees, float _x, float _y, float _z) {
	sp->preRot(_degrees, _x, _y, _z, *sp);
    }
    
    void	postRot(float _degrees, float _x, float _y, float _z) {
	sp->postRot(*sp, _degrees, _x, _y, _z);
    }

    void	preScale(float _xs, float _ys, float _zs) {
	sp->preScale(_xs, _ys, _zs, *sp);
    }
    void	postScale(float _xs, float _ys, float _zs) {
	sp->postScale(*sp, _xs, _ys, _zs);
    }
#endif // NOTYET

    // Expose precision of top-of-stack to the world. Needed in here:
    // pfSCS Cull processing when matrix is identity can be skipped as long as
    // the culler matrix is single-precision. (Yair).

    int	    isSinglePrecision()
    {
	if (sp->getMatrixType() == PF_MATRIXTYPE_SS)
	    return (1);
	else
	    return (0);
    }

private:
    pfGenericMatrix    *base;	// Matrix stack 

#ifdef DEFUNCT // These are the two pointers that were in 2.2
    pfGenericMatrix    *end;	// End of stack-- UNUSED in 2.2
    pfGenericMatrix    *sp;	// Stack pointer 
#endif

    pfGenericMatrix *sp;	// Stack pointer
#ifndef POST_2_2_COMPAT
    pfMatrix *old_sp_for_2_2_compatibility;
		// Same position as pfMatStack::sp.
		// Needs to ALWAYS point at a valid single-precision
		// version of the top, so that the 2.2 inlined
		// pfTraverser::getMat() will work.
		// pfGenericMatStack needs to be the same size
		// as pfMatStack too.
#endif /* 2_2_COMPAT */

    int	   	depth;	// redundant info for quick tests 
    int	   	stackSize; // size of stack 

#ifndef POST_2_2_COMPAT
    void _fix_2_2_compatibility_sp()
    {
	// for the benefit of 2.2 pfTraverser::getMatrix()
	// which assumes the single-precision matrix is always available...
	if (sp != NULL) // if it's NULL, we're in the void* constructor
	{
	    if (sp->getMatrixType() == PF_MATRIXTYPE_SS)
		old_sp_for_2_2_compatibility = &sp->s();
	    else
	    {
#ifdef DOUBLE_DEBUG
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "XXX HAVE TO FIND A PRIVATE PLACE FOR old_sp_for_2_2_compatibility!!!");
#endif /* DOUBLE_DEBUG */
		old_sp_for_2_2_compatibility = &XXXtempMat; // XXX the private place
		PFCOPY_MAT(*old_sp_for_2_2_compatibility, sp->d());
	    }
	}
    }
    static pfMatrix XXXtempMat; // XXX COMPLETELY WRONG!
#endif /* 2_2_COMPAT */

private:
    static pfType *classType;
};


#define PF_MULT_MATRIXTYPE(AB,CD) ((pfMatrixType)(((AB)&2)|((CD)&1)))   /* AD */
#define PF_INVERT_MATRIXTYPE(AB) ((pfMatrixType)((((AB)&2)>>1)|(((AB)&1)<<1))) /* BA */

#define DEFINE_INVERTFUNC(TYPE, INVERTFUNCNAME, RETURN) \
    inline TYPE pfGenericMatrix::INVERTFUNCNAME(const pfGenericMatrix&  _m) \
    { \
	matrixType = PF_INVERT_MATRIXTYPE(_m.matrixType); \
	if (_m.matrixType == PF_MATRIXTYPE_SS) \
	    RETURN s().INVERTFUNCNAME((pfMatrix &)_m.s()); \
	else \
	    RETURN d().INVERTFUNCNAME((pfMatrix4d &)_m.d()); \
	/* XXX casting away const because param types of invertFull, invertOrthoN, transpose are wrong */ \
    } \
    inline TYPE pfGenericMatrix:: INVERTFUNCNAME##4f(const pfMatrix& _m) \
    { \
	matrixType = PF_MATRIXTYPE_SS; \
	RETURN s().INVERTFUNCNAME((pfMatrix &)_m); \
    } \
    inline TYPE pfGenericMatrix:: INVERTFUNCNAME##4d(const pfMatrix4d& _m) \
    { \
	matrixType = PF_MATRIXTYPE_DD; \
	RETURN d().INVERTFUNCNAME((pfMatrix4d &)_m); \
    }


#pragma warning(push)
#pragma warning(disable: 4003)

DEFINE_INVERTFUNC(int, invertFull, return)
DEFINE_INVERTFUNC(void, invertAff,)
DEFINE_INVERTFUNC(void, invertOrtho,)
DEFINE_INVERTFUNC(void, invertOrthoN,)
DEFINE_INVERTFUNC(void, invertIdent,)
DEFINE_INVERTFUNC(void, transpose,)

#pragma warning(pop)

#endif // __PF_GENERIC_MATRIX_H__
