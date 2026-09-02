#ifndef __PF_GSETBANK_H__
#define __PF_GSETBANK_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

/*
  The pfObject header file is pulled from a different place when building the 
  library than when compiling against it. 
  */
#include <Performer/pr/pfObject.h>

// Export to C API.

extern "C" {
}

class DLLEXPORT pfGeoSet;

#define PFGSETBANK ((pfGSetBank*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFGSETBANKBUFFER ((pfGSetBank*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfGSetBank : public pfObject {
public:

    pfGSetBank();
    ~pfGSetBank();

    static void 	init();
    static pfType* 	getClassType() {return classType;};

    pfGeoSet		*withdraw(void);
    void 		reset();

protected:

    int			numFreeGSets;
    int			numAllocatedFreeGSets;
    pfGeoSet		**freeGSets;

    int			numUsedGSets;
    int			numAllocatedUsedGSets;
    pfGeoSet		**usedGSets;

private:
    static pfType 	*classType;

};


#endif //__PF_GSETBANK_H__






