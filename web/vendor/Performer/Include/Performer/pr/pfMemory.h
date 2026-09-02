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
// pfMemory.h
//
// $Revision: 1.64 $
// $Date: 2002/03/20 04:06:06 $
//

#ifndef __PF_MEMORY_H__
#define __PF_MEMORY_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <sys/types.h>
#include <string.h>
#ifdef __sgi
#include <bstring.h>
#else
#include <string.h>
#endif
#include <Performer/pr.h>
#include <Performer/pr/pfType.h>

// THESE MUST BE KEPT UP TO DATE WITH THE ACTUAL SIZES USED BY pfGetMemory
// WORDS are in sizeof(int)
#ifndef N64
#define PFMEM_SIZE		16  // byte offset back from data 
#define PFCMEM_SIZE		8   // additional size of cmem fields
#define PFALIGN_MASK		0x7 // for testing possible pfMemory::data pointers
#define PFARENA_BITS		8   // no more than 256 arenas
#define PFREF_BITS		24  // no more than 6M references
#define PFFLUXED_BIT		0x80000000
#define PFSIZE_MASK		0x7fffffff
#else
#define PFMEM_SIZE	     	32  // byte offset back from data
#define PFCMEM_SIZE		24  // additional size of cmem fields
#define PFALIGN_MASK		0x7 // for testing possible pfMemory::data pointers
#define PFARENA_BITS		32
#define PFREF_BITS		32
#define PFFLUXED_BIT		0x8000000000000000LL
#define PFSIZE_MASK		0x7fffffffffffffffLL
#endif

// ANY CHANGE IN THE SIZE OF PFMEMORY MUST BE REFLECTED IN PFMEM_WORDS

// CAPI:basename Mem
class DLLEXPORT pfMemory
{
public:	
    // new and delete
    // CAPI:private
    // operator to catch bad usage
    void* operator 	new(size_t s);
    // operator for allocating nbytes from default arena
    void* operator 	new(size_t s, size_t nybtes);
    // operator for allocating nbytes from specific arena
    void* operator 	new(size_t s, size_t nbytes, void *arena);
    // operator to turn the data portion of a pfFluxMemory into a pfMemory
    void* operator	new(size_t s, size_t nbytes, pfFluxMemory *_fmem);
    void  operator 	delete(void *);

public:
    // Constructors, destructors
    // CAPI:newargs int nbytes, void *arena
    // CAPI:private
    pfMemory();
    virtual ~pfMemory();

public:
    // per class functions;
    static void		init();
    static pfType*	getClassType() { return classType; }
    static int 		getArenaBytesUsed();

public:
    // CAPI:basename
    // virtual functions
    virtual int 	compare(const pfMemory *_mem) const;
    virtual int 	copy(const pfMemory *_src);
    virtual int 	print(uint _travMode, uint _verbose, char *_prefix, FILE *file);

    virtual void*	getData() const { 
	return (void*) (((char *)this)+sizeof(pfMemory)); 
    }

    virtual int		isOfType(pfType *_type) {
	return type->isDerivedFrom(_type);
    }

    virtual int		isExactType(pfType *_type) {
	return (type == _type);
    }

    virtual pfMemory*	operator =(const pfMemory *_src) { 
	copy(_src); 
	return this; 
    }

public:
    // virtual functions
    // CAPI:private
    virtual int 	ref();
    virtual int 	unref();
    virtual int		unrefGetRef(); // changed in 2.2.2 from ushort
    virtual int 	unrefDelete();
    virtual int 	checkDelete();
    virtual void	destroy();

public:
    // set and get functions
    // CAPI:private
    pfType* 		getType() const { return type; }
    const char*		getTypeName() const { return getType()->getName(); }
    void* 	  	getArena() const;
    size_t   		getSize() const
    {
	return isFluxed()? (size & PFSIZE_MASK) : size;
    }
    virtual int  	getRef(); // changed in 2.2.2 from ushort

public: 
    // exposed functions
    // CAPI:private
    pfMemory*		realloc(size_t nbytes);
    void  		clear()	{ memset(getData(), 0, getSize()); }
    int			isFluxed() const
    {
	return((size & PFFLUXED_BIT) && type != pfMemory::classType);
    }

public:
    // static functions
    // CAPI:header #if !PF_CPLUSPLUS_API

    // malloc from specific arena
    static void*	malloc(size_t nbytes, void *arena);
    static void*	calloc(size_t numelem, size_t elsize, void *arena);
    static char*	strdup(const char *str, void *arena);

    // malloc from current shared arena
    //CAPI:private
    static void*	malloc(size_t nbytes);
    static void*	calloc(size_t numelem, size_t elsize);
    static char*	strdup(const char *str);

public:
    // static functions that accept a void* that could be
    // the data pointer ::getData() of a pfMemory.
    static void*	realloc(void *data, size_t nbytes);
    static size_t	getSize(void *data);
    static void*	getArena(void *data);
    static void 	free(void *data);

    static void*   	getData(const void *data);
    static pfMemory*   	getMemory(const void *data);
    static const char* 	getTypeName(const void *data);
    static pfType*     	getType(const void *data);
    static int		isOfType(const void *data, pfType *type);
    static int		isExactType(const void *data, pfType *type);

    static int		isFluxed(const void *data);

    static int         	ref(void* _mem);
    static int         	unref(void* _mem);
    static int      	getRef(const void* _mem);// changed in 2.2.2 from ushort
    
    static int         	compare(const void* _mem1, const void* _mem2);
    // XXX leave out _prefix arg for now, should go in pfTraverser-based API
    static int         	print(const void* _mem, uint _travMode, uint _verbose, FILE* file);

    // CAPI:prbase
    // CAPI:verb Delete
    static int         	checkDelete(void* _mem);
    static int      	unrefGetRef(void* _mem); // changed in 2.2.2 from ushort
    static int         	unrefDelete(void* _mem);
    static int         	copy(void* dst, const void* src);

    // CAPI:private
    void		pr_initHeader(int _size, void *_arena);

protected:
    // for use in derived class constructors
    void setType(pfType* _type) { type = _type; }

    // CAPI:header #endif /* !PF_CPLUSPLUS_API */
protected:					//   32bit     64bit
						//  #bytes    #bytes
    pfType*	    type;			//	4	8
    unsigned int    arenaIndex : PFARENA_BITS;	//	4	8
    signed int      refCount   : PFREF_BITS;
    size_t	    size;			//	4	8
    //		    vtable			//	4	8
    //						//-------------------
    //			TOTAL			//	16	32
    // Note ref counting functions are hard coded for the above setup--
    // if it changes, they need to change too

private:
    static pfType	*classType;
};

#ifndef __BUILDCAPI__

#if PF_CPLUSPLUS_API

#define pfMalloc	pfMemory::malloc
#define pfCalloc	pfMemory::calloc
#define pfRealloc	pfMemory::realloc
#define pfStrdup	pfMemory::strdup
#define pfFree		pfMemory::free

#define pfGetMemory 	pfMemory::getMemory
#define pfGetSize  	pfMemory::getSize
#define pfGetArena	pfMemory::getArena

#define pfGetType 	pfMemory::getType
#define pfGetTypeName 	pfMemory::getTypeName

#define pfIsOfType	pfMemory::isOfType

#define pfRef 		pfMemory::ref
#define pfUnref 	pfMemory::unref
#define pfGetRef 	pfMemory::getRef
#define pfUnrefGetRef   pfMemory::unrefGetRef

#define pfPrint 	pfMemory::print
#define pfCompare 	pfMemory::compare

#ifndef __PF_H__
#define prDelete 	pfMemory::checkDelete
#define prUnrefDelete 	pfMemory::unrefDelete
#define prCopy	 	pfMemory::copy
#endif /* !__PF_H__ */
#endif // PF_CPLUSPLUS_API

#endif // !__BUILDCAPI__

#endif // !__PF_MEMORY_H__
