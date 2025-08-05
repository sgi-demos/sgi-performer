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
 * flybox.c
 *
 * $Revision: 1.1 $
 * $Date: 2000/11/21 21:39:36 $
 *
 * A simple fly-thru program that emulates the flight 
 * characteristics of a flying carpets, flying saucer, or whatever
 * you think it  is.   The goal is to have an easy to use interface 
 * with a flybox for quick, easy to learn database fly-throughs.
 *
 * Drive mode:	stick		effect
 *		forward - move forward
 *		backward - move backward
 *		right - turn right
 *		left - turn left
 *		twist clockwise - turn right
 *		twist counter clockwise - turn right
 *
 * Fly mode:	stick		effect
 *		forward - move forward
 *		backward - move backward
 *		right - roll right
 *		left - roll left
 *		twist clockwise - turn right
 *		twist counter clockwise - turn right
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <math.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <X11/X.h>
#include <unistd.h>

/* undef this to get new 6.4 POSIX fields */
#define _OLD_TERMIOS 1
#include <termio.h>
#include <termios.h>

#include <Performer/pfutil.h>

#ifndef __linux__
#ifdef _POSIX_SOURCE
extern int ioctl(int fildes, int request, ...);
extern long sginap(long ticks);
#endif
#endif  /* __linux__ */

/*
 * simple flight characteristics
 *
 * speeds in meters/second, headings in degrees/second
 */

#define NOISE 0.05f
#define TOFFSET .22f

#define DENOISE(x) (((x<=-NOISE)||(x>=NOISE)) ? x : 0.0f )

#define PORT "/dev/flybox"	/* symlink to /dev/ttydX  */

static int	FlyboxPort;
static int 	FlyboxActive = 0;

static void ConvertSerial(char *str,float *newbuf, int *dioval);

int
pfuGetFlyboxActive(void)
{
    return FlyboxActive;
}
int
pfuInitFlybox(void)
{
    int   st, dig, i;
    float ana[8];
    
    st = pfuOpenFlybox(PORT);
    if (st < 0)
	return(-1);
    
    for(i=0; i<10; i++)
    {
	if(pfuGetFlybox(ana, &dig) >= 0)
	    break;
	sginap(2);
    }
    if(i >= 10)
	return(-1);
    
    FlyboxActive = 1;
    return(0);
    
}


int
pfuGetFlybox(float *analog, int *dig)
{
    static float a[8];
    static int but;
    
    if(pfuReadFlybox(&but, a) < 0)
	return(-1);
    analog[0] = DENOISE(a[0]);
    analog[1] = DENOISE(a[1]);
    analog[2] = DENOISE(a[2]);
    if((a[3] + TOFFSET) < 0.0f)
	analog[3] = 0.0f;
    else
	analog[3] = a[3] + TOFFSET;
    if((a[4] + TOFFSET) < 0.0f)
	analog[4] = 0.0f;
    else
	analog[4] = a[4] + TOFFSET;
    *dig = but;
    return(0);
}

int 
pfuOpenFlybox(char *ptr) 
{
#ifndef __linux__
    struct termio tios;
#else
    struct termios tios;
#endif
    int  st;
    char pt[32];
    
    /*
     * Initialize flybox port.
     */
    
    if (ptr == NULL)
	sprintf(pt,"%s",PORT);
    else
	sprintf(pt,"%s",ptr);
    
    FlyboxPort = open(pt, O_RDWR|O_NONBLOCK);
    if (FlyboxPort < 0)
    {
	perror(pt);
	return(-1);
    }
#ifndef __linux__
    st = ioctl(FlyboxPort,TCGETA,&tios);
    if (st < 0)
    {
	perror(pt);
	return(-1);
    }
#endif
    tios.c_iflag = IGNBRK|IXON|IXOFF;
    tios.c_oflag = 0;
    tios.c_lflag = ICANON;
    tios.c_cflag = B19200|CS8|CREAD|CLOCAL;

/* this ifdef only works if on a 6.4 or later system!! */
#if !defined(_OLD_TERMIOS) && _NO_ABIAPI 
   tios.c_cflag = CS8|CREAD|CLOCAL;
   tios.c_ospeed = 19200;
   tios.c_ispeed = 19200;
#else /* this if for before IRIX 6.4 (6.3?) */
   tios.c_cflag = B19200|CS8|CREAD|CLOCAL;
#endif

#ifndef __linux__
    st = ioctl(FlyboxPort,TCSETAF,&tios);
    if (st < 0)
    {
	perror(pt);
	return(-1);
    }
#else
    tcflush(FlyboxPort, TCIFLUSH);
	tcsetattr(FlyboxPort,TCSANOW,&tios);
#endif
    
    return(1);
}

int 
pfuReadFlybox(int *dioval, float *inbuf)
{
    long i,st;
    char str[100];
    
    st = write(FlyboxPort, "O",1);
    if (st < 0)
	return(-1);
    
    /* XXX - potentially int wait??? */
    
    st = read(FlyboxPort, str, sizeof(str)-1);
    if (st < 0)
       	return(-2);
    if (st > 0)
	str[st-1] = 0;
    
    if (strlen(str) == 24)
    {
	st = 0;
	for (i = 0; i < 22; i++)
	    st += str[i];
	st &= 0xff;
	i = ((0x0f & (str[22]-0x21)) <<  4) |
	    (0x0f & (str[23]-0x21));
	if(st != i)
	    return(-5);
    }
    ConvertSerial(str, inbuf, dioval);
    return(0);
}


static void 
ConvertSerial(char *str,float *inbuf, int *dioval)
{
    int i,digp=0;
    
    if (strlen(str) == 24 && str[0] == 'B')
    {
	/*
	 *  Load the digital input values longo dioval
	 */
        for (i = 2; i < 6; i++)
            digp = (digp << 4) | (0x0f & (str[i]-0x21));
        *dioval = 0xff & digp;
        *dioval |= 0x100 & ~digp;
	
	/*
	 *  Load the 8 analog values into inbuf
	 */
        for (i = 6; i < 22; i += 2)
        {
            digp = (((0x3f & (str[i]-0x21)) << 6) |
		     (0x3f & (str[i+1]-0x21)));
            inbuf[(i-6)/2] = -1.0f + 2.0f* (1.0f * digp/4096);
        }
	
    }
}
