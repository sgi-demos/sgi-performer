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

/* mallocv.c
 * Written by Don Hatch (hatch@sgi.com)
 * 2/2/88
 * revised 3/16/90
 * revised again 9/27/92
 * alignment bug fixed 5/5/97
 *
 * Contains the functions {m,c,re}alloc{v,l}, {c,}free{v,l},
 * malloc{v,l}_space_needed
 *
 * Allocator for "multi-dimensional arrays" with number and values of the
 * dimensions unknown at compile time.  Actually these are multiple-indirection
 * pointers, but they are convenient because they are referenced in a C
 * program like multidimensional arrays (but faster!).
 *
 * Makes a single call to malloc.  Returns a void *, which should be typecast
 * into the desired pointer type.  The pointer returned can be freed using the
 * corresponding "free" function; they are all the same.
 * Do NOT use free!  (This is different from previous versions).
 *
 * The caller may rearrange the pointers (using qsort, for example).
 *
 * The number of dimensions is limited to 128 for mallocl, and is
 * unlimited for mallocv.
 *
 * The total number of bytes needed to be malloced for given dimensions
 * can be obtained by calling mallocv_space_needed or mallocl_space_needed,
 * using the same arguments as for mallocv or mallocl respectively.
 *
 * If reallocv is passed a NULL pointer, it simply calls mallocv;
 * if the argument is not NULL, the number of dimensions and element size
 * of the old array must match the ones being asked for; all array
 * positions that exist simultaneously in the old and new arrays
 * will be copied.
 *
Usage:

 To get something that acts like
	struct foo ptr[length][width][height][depth];

 use the following:

	struct foo ****ptr;
	ptr = (struct foo ****)
	       mallocl(sizeof(struct foo), 4, length, width, height, depth);

 or equivalently:

	int dims[] = {length, width, height, depth};
	struct foo ****ptr;
	ptr = (struct foo ****) mallocv(sizeof(struct foo), 4, dims);

 Error checking would then be done as follows:
	if (!ptr) {
	    fprintf(stderr, "malloc %d:",
			mallocv_space_needed(sizeof(struct foo), 4, dims));
	    perror("");
	}

 */

#include "mallocv.h"
#include <stdlib.h>
#include <errno.h>
#undef NDEBUG
#include <assert.h>
#include <stdarg.h>
#ifndef __linux__
#include <bstring.h>
#endif

#ifndef MALLOCL_MAXDIMS
#define MALLOCL_MAXDIMS 128
#endif /* MALLOCL_MAXDIMS */


#define MIN(a,b) ((a)<(b)?(a):(b))
#define ROUNDUP(a,b) (((a)+(b)-1)/(b)*(b))

/*
 * The "private" pointer is the pointer actually returned
 * from malloc.  The "public" pointer is what is returned to
 * the caller; eltsiz, ndims, and dims are stored just before the
 * beginning of public.
 */
#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT 8 /*  "suitable for any purpose" */
#endif
#define eltsiz_of(public)	(((int *)(public))[-1])
#define ndims_of(public)	(((int *)(public))[-2])

#define dims_of(public)		((int *)(public) - 2 - ndims_of(public))
#define private_of(public)	((void *)((char *)public \
		- ROUNDUP((ndims_of(public)+2)*sizeof(int), MALLOC_ALIGNMENT)))
    /* don't try to be clever and use different alignments for the pointers
       and the data-- it would get really messy in the case that there
       are no pointers (i.e. ndims=1) */

static void
pointat(void **ptr_array, void **array, size_t pointed_at_size, int n)
{
    int i;
    for (i = 0; i < n; ++i)
	ptr_array[i] = (char *)array + i * pointed_at_size;
}

static void
point_everything(void **pp, size_t eltsiz, int ndims, int dims[])
{
    int nptrs = 1;
    if (ndims <= 1)
	return;		/* single array, no pointers needed */
    while (ndims >= 3) {
	nptrs *= dims[0];
	pointat(pp, pp + nptrs, sizeof(void *) * dims[1], nptrs);
	pp += nptrs;
	dims++;
	ndims--;
    }
    nptrs *= dims[0];
    pointat(pp, (void **)ROUNDUP((size_t)(pp + nptrs), MALLOC_ALIGNMENT),
	    eltsiz * dims[1], nptrs);
}

/*
 * Core functions: mallocv, mallocv_space_needed, freev
 */

extern void *
mallocv(size_t eltsiz, int ndims, int dims[])
{
    void *_private, *_public;
    int n;

    if (ndims <= 0) {
	errno = EINVAL;
	return 0;
    }

    _private = malloc(mallocv_space_needed(eltsiz, ndims, dims));
    if (_private) {
	assert((size_t)_private % MALLOC_ALIGNMENT == 0);
	_public = (char *)_private + ROUNDUP((ndims+2)*sizeof(int),
					     MALLOC_ALIGNMENT);
	eltsiz_of(_public) = eltsiz;
	ndims_of(_public) = ndims;
	assert(private_of(_public) == _private);
	for (n = 0; n < ndims; ++n)
	    dims_of(_public)[n] = dims[n];

        point_everything((void **)_public, eltsiz, ndims, dims);
	return _public;
    } else
	return (void *)0;
}

static int
nptrs_needed(int ndims, int dims[])
{
    return ndims > 1 ? (dims[0] + nptrs_needed(ndims-1,dims+1) * dims[0]) : 0;
}

static int
ndata_needed(int ndims, int dims[])
{
    return ndims > 0 ? dims[0] * ndata_needed(ndims-1,dims+1) : 1;
}

extern ssize_t
mallocv_space_needed(size_t eltsiz, int ndims, int dims[])
{
    int sum;
    if (ndims <= 0) {
	errno = EINVAL;
	return -1;
    }

    sum = (ndims+2)*sizeof(int);
    sum = ROUNDUP(sum, MALLOC_ALIGNMENT);
    sum += nptrs_needed(ndims, dims) * sizeof(void *);
    sum = ROUNDUP(sum, MALLOC_ALIGNMENT);
    sum += ndata_needed(ndims, dims) * eltsiz;
    return sum;
}

extern void
freev(void *p)
{
    if (p)
	(void) free(private_of(p));
}


/*
 * Convenience functions: callocv, cfreev, reallocv
 */

extern void *
callocv(size_t eltsiz, int ndims, int dims[])
{
    int n;
    void *data;
    void *p = mallocv(eltsiz, ndims, dims);
    if (p) {
	data = p;
	for (n = 0; n < ndims-1; ++n)
	    data = *(void **)data;
	bzero(data, ndata_needed(ndims, dims) * eltsiz);
    }
    return p;
}

extern void
cfreev(void *p)
{
    freev(p);
}

static void
copy_data(void *p, void *q, size_t eltsiz, int ndims, int p_dims[], int q_dims[])
{
    int dim0 = MIN(p_dims[0], q_dims[0]), n;

    if (ndims == 1)
	bcopy(p, q, dim0 * eltsiz);
    else
	for (n = 0; n < dim0; ++n)
	    copy_data(((void **)p)[n],
		      ((void **)q)[n], eltsiz, ndims-1, p_dims+1, q_dims+1);
}

extern void *
reallocv(void *p, size_t eltsiz, int ndims, int dims[])
{
    void *q;

    if (!p)
	return mallocv(eltsiz, ndims, dims);

    if (eltsiz != eltsiz_of(p)
     || ndims != ndims_of(p)) {
	errno = EINVAL;
	return 0;
    }
    q = mallocv(eltsiz, ndims, dims);
    if (q) {
	copy_data(p, q, eltsiz, ndims, dims_of(p), dims_of(q));
	freev(p);
    }
    return q;
}


/*
 * Varargs interface: mallocl, mallocl_space_needed, callocl, reallocl,
 *		      freel, cfreel
 */

extern void *
mallocl(size_t eltsiz, int ndims, ...)
{
    va_list ap;
    int dims[MALLOCL_MAXDIMS];
    int n = 0;

    va_start(ap, ndims);
    if (ndims <= MALLOCL_MAXDIMS)
	for (n = 0; n < ndims; ++n)
	    dims[n] = va_arg(ap, int);
    va_end(ap);

    if (ndims <= MALLOCL_MAXDIMS)
	return mallocv(eltsiz, ndims, dims);
    else {
	errno = EINVAL;
	return 0;
    }
}

extern ssize_t
mallocl_space_needed(size_t eltsiz, int ndims, ...)
{
    va_list ap;
    int dims[MALLOCL_MAXDIMS];
    int n = 0;

    va_start(ap, ndims);
    if (ndims <= MALLOCL_MAXDIMS)
	for (n = 0; n < ndims; ++n)
	    dims[n] = va_arg(ap, int);
    va_end(ap);

    if (ndims <= MALLOCL_MAXDIMS)
	return mallocv_space_needed(eltsiz, ndims, dims);
    else {
	errno = EINVAL;
	return -1;
    }
}

extern void *
callocl(size_t eltsiz, int ndims, ...)
{
    va_list ap;
    int dims[MALLOCL_MAXDIMS];
    int n = 0;

    va_start(ap, ndims);
    if (ndims <= MALLOCL_MAXDIMS)
	for (n = 0; n < ndims; ++n)
	    dims[n] = va_arg(ap, int);
    va_end(ap);

    if (ndims <= MALLOCL_MAXDIMS)
	return callocv(eltsiz, ndims, dims);
    else {
	errno = EINVAL;
	return 0;
    }
}

extern void *
reallocl(void *p, size_t eltsiz, int ndims, ...)
{
    va_list ap;
    int dims[MALLOCL_MAXDIMS];
    int n = 0;

    va_start(ap, ndims);
    if (ndims <= MALLOCL_MAXDIMS)
	for (n = 0; n < ndims; ++n)
	    dims[n] = va_arg(ap, int);
    va_end(ap);

    if (ndims <= MALLOCL_MAXDIMS)
	return reallocv(p, eltsiz, ndims, dims);
    else {
	errno = EINVAL;
	return 0;
    }
}

extern void
freel(void *p)
{
    freev(p);
}

extern void
cfreel(void *p)
{
    cfreev(p);
}


#ifdef MAIN
/* test program */

#include <stdio.h>

#define EXPAND(X) X[0],X[1],X[2],X[3],X[4],X[5],X[6],X[7],X[8],X[9], \
	X[10],X[11],X[12],X[13],X[14],X[15],X[16],X[17],X[18],X[19], \
	X[20],X[21],X[22],X[23],X[24],X[25],X[26],X[27],X[28],X[29], \
	X[30],X[31],X[32],X[33],X[34],X[35],X[36],X[37],X[38],X[39], \
	X[40],X[41],X[42],X[43],X[44],X[45],X[46],X[47],X[48],X[49], \
	X[50],X[51],X[52],X[53],X[54],X[55],X[56],X[57],X[58],X[59], \
	X[60],X[61],X[62],X[63],X[64],X[65],X[66],X[67],X[68],X[69], \
	X[70],X[71],X[72],X[73],X[74],X[75],X[76],X[77],X[78],X[79], \
	X[80],X[81],X[82],X[83],X[84],X[85],X[86],X[87],X[88],X[89], \
	X[90],X[91],X[92],X[93],X[94],X[95],X[96],X[97],X[98],X[99], \
	X[100],X[101],X[102],X[103],X[104],X[105],X[106],X[107],X[108],X[109], \
	X[110],X[111],X[112],X[113],X[114],X[115],X[116],X[117],X[118],X[119], \
	X[120],X[121],X[122],X[123],X[124],X[125],X[126],X[127],X[128],X[129]
void printout(void *p, size_t eltsiz, int ndims, int dims[]);

main(int argc, char **argv)
{

    size_t eltsiz;
    int dims[130], ndims = 0, i;
    void *p;

    if (argc < 2)
	printf("Usage: %s eltsiz dim1 ... dimn\n", argv[0]), exit(1);
    eltsiz = (size_t)atoi(*++argv);
    while (*++argv)
	dims[ndims++] = atoi(*argv);
    
    printf("eltsiz = %d\n", (int)eltsiz);
    printf("dims = ");
    for (i = 0; i < ndims; ++i)
	printf("%d ", dims[i]);
    printf("\n\n");

    printf("Mallocv:\n");
    {
    int size = mallocv_space_needed(eltsiz, ndims, dims);
    int nptrs = nptrs_needed(ndims, dims);

    printf("Number of pointers = %d\n", nptrs);
    printf("Number of items = %d\n", ndata_needed(ndims, dims));
    printf("Total malloced size = %d (unsigned %u)\n", size, size);
    }
    p = mallocv(eltsiz, ndims, dims);
    printout(p, eltsiz, ndims, dims);
    freev(p);
    perror("Errors");

    printf("\n");

    printf("Mallocl:\n");
    {
    int size = mallocl_space_needed(eltsiz, ndims, EXPAND(dims));
    int nptrs = nptrs_needed(ndims, dims);

    printf("Number of pointers = %d\n", nptrs);
    printf("Number of items = %d\n", ndata_needed(ndims, dims));
    printf("Total malloced size = %d (unsigned %u)\n", size, size);
    }
    p = mallocl(eltsiz, ndims, EXPAND(dims));
    printout(p, eltsiz, ndims, dims);
    freel(p);
    perror("Errors");

    return 0;
}

void printout(void *p, size_t eltsiz, int ndims, int dims[])
{
    int *ip = (int *)p;
    int i;
    int nptrs = nptrs_needed(ndims, dims);

    if (!p) {
	printf("zilch.\n");
	return;
    }

    for (i = 0; i < nptrs; ++i)
	printf("%4d:", i*4);
    puts("");
    for (i = 0; i < nptrs + 1; ++i)	/* intentionally go one step too far */
	printf("%4d ", ip[i] - (int)p);
    puts("");
}
#endif /* MAIN */
