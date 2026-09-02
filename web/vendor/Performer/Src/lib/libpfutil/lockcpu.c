/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 */

/*
 * file: lockcpu.c
 * ----------------
 * 
 * 
 * This file contains routines to lock down processes on processors for 
 *	    app, cull, and draw process.
 *
 * Note: isolating a CPU to specific processors requires root id.
 * 
 *
 * Priority:
 * --------
 *	All processes are 128 or lower so they are not higher priority then kernel
 *	APP  - 120
 *	CULL - 120	
 *	DRAW - 125
 *
 * Policy:
 * -------
 *	CPU0 is never restricted.
 *	APPCULLDRAW - handled by app - takes CPU1.
 *	APP_CULLDRAW - handled by app and draw
 *	APP_CULL_DRAW - each process locks downs its own.
 *
 * Single pipe mappings:
 *  app=APP_CPU, cull=CULL_CPU, draw=DRAW_CPU (defined below)
 *  If there are only 2 CPUs,  app is put on CPU0,  which is not isolated, 
 *	and the draw and cull share CPU1.
 *
 * Multipipe mappings:
 *  If NumCPUs >= 2 + 2*NumPipes then each process can have its own,  with
 *	the application getting CPU1, and UNIX getting CPU0.
 *	The draw will get the next NumPipes cpus, and then the cull
 *	will get the next set.
 *  If  NumCPUs == 1 + 2*NumPipes, then the application shares CPU0 with
 *	UNIX.
 *  Otherwise,  if NumCPUs >= 2 + NumPipes,  then cull and draw processes
 *	for each pipe are paired together.
 *  If their are fewer CPUs then that, then the application is put on
 *	CPU0 with UNIX and we start pairing and once we run 
 *	out, everyone is put on the last CPU.
 *  
 * Routines return 1 if successful,  0 if an error is encountered.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

/* process control includes */
#ifndef WIN32
#include <sys/statfs.h>
#include <sys/sysmp.h>
#endif
#include <sys/types.h>
#if defined(__linux__has_schedctl_h__) || defined(mips) || defined(__win32__has_schedctl_h__)
#include <sys/schedctl.h>
#endif  /* __linux__ */

/* IRIS Performer includes */
#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pfutil.h>

/* default CPUs assignments for locking down processes for 
 * single pipe 4 CPU system
 */
#define APP_CPU 1
#define CULL_CPU 3
#define DRAW_CPU 2
#define LPOINT_CPU 4

#ifndef WIN32
#ifdef HAVE_POSIX
#include <sched.h>  /* 6.4+ POSIX Scheduling */
#include <dlfcn.h>
#endif
#endif

#define APP_PRIORITY 120
#define CULL_PRIORITY 120
#define DRAW_PRIORITY 125
#define CLOCK_PRIORITY 50
#define ISECT_PRIORITY 75

PFUDLLEXPORT int pfuLockDownProc(int cpu);
PFUDLLEXPORT int pfuRunProcOn(int cpu);
PFUDLLEXPORT int pfuLockDownApp(void);
PFUDLLEXPORT int pfuLockDownCull(pfPipe *);
PFUDLLEXPORT int pfuLockDownDraw(pfPipe *);
PFUDLLEXPORT int pfuLockDownLPoint(pfPipe *);
PFUDLLEXPORT int pfuRunDiskProcsOn(int cpu);

/* None of these functions are implemented for Linux.
   Will stub them away for now. */
#if (!defined(__linux__has_sysmp_h__) && defined(__linux__)) || (!defined(__win32__has_sysmp_h__) && defined(WIN32))
PFUDLLEXPORT int pfuFreeAllCPUs(void) { STUB; }
PFUDLLEXPORT int pfuLockDownProc(int cpu) { STUB; }
PFUDLLEXPORT int pfuRunProcOn(int cpu) { STUB; }
PFUDLLEXPORT int pfuRunDiskProcsOn(int cpu) { STUB; }
PFUDLLEXPORT int pfuLockDownApp(void) { STUB; }
PFUDLLEXPORT int pfuLockDownCull(pfPipe* p) { STUB; }
PFUDLLEXPORT int pfuLockDownDraw(pfPipe* p) { STUB; }
PFUDLLEXPORT int pfuLockDownLPoint(pfPipe* p) { STUB; }
#endif

#if (!defined(__linux__had_schedctl_h__) && defined(__linux__)) || (!defined(__win32__has_schedctl_h__) && defined(WIN32))
PFUDLLEXPORT int pfuPrioritizeProcs(int cpu) { STUB; }
#endif

#if defined(mips) || defined(__linux__has_sysmp_h__) || defined(__win32__has_sysmp_h__)
/*
 * FreeAllCPUs (when run as root) will free up all restricted/isolated CPUs
 *  on the system.  This routine should be called before pfConfig() and
 *  just before pfExit();
 */
PFUDLLEXPORT int
pfuFreeAllCPUs(void)
{
    ptrdiff_t numCPUs = sysmp(MP_NPROCS);
    int i;

    if (numCPUs == sysmp(MP_NAPROCS))
	return TRUE;

    if (!(geteuid() == 0))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		"pfuFreeAllCPUs - Not root - can't free restricted CPUs.");
	return FALSE;
    }
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfuFreeAllCPUs - Freeing up isolated CPUs.");
    for (i=0; i < numCPUs; i++)
    {
	sysmp(MP_UNISOLATE, i);
	sysmp(MP_EMPOWER, i); /* allow CPU to run any process */
    }
    return TRUE;
}

/*
 * pfuLockDownProc(int cpu) restricts the calling process to the specified cpu
 * and isolates the CPU to running only processes that have locked themselves to
 * this cpu.
 */
PFUDLLEXPORT int
pfuLockDownProc(int cpu)
{
    ptrdiff_t numCPUs = sysmp(MP_NPROCS);
    long pid = getpid();
    
    if (cpu >= numCPUs)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfuLockDownProc FAILED: CPU %d requested for process %d but I only have %d CPUS!", 
		cpu, pid, numCPUs);
	return 0;
    }
    
    if (sysmp(MP_MUSTRUN, cpu) < 0)
    {
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	    "pfuLockDownProc - MUSTRUN of process %d on CPU %d failed.", pid, cpu);
	return 0;
    } else
	pfNotify(PFNFY_INFO,PFNFY_PRINT,
	    "pfuLockDownProc - Process %d running on CPU %d.", pid, cpu);
	    
    if (sysmp(MP_RESTRICT, cpu) < 0)
    {
	sysmp(MP_RUNANYWHERE, 1);
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"pfuLockDownProc - RESTRICT of CPU %d for process %d FAILED. "
		"Are you root?", cpu, pid);
	return 0;
    } else {
	sysmp(MP_ISOLATE, cpu);
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	    "pfuLockDownProc - CPU %d is LOCKED running process %d.", cpu, pid);
    }
    return 1;
}

/*
 * pfuRunProcOn(int cpu) restrictes the calling process to running only on the
 * specified CPU but does not isolate the CPU from running any other processes.
 */
PFUDLLEXPORT int 
pfuRunProcOn(int cpu)
{
    ptrdiff_t numCPUs = sysmp(MP_NPROCS);
    long pid = getpid();
    
    if (cpu >= numCPUs)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfuRunProcOn FAILED: CPU %d requested for process %d but I only have %d CPUS!", 
		cpu, pid, numCPUs);
	return 0;
    }
    
    if (sysmp(MP_MUSTRUN, cpu) < 0)
    {
#ifdef __linux__
	pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
	    "pfuRunProcOn() - MUSTRUN of process %d on CPU %d failed.", pid, cpu);
#else
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	    "pfuRunProcOn() - MUSTRUN of process %d on CPU %d failed.", pid, cpu);
#endif
	return 0;
    } else
	pfNotify(PFNFY_INFO,PFNFY_PRINT,
	    "pfuRunProcOn() - Process %d running on CPU %d.", pid, cpu);
    
    return 1;	  
}

/*
 * pfuDiskProcsRunOn will place the all disk reading threads
 * to run on the specified cpu
 * If -1 is passed in, then the disk threads will get their
 * own cpu if available or cpu 0 otherwise.
 */
PFUDLLEXPORT int
pfuRunDiskProcsOn(int cpu)
{
    int nprocs,i;
    int numPipes = pfGetMultipipe();
    ptrdiff_t numCPUs = sysmp(MP_NPROCS);
    
    if (numCPUs == 1)
    {
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	    "I only have %d processors - "
	    "all disk threads have to run on cpu 0", numCPUs);
	return 0;
    }
   
    if (cpu == -1)
    {
    	if (numCPUs >= 3 + 2 * numPipes)
	    cpu = 2*numPipes+3; /* everyone gets their own = disks go to end */
        else
	    cpu = 0;
    }

#ifndef IRIX5
    nprocs = pfGetNumGlobalQueueServiceProcs();
    for(i=0;i<nprocs;i++)
    	if (sysmp(MP_MUSTRUN_PID, cpu, pfGetGlobalQueueServiceProcPID(i)) < 0)
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	         "MUSTRUN of disk Process %d on cpu %d failed.", pfGetGlobalQueueServiceProcPID(i),cpu);
    	else
	    pfNotify(PFNFY_INFO, PFNFY_PRINT,
	    	"Disk Process %d running on cpu %d.", pfGetGlobalQueueServiceProcPID(i),cpu);
#endif
#if 0
/* XXX - Dont restrict disk read cpu by default */
    if (cpu)
    {
	if (sysmp(MP_RESTRICT, cpu) < 0)
	{
	    sysmp(MP_RUNANYWHERE, 1);
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		    "RESTRICT of cpu %d for disk processes FAILED. "
		    "Are you root?", cpu);
	    return 0;
	} else  {
	    sysmp(MP_ISOLATE, cpu);
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"Processor %d is LOCKED running disk processes.", cpu);
	}
    } 
#endif

    return 1;
}


/*
 * pfuLockDownApp will place the application processor on processor 0 or 1.
 *  If it is placed on CPU1,  then CPU1 will be isolated.
 *  This routine should be called after pfConfig() and before 
 *  pfConfigPipe or pfFrame().
 */
PFUDLLEXPORT int
pfuLockDownApp(void)
{
    ptrdiff_t numCPUs = sysmp(MP_NPROCS);
    int numPipes = pfGetMultipipe();
    int cpu = 0;
    
    if (numCPUs == 1)
    {
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	    "I only have %d processors - "
	    "not locking down application process.", numCPUs);
	return 0;
    }
   
    if (numCPUs >= 2 + 2 * numPipes)
	cpu = 1; /* everyone gets their own = application goes on CPU1 */

    if (sysmp(MP_MUSTRUN, cpu) < 0)
    {
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	    "MUSTRUN of app on cpu %d failed.", cpu);
	return 0;
    } else
	pfNotify(PFNFY_INFO, PFNFY_PRINT,
	    "Application process running on cpu %d.", cpu);
    if (cpu)
    {
	if (sysmp(MP_RESTRICT, cpu) < 0)
	{
	    sysmp(MP_RUNANYWHERE, 1);
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		    "RESTRICT of cpu %d for app process FAILED. "
		    "Are you root?", cpu);
	    return 0;
	} else  {
	    sysmp(MP_ISOLATE, cpu);
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"Processor %d is LOCKED running application process.", cpu);
	}
    } 
    return 1;
}


/* pfuLockDownCull() locks down a cpu for a cull process - requires root id.
 *  This routine should be called from the pfConfigPipeStage cull callback.
 *
 *  If single pipe, uses CPU defined in DRAW_CPU.
 *  If multipipe (and there are enough CPUs),  uses CPU = 2 + 2*pfGetId().
 *  Alternatives tried in order:
 *	1 + 2*pfGetId() -- assumes app is on CPU0
 *	1 + pfGetId() -- will share with draw
 */
PFUDLLEXPORT int
pfuLockDownCull(pfPipe *p)
{
    ptrdiff_t numCPUs = sysmp(MP_NPROCS);
    long numPipes = pfGetMultipipe();
    long cpu = 0;
    long id;
    
    if (!(pfGetMPBitmask() & PFMP_FORK_CULL))
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	    "LockDownCull - cull process not forked - handles by app or draw");
	return 0;
    }

    id = pfGetId(p);
    if (numPipes == 1)
    {
	if (numCPUs >= 4)
	    cpu = CULL_CPU;
	else if (numCPUs == 2)
	    cpu = 1;
    } else {
        if (numCPUs >= 2 + 2 * numPipes)
	    cpu = 2 + numPipes + id; /* everyone gets their own! */
	else if (numCPUs >= 1 + numPipes)
	    cpu = 1 + id;  /* unix and app on CPU0, pair up cull and draw */
	else if (id+1 < numCPUs)
	    cpu = id+1; /* unix and app on CPU0 */
	else
	    cpu = numCPUs - 1;
    }
    
    if (cpu)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"Locking down processor %d for the Cull Process.", cpu);
	if (sysmp(MP_MUSTRUN, cpu) < 0)
	{
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"MUSTRUN of Cull on processor %d failed.", cpu);
	    return 0;
	}
	else if (sysmp(MP_RESTRICT, cpu) < 0)
	{
	    sysmp(MP_RUNANYWHERE, 1);
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		    "ISOLATE of processor %d for Cull process FAILED. "
		    "Are you root?", cpu);
	    return 0;
	} else {
	    sysmp(MP_ISOLATE, cpu);
	}
    } 
    return 1;
}


/* pfuLockDownDraw() locks down a cpu for the draw process 
 *		- requires root id.
 *  This routine should be called from the pfConfigPipeStage callback.
 * 
 *  If single pipe, uses CPU defined in DRAW_CPU.
 *  If multipipe (and there are enough CPUs),  uses CPU 1 + pfGetId().
 */
PFUDLLEXPORT int
pfuLockDownDraw(pfPipe *p)
{
    ptrdiff_t numCPUs = sysmp(MP_NPROCS);
    long numPipes = pfGetMultipipe();
    long proc = 0;
    long id;
    
    if (numCPUs == 1)
    {
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	"I only have 1 processor - can't Lock Draw down!!!");
	return 0;
    }
    
    id = pfGetId(p);
    if (numPipes == 1)
    {
	if (numCPUs >= 4)
	    proc = DRAW_CPU;
	else if (numCPUs == 2)
	    proc = 1;
    } else {
        if (numCPUs >= 2 + numPipes)
	    proc = 2 + id; /* unix on CPU0, app on CPU1 */
	else if (numCPUs >= 1 + numPipes)
	    proc = 1 + id;  /* unix and app on CPU0 */
	else if (id+1 < numCPUs)
	    proc = id+1; /* unix and app on CPU0 */
	else
	    proc = numCPUs - 1;
    }
    if (proc)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	    "Locking  down processor %d for Draw Process of Pipe %d.",
		    proc, id);
	if (sysmp(MP_MUSTRUN, proc) < 0)
	{
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"MUSTRUN of draw on processor %d failed.", proc);
	    return 0;
	}
	else if (sysmp(MP_RESTRICT, proc) < 0)
	{
	    sysmp(MP_RUNANYWHERE, 1);
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
			"RESTRICT of processor %d for draw process FAILED. "
			"Are you root?", proc);
	    return 0;
	} else {
	    sysmp(MP_ISOLATE, proc);
	}
    } 
    return 1;
}


/* pfuLockDownLPoint() locks down a cpu for the Lpoint process 
 *		- requires root id.
 *  This routine should be called from the pfConfigPipeStage callback.
 * 
 *  If single pipe, uses APP CPU if possible
 *  If multipipe (and there are enough CPUs),  uses CPU 1 + pfGetId().
 */
PFUDLLEXPORT int
pfuLockDownLPoint(pfPipe *p)
{
    ptrdiff_t numCPUs = sysmp(MP_NPROCS);
    long numPipes = pfGetMultipipe();
    long proc = 0;
    long id;
    
    if (numCPUs < 4 )
    {
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	"I only have %d processor - can't Lock LPoint down!!!", numCPUs);
	return 0;
    }
    
    id = pfGetId(p);
    if (numPipes == 1)
    {
	if (numCPUs == 4)
	{
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"Lpoint share the APP cpu because I have only 4 cpus\n");
	    proc = APP_CPU;
	}
	else
	    proc = LPOINT_CPU;
    } else {
        if (numCPUs >= 2 + 2*numPipes)
	    proc = 2 + 2*numPipes + id; /* unix on CPU0, app on CPU1 , Draws on cpu 2,3,4, Cull on cpu 5,6,7
						Lpoint on cpu 8,9,10 */
	else if (numCPUs >= 1 + numPipes)
	    proc = 1 + id;  /* unix and app on CPU0 */
	else if (id+1 < numCPUs)
	    proc = id+1; /* unix and app on CPU0 */
	else
	    proc = numCPUs - 1;
    }
    if (proc)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	    "Locking  down processor %d for LPoint Process of Pipe %d.",
		    proc, id);
	if (sysmp(MP_MUSTRUN, proc) < 0)
	{
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"MUSTRUN of LPoint on processor %d failed.", proc);
	    return 0;
	}
	else if (sysmp(MP_RESTRICT, proc) < 0)
	{
	    sysmp(MP_RUNANYWHERE, 1);
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
			"RESTRICT of processor %d for LPoint process FAILED. "
			"Are you root?", proc);
	    return 0;
	} else {
	    sysmp(MP_ISOLATE, proc);
	}
    } 
    return 1;
}

#if defined(__linux__had_schedctl_h__) || defined(mips) || defined(__win32__has_schedctl_h__)
    /* pri is a bool. TRUE - Then set non-degrading priorities, FALSE - clear priorities */
PFUDLLEXPORT int
pfuPrioritizeProcs(int pri)
{
    int numPipes = pfGetMultipipe();
    int mpbitmask = pfGetMPBitmask();
    int	i;
    int app;

#ifdef HAVE_POSIX
    void *handle=0;
    int _usePOSIX;	/* Should we use POSIX scheduling */
    struct sched_param schedAttrs;
    
    /* everyone else (libGL.so) explicitly references libc.so.1
     * so we have to do it that way too for the rld refcounting to work.
     */
    _usePOSIX = (pfGetIRIXRelease() >= 6.4);
    if (!_usePOSIX)
    {
	if (!(getenv("_PF_NO_POSIX_SCHED")))
	{
	    handle = dlopen("libc.so.1", RTLD_LAZY);
	    _usePOSIX = (dlsym(handle, "sched_setscheduler") != 0);
	}
    }
#endif
    
    if (pri && geteuid() != 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfuPrioritizeProcs() Must be root to increase priorities.");
 	return 0;
    }

#ifdef HAVE_POSIX
    if (!_usePOSIX)
    {
#endif
	app = NDPHIMAX - numPipes + (numPipes * ((mpbitmask & PFMP_FORK_DRAW) != 0));
	if (app > NDPHIMIN)
		app = NDPHIMIN;
	if (pri)
	{
	    if (schedctl(NDPRI, pfGetPID(0, PFPROC_APP), app) < 0)
		pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
		    "pfuPrioritizeProcs() schedctl(NDPRI, pfGetPID(0, PFPROC_APP), %d) failed", app);
		    
	    if (schedctl(NDPRI, pfGetPID(0, PFPROC_CLOCK), app+3) < 0)
		pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
		    "pfuPrioritizeProcs() schedctl(NDPRI, pfGetPID(0, PFPROC_CLOCK), %d) failed", app+3);
		    
	    if (mpbitmask & PFMP_FORK_ISECT)
	    {
		if (schedctl(NDPRI, pfGetPID(0, PFPROC_ISECT), app+2) < 0)
		    pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
			"pfuPrioritizeProcs() schedctl(NDPRI, pfGetPID(0, PFPROC_ISECT), %d) failed", app+2);
	    }
	    if (mpbitmask & PFMP_FORK_COMPUTE)
	    {
		if (schedctl(NDPRI, pfGetPID(0, PFPROC_COMPUTE), app+1) < 0)
		    pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
			"pfuPrioritizeProcs() schedctl(NDPRI, pfGetPID(0, PFPROC_COMPUTE), %d) failed", app+1);
	    }
	}
	else
	{
	    if (schedctl(NDPRI, pfGetPID(0, PFPROC_APP), 0) < 0)
		pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
		    "pfuPrioritizeProcs() schedctl(NDPRI, pfGetPID(0, PFPROC_APP), 0) failed");
		    
	    if (schedctl(NDPRI, pfGetPID(0, PFPROC_CLOCK), 0) < 0)
		pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
		    "pfuPrioritizeProcs() schedctl(NDPRI, pfGetPID(0, PFPROC_CLOCK), 0) failed");
		    
	    if (schedctl(NDPRI, pfGetPID(0, PFPROC_ISECT), 0) < 0)
		pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
		    "pfuPrioritizeProcs() schedctl(NDPRI, pfGetPID(0, PFPROC_ISECT), 0) failed");
	}
    
	for (i=0; i<numPipes; i++)
	{
	    if (pri)
	    {
		if (mpbitmask & PFMP_FORK_CULL)
		{
		if (schedctl(NDPRI, pfGetPID(i, PFPROC_CULL), app - i) < 0)
		    pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
			"pfuPrioritizeProcs() schedctl(NDPRI, pfGetPID(%d, PFPROC_CULL), %d) failed", i, app-i);
		}
		    
		if (mpbitmask & PFMP_FORK_DRAW)
		{
		if (schedctl(NDPRI, pfGetPID(i, PFPROC_DRAW), app - numPipes - i) < 0)
		    pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
			"pfuPrioritizeProcs() schedctl(NDPRI, pfGetPID(%d, PFPROC_DRAW), NDPHIMAX+0) failed", i, 
				app - numPipes - i);
		}
	    }
	    else
	    {
		if (schedctl(NDPRI, pfGetPID(i, PFPROC_CULL), 0) < 0)
		    pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
			"pfuPrioritizeProcs() schedctl(NDPRI, pfGetPID(%d, PFPROC_CULL), 0) failed", i);
		    
		if (schedctl(NDPRI, pfGetPID(i, PFPROC_DRAW), 0) < 0)
		    pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
			"pfuPrioritizeProcs() schedctl(NDPRI, pfGetPID(%d, PFPROC_DRAW), 0) failed", i);
	    }
	}
#ifdef HAVE_POSIX

    } else {	    /* (!_usePOSIX) --- ASSERT: We want to use POSIX */
 	if (pri) 
 	{ 
 	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "pfuPrioritizeProcs(): Using POSIX"); 
 	     
 	    schedAttrs.sched_priority = APP_PRIORITY; 
 	     
 	    if (sched_setscheduler(pfGetPID(0, PFPROC_APP), SCHED_FIFO, &schedAttrs) < 0) 
 		pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
 		    "pfuPrioritizeProcs() sched_setscheduler(pfGetPID(0, PFPROC_APP), SCHED_FIFO, &schedAttrs) failed"); 
 	     
 	    schedAttrs.sched_priority = CLOCK_PRIORITY; 
 	    	     
 	    if (sched_setscheduler(pfGetPID(0, PFPROC_CLOCK), SCHED_FIFO, &schedAttrs) < 0) 
 		pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
 		    "pfuPrioritizeProcs() sched_setscheduler(pfGetPID(0, PFPROC_CLOCK), SCHED_FIFO, &schedAttrs) failed"); 
 	     
	    if (mpbitmask & PFMP_FORK_ISECT)
	    {
		schedAttrs.sched_priority = APP_PRIORITY - 2; 
		     
		if (sched_setscheduler(pfGetPID(0, PFPROC_ISECT), SCHED_FIFO, &schedAttrs) < 0) 
		    pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
			"sched_setscheduler(pfGetPID(0, PFPROC_ISECT), SCHED_FIFO, &schedAttrs)"); 
	    }
	    if (mpbitmask & PFMP_FORK_COMPUTE)
	    {
		schedAttrs.sched_priority = APP_PRIORITY - 1; 
		     
		if (sched_setscheduler(pfGetPID(0, PFPROC_COMPUTE), SCHED_FIFO, &schedAttrs) < 0) 
		    pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
			"sched_setscheduler(pfGetPID(0, PFPROC_COMPUTE), SCHED_FIFO, &schedAttrs)"); 
	    }
 	} 
 	else 
 	{ 
 	    schedAttrs.sched_priority = 0; 
 	     
 	    if (sched_setscheduler(pfGetPID(0, PFPROC_APP), SCHED_OTHER, &schedAttrs) < 0) 
 		pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
 		    "pfuPrioritizeProcs() sched_setscheduler(pfGetPID(0, PFPROC_APP), SCHED_OTHER, &schedAttrs) failed"); 
 	     
 	    if (sched_setscheduler(pfGetPID(0, PFPROC_CLOCK), SCHED_OTHER, &schedAttrs) < 0) 
 		pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
 		    "pfuPrioritizeProcs() sched_setscheduler(pfGetPID(0, PFPROC_CLOCK), SCHED_OTHER, &schedAttrs) failed"); 
 	     
 	    if (sched_setscheduler(pfGetPID(0, PFPROC_ISECT), SCHED_OTHER, &schedAttrs) < 0) 
 		pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
 		    "sched_setscheduler(pfGetPID(0, PFPROC_ISECT), SCHED_OTHER, &schedAttrs)"); 
 	} 
      
 	for (i=0; i<numPipes; i++) 
 	{ 
 	    if (pri) 
 	    { 
		if (mpbitmask & PFMP_FORK_CULL)
		{
		    schedAttrs.sched_priority = APP_PRIORITY + 1 + i; 
		     
		    if (sched_setscheduler(pfGetPID(i, PFPROC_CULL), SCHED_FIFO, &schedAttrs) < 0) 
			pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
			    "pfuPrioritizeProcs() sched_setscheduler(pfGetPID(i, PFPROC_CULL), SCHED_FIFO, &schedAttrs)", i); 
			 
		}
		if (mpbitmask & PFMP_FORK_DRAW)
		{
		    schedAttrs.sched_priority = APP_PRIORITY + 1 + numPipes + i; 
		     
		    if (sched_setscheduler(pfGetPID(i, PFPROC_DRAW), SCHED_FIFO, &schedAttrs) < 0) 
			pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
			    "pfuPrioritizeProcs() sched_setscheduler(pfGetPID(i, PFPROC_DRAW), SCHED_FIFO, &schedAttrs)", i); 
		}
 	    } 
 	    else 
 	    { 
 		schedAttrs.sched_priority = 0; 
 		 
 		if (sched_setscheduler(pfGetPID(i, PFPROC_CULL), SCHED_OTHER, &schedAttrs) < 0) 
 		    pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
 			"pfuPrioritizeProcs() sched_setscheduler(pfGetPID(i, PFPROC_CULL), SCHED_OTHER, &schedAttrs)", i); 
 		     
 		if (sched_setscheduler(pfGetPID(i, PFPROC_DRAW), SCHED_OTHER, &schedAttrs) < 0) 
 		    pfNotify(PFNFY_WARN, PFNFY_SYSERR,  
 			"pfuPrioritizeProcs() sched_setscheduler(pfGetPID(i, PFPROC_DRAW), SCHED_OTHER, &schedAttrs)", i); 
 	    } 
 	} 
    }
#endif

    return 1;
}
#endif  
#endif  
