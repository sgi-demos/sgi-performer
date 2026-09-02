/**************************************************************************
 *									  *
 *		 Copyright (C) 1997, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
 *  pfpfi.h $Revision: 1.5 $
 */


#ifndef __PFPFI_H__
#define __PFPFI_H__


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    uint magic;
    int version;
    int data_start;
    int data_size;
    int size[3];
    int num_comp;
    int format;
    int gl;
    int mipmaps;
    int num_images;
    int alignment;
} pfi_header_t;

#define PFI_MAGIC_NUMBER	0xdb0fdb00
#define PFI_MAGIC_ENDIAN	0x00db0fdb
#define PFI_VERSION_NUMBER      0

#define PFI_FORMAT_UINT_8		1
#define PFI_FORMAT_INT_8		2
#define PFI_FORMAT_UINT_16		3
#define PFI_FORMAT_INT_16		4
#define PFI_FORMAT_UINT_32		5
#define PFI_FORMAT_INT_32		6
#define PFI_FORMAT_FLOAT_32		7
#define PFI_FORMAT_PACK_3_3_2		8
#define PFI_FORMAT_PACK_4_4_4_4		9
#define PFI_FORMAT_PACK_5_5_5_1		10
#define PFI_FORMAT_PACK_8_8_8_8		11
#define PFI_FORMAT_PACK_10_10_10_2	12

#define PFI_OPENGL	0
#define PFI_IRISGL	1


#ifdef __cplusplus
}
#endif


#endif /* __PFPFI_H__ */
