#ifndef __PF_BYTEBANK_H__
#define __PF_BYTEBANK_H__

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

typedef struct
{
    int		blockSize;

    int		numFreeBlocks;
    int		numAllocatedFreeBlocks;
    void	**freeBlocks;

    int		numUsedBlocks;
    int		numAllocatedUsedBlocks;
    void	**usedBlocks;

} pfByteAccount;

#define PFBYTEBANK ((pfByteBank*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFBYTEBANKBUFFER ((pfByteBank*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfByteBank : public pfObject {
public:

    pfByteBank();
    ~pfByteBank();

    static void 	init();
    static pfType* 	getClassType() {return classType;};

    void		*withdraw(int size);
    void 		reset();

protected:

    pfByteAccount	*accounts;
    int			numAccounts;

private:
    static pfType 	*classType;
};


#endif //__PF_BYTEBANK_H__






