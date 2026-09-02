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
// pfLists.h
//
// $Revision: 1.95 $
// $Date: 2002/10/31 17:29:10 $
//
//
#ifndef __PF_LISTS_H__
#define __PF_LISTS_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <sys/types.h>
#ifdef __sgi
#include <sys/stream.h>
#else
//#include <iostream.h>
#endif  /* __linux__ */
#include <string.h>


#include <Performer/pr/pfStruct.h>
#include <Performer/pf.h> // PFNEW


class pfUpdatable;
class pfBuffer;
class pfPipe;
class pfPipeVideoChannel;
class pfPipeWindow;
class pfLightSource;
class pfChannel;
class pfScene;
class pfNode;
class pfGroup;
class pfDCS;
class pfGeode;
class pfSequence;
class pfMPClipTexture;


// Performer Internal list types

#define SLIST_EXTERNAL(Type, SList)				\
  protected:						\
							\
    int     end;	/* Next available index */	\
    int     asize;	/* Actual size of array */	\
    Type    *array;     /* Array of elements */		\
  public:						\
    int 	getNum() const	{return end;}		\
    Type&    operator[](int index) const {		\
	return array[index];				\
    }

#define SLIST_INTERNAL(a, b)

#define SLIST_DECLARE(Type, SList)			\
class DLLEXPORT SList : public pfStruct {		\
    SLIST_EXTERNAL(Type, SList)				\
    SLIST_INTERNAL(Type, SList)				\
}

#define ALIST_EXTERNAL(Type, AList)		    		\
  protected:						\
							\
    int     end;	/* Next available index */	\
    int     asize;	/* Actual size of array */	\
    Type    *array;     /* Array of elements */		\
 public:						\
    int 	getNum() const	{return end;}		\
    Type	get(int i) const {			\
	if(i >= asize || i < 0) return 0; else return array[i];} \
    Type    operator[](int index) const {		\
	return array[index];				\
    }

#define ALIST_INTERNAL(a, b)

#define ALIST_DECLARE(Type, AList)			\
    class DLLEXPORT AList : public pfStruct {		\
    ALIST_EXTERNAL(Type, AList)				\
    ALIST_INTERNAL(Type, AList)				\
}

ALIST_DECLARE(void*, _pfVoidList);	// Free-for-all list
ALIST_DECLARE(float, _pfFloatList);	
ALIST_DECLARE(int, _pfIntList);

ALIST_DECLARE(pfUpdatable*, _pfUpdatablePList);
ALIST_DECLARE(pfPipe*, _pfPipePList);

#define ULIST_EXTERNAL(Type, UList)		    		\
  protected:						\
							\
    int     end;	/* Next available index */	\
    int     asize;	/* Actual size of array */	\
    int    *array;      /* Array of id's */		\
 public:						\
    int 	getNum() const	{return end;}		\
    Type	getApp(int i) const;			\
    int 	search(const Type find) const;		\
    Type    operator[](int index) const;

#define ULIST_INTERNAL(a, b)

// NODELIST stores node id's and handles indirection through 
// _pfCurrentBuffer table and casting.
#define UPDATABLE_INDEX_LIST_DECLARE(Type, UList)	\
  class DLLEXPORT UList : public pfStruct {		\
    ULIST_EXTERNAL(Type, UList)				\
    ULIST_INTERNAL(Type, UList)				\
}

UPDATABLE_INDEX_LIST_DECLARE(pfUpdatable*, _pfUpdatableList);
UPDATABLE_INDEX_LIST_DECLARE(pfPipe*, _pfPipeList);
UPDATABLE_INDEX_LIST_DECLARE(pfPipeWindow*, _pfPipeWindowList);
UPDATABLE_INDEX_LIST_DECLARE(pfPipeVideoChannel*, _pfPipeVideoChannelList);
UPDATABLE_INDEX_LIST_DECLARE(pfChannel*, _pfChannelList);
UPDATABLE_INDEX_LIST_DECLARE(pfScene*, _pfSceneList);
UPDATABLE_INDEX_LIST_DECLARE(pfNode*, pfNodeList);
UPDATABLE_INDEX_LIST_DECLARE(pfGroup*, _pfGroupList);
UPDATABLE_INDEX_LIST_DECLARE(pfSequence*, _pfSequenceList);
UPDATABLE_INDEX_LIST_DECLARE(pfLightSource*, _pfLSourceList);
UPDATABLE_INDEX_LIST_DECLARE(pfMPClipTexture*, _pfMPClipTextureList);

///////////////////////////////////////////////////////////////////


struct _pfHashElt
{
    PFSTRUCT_DECLARE

    long   	key;
    void    	*elt;

};

SLIST_DECLARE(_pfHashElt, _pfHashBucket);

struct DLLEXPORT _pfHashTable : public pfStruct
{
    _pfHashBucket	*hashTable;  	// Table of buckets
    _pfHashBucket	**occupiedList;	// List of non-empty buckets
    int 		numOccupied;   	// Number of non-empty buckets
    int 		hashSize;    	// Size of hash table
    _pfVoidList		hashList;    	// Flat list of hash entries

};


///////////////////////////////////////////////////////////////////

/* A special class that contains a list of void pointers, used
   to store states in cull.
   Once a pointer is added it is never removed, only a special
   index points to the first available non-zero pointer. This
   Pointer is reset at the beginning of each cull.
*/
// ============== List Template  =================================

class DLLEXPORT _pfVoidListIndex : public pfStruct 
{
 protected:
    void **Elements;
    int  nofElements, nofAllocatedElements, firstAvailable;

 public:

    // ===================================================================
    void reset(void) {
	firstAvailable = 0;
    }

    // ===================================================================
    int isAvailable(void) {
	return firstAvailable < nofElements;
    }
  
    // ===================================================================
    void* get(void) { // error checking must be done outside
	return Elements[firstAvailable++];
    }

    // ===================================================================

};

///////////////////////////////////////////////////////////////////

// XXX what parts should be visible outside?
// ============== List Template  =================================

template < class T >
class _pfTemplateList
{
PFSTRUCT_DECLARE

public:

    // ===================================================================

    _pfTemplateList()
    {
        nofElements = 0;
        nofAllocatedElements = 0;
        Elements = NULL;
    }

    // ===================================================================

    _pfTemplateList(int n)
    {
        nofElements = 0;
        Elements = NULL;
        nofAllocatedElements = 0;
        setSize(n);
    }

    // ===================================================================

    ~_pfTemplateList()
    {
        if (Elements)
            pfMemory::free(Elements);
    }

    // ===================================================================

    int  add(T *n)
    {
        if (nofElements >= nofAllocatedElements)
	    grow(nofAllocatedElements);

        Elements[nofElements] = *n;
        nofElements ++;

        return (nofElements - 1);
    }

    // ===================================================================

    int  add()
    {
        if (nofElements >= nofAllocatedElements)
	    grow(nofAllocatedElements);

        nofElements ++;

        return (nofElements - 1);
    }

    // ===================================================================

    void  set(int i, T *n)
    {
        if (i >= nofAllocatedElements)
	    grow(i);
	
        Elements[i] = *n;
        if (i >= nofElements)
            nofElements = i + 1;
    }

    // ===================================================================

    void  set(int i, T n)
    {
        if (i >= nofAllocatedElements)
	    grow(i);

        Elements[i] = n;
        if (i >= nofElements)
            nofElements = i + 1;
    }

    // ===================================================================
    T&  operator[](int n) const{
	return (Elements[n]);
    }

    // ===================================================================
    T   get(int n) { 
	if(n < 0 || n>=nofAllocatedElements) return 0;
	else
	    return (Elements[n]); 
    }

    // ===================================================================

    void zero()
    { 
        nofElements = 0; 
    }

    // ===================================================================

    int  getNum()
    { 
        return (nofElements); 
    }

    // ===================================================================

    void  setNum(int num)
    { 
	if(num>=0) {
	    setSize(num);
	    nofElements = num;
	}
    }

    // ===================================================================

    void setSize(int num)
    {
	if(num > nofAllocatedElements)
	    grow(num);
    }

    // ===================================================================

    void copy (_pfTemplateList *src)
    {
        int         i;

        zero();
        for (i = 0 ; i < src -> getNum() ; i ++)
            add (src -> get(i));
    }

    // ===================================================================


private:
    void grow(int n)
    {
        int newNumber = (n + 1) * 5 / 3;
	if(newNumber < nofAllocatedElements)
	    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, 
		     "List grown out of bounds\n");
	if(!Elements)
	    Elements = (T *)pfMemory::malloc(newNumber * sizeof(T),
					     _pfSharedArena);
	else
	    Elements = (T *)pfMemory::realloc(Elements, newNumber * sizeof(T));
	nofAllocatedElements = newNumber;
    }
 
    T    *Elements;
    int  nofElements, nofAllocatedElements;
};

//////////////////////////////////////////////////////////////////////////
// A version of the class above, you should use when T is not a pointer
// or a single value. The only difference is that the set and get functions
// operate with pointers

template < class T >
class _pfTemplateList2
{
PFSTRUCT_DECLARE

public:

    // ===================================================================

    _pfTemplateList2()
    {
        nofElements = 0;
        nofAllocatedElements = 0;
        Elements = NULL;
    }

    // ===================================================================

    _pfTemplateList2(int n)
    {
        nofElements = 0;
        Elements = NULL;
        nofAllocatedElements = 0;
        setSize(n);
    }

    // ===================================================================

    ~_pfTemplateList2()
    {
        if (Elements)
            pfMemory::free(Elements);
    }

    // ===================================================================

    int  add(T *n)
    {
        if (nofElements >= nofAllocatedElements)
	    grow(nofAllocatedElements);

        Elements[nofElements] = *n;
        nofElements ++;

        return (nofElements - 1);
    }

    // ===================================================================

    int  add()
    {
        if (nofElements >= nofAllocatedElements)
	    grow(nofAllocatedElements);

        nofElements ++;

        return (nofElements - 1);
    }

    // ===================================================================

    void  set(int i, T *n)
    {
        if (i >= nofAllocatedElements)
	    grow(i);
	
        Elements[i] = *n;
        if (i >= nofElements)
            nofElements = i + 1;
    }

    // ===================================================================
    T&  operator[](int n) const{
	return (Elements[n]);
    }

    // ===================================================================
    T  *get(int n) { 
	if(n < 0 || n>=nofAllocatedElements) return NULL;
	else
	    return (&Elements[n]);
    }

    // ===================================================================

    void zero()
    { 
        nofElements = 0; 
    }

    // ===================================================================

    int  getNum()
    { 
        return (nofElements); 
    }

    // ===================================================================

    void  setNum(int num)
    { 
	if(num>=0) {
	    setSize(num);
	    nofElements = num;
	}
    }

    // ===================================================================

    void setSize(int num)
    {
	if(num > nofAllocatedElements)
	    grow(num);
    }

    // ===================================================================

    void copy (_pfTemplateList2 *src)
    {
        int         i;

        zero();
        for (i = 0 ; i < src -> getNum() ; i ++)
            add (src -> get(i));
    }

    // ===================================================================


private:
    void grow(int n)
    {
        int newNumber = (n + 1) * 5 / 3;
	if(newNumber < nofAllocatedElements)
	    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, 
		     "List grown out of bounds\n");
	if(!Elements)
	    Elements = (T *)pfMemory::malloc(newNumber * sizeof(T),
					     _pfSharedArena);
	else
	    Elements = (T *)pfMemory::realloc(Elements, newNumber * sizeof(T));
	nofAllocatedElements = newNumber;
    }
 
    T    *Elements;
    int  nofElements, nofAllocatedElements;
};

#endif // !__PF_LISTS_H__
