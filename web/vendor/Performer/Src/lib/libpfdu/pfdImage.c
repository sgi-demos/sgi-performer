/*
 * Copyright 1997, Silicon Graphics, Inc.
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
 * file: pfdImage.c
 * ----------------
 *
 * $Revision: 1.27 $
 * $Date: 2002/12/09 04:17:56 $
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <ctype.h>
#ifndef WIN32
#include <getopt.h>
#endif
#undef NDEBUG /* assert used for error checking */
#include <assert.h>

#ifdef WIN32
#include "opengl.h"
#endif
#include <Performer/image.h>
#include <Performer/pfdu.h>
#include <Performer/pfpfi.h>
/* #include <Performer/pr.h> */

#ifdef __linux__
#include <asm/byteorder.h>
#endif

#ifdef __LITTLE_ENDIAN
#ifndef P_32_SWAP
#define	P_32_SWAP(a) {							\
	uint _tmp = *(uint *)a;				\
	((char *)a)[0] = ((char *)&_tmp)[3];				\
	((char *)a)[1] = ((char *)&_tmp)[2];				\
	((char *)a)[2] = ((char *)&_tmp)[1];				\
	((char *)a)[3] = ((char *)&_tmp)[0];				\
}
#endif  /* P_32_SWAP */

static
void swap_pfi_header(pfi_header_t *ptr)
{
	P_32_SWAP(&ptr->magic);
	P_32_SWAP(&ptr->version);
	P_32_SWAP(&ptr->data_start);
	P_32_SWAP(&ptr->data_size);
	P_32_SWAP(&ptr->size[0]);
	P_32_SWAP(&ptr->size[1]);
	P_32_SWAP(&ptr->size[2]);
	P_32_SWAP(&ptr->num_comp);
	P_32_SWAP(&ptr->format);
	P_32_SWAP(&ptr->gl);
	P_32_SWAP(&ptr->mipmaps);
	P_32_SWAP(&ptr->num_images);
	P_32_SWAP(&ptr->alignment);
}

#endif /* __LITTLE_ENDIAN */

/*
 *  local prototypes
 */
static int get_num_levels(pfdImage *_image);
static int get_num_images(pfdImage *_image);
static double ****malloc_image(int xsize, int ysize, int zsize, int num_comp,
			       int xborder, int yborder, int zborder);
static void copy_image(double ****dst, double ****src,
		       int xsize, int ysize, int zsize, int num_comp);
static void make_border(double ****image,
			int xsize, int ysize, int zsize, int num_comp,
			int xborder, int yborder, int zborder, int *wrap);

/*
 *  external prototypes
 */
extern PFDUDLLEXPORT uint *_pr_flip_components(uint *image, int size, int num_comp,
				 int comp_size, int row_size);


PFDUDLLEXPORT pfdImage *pfdLoadImage(const char *_fileName)
{
    char *ext;

    /*
     *  If the name ends in .pfi load it as a .pfi image file.
     */
    if ((ext = strrchr(_fileName, '.')) && strcmp(ext, ".pfi") == 0)
	return pfdLoadImage_pfi(_fileName);

    /*
     *  else load it as a sgi image file.
     */
    return pfdLoadImage_sgi(_fileName);
}


PFDUDLLEXPORT pfdImage *pfdLoadImage_sgi(const char *_fileName)
{
    IMAGE *in_image;
    pfdImage *image;
    int x, y, c;
    unsigned short inbuf[8192];
    char path[PF_MAXSTRING];

    /*
     *  Find the file
     */
    if (!pfFindFile(_fileName, path, R_OK))
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadImage() - could not find image file %s", _fileName);
	return NULL;
    }

    if ((in_image = iopen(path, "r")) == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadImage() - could not open image file %s", path);
	return NULL;
    }

    image = (pfdImage *)malloc(sizeof(pfdImage));
    bzero(image, sizeof(pfdImage));

    image->xsize = in_image->xsize;
    image->ysize = in_image->ysize;
    image->zsize = 1;
    image->num_comp = in_image->zsize;

    image->image = malloc_image(image->xsize, image->ysize, image->zsize,
				image->num_comp, 0, 0, 0);

    /*
     *  load the old .rgb image
     */
    for (c = 0; c < image->num_comp; c++)
    {
	for (y = 0; y < image->ysize; y++)
	{
	    getrow(in_image, inbuf, y, c);
	    for (x = 0; x < image->xsize; x++)
		image->image[c][0][y][x] = inbuf[x] / 255.0;
	}
    }

    iclose(in_image);

    return(image);
}


PFDUDLLEXPORT pfdImage *pfdLoadImage_pfi(const char *_fileName)
{
    /* XXX */
    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	     "pfdLoadImage_pfi:  Not implemented yet. \"%s\" not loaded.",
	     _fileName);

    return(NULL);
}


PFDUDLLEXPORT void pfdDelImage(pfdImage *_image)
{
    if (_image->mipmaps)
	pfdImageDelMipmaps(_image);

    if (_image->image)
	free(_image->image);

    if (_image->packed)
	free(_image->packed);

    free(_image);
}


PFDUDLLEXPORT void pfdStoreImage(pfdImage *_image, const char *_fileName)
{
    char *ext;

    /*
     *  If the name ends in .pfi store it as a .pfi image file.
     */
    if ((ext = strrchr(_fileName, '.')) && strcmp(ext, ".pfi") == 0)
    {
	pfdStoreImage_pfi(_image, _fileName);
	return;
    }

    /*
     *  else store it as a sgi image file.
     */
    pfdStoreImage_sgi(_image, _fileName);
}


PFDUDLLEXPORT void pfdStoreImage_sgi(pfdImage *_image, const char *_fileName)
{
    /* XXX */
    _image = _image;
    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	     "pfdStoreImage_sgi:  Not implemented yet. \"%s\" not created.",
	     _fileName);
}


PFDUDLLEXPORT void pfdStoreImage_pfi(pfdImage *_image, const char *_fileName)
{
    pfi_header_t pfih;
    pfdImage *itmp;
    FILE *fp;
    int i, j, num_levels, num_images, offset;

    /*
     *  store the file
     */
    if ((fp = fopen(_fileName, "wb")) == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdStoreImage_pfi:  Could not open \"%s\" for writing.",
		 _fileName);
	return;
    }

    /*
     *  If the image has not yet been packed pack it with the default packing.
     */
    if (_image->packed == NULL)
	pfdImagePack(_image, PFTEX_UNSIGNED_BYTE);

    /*
     *  If this image has multiple images check that all images are of the
     *  same size and that all are packed.
     */
    num_images = 1;
    if (_image->next)
    {
	for (itmp = _image->next; itmp != NULL; itmp = itmp->next)
	{
	    num_images++;

	    if (itmp->xsize != _image->xsize ||
		itmp->ysize != _image->ysize ||
		itmp->zsize != _image->zsize ||
		itmp->num_comp != _image->num_comp)
	    {
		pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
			 "pfdStoreImage_pfi:  Can not store \"%s\".",
			 _fileName);
		pfNotify(PFNFY_WARN, PFNFY_MORE,
			 "All images in a pfi file must be the same size.");
		return;
	    }

	    if (_image->mipmaps != NULL && itmp->mipmaps == NULL)
	    {
		pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
			 "pfdStoreImage_pfi:  Can not store \"%s\".",
			 _fileName);
		pfNotify(PFNFY_WARN, PFNFY_MORE,
			 "All images in a pfi file must have or not have "
			 "mipmaps.");
		return;
	    }

	    if (itmp->format != _image->format)
		pfdImagePack(itmp, _image->format);
	}
    }

    /*
     *  Store the header of the new .pfi image
     */
    pfih.magic = PFI_MAGIC_NUMBER;
    pfih.version = PFI_VERSION_NUMBER;
    if (_image->alignment == 0 ||
	_image->alignment == sizeof(pfi_header_t))
	pfih.data_start = sizeof(pfi_header_t);
    else
	pfih.data_start = sizeof(pfi_header_t) + _image->alignment -
			  (sizeof(pfi_header_t) % _image->alignment);
    pfih.size[0] = _image->xsize;
    pfih.size[1] = _image->ysize;
    pfih.size[2] = _image->zsize;
    pfih.alignment = _image->alignment;
    switch (_image->format)
    {
	case PFTEX_BYTE:
	    pfih.format = PFI_FORMAT_INT_8;
	    pfih.num_comp = _image->num_comp;
	    break;
	case PFTEX_UNSIGNED_BYTE:
	    pfih.format = PFI_FORMAT_UINT_8;
	    pfih.num_comp = _image->num_comp;
	    break;
	case PFTEX_SHORT:
	    pfih.format = PFI_FORMAT_INT_16;
	    pfih.num_comp = _image->num_comp;
	    break;
	case PFTEX_UNSIGNED_SHORT:
	    pfih.format = PFI_FORMAT_UINT_16;
	    pfih.num_comp = _image->num_comp;
	    break;
	case PFTEX_INT:
	    pfih.format = PFI_FORMAT_INT_32;
	    pfih.num_comp = _image->num_comp;
	    break;
	case PFTEX_UNSIGNED_INT:
	    pfih.format = PFI_FORMAT_UINT_32;
	    pfih.num_comp = _image->num_comp;
	    break;
	case PFTEX_FLOAT:
	    pfih.format = PFI_FORMAT_FLOAT_32;
	    pfih.num_comp = _image->num_comp;
	    break;
	case PFTEX_UNSIGNED_BYTE_3_3_2:
	    pfih.format = PFI_FORMAT_PACK_3_3_2;
	    pfih.num_comp = 3;
	    break;
	case PFTEX_UNSIGNED_SHORT_4_4_4_4:
	    pfih.format = PFI_FORMAT_PACK_4_4_4_4;
	    pfih.num_comp = 4;
	    break;
	case PFTEX_UNSIGNED_SHORT_5_5_5_1:
	    pfih.format = PFI_FORMAT_PACK_5_5_5_1;
	    pfih.num_comp = 4;
	    break;
	case PFTEX_UNSIGNED_INT_8_8_8_8:
	    pfih.format = PFI_FORMAT_PACK_8_8_8_8;
	    pfih.num_comp = 4;
	    break;
	case PFTEX_UNSIGNED_INT_10_10_10_2:
	    pfih.format = PFI_FORMAT_PACK_10_10_10_2;
	    pfih.num_comp = 4;
	    break;
	default:
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		     "pfdStoreImage_pfi:  Image has bad format.  "
		     "Can't store \"%s\".",
		     _fileName);
	    fclose(fp);
	    return;
    }
    pfih.gl = PFI_OPENGL;
    pfih.num_images = num_images;

    if (_image->mipmaps && pfih.gl == PFI_OPENGL)
    {
	pfih.mipmaps = 1;
	pfih.data_size = _image->packed_size;
	num_levels = get_num_levels(_image);
	for (i = 0; i < num_levels; i++)
	    pfih.data_size += _image->mipmaps[i].packed_size;
    }
    else
    {
	pfih.mipmaps = 0;
	pfih.data_size = _image->packed_size;
	num_levels = 0;
    }

#ifdef __LITTLE_ENDIAN
	swap_pfi_header(&pfih);
#endif

    fwrite(&pfih, sizeof(pfi_header_t), 1, fp);

#ifdef __LITTLE_ENDIAN
	swap_pfi_header(&pfih);
#endif
    /*
     *  store the images
     */
    for (itmp = _image; itmp != NULL; itmp = itmp->next)
    {
	if (itmp == _image)
	    fseek(fp, pfih.data_start, SEEK_SET);
	else if (pfih.alignment != 0 &&
		 (offset = pfih.data_size % pfih.alignment))
	    fseek(fp, pfih.alignment - offset, SEEK_CUR);

	/*
	 *  Store the base image of the new .pfi image file
	 */
	fwrite(itmp->packed, 1, itmp->packed_size, fp);

	/*
	 *  Store the mipmap images of the new .pfi image file
	 */
	for (j = 0; j < num_levels; j++)
	    fwrite(itmp->mipmaps[j].packed, 1,
		   itmp->mipmaps[j].packed_size, fp);
    }

    fclose(fp);
}


#define PACK(d, s, tmp)  (tmp = ((d) * (s) + 0.5), (tmp <= (s))? tmp : (s))

#define SPACK(d, s, tmp) (tmp = ((d) * (s) + ((d >= 0.0)? 0.5: -0.5)),	\
			  PF_CLAMP(tmp, (-s), (s)))

PFDUDLLEXPORT void pfdImagePack(pfdImage *_image, int _format)
{
    unsigned char *uc;
    unsigned short *us;
    unsigned int ut;
    int row_size;
    int x, y, z;
    unsigned int *ui;
    signed char *sc;
    signed short *ss;
    signed int *si, st;
    float *f;

    /*
     *  Delete old packed image if present.
     */
    if (_image->packed)
	free(_image->packed);

    _image->format = _format;


    /*
     *  Opengl packing
     */
    switch (_format)
    {
	case PFTEX_BYTE:
	case PFTEX_UNSIGNED_BYTE:
	    _image->texel_size = _image->num_comp;
	    break;
	case PFTEX_SHORT:
	case PFTEX_UNSIGNED_SHORT:
	    _image->texel_size = _image->num_comp * 2;
	    break;
	case PFTEX_INT:
	case PFTEX_UNSIGNED_INT:
	case PFTEX_FLOAT:
	    _image->texel_size = _image->num_comp * 4;
	    break;
	case PFTEX_UNSIGNED_BYTE_3_3_2:
	    _image->texel_size = 1;
	    break;
	case PFTEX_UNSIGNED_SHORT_4_4_4_4:
	case PFTEX_UNSIGNED_SHORT_5_5_5_1:
	    _image->texel_size = 2;
	    break;
	case PFTEX_UNSIGNED_INT_8_8_8_8:
	case PFTEX_UNSIGNED_INT_10_10_10_2:
	    _image->texel_size = 4;
	    break;
	default:
	    pfNotify(PFNFY_WARN, PFNFY_USAGE,
		     "pfdImagePack() - unknown format %d", _format);
	    return;
    }
    row_size = _image->xsize * _image->texel_size;
    _image->packed_size = row_size * _image->ysize * _image->zsize;
    if (_image->packed_size & 0x3)
	_image->packed_size += 0x4 - (_image->packed_size & 0x3);

    _image->packed = (unsigned char *)malloc(_image->packed_size);

    switch (_format)
    {
	case PFTEX_BYTE:
	    sc = (signed char *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				sc[0] = SPACK(_image->image[0][z][y][x],
					      0x7f, st);
				sc++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				sc[0] = SPACK(_image->image[0][z][y][x],
					      0x7f, st);
				sc[1] = SPACK(_image->image[1][z][y][x],
					      0x7f, st);
				sc += 2;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				sc[0] = SPACK(_image->image[0][z][y][x],
					      0x7f, st);
				sc[1] = SPACK(_image->image[1][z][y][x],
					      0x7f, st);
				sc[2] = SPACK(_image->image[2][z][y][x],
					      0x7f, st);
				sc += 3;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				sc[0] = SPACK(_image->image[0][z][y][x],
					      0x7f, st);
				sc[1] = SPACK(_image->image[1][z][y][x],
					      0x7f, st);
				sc[2] = SPACK(_image->image[2][z][y][x],
					      0x7f, st);
				sc[3] = SPACK(_image->image[3][z][y][x],
					      0x7f, st);
				sc += 4;
			    }
		    break;
	    }
	    break;
	case PFTEX_UNSIGNED_BYTE:
	    uc = (unsigned char *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				uc[0] = PACK(_image->image[0][z][y][x],
					     0xff, ut);
				uc++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				uc[0] = PACK(_image->image[0][z][y][x],
					     0xff, ut);
				uc[1] = PACK(_image->image[1][z][y][x],
					     0xff, ut);
				uc += 2;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				uc[0] = PACK(_image->image[0][z][y][x],
					     0xff, ut);
				uc[1] = PACK(_image->image[1][z][y][x],
					     0xff, ut);
				uc[2] = PACK(_image->image[2][z][y][x],
					     0xff, ut);
				uc += 3;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				uc[0] = PACK(_image->image[0][z][y][x],
					     0xff, ut);
				uc[1] = PACK(_image->image[1][z][y][x],
					     0xff, ut);
				uc[2] = PACK(_image->image[2][z][y][x],
					     0xff, ut);
				uc[3] = PACK(_image->image[3][z][y][x],
					     0xff, ut);
				uc += 4;
			    }
		    break;
	    }
	    break;
	case PFTEX_SHORT:
	    ss = (signed short *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ss[0] = SPACK(_image->image[0][z][y][x],
					      0x7fff, st);
				ss++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ss[0] = SPACK(_image->image[0][z][y][x],
					      0x7fff, st);
				ss[1] = SPACK(_image->image[1][z][y][x],
					      0x7fff, st);
				ss += 2;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ss[0] = SPACK(_image->image[0][z][y][x],
					      0x7fff, st);
				ss[1] = SPACK(_image->image[1][z][y][x],
					      0x7fff, st);
				ss[2] = SPACK(_image->image[2][z][y][x],
					      0x7fff, st);
				ss += 3;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ss[0] = SPACK(_image->image[0][z][y][x],
					      0x7fff, st);
				ss[1] = SPACK(_image->image[1][z][y][x],
					      0x7fff, st);
				ss[2] = SPACK(_image->image[2][z][y][x],
					      0x7fff, st);
				ss[3] = SPACK(_image->image[3][z][y][x],
					      0x7fff, st);
				ss += 4;
			    }
		    break;
	    }
	    break;
	case PFTEX_UNSIGNED_SHORT:
	    us = (unsigned short *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = PACK(_image->image[0][z][y][x],
					     0xffff, ut);
				us++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = PACK(_image->image[0][z][y][x],
					     0xffff, ut);
				us[1] = PACK(_image->image[1][z][y][x],
					     0xffff, ut);
				us += 2;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = PACK(_image->image[0][z][y][x],
					     0xffff, ut);
				us[1] = PACK(_image->image[1][z][y][x],
					     0xffff, ut);
				us[2] = PACK(_image->image[2][z][y][x],
					     0xffff, ut);
				us += 3;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = PACK(_image->image[0][z][y][x],
					     0xffff, ut);
				us[1] = PACK(_image->image[1][z][y][x],
					     0xffff, ut);
				us[2] = PACK(_image->image[2][z][y][x],
					     0xffff, ut);
				us[3] = PACK(_image->image[3][z][y][x],
					     0xffff, ut);
				us += 4;
			    }
		    break;
	    }
	    break;
	case PFTEX_INT:
	    si = (signed int *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				si[0] = SPACK(_image->image[0][z][y][x],
					      0x7fffffff, st);
				si++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				si[0] = SPACK(_image->image[0][z][y][x],
					      0x7fffffff, st);
				si[1] = SPACK(_image->image[1][z][y][x],
					      0x7fffffff, st);
				si += 2;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				si[0] = SPACK(_image->image[0][z][y][x],
					      0x7fffffff, st);
				si[1] = SPACK(_image->image[1][z][y][x],
					      0x7fffffff, st);
				si[2] = SPACK(_image->image[2][z][y][x],
					      0x7fffffff, st);
				si += 3;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				si[0] = SPACK(_image->image[0][z][y][x],
					      0x7fffffff, st);
				si[1] = SPACK(_image->image[1][z][y][x],
					      0x7fffffff, st);
				si[2] = SPACK(_image->image[2][z][y][x],
					      0x7fffffff, st);
				si[3] = SPACK(_image->image[3][z][y][x],
					      0x7fffffff, st);
				ss += 4;
			    }
		    break;
	    }
	    break;
	case PFTEX_UNSIGNED_INT:
	    ui = (unsigned int *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = PACK(_image->image[0][z][y][x],
					     0xffffffff, ut);
				ui++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = PACK(_image->image[0][z][y][x],
					     0xffffffff, ut);
				ui[1] = PACK(_image->image[1][z][y][x],
					     0xffffffff, ut);
				ui += 2;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = PACK(_image->image[0][z][y][x],
					     0xffffffff, ut);
				ui[1] = PACK(_image->image[1][z][y][x],
					     0xffffffff, ut);
				ui[2] = PACK(_image->image[2][z][y][x],
					     0xffffffff, ut);
				ui += 3;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = PACK(_image->image[0][z][y][x],
					     0xffffffff, ut);
				ui[1] = PACK(_image->image[1][z][y][x],
					     0xffffffff, ut);
				ui[2] = PACK(_image->image[2][z][y][x],
					     0xffffffff, ut);
				ui[3] = PACK(_image->image[3][z][y][x],
					     0xffffffff, ut);
				ui += 4;
			    }
		    break;
	    }
	    break;
	case PFTEX_FLOAT:
	    f = (float *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				f[0] = _image->image[0][z][y][x];
				f++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				f[0] = _image->image[0][z][y][x];
				f[1] = _image->image[1][z][y][x];
				f += 2;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				f[0] = _image->image[0][z][y][x];
				f[1] = _image->image[1][z][y][x];
				f[2] = _image->image[2][z][y][x];
				f += 3;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				f[0] = _image->image[0][z][y][x];
				f[1] = _image->image[1][z][y][x];
				f[2] = _image->image[2][z][y][x];
				f[3] = _image->image[3][z][y][x];
				f += 4;
			    }
		    break;
	    }
	    break;
	case PFTEX_UNSIGNED_BYTE_3_3_2:
	    uc = (unsigned char *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				uc[0] = (PACK(_image->image[0][z][y][x],
					      0x7, ut) << 5) |
					(PACK(_image->image[0][z][y][x],
					      0x7, ut) << 2) |
					(PACK(_image->image[0][z][y][x],
					      0x3, ut));
				uc++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				uc[0] = (PACK(_image->image[0][z][y][x],
					      0x7, ut) << 5) |
					(PACK(_image->image[0][z][y][x],
					      0x7, ut) << 2) |
					(PACK(_image->image[0][z][y][x],
					      0x3, ut));
				uc++;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				uc[0] = (PACK(_image->image[0][z][y][x],
					      0x7, ut) << 5) |
					(PACK(_image->image[1][z][y][x],
					      0x7, ut) << 2) |
					(PACK(_image->image[2][z][y][x],
					      0x3, ut));
				uc++;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				uc[0] = (PACK(_image->image[0][z][y][x],
					      0x7, ut) << 5) |
					(PACK(_image->image[1][z][y][x],
					      0x7, ut) << 2) |
					(PACK(_image->image[2][z][y][x],
					      0x3, ut));
				uc++;
			    }
		    break;
	    }
	    break;
	case PFTEX_UNSIGNED_SHORT_4_4_4_4:
	    us = (unsigned short *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = (PACK(_image->image[0][z][y][x],
					      0xf, ut) << 12) |
					(PACK(_image->image[0][z][y][x],
					      0xf, ut) << 8) |
					(PACK(_image->image[0][z][y][x],
					      0xf, ut) << 4) |
					0xf;
				us++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = (PACK(_image->image[0][z][y][x],
					      0xf, ut) << 12) |
					(PACK(_image->image[0][z][y][x],
					      0xf, ut) << 8) |
					(PACK(_image->image[0][z][y][x],
					      0xf, ut) << 4) |
					(PACK(_image->image[1][z][y][x],
					      0xf, ut));
				us++;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = (PACK(_image->image[0][z][y][x],
					      0xf, ut) << 12) |
					(PACK(_image->image[1][z][y][x],
					      0xf, ut) << 8) |
					(PACK(_image->image[2][z][y][x],
					      0xf, ut) << 4) |
					0xf;
				us++;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = (PACK(_image->image[0][z][y][x],
					      0xf, ut) << 12) |
					(PACK(_image->image[1][z][y][x],
					      0xf, ut) << 8) |
					(PACK(_image->image[2][z][y][x],
					      0xf, ut) << 4) |
					(PACK(_image->image[3][z][y][x],
					      0xf, ut));
				us++;
			    }
		    break;
	    }
	    break;
	case PFTEX_UNSIGNED_SHORT_5_5_5_1:
	    us = (unsigned short *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = (PACK(_image->image[0][z][y][x],
					      0x1f, ut) << 11) |
					(PACK(_image->image[0][z][y][x],
					      0x1f, ut) << 6) |
					(PACK(_image->image[0][z][y][x],
					      0x1f, ut) << 1) |
					0x1;
				us++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = (PACK(_image->image[0][z][y][x],
					      0x1f, ut) << 11) |
					(PACK(_image->image[0][z][y][x],
					      0x1f, ut) << 6) |
					(PACK(_image->image[0][z][y][x],
					      0x1f, ut) << 1) |
					(PACK(_image->image[1][z][y][x],
					      0x1, ut));
				us++;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = (PACK(_image->image[0][z][y][x],
					      0x1f, ut) << 11) |
					(PACK(_image->image[1][z][y][x],
					      0x1f, ut) << 6) |
					(PACK(_image->image[2][z][y][x],
					      0x1f, ut) << 1) |
					0x1;
				us++;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				us[0] = (PACK(_image->image[0][z][y][x],
					      0x1f, ut) << 11) |
					(PACK(_image->image[1][z][y][x],
					      0x1f, ut) << 6) |
					(PACK(_image->image[2][z][y][x],
					      0x1f, ut) << 1) |
					(PACK(_image->image[3][z][y][x],
					      0x1, ut));
				us++;
			    }
		    break;
	    }
	    break;
	case PFTEX_UNSIGNED_INT_8_8_8_8:
	    ui = (unsigned int *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = (PACK(_image->image[0][z][y][x],
					      0xff, ut) << 24) |
					(PACK(_image->image[0][z][y][x],
					      0xff, ut) << 16) |
					(PACK(_image->image[0][z][y][x],
					      0xff, ut) << 8) |
					0xff;
				ui++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = (PACK(_image->image[0][z][y][x],
					      0xff, ut) << 24) |
					(PACK(_image->image[0][z][y][x],
					      0xff, ut) << 16) |
					(PACK(_image->image[0][z][y][x],
					      0xff, ut) << 8) |
					(PACK(_image->image[1][z][y][x],
					      0xff, ut));
				ui++;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = (PACK(_image->image[0][z][y][x],
					      0xff, ut) << 24) |
					(PACK(_image->image[1][z][y][x],
					      0xff, ut) << 16) |
					(PACK(_image->image[2][z][y][x],
					      0xff, ut) << 8) |
					0xff;
				ui++;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = (PACK(_image->image[0][z][y][x],
					      0xff, ut) << 24) |
					(PACK(_image->image[1][z][y][x],
					      0xff, ut) << 16) |
					(PACK(_image->image[2][z][y][x],
					      0xff, ut) << 8) |
					(PACK(_image->image[3][z][y][x],
					      0xff, ut));
				ui++;
			    }
		    break;
	    }
	    break;
	case PFTEX_UNSIGNED_INT_10_10_10_2:
	    ui = (unsigned int *)_image->packed;
	    switch (_image->num_comp)
	    {
		case 1:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = (PACK(_image->image[0][z][y][x],
					      0x3ff, ut) << 22) |
					(PACK(_image->image[0][z][y][x],
					      0x3ff, ut) << 12) |
					(PACK(_image->image[0][z][y][x],
					      0x3ff, ut) << 2) |
					0x3;
				ui++;
			    }
		    break;
		case 2:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = (PACK(_image->image[0][z][y][x],
					      0x3ff, ut) << 22) |
					(PACK(_image->image[0][z][y][x],
					      0x3ff, ut) << 12) |
					(PACK(_image->image[0][z][y][x],
					      0x3ff, ut) << 2) |
					(PACK(_image->image[1][z][y][x],
					      0x3, ut));
				ui++;
			    }
		    break;
		case 3:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = (PACK(_image->image[0][z][y][x],
					      0x3ff, ut) << 22) |
					(PACK(_image->image[1][z][y][x],
					      0x3ff, ut) << 12) |
					(PACK(_image->image[2][z][y][x],
					      0x3ff, ut) << 2) |
					0x3;
				ui++;
			    }
		    break;
		case 4:
		    for(z = 0; z < _image->zsize; z++)
			for(y = 0; y < _image->ysize; y++)
			    for (x = 0; x < _image->xsize; x++)
			    {
				ui[0] = (PACK(_image->image[0][z][y][x],
					      0x3ff, ut) << 22) |
					(PACK(_image->image[1][z][y][x],
					      0x3ff, ut) << 12) |
					(PACK(_image->image[2][z][y][x],
					      0x3ff, ut) << 2) |
					(PACK(_image->image[3][z][y][x],
					      0x3, ut));
				ui++;
			    }
		    break;
	    }
	    break;
    }

    if (_image->mipmaps)
    {
	int i, num_levels;

	num_levels = get_num_levels(_image);

	for (i = 0; i < num_levels; i++)
	    pfdImagePack(&_image->mipmaps[i], _format);
    }

    if (_image->next)
	pfdImagePack(_image->next, _format);
}

PFDUDLLEXPORT void pfdImageFromPfTexture(pfdImage *_image, pfTexture *pftex)
{
    uint *image;
    int comp, ns, nt, nr;
    int x, y, z, c;
    unsigned char *bimage;
    unsigned char n;
    int ext;

    if (! pftex)
	return;

    if (! _image)
	return;

    ext = pfGetTexFormat( pftex, PFTEX_EXTERNAL_FORMAT );
    if (ext != PFTEX_UNSIGNED_BYTE) {
	pfNotify( PFNFY_WARN, PFNFY_WARN, "pfdImageFromPfTexture "
		  "unsupported format 0x%x\n", ext);
    }

    pfGetTexImage( pftex, &image, &comp, &ns, &nt, &nr );

    _image->xsize = ns;
    _image->ysize = nt;
    _image->zsize = nr;
    _image->num_comp = comp;
    _image->image = malloc_image(ns, nt, nr, comp, 0, 0, 0);
    _image->mipmaps = NULL;
    _image->next = NULL;

    for (c = 0; c < comp; c++)
	for (z = 0; z < nr; z++)
	    for (y = 0; y < nt; y++)
		for (x = 0; x < ns; x++) {
		    bimage = (unsigned char *)image;
		    n = bimage[ z*nt*ns*comp + 
			      y*ns*comp + x*comp + c];
		    _image->image[c][z][y][x] = n / 255.0;
		}
	
    _image->format = pfGetTexFormat( pftex, PFTEX_INTERNAL_FORMAT );
}

#ifdef WIN32 
/* XXX Alex - not the fastest way to do this but will do for now */
static int rint(double v) { return (int)(v + 0.5); }
#endif

PFDUDLLEXPORT void pfdImageToPfTexture(pfdImage *_image, pfTexture *pftex)
{
    uint *image;
    int comp, ns, nt, nr;
    int x, y, z, c;
    unsigned char *bimage;
    unsigned char n;
    int ext;

    if (! pftex)
	return;

    if (! _image)
	return;

    ext = pfGetTexFormat( pftex, PFTEX_EXTERNAL_FORMAT );
    if (ext != PFTEX_UNSIGNED_BYTE) {
	pfNotify( PFNFY_WARN, PFNFY_WARN, "pfdImageToPfTexture "
		  "unsupported format 0x%x\n", ext);
    }

    if (_image->mipmaps) {
	pfNotify( PFNFY_WARN, PFNFY_WARN, "pfdImageToPfTexture "
		  "doesn't support mipmaps yet\n");
    }

    pfGetTexImage( pftex, &image, &comp, &ns, &nt, &nr );

    if (image)
	pfFree(image);

    /* assuming 1 byte per comp */
    image = 
	(uint *)pfMalloc( _image->xsize * _image->ysize * _image->zsize *
			  _image->num_comp, pfGetSharedArena() );

    for (c = 0; c < _image->num_comp; c++)
	for (z = 0; z < _image->zsize; z++)
	    for (y = 0; y < _image->ysize; y++)
		for (x = 0; x < _image->xsize; x++) {
		    unsigned char *bimage = (unsigned char *)image;
		    unsigned char n = (unsigned char)
			rint( _image->image[c][z][y][x] * 
			      255.0 );
		    bimage[ z*_image->ysize*_image->xsize*_image->num_comp + 
			  y*_image->xsize*_image->num_comp +
			  x*_image->num_comp + c] = n;
		}

    pfTexImage( pftex, image, _image->num_comp, _image->xsize,
		_image->ysize, _image->zsize );
}

PFDUDLLEXPORT
void pfdImageGenMipmaps(pfdImage *_image,
			int *_ksize, double ***_kernel, int *_wrap)
{
    double ****image;
    double ****old_image;
    int xs, ys, zs, x, y, z, c, old_x, old_y, old_z;
    int num_levels, level;

    if (_image->mipmaps)
	pfdImageDelMipmaps(_image);

    num_levels = get_num_levels(_image);
    _image->mipmaps = (pfdImage*)malloc(sizeof(pfdImage) * num_levels);
    bzero(_image->mipmaps, sizeof(pfdImage) * num_levels);

    if (_kernel == NULL)
    {
	old_image = _image->image;

	xs = _image->xsize >> 1;
	ys = _image->ysize >> 1;
	zs = _image->zsize >> 1;
	level = 0;

	while (xs > 0 || ys > 0 || zs > 0)
	{
	    image = malloc_image(xs == 0 ? 1 : xs,
				 ys == 0 ? 1 : ys,
				 zs == 0 ? 1 : zs,
				 _image->num_comp, 0, 0, 0);

	    /*
	     *  Shrink the image
	     */
	    if (zs == 0)
	    {
		zs = 1;
		if (xs == 0)
		{
		    xs = 1;
		    for (c = 0; c < _image->num_comp; c++)
			for (y = 0, old_y = 0; y < ys; y++, old_y += 2)
			{
			    image[c][0][y][0] =
					(old_image[c][0][old_y  ][0] +
					 old_image[c][0][old_y+1][0]) /
					2.0;
			}
		}
		else if (ys == 0)
		{
		    ys = 1;
		    for (c = 0; c < _image->num_comp; c++)
			 for (x = 0, old_x = 0; x < xs; x++, old_x += 2)
			 {
			    image[c][0][0][x] =
					(old_image[c][0][0][old_x  ] +
					 old_image[c][0][0][old_x+1]) /
					2.0;
			 }
		}
		else
		{
		    for (c = 0; c < _image->num_comp; c++)
			for (y = 0, old_y = 0; y < ys; y++, old_y += 2)
			    for (x = 0, old_x = 0; x < xs; x++, old_x += 2)
			    {
				image[c][0][y][x] =
					(old_image[c][0][old_y  ][old_x  ] +
					 old_image[c][0][old_y  ][old_x+1] +
					 old_image[c][0][old_y+1][old_x  ] +
					 old_image[c][0][old_y+1][old_x+1]) /
					4.0;
			    }
		}
	    }
	    else
	    {
		if (xs == 0 && ys == 0)
		{
		    xs = 1;
		    ys = 1;
		    for (c = 0; c < _image->num_comp; c++)
			for (z = 0, old_z = 0; z < zs; z++, old_z += 2)
			{
			    image[c][z][0][0] =
					(old_image[c][old_z  ][0][0] +
					 old_image[c][old_z+1][0][0]) /
					2.0;
			}
		}
		else if (xs == 0)
		{
		    xs = 1;
		    for (c = 0; c < _image->num_comp; c++)
			for (z = 0, old_z = 0; z < zs; z++, old_z += 2)
			    for (y = 0, old_y = 0; y < ys; y++, old_y += 2)
			    {
				image[c][z][y][0] =
					(old_image[c][old_z  ][old_y  ][0] +
					 old_image[c][old_z  ][old_y+1][0] +
					 old_image[c][old_z+1][old_y  ][0] +
					 old_image[c][old_z+1][old_y+1][0]) /
					4.0;
			    }
		}
		else if (ys == 0)
		{
		    ys = 1;
		    for (c = 0; c < _image->num_comp; c++)
			for (z = 0, old_z = 0; z < zs; z++, old_z += 2)
			    for (x = 0, old_x = 0; x < xs; x++, old_x += 2)
			    {
				image[c][z][0][x] =
					(old_image[c][old_z  ][0][old_x  ] +
					 old_image[c][old_z  ][0][old_x+1] +
					 old_image[c][old_z+1][0][old_x  ] +
					 old_image[c][old_z+1][0][old_x+1]) /
					4.0;
			    }
		}
		else
		{
		    for (c = 0; c < _image->num_comp; c++)
			for (z = 0, old_z = 0; z < zs; z++, old_z += 2)
			    for (y = 0, old_y = 0; y < ys; y++, old_y += 2)
				for (x = 0, old_x = 0; x < xs; x++, old_x += 2)
				{
				    image[c][z][y][x] =
				     (old_image[c][old_z  ][old_y  ][old_x  ] +
				      old_image[c][old_z  ][old_y  ][old_x+1] +
				      old_image[c][old_z  ][old_y+1][old_x  ] +
				      old_image[c][old_z  ][old_y+1][old_x+1] +
				      old_image[c][old_z+1][old_y  ][old_x  ] +
				      old_image[c][old_z+1][old_y  ][old_x+1] +
				      old_image[c][old_z+1][old_y+1][old_x  ] +
				      old_image[c][old_z+1][old_y+1][old_x+1]) /
				     8.0;
				}
		}
	    }

	    _image->mipmaps[level].image = image;
	    _image->mipmaps[level].xsize = xs;
	    _image->mipmaps[level].ysize = ys;
	    _image->mipmaps[level].zsize = zs;
	    _image->mipmaps[level].num_comp = _image->num_comp;

	    old_image = image;
	    level++;
	    xs >>= 1;
	    ys >>= 1;
	    zs >>= 1;
	}
    }
    else
    {
	double ****tmp_image;
	int xhks, yhks, zhks;
	int xk, yk, zk;

	if (_ksize == NULL || _wrap == NULL)
	{
	    pfNotify(PFNFY_WARN, PFNFY_USAGE,
		     "pfdImageGenMipmaps() - _kernel, _ksize, and _wrap "
		     "must be non NULL to use specify a kernel.");
	    return;
	}
	if ((_ksize[PF_X] == 1 && _image->xsize != 1) ||
	    (_ksize[PF_Y] == 1 && _image->ysize != 1) ||
	    (_ksize[PF_Z] == 1 && _image->ysize != 1) ||
	    (_ksize[PF_X] != 1 && _image->xsize == 1) ||
	    (_ksize[PF_Y] != 1 && _image->ysize == 1) ||
	    (_ksize[PF_Z] != 1 && _image->ysize == 1) ||
	    (_ksize[PF_X] & 0x1 && _ksize[PF_X] != 1) ||
	    (_ksize[PF_Y] & 0x1 && _ksize[PF_Y] != 1) ||
	    (_ksize[PF_Z] & 0x1 && _ksize[PF_Z] != 1))
	{
	    pfNotify(PFNFY_WARN, PFNFY_USAGE,
		     "pfdImageGenMipmaps() - illegal kernel size");
	    return;
	}

	xhks = _ksize[PF_X] >> 1;
	yhks = _ksize[PF_Y] >> 1;
	zhks = _ksize[PF_Z] >> 1;

	image = malloc_image(_image->xsize, _image->ysize, _image->zsize,
			     _image->num_comp, xhks, yhks, zhks);
	old_image = malloc_image(_image->xsize, _image->ysize, _image->zsize,
				 _image->num_comp, xhks, yhks, zhks);

	xs = _image->xsize;
	ys = _image->ysize;
	zs = _image->zsize;
	level = 0;

	copy_image(old_image, _image->image, xs, ys, zs, _image->num_comp);

	while (xs > 1 || ys > 1 || zs > 1)
	{
	    /*
	     *  Note: hks-1 could be used as the border size in all cases
	     *  except when a dimension of the image we are shrinking has
	     *  reached 1.  Using hks as the border size is simpler.
	     */
	    make_border(old_image, xs, ys, zs, _image->num_comp,
			xhks, yhks, xhks, _wrap);

	    if (xs > 1)
		xs >>= 1;
	    if (ys > 1)
		ys >>= 1;
	    if (zs > 1)
		zs >>= 1;

	    /*
	     *  make mipmap
	     */
	    for (c = 0; c < _image->num_comp; c++)
	    for (z = 0, old_z = (zhks)?1-zhks:0; z < zs; z++, old_z += 2)
	    for (y = 0, old_y = (yhks)?1-yhks:0; y < ys; y++, old_y += 2)
	    for (x = 0, old_x = (xhks)?1-xhks:0; x < xs; x++, old_x += 2)
	    {
		/*
		 *  make texel in mipmap
		 */
		image[c][z][y][x] = 0.0;
		for (zk = 0; zk < _ksize[PF_Z]; zk++)
		for (yk = 0; yk < _ksize[PF_Y]; yk++)
		for (xk = 0; xk < _ksize[PF_X]; xk++)
		    image[c][z][y][x] +=
				old_image[c][old_z+zk][old_y+yk][old_x+xk] *
				_kernel[zk][yk][xk];
	    }

	    tmp_image = malloc_image(xs, ys, zs, _image->num_comp, 0, 0, 0);
	    copy_image(tmp_image, image, xs, ys, zs, _image->num_comp);
	    _image->mipmaps[level].image = tmp_image;
	    _image->mipmaps[level].xsize = xs;
	    _image->mipmaps[level].ysize = ys;
	    _image->mipmaps[level].zsize = zs;
	    _image->mipmaps[level].num_comp = _image->num_comp;

	    tmp_image = old_image;
	    old_image = image;
	    image = tmp_image;

	    level++;
	}
    }

    /*
     *  If this image was already packed, pack the mipmaps.
     */
    if (_image->packed)
    {
	int i;

	for (i = 0; i < num_levels; i++)
	    if (_image->mipmaps[i].packed)
		pfdImagePack(&_image->mipmaps[i], _image->format);
    }

    /*
     *  If this image has multiple images, generate mipmaps for them.
     */
    if (_image->next)
	pfdImageGenMipmaps(_image->next, _ksize, _kernel, _wrap);
}


PFDUDLLEXPORT void pfdImageDelMipmaps(pfdImage *_image)
{
    if (_image->mipmaps)
    {
	int i, num_levels;

	num_levels = get_num_levels(_image);

	for (i = 0; i < num_levels; i++)
	{
	    if (_image->mipmaps[i].image)
		free(_image->mipmaps[i].image);
	    if (_image->mipmaps[i].packed)
		free(_image->mipmaps[i].packed);
	}

	free(_image->mipmaps);

	_image->mipmaps = NULL;
    }

    /*
     *  If this image has multiple images, delete their mipmaps.
     */
    if (_image->next)
	pfdImageDelMipmaps(_image->next);
}


PFDUDLLEXPORT void pfdImageAlignment(pfdImage *_image, int _alignment)
{
    _image->alignment = _alignment;

    if (_image->next)
	pfdImageAlignment(_image->next, _alignment);
}


static int get_num_levels(pfdImage *_image)
{
    int max;
    int levels;

    max = (_image->xsize > _image->ysize) ?
	      ((_image->xsize > _image->zsize) ? _image->xsize : _image->zsize)
	  :
	      ((_image->ysize > _image->zsize) ? _image->ysize : _image->zsize);

    levels = 0;
    while (max > 1)
    {
	levels++;
	max >>= 1;
    }

    return levels;
}


static int get_num_images(pfdImage *_image)
{
    pfdImage *itmp;
    int num;

    for (num = 0, itmp = _image; itmp != NULL; num++, itmp = itmp->next);

    return num;
}

static
double ****malloc_image(int xsize, int ysize, int zsize, int num_comp,
			       int xborder, int yborder, int zborder)
{
    unsigned char *data;
    double ****image;
    int xs, ys, zs;
    int y, z, c;

    xs = xsize + 2 * xborder;
    ys = ysize + 2 * yborder;
    zs = zsize + 2 * zborder;

    /*
     *  malloc the data
     */
    data = (unsigned char*)malloc((sizeof(double***) * num_comp) +
				  (sizeof(double**) * num_comp * zs) +
				  (sizeof(double*) * num_comp * zs * ys) +
				  (sizeof(double) * num_comp * zs * ys * xs) +
				  4);
    assert(data != NULL);

    /*
     *  Connect the data to make image[num_comp][zs][ys][xs]
     */
    image = (double****)data;

    data += sizeof(double***) * num_comp;

    for (c = 0; c < num_comp; c++)
    {
	image[c] = (double***)data;
	data += sizeof(double**) * zs;
    }

    for (c = 0; c < num_comp; c++)
	for (z = 0; z < zs; z++)
	{
	    image[c][z] = (double**)data;
	    data += sizeof(double*) * ys;
	}

    /*
     *  make sure double data starts on a double address
     */
    if ((long)data & 0x4)
	data += 0x4;

    for (c = 0; c < num_comp; c++)
	for (z = 0; z < zs; z++)
	    for (y = 0; y < ys; y++)
	    {
		image[c][z][y] = (double*)data;
		data += sizeof(double) * xs;
	    }

    /*
     *  Shift so that the corner of the image is at [0][0][0].
     */
    for (c = 0; c < num_comp; ++c)
    {
	for (z = 0; z < zs; z++)
	{
	    for (y = 0; y < ys; y++)
		image[c][z][y] += xborder;
	    image[c][z] += yborder;
	}
	image[c] += zborder;
    }

    return(image);
}


static void copy_image(double ****dst, double ****src,
		       int xsize, int ysize, int zsize, int num_comp)
{
    int x, y, z, c;

    for (c = 0; c < num_comp; c++)
	for (z = 0; z < zsize; z++)
	    for (y = 0; y < ysize; y++)
		for (x = 0; x < xsize; x++)
		    dst[c][z][y][x] = src[c][z][y][x];
}


static void make_border(double ****image,
			int xsize, int ysize, int zsize, int num_comp,
			int xborder, int yborder, int zborder, int *wrap)
{
    int x, y, z, c;

    /*
     *  Make x border
     */
    if (wrap[PF_X])
	for (c = 0; c < num_comp; c++)
	    for (z = 0; z < zsize; z++)
		for (y = 0; y < ysize; y++)
		    for (x = 0; x < xborder; x++)
		    {
			image[c][z][y][   -1-x] = image[c][z][y][xsize-1-x];
			image[c][z][y][xsize+x] = image[c][z][y][	 x];
		    }
    else
	for (c = 0; c < num_comp; c++)
	    for (z = 0; z < zsize; z++)
		for (y = 0; y < ysize; y++)
		    for (x = 0; x < xborder; x++)
		    {
			image[c][z][y][   -1-x] = image[c][z][y][      0];
			image[c][z][y][xsize+x] = image[c][z][y][xsize-1];
		    }

    /*
     *  Make y border
     */
    if (wrap[PF_Y])
	for (c = 0; c < num_comp; c++)
	    for (z = 0; z < zsize; z++)
		for (y = 0; y < yborder; y++)
		    for (x = -xborder; x < xsize+xborder; x++)
		    {
			image[c][z][   -1-y][x] = image[c][z][ysize-1-y][x];
			image[c][z][ysize+y][x] = image[c][z][	      y][x];
		    }
    else
	for (c = 0; c < num_comp; c++)
	    for (z = 0; z < zsize; z++)
		for (y = 0; y < yborder; y++)
		    for (x = -xborder; x < xsize+xborder; x++)
		    {
			image[c][z][   -1-y][x] = image[c][z][      0][x];
			image[c][z][ysize+y][x] = image[c][z][ysize-1][x];
		    }

    /*
     *  Make z border
     */
    if (wrap[PF_Z])
	for (c = 0; c < num_comp; c++)
	    for (z = 0; z < zborder; z++)
		for (y = -yborder; y < ysize+yborder; y++)
		    for (x = -xborder; x < xsize+xborder; x++)
		    {
			image[c][   -1-z][y][x] = image[c][zsize-1-z][y][x];
			image[c][zsize+z][y][x] = image[c][	   z][y][x];
		    }
    else
	for (c = 0; c < num_comp; c++)
	    for (z = 0; z < zborder; z++)
		for (y = -yborder; y < ysize+yborder; y++)
		    for (x = -xborder; x < xsize+xborder; x++)
		    {
			image[c][   -1-z][y][x] = image[c][      0][y][x];
			image[c][zsize+z][y][x] = image[c][zsize-1][y][x];
		    }
}
