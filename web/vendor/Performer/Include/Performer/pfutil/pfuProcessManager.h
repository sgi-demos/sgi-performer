/*
 * Copyright 1997, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that (i) the above copyright notices and this
 * permission notice appear in all copies of the software and related
 * documentation, and (ii) the name of Silicon Graphics may not be
 * used in any advertising or publicity relating to the software
 * without the specific, prior written permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL,
 * INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY
 * THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * file: pfuProcessManager.h
 * -------------------------
 */

#ifndef _PFU_PROCESS_MANAGER_H_
#define _PFU_PROCESS_MANAGER_H_

#if defined(__linux__has_pda_h__) || defined(mips) || defined(__win32__has_pda_h__)
#include <sys/pda.h> 
#endif 

#include <Performer/pf.h>
#include <Performer/pfutil.h>


#ifdef HAVE_POSIX
/* This declaration is deprecated. It shouldn't be used */
#define PFUPROC_APP_PRIORITY_POSIX 120
/* 

In 6.5, priorities will be assigned a little differently. It's not a
good idea to set an app priority and schedule relative to that, but
rather, it's better to set a maximum priority for any process to use,
and figure out the best app priority to make the maximum priority
process not exceed the maximum priority limit. Here's how 6.5 priority
ranges look:
  
  priorty         description
  -----------------------------------------------------------
  255             Reserved c-a thread
  240-254         Hard real time
  200-239         Interrupt threads
  110-199         Interactive real time
  90-109          System daemons
  0-89            Soft real time
  (-1)            Batch
  (-3)-(-2)       Timeshare
  (-4)            Batch
  (-5)            Weightless

  Scheduling performer processes around priority 120 puts it above the system
  daemons in 6.5, while it didn't in 6.4. In 6.5 the maximum priority performer
  should use is 89, the top of the soft realtime band. Here, we set the cap
  at 80, so there is room to run more important processes without running into
  the daemon range. */

/*
	Under Linux, the soft real time priorities range from 1-99
	(see: man sched_setscheduler).  Since 80 is within that range,
	we'll leave it at that for now, and change later if problems
	occur.
*/
/* The maximum priority which any performer process is allowed to use */
#define PFUPROC_MAXIMUM_POSIX_PRIORITY 80

#endif
#define PFUPROC_APP_PRIORITY_COMPAT (NDPHIMAX+2)

/*
// External C api...
*/
#include <Performer/pfutil-DLL.h>

#ifdef __cplusplus
extern "C" {
#endif
extern PFUDLLEXPORT void pfuInitProcessManagerClasses(void);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <Performer/pf/pfPipe.h>
#include <Performer/pr/pfList.h>

#define _PM_LOCK_DOWN  0x1
#define _PM_PRIORITIES 0x2
#define _PM_CONTINUOUS 0x4


//---------------------------------------------------
/*
// pfuBaseProcessManager
//  PURPOSE:
//	This is the base class that all pf*ProcessManagers
//  are to inherit from.  It defines members that all
//  processManagers will most likely need to function.
//  For example the functions to lock and unlock CPUs
//---------------------------------------------------
*/
class PFUDLLEXPORT pfuBaseProcessManager : public pfObject
{	
    public:
	pfuBaseProcessManager();
	virtual ~pfuBaseProcessManager() 
		{;}
    
	virtual int reconfigure() = 0;
	virtual void release();
	static void init();
	static pfType* getClassType() { return classType; }
	virtual const char* getTypeName(void) const { return "pfuBaseProcessManager"; }
	
	void setMode(int _sel, int _val);
	int getMode(int _sel);
	void setCreateFunc(pfuProcessHandlerFuncType func, uint mask) {
		uCreateCB = func; uCreateMask = mask;
	}
	void getCreateFunc(pfuProcessHandlerFuncType *func, uint *mask) const {
		*func = uCreateCB; *mask = uCreateMask;
	}
	void setPlaceFunc(pfuProcessHandlerFuncType func, uint mask) {
		uPlaceCB = func; uPlaceMask = mask;
	}
	void getPlaceFunc(pfuProcessHandlerFuncType *func, uint *mask) const {
		*func = uPlaceCB; *mask = uPlaceMask;
	}
	void setPrioritizeFunc(pfuProcessHandlerFuncType func, uint mask) {
		uPrioritizeCB = func; uPriMask = mask;
	}
	void getPrioritizeFunc(pfuProcessHandlerFuncType *func, uint *mask) const {
		*func = uPrioritizeCB; *mask = uPriMask;
	}
    private:
	friend void pfuInitProcessManagerClasses();
	static pfType *classType;
    protected:	// Utility functions
	int runProcOn(pid_t pid, int cpu);
	int isolateCPU(int cpu);
	int freeAllCPUs(void);

    public:
	void dontScheduleCPU(int cpu)	// Erase the cpu from the availableCPU list
	    { 
		availableCPUList->remove((void*)cpu); 
		numAvailableCPUs = availableCPUList->getNum();
	    }
	
    public:	// Member function called by the callback
	virtual void addProc(int _pipe, uint _stage, pid_t _pid) = 0;


    public:	// pbulic data members
	pfuProcessHandlerFuncType uCreateCB; // call this sb for unknown or umask stages
	uint	uCreateMask;
	pfuProcessHandlerFuncType uPlaceCB; // call this sb for unknown or umask stages
	uint	uPlaceMask;
	pfuProcessHandlerFuncType uPrioritizeCB; // call this sb for unknown or umask stages
	uint	uPriMask;
			
    protected:	// --- Data Members --- //
	uint	mode;
	int	numCPUs;		// Number the system actually has
	int	numAvailableCPUs;	// Number we can use
	int	numCriticalProcs;
	int	numPipes;
	int	mpBitmask;	// Bitmask of the APP/CULL/DRAW/FORK..
	int	usePosix; 	// we need run-time posix check
	
	pfList	    *availableCPUList;	// The List of CPUs we can use - vector<int>
	#if defined(__linux__has_pda_h__) || defined(mips) || defined(__win32__has_pda_h__)
	pda_stat*   pdaStats;		// Array of pda_stat's
	#endif 
};

/*
//-------------------------------------------------------------
// pfuProcessManager
//  PURPOSE:
//	This class is actually just a set of global
//  static functions used to store the current
//  pfuBaseProcessManager derived manager.
//	It also has the actual callback funtion that
//  pfFork should call.
//
// XXX: Should this be a singleton instead???
//
// NOTE: I used a class instead of global variables and global
//	  functions for the simple reason of trying to keep the
//	  namespace clean.
//-------------------------------------------------------------
*/

class PFUDLLEXPORT pfuProcessManager
{
private:
    pfuProcessManager()	    // Not allowed to create object
	{ ; }
public:
    static void select(pfuBaseProcessManager* pm)
	{ currentManager = pm; }
    static pfuBaseProcessManager* getCur()
	{ return currentManager; }

    static int reconfigure()
	{
	    if(getCur())
		return getCur()->reconfigure();
	    else
	    {
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "pfuProcessManager::reconfigure: No manager set.");
		return 0;
	    }
	}
	
    static void callback(int _pipe, uint _stage, pid_t _pid)
	{ 
	    // pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "Entering pfuProcessManager::callback.");
	    
	    if(getCur())
		getCur()->addProc(_pipe, _stage, _pid);
	    else
		pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "pfuProcessManager::callback: No manager set.");
	}

    static void setMode(int m, int v) 
    { 
	if (currentManager)
	    currentManager->setMode(m, v);
	else
	     pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	     "pfuProcessManager::setMode() - no process manager has been created.");
    }
    static int getMode(int m)
    { 
	if (currentManager)
	    return currentManager->getMode(m);
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	     "pfuProcessManager::getMode() - no process manager has been created.");
	    return -1;
	}
    }
    static void setCreateFunc(pfuProcessHandlerFuncType func, uint mask) {
	if (currentManager)
	    currentManager->setCreateFunc(func, mask);
	else
	     pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	     "pfuProcessManager::setCreateFunc() - no process manager has been created.");
    }
    static void getCreateFunc(pfuProcessHandlerFuncType *func, uint *mask) {
	if (currentManager)
	    currentManager->getCreateFunc(func, mask);
	else
	     pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	     "pfuProcessManager::getCreateFunc() - no process manager has been created.");
    }
    static void setPlaceFunc(pfuProcessHandlerFuncType func, uint mask) {
	if (currentManager)
	    currentManager->setPlaceFunc(func, mask);
	else
	     pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	     "pfuProcessManager::setPlaceFunc() - no process manager has been created.");
    }
    static void getPlaceFunc(pfuProcessHandlerFuncType *func, uint *mask) {
	if (currentManager)
	    currentManager->getPlaceFunc(func, mask);
	else
	     pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	     "pfuProcessManager::getPlaceFunc() - no process manager has been created.");
    }
    static void setPrioritizeFunc(pfuProcessHandlerFuncType func, uint mask) {
	if (currentManager)
	    currentManager->setPrioritizeFunc(func, mask);
	else
	     pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	     "pfuProcessManager::setPrioritizeFunc() - no process manager has been created.");
    }
    static void getPrioritizeFunc(pfuProcessHandlerFuncType *func, uint *mask) {
	if (currentManager)
	    currentManager->getPrioritizeFunc(func, mask);
	else
	     pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	     "pfuProcessManager::getPrioritizeFunc() - no process manager has been created.");
    }
    static void release(void)
    {
	if (currentManager)
	    currentManager->release();
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	     "pfuProcessManager::release() - no process manager has been created.");
	}
    }
public:
    static pfuBaseProcessManager*   currentManager;    
};

// PURPOSE:
//	This struct/class is used to hold data about the actual
//  processes to be assigned to CPUs.
class PFUDLLEXPORT _pfuProc
{
public:
    pid_t   pid;	// Process pid
    uint    stage;	// Performer stage bitmask
    int	pipe;	// Performer pipe
    int	runOn;	// Has the proc been sysmp(RUN_ON..)'ed
    int	cpu;	
    int	isolate; // this guy's CPU should be isolated
    int doPri;  // set non-degrading priorities
    int	pri;	// target priority relative to APP's 
    
    _pfuProc(int _pipe, uint _stage, pid_t _pid, int _numPipes);
    void printDebug();
    
};

/*
//-------------------------------------------------------------
// pfuDefaultProcessManager
//  PURPOSE:
//	Implements a process manager with semantics similar
//  to the old lockcpu.c routines.
//
//-------------------------------------------------------------
*/
class PFUDLLEXPORT pfuDefaultProcessManager : public pfuBaseProcessManager
{
protected:
	

public:
    pfuDefaultProcessManager();
    virtual int reconfigure(void);
    virtual void release(void);
    virtual void placeProcesses(int placeAll = -1);
    void addProcToCPUList(int procIndex);
    int calcAppProcCPU(_pfuProc* _proc);
    int calcDrawProcCPU(_pfuProc* _proc);
    int calcCullProcCPU(_pfuProc* _proc);
    int calcLPointProcCPU(_pfuProc* _proc);
    int calcIsectProcCPU(_pfuProc* _proc);
    int calcComputeProcCPU(_pfuProc* _proc);
    int calcQueueProcCPU(_pfuProc* _proc);
    int calcDbaseProcCPU(_pfuProc* _proc);
    int calcSidekickProcCPU(_pfuProc* _proc);
    
protected:
	// ---- CPU assignment data members ----- //
    int criticalPathHasOwnCPU;// Does each critical path proc have its own CPU?
    int computeHasOwnCPU;   // Does each compute proc have its own CPU?
    int dbaseHasOwnCPU;   // Does each dbase proc have its own CPU?
    int queueServiceCPU;    // Service proc woud like its own.
    int pairCullDraw;	    // Do we pair cull & Draw
    int baseAppCPU;	    // The CPU for the app - start of critical path
    int baseComputeCPU;	    // The CPU for the compute processes
    int baseDBaseCPU;	    // The CPU for the dbase processes
    int baseLPointCPU;	    // The CPU for the lpoint processes
    int	numComputeProcs;
    int numCriticalCPUs;    // Num CPUs available for critical path

    int numSidekickProcs;   // Counter for CULL_SIDEKICK processes.
    int	sidekickHasOwnCPU;  // Can we afford to allocate each CULL_SIDEKICK 
			    // processes a separate CPU ? 
			    //
			    // Possible Values:
			    //       0    no
			    //     (-1)   All CULL_SIDEKICKS share a CPU.
			    //       1    Yes. 
    int baseSidekickCPU;    // Base CPU index for all CULL_SIDEKICK processes.
    int	sidekickID;         // Internal counter for CULL_SIDEKICK processes we
			    // encounter.
        
public:	// call back member function
    virtual void addProc(int _pipe, uint _stage, pid_t _pid);
    void dumpProcList(void);
    public:
	static void init();
	static pfType* getClassType() { return classType; }
	virtual const char* getTypeName(void) const { return "pfuDefaultProcessManager"; }
	
    private:
	friend void pfuInitProcessManagerClasses();
	static pfType *classType;
    
protected:	// --- Data Members --- //
    pfList	    *procList;	// The process list - vector<Proc>
    pfList          *CPUList;	// This list holds info about which CPU "has" which procs
				    // Holds a vector of CPU's.  Each CPU is a vector
				    // of indexes into the procList->
				    // vector<vector<int> >
};

#endif /* __cplusplus */

#endif /* _PFU_PROCESS_MANAGER_H_ */
