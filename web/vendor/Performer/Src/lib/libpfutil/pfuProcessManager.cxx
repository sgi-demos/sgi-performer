/*
 * Copyright 1997 Silicon Graphics, Inc.
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

//  file: pfuProcessManager.C
//
//  For now this file is only used to store the C interface
// and to include the actual class code.

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#ifndef WIN32
#include <sys/sysmp.h>
#endif
#if defined(__linux__has_pda_h__) || defined(mips) || defined(__win32__has_pda_h__)
#include <sys/pda.h> 
#endif  /* __linux__ */
#if defined(__linux__has_schedctl_h__) || defined(mips) || defined(__win32__has_schedctl_h__)
#include <sys/schedctl.h>
#endif  /* __linux__ */
#include <errno.h>
#ifdef HAVE_POSIX
#include <sched.h>
#include <dlfcn.h>
#endif

#include "pfuProcessManager.h"

pfType *pfuBaseProcessManager::classType = NULL;
pfType *pfuDefaultProcessManager::classType = NULL;

// Make compiler allocate static
pfuBaseProcessManager* pfuProcessManager::currentManager = NULL;	

pfuBaseProcessManager::pfuBaseProcessManager() 
{
    setType(classType); 
    numPipes = pfGetMultipipe();
    mpBitmask = pfGetMPBitmask();
    #if defined(__linux__has_sysmp_h__) || defined(mips) || defined(__win32__hash_sysmp_h__)
    numCPUs = sysmp(MP_NPROCS);
    #else
    numCPUs = 1;
    #endif  /* __linux__ */
    numCriticalProcs = 0;
    mode = 0;
    uCreateCB = uPlaceCB = uPrioritizeCB = NULL;
    uCreateMask = uPlaceMask = uPriMask = 0x0;

    // find out if we can have posix
    usePosix = 0;
#if defined(HAVE_POSIX) && defined(mips)
    void *handle;
    // everyone else (libGL.so) explicitly references libc.so.1
    // so we have to do it that way too for the rld refcounting to work.
    usePosix = (pfGetIRIXRelease() >= 6.4);
    if (!usePosix)
    {
	if (!(getenv("_PF_NO_POSIX_SCHED")))
	{
	    handle = dlopen("libc.so.1", RTLD_LAZY);
	    usePosix = (dlsym(handle, "sched_setscheduler") != 0);
	    dlclose(handle);
	}
    }
#else /* linux */
	usePosix=1;
#endif
    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"pfuBaseProcessManager - HAVE POSIX: %d", usePosix);

    availableCPUList = new pfList;
    for(int cpu=0;cpu<numCPUs;cpu++)	    // Create list of CPU numbers to use
	availableCPUList->add((void*)cpu);
	
#if defined(__linux__has_pda_h__) || defined(mips) || defined(__win32__has_pda_h__)
    pdaStats = (pda_stat*)pfMalloc(sizeof(pda_stat) * numCPUs);
    sysmp(MP_STAT, pdaStats);		    // Get the pda_stats for all CPUs
    
	// If a CPU is currently restricted, exclude it from consideration
    for(cpu=0; cpu < numCPUs; cpu++)
	if (!(pdaStats[cpu].p_flags & PDAF_ENABLED))
	{
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "CPU:%d is unavailable.  Removing it from consideration.", cpu);
	    dontScheduleCPU(cpu);
	}
/* 
#else
    pfNotify(PFNFY_WARN, PFNFY_PRINT,
	"pfuProcessManager not supported in Linux\n");
*/
#endif  /* __linux__ */

    numAvailableCPUs = availableCPUList->getNum();  
}

void 
pfuBaseProcessManager::setMode(int _sel, int _val) 
{ 
    switch(_sel)
    {
    case PFUPM_LOCK_DOWN:
	if (_val < 0) 
	    _val = 1;
	PFFLAG_BOOL_SET(mode, _PM_LOCK_DOWN, _val);
	break;
    case PFUPM_PRIORITIES:
	if (_val < 0) 
	    _val = 1;
	PFFLAG_BOOL_SET(mode, _PM_PRIORITIES, _val);
	break;
    case PFUPM_CONTINUOUS:
	if (_val < 0) 
	    _val = 1;
	PFFLAG_BOOL_SET(mode, _PM_CONTINUOUS, _val);
	break;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,"pfuDefaultProcessManager::setMode()"
	" no such mode %d", _sel);
    }
}

int 
pfuBaseProcessManager::getMode(int _sel)
{
    switch(_sel)
    {
    case PFUPM_LOCK_DOWN:
	return PFFLAG_BOOL_GET(mode, _PM_LOCK_DOWN);
    case PFUPM_PRIORITIES:
	return PFFLAG_BOOL_GET(mode, _PM_PRIORITIES);
    case PFUPM_CONTINUOUS:
	return PFFLAG_BOOL_GET(mode, _PM_CONTINUOUS);
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,"pfuDefaultProcessManager::getMode()"
	" no such mode %d", _sel);
	return -1;
    }
}

void 
pfuBaseProcessManager::release(void)
{
    freeAllCPUs();			
}
 

/*
//-------------------------------------------------------------
// pfuDefaultProcessManager
//  PURPOSE:
//	Implements a process manager with semantics similar
//  to the old lockcpu.c routines.
//
//-------------------------------------------------------------
*/

_pfuProc::_pfuProc(int _pipe, uint _stage, pid_t _pid, int _numPipes)
{ 
    pid = _pid;
    pipe = _pipe;
    stage = _stage;
    runOn = 0;
    isolate = 0;
    doPri = 0;
    pri = 0;
    cpu = -1;

    // higher numbers are higher relative priority to APP
    // order the culls and draws so that if there are collisions on
    // a CPU, draw has higher priority and processes of different pipes
    // don't fight but order themselves by pipe number.
    if (pfuProcessManager::currentManager->uPrioritizeCB &&
	(pfuProcessManager::currentManager->uPriMask & stage))
    {
	pri = pfuProcessManager::currentManager->uPrioritizeCB(pipe, stage, pid);
	if (pri != PFUPM_NOPRI)
	    doPri = 1;
    }
    else if (stage & PFPROC_APP)
    {
	pri = 0;
	doPri = 1;
    }
    else if (stage & PFPROC_CULL)
    {
	pri = 1 + pipe;
	doPri = 1;
    }
    else if (stage & PFPROC_DRAW)
    {
	pri = 1 + _numPipes + pipe;
	doPri = 1;
    }
    else if (stage & PFPROC_CULL_SIDEKICK)
    {
	pri = -1;
	doPri = 1;
    }
    else if (stage & PFPROC_ISECT)
    {
	pri = -2;
	doPri = 1;
    }
    else if (stage & PFPROC_DBASE)
    {
	pri = -3;
	doPri = 1;
    }
    else if (stage & PFPROC_COMPUTE)
    {
	pri = -4;
	doPri = 1;
    }
    else if (pfuProcessManager::currentManager->uPrioritizeCB &&
	(!(pfuProcessManager::currentManager->uPriMask & stage)))
    {
	pri = pfuProcessManager::currentManager->uPrioritizeCB(pipe, stage, pid);
	if (pri != PFUPM_NOPRI)
	    doPri = 1;
    }
}

void
_pfuProc::printDebug()
{ 
    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_MORE,  "pid:%d \tstage:0%x \tpipe:%d", pid, stage, pipe);
}

pfuDefaultProcessManager::pfuDefaultProcessManager(void)
{
    setType(classType); 
	// Create sub lists.  One per CPU.
    CPUList = new pfList;
    procList = new pfList;
    for (int cpu=0;cpu < numCPUs;cpu++)
    {
	pfList* newList = new pfList;
	CPUList->add(newList);
    } 
    mode = 0;
    criticalPathHasOwnCPU = 0;
    computeHasOwnCPU = 0;
    numCriticalCPUs = 0;
    numComputeProcs = 0;
    numSidekickProcs = 0;
    sidekickHasOwnCPU = computeHasOwnCPU = dbaseHasOwnCPU = 0;
    baseSidekickCPU = baseComputeCPU = baseDBaseCPU = baseLPointCPU = 0;
    queueServiceCPU = -1;
}

void
pfuDefaultProcessManager::release(void)
{
    freeAllCPUs();			

    // For each CPU unschedule its processes
    for(int cpu=0; cpu < numCPUs; cpu++)
    {
	pfList* list = (pfList*)(CPUList->get(cpu));
	for(int iter = 0; iter < list->getNum(); iter++)
	{
	    _pfuProc* proc = (_pfuProc*)procList->get((int)((long)list->get(iter)));

	    // unset non-degrading priority
#ifdef HAVE_POSIX
	    if (usePosix)
	    {
		struct sched_param schedAttrs;
		schedAttrs.sched_priority = 0;
		if (sched_setscheduler(proc->pid, SCHED_FIFO, &schedAttrs) < 0) 
		{
		    pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
			"pfuProcessManager::release()"
			"sched_setscheduler() failed to unschedule process stage=0x%x pid=%d ", 
			proc->stage, proc->pid);
		}
	    }
	    else // !!!!!
#endif
#if defined(__linux__has_schedctl_h__) || defined(mips)
	    if (schedctl(NDPRI, 0, 0) < 0)
		pfNotify(PFNFY_WARN, PFNFY_SYSERR,
			"pfuProcessManager::release() schedctl() failed to release "
			"process stage=%d pid=%d", proc->stage, proc->pid);

	    // allow process to run anywhere
	    if (sysmp(MP_RUNANYWHERE_PID, proc->pid) < 0)
		pfNotify(PFNFY_WARN, PFNFY_SYSERR,
		    "pfuProcessManager::release() sysmp(MP_RUNANYWHERE_PID, %d) failed.",
		    proc->pid);
#else
			;
#endif
	}
    }
}
    
// PURPOSE:
//	This function is called to reconfigure process allocation and prioritization
int 
pfuDefaultProcessManager::reconfigure(void)
{
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Reconfiguring Processes.");
    
    freeAllCPUs();			// ASSERT: We have a "clean" system
    
	// ASSERT: We have access to all CPUs 
    if (numAvailableCPUs == 1)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
	    "There is only one processor available.  Not locking any processes.");
	return 0;
    }
    
    placeProcesses(1);		// Put ALL processes on the CPU's they should go to
    
    return 1;
}

// PURPOSE:
//	This function takes the functions in the procList and places
//  Them on/into the cpuList.  
//  Processes are then placed on CPUs and given priorities as allowed by mode.
//  placeAll - If > 0 we should place all processes even if they have
//		     been placed before. (Have runOn flag set)
//	       If == 0 then place none and only do priorities
//	       if < 0, place dirty processes
void 
pfuDefaultProcessManager::placeProcesses(int placeAll)
{
    int num, prevNum, base;
    int lumpLPoint = 0;
    int cpu;
    
    if (!(mode & _PM_LOCK_DOWN))
	placeAll = 0;

    // Clear the CPUList since we are going to re-place processes
    for(cpu=0;cpu< numCPUs;cpu++)
    {
	pfList*  theList = (pfList*)(CPUList->get(cpu));
	while(theList->getNum() != 0)
	    theList->fastRemoveIndex(0);
    }
    
	// --- Intialize assignment data --- //
    criticalPathHasOwnCPU   = (numAvailableCPUs >= numCriticalProcs);  
    if (!criticalPathHasOwnCPU && (pfGetMPBitmask() & PFMP_FORK_LPOINT))
    {
	lumpLPoint = 1;
	numCriticalProcs -= numPipes;
	criticalPathHasOwnCPU = (numAvailableCPUs >= numCriticalProcs);  
    }
    else
	pairCullDraw = ((criticalPathHasOwnCPU) ? 0 : 
		(numAvailableCPUs >= (numCriticalProcs - numPipes)) ); 
    if (pairCullDraw)
    {
	numCriticalCPUs = (numCriticalProcs - numPipes);
	criticalPathHasOwnCPU   = (numAvailableCPUs >= numCriticalProcs);  
    }
    else
    {
	numCriticalCPUs = numCriticalProcs;
	criticalPathHasOwnCPU   = (numAvailableCPUs >= (numCriticalProcs - numPipes));  
    }
    
    baseAppCPU = calcAppProcCPU(NULL);	    // Get it here because everyone else needs to know it
    
    // put them after the critical path CPUs
    if (numCriticalCPUs > numAvailableCPUs)
	numCriticalCPUs = numAvailableCPUs;
    num = numAvailableCPUs - numCriticalCPUs;
    if (num < 0)
	num = 0;
	
    prevNum = 0;
    base = ((num > 0) ? numCriticalCPUs+1 : pfGetProcessMiscCPU());
    if (lumpLPoint)
    {
	baseLPointCPU = -base;
	prevNum = 1;
    }
    else
    {
	baseLPointCPU = base;
	prevNum = numPipes;
    }
    num = ((num >= prevNum) ? (num - prevNum) : 0);
    base = ((num > 0) ? base+prevNum : pfGetProcessMiscCPU());

    /////////////////////////////////////////////////////////////////
    // Place CULL_SIDEKICK processes
 
    if (numSidekickProcs > 0)
    {
	if (num >= numSidekickProcs) 
	{
	    sidekickHasOwnCPU = 1;
	    baseSidekickCPU = base;
	    prevNum = numSidekickProcs;
	}
	else if (num >= 1)
	{
	    sidekickHasOwnCPU = -1;
	    baseSidekickCPU = base;
	    prevNum = 1;
	}
	else
	{
	    sidekickHasOwnCPU = 0;
	    baseSidekickCPU = base;
	    prevNum = 0;
	}
	num = ((num >= prevNum) ? (num - prevNum) : 0);
	base = ((num > 0) ? base+prevNum : pfGetProcessMiscCPU());
    }
    
    /////////////////////////////////////////////////////////////////
    // figure out what we will do with the misc compute procs
    if (num >= numComputeProcs)
    {
	computeHasOwnCPU = 1;
	baseComputeCPU = base;
	prevNum = numComputeProcs;
    }
    else if (num >= 1)
    {
	computeHasOwnCPU = -1;
	baseComputeCPU = base;
	prevNum = 1;
    }
    else 
    {
	computeHasOwnCPU = 0;
	baseComputeCPU = base;
	prevNum = 0;
    }
    num = ((num >= prevNum) ? (num - prevNum) : 0);
    base = ((num > 0) ? base+prevNum : pfGetProcessMiscCPU());
    
    /////////////////////////////////////////////////////////////////
    // see if we have enough room to put DBase and Queue on their own CPU(s)
    if (num >= 3) // 1 for dbse, 1 for queue service and 1 for queue sort
    {
	prevNum = 3;
	dbaseHasOwnCPU = 1;
	queueServiceCPU = base+2;
	baseDBaseCPU = base;
    }
    else if (num >= 2) // 1 for dbse + sort, 1 for queue service
    {
	prevNum = 2;
	dbaseHasOwnCPU = 0;
	queueServiceCPU = base+1;
	baseDBaseCPU = base;
    }
    else 
    {
	prevNum = 1;
	dbaseHasOwnCPU = 0;
	queueServiceCPU = base;
	baseDBaseCPU = base;
    }
    num = ((num >= prevNum) ? (num - prevNum) : 0);
    
    // Initialize counter for CULL_SIDEKICK processes.
    sidekickID = 0;

    // Put All Processes to their correct CPU
    // if (placeAll)
    for(int procIndex = 0; procIndex < procList->getNum(); procIndex++)
	addProcToCPUList(procIndex);

    /*
      Since everything is scheduled relative to app, and we do not want the
      highest priority process to exceed PFUPROC_MAXIMUM_POSIX_PRIORITY,
      scheduling app at PFUPROC_MAXIMUM_POSIX_PRIORITY - maxPriorityOffset
      will ensure that the highest priority process will fit under 
      PFUPROC_MAXIMUM_POSIX_PRIORITY
      */
    int maxPriorityOffset = 0;
    
    for(cpu=0; cpu < numCPUs; cpu++) {
      pfList* list = (pfList*)(CPUList->get(cpu));
      for(int iter = 0; iter < list->getNum(); iter++) {
	_pfuProc* proc = 
	  (_pfuProc*)procList->get((int)((long)list->get(iter)));
	
	if(proc->pri > maxPriorityOffset) {
	  maxPriorityOffset = proc->pri;
	}
      }
    }
#ifdef HAVE_POSIX
    int appPriority = PFUPROC_MAXIMUM_POSIX_PRIORITY - maxPriorityOffset;
#endif

    // For each CPU handle it's processes
    for(cpu=0; cpu < numCPUs; cpu++)
    {
	pfList* list = (pfList*)(CPUList->get(cpu));
	for(int iter = 0; iter < list->getNum(); iter++)
	{
	    _pfuProc* proc = (_pfuProc*)procList->get((int)((long)list->get(iter)));

	    // set priority of the process relative to APP
	    if ((mode & _PM_PRIORITIES) && ((placeAll>0) || (proc->runOn == 0)))
	    {
		int pri=0;
#ifdef HAVE_POSIX
		if (usePosix)
		{
		    struct sched_param schedAttrs;
		    if (proc->doPri)
		        pri = appPriority + proc->pri;
		    
                    /*
		    This way no longer works, it assumes processes increase
		    in priority as the priority value decreases, and schedule
		    relative to a hardcoded app priority
		    //pri = PFUPROC_APP_PRIORITY_POSIX - proc->pri; */
		      
		    schedAttrs.sched_priority = pri;
		    if (sched_setscheduler(proc->pid, SCHED_FIFO, &schedAttrs) < 0) 
                    {
                        int tmperr = errno;
		        pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
			    "pfuProcessManager::placeProcesses() failed.");
                        if (tmperr == EPERM)
		            pfNotify(PFNFY_WARN, PFNFY_MORE,  
			        "    Root permission required for this operation." );
		        pfNotify(PFNFY_WARN, PFNFY_MORE,  
                            "    stage=0x%x pid=%d pri=%d errno=%d", 
			    proc->stage, proc->pid, pri, tmperr);
                    }
		    else
		        pfNotify(PFNFY_INFO, PFNFY_PRINT,
			    "Process %s (%d) given pri %d",
		            pfGetPIDName(proc->pid), proc->pid, pri);
	        }
		else // !!!!!!
#endif
		{ // old 6.2 style schedule control
		#if defined(__linux__has_schedctl_h__) || defined(mips)
		if (proc->doPri)
		    pri = PFUPROC_APP_PRIORITY_COMPAT + proc->pri;
		if (schedctl(NDPRI, proc->pid, 0) < 0)
		    pfNotify(PFNFY_WARN, PFNFY_SYSERR, "pfuProcessManager::placeProcesses() "
			" schedctl() failed for process stage=%d pid=%d, pri=%d",
			    proc->stage, proc->pid, pri);
		    else
			pfNotify(PFNFY_INFO, PFNFY_PRINT,
			    "Process %s (%d) given pri %d",
			    pfGetPIDName(proc->pid), proc->pid, pri);
		#endif  /* __linux__ */
		}
	    }

	    // If (Replace all procs OR proc hasn't been runon'ed)
	    if((mode & _PM_LOCK_DOWN) && ((placeAll>0) || (proc->runOn == 0)))
	    {
		if (cpu > -1)
		    runProcOn( proc->pid, cpu);
		if (proc->isolate && (cpu > 0))
		    isolateCPU(cpu);  
	    }
	    proc->runOn = 1;
	}
    }    
}
    
// PURPOSE:
//	This function takes a given proc and
//  adds it to the correct CPU in the CPUList
void 
pfuDefaultProcessManager::addProcToCPUList(int procIndex)
{
    int acpu = -1, cpu = -1, ocpu=-1;
    _pfuProc* proc = (_pfuProc*)procList->get(procIndex);
    uint stage = proc->stage;
    pfList *list;

    if (mpBitmask < 0)
	mpBitmask = pfGetMPBitmask();
    
    // see if user takes this one
    if (uPlaceCB && (uPlaceMask & stage))
	acpu = uPlaceCB(proc->pipe, stage, proc->pid);
    else if (stage & PFPROC_APP)
	acpu = calcAppProcCPU(proc);
    else if (stage & PFPROC_CULL)
	acpu = calcCullProcCPU(proc);
    else if (stage & PFPROC_DRAW)
	acpu = calcDrawProcCPU(proc);
    else if (stage & PFPROC_CULL_SIDEKICK)
	acpu = calcSidekickProcCPU(proc);
    else if (stage & PFPROC_ISECT)
	acpu = calcIsectProcCPU(proc);
    else if (stage & PFPROC_COMPUTE)
	acpu = calcComputeProcCPU(proc);
    else if (stage & PFPROC_LPOINT)
	acpu = calcLPointProcCPU(proc);
    else if (stage & (PFPROC_QUEUE_SORT | PFPROC_QUEUE_SERVICE))
	acpu = calcQueueProcCPU(proc);
    else if (uPlaceCB && (!(uPlaceMask & stage)))
	acpu = uPlaceCB(proc->pipe, stage, proc->pid);
	
    if (acpu > -1)
	cpu = (int)((long)availableCPUList->get(acpu));	// Convert "virtual" cpu number to real CPU number
    else
	cpu = 0;

    proc->isolate =  (cpu > 0);


    pfNotify(PFNFY_INTERNAL_DEBUG,PFNFY_PRINT,"Process %s placed on CPU %d (avail=%d)",
	    pfGetPIDName(proc->pid), cpu, acpu);

    // remove proc off its old list
    if ((ocpu = (int)((long)availableCPUList->get(proc->cpu))) > -1)
    {
	list = (pfList*)(CPUList->get(ocpu));
	list->remove((void*)procIndex);
    }

    
    // add proc to a new list
    if (cpu >= 0)
    {
	list = (pfList*)(CPUList->get(cpu));
	list->add((void*)procIndex);
	proc->cpu = cpu;
	//cerr << "\tAdded to list:" << (void*)list << "\tprocIndex:" 
	//			       << procIndex << "\tpid:" << _proc->pid << endl;
    }
}

// --------------------- GET CPU ROUTINES -----------------------
// PURPOSE:
//	These are the routines that do the actual assignment to CPUs
//  They return a pseudo CPU number.  It is not the "real" CPU number
//  it is actually the Nth available CPU.
//  In a system with all the CPUs available for scheduling, then these
//  routines do return the real CPU number
// -------------------------------------------------------------------
// POLICY:
//	CPU0 is never restricted.
//	APPCULLDRAW - handled by app - takes CPU1.
//	APP_CULLDRAW - handled by app and draw
//	APP_CULL_DRAW - each process locks downs its own.
//
// Single pipe mappings:
//  app=APP_CPU, cull=CULL_CPU, draw=DRAW_CPU (defined below)
//  If there are only 2 CPUs,  app is put on CPU0,  which is not isolated, 
//	and the draw and cull share CPU1.
//
// Multipipe mappings:
//	XXX: What about other procs
//  
//  If NumCPUs >= 2 + 2*NumPipes then each process can have its own,  with
//	the application getting CPU1, and UNIX getting CPU0.
//	The draw will get the next NumPipes cpus, and then the cull
//	will get the next set.
//  If  NumCPUs == 1 + 2*NumPipes, then the application shares CPU0 with
//	UNIX.
//  Otherwise,  if NumCPUs >= 2 + NumPipes,  then cull and draw processes
//	for each pipe are paired together.
//  If their are fewer CPUs then that, then the application is put on
//	CPU0 with UNIX and we start pairing and once we run 
//	out, everyone is put on the last CPU.
// ---------------------------------------------------------------------
// NOTE: Only call once changes have been made

int 
pfuDefaultProcessManager::calcAppProcCPU(_pfuProc* _proc)
{
    if (numAvailableCPUs >= (1+numCriticalProcs))
	return 1;
    else
	return 0; 
}

// Cull and Draw CPUS:
//  if cull and draw are on separate CPUs,
//  we'd like a cull/draw pair to be on the same CPU node
//  give cull the even one and draw the odd one

int 
pfuDefaultProcessManager::calcLPointProcCPU(_pfuProc* _proc)
{
    if (baseLPointCPU < 0)
	return -baseLPointCPU;
    else
	return (baseLPointCPU + pfGetId(pfGetPipe(_proc->pipe)));  
}

int 
pfuDefaultProcessManager::calcDrawProcCPU(_pfuProc* _proc)
{
    int pipeNum = pfGetId(pfGetPipe(_proc->pipe));
    if (criticalPathHasOwnCPU)
    {
	if (pairCullDraw)
	    return (baseAppCPU + (pipeNum) + 1);
	else
	    return (baseAppCPU + (2*pipeNum) + 2);
    }
    else
		    // The cpu num would be out of range
	return ( ((baseAppCPU + pipeNum + 1)  >= numCPUs) ? (numCPUs - 1) :
		 (baseAppCPU + pipeNum + 1) );
}

int 
pfuDefaultProcessManager::calcSidekickProcCPU(_pfuProc* _proc)
{
    int		result;

    if (sidekickHasOwnCPU < 0)
	result = baseSidekickCPU;
    else
    if (sidekickHasOwnCPU == 0)
	result = baseSidekickCPU;
    else
	result = (baseSidekickCPU + sidekickID);

    // Count how many CULL_SIDEKICK processes we hit so far.
    sidekickID ++;
    return result;
}

int 
pfuDefaultProcessManager::calcCullProcCPU(_pfuProc* _proc)
{
    int pipeNum = pfGetId(pfGetPipe(_proc->pipe));

    if (criticalPathHasOwnCPU)
    {
	if (pairCullDraw)
	    return (baseAppCPU + (pipeNum) + 1);
	else
	    return (baseAppCPU + (2*pipeNum) + 1);
    }
    else
		    // The cpu num would be out of range
	return ( ((baseAppCPU + pipeNum + 1)  >= numCPUs) ? (numCPUs - 1) :
		 (baseAppCPU + pipeNum + 1) );
}

int 
pfuDefaultProcessManager::calcIsectProcCPU(_pfuProc* _proc)
{
    return baseComputeCPU;
}
    
int 
pfuDefaultProcessManager::calcComputeProcCPU(_pfuProc* _proc)
{
    if (computeHasOwnCPU > 0)
    {
	return baseComputeCPU + ((_proc->stage & PFPROC_COMPUTE) != 0);
    }
    else
	return baseComputeCPU;
}

int 
pfuDefaultProcessManager::calcQueueProcCPU(_pfuProc* _proc)
{
    // if there is enough, put the QUEUE procs on their
    // own CPU, else on the DBASE CPU
    if ((_proc->stage & PFPROC_QUEUE_SERVICE) && (queueServiceCPU >= 0))
    {
	return queueServiceCPU;
    }
    if (dbaseHasOwnCPU && _proc->stage & (PFPROC_QUEUE_SERVICE | PFPROC_QUEUE_SORT))
	    return baseDBaseCPU+1;
    else
	return baseDBaseCPU;
}

int 
pfuDefaultProcessManager::calcDbaseProcCPU(_pfuProc* _proc)
{
    // if there is enough, put the BASE proc on its own CPU
    return baseDBaseCPU;
}
        
void 
pfuDefaultProcessManager::addProc(int _pipe, uint _stage, pid_t _pid)
{
    _pfuProc*	procToAdd = new _pfuProc(_pipe, _stage, _pid, numPipes);


    // see if user takes this one
    if (uCreateCB  && (uCreateMask & procToAdd->stage))
	if (uCreateCB(_pipe, _stage, _pid) != PFU_CB_CONT)
	    return;

    procList->add(procToAdd);	// Add proc to the procList
    
    if (procToAdd->stage & 
	    (PFPROC_APP | PFPROC_CULL | PFPROC_LPOINT | PFPROC_DRAW))
	numCriticalProcs++;	    // Added another critical-path proc

    if (procToAdd->stage & 
	    (PFPROC_ISECT | PFPROC_COMPUTE))
	numComputeProcs++;	    // Added another non-critical-path proc

    if (procToAdd->stage & PFPROC_CULL_SIDEKICK)
	numSidekickProcs++;	    // Added another CULL_SIDEKICK proc
    
    //dumpProcList();
    
    // if doing continuous placement, place proc now.
    // otherwise, we'll wait for a call to reconfigure.
    if (mode & _PM_CONTINUOUS)
	placeProcesses(-1);	    // Tell all the processes which CPUs to go to
}

void 
pfuDefaultProcessManager::dumpProcList(void)
{
    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_INTERNAL, "------ ProcList dump -----");
    
    for(int iter = 0; iter < procList->getNum(); iter++)
	((_pfuProc*)procList->get(iter))->printDebug();
}
   
// ---------------------------------------- //
// ----- pfuBaseProcessManager Members ----- //
// ---------------------------------------- //
int 
pfuBaseProcessManager::freeAllCPUs(void)
{
#if (!defined(__linux__has_sysmp_h__) && defined(__linux__)) || (!defined(__win32__has_sysmp_h__) && defined(WIN32))
    pfNotify(PFNFY_WARN, PFNFY_PRINT,
	"pfuBaseProcessManager::freeAllCPUs not supported in Linux\n");
    return TRUE;
#else
    if (numCPUs == sysmp(MP_NAPROCS))
	return TRUE;
	
    if (!(geteuid() == 0))	// Check for root
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfuProcessManager::freeAllCPUs - Not root - can't free restricted CPUs.");
	return FALSE;
    }
    
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfuBaseProcessManager::freeAllCPUs - Freeing up isolated CPUs.");
    for (int i=0; i < availableCPUList->getNum(); i++)
    {
	sysmp(MP_UNISOLATE, (int)((long)availableCPUList->get(i)));
	sysmp(MP_EMPOWER, (int)((long)availableCPUList->get(i)));		// allow CPU to run any process
    }
    return TRUE;
#endif  /* __linux__ */
}

// PURPOSE: restrictes the calling process to running only on the
// specified CPU but does not isolate the CPU from running any other processes.
int 
pfuBaseProcessManager::runProcOn(pid_t pid, int cpu)
{
#if (!defined(__linux__has_sysmp_h__) && defined(__linux__)) || (!defined(__win32__has_sysmp_h__) && defined(WIN32))
    pfNotify(PFNFY_WARN, PFNFY_PRINT,
	"pfuBaseProcessManager::runProcOn not supported in Linux\n");
    return TRUE;
#else
    if (cpu >= numCPUs)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
		"pfuProcessManager::runProcOn FAILED: CPU %d requested for process %d but I only have %d CPUS!", 
		cpu, pid, numCPUs);
	return FALSE;
    }
    
    if (sysmp(MP_MUSTRUN_PID, cpu, pid) < 0)
    {
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT, 
		"pfuProcessManager::runProcOn - MUSTRUN of process %d on CPU %d failed.", pid, cpu);
	return 0;
    } else
	pfNotify(PFNFY_INFO,PFNFY_PRINT, 
	"pfuProcessManager::runProcOn - Process %d \"%s\" running on CPU %d.", 
	pid, pfGetPIDName(pid), cpu);
    
    return TRUE;	  
#endif
}

// PURPOSE: isolates the specified CPU to running only processes locked to it   
int 
pfuBaseProcessManager::isolateCPU(int cpu)
{
#if (!defined(__linux__has_sysmp_h__) && defined(__linux__)) || (!defined(__win32__has_sysmp_h__) && defined(WIN32))
    pfNotify(PFNFY_WARN, PFNFY_PRINT,
	"pfuBaseProcessManager::runProcOn not supported in Linux\n");
    return TRUE;
#else
    if (cpu >= numCPUs)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfuProcessManager::isolateCPU FAILED: CPU %d requested, but I only have %d CPUS!", cpu, numCPUs);
	return FALSE;
    }
    
    
    if (sysmp(MP_ISOLATE, cpu) < 0)
    {
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"pfuProcessManager::isolateCPU - ISOLATE of CPU %d for process FAILED. "
		"Are you root?", cpu);
	return 0;
    } else {
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	    "pfuProcessManager::isolateCPU - CPU %d is ISOLATED.", cpu);
    }
    return TRUE;
#endif
}

void
pfuBaseProcessManager::init(void)
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfuBaseProcessManager");
    }
}

void
pfuDefaultProcessManager::init(void)
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfuDefaultProcessManager");
    }
}

    // Start up default process Manager

PFUDLLEXPORT void
pfuInitProcessManagerClasses(void)
{
   if (pfuBaseProcessManager::classType)
    return;
    
	
    if (pfIsConfiged())
        pfNotify(PFNFY_DEBUG, PFNFY_USAGE, 
	"pfuInitProcessManagerClasses() should be called before pfConfig()"
	" for multiprocessed operation");
    pfuBaseProcessManager::init();
    pfuDefaultProcessManager::init();

}

PFUDLLEXPORT void 
pfuInitDefaultProcessManager(void)
{
   pfuDefaultProcessManager* manager;

   manager = new pfuDefaultProcessManager;
   if (!manager)
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	"pfuInitDefaultProcessManager() - creation of pfuDefaultProcessManager failed!");

   pfuProcessManager::select(manager);
   
   pfCreateProcessFunc(pfuProcessManager::callback);
}

    // Reconfigure the system using the given process Manager
PFUDLLEXPORT void 
pfuReconfigureProcessManager()
{
    pfuProcessManager::reconfigure();
}

    // Release all system resources helld by the process Manager
PFUDLLEXPORT void 
pfuReleaseProcessManager()
{
    pfuProcessManager::release();
}


    // Set the current process manager 
PFUDLLEXPORT void 
pfuSelectProcessManager(pfuBaseProcessManager *mngr)
{
   pfuProcessManager::select(mngr);
}

    // Get the current process manager 
PFUDLLEXPORT pfuBaseProcessManager * 
pfuGetCurProcessManager(void)
{
   return pfuProcessManager::getCur();
}


    // Set the process manager run-time mode
PFUDLLEXPORT void 
pfuProcessManagerMode(int mode, int val)
{
    pfuProcessManager::setMode(mode, val);
}

PFUDLLEXPORT int 
pfuGetProcessManagerMode(int mode)
{
    return pfuProcessManager::getMode(mode);
}

PFUDLLEXPORT void 
pfuProcessManagerCreateFunc(pfuProcessHandlerFuncType func, uint procMask)
{
    pfuProcessManager::setCreateFunc(func, procMask);
}

PFUDLLEXPORT void 
pfuGetProcessManagerCreateFunc(pfuProcessHandlerFuncType *func, uint *procMask)
{
    pfuProcessManager::getCreateFunc(func, procMask);
}

PFUDLLEXPORT void 
pfuProcessManagerPlaceFunc(pfuProcessHandlerFuncType func, uint procMask)
{
    pfuProcessManager::setPlaceFunc(func, procMask);
}

PFUDLLEXPORT void 
pfuGetProcessManagerPlaceFunc(pfuProcessHandlerFuncType *func, uint *procMask)
{
    pfuProcessManager::getPlaceFunc(func, procMask);
}


PFUDLLEXPORT void 
pfuProcessManagerPrioritizeFunc(pfuProcessHandlerFuncType func, uint procMask)
{
    pfuProcessManager::setPrioritizeFunc(func, procMask);
}

PFUDLLEXPORT void 
pfuGetProcessManagerPrioritizeFunc(pfuProcessHandlerFuncType *func, uint *procMask)
{
    pfuProcessManager::getPrioritizeFunc(func, procMask);
}
