/*
 * Copyright 2000, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * This source code ("Source Code") was originally derived from a
 * code base owned by Silicon Graphics, Inc. ("SGI")
 * 
 * LICENSE: SGI grants the user ("Licensee") permission to reproduce,
 * distribute, and create derivative works from this Source Code,
 * provided that: (1) the user reproduces this entire notice within
 * both source and binary format redistributions and any accompanying
 * materials such as documentation in printed or electronic format;
 * (2) the Source Code is not to be used, or ported or modified for
 * use, except in conjunction with OpenGL Performer; and (3) the
 * names of Silicon Graphics, Inc.  and SGI may not be used in any
 * advertising or publicity relating to the Source Code without the
 * prior written permission of SGI.  No further license or permission
 * may be inferred or deemed or construed to exist with regard to the
 * Source Code or the code base of which it forms a part. All rights
 * not expressly granted are reserved.
 * 
 * This Source Code is provided to Licensee AS IS, without any
 * warranty of any kind, either express, implied, or statutory,
 * including, but not limited to, any warranty that the Source Code
 * will conform to specifications, any implied warranties of
 * merchantability, fitness for a particular purpose, and freedom
 * from infringement, and any warranty that the documentation will
 * conform to the program, or any warranty that the Source Code will
 * be error free.
 * 
 * IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
 * LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
 * ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
 * SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
 * OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
 * PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
 * OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
 * USE, THE SOURCE CODE.
 * 
 * Contact information:  Silicon Graphics, Inc., 
 * 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
 * or:  http://www.sgi.com
 */

/*******************************************
 * this program converts all the clipmap 
 * textures  into 888 sgi raw format
 * the textures includes a set of mipmaps
 * each of which is of size unitxunit.
 * when the virtual texture size is less
 * than unit, it is a pyramid.
 ****************************************/

#include<stdio.h>
main()
{
    int i, j, k;
    int maxsize, unit;
    char s1[30], s2[30], cmd[100];
    char foo[10];

    maxsize = 16384;
    unit = 1024;
    strcpy(foo, "AF");

    for(i = maxsize; i > 0; i /= 2) {
	if(i > unit) {
	    for(j = 0; j < i/unit; j++) 
	    for(k = 0; k < i/unit; k++) {
		sprintf(s1, "%s.%d.r%03d.c%03d.rgb", foo, i, j, k);
		sprintf(s2, "%s.%d.r%03d.c%03d.raw888", foo, i, j, k);
		sprintf(cmd, "./to888 %s %s", s1, s2);
		printf("%s\n", cmd);
		system(cmd);
	    }
	}
	else
	{
	    sprintf(s1, "%s.%d.r000.c000.rgb", foo, i);
	    sprintf(s2, "%s.%d.r000.c000.raw888", foo, i);
	    sprintf(cmd, "./to888 %s %s", s1, s2);
	    printf("%s\n", cmd);
	    system(cmd);
	}
    }
}
