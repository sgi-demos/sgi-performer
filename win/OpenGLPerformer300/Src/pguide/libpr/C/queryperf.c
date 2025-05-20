/*
 * Copyright 1995, Silicon Graphics, Inc.
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
 *
 * queryperf.c: query performance parameters of machine
 *
 * $Revision: 1.3 $
 *
 */

#include <Performer/pr.h>

static char *glstr[] = {"IRISGL",  "OPENGL"};

pfTexSubloadCostTable *table;

void
printArray(float *array, int wid, int ht, char *str)
{
    int i, j;
    if(array)
    {
	for(j = 0; j < ht; j++)
	{
	    for(i = 0; i < wid; i++)
	    {
		printf("%.2f ", array[i + wid * j]);
	    }	
	    printf("\n");
	}
    }
    else
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "No array defined for %s", str);
}

void
printTable(pfTexSubloadCostTable *table)
{
    printf("Cost Table Dimensions: %d, %d\n", table->wid, table->ht);
    printf("Cost Table Alignment: %d, %d, %d\n", 
	   table->alignS, table->alignT, table->alignR);

    printf("1 Byte Cost Table Times (msec):\n");
    printArray(table->cost1BPT, table->wid, table->ht, "1 byte table");
    printf("2 Byte Cost Table Times (msec):\n");
    printArray(table->cost2BPT, table->wid, table->ht, "2 byte table");
    printf("3 Byte Cost Table Times (msec):\n");
    printArray(table->cost3BPT, table->wid, table->ht, "3 byte table");
    printf("4 Byte Cost Table Times (msec):\n");
    printArray(table->cost4BPT, table->wid, table->ht, "4 byte table");
    printf("\n");
}

void
setUserTable(void)
{
    static float table[] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f};
    static pfTexSubloadCostTable usertable;
    usertable.wid = 2;
    usertable.ht = 4;
    usertable.alignS = 2;
    usertable.alignT = 2;
    usertable.alignR = 1;
    usertable.cost1BPT = 0;
    usertable.cost2BPT = table;
    usertable.cost3BPT = 0;
    usertable.cost4BPT = table;

    pfPerf(PFQPERF_USER_TEXLOAD_TABLE, &usertable);
}


int main (int argc, char **argv)
{
    int ret;

    /* set user defined cost table; must currently be done before pfInit() */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Set a user-defined cost table");
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "Must use static or malloced values");
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "Must be set before pfInit");
    setUserTable();

    /* Initialize Performer */
    pfInit();

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,"Mach String: %s", pfGetMachString());

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "User-Defined Cost Tables");
    pfQueryPerf(PFQPERF_USER_TEXLOAD_TABLE, &table);
    if(table)
	printTable(table);
    else
	pfNotify(PFNFY_NOTICE, PFNFY_MORE, 
		 "No User-Defined Cost Table Defined");

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Default Cost Tables");
    pfQueryPerf(PFQPERF_DEFAULT_TEXLOAD_TABLE, &table);
    printTable(table);

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Current Cost Tables");
    pfQueryPerf(PFQPERF_CUR_TEXLOAD_TABLE, &table);
    printTable(table);

    return 0;
}
