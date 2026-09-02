/*
 * Copyright 1996, Silicon Graphics, Inc.
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

/*
 *  file: topfi.c		$Revision: 1.4 $
 *  --------------
 *
 *  utility for converting .rgb image files to .pfi image files.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h>
#endif
#undef NDEBUG /* assert used for error checking */
#include <assert.h>

#include <gl/image.h>
#include <pfpfi.h>

#define SWAP(a,b,temp) ((temp)=(a),(a)=(b),(b)=(temp))

unsigned short inbuf[8192];
int save_mipmaps;

#define DEFAULT_HALF_KERNEL {.5}
#define DEFAULT_HALF_KERNEL_SIZE 1
/* #define DEFAULT_HALF_KERNEL {.375,.125} */
/* #define DEFAULT_HALF_KERNEL_SIZE 2 */
/* #define DEFAULT_HALF_KERNEL { .48125,.05,-.03125 }; */
/* #define DEFAULT_HALF_KERNEL_SIZE 3 */

#define MAX_HALF_KERNEL_SIZE 10
static double half_kernel[MAX_HALF_KERNEL_SIZE] = DEFAULT_HALF_KERNEL;
int half_kernel_size = DEFAULT_HALF_KERNEL_SIZE;

/* XXX these need to be parameters */
int wrap_x = 0, wrap_y = 0;

/*
 *  local prototypes
 */
static void make_out_image(double ***image, unsigned char *out_image,
			   int xsize, int ysize, int num_comp);
static int parse_args(int argc, char *argv[], char ***files);
static void usage(char *prog_name);


main(int argc, char **argv)
{
    char **files;
    int num_files;
    IMAGE *in_image;
    double ***image, ***old_image, ***temp_image;
    unsigned char *out_image;
    pfi_header_t pfih;
    FILE *ofp;
    int x, y, xsize, ysize;
    int num_comp, texel_size, image_size, border;
    int i, j;

    save_mipmaps = 0;

    if ((num_files = parse_args(argc, argv, &files)) != 2)
	usage(argv[0]);

    if ((in_image = iopen(files[0], "r")) == NULL)
    {
	fprintf(stderr, "%s: can't open input file %s\n", argv[0], files[0]);
	exit(1);
    }

    if ((ofp = fopen(files[1], "wb")) == NULL)
    {
	fprintf(stderr, "%s: can't open output file %s\n", argv[0], files[1]);
	exit(2);
    }

    xsize = in_image->xsize;
    ysize = in_image->ysize;
    num_comp = in_image->zsize;
    texel_size = num_comp;
    border = (save_mipmaps ? half_kernel_size-1 : 0); /* 0 for 2x2 box filter */

    image = (double ***)mallocl(sizeof(double), 3,
				num_comp,
				ysize + 2*border,
				xsize + 2*border);
    assert(image != NULL);
    for (i = 0; i < num_comp; ++i)
    {
	for (y = 0; y < ysize+2*border; ++y)
	    image[i][y] += border;
	image[i] += border;
    }
    if (save_mipmaps)
    {
	/* need to double buffer... will shrink in x first */
	old_image = (double ***)mallocl(sizeof(double), 3,
				        num_comp,
					ysize + 2*border,
				        (xsize+1)/2 + 2*border);
	assert(old_image != NULL);
	for (i = 0; i < num_comp; ++i)
	{
	    for (y = 0; y < ysize+2*border; ++y)
		old_image[i][y] += border;
	    old_image[i] += border;
	}
    }

    /*
     *  load the old .rgb image
     */
    for(y = 0; y < ysize; y++)
    {
	short r, g, b, a, i;

	for (i = 0; i < num_comp; i++)
	{
	    getrow(in_image, inbuf, y, i);
	    for (x = 0; x < xsize; x++)
		image[i][y][x] = inbuf[x];
	}
    }

    iclose(in_image);

    /*
     *  Store the header of the new .pfi image
     */
    pfih.magic = PFI_MAGIC_NUMBER;
    pfih.version = PFI_VERSION_NUMBER;
    pfih.data_start = sizeof(pfi_header_t);
    pfih.data_size = xsize + ysize * num_comp;
    pfih.size[0] = xsize;
    pfih.size[1] = ysize;
    pfih.size[2] = 1;
    pfih.num_comp = num_comp;
    pfih.format = PFI_FORMAT_UINT_8;
    pfih.packing = PFI_PACK_DEFAULT;
    pfih.gl = PFI_OPENGL;
    pfih.num_images = 1;
    pfih.mipmaps = save_mipmaps;

    fwrite(&pfih, sizeof(pfi_header_t), 1, ofp);

    /*
     *  Store the base image of the new .pfi image
     */
    image_size = xsize * ysize * texel_size;
    if (image_size & 0x3)
	image_size += 0x4 - (image_size & 0x3);
    out_image = (unsigned char *)malloc(image_size);
    assert(out_image != NULL);
    make_out_image(image, out_image, xsize, ysize, num_comp);
    fwrite(out_image, 1, image_size, ofp);

    /*
     *  Store the mipmap images of the new .pfi image
     */
    if (save_mipmaps)
    {
	int xs, ys, old_x, old_y;

	printf("Mipmap generation kernel:\n");
	for (i = -half_kernel_size; i < half_kernel_size; ++i)
	{
	    for (j = -half_kernel_size; j < half_kernel_size; ++j)
		    printf("    %f", half_kernel[i<0 ? -1-i : i]
				   * half_kernel[j<0 ? -1-j : j]);
	    printf("\n");
	}

	xs = xsize;
	ys = ysize;

	while (xs > 1 || ys > 1)
	{
	    /*
	     * Shrink the image.
	     * Shrink in x first so that the (probably) more expensive
	     * y shrink will be on a smaller image.
	     */
	    if (xs > 1)
	    {
		/*
		 * Add border of width half_kernel_size-1
		 * to simplify big convolve loop
		 * (note that the wrapping works even when the kernel
		 * is bigger than the image).
		 */
		for (i = 0; i < num_comp; i++)
		    for (y = 0; y < ys; ++y)
			for (x = 0; x < half_kernel_size-1; ++x)
			{
			    image[i][y][xs+x] = image[i][y][wrap_x ? x : xs-1];
			    image[i][y][-1-x] = image[i][y][wrap_x ? xs-1-x : 0];
			}

		xs >>= 1;	/* xs is now the x size of the shrunken image */
		SWAP(image, old_image, temp_image);

		for (i = 0; i < num_comp; i++)
		    for(y = 0; y < ys; y++)
			for (x = 0, old_x = 0; x < xs; x++, old_x += 2)
			{
			    double sum = 0.;
			    for (j = 0; j < half_kernel_size; ++j)
			    {
				sum += half_kernel[j]
				     * (old_image[i][y][old_x  -j]
				      + old_image[i][y][old_x+1+j]);
			    }
			    image[i][y][x] = sum;
			}
	    }
	    if (ys > 1)
	    {
		/*
		 * Add border of height half_kernel_size-1
		 * to simplify big convolve loop
		 * (note that the wrapping works even when the kernel
		 * is bigger than the image).
		 */
		for (i = 0; i < num_comp; i++)
		    for (y = 0; y < half_kernel_size-1; ++y)
			for (x = 0; x < xs; ++x)
			{
			    image[i][ys+y][x] = image[i][wrap_y ? y : ys-1][x];
			    image[i][-1-y][x] = image[i][wrap_y ? ys-1-y : 0][x];
			}

		ys >>= 1;	/* ys is now the y size of the shrunken image */
		SWAP(image, old_image, temp_image);

		for (i = 0; i < num_comp; i++)
		    for(y = 0, old_y = 0; y < ys; y++, old_y += 2)
			for (x = 0; x < xs; x++)
			{
			    double sum = 0.;
			    for (j = 0; j < half_kernel_size; ++j)
			    {
				sum += half_kernel[j]
				     * (old_image[i][old_y  -j][x]
				      + old_image[i][old_y+1+j][x]);
			    }
			    image[i][y][x] = sum;
			}
	    }

	    /*
	     *  Store a mipmap image of the new .pfi image
	     */
	    image_size = xs * ys * texel_size;
	    if (image_size & 0x3)
		image_size += 0x4 - (image_size & 0x3);
	    make_out_image(image, out_image, xs, ys, num_comp);
	    fwrite(out_image, 1, image_size, ofp);
	}
    }

    fclose(ofp);

    free(out_image);

    freel(image);
    if (save_mipmaps)
	freel(old_image);
}


/*
 *  Make an output packed version of the image.
 */
static void make_out_image(double ***image, unsigned char *out_image,
			   int xsize, int ysize, int num_comp)
{
    int x, y;

    switch (num_comp)
    {
	case 1:
	    for(y = 0; y < ysize; y++)
		for (x = 0; x < xsize; x++)
		{
		    out_image[0] = PF_CLAMP((int)(image[0][y][x] + .5), 0, 255);
		    out_image++;
		}
	    break;
	case 2:
	    for(y = 0; y < ysize; y++)
		for (x = 0; x < xsize; x++)
		{
		    out_image[0] = PF_CLAMP((int)(image[0][y][x] + .5), 0, 255);
		    out_image[1] = PF_CLAMP((int)(image[1][y][x] + .5), 0, 255);
		    out_image += 2;
		}
	    break;
	case 3:
	    for(y = 0; y < ysize; y++)
		for (x = 0; x < xsize; x++)
		{
		    out_image[0] = PF_CLAMP((int)(image[0][y][x] + .5), 0, 255);
		    out_image[1] = PF_CLAMP((int)(image[1][y][x] + .5), 0, 255);
		    out_image[2] = PF_CLAMP((int)(image[2][y][x] + .5), 0, 255);
		    out_image += 3;
		}
	    break;
	case 4:
	    for(y = 0; y < ysize; y++)
		for (x = 0; x < xsize; x++)
		{
		    out_image[0] = PF_CLAMP((int)(image[0][y][x] + .5), 0, 255);
		    out_image[1] = PF_CLAMP((int)(image[1][y][x] + .5), 0, 255);
		    out_image[2] = PF_CLAMP((int)(image[2][y][x] + .5), 0, 255);
		    out_image[3] = PF_CLAMP((int)(image[3][y][x] + .5), 0, 255);
		    out_image += 4;
		}
	    break;
    }
}


/*
 *  parse commnand line arguments
 */
static int parse_args(int argc, char *argv[], char ***files)
{
    int opt;

    /*
     *  process command-line arguments
     */
    while ((opt = getopt(argc, argv, "mk:")) != -1)
    {
	switch (opt)
	{
	    case 'm':			/* save mipmaps */
		save_mipmaps = 1;
		break;
	    case 'k':		/* right half of symmetric separable kernel */
		assert(10 <= MAX_HALF_KERNEL_SIZE);
		half_kernel_size = sscanf(optarg,
			"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
			&half_kernel[0],
			&half_kernel[1],
			&half_kernel[2],
			&half_kernel[3],
			&half_kernel[4],
			&half_kernel[5],
			&half_kernel[6],
			&half_kernel[7],
			&half_kernel[8],
			&half_kernel[9]);
		if (half_kernel_size < 1)
		    usage(argv[0]);
		break;
	    default:
		usage(argv[0]);
	}
    }

    *files = &(argv[optind]);   /* return address of filename list */

    return(argc - optind);      /* number of files */
}


/*
 *  print usage
 */
static void usage(char *prog_name)
{
    char *base_name;
    static const char *help_list =
	"usage:\n"
	"       %s [-m [-k <right half of symmetric separable convolution kernel>]]\n"
	"               inimage.rgb outimage.pfi\n";

    /*
     *  just use program's base name: /usr/fred/base -> base
     */
    if (prog_name == NULL)
	base_name = "UNKNOWN";
    else if ((base_name = strrchr(prog_name, '/')) == NULL)
	base_name = prog_name;
    else
	base_name++;

    /*
     *  print list of command-line options and arguments
     */
    fprintf(stderr, help_list, base_name);
    fflush(stderr);
    exit(1);
}
