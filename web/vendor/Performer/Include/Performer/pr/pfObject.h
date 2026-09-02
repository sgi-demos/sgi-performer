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
// pfObject.h
//
// $Revision: 1.34 $
// $Date: 2002/03/14 21:11:10 $
//

#ifndef __PF_OBJECT_H__
#define __PF_OBJECT_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfMemory.h>

extern "C" {     // EXPORT to CAPI
/* ----------------------- pfObject Tokens ----------------------- */

/* 
 * pfPrint() 
 * verbosity levels for printing -- XXX should we just use notify tokens ??? 
 */
#define PFPRINT_VB_OFF	    0	/* no printing */
#define PFPRINT_VB_ON	    1	/* default - minimal printing */
#define PFPRINT_VB_NOTICE   1	/* default - minimal printing */
#define PFPRINT_VB_INFO     2	/* basic info */
#define PFPRINT_VB_DEBUG    3	/* prints everything */

} // extern "C" END of C include export




// CAPI:basename
// CAPI:argname obj
#define PFOBJECT ((pfObject*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFOBJECTBUFFER ((pfObject*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfObject : public pfMemory
{
public:
    // new and delete
    // CAPI:private
    void* operator new(size_t s);
    void* operator new(size_t s, void *arena);
    // operator to turn the data portion of a pfFluxMemory into a pfObject
    void* operator new(size_t s, pfFluxMemory *_fmem);
    void  operator 	delete(void *p) {pfMemory::operator delete(p);}

public:
    // Constructors, destructors
    // CAPI:private
    pfObject();
    virtual ~pfObject();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // CAPI:private
    // virtual functions
    virtual int 	compare(const pfMemory *_obj) const;
    virtual int 	copy(const pfMemory *_src);
    virtual int 	print(uint _travMode, uint _verbose, char *prefix, FILE *file);

    // CAPI:public
    virtual int 	getGLHandle() const { return -1; }
    virtual void	destroy();
    virtual void 	deleteGLHandle() {}

public:
    // virtual functions
    // CAPI:noverb
    // CAPI:private
    virtual void  	setUserData(void *data)	{ setUserData(0, data); }
    virtual void  	setUserData(int _slot, void *data);
    virtual void* 	getUserData() const	{ return getUserData(0); }
    virtual void* 	getUserData(int _slot) const;
    virtual int 	getNumUserData() const;

public:
    // static
    // CAPI:basename
    static void		    setCopyFunc(pfCopyFuncType _func)
			    { setCopyFunc(0, _func); }
    static pfCopyFuncType   getCopyFunc()
			    { return getCopyFunc(0); }
    static void		    setDeleteFunc(pfDeleteFuncType _func)
			    { setDeleteFunc(0, _func); }
    static pfMergeFuncType  getMergeFunc()
			    { return getMergeFunc(0); }
    static void		    setMergeFunc(pfMergeFuncType _func)
			    { setMergeFunc(0, _func); }
    static pfDeleteFuncType getDeleteFunc()
			    { return getDeleteFunc(0); }
    static void		    setPrintFunc(pfPrintFuncType _func)
			    { setPrintFunc(0, _func); }
    static pfPrintFuncType  getPrintFunc()
			    { return getPrintFunc(0); }
    // CAPI:verb CopyFuncSlot
    static void		    setCopyFunc(int _slot, pfCopyFuncType _func);
    // CAPI:verb GetCopyFuncSlot
    static pfCopyFuncType   getCopyFunc(int _slot);
    // CAPI:verb DeleteFuncSlot
    static void		    setDeleteFunc(int _slot, pfDeleteFuncType _func);
    // CAPI:verb GetDeleteFuncSlot
    static pfDeleteFuncType getDeleteFunc(int _slot);
    // CAPI:verb MergeFuncSlot
    static void		    setMergeFunc(int _slot, pfMergeFuncType _func);
    // CAPI:verb GetMergeFuncSlot
    static pfMergeFuncType  getMergeFunc(int _slot);
    // CAPI:verb PrintFuncSlot
    static void		    setPrintFunc(int _slot, pfPrintFuncType _func);
    // CAPI:verb GetPrintFuncSlot
    static pfPrintFuncType  getPrintFunc(int _slot);
    // CAPI:noverb

    static int	       	getGLHandle(const pfObject *_obj);
    static void	       	deleteGLHandle(pfObject *_obj);
    static void		setUserData(pfObject* _obj, void* data)
			{ setUserData(_obj, 0, data); }
    static void*       	getUserData(pfObject* _obj)
			{ return getUserData(_obj, 0); }
    // CAPI:verb UserDataSlot
    static void		setUserData(pfObject* _obj, int _slot, void* data);
    // CAPI:verb GetUserDataSlot
    static void*       	getUserData(pfObject* _obj, int _slot);
    // CAPI:verb GetNumUserData
    static int	 	getNumUserData(pfObject* _obj);
    // CAPI:noverb
    static int		getNamedUserDataSlot(const char *_name);
    static const char*  getUserDataSlotName(int _slot);
    static int		getNumNamedUserDataSlots();

public:
    // getSize not supported on pfObjects
    // CAPI:private
    size_t   		getSize() const { return 0; }


protected:
    // Derived classes call these to copy/compare their inherited members
    virtual int		parentCopy(const pfMemory *src, int doCompare);
    virtual int		parentCompare(const pfMemory *mem) const;

protected:
    pfList	*userDataList;


private:
    static pfType *classType;
};


#endif // !__PF_OBJECT_H__
