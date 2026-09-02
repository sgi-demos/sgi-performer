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
// pfList.h - Growable arrays
//
// $Revision: 1.28 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_LIST_H__
#define __PF_LIST_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfMemory.h>

#define PFLIST ((pfList*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFLISTBUFFER ((pfList*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfList : public pfObject
{
public:
    // constructors and destructors
    pfList(int _eltSize, int _listLength);

    // CAPI:private
    // constructor with reasonable defaults
    pfList();

    // Special constructor for pfList which does not allocate array - used
    // when allocating fixed-size pfList on stack. Need a better way - jr.
    pfList(int _eltSize, int _listLength, void *);

    virtual ~pfList();

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
    int 	getEltSize() const { return eltSize; }
    void** getArray() const { return array; }

    void 	setArrayLen(int alen);
    int 	getArrayLen() const { return arrayLen; }

    // CAPI:basename
    void	setNum(int newNum);
    int		getNum() const { return num; }

    void	set(int index, void *elt);
    void*	get(int index) const { 
	return ((index<0||index>=arrayLen)?NULL:array[index]); 
    }

    void*	operator[](int index) const {
	return array[index];
    }

    // CAPI:private
    // !!! No checking for speed. pop() doesn't need it but push() probably 
    // !!! does. These are used by libpf traversal code.    
    void	push() {
	num++;
    }
    void	pop() {
	num--;
    }
    int		getArraySize(int length) const {
	return eltSize * length;
    }
    void	setFixedArray(void *_array, int length);

public:
    // CAPI:verb
    // CAPI:basename List
    void	reset();
    // CAPI:basename Lists
    void	combine(const pfList *a, const pfList *b);

    // CAPI:basename 
    void	add(void *elt);
    void	*rem(void); // opposite of add (like a pop)
    void	insert(int index, void *elt);
    int		search(void *elt) const;
    int		remove(void *elt);
    void	removeIndex(int index);
    int		move(int index, void *elt);
    int		fastRemove(void *elt);
    void	fastRemoveIndex(int index);
    int		replace(void *oldElt, void *newElt);


private:
    int           num;		// Number of elements in array 
    int           arrayLen;	// Length of array in elements
    short	  eltSize;	// Byte size of list element 
    short	  flags;
    void**        array;	// Array of void* 

private:
    static pfType *classType;
}; 

#ifndef __BUILDCAPI__
extern "C" {     // EXPORT to CAPI
/* ------------------ pfList/pfPath Macros --------------------- */
#if defined(__STDC__) || defined(__cplusplus)

#define pfGetListArrayLen(_list)	pfGetListArrayLen((pfList*)_list)
#define pfListArrayLen(_list, _len)	pfListArrayLen((pfList*)_list, _len)
#define pfGetListArray(_list)		pfGetListArray((pfList*)_list)
#define pfResetList(_list)		pfResetList((pfList*)_list)
#define pfCombineLists(_dst, _a, _b)	\
	pfCombineLists((pfList*)_dst, (pfList*)_a, (pfList*)_b)
#define pfNum(_list, _num)		pfNum((pfList*)_list, _num)
#define pfGetNum(_list)			pfGetNum((pfList*)_list)
#define pfAdd(_list, _elt)		pfAdd((pfList*)_list, _elt)
#define pfInsert(_list, _index, _elt)	pfInsert((pfList*)_list, _index, _elt)
#define pfSearch(_list, _elt)		pfSearch((pfList*)_list, _elt)
#define pfRemove(_list, _elt)		pfRemove((pfList*)_list, _elt)
#define pfReplace(_list, _old, _new)	pfReplace((pfList*)_list, _old, _new)
#define pfSet(_list, _index, _elt)	pfSet((pfList*)_list, _index, _elt)
#define pfGet(_list, _index)		pfGet((pfList*)_list, _index)

#endif /* defined(__STDC__) || defined(__cplusplus) */
}
#endif // !__BUILDCAPI__
#endif // !__PF_LIST_H__
