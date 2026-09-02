#ifndef __PF_UPDATE_QUEUE_H__
#define __PF_UPDATE_QUEUE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pf.h>  // _pfCurrentBuffer
#include <Performer/pr/pfObject.h>


class pfBuffer;
class _pfUpdateList;

typedef struct _pfUpdateChain
{

    _pfUpdateList		*list;
    struct      _pfUpdateChain  *next;
    struct      _pfUpdateChain  *prev;
} pfUpdateChain;


#define PFUPDATEQUEUE ((pfUpdateQueue*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFUPDATEQUEUEBUFFER ((pfUpdateQueue*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfUpdateQueue : public pfObject
{
public:
    //CAPI:updatequeue

    pfUpdateQueue(int _numClients);
    virtual ~pfUpdateQueue();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }
    
public: // virtual functions

    virtual pfUpdateChain	*getFirstUpdate();
    virtual pfUpdateChain	*getTail();
    virtual void		endUpdate();
    virtual void 		startUpdate();
    virtual void 		cleanUpdates();
    virtual _pfUpdateList	*getCurrentUpdateList();
    virtual void		executeUpdates(int clientID, pfBuffer *buf0, pfBuffer *buf1);
    virtual void		skipClientUpdates(int clientID);

    virtual int			getNumClients();
    virtual void		printUpdates();

protected:

    // ====================================================================
    // Pointers to queue endpoints.
    // ====================================================================

    pfUpdateChain   *head, *tail;

    // ====================================================================
    // List of free queue atoms.
    // ====================================================================

    pfUpdateChain   *ready;

    // ====================================================================
    // Atom containint currently open update list. This list is not on 
    // the queue. It is put on the queue when calling endUpdate (also 
    // forced by startUpdate).
    // ====================================================================

    pfUpdateChain   *currentUpdate;

    // ====================================================================
    // How many buffers use this queue ? 
    // We will only delete a node from the bottom of the queue when all 
    // clients are done with it.
    // ====================================================================

    int		    numClients;
    pfUpdateChain   **clientPtr;
    int    	    *clientPID;

private:
    static pfType *classType;
};

#endif // !__PF_UPDATE_QUEUE_H__
