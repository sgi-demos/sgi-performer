
/*              Copyright (C) Coryphaeus Software, Inc. 1997             */

  static char revision[] = "$Revision: 1.73 $";
  static char date[] = "$Date: 2002/09/22 03:06:30 $";
  static char source[] = "$RCSfile: pfdwb.c,v $";

/*=======================================================================
 *========================================================================
 *
 * MODULE	: pfdwb.c	
 *
 * DESCRIPTION  : open a Designer's Workbench (.dwb) file & convert to  
 *		  Performer in-memory structures. Makes use of pfdBuilder
 *		  utilities for sorting/creating pfGeoSets.This software
 *		  will only run under Performer 2.x
 *
 *========================================================================
 *========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef sgi
#include <bstring.h>
#endif
#include <ctype.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pfdu.h>

#include <Performer/pfdb/pfdwb.h>

#ifdef __linux__
#include <asm/byteorder.h>
#endif

/*========================================================================
 *========================================================================
 *
 * 		             Defines and Macros
 *
 *========================================================================
 *========================================================================*/

#define LOWEST_DWB_FORMAT_VERSION_SUPPORTED	2.1
#define DWB_3_0_FORMAT_VERSION   		3.0
#define HIGHEST_DWB_FORMAT_VERSION_SUPPORTED    3.2
#define FLUSH_AND_EXIT 	{ fflush (stderr); exit (-1); } 
#define ROUNDING_ERROR				0.05

/* string comparison that avoids function call when possible */
#define SAME(_a, _b)	(strcasecmp(_a, _b) == 0)

/* define program limits and sizes */
#define	BUFFER_SIZE	4096
#define	MAX_STRING	4096
#define	GFX_REALITY	0x1
#define	GFX_MULTISAMPLE	0x10

/* define parse render group instructions */
#define GET_ALL_POLYS	0
#define GET_ONE_POLY	1

#define NORMAL		0
#define LAYER_BASE	2
#define LAYER_DECAL	4
#define DECAL_NONE     -1
#define LIGHTPOINT	8
#define BILLBOARD	16
#define SEQUENCE	32

#define RADTODEG 57.29578
#define DEGTORAD 0.017453293
#define copyNyup(dst,src,m) \
       {dst[0]=  src[0] * m; dst[1]= src[1] * m; dst[2]= src[2] * m; }

     /* byte sex */
#ifdef __LITTLE_ENDIAN
    
    short get16_le(void *ptr)
    {
       short tmp;
       ((unsigned char *)&tmp)[0] = ((unsigned char *)ptr)[1];
       ((unsigned char *)&tmp)[1] = ((unsigned char *)ptr)[0];
       return tmp;
    }
    
    int get32_le(void *ptr)
    {
       int tmp;
       ((unsigned char *)&tmp)[0] = ((unsigned char *)ptr)[3];
       ((unsigned char *)&tmp)[1] = ((unsigned char *)ptr)[2];
       ((unsigned char *)&tmp)[2] = ((unsigned char *)ptr)[1];
       ((unsigned char *)&tmp)[3] = ((unsigned char *)ptr)[0];
       return tmp;
    }
    
    void get16v_le(void *dstp, void *srcp, int n)
    {
       unsigned char *dst = dstp;
       unsigned char *src = srcp;
       int i;
       for (i = 0; i < n; ++i) {
          int t0 = src[0];
          dst[0] = src[1];
          dst[1] = t0;
          dst += 2;
          src += 2;
       }
    }
    void get32v_le(void *dstp, void *srcp, int n)
    {
       unsigned char *dst = dstp;
       unsigned char *src = srcp;
       int i;
       for (i = 0; i < n; ++i) {      
          int t0 = src[0];
          int t1 = src[1];
          dst[0] = src[3];
          dst[1] = src[2];
          dst[2] = t1;
          dst[3] = t0;
          dst += 4;
          src += 4;
       }
    }
    
    void get64v_le(void *dstp, void *srcp, int n)
    {
       unsigned char *dst = dstp;
       unsigned char *src = srcp;
       int i;
       for (i = 0; i < n; ++i) {
          int t0 = src[0];
          int t1 = src[1];
          int t2 = src[2];
          int t3 = src[3];      
          dst[0] = src[7];
          dst[1] = src[6];
          dst[2] = src[5];
          dst[3] = src[4];
          dst[4] = t3;
          dst[5] = t2;
          dst[6] = t1;
          dst[7] = t0;
          dst += 8;
          src += 8;
       }
    }
   
#else
    
    int get16_be(void *ptr)
    {
       return *(short *)ptr;
    }
    
    int get32_be(void *ptr)
    {
       return *(int *)ptr;
    }
    void get16v_be(void *dst, void *src, int n)
    {
       memcpy(dst, src, 2*n);
    }
    
    void get32v_be(void *dst, void *src, int n)
    {
       memcpy(dst, src, 4*n);
    }
    
    void get64v_be(void *dst, void *src, int n)
    {
       memcpy(dst, src, 8*n);
    }
    
#endif /* __LITTLE_ENDIAN */
    
    
#ifdef __LITTLE_ENDIAN
    
#define get16 get16_le
#define get32 get32_le
#define get16v get16v_le
#define get32v get32v_le
#define get64v get64v_le

#else
    /* big endian */
    
#define get16(p) (*(short *)(p))
#define get32(p) (*(int *)(p))
#define get16v(q,p,n) memcpy(q,p,2*(n))
#define get32v(q,p,n) memcpy(q,p,4*(n))
#define get64v(q,p,n) memcpy(q,p,8*(n))

#endif

/* swapping routines */

	void swapDwbLModelInfo(dwbLModelInfo *ptr)
	{
		int i;
	
		for(i=0; i<8; i++)
		{
			ptr->defn[i] = get32((void *)&ptr->defn[i]);
		}
	}

    void swapDwbLSourceInfo(dwbLSourceInfo *ptr)
    {
    	int i;

    	for(i=0; i<15; i++)
    	{
    		ptr->defn[i] = get32((void *)&ptr->defn[i]);
    	}
    }

    void swapDwbInstanceDef(dwbInstanceDef *ptr)
    {
    	ptr->val = get16(&ptr->val);
    	ptr->spare = get16(&ptr->spare);
    } /* end swapDwbInstanceDef() */
    
    void swapDwbInstanceRef(dwbInstanceRef *ptr)
    {
    	int i;
    
    	ptr->val = get16(&ptr->val);
    	ptr->spare = get16(&ptr->spare);
    	for(i=0; i<4; i++)
    	{
    		get32v(&ptr->mat[i][0], &ptr->mat[i][0], 1);
    		get32v(&ptr->mat[i][1], &ptr->mat[i][1], 1);
    		get32v(&ptr->mat[i][2], &ptr->mat[i][2], 1);
    		get32v(&ptr->mat[i][3], &ptr->mat[i][3], 1);
    	}
    } /* end swapDwbInstanceRef() */
    
   void swapDwbLineStyle(dwbLineStyle *ptr)
    {
    	ptr->lstyle = get16(&ptr->lstyle);
    	ptr->repeat = get16(&ptr->repeat);
    	get32v(&ptr->spare[0], &ptr->spare[0], 1);
    	get32v(&ptr->spare[1], &ptr->spare[1], 1);
    } /* end swapDwbLineStyle() */
    
    void swapDwbMatrix(dwbMatrix *ptr)
    {
    	int i;
    
    	for(i=0; i<4; i++)
    	{
    		get32v(&ptr->mat[i][0], &ptr->mat[i][0], 1);
    		get32v(&ptr->mat[i][1], &ptr->mat[i][1], 1);
    		get32v(&ptr->mat[i][2], &ptr->mat[i][2], 1);
    		get32v(&ptr->mat[i][3], &ptr->mat[i][3], 1);
    	}
    } /* end swapDwbMatrix() */

  void swapDwbHeader(dwbHeader *ptr)
    {
    	int myflags;
    
    	ptr->units = get16(&ptr->units);
    	ptr->reserved = get16(&ptr->reserved);
    	get32v(&ptr->version, &ptr->version, 1);
    
    
    	ptr->next_node_ids[0] = get16(&ptr->next_node_ids[0]);
    	ptr->next_node_ids[1] = get16(&ptr->next_node_ids[1]);
    	ptr->next_node_ids[2] = get16(&ptr->next_node_ids[2]);
    	ptr->next_node_ids[3] = get16(&ptr->next_node_ids[3]);
    	ptr->next_node_ids[4] = get16(&ptr->next_node_ids[4]);
    	ptr->next_node_ids[5] = get16(&ptr->next_node_ids[5]);
    
    	get32v(&ptr->next_node_ids[0], &ptr->next_node_ids[0], 1);
    	get32v(&ptr->next_node_ids[1], &ptr->next_node_ids[1], 1);
    	get32v(&ptr->next_node_ids[2], &ptr->next_node_ids[2], 1);
    	get32v(&ptr->next_node_ids[3], &ptr->next_node_ids[3], 1);
    	get32v(&ptr->next_node_ids[4], &ptr->next_node_ids[4], 1);
    	get32v(&ptr->next_node_ids[5], &ptr->next_node_ids[5], 1);
    
    	ptr->drawstyle = get16(&ptr->drawstyle);
    	ptr->movestyle = get16(&ptr->movestyle);
    	ptr->shademodel = get16(&ptr->shademodel);
    	ptr->zbuffer = get16(&ptr->zbuffer);
    	ptr->backface = get16(&ptr->backface);
    	ptr->concave = get16(&ptr->concave);
    
    	/* swap flags...happens to coorespond to 32 bits/sizeof int */
/*
    	memcpy(&myflags, &ptr->flags, sizeof(int));
    	myflags = get32(&myflags);
    	memcpy(&ptr->flags, &myflags, sizeof(int));
*/    
    	get32v(&ptr->spare[0], &ptr->spare[0], 1);
    	get32v(&ptr->spare[1], &ptr->spare[1], 1);
    	get32v(&ptr->spare[2], &ptr->spare[2], 1);
    	get32v(&ptr->spare[3], &ptr->spare[3], 1);
    	ptr->spare2 = get16(&ptr->spare2);
    
    	ptr->color = get16(&ptr->color);
    	ptr->material_binding = get16(&ptr->material_binding);
    	ptr->transparency = get16(&ptr->transparency);
    	ptr->texture = get16(&ptr->texture);
    	ptr->up_axis = get16(&ptr->up_axis);
    
    } /* end swapDwbHeader() */
    void swapDwbExtTexInfo(dwbExtTexInfo *ptr)
    {
    	int i;
    
    	ptr->xsize = get16(&ptr->xsize);
    	ptr->ysize = get16(&ptr->ysize);
    	ptr->zsize = get16(&ptr->zsize);
    	ptr->type = get16(&ptr->type);
    
    	get32v(&ptr->minfilter, &ptr->minfilter, 1);
    	get32v(&ptr->magfilter, &ptr->magfilter, 1);
    	
    	get32v(&ptr->wrap, &ptr->wrap, 1);
    	get32v(&ptr->wraps, &ptr->wraps, 1);
    	get32v(&ptr->wrapt, &ptr->wrapt, 1);
    	get32v(&ptr->envtype, &ptr->envtype, 1);
    
    	get32v(&ptr->version, &ptr->version, 1);
    
    	get32v(&ptr->component_select, &ptr->component_select, 1);
    	get32v(&ptr->envcolor[0], &ptr->envcolor[0], 1);
    	get32v(&ptr->envcolor[1], &ptr->envcolor[1], 1);
    	get32v(&ptr->envcolor[2], &ptr->envcolor[2], 1);
    	get32v(&ptr->envcolor[3], &ptr->envcolor[3], 1);
    
    	get32v(&ptr->magfilter_alpha, &ptr->magfilter_alpha, 1);
    	get32v(&ptr->magfilter_color, &ptr->magfilter_color, 1);
    
    	get32v(&ptr->bicubic_filter[0], &ptr->bicubic_filter[0], 1);
    	get32v(&ptr->bicubic_filter[1], &ptr->bicubic_filter[1], 1);
    	get32v(&ptr->mipmap_filter[0], &ptr->mipmap_filter[0], 1);
    	get32v(&ptr->mipmap_filter[1], &ptr->mipmap_filter[1], 1);
    	get32v(&ptr->mipmap_filter[2], &ptr->mipmap_filter[2], 1);
    	get32v(&ptr->mipmap_filter[3], &ptr->mipmap_filter[3], 1);
    	get32v(&ptr->mipmap_filter[4], &ptr->mipmap_filter[4], 1);
    	get32v(&ptr->mipmap_filter[5], &ptr->mipmap_filter[5], 1);
    	get32v(&ptr->mipmap_filter[6], &ptr->mipmap_filter[6], 1);
    	get32v(&ptr->mipmap_filter[7], &ptr->mipmap_filter[7], 1);
    
    	get32v(&ptr->internal, &ptr->internal, 1);
    	get32v(&ptr->external, &ptr->external, 1);
    
    	for(i=0; i<4; i++)
    	{
    		get32v(&ptr->control_points[0], &ptr->control_points[0], 1);
    		get32v(&ptr->control_points[1], &ptr->control_points[1], 1);
    	}
    	get32v(&ptr->control_clamp, &ptr->control_clamp, 1);
    
    	get32v(&ptr->detail[0], &ptr->detail[0], 1);
    	get32v(&ptr->detail[1], &ptr->detail[1], 1);
    	get32v(&ptr->detail[2], &ptr->detail[2], 1);
    	get32v(&ptr->detail[3], &ptr->detail[3], 1);
    	get32v(&ptr->detail[4], &ptr->detail[4], 1);
    
    
    	get32v(&ptr->tile[0], &ptr->tile[0], 1);
    	get32v(&ptr->tile[1], &ptr->tile[1], 1);
    	get32v(&ptr->tile[2], &ptr->tile[2], 1);
    	get32v(&ptr->tile[3], &ptr->tile[3], 1);
    
    } /* end swapDwbExtTexInfo() */
    
    
    void swapDwbTexInfo(dwbTexInfo *ptr)
    {
    	ptr->xsize = get16(&ptr->xsize);
    	ptr->ysize = get16(&ptr->ysize);
    	ptr->zsize = get16(&ptr->zsize);
    	ptr->type = get16(&ptr->type);
    
    	get32v(&ptr->minfilter, &ptr->minfilter, 1);
    	get32v(&ptr->magfilter, &ptr->magfilter, 1);
    
    	get32v(&ptr->wrap, &ptr->wrap, 1);
    	get32v(&ptr->wraps, &ptr->wraps, 1);
    	get32v(&ptr->wrapt, &ptr->wrapt, 1);
    	get32v(&ptr->envtype, &ptr->envtype, 1);
    } /* end swapDwbTexInfo() */
    
    
    void swapDwbOpcodeRec(dwbOpcodeRec *ptr)
    {
    	ptr->opcode = get16(&ptr->opcode);
    	ptr->size = get16(&ptr->size);
    } /* end swapDwbOpcodeRec() */
    
    void swapDwbMaterial(dwbMaterial *ptr)
    {
    	get32v(&ptr->diffuse[0], &ptr->diffuse[0], 1);
    	get32v(&ptr->diffuse[1], &ptr->diffuse[1], 1);
    	get32v(&ptr->diffuse[2], &ptr->diffuse[2], 1);
    
    	get32v(&ptr->ambient[0], &ptr->ambient[0], 1);
    	get32v(&ptr->ambient[1], &ptr->ambient[1], 1);
    	get32v(&ptr->ambient[2], &ptr->ambient[2], 1);
    
    	get32v(&ptr->shininess, &ptr->shininess, 1);
    
    	get32v(&ptr->specular[0], &ptr->specular[0], 1);
    	get32v(&ptr->specular[1], &ptr->specular[1], 1);
    	get32v(&ptr->specular[2], &ptr->specular[2], 1);
    
    	get32v(&ptr->emissive[0], &ptr->emissive[0], 1);
    	get32v(&ptr->emissive[1], &ptr->emissive[1], 1);
    	get32v(&ptr->emissive[2], &ptr->emissive[2], 1);
    
    	get32v(&ptr->alpha, &ptr->alpha, 1);
   
            /*printf("alpha: %.2f\n", ptr->alpha);*/
    } /* end swapDwbMaterial() */
    
    
    void swapDwbMatInfo(dwbMatInfo *ptr)
    {
    	swapDwbMaterial(&ptr->mat);
    } /* end swapDwbMatInfo() */
    
    void swapDwbColorTable(dwbColorTable *ptr)
    {
    	get16v(ptr->shaded, ptr->shaded, 31*3);
            get16v(ptr->fixed, ptr->fixed, 64*3);        
            get16v(ptr->overlay, ptr->overlay, 16*3);
            get16v(ptr->underlay, ptr->underlay, 16*3);
    } /* end swapDwbColorTable() */
    
    void swapDwbGroup(dwbGroup *ptr)
    {
    	int myflags;
    
    	ptr->layer = get16(&ptr->layer);
    	ptr->color = get16(&ptr->color);
    
    	ptr->drawstyle = get16(&ptr->drawstyle);
    	ptr->shademodel = get16(&ptr->shademodel);
    
    	ptr->linestyle = get16(&ptr->linestyle);
    	ptr->linewidth = get16(&ptr->linewidth);
    
    	ptr->fill_pattern = get16(&ptr->fill_pattern);
    	ptr->texture = get16(&ptr->texture);
   
    	get32v(&ptr->transparency, &ptr->transparency, 1);
   
    	ptr->material = get16(&ptr->material);
    	ptr->use_group_color = get16(&ptr->use_group_color);
    	ptr->use_group_dstyle = get16(&ptr->use_group_dstyle);
    
    	ptr->surfaceType = get16(&ptr->surfaceType);
    
    	get32v(&ptr->bbox[0], &ptr->bbox[0], 1);
    	get32v(&ptr->bbox[1], &ptr->bbox[1], 1);
    	get32v(&ptr->bbox[2], &ptr->bbox[2], 1);
    	get32v(&ptr->bbox[3], &ptr->bbox[3], 1);
    	get32v(&ptr->bbox[4], &ptr->bbox[4], 1);
    	get32v(&ptr->bbox[5], &ptr->bbox[5], 1);
    
    	get32v(&ptr->ll[0], &ptr->ll[0], 1);
    	get32v(&ptr->ll[1], &ptr->ll[1], 1);
    	get32v(&ptr->ll[2], &ptr->ll[2], 1);
    
    	get32v(&ptr->ur[0], &ptr->ur[0], 1);
    	get32v(&ptr->ur[1], &ptr->ur[1], 1);
    	get32v(&ptr->ur[2], &ptr->ur[2], 1);
    
    	get32v(&ptr->center[0], &ptr->center[0], 1);
    	get32v(&ptr->center[1], &ptr->center[1], 1);
    	get32v(&ptr->center[2], &ptr->center[2], 1);
    
    	ptr->material_binding = get16(&ptr->material_binding);
    
    	ptr->decal_type = get16(&ptr->decal_type);
    
    	ptr->packed_color = get32(&ptr->packed_color);
    
    	ptr->collapsed = get16(&ptr->collapsed);
    	ptr->seq_mode = get16(&ptr->seq_mode);
    
    	get32v(&ptr->seq_time, &ptr->seq_time, 1);
    	get32v(&ptr->seq_speed, &ptr->seq_speed, 1);
    
    	ptr->seq_nreps = get16(&ptr->seq_nreps);
    	ptr->seq_bgn = get16(&ptr->seq_bgn);
    	ptr->seq_end = get16(&ptr->seq_end);
    	ptr->seq_random = get16(&ptr->seq_random);
    
		
    	ptr->sound_index = get16(&ptr->sound_index);
    	ptr->spare5 = get16(&ptr->spare5);
		
    } /* end swapDwbGroup() */
    
    
    void swapDwbSwitch(dwbSwitch *ptr)
    {
    	ptr->layer = get16(&ptr->layer);
    	ptr->spare = get16(&ptr->spare);
    
    	ptr->switchtype = get16(&ptr->switchtype);
    	ptr->threshold = get16(&ptr->threshold);
    
    	get32v(&ptr->switchin, &ptr->switchin, 1);
    	get32v(&ptr->switchout, &ptr->switchout, 1);
    
    	get32v(&ptr->center[0], &ptr->center[0], 1);
    	get32v(&ptr->center[1], &ptr->center[1], 1);
    	get32v(&ptr->center[2], &ptr->center[2], 1);
    
    } /* end swapDwbSwitch() */
    
    void swapDwbNewFace(dwbNewFace *ptr)
    {
    	int myflags;
    
    	ptr->layer = get16(&ptr->layer);
    	ptr->color = get16(&ptr->color);
    
    	ptr->numverts = get16(&ptr->numverts);
    	ptr->ptype = get16(&ptr->ptype);
    
    	ptr->linewidth = get16(&ptr->linewidth);
    	ptr->drawstyle = get16(&ptr->drawstyle);
    
    	ptr->linestyle = get16(&ptr->linestyle);
    	ptr->back_color = get16(&ptr->back_color);
    
    	ptr->texture = get16(&ptr->texture);
    	ptr->pntsize = get16(&ptr->pntsize);
    
    	get32v(&ptr->ir, &ptr->ir, 1);
    
    	/* not bothering with flags */
    	memcpy(&myflags, &ptr->flags, sizeof(int));
    	myflags = get32(&myflags);
    	memcpy(&ptr->flags, &myflags, sizeof(int));
    
    	/* not bothering with user fields */
    	ptr->packed_color = get32(&ptr->packed_color);
    
    	/* not bothering with flags */
    	memcpy(&myflags, &ptr->ig_flags, sizeof(int));
    	myflags = get32(&myflags);
    	memcpy(&ptr->ig_flags, &myflags, sizeof(int));
    
    
    	ptr->colapsed = get16(&ptr->colapsed);
    	ptr->detail_tex = get16(&ptr->detail_tex);
    
    	ptr->surface_material_code = get16(&ptr->surface_material_code);
    	ptr->feature_id = get16(&ptr->feature_id);
    
    	ptr->alt_texture = get16(&ptr->alt_texture);
    	ptr->material = get16(&ptr->material);
    
    } /* end swapDwbNewFace() */
    
    void swapDwbPackedVertex(dwbPackedVertex *ptr)
    {
    	ptr->packed_color = get32(&ptr->packed_color);
    
    	get32v(&ptr->coords[0], &ptr->coords[0], 1);
    	get32v(&ptr->coords[1], &ptr->coords[1], 1);
    	get32v(&ptr->coords[2], &ptr->coords[2], 1);
    
    	get32v(&ptr->normal[0], &ptr->normal[0], 1);
    	get32v(&ptr->normal[1], &ptr->normal[1], 1);
    	get32v(&ptr->normal[2], &ptr->normal[2], 1);
    
    	get32v(&ptr->tex_coords[0], &ptr->tex_coords[0], 1);
    	get32v(&ptr->tex_coords[1], &ptr->tex_coords[1], 1);
    	get32v(&ptr->tex_coords[2], &ptr->tex_coords[2], 1);
    	get32v(&ptr->tex_coords[3], &ptr->tex_coords[3], 1);
    } /* end swapDwbPackedVertex() */
    
    void swapDwbVertex(dwbVertex *ptr)
    {	
    	ptr->spare = get16(&ptr->spare);
    	ptr->color = get16(&ptr->color);
    
    	get32v(&ptr->coords[0], &ptr->coords[0], 1);
    	get32v(&ptr->coords[1], &ptr->coords[1], 1);
    	get32v(&ptr->coords[2], &ptr->coords[2], 1);
    
    	get32v(&ptr->normal[0], &ptr->normal[0], 1);
    	get32v(&ptr->normal[1], &ptr->normal[1], 1);
    	get32v(&ptr->normal[2], &ptr->normal[2], 1);

    	get32v(&ptr->tex_coords[0], &ptr->tex_coords[0], 1);
    	get32v(&ptr->tex_coords[1], &ptr->tex_coords[1], 1);
    	get32v(&ptr->tex_coords[2], &ptr->tex_coords[2], 1);
    	get32v(&ptr->tex_coords[3], &ptr->tex_coords[3], 1);
    } /* end swapDwbVertex() */
    
    void swapDwbLightPt(dwbLightPt *ptr)
    {
    	ptr->ltype = get16(&ptr->ltype);
    	ptr->directionality = get16(&ptr->directionality);
    	ptr->shape = get16(&ptr->shape);
    	ptr->suppress_last_light = get16(&ptr->suppress_last_light);
    
    	get32v(&ptr->dir[0], &ptr->dir[0], 1);
    	get32v(&ptr->dir[1], &ptr->dir[1], 1);
    	get32v(&ptr->dir[2], &ptr->dir[2], 1);
    
    	get32v(&ptr->lwidth, &ptr->lwidth, 1);
    	get32v(&ptr->lheight, &ptr->lheight, 1);
    	get32v(&ptr->diam, &ptr->diam, 1);
    
    	get64v(&ptr->intensity, &ptr->intensity, 1);
    	get64v(&ptr->intensity_variance, &ptr->intensity_variance, 1);
    
    	get32v(&ptr->calligraphic_priority, &ptr->calligraphic_priority, 1);
    
    } /* end swapDwbLightPt() */
    
    
    void swapDwbBSpline(dwbBSpline *ptr)
    {
    	ptr->order = get32(&ptr->order);
    	ptr->numKnots = get32(&ptr->numKnots);
    	ptr->drawCntrlPnts = get32(&ptr->drawCntrlPnts);
    	ptr->spare[0] = get16(&ptr->spare[0]);
    	ptr->spare[1] = get16(&ptr->spare[1]);
    	get32v(&ptr->spare3[0], &ptr->spare3[0], 1);
    	get32v(&ptr->spare3[1], &ptr->spare3[1], 1);
    	get32v(&ptr->spare3[2], &ptr->spare3[2], 1);
    	get32v(&ptr->spare3[3], &ptr->spare3[3], 1);
    	get32v(&ptr->spare3[4], &ptr->spare3[4], 1);
    	get32v(&ptr->spare3[5], &ptr->spare3[5], 1);
    } /* end swapDwbBSpline() */
    
    void swapDwbArc(dwbArc *ptr)
    {
    	get32v(&ptr->pfOrigin[0], &ptr->pfOrigin[0], 1);
    	get32v(&ptr->pfOrigin[1], &ptr->pfOrigin[1], 1);
    	get32v(&ptr->pfOrigin[2], &ptr->pfOrigin[2], 1);
    
    	get32v(&ptr->fStartAngle, &ptr->fStartAngle, 1);
    	get32v(&ptr->fEndAngle, &ptr->fEndAngle, 1);
    	get32v(&ptr->fRadius, &ptr->fRadius, 1);
    
    	get32v(&ptr->pfNormal[0], &ptr->pfNormal[0], 1);
    	get32v(&ptr->pfNormal[1], &ptr->pfNormal[1], 1);
    	get32v(&ptr->pfNormal[2], &ptr->pfNormal[2], 1);
    
    	get32v(&ptr->matrix, &ptr->matrix, 16);
    
    	ptr->drawstyle = get16(&ptr->drawstyle);
    	ptr->layer = get16(&ptr->layer);
    	ptr->color = get16(&ptr->color);
    	ptr->linewidth = get16(&ptr->linewidth);
    	ptr->linestyle = get16(&ptr->linestyle);
    
    	ptr->packed_color = get32(&ptr->packed_color);
    
    } /* end swapDwbArc() */
    
    void swapDwbPage(dwbPage *ptr)
    {
    	int myflags;
    
    	ptr->layer = get16(&ptr->layer);
    	ptr->color = get16(&ptr->color);
    
    	ptr->pageid = get16(&ptr->pageid);
    	ptr->spare = get16(&ptr->spare);
    
    	memcpy(&myflags, &ptr->flags, sizeof(int));
    	myflags = get32(&myflags);
    	memcpy(&ptr->flags, &myflags, sizeof(int));
    
    
    	get32v(&ptr->fov, &ptr->fov, 1);
    	get32v(&ptr->aspect, &ptr->aspect, 1);
    	get32v(&ptr->pitch, &ptr->pitch, 1);
    	get32v(&ptr->roll, &ptr->roll, 1);
    	get32v(&ptr->heading, &ptr->heading, 1);
    	get32v(&ptr->scale, &ptr->scale, 1);
    	get32v(&ptr->eyept[0], &ptr->eyept[0], 1);
    	get32v(&ptr->eyept[1], &ptr->eyept[1], 1);
    	get32v(&ptr->eyept[2], &ptr->eyept[2], 1);
    
    	get32v(&ptr->pgcen[0], &ptr->pgcen[0], 1);
    	get32v(&ptr->pgcen[1], &ptr->pgcen[1], 1);
    	get32v(&ptr->pgcen[2], &ptr->pgcen[2], 1);
    
    	get32v(&ptr->azim, &ptr->azim, 1);
    	get32v(&ptr->elev, &ptr->elev, 1);
    	get32v(&ptr->twist, &ptr->twist, 1);
    
    	ptr->packed_color = get32(&ptr->packed_color);
    
    } /* end swapDwbPage() */
    
#define CACHE_MAX 1024
    
#define UNPACK_ABGR(dst, src) \
         ((dst)[0] = (1.0f/255.0f)*(src)[3], \
          (dst)[1] = (1.0f/255.0f)*(src)[2], \
          (dst)[2] = (1.0f/255.0f)*(src)[1], \
          (dst)[3] = (1.0f/255.0f)*(src)[0])
    
    

/*----------------------------------------------*/
/* Macro to set the builder utility color array */
/*----------------------------------------------*/
#if 0
#define SET_COLOR(poly,i,color_index,packed_color)		\
{								\
    float tmp_arr[4]; int tmp_int;				\
    if ( color_index == PACKED_COLOR_INDEX )			\
    {								\
        unPackRGBA ( packed_color,tmp_arr );			\
        for ( tmp_int=0; tmp_int<3; tmp_int++ )			\
            poly->colors[i][tmp_int] = tmp_arr[tmp_int]; 	\
    }								\
    else							\
	pfCopyVec3 ( poly->colors[i],dwbRoot->colorTable[color_index] );\
    poly->colors[i][3] = dwbRoot->currentGroup.alpha;		\
}
#else
#define SET_COLOR(poly,i,color_index,packed_color)		\
     {								\
         if (color_index == PACKED_COLOR_INDEX)			\
             unPackRGBA(packed_color, poly->colors[i]);              \
         else 							\
     	pfCopyVec4(poly->colors[i],dwbRoot->colorTable[color_index]);\
         poly->colors[i][3] = dwbRoot->currentGroup.alpha;		\
     }
#endif


/*========================================================================
 *========================================================================
 *
 * 		             PRIVATE variables
 *
 *========================================================================
 *========================================================================*/
/* 
 * The Link loader package uses checkForLink otherwise just throw away any
 * unknown opcodes.
 */
 
#ifndef LINKLOADER

static int checkForLink( memPtr *ifp, short opcode, short length, 
                  pfNode *parent, pfNode **current, int treeDepth )
{
 
  /* comment this out if you do not want info on unknowns */ 
/*  printf("\t\tUnknown opcode %u \t: length %u \n",opcode,length); */

  /* throw away everything */
  memSeek(ifp,length,SEEK_CUR);

  return 1;
}

#else

extern int checkForLink( memPtr *ifp, short opcode, short length, 
                         pfNode *parent, pfNode **current, int depth );

extern void addSoundReference(int id);

#endif


typedef struct _LineStyle
{
	short index;
	short style;
	short repeat;

} LineStyle;


/*---------------------------------------------------------------------------*/
/*                             Global structures                             */
/*---------------------------------------------------------------------------*/
static short faceBackColor;
int externDepth =0;
#define MAX_EXTERN_DEPTH 100
dwbFile *externArray[MAX_EXTERN_DEPTH];
static int externChildYUp[MAX_EXTERN_DEPTH+1];

#ifdef LINKLOADER
LineStyle *lineStyles;          /* local linestyle pallete */
int numLineStyles = 0;
#else
static LineStyle *lineStyles;          /* local linestyle pallete */
static int numLineStyles = 0;
#endif


/*---------------------------------------------------------------------------*/
/*                               Global Variables                            */
/*---------------------------------------------------------------------------*/
       void             *Arena;             /*Performer shared memory Arena*/
static int               GfxType;
       dwbFile		*dwbRoot;	    /*local container for global vars */
static pfTexEnv         *defTEnv = NULL;    /*default texture environment */
static pfSphere groupBoundingSphere;
static int      currentFacePType = -1; /* current polygon type of the face */
static int      lastNodeIsDrawable = 0;  /* Last node read is drawable? */

static int      extendedVersion = 0;   /* version 2.28 and above */

static int clipMode =  CLIP_RECALCULATE;  /* by default  calculate clip
		        			 regions every frame  */

pfNode *LODChild = NULL; /* set by pfdwb.c used by links */
pfNode *LODParent = NULL; /* set by pfdwb.c used by links */
int encrypted = 0;

/*----------------------------------------------*/
/* It is crucial to share textures between      */
/* files - so these lists store textures for 	*/
/* sharing.					*/
/*----------------------------------------------*/

static pfTexture    *sharedTexList[DWB_MAX_TEXTURES]; /* XXX - should grow */
static pfTexEnv	    *sharedTevList[DWB_MAX_TEXTURES]; /* XXX - should grow */
static int 	    sharedTexCount = 0;

static pfVec4       nextBSplineColor; /* color to be attached to bspline */
				      /* record when read */
static pfSCS 	    *externSCSGroup; /* pointer to last extern SCS group read */
static  int         clipRegionsPresent;
static int          polyLength = 256;


typedef struct _ExternList
{
	char name[256];
	pfNode *node;
	struct _ExternList *next;
} ExternList;

typedef struct _ClipRegion
{
	pfChannel *channel;
        int  chanSize[2];
        int  chanOrigin[2];
	float ll[3];
	float ur[3];
	short min[2];
	short max[2];
	int   gotOld;
	short oldmin[2];
	short oldmax[2];

} ClipRegion;

static ExternList *externs=NULL;

/*----------------------------------------------*/
/* database counters 				*/
/*----------------------------------------------*/

static pfSCS        *childYupSCS;
static float        lineWidth = 1.0;

/*========================================================================
 *========================================================================
 *
 *				PROTOTYPES 
 *
 *========================================================================
 *========================================================================*/


static void    addLineToGeoState        (dwbGeoState *gstate, pfdGeom *polygon,
                                        int drawStyle);
static void    addLineList              (pfGeode *geode,dwbGeoState *gstate );
static int     applyDetailTexture       (short texture, short detail);
static void    cleanUp                  (void);
static int    clipRegionOff            ( pfTraverser *trav, void *data );
static int    clipRegionOn             ( pfTraverser *trav, void *data );
static int    drawArc                  ( pfTraverser *trav, void *data );
static void    dwbPop                   (pfNode **parent, pfNode **currentNode);
static void    dwbPush                  (pfNode **parent, pfNode *currentNode);
static void    freeLineList             (dwbGeoState *gstate );
static dwbGeoState *getDwbGeoState      (void);
static pfBillboard *getBillboard        (pfGeode *geode);
static int     getDwbTexture            (int id);
static pfMaterial *getPfMaterial        (dwbGeoState *gs);
static int     handleArc                (memPtr *ifp, short length,
					 pfNode *current,pfNode *parent);
static int     handleBSpline            (memPtr *ifp, short length, 
                                         pfNode *current, pfNode *parent);
static int     handleColorTable         (memPtr *ifp);
static int     handleComment            (memPtr *ifp, short length, 
                                         pfNode *currentNode, pfNode *parent);
static int     handleExternal           (memPtr *ifp,int length,pfNode *parent,
					 pfNode **currNode);
static void    handleDecal              (pfNode **parent );
static int     handleFace               (memPtr *ifp, pfNode *parent, pfdGeom *poly,
				  	dwbGeoState **currentGState,int *vType,
					pfNode *current);
static int     handleGroup              (memPtr *ifp, pfNode *parent, 
				         pfNode **curr, int *faceType);
static int     handleHeader             (memPtr *ifp, pfNode *currentNode);
static int     handleInstanceDef        (memPtr *ifp, pfNode *currentNode,
                                         pfNode *parent);
static int     handleInstanceRef        (memPtr *ifp, pfNode *currentNode,
				         pfNode *parentNode);
static int     handleLightPoint         (memPtr *ifp, pfdGeom *polygon,
                                         pfNode **parent, pfNode *current,
                                         int vertType);
static int     handleLineStyle          (memPtr *ifp, int size);
static int     handleMaterial           (memPtr *ifp);
static int     handleMatrix             (memPtr *ifp, pfNode **currentNode, 
                                         pfNode *parent);
static int     handlePackedVertex       (memPtr *ifp, int vertType, 
                                         pfdGeom *polygon, short length);
static int     handlePage               (memPtr *ifp, short length, 
                                         pfNode **currentNode, pfNode *parent);
static int     handleSwitch             (memPtr *ifp, pfNode *parent, 
                                         pfNode **currentNode);
static int     handleTexture            (memPtr *ifp);
static int     handleVertex             (memPtr *ifp, int vertType, 
                                         pfdGeom *polygon, short length);
static void    initLoader               (void);
static void    insertFaces              (pfNode *parent,int faceType);
static dwbGeoState *makeDwbGeoState     (void);
static pfGeoState *makePfGeoState       (dwbGeoState *gstate);
static pfNode *loadDwbFile              (memPtr *ifp);
static instRefList *newInstance         (void );
static instRefList *getInstance         (short val);
static memPtr  *openFile                 (char *fileName);
static char   *readVariableLengthString (memPtr *ifp, int length);
static void    setUpPolygonBindTypes    (pfdGeom *polyptr,int vert_type);
static int     setZbufferOp             (pfTraverser *trav,void *data);
static void    unCopyrightNotice        (void);
       void    unPackRGBA               (unsigned int pcolor,float *rgba);
static int     unsetZbufferOp           (pfTraverser *trav,void *data);
static int parseToOpcode(memPtr *ifp, short searchOpcode);
int lineWidthOff ( pfTraverser *trav, void *data) ;
int lineWidthOn ( pfTraverser *trav, void *data );

int memEof( memPtr *ptr);
extern setRootNode (pfGroup *);
extern linkEnd (pfNode *);
extern int decode (dwbHeader *);
extern mypfLoadTexFile (pfTexture *, char *);
int map_viewport (ClipRegion *,pfChannel *, float *,float *,short *,short *);
int initUserFuncs(void);



/*========================================================================
 *========================================================================
 *
                       USER DEFINED FUNCTIONS

   An application can define a callback to be executed for almost every
   dwbRecord. Callbacks are defined by calling setUserFunc with the appropriate
   callback address and the RECORD type as defined in pfdwb.h

   The callback is called immediately after the record is read, and if it
   returns a negative number the loader will abort any further processing of 
   the record in question. A non negative return value will mean that the 
   loader will continue as usual after the callback.

   The callback must accept the following as paramaters

	type - record type as defined by pfdwb.h
	rec  - pointer to the record just read
	current - pointer to the current Node position
	parent - pointer to the parent of the current Node position
        ifp  - pointer to the current position in the file.

 *
 *========================================================================
 *========================================================================*/


/* macro to check the use defined functions of each opcode */

#define USERFUNCS(opcode,rec,curr,parent,ifp) \
  if (  userFunctions[(opcode) - DB_HEADER].func != NULL) {	\
    if ( userFunctions[(opcode) - DB_HEADER].func( (opcode),	\
                      (rec), (curr), (parent),(ifp)) < 0)		\
        return(1);						\
  }								\

typedef struct _FuncEntry
{
  int (*func)(int type, void *rec, pfNode *current, pfNode *parent, memPtr *ifp);
} FuncEntry;

static FuncEntry userFunctions[DB_LASTREC - DB_HEADER]; 

typedef struct _BsplineInfo
{
  int	numKnots;

  GLfloat *knots;
  GLfloat *cntlPnts;
  GLUnurbsObj *nobj;

  pfVec4 color;
} BsplineInfo;

typedef struct _ArcInfo
{
  float fRadius, fStartAngle, fEndAngle;
  float matrix[4][4];
  float pfNormal[3];
  short drawstyle;
  pfVec4 color;
} ArcInfo;

/*
 * The following routine parses the scene group and looks for
 * instances of one group having a single group as a child.
 * If any are found they are removed. DWB files often have
 * single group hierarchies and always have a group before
 * a renderable node. 
 */

static void removeRedundantGroups ( pfNode *_parent, pfNode *_node )
{
int type, i;
pfNode *tmp;

  if ( _node && _parent != _node)
  {
  int nc;

  if (pfIsOfType(_node, pfGetGroupClassType()) &&
      !pfIsOfType(_node, pfGetLODClassType()))
    {
      nc = pfGetNumChildren(_node);

      if (nc == 0)   /* group with no children is useless(except for callback and linkss!!)*/
      {
        pfRemoveChild(_parent, _node);
	pfDelete(_node);
	_node = NULL;
      }
      else if (nc == 1)/* remove intermediate group */
      {
        tmp = pfGetChild(_node,0);
        pfReplaceChild(_parent,_node,tmp);
        pfDelete(_node);
	_node = tmp;	
      }
     }
  }
  
  if ((_node != NULL) && pfIsOfType(_node, pfGetGroupClassType()))
  {
  int nc = pfGetNumChildren(_node);

    /*
     * Recurse on children.
     */
    for(i = nc-1; i >= 0; i--)
      removeRedundantGroups( _node, pfGetChild(_node,i) );
  }
}


static int parseToOpcode(memPtr *ifp, short searchOpcode)
{
  short opcode, length;
  int found = 0;

  while ( !memEof (ifp) && !found)
  {
      memRead(&opcode,sizeof(short),1,ifp);
	  opcode = get16(&opcode);
      if (searchOpcode == opcode)
          found = TRUE;
      else
      {
	memRead(&length,sizeof(short),1,ifp);
    length = get16(&length);
	if (length > 0)
            memSeek(ifp,length,SEEK_CUR);
      }
  }

  return(found);
}

/*========================================================================
 *========================================================================
 *
 * ENTRY POINT from the outside world. Routine that will load a dwb file
 * and convert that into a performer nodal tree. Returns a pointer to the
 * first element in the tree.
 *
 * Can be compiled for editting and relinking into EasyScene
 * or DSO to be used with pfdLoadFile.
 *
 *========================================================================
 *========================================================================*/
 
#ifdef __ESCENE__
extern pfNode *pfdLoad_dwb ( char *fileName )
#else
/*extern */PFDWB_DLLEXPORT pfNode *pfdLoadFile_dwb ( char *fileName )
#endif
{
memPtr *ifp;
pfNode *node=NULL;
short opcode;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

  /* --------------------------------------- */
  /* open file and check that it is readable */
  /* --------------------------------------- */
  if ( !(ifp = openFile( fileName )) )
    return( NULL );

  /* ----------------------------------------------------- */
  /* let the world know that this format is NON-proprietry */
  /* ----------------------------------------------------- */
  unCopyrightNotice();

  /* --------------------------------------------------------------- */
  /* if the first opcode is not DB_HEADER then the format is _WRONG_ */
  /* --------------------------------------------------------------- */
  memRead(&opcode,sizeof(short),1,ifp);
  opcode = get16(&opcode);
  if (opcode != DB_HEADER)
  {
    pfNotify(PFNFY_WARN,PFNFY_RESOURCE,
	"LoadDwb: File format error. Cannot load %s",fileName);
    if (opcode == 1)
    {
       pfNotify(PFNFY_WARN,PFNFY_RESOURCE,
           "LoadDwb: Format could be early .flt");
    }
    cleanUp();
  }
  else
  {
    strncpy(dwbRoot->fname, fileName, PF_MAXSTRING);
    memSeek(ifp, 0, SEEK_SET);
    node = loadDwbFile( ifp );
  }
  
 
  /*
   * Now if this is not a linked file then remove any redundant
   * groups this changes the structure from a one to one mapping 
   * with the dwb hierarchy but should result in a more optimal
   * scene if clip regions exist within the model then they may
   * be on nodes with no geometry
   */
  if (dwbRoot && !dwbRoot->containsLinks && !clipRegionsPresent)
    {
      removeRedundantGroups(node, node);
    }
  /*  
   {
  FILE *ifp;

    ifp = fopen ("scene.out","w");
    pfPrint(node, PFTRAV_SELF|PFTRAV_DESCEND, PFPRINT_VB_INFO, ifp);
    fclose(ifp);
  }
  */
    
  return (node);
}


/*========================================================================
 * Real loading routine. Reads opcodes from the file and processes them
 *======================================================================== */

static pfNode *loadDwbFile( memPtr *ifp )
{
unsigned short opcode,length;
pfGroup *top;
pfNode *currentNode, *parentNode;
int depth;
int ok = 1;
int renderFace = -99;
dwbGeoState *currentGState;
int vertType;
/* allocate a new primitive buffer able to hold up to 256 vertices */
pfdGeom *polygon = pfdNewGeom (polyLength);
int faceType;

  /* create top level container node */
  top = pfNewGroup();

  dwbRoot->root = top;
  currentNode = parentNode = (pfNode *) top;
  clipRegionsPresent =1;
  
#ifdef LINKLOADER
  /* to allow top level links to work we need different parent/current nodes */
  currentNode = (pfNode *) pfNewGroup();
  pfAddChild(parentNode,currentNode); 
#endif

  depth = 0;
  /* ----------------------------------------------- */
  /* main loop, reads each opcode and then calls the */
  /* appropriate handling routines.                  */
  /* ----------------------------------------------- */
  while ( !memEof (ifp) && ok )
  {
	/* read the opcode and length */
	memRead(&opcode,sizeof(short),1,ifp);
	memRead(&length,sizeof(short),1,ifp);
    	opcode = get16(&opcode);
    	length = get16(&length);
        switch ( opcode )
        {
            case DB_HEADER :
		ok = handleHeader(ifp, currentNode);
                lastNodeIsDrawable = 0;
                break;

            case DB_COLOR_TABLE:
                ok = handleColorTable(ifp);
                break;

            case DB_MATERIAL_TABLE:
                ok = handleMaterial(ifp);
                break;

            case DB_TEXTURE_TABLE:
		ok = handleTexture(ifp);
                break;


            case DB_GROUP:
                handleGroup(ifp,parentNode,&currentNode,&faceType);
                lastNodeIsDrawable = 0;
                break;


            case DB_FACE:
		/* only attach faces when we go back to parent level */
		renderFace = dwbRoot->treeDepth -1;
                ok = handleFace(ifp,parentNode,polygon,
				&currentGState, &vertType,currentNode);
                lastNodeIsDrawable = 1;
                break;

            case DB_SWITCH:  /* LOD */
		handleSwitch(ifp, parentNode, &currentNode);
                lastNodeIsDrawable = 0;
                break;

           case DB_PUSH:
		if (length > 0)
                  memSeek(ifp,length,SEEK_CUR);

		dwbPush(&parentNode,currentNode);
                break;

           case DB_PACKED_VERTEX:
		{
                int drawStyle = dwbRoot->currentGroup.drawstyle;

		handlePackedVertex(ifp,vertType, polygon,length);
                /* add verts to triangle or line set */
                if (polygon->numVerts < 3 ||
                   (drawStyle != DS_SOLID && drawStyle != DS_SOLID_BOTH_SIDES))
                     addLineToGeoState(currentGState,polygon,drawStyle);
                else if (currentGState->builder )
                         pfdAddPoly( currentGState->builder, polygon);

		}
		break;

  	   case DB_VERTEX:
		{
                int drawStyle = dwbRoot->currentGroup.drawstyle;

		ok = handleVertex(ifp,vertType, polygon,length);
                /* add verts to triangle or line set */
                if (polygon->numVerts < 3 || 
                   (drawStyle != DS_SOLID && drawStyle != DS_SOLID_BOTH_SIDES))
                     addLineToGeoState(currentGState,polygon,drawStyle);

                else if (currentGState->builder )
                         pfdAddPoly( currentGState->builder, polygon);

		}
		break;

  	   case DB_LIGHT_PT:
		handleLightPoint(ifp,polygon,&parentNode,currentNode,vertType);
                lastNodeIsDrawable = 1;
                vertType += LIGHTPOINT;
		break;

           case DB_POP:
		/* 
		 * Make sure we are back in the right level and also that the
		 * face.ptype is NOT BSpline.
		 */

		if (renderFace == dwbRoot->treeDepth -1 && 
		    currentFacePType != 3 && currentFacePType != 4) 
                {
		  insertFaces(parentNode,faceType);
		  renderFace = -99;
		  currentFacePType = -1;
                }

		if (length > 0)
                  memSeek(ifp,length,SEEK_CUR);

		dwbPop(&parentNode,&currentNode);

		if ((! extendedVersion) || 
		    (extendedVersion && dwbRoot->renderGroup))
		{
		  /* if we have just finished reading the verts for a face */
		  /* and that face is the first of a decal group */
                  /* then decal that face */

		  if (faceType == LAYER_BASE)
                  {
		    handleDecal(&parentNode);
		    dwbRoot->renderGroup = 0;
		    faceType = LAYER_DECAL; /* only first face is decal */
		  }
		  else if (faceType == SEQUENCE)
		  {
		    insertFaces (parentNode, SEQUENCE);
                  }
                }
              break;

            case DB_NAME_STR:
                /*----------------------------------------------*/
                /* all nodes except palettes & vertices have a  */
                /* name string immediately after the node       */
                /*----------------------------------------------*/
		{
		  char *strVal;
		  strVal = readVariableLengthString(ifp,length);

		  /* attach onto node name */
		  if ((strlen(strVal) > 0) && (!lastNodeIsDrawable) )
		    pfNodeName(currentNode,strVal);

                  pfFree(strVal);
		}
                break;

	    case DB_VIEW_PARAMS:
		memSeek(ifp,length,SEEK_CUR);
		break;

	    case DB_LINESTYLE_TABLE:
		handleLineStyle(ifp,length);
		break;

	    case  DB_COMMENT :
	        handleComment(ifp,length,currentNode,parentNode);	
                break;

	    case  DB_PAGE :
	        handlePage(ifp,length,&currentNode,parentNode);	
                lastNodeIsDrawable = 0;
                break;

	    case  DB_IMAGE :
		memSeek(ifp,length,SEEK_CUR);
                lastNodeIsDrawable = 1;
                break;

	    case  DB_LSOURCE :
		memSeek(ifp,length,SEEK_CUR);
                break;

	    case  DB_LMODEL :
		memSeek(ifp,length,SEEK_CUR);
                break;

	    case  DB_MATRIX :
		handleMatrix(ifp, &currentNode, parentNode);
                break;

	    case DB_EXTERNAL:  /* external file reference */
		ok = handleExternal(ifp,length,parentNode,&currentNode);
                lastNodeIsDrawable = 0;
		break;

	    case DB_INSTANCE_DEF:
		   handleInstanceDef(ifp,currentNode,parentNode);
		break;

	    case DB_INSTANCE_REF:
		   handleInstanceRef(ifp,currentNode,parentNode);
		break;

	   /*list of opcodes that contain info that only the dwb tool requires*/
	    case DB_CONSTRUCTION:
	    case DB_TEXMAP:
		  /* skip past */
		  memSeek(ifp,length,SEEK_CUR);
		break;

	    case DB_BSPLINE:
		handleBSpline(ifp,length,currentNode,parentNode);
                lastNodeIsDrawable = 1;
                break;

            case DB_ARC:
		handleArc(ifp, length, currentNode, parentNode);
                lastNodeIsDrawable = 1;
		break;

            default:
		checkForLink( ifp, opcode, length, parentNode, &currentNode,
			      dwbRoot->treeDepth);
                break;
        }
  }

  /* 
   * Some instances have been referenced before they were defined. Fix
   * those.
   */

  if (dwbRoot->refBeforeDef) {
      refBeforeDefList *ptr = dwbRoot->refList;
      pfSCS *scs;
      instRefList *defPtr;
      pfMatrix rot;

    while (ptr) {
      defPtr = getInstance (ptr->inst.val);

      if (! defPtr) {
        fprintf (stderr, "LoadDwb: Instance referenced but never defined\n");
        continue;
      }

      scs = pfNewSCS(ptr->inst.mat);
      pfAddChild(ptr->parent, scs);
      pfAddChild(scs, defPtr->node);

      ptr = ptr->next;
    }
  }
    
  /* if the Y_UP flag is set then rotate the entire tree by 90 degrees */
  if ( dwbRoot->header.up_axis == Y_UP )
  {
    pfSCS *scs;
    pfMatrix rot;

    pfMakeEulerMat(rot,0.0,90.0,0.0);
    scs = pfNewSCS(rot);
    pfAddChild(scs,top);
    top = (pfGroup *) scs;

    /* store this value so its visible to any parent thats loadin an external */
    childYupSCS = scs;

    /* flatten any SCS nodes found */
    if (strstr(dwbRoot->fname, ".dwb") != NULL && !dwbRoot->containsDCS && 
        !externDepth)
    {
       pfFlatten(top,0); 
    }
 
  }

#ifdef LINKLOADER
/* this is the only time when the total tree is known - inform the link loader */
  setRootNode(top);
  linkEnd(currentNode);
#endif

  cleanUp();

  if (!ok)
    pfNotify(PFNFY_WARN, PFNFY_USAGE, "LoadDwb: Error in file format. \n");

  return ((pfNode *) top);
}


/*========================================================================
 * free all temporary memory used by this iteration of the loader
 *======================================================================== */

static void cleanUp( void )
{
int i;

  if (dwbRoot)
  {
    if (dwbRoot->materials)     pfFree( dwbRoot->materials);
    if (dwbRoot->materialTable) pfFree( dwbRoot->materialTable );
    if (dwbRoot->textureTable)  
    {
      for (i=0; i<dwbRoot->texturesRead; ++i)
      {
        pfFree (dwbRoot->textureTable[i].name);
      }
      pfFree( dwbRoot->textureTable );
    }
    if (dwbRoot->stack)         pfFree( dwbRoot->stack);

    if (lineStyles)
      pfFree( lineStyles); /* free the local line style pallete */
    lineStyles = NULL;

    /* free up the geostates for this model */
   if (dwbRoot->gsList)
   {
     dwbGeoState *tmp,*ptr = dwbRoot->gsList;
     while(ptr)
     {
       tmp = ptr->next;
       if (ptr->builder) pfdDelGeoBldr (ptr->builder);
       pfFree(ptr);
       ptr = tmp;
     }

   }
   /* free up the memory buffer */
   pfFree(dwbRoot->file.ptr);    

  }

  pfFree(dwbRoot);
  dwbRoot = NULL;
}


/*========================================================================
 * create a new entry in this files instance list 
 *======================================================================== */

static instRefList *newInstance(void )
{
instRefList *ptr;

  ptr = pfCalloc(1, sizeof(instRefList), NULL);
  if (dwbRoot->instList)
    ptr->next = dwbRoot->instList;

  dwbRoot->instList = ptr;

  return(ptr);
}


static refBeforeDefList *newReference (void) {
    refBeforeDefList *ptr;

  ptr = pfCalloc (1, sizeof(refBeforeDefList), NULL);
  if (dwbRoot->refList)
    ptr->next = dwbRoot->refList;

  dwbRoot->refList = ptr;

  return (ptr);
}


/*========================================================================
 * Search this files instance list and return the node that
 * matches this instance value
 *======================================================================== */

static instRefList *getInstance(short val)
{
instRefList *ptr = dwbRoot->instList;

  while (ptr)
  {
    if (ptr->inst.val == val)
      return(ptr);
 
    ptr = ptr->next;
  }
  return(NULL);
}


/*========================================================================
 * read an instance definition, and add that to
 * the existing definitions for this file
 *======================================================================== */

static int handleInstanceDef(memPtr *ifp, pfNode *currentNode,pfNode *parent)
{
instRefList *ptr;

  /* instance def refers to last node read */
  /* so add the current node onto the instance list */
  ptr = newInstance();

  memRead(&(ptr->inst),sizeof(dwbInstanceDef),1,ifp)
  swapDwbInstanceDef(&(ptr->inst));
  /* call any user functions for this opcode */
  USERFUNCS(DB_INSTANCE_DEF,&ptr->inst,currentNode,parent,ifp);

  ptr->node = currentNode;
  return(1);

}


/*========================================================================
 * read an instance reference, extract this references definition
 * and then add the original definition into the tree at this point 
 * The instance is not cloned so it shares ALL information with the
 * original definition.
 *======================================================================== */

static int handleInstanceRef(memPtr *ifp, pfNode *current, pfNode *parentNode)
{
dwbInstanceRef inst;
instRefList *ptr;
pfSCS *scs;
pfMatrix rot;

  memRead(&inst,sizeof(dwbInstanceRef),1,ifp);
  swapDwbInstanceRef(&inst);
  USERFUNCS(DB_INSTANCE_REF, &inst, current, parentNode,ifp);

  ptr = getInstance(inst.val);
  if (!ptr)
  {
      refBeforeDefList *refPtr;

    dwbRoot->refBeforeDef = 1;
    refPtr = newReference();
    refPtr->parent = parentNode;
    refPtr->inst = inst;
    return (0);
  }
 
  /* give this instance its position and attach that to the tree */
  scs = pfNewSCS(inst.mat);
  pfAddChild(parentNode,scs);

  /* Instances share everything with the original instance */
  /* so just attach the node again */ 
  pfAddChild(scs,ptr->node);

  dwbRoot->containsInstancing = 1;

  return(1);

}


/*========================================================================
 * HandleLineStyle  - build the linestyle table
 *======================================================================== */

static int handleLineStyle(memPtr *ifp, int size)
{
int numRecords;
int count;
dwbLineStyle lstyle;

  numRecords = size / sizeof(dwbLineStyle);

  numLineStyles = numRecords;
  
  lineStyles = pfCalloc( numRecords, sizeof(LineStyle),Arena);

  /* read in the records */
  for (count =0; count < numRecords; count++)
  {
    memRead(&lstyle,sizeof(dwbLineStyle),1,ifp);
	swapDwbLineStyle(&lstyle);
    lineStyles[count].index = count;
    lineStyles[count].style = lstyle.lstyle;
  }

  /* define the linestyles */
  /* NOTE: current implementation will share linestyle definitions */
  /* if you load more than one dwb with different linestyle defs for the */
  /* same index values then the last definition will rule */
  /* define all the linestyles for this model */

  return 1;
}


/*========================================================================
 * Read a matrix record and attach that to the current position in
 * the performer tree. The currentNode is updates but the parent not
 * to ensure that when a pop is encountered the correct level is returned
 * to
 *======================================================================== */

static int handleMatrix( memPtr *ifp, pfNode **currentNode, pfNode *parent)
{                
pfSCS *scs;
pfMatrix mat;
dwbMatrix dwbMat;
float largest;


  /* --------------------------------------------- */
  /* Matrix always follows node that it references */
  /* just add a SCS for the currentNode            */
  /* --------------------------------------------- */
  memRead(&dwbMat,sizeof(dwbMatrix),1,ifp);
  swapDwbMatrix(&dwbMat); 
  USERFUNCS(DB_MATRIX, &dwbMat, *currentNode, parent,ifp);

  pfCopyMat(mat, dwbMat.mat);
  
  scs = pfNewSCS(mat);
 
  /* insert SCS infront of currentNode */
  pfRemoveChild(parent, *currentNode);
  pfAddChild(parent,scs);
  pfAddChild(scs,*currentNode);

  /* update the current node */
  *currentNode = (pfNode *)scs; 

  /* extract the scale value from the matrix and set that to the lod scale */
  /* this is not depth sensitive - EVERYTHING after this point will be scaled */
  /* by this amount */

  largest = dwbMat.mat[0][0];
  if (dwbMat.mat[1][1] > largest)
    largest = dwbMat.mat[1][1];
  if (dwbMat.mat[2][2] > largest)
    largest = dwbMat.mat[2][2];
 
  dwbRoot->lodScale = largest;

  return(1);
}


/*========================================================================
 * Read a comment and then call any user supplied calls
 * the loader does nothing with comment so this handling routine exists
 * solely for user callabcks.
 *======================================================================== */

static int handleComment( memPtr *ifp, short length, pfNode *currentNode, 
                          pfNode *parent)
{
short *space;

  space = pfMalloc(length,0);
  memRead(space,length,1,ifp);

  USERFUNCS(DB_COMMENT, space, currentNode, parent,ifp);

  pfFree(space);

  return 1;
}


static int handlePage( memPtr *ifp, short length, pfNode **currentNode, 
                          pfNode *parent)
{
dwbPage page;
pfSwitch *sw;

  memRead(&page,length,1,ifp);
  swapDwbPage(&page);
  USERFUNCS(DB_PAGE, &page, *currentNode, parent,ifp);

  /* create a container node for children of the page */
  *currentNode = (pfNode*)  pfNewGroup();
  pfAddChild(parent, *currentNode);

#ifdef LINKLOADER
  {
    extern void addPageToList(pfNode *parent, pfNode *currentNode, int id, int active);
    addPageToList(parent, *currentNode, page.pageid, page.flags.active);
  }
#else
  if (!page.flags.active)
  {
      sw = pfNewSwitch();
      pfSwitchVal(sw, PFSWITCH_OFF);
      pfAddChild(parent,sw);
      pfRemoveChild(parent,*currentNode);
      pfAddChild(sw, *currentNode);
  }
#endif

  return 1;
}

/*========================================================================
 * Read Header record and process it
 *======================================================================== */

static int handleHeader( memPtr *ifp, pfNode *currentNode )
{
int ok=1;

  /* -------------------------------------------------------- */
  /* read the header and set it into the global header record */
  /* A global record is used cause lots of things need access */
  /* to this structure.			 		      */ 
  /* -------------------------------------------------------- */
  memRead( &dwbRoot->header,sizeof(dwbHeader),1,ifp);
  swapDwbHeader(&dwbRoot->header);
#ifndef ENCRYPT
    /* check that the object has the correct encryption */
#ifdef __ESCENE__
    if (!decode(&dwbRoot->header))
    {
      encrypted = 0;
    }
    else
      encrypted = 1;
#endif

#else
    if (!decode(&dwbRoot->header))
    {
      pfNotify(PFNFY_WARN, PFNFY_USAGE, "\n\nError: Non Demo Model Load Attemp\n");
      exit(1);
    }
#endif

  USERFUNCS(DB_HEADER, &dwbRoot->header, currentNode, NULL,ifp);

  switch (dwbRoot->header.units)
  {
      case UT_FEET:
          dwbRoot->unitChange = (float)FEET_TO_METERS;
          break;

      case UT_INCHES:
          dwbRoot->unitChange = (float)INCHES_TO_METERS;
          break;

      case UT_KILOMETERS:
          dwbRoot->unitChange = (float)KM_TO_METERS;
          break;

      case UT_CENTIMETERS:
          dwbRoot->unitChange = (float)CM_TO_METERS;
          break;

      case UT_NAUTICAL_MILES:
          dwbRoot->unitChange = (float)NM_TO_METERS;
          break;

      default:
          dwbRoot->unitChange = 1.0;
          /*UT_METERS*/
          break;
  }


  if (dwbRoot->header.version < (LOWEST_DWB_FORMAT_VERSION_SUPPORTED 
				 - ROUNDING_ERROR) )
  {
    pfNotify (PFNFY_WARN, PFNFY_USAGE,"\n\n" );
    pfNotify (PFNFY_WARN, PFNFY_USAGE,"***** WARNING *****\n" );
    pfNotify (PFNFY_WARN, PFNFY_USAGE,"*\n" );
    pfNotify (PFNFY_WARN, PFNFY_USAGE,"* DWB-Performer loader only supports format revision 2.1 and above\n" );
    pfNotify (PFNFY_WARN, PFNFY_USAGE,"*\n" );
    pfNotify (PFNFY_WARN, PFNFY_USAGE,"***** WARNING *****\n" );
    pfNotify (PFNFY_WARN, PFNFY_USAGE,"\n\n" );
    return ( FALSE );
    }

  /* This is mainly for new Texture structures and Decal groups */
  if (dwbRoot->header.version > DWB_3_0_FORMAT_VERSION -
	                        ROUNDING_ERROR) {
    extendedVersion = 1;
  }
  else {
    extendedVersion = 0;
  }


  /* set up the overriding zbuffer state - if header says no zbuffering */

  if ( !dwbRoot->header.zbuffer)
  {
    pfNodeTravFuncs ( currentNode,PFTRAV_DRAW,unsetZbufferOp,setZbufferOp );
  }

  /* store up axis so that any externals can signal to there parent what */
  /* orientation that they have. This is used only in handleExternal */
  externChildYUp[externDepth] = dwbRoot->header.up_axis;

  return(ok);

}


/*========================================================================
 *
 * FUNCTION     : unCopyrightNotice
 * IN           : nothing
 * OUT          : nothing
 * DESCRIPTION  : make sure the world knows its using a Designer's Workbench
 *                file.
 *
 *========================================================================*/

static void unCopyrightNotice ( void )
{
static int firstTime =1;

  /* Only want the uncopyright notice once per application */

  if (firstTime)
  { 
    fprintf ( stderr,"==========================================================================\n" );
    fprintf ( stderr,"Coryphaeus Software DWB Version 4.0 Loader for Iris Performer 2.x 03/13/97\n" );
    fprintf ( stderr,"==========================================================================\n" );
    fprintf ( stderr,"Unlimited License: The Designer's Workbench (DWB) loader software contained\n");

    fprintf ( stderr,"in this program is in the PUBLIC DOMAIN. There are NO restrictions in the\n");
    fprintf ( stderr,"use of this software. For further information about the loader and other \n" );
    fprintf ( stderr,"Performer related products contact: Coryphaeus Software, 985 University Av.\n" );
    fprintf ( stderr,"Suite 31, Los Gatos, CA 95030. Tel (408) 395-4537, Fax: (408) 395-6351\n");
    fprintf ( stderr,"==========================================================================\n" );

    firstTime = 0;
  }
}


/*========================================================================
 *
 * FUNCTION     : initLoader
 * DESCRIPTION  : init variables, counters etc
 *
 *========================================================================*/

static void initLoader ( void )
{
    int sharpen, multisample = 1;
   
  /*------------------------------------------*/
  /* Get Performer shared memory Arena        */
  /*------------------------------------------*/
  Arena = pfGetSharedArena();

  /*------------------------------------------*/
  /* Determine graphics configuration         */
  /*------------------------------------------*/
  GfxType = 0;

  pfQueryFeature (PFQFTR_TEXTURE_SHARPEN, &sharpen);
  pfQueryFeature (PFQFTR_MULTISAMPLE, &multisample);

  if (sharpen) GfxType |= GFX_REALITY;
  if (multisample) GfxType |= GFX_MULTISAMPLE;

  /*------------------------------------------*/
  /* Create Re-entrant safe  global variables */
  /*------------------------------------------*/

  if ((dwbRoot = pfCalloc(1,sizeof(dwbFile),NULL)) == NULL) {
    pfNotify (PFNFY_FATAL, PFNFY_RESOURCE, 
      "LoadDwb: Insufficient memory to load file\n");
  }

  if (!defTEnv) /* created once and once only */
    {
    defTEnv = pfNewTEnv(Arena);
    pfRef(defTEnv);  /* force an extra reference so that will not
                        be removed by any subsequent pfDelete */
    }

  /*------------------------------------------*/
  /* alloc max number of materials per file.. */
  /*------------------------------------------*/
  dwbRoot->materials = 
    (pfMaterial**) pfCalloc(DWB_MAX_MATERIALS, sizeof(pfMaterial*), NULL);


}


/*========================================================================
 * read a string into locally created memory. The caller is responsible
 * for freeing that memory
 *
 *========================================================================*/

static char *readVariableLengthString ( memPtr *ifp, int length )
{
    static char *name;

    /* ----------------------------------------------------------------- */
    /* allocate space for the name and make sure that it is quad aligned */
    /* ----------------------------------------------------------------- */
    name = (char *) pfCalloc (((length/16)+ 1)*16,sizeof(int),NULL );

    memRead(name,sizeof(char),length,ifp);
    name[length] = '\0';

    if ( strlen(name) > (PF_MAXSTRING-1) )
        name[PF_MAXSTRING-1] = '\0';

    return ( name );                    /* Must be ok if we got this far */
}


/*========================================================================
 *
 * FUNCTION     : makeStructArray
 * IN           : number of structs to malloced,sizeof data & ptr to existing
data
 * OUT          : ptr to new array.
 * DESCRIPTION  : alloc array of structures.
 *
 *========================================================================*/

static void *makeStructArray ( int numstructs,int size,void *old_data_ptr )
{
    void *mptr = NULL;
    int new_data_size;

    if ( numstructs)
    {
        new_data_size = ((numstructs * size)/16 +1) *16;

        if ( old_data_ptr )     /* realloc required */
            mptr = pfRealloc ( old_data_ptr,new_data_size );
        else
            mptr = pfCalloc ( 1,(size /16 + 1)*16,NULL );
    }

    return ( mptr );
}


/*========================================================================
 *
 * FUNCTION     : handleTexture
 * IN           : current File Pointer
 * OUT          : success(1) or failure (0)
 * DESCRIPTION  : read the dwb texture record from disk
 *
 *========================================================================*/

static int handleTexture ( memPtr *ifp )
{
dwbOpcodeRec command;
char *fname;
dwbExtTexInfo texInfo;  /* New texture information structure */

  /* increase the space reserved for the material table */
  dwbRoot->textureTable = (dwbTexture *)
                  makeStructArray ( dwbRoot->texturesRead+1, sizeof(dwbTexture),
                                  (void *)dwbRoot->textureTable );

  /* update the quick access texture -> pfTex table */
  dwbRoot->pfTexIndex = 
                   makeStructArray( dwbRoot->texturesRead+1, sizeof(int),
			            (void *) dwbRoot->pfTexIndex);

  dwbRoot->pfTexIndex[dwbRoot->texturesRead] = -1;

  /* read in the material and increment the global material count   */
  /* Since the 1st few fields in version 3.0 and the lower versions */
  /* are the same, read in the contents into the same structure.    */
  /* But depending upon the version, the rest of the fields may or  */
  /* may not have valid information.                                */
  
  if (extendedVersion) {
    memRead(&texInfo,sizeof(dwbExtTexInfo),1,ifp);
	swapDwbExtTexInfo(&texInfo);
  }
  else {
    memRead(&texInfo,sizeof(dwbTexInfo),1,ifp);
	swapDwbTexInfo((dwbTexInfo *)&texInfo);
    /* bzero((void *)(&texInfo+sizeof (dwbTexInfo)), 
		   sizeof(dwbExtTexInfo)-sizeof(dwbTexInfo));  */
  }

  /* read the texture name */
  memRead(&command, sizeof(dwbOpcodeRec),1,ifp);
  swapDwbOpcodeRec(&command);
  fname = readVariableLengthString(ifp,command.size); 

  if (extendedVersion)
  {
  dwbTexture tex;

    tex.name = fname;
    bcopy ((void *) &texInfo, (void *) &tex.tex, sizeof (dwbExtTexInfo));
    USERFUNCS (DB_TEXTURE_TABLE, &tex, NULL, NULL,ifp);
    bcopy ((void *) &tex.tex, (void *) &texInfo, sizeof (dwbExtTexInfo));
  }
  else USERFUNCS (DB_TEXTURE_TABLE, &texInfo, NULL, NULL,ifp);

  /* add new values to the texture table and increment counter */
  dwbRoot->textureTable[dwbRoot->texturesRead].name = fname; 
  dwbRoot->textureTable[dwbRoot->texturesRead++].tex = texInfo; 


  if (!fname )
    return(0);
  else
    return(1);

}


/*========================================================================
 *
 * FUNCTION     : handleMaterial
 * IN           : current File Pointer
 * OUT          : success(1) or failure (0)
 * DESCRIPTION  : read the dwb Material record from disk
 *
 *========================================================================*/

static int handleMaterial ( memPtr *ifp )
{
 
  /* increase the space reserved for the material table */
  dwbRoot->materialTable = (dwbMatInfo *)
                           makeStructArray ( dwbRoot->materialsRead+1, 
			         	     sizeof(dwbMatInfo),
                                             (void *)dwbRoot->materialTable );

  /* read in the material and increment the global material count */
  memRead(&(dwbRoot->materialTable[dwbRoot->materialsRead++]),
                  sizeof(dwbMatInfo),1,ifp);

  swapDwbMatInfo(&(dwbRoot->materialTable[dwbRoot->materialsRead-1]));

  USERFUNCS(DB_MATERIAL_TABLE,&dwbRoot->materialTable[dwbRoot->materialsRead-1],
            NULL,NULL,ifp);

  return(1);

}


/*========================================================================
 *
 * FUNCTION     : readDwbColorPalette
 * IN           : nothing
 * OUT          : success(1) or failure (0)
 * DESCRIPTION  : read the dwb color table from disk & convert it to
 *                a floating pt RGB table in the dwbFile struct.
 *
 *========================================================================*/

static int handleColorTable ( memPtr *ifp )
{
register int x, y, z;
dwbColorTable fileValues;

  memRead(&fileValues,sizeof(dwbColorTable),1,ifp)
  swapDwbColorTable(&fileValues);

  USERFUNCS(DB_COLOR_TABLE, &fileValues, NULL, NULL,ifp);

  /*----------------------------------*/
  /* calculate variable color shades  */
  /*----------------------------------*/
  for(x=0; x<30; x++)
  {
    for(y=0; y<128; y++)
    {
      dwbRoot->colorTable[x*128+y][0]=
                                 (float)((fileValues.shaded[x][0]*y)/127)/255.0;
      dwbRoot->colorTable[x*128+y][1]=
                                 (float)((fileValues.shaded[x][1]*y)/127)/255.0;
      dwbRoot->colorTable[x*128+y][2]=
                                 (float)((fileValues.shaded[x][2]*y)/127)/255.0;
    }
  }
  /*----------------------------------*/
  /* fixed colors                     */
  /*----------------------------------*/
  x = 4096;
  for ( z=0; z<64; z++ )
  {
    dwbRoot->colorTable[x][0] = ( (float)fileValues.fixed[z][0] ) / 255.0;
    dwbRoot->colorTable[x][1] = ( (float)fileValues.fixed[z][1] ) / 255.0;
    dwbRoot->colorTable[x][2] = ( (float)fileValues.fixed[z][2] ) / 255.0;

    x++;
  }
  /*----------------------------------*/
  /* overlay colors                   */
  /*----------------------------------*/
  x = 4160;
  for ( z=0; z<16; z++ )
  {
    dwbRoot->colorTable[x][0] = ( (float)fileValues.overlay[z][0] ) / 255.0;
    dwbRoot->colorTable[x][1] = ( (float)fileValues.overlay[z][1] ) / 255.0;
    dwbRoot->colorTable[x][2] = ( (float)fileValues.overlay[z][2] ) / 255.0;

    x++;
  }
  /*----------------------------------*/
  /* underlay colors                  */
  /*----------------------------------*/
  x = 4176;
  for ( z=0; z<16; z++ )
  {
    dwbRoot->colorTable[x][0] = ( (float)fileValues.underlay[z][0] ) / 255.0;
    dwbRoot->colorTable[x][1] = ( (float)fileValues.underlay[z][1] ) / 255.0;
    dwbRoot->colorTable[x][2] = ( (float)fileValues.underlay[z][2] ) / 255.0;

    x++;
  }

  return (TRUE);   /* Must be ok if we got this far */
}


/*========================================================================
 * When an external is received the load routines are called recursively
 * with the new file name. Parent and current node are not reset though
 * and will continue adding to the performer tree from the calling point
 *========================================================================*/

static pfNode *reuseExtern(char *name )
{
ExternList *ptr = externs;

  while (ptr)
  {
    if (!strcmp(ptr->name,name))
    {
      pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Reusing Extern %s\n",name);
      return(ptr->node);
    }
    ptr = ptr->next;
  }

  /* else add new extern onto list */
  printf("Creating Extern...%s\n",name);
  ptr = pfCalloc(1, sizeof(ExternList),0);
  strncpy(ptr->name,name,255);
  ptr->next = externs;
  externs = ptr;

  return(0); 
}


/*========================================================================
 * read an external record, and then process it.
 * This routine makes use of the re-entrant nature of the loader and 
 * recursively calls the top load routine. Which returns a pointer to
 * a node that is then attached to the current position.
 * An external will generally be followed by a matrix record, this routine
 * ensures that the correct tree structure is present for the matrix record
 *========================================================================*/

static int handleExternal(memPtr *ifp,int length,pfNode *parent,pfNode **currNode)
{
char *str;
pfNode *extNode;

  USERFUNCS(DB_EXTERNAL, NULL, *currNode, parent,ifp);

  str = readVariableLengthString(ifp,length);
  extNode = reuseExtern(str);
  if (!extNode)
  {
    if (externDepth > MAX_EXTERN_DEPTH)
    {
       printf("Error: Too many nested externals \n");
       return 0;
    }

    dwbRoot->insideExternal++;
    externArray[externDepth++] = dwbRoot;
#ifdef __ESCENE__
    if ((extNode = pfdLoad_dwb(str)) == NULL) return 1;
#else
    if ((extNode = pfdLoadFile_dwb(str)) == NULL) return 1;
#endif

    externDepth--;

    dwbRoot = externArray[externDepth];
    dwbRoot->insideExternal--;

    /* 
     * Now we need to work the matrices so that the matrix which follows
     * this record is placed into the correct coordinate frame.
     * It all rests on whether the parent is Y-up and whether the external
     * is Y-up. Placing a value int externSCSGroup will ensure that the next
     * matrix record read will get placed AFTER that group.
     * if either Y-Up,  then there will be a 90 rotation matrix at its root 
     *
     * 4 Possibilites:
     *	Parent Y-up, child Y-up: Already a rot matrix so remove the childs matrx
     *	Parent Y-up, child Not Y-up: add a -90 rotation to child
     *	Parent NOT Y-up, child Y-up: ok - record SCS so that matrix can get 
     *  placed after SCS
     *	Parent NOT Y-up, child Not Y-up: ok no changes needed
     *	The global variable used to check is not re-entrant so an nested globals
     *	MUST have the same orientation as the initial external
     */

    if ( dwbRoot->header.up_axis == Y_UP && externChildYUp[externDepth+1] != Y_UP ) 
    {
      pfSCS *scs;
      pfMatrix rot;

      pfMakeEulerMat(rot,0.0,-90.0,0.0);
      scs = pfNewSCS(rot);
      pfAddChild(scs,extNode);

      extNode = (pfNode *) scs;

      /* set global variable to allow the next matrix record to be positioned */
      /* after this scs orientation */
      externSCSGroup = scs;
    }
    else if (dwbRoot->header.up_axis == Y_UP && externChildYUp[externDepth+1] == Y_UP)
    {
      pfNode *node;

      /* remove the Y-up orientation from the nested file */
      if (childYupSCS)
      {
        node = pfGetChild(childYupSCS,0);
        pfRemoveChild(childYupSCS,node);
        pfDelete(childYupSCS);
        extNode = node; 
        externSCSGroup = NULL;
      }
      else
      pfNotify(PFNFY_WARN, PFNFY_INTERNAL, "Internal Error:No Yup orientation matrix defined for external\n");
    }
    else if (dwbRoot->header.up_axis != Y_UP && externChildYUp[externDepth+1]==Y_UP)
      externSCSGroup = (pfSCS *) extNode;

 

    externs->node = extNode;
  }
  pfFree (str);

  /* attach the returned node onto the current tree position */
  pfAddChild(parent,extNode);

  /* update the current node */
  *currNode = extNode;
  return(1);

}


/*========================================================================
 *
 * FUNCTION     : unsetZbufferOp
 * IN           : pfTraverser pointer (unused) & operation ID
 * OUT          :
 * DESCRIPTION  : unset GL zbuffer before traversing a particular  Node.
 *                Ignore incoming parameters.
 *
 *========================================================================*/

static int unsetZbufferOp ( pfTraverser *trav,void *data )
{
  GLboolean zbuffval = glIsEnabled (GL_DEPTH_TEST);

  if (zbuffval == GL_TRUE) glDisable (GL_DEPTH_TEST);
  return ( 1 );
}


/*========================================================================
 *
 * set zbuffer on inside a draw callback
 *
 *========================================================================*/

static int setZbufferOp ( pfTraverser *trav,void *data )
{
  GLboolean zbuffval = glIsEnabled (GL_DEPTH_TEST);

  if (zbuffval == GL_FALSE) glEnable (GL_DEPTH_TEST);

  return ( 1 );
}


/*
 * Take the bounding box from the current group and put that into
 * a Performer Sphere structure. This sphere structure 
		groupBoundingSphere
 * is global and is available to any child of the group to access.
 * Currently the only thing which needs this is the support for
 * bsplines. Note. its value is not preserved if an external is
 * processed, so there could be conditions when it is not correct
 * Add it to the dwbRoot structure to get around this if it causes
 * problems
 */

static void setBoundingSphereRadius(pfNode *currentNode,float *bbox)
{
float units;
pfSphere *sph = &groupBoundingSphere;
pfVec3 pnts[2];

  units = dwbRoot->unitChange;

  /* given the two corners of the bounding box compute */
  /* the radius of the sphere needed to cut that */
  pnts[0][0] = bbox[0] * units; pnts[1][0] = bbox[1] * units;
  pnts[0][1] = bbox[2] * units; pnts[1][1] = bbox[3] * units;
  pnts[0][2] = bbox[4] * units; pnts[1][2] = bbox[5] * units;

  pfSphereAroundPts(sph,pnts,2);
}


static int setLineStyleOn ( pfTraverser *trav, void *data )
{
LineStyle *style = (LineStyle*) data;
static short firstTime = 1;

  if (!style) return PFTRAV_CONT;

  if (firstTime)
  {
    glNewList ((GLuint) data, GL_COMPILE);
    glLineStipple (8, style->style);
    glEndList ();
    firstTime = 0;
  }

  glEnable (GL_LINE_STIPPLE);
  glCallList ((GLuint) data);

  return PFTRAV_CONT;
}


static int setLineStyleOff ( pfTraverser *trav, void *data )
{
  glDisable (GL_LINE_STIPPLE);

  return PFTRAV_CONT;
}


/*========================================================================
 *
 * Do all the processing associated with a group. As groups in dwb cover
 * a variety of tasks ranging from simple containers all the way down
 * to Decal groups this module is necessarily large
 *
 *========================================================================*/

static int handleGroup(memPtr *ifp, pfNode *parent, pfNode **curr, int *faceType)
{
pfNode *currentNode;
int ok;
dwbGroup group;
dwbHeader *dwb_header = &dwbRoot->header;
groupValues *currentGroup = &dwbRoot->currentGroup;

  /* -------------------------------------------- */
  /* No matter what create a group container node */
  /* -------------------------------------------- */
  currentNode = (pfNode *) pfNewGroup();
  pfAddChild(parent,currentNode);

  /* read the group */
  memRead( &group, sizeof(dwbGroup),1,ifp);
  swapDwbGroup(&group);

  lineWidth = (float) group.linewidth;

  if (group.grp_flags.sequence == 0 || group.grp_flags.decal == 0) {
    if (! extendedVersion) {
      USERFUNCS(DB_GROUP, &group, currentNode, parent,ifp);
    }
    else {
      /* 
       * If the renderGrp flag is set, then treat this group as an
       * older version group
       */
      if (group.grp_flags.renderGrp) {
	dwbRoot->renderGroup = 1;
	USERFUNCS(DB_GROUP, &group, currentNode, parent,ifp);
      }
      else dwbRoot->renderGroup = 0;
    }
  }
 
  setBoundingSphereRadius(currentNode,group.bbox);

  if (group.grp_flags.decal)
  {
    if (! extendedVersion) 
    {
      *faceType = LAYER_BASE;
    }
    else 
    {
      /*
       * Older version of handling Decals and the newer version of handling
       * decals when renderGrp flag is set are the same.
       */
      if (group.grp_flags.renderGrp) 
      {
	*faceType = LAYER_BASE;
      }
      else 
      {
	  pfLayer *temp;
        temp = pfNewLayer();

        pfReplaceChild (parent, currentNode, temp);

	{
	short pfdecalmode;
	  switch (group.decal_type)
	  {
	    case DWB_DECAL_BASE_FAST: 
	      pfdecalmode = PFDECAL_BASE_FAST; 
	    break;

	    case DWB_DECAL_BASE_HIGH_QUALITY:
	      pfdecalmode = PFDECAL_BASE_HIGH_QUALITY;
	    break;

	    case DWB_DECAL_BASE_STENCIL:
	      pfdecalmode = PFDECAL_BASE_STENCIL;
	    break;

	    case DWB_DECAL_BASE_DISPLACE:
	      pfdecalmode = PFDECAL_BASE_DISPLACE;
	    break;

	    default:
	      fprintf (stderr, "LoadDwb: unknown Decal mode\n");
	      pfdecalmode = PFDECAL_BASE_HIGH_QUALITY;
	    break;
	  }
          pfLayerMode(temp, pfdecalmode);
	}

        currentNode = (pfNode *) temp;
        USERFUNCS (DB_GROUP, &group, currentNode, parent, ifp);
        *faceType = NORMAL;
      }
    }
  }
  else if  (group.grp_flags.billboard)
         *faceType = BILLBOARD;
  else if  (group.grp_flags.sequence)
  {
    pfSequence *seq;

    /* add a sequence to the tree */
    seq = pfNewSeq();
    pfSeqTime(seq, PFSEQ_ALL, group.seq_time);

    {
    short pfseqmode;

      switch (group.seq_mode)
      {
	case DWB_SEQ_CYCLE:
	  pfseqmode = PFSEQ_CYCLE;
	break;

	case DWB_SEQ_SWING:
	  pfseqmode = PFSEQ_SWING;
	break;

	default:
	  fprintf (stderr, "LoadDwb: Unknown sequence mode\n");
	  pfseqmode = PFSEQ_CYCLE;
	break;
      }

      pfSeqInterval(seq, pfseqmode, group.seq_bgn, group.seq_end);
    }

    pfSeqDuration(seq,group.seq_speed,group.seq_nreps);
    pfSeqMode(seq, PFSEQ_START);

    pfReplaceChild (parent, currentNode, seq);
    currentNode = (pfNode *) seq;

    USERFUNCS(DB_GROUP, &group, currentNode, parent,ifp);

    *faceType = SEQUENCE;
  }
  else if (group.grp_flags.clip_region)
  {
  ClipRegion *clip;
  int switchOn(pfTraverser *trav, void *data);

     clip = pfCalloc(1, sizeof(ClipRegion),Arena);
     memcpy(clip->ll,&group.ll,sizeof(float)*3);
     memcpy(clip->ur,&group.ur,sizeof(float)*3);

     /* adjust the clip position to account for the units used */
     clip->ll[0] *=  dwbRoot->unitChange;
     clip->ll[1] *=  dwbRoot->unitChange;
     clip->ll[2] *=  dwbRoot->unitChange;

     clip->ur[0] *=  dwbRoot->unitChange;
     clip->ur[1] *=  dwbRoot->unitChange;
     clip->ur[2] *=  dwbRoot->unitChange;

     pfNodeTravFuncs(currentNode,PFTRAV_CULL,switchOn, NULL); 

     
     pfNodeTravFuncs(currentNode, PFTRAV_DRAW,clipRegionOn,clipRegionOff);
     pfNodeTravData(currentNode, PFTRAV_DRAW, (void *) clip);
     *faceType = NORMAL; 
     clipRegionsPresent = 1;
     
  }  
  else {
    *faceType = NORMAL;
  }

  /* set up new current node */
  *curr = currentNode;

  /*--------------------------------------------------*/
  /* set up the backface mode for this group.         */
  /*--------------------------------------------------*/
  if ( dwb_header->backface == TS_SELECTIVE )
      currentGroup->backface = group.gfx_flags.backface;
  else
      currentGroup->backface = dwb_header->backface;


  /*--------------------------------------------------*/
  /* set up the drawstyle mode for this group. For    */
  /* now - we are only interested in solid & wire.    */
  /*--------------------------------------------------*/
  if ( dwb_header->drawstyle == DS_SELECTIVE )
  {
    switch  ( group.drawstyle )
    {
        case DS_POINTS:
        case DS_OPEN_WIRE:
        case DS_CLOSED_WIRE:
            currentGroup->drawstyle = group.drawstyle;
            break;
        default:
            currentGroup->drawstyle = DS_SOLID;
            break;
    }
  }
  else
      currentGroup->drawstyle = dwb_header->drawstyle;

  /*--------------------------------------------------*/
  /* set up the zbuffer mode for this group.          */
  /*--------------------------------------------------*/
  if ( dwb_header->zbuffer == TS_SELECTIVE )
  {
    currentGroup->zbuffer = group.gfx_flags.zbuffer;

    /*----------------------------------------------*/
    /* check the zbuffer state for this group.If its*/
    /* OFF and its selective zbuffering we will zap */
    /* the  zbuffer FOR THIS NODE ONLY.             */
    /*----------------------------------------------*/

    if ( ! currentGroup->zbuffer )
      pfNodeTravFuncs ( currentNode,PFTRAV_DRAW,unsetZbufferOp,setZbufferOp );

  }
  else
    currentGroup->zbuffer = dwb_header->zbuffer;

  /*--------------------------------------------------*/
  /* set up the shademodel for the renderable nodes.  */
  /* Either use group shademodel or the global ones.  */
  /*--------------------------------------------------*/
  if ( dwb_header->shademodel == SM_SELECTIVE )
    currentGroup->shading = group.shademodel;
  else
    currentGroup->shading = dwb_header->shademodel;

  /*--------------------------------------------------*/
  /* only light the group if the group shading is     */
  /* illuminated...in a  Designer's Workbench file    */
  /* FLAT means 1 color per primitive - non lit,      */
  /* GOURAUD means 1 color per vertex - non lit and   */
  /* ILLUMINATED means lit using lmcolor              */
  /*--------------------------------------------------*/
  if ( currentGroup->shading == SM_ILLUMINATED )
  {
    /*----------------------------------------------*/
    /* set illumination binding                     */
    /*----------------------------------------------*/
    if ( dwb_header->material_binding == MB_SELECTIVE )
      currentGroup->binding = group.material_binding;
    else
      currentGroup->binding = dwb_header->material_binding;

    currentGroup->material = group.material;

    /*----------------------------------------------*/
    /* defensive programming here because some      */
    /* older dwb files could have group shading set */
    /* to ILLUMINATED but actually be referencing   */
    /* index -1.                                    */
    /*----------------------------------------------*/
    if ( currentGroup->material == -1 || !dwbRoot->materialTable)
    {
      currentGroup->shading = SM_FLAT_NON_LIT;
      currentGroup->alpha   = 1.0;
      currentGroup->binding = MB_LIGHTING_OFF;
    }
    else
    {
      currentGroup->alpha = dwbRoot->materialTable[group.material].mat.alpha;
    }
  }
  else
  {
    currentGroup->material = -1;
    currentGroup->binding  = MB_LIGHTING_OFF;
    /*----------------------------------------------*/
    /* check group-level transparency - use this for*/
    /* non lit models. Lit models will use the      */
    /* transparency  in the material definition.    */
    /*----------------------------------------------*/
    if ( group.transparency != 1.0 /*&& dwb_header->transparency*/ )
      currentGroup->alpha = group.transparency;
    else
      currentGroup->alpha = 1.0;
  }


  /*-------------------------------------------------------*/
  /*  Handle Sound index                                   */
  /*-------------------------------------------------------*/

#ifdef LINKLOADER
  if (dwbRoot->header.version > HIGHEST_DWB_FORMAT_VERSION_SUPPORTED -
	                        ROUNDING_ERROR) {

      if (group.sound_index > -1)
          addSoundReference(group.sound_index);
  }
#endif

  return 1;
}


#ifdef LINKLOADER
pfSphere getBoundingSphere(void)
{
    return(groupBoundingSphere);
}
#endif


/* --------------------------------------------------------------------- */
/* implement a simple dynamic length stack that is used to store the   */
/* parent and current node information at every push and pop encountered */
/* --------------------------------------------------------------------- */

printStack( void )
{
int count;
treeStack *stack = dwbRoot->stack;

  for (count=0; count < dwbRoot->treeDepth; count++)
    printf("%d : %x \t\t: %x\n",count, stack[count].parent, stack[count].current);

}


static void dwbPush(pfNode **parent, pfNode *currentNode)
{

  if (dwbRoot->treeDepth+1 > dwbRoot->stackDepth)
  {
    /* increase the size of the stack by STACKSIZE */
    dwbRoot->stackDepth += STACKSIZE;
    if (!dwbRoot->stack)
      dwbRoot->stack =(treeStack*) pfMalloc(sizeof(treeStack)*dwbRoot->stackDepth,                                            NULL);
    else
      dwbRoot->stack =(treeStack*) makeStructArray(dwbRoot->stackDepth,
			                     sizeof(treeStack), dwbRoot->stack);
  }

  dwbRoot->stack[dwbRoot->treeDepth].parent = *parent;
  dwbRoot->stack[dwbRoot->treeDepth].current = currentNode;

  *parent = currentNode;
  dwbRoot->treeDepth++;
}


static void dwbPop(pfNode **parent, pfNode **currentNode)
{
  /* dwb2 will store one more pop than necessary - check for that */ 
  if (dwbRoot->treeDepth > 0)
    dwbRoot->treeDepth--;

  *parent      = dwbRoot->stack[dwbRoot->treeDepth].parent;
  *currentNode = dwbRoot->stack[dwbRoot->treeDepth].current;
}


/*========================================================================
 *
 * Process a Switch Record
 * doesnt use lods the way Performer expects yet 
 * these lod nodes will only ever contain one object and it is 
 * either on or off depending on range 
 * later version will compact all lods with adjacent ranges into single 
 * lod nodes 
 *
 *========================================================================*/

static int handleSwitch(memPtr *ifp, pfNode *parent, pfNode **currentNode)
{
float unitChange = dwbRoot->unitChange;
dwbSwitch sw;
float buff[3];
pfLOD *lod;
pfGroup *tmp;

  memRead(&sw,sizeof(dwbSwitch),1,ifp);
  swapDwbSwitch(&sw);

  /* only support switches on distance */
  if (sw.switchtype == ST_DISTANCE)
  {
    lod = pfNewLOD();     
    pfAddChild(parent,lod);

    USERFUNCS(DB_SWITCH, &sw, (pfNode *) lod, parent,ifp);

    /* set the switch distances */
    pfLODRange ( lod, 0, sw.switchout*unitChange ); 
    pfLODRange ( lod, 1, sw.switchin*unitChange ); 

    copyNyup(buff,sw.center,unitChange)
    
    /* set the centre */
    pfLODCenter( lod, buff );

    /* now create a single group for all nested children to reference */
   tmp = pfNewGroup();
#if LINKLOADER
   LODChild = (pfNode *)tmp; /* only of relevance to linked files */
   LODParent = (pfNode *)lod; /* only of relevance to linked files */
#endif

   pfAddChild(lod,tmp);
    *currentNode = (pfNode *) tmp;

    /* Set LOD fade range to 1 so that the global LOD fade flag can take effect */
    pfLODTransition(lod,0,1.0);
    pfLODTransition(lod,1,1.0);
  }
  else {
   pfSwitch *sw;

    printf("Switch node contains thresholding:\tpfSwitch Node created, but switching is left to the user's application.\n");
    sw = pfNewSwitch();
    pfAddChild(parent,sw);

    /* now create a single child that is off by default */
    tmp = pfNewGroup();
    pfAddChild(sw,tmp);
    *currentNode = (pfNode *) tmp;

    pfSwitchVal(sw,PFSWITCH_OFF);

  }

  return(1);
}


/*========================================================================
 *
 * Read a face record and set up the variables needed to
 * later create the GeoSets when the vertices are read:
 *
 *	 polygon: will contain the vertices,colours, and normals values but just
 *		 set number of vertices and binding information here.
 *
 *	currentGstate: ptr to the dwbGeoState that should be
 *		       used for this face. These states are 
 *		       shared whereever possible but must be
 *		       set at polygon level cause each face can 
 *		       have its own texture.
 *
 *	vType: just contains the type of face required, flat,gouraud,illuminated
 *	       or textured.
 *
 *
 *========================================================================*/

static int handleFace(memPtr *ifp, pfNode *parent, pfdGeom *polygon, 
		      dwbGeoState **currentGState,int *vType, pfNode *current)
{
/* 
 * The first few fields in dwbFace and dwbNewFace are the same except
 * dwbNewFace.pntsize and dwbFace.material which happen to share the same
 * memory location. Since this field [material] is not used in versions 2.21
 * and earlier, just load everything into the new structure without any
 * problems. All the other fields that are used in Version 2.1 have the same
 * names and memory locations in the later versions.
 */

dwbNewFace face;
int ok=1;
groupValues *currentGroup = &dwbRoot->currentGroup;
int vertType;

  /* read the face record */
  memRead(&face,sizeof(dwbNewFace),1,ifp);
  swapDwbNewFace(&face);
  USERFUNCS(DB_FACE, &face, current, parent,ifp);

  /* ------------------- */
  /* now handle the face */
  /* ------------------- */

  faceBackColor = face.back_color;

  /* pass on the information up to the DB_POP handler */
  currentFacePType = face.ptype;

  /* if this is a bspline then return immediately */
  if (face.ptype == 3)
  {
    SET_COLOR ( polygon,0,face.color, face.packed_color )
    pfCopyVec4(nextBSplineColor, polygon->colors[0]);
    return(1);
  }

  polygon->numVerts = face.numverts;

  if (polygon->numVerts > polyLength)
  {
      polyLength = polygon->numVerts + 1;
      pfdResizeGeom(polygon, polyLength);
      polygon->numVerts = face.numverts;
  }

  /* because pfdGeoBuilder doesn't handle lines properly extract those */
  /* and store them elsewhere */

  /* get the vertex type */
  switch ( currentGroup->shading )
  {
    case SM_FLAT_NON_LIT:
        vertType = VT_FLAT_VERTEX;
        break;
    case SM_GOURAUD_NON_LIT:
        vertType = VT_SHADED_VERTEX;
        break;
    case SM_ILLUMINATED:
        switch ( currentGroup->binding )
        {
            case MB_PER_VERTEX:
                vertType = VT_SHADED_ILLUM_VERTEX;
                break;
            default:
                vertType = VT_ILLUM_VERTEX;
                break;
        }
        break;
    default:
        break;
  }

  if (face.texture >= 0)
  {
    vertType ^= TEXTURED_VERT;
    dwbRoot->currentGroup.texture = getDwbTexture(face.texture);
    /*
     * Only the extended version supports loading of Detailed texture.
     * So check for the version and then if this texture is flagged as
     * a detailed texture, then load the texture.
     * If the loading was successful, then apply the detail texture
     * using the texture indices. 
     */
    if (extendedVersion && face.detail_tex >= 0) {
      if (getDwbTexture (face.detail_tex) >= 0) {
	applyDetailTexture (face.texture, face.detail_tex);
      }
    }
  }
  else
    dwbRoot->currentGroup.texture = -1;

  /*--------------------------------------------------*/
  /* tell the pfdGeoBuilder polygon how to interpret the */
  /* data being passed to it. Ie how to bind colors   */
  /* etc.                                             */
  /*--------------------------------------------------*/
  setUpPolygonBindTypes ( polygon,vertType );

  /*--------------------------------------------------*/
  /* certain vertex types require 1 color for the     */
  /* primitive. Assign that color now before we get to*/
  /* the vertex level.                                */
  /*--------------------------------------------------*/
  switch ( vertType )
  {
    case VT_FLAT_VERTEX:
    case VT_FLAT_VERTEX + TEXTURED_VERT:
    case VT_ILLUM_VERTEX:
    case VT_ILLUM_VERTEX + TEXTURED_VERT:
        SET_COLOR ( polygon,0,face.color, face.packed_color )
        break;
    default:
        break;
  }


  /* override the group drawstyle */
  dwbRoot->currentGroup.drawstyle = face.drawstyle;

  /* get the dwbGeoState for this face */
  *currentGState = getDwbGeoState ();

  *vType = vertType;

  /* ------------------------------------------------------- */
  /* linestyles are added per face to avoid conflicting with */
  /* clip regions at group level                             */
  /* ------------------------------------------------------- */

  if (face.linestyle > 0)
  {
  LineStyle *lineStyle;

      lineStyle = pfCalloc(1, sizeof(LineStyle),Arena);
      lineStyle->index = face.linestyle;
      lineStyle->style = lineStyles[lineStyle->index].style;

      pfNodeTravFuncs(current,PFTRAV_DRAW,setLineStyleOn,setLineStyleOff);
      pfNodeTravData(current, PFTRAV_DRAW, (void *) lineStyle);
  }

  return(ok);
}


/*========================================================================
 *
 * FUNCTION     : setUpPolygonBindTypes
 * IN           : ptr to poly being set up & vert type associated with the
 *                current polygon.
 * OUT          : nothing
 * DESCRIPTION  : tidy up the the convDwbFace() routine by calling this func.
 *                Set poly bind types for pfdGeoBuilder utilities
 *
 *========================================================================*/

static void setUpPolygonBindTypes ( pfdGeom *polyptr,int vertType )
{
int newVertType = vertType;
int t;

  if (vertType & TEXTURED_VERT)
  {
    polyptr->tbind[0] = PFGS_PER_VERTEX;
    for (t = 1 ; t < PF_MAX_TEXTURES ; t ++)
	polyptr->tbind[t] = PFGS_OFF;
    newVertType ^= TEXTURED_VERT;
  }
  else
  {
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	polyptr->tbind[t] = PFGS_OFF;
  }

  switch (  newVertType )
  {
    case VT_FLAT_VERTEX:
      polyptr->cbind = PFGS_PER_PRIM;     /* build polygon definitions */
      polyptr->nbind = PFGS_OFF;
      break;
    case VT_SHADED_VERTEX:
      polyptr->cbind = PFGS_PER_VERTEX;
      polyptr->nbind = PFGS_OFF;
      break;
    case VT_ILLUM_VERTEX:
      polyptr->cbind = PFGS_PER_PRIM;
      polyptr->nbind = PFGS_PER_VERTEX;
      break;
    case VT_SHADED_ILLUM_VERTEX:
      polyptr->cbind = PFGS_PER_VERTEX; /* old loader did PFGS_PER_PRIM */
      polyptr->nbind = PFGS_PER_VERTEX;
      break;
    default:
      break;
  }

}


/*========================================================================
 *
 * FUNCTION     : getDwbGeoState
 * IN           : nothing
 * OUT          : ptr to geostate wrapper
 * DESCRIPTION  : check geostate stuff & return the 1 which matches current
 *                group attribs.
 *
 *========================================================================*/

static dwbGeoState *getDwbGeoState ( void )
{
dwbGeoState *gstate;
groupValues *curr = &dwbRoot->currentGroup;

  if ( dwbRoot->gsList )
    for ( gstate = dwbRoot->gsList; gstate != NULL; gstate = gstate->next)
            if (curr->material  == gstate->values.material   &&
                curr->binding   == gstate->values.binding    &&
                curr->backface  == gstate->values.backface   &&
                curr->zbuffer   == gstate->values.zbuffer    &&
                curr->drawstyle == gstate->values.drawstyle  &&
                curr->alpha     == gstate->values.alpha      &&
                curr->texture   == gstate->values.texture  )
        return ( gstate );
  
  /* if we get to this point then a new geoState is needed */
  gstate = makeDwbGeoState();

  gstate->next = dwbRoot->gsList;
  dwbRoot->gsList = gstate;

  return(gstate);
}


/*======================================================================== *
 * FUNCTION     : makeDwbGeoState
 * IN           : ptr to geostate wrapper
 * OUT          : ptr to created dwb geostate wrapper func.
 * DESCRIPTION  : assign geostate stuff
 *
 *========================================================================*/

static dwbGeoState *makeDwbGeoState ( void )
{
dwbGeoState *gstate = NULL;
groupValues *group = &dwbRoot->currentGroup;

  gstate = (dwbGeoState *) pfCalloc(1, sizeof(dwbGeoState), Arena);

  if ( gstate )
  {
    gstate->values = *group;

    /*----------------------------------------------*/
    /* initialize utility library triangle/geoset   */
    /* builder                                      */
    /*----------------------------------------------*/
    if ( group->drawstyle == DS_OPEN_WIRE || 
         group->drawstyle == DS_CLOSED_WIRE )
        gstate->builder     = NULL;
    else
        gstate->builder     = pfdNewGeoBldr();

    /*----------------------------------------------*/
    /* seperate list of non-solid-filled line polys */
    /*----------------------------------------------*/
    gstate->lineList        = NULL;

    /*----------------------------------------------*/
    /* also create a performer geostate with the    */
    /* passed in values.                            */
    /*----------------------------------------------*/
    gstate->pfgs = makePfGeoState(gstate);

    gstate->next = NULL;

  }
  return ( gstate );
}

typedef struct _DuffTexList
{
  char *name;
  struct _DuffTexList *next;
} duffTexList;
static duffTexList *firstDuffTex;

static int notInDuffTexList( char *str)
{
duffTexList *ptr = firstDuffTex;

  while (ptr)
  {
    if (!strcmp(str,ptr->name))
      return(0);
    
    ptr = ptr->next;
  }

  /* if you get to here, the name is not in the list */
  ptr = pfCalloc(1, sizeof(duffTexList),NULL);
  ptr->name = pfMalloc(strlen(str), NULL);
  strcpy (ptr->name,str);

  ptr->next = firstDuffTex;
  firstDuffTex = ptr;

  return (1);
}
  

/*========================================================================
 *
 * FUNCTION     : getDwbTexture
 * IN           : texture index into memPtr's texpalette
 * OUT          : PfTexture table index
 * DESCRIPTION  : Return Performer texture corresponding to dwb texture
 *                and create new pfTexture if necessary. Textures are shared
 *                between .dwb  files by texture file name.
 *
 *========================================================================*/

static int getDwbTexture ( int id )
{
int            i;
dwbTexture     *tex;
pfTexture      *pfTex;
pfTexEnv       *pfTev;
dwbFile        *dwb = dwbRoot;
float          minf,magf,
	       wrap,wrapS,wrapT,
	       env, r,g,b,a;
char 	       path[PF_MAXSTRING];
int	       notFound =0;

  if (id < 0 )
    return ( id );

  /*--------------------------------------------------*/
  /* If the texture index is valid but NO texture     */
  /* table was found in the file.Probarbly originated */
  /* as an early dwb file and treat it as untextured. */
  /*--------------------------------------------------*/
  if ( dwb->textureTable == NULL )
    return ( -1 );

  if ( id >= dwb->texturesRead )
    return ( -1 );

  /* -------------------------------------------- */
  /* check to see if this file has reference this */
  /* texture before */
  /*----------------------------------------------*/
  if (dwbRoot->pfTexIndex[id] >= 0)
    return (dwbRoot->pfTexIndex[id]);
  else if (dwbRoot->pfTexIndex[id] == TEXTURE_NOT_FOUND)
    return(-1);

  /* get a local handle to texture table struct       */
  /*--------------------------------------------------*/
  tex = &(dwb->textureTable[id]);

  /*--------------------------------------------------*/
  /* read tex props from the table.                   */
  /*--------------------------------------------------*/

  switch ((int) ((double) tex->tex.minfilter))
  {
    case DWB_TX_POINT: minf = PFTEX_POINT; break;
    case DWB_TX_BILINEAR: minf = PFTEX_BILINEAR; break;


    case DWB_TX_BICUBIC: minf = PFTEX_BICUBIC; break;
    case DWB_TX_MIPMAP_POINT: minf = PFTEX_MIPMAP_POINT; break;
    case DWB_TX_MIPMAP_LINEAR:
      if (GfxType & GFX_REALITY)
	minf = PFTEX_MIPMAP_TRILINEAR;
      else
	minf = PFTEX_MIPMAP_LINEAR;
      break;
    case DWB_TX_MIPMAP_BILINEAR:
      if (GfxType & GFX_REALITY)
	minf = PFTEX_MIPMAP_TRILINEAR;
      else
	minf = PFTEX_MIPMAP_BILINEAR;
      break;
    case DWB_TX_MIPMAP_TRILINEAR: minf = PFTEX_MIPMAP_TRILINEAR; break;
    default:
      fprintf (stderr, "LoadDwb: Unknown minFilter for %s\n", tex->name);
      minf = PFTEX_BILINEAR;
    break;
  }

  switch ((int) ((double) tex->tex.magfilter))
  {
    case DWB_TX_POINT: magf = PFTEX_POINT; break;
    case DWB_TX_BILINEAR: magf = PFTEX_BILINEAR; break;


    case DWB_TX_BICUBIC: magf = PFTEX_BICUBIC; break;
    case DWB_TX_SHARPEN: magf = PFTEX_SHARPEN; break;
    case DWB_TX_ADD_DETAIL: magf = PFTEX_ADD_DETAIL; break;
    case DWB_TX_MODULATE_DETAIL: magf = PFTEX_MODULATE_DETAIL; break;
    default:
      fprintf (stderr, "LoadDwb: Unknown magFilter for %s\n", tex->name);
      magf = PFTEX_BILINEAR;
    break;
  }

  switch ((int) ((double) tex->tex.wrap))
  {
    case DWB_TX_REPEAT: wrap = PFTEX_REPEAT; break;
    case DWB_TX_CLAMP: wrap = PFTEX_CLAMP; break;
    case DWB_TX_SELECT: wrap = PFTEX_SELECT; break;
    default:
      fprintf (stderr, "LoadDwb: Unknown texture wrap for %s\n", tex->name);
      wrap = PFTEX_REPEAT;
    break;
  }

  switch ((int) ((double) tex->tex.wraps))
  {
    case DWB_TX_REPEAT: wrapS = PFTEX_REPEAT; break;
    case DWB_TX_CLAMP: wrapS = PFTEX_CLAMP; break;
    case DWB_TX_SELECT: wrapS = PFTEX_SELECT; break;
    default:
      fprintf (stderr, "LoadDwb: Unknown texture wrapS for %s\n", tex->name);
      wrapS = PFTEX_REPEAT;
    break;
  }

  switch ((int) ((double) tex->tex.wrapt))
  {
    case DWB_TX_REPEAT: wrapT = PFTEX_REPEAT; break;
    case DWB_TX_CLAMP: wrapT = PFTEX_CLAMP; break;
    case DWB_TX_SELECT: wrapT = PFTEX_SELECT; break;
    default:
      fprintf (stderr, "LoadDwb: Unknown texture wrapT for %s\n", tex->name);
      wrapT = PFTEX_REPEAT;
    break;
  }

  switch ((int) ((double) tex->tex.envtype))
  {
    case DWB_TV_MODULATE: env = PFTE_MODULATE; break;
    case DWB_TV_BLEND: env = PFTE_BLEND; break;
    case DWB_TV_DECAL: env = PFTE_DECAL; break;
    default:
      fprintf (stderr, "LoadDwb: Unknown texEnv for %s\n", tex->name);
      env = PFTE_MODULATE;
    break;
  }

  if (extendedVersion) {
    r = tex->tex.envcolor[0];
    g = tex->tex.envcolor[1];
    b = tex->tex.envcolor[2];
    a = tex->tex.envcolor[3];
  }

  /*--------------------------------------------------*/
  /* See if we can find texture file                  */
  /*--------------------------------------------------*/

  if (!pfFindFile(tex->name, path, R_OK))
  {
      pfNotify(PFNFY_WARN,PFNFY_RESOURCE,
            "LoadDwb: Could not find texture \"%s\"", tex->name);

      dwbRoot->pfTexIndex[id] = TEXTURE_NOT_FOUND;  

    return(-1);

  }

  /*--------------------------------------------------*/
  /* See if texture was already loaded by another file*/
  /*--------------------------------------------------*/
  for (i = 0; i < sharedTexCount; i++)
  {
      const char *str =  pfGetTexName(sharedTexList[i]);
      if (!str)
          continue;

      if(!strcmp(path, str))
        {
        dwbRoot->pfTexIndex[id] = i;
        return (i);
        }
  }

  /*--------------------------------------------------*/
  /* Could not share texture ...create a new pfTexture*/
  /*--------------------------------------------------*/
  pfTex = pfNewTex(Arena);
  pfRef(pfTex);  /* force an extra reference so that will not
                    be removed by any subsequent pfDelete */

  /*--------------------------------------------------*/
  /* load it..                                        */
  /*--------------------------------------------------*/
#ifndef ENCRYPT

#ifdef __ESCENE__
  if (encrypted)
      mypfLoadTexFile(pfTex, path);
  else
    pfLoadTexFile(pfTex,path);
#else
    pfLoadTexFile(pfTex,path);
#endif

#else
    mypfLoadTexFile(pfTex, path);

#endif

  if(pfGetNotifyLevel() >= PFNFY_INFO)
  {
  int comp, x, y, z;
  unsigned int *image;
    pfGetTexImage(pfTex, &image, &comp, &x, &y, &z);

    /* don't shorten name so user can see full path */
    /* could resolve some path problems as dwb stores full path names */

    pfNotify(PFNFY_INFO,PFNFY_RESOURCE, 
           "Loaded texture %s:\n\t %d component, (%d, %d, %d)\n",
                    path, comp, x, y, z);
  }

  pfTexFilter(pfTex, PFTEX_MINFILTER, minf );
  pfTexFilter(pfTex, PFTEX_MAGFILTER, magf );
   
  if (! extendedVersion)
  {
    if (GfxType & GFX_REALITY)
    {
      int comp,foo;

      pfGetTexImage(pfTex, (unsigned int **)&foo, &comp, &foo, &foo, &foo);

      /* Assume alpha texture is a template so sharpen alpha */
      if(comp == 2 || comp == 4)
	pfTexFilter(pfTex, PFTEX_MAGFILTER_ALPHA, PFTEX_SHARPEN);
    }
  }

  if (wrapS == PFTEX_REPEAT && wrapT == PFTEX_REPEAT)
    pfTexRepeat ( pfTex, PFTEX_WRAP, wrap );
  else
  {
    pfTexRepeat ( pfTex, PFTEX_WRAP_S, wrapS );
    pfTexRepeat ( pfTex, PFTEX_WRAP_T, wrapT );
  }

  /* Theres's no PFTE_ALPHA in OpenGL */

  if (env == PFTE_DECAL || env == PFTE_MODULATE) 
  {
    pfTev = pfNewTEnv (Arena);
    pfRef(pfTev);  /* force an extra reference so that will not
                      be removed by any subsequent pfDelete */

    pfTEnvMode (pfTev, env);
  }
  else if (env == PFTE_BLEND) {
    if (! extendedVersion) {
      fprintf (stderr, 
	  "LoadDwb: Older version does not support BLENDed Textures\n"
	  "         Defaulting to MODULATEd textures\n");
      pfTev = defTEnv;
    }
    else {
      pfTev = pfNewTEnv (Arena);
      pfRef(pfTev);  /* force an extra reference so that will not
                        be removed by any subsequent pfDelete */
      pfTEnvMode (pfTev, env);
      pfTEnvBlendColor (pfTev, r, g, b, a);
    }
  }
  else {
    fprintf (stderr,
	"LoadDwb: Unknown texture environment for \"%s\"\n", tex->name);
    pfTev = defTEnv;
  }

  /*
   * The flags in the extended structure will definitely be zero, if the
   * file is a older version, because all the bytes past the 96th are 
   * bzero()ed in handleTexture.
   */
  if (extendedVersion) {
    if (tex->tex.flags.component_select) {
    float pfcomp;

      fprintf (stderr, "compselect: %.4f\n", tex->tex.component_select);
      switch ((int) ((double) tex->tex.component_select))
      {
	case DWB_TV_I_GETS_R: pfcomp = PFTE_COMP_I_GETS_R; break;
	case DWB_TV_I_GETS_G: pfcomp = PFTE_COMP_I_GETS_G; break;
	case DWB_TV_I_GETS_B: pfcomp = PFTE_COMP_I_GETS_B; break;
	case DWB_TV_I_GETS_A: pfcomp = PFTE_COMP_I_GETS_A; break;
        case DWB_TV_I_GETS_I: pfcomp = PFTE_COMP_I_GETS_I; break;
      }

      pfTEnvComponent (pfTev, pfcomp);
    }

    if (tex->tex.flags.magfilter_split && (GfxType & GFX_REALITY)) 
    {
    float malpha, mcolor;

      switch ((int) ((double) tex->tex.magfilter_alpha))
      {
	case DWB_TX_POINT: malpha = PFTEX_POINT; break;
	case DWB_TX_BILINEAR: malpha = PFTEX_BILINEAR; break;


	case DWB_TX_BICUBIC: malpha = PFTEX_BICUBIC; break;
	case DWB_TX_SHARPEN: malpha = PFTEX_SHARPEN; break;
	case DWB_TX_ADD_DETAIL: malpha = PFTEX_ADD_DETAIL; break;
	case DWB_TX_MODULATE_DETAIL: malpha = PFTEX_MODULATE_DETAIL; break;
	default:
	  fprintf (stderr, "LoadDwb: Unknown alphaFilter for %s\n", tex->name);
	  malpha = PFTEX_POINT;
	break;
      }

      pfTexFilter (pfTex, PFTEX_MAGFILTER_ALPHA, malpha);

      switch ((int) ((double) tex->tex.magfilter_color))
      {
	case DWB_TX_POINT: mcolor = PFTEX_POINT; break;
	case DWB_TX_BILINEAR: mcolor = PFTEX_BILINEAR; break;


	case DWB_TX_BICUBIC: mcolor = PFTEX_BICUBIC; break;
	case DWB_TX_SHARPEN: mcolor = PFTEX_SHARPEN; break;
	case DWB_TX_ADD_DETAIL: mcolor = PFTEX_ADD_DETAIL; break;
	case DWB_TX_MODULATE_DETAIL: mcolor = PFTEX_MODULATE_DETAIL; break;
	default:
	  fprintf (stderr, "LoadDwb: Unknown colorFilter for %s\n", tex->name);
	  mcolor = PFTEX_POINT;
	break;
      }

      pfTexFilter (pfTex, PFTEX_MAGFILTER_COLOR, mcolor);
    }

    if (tex->tex.flags.internal && (GfxType & GFX_REALITY)) 
    {
    float internal;
      
      switch ((int) ((double) tex->tex.internal))
      {
	case DWB_TX_I_12A_4: internal = PFTEX_I_12A_4; break;
	case DWB_TX_IA_8: internal = PFTEX_IA_8; break;
	case DWB_TX_RGB_5: internal = PFTEX_RGB_5; break;
	case DWB_TX_RGBA_4: internal = PFTEX_RGBA_4; break;
	case DWB_TX_IA_12: internal = PFTEX_IA_12; break;
	case DWB_TX_RGBA_8: internal = PFTEX_RGBA_8; break;
	case DWB_TX_RGBA_12: internal = PFTEX_RGBA_12; break;
	case DWB_TX_RGB_12: internal = PFTEX_RGB_12; break;
	case DWB_TX_I_16: internal = PFTEX_I_16; break;
	default:
	  fprintf (stderr, "LoadDwb: Unknown internal format for %s\n", 
			   tex->name);
	  internal = PFTEX_RGBA_4;
	break;
      }

      pfTexFormat (pfTex, PFTEX_INTERNAL_FORMAT, internal);
    }

    if (tex->tex.flags.external && (GfxType & GFX_REALITY)) 
    {
    float ext;

      switch ((int) ((double) tex->tex.external))
      {
	case DWB_TX_PACK_8: ext = PFTEX_PACK_8; break;
	case DWB_TX_PACK_16: ext = PFTEX_PACK_16; break;
	/*
	 * Performer doesn't understand DWB_TX_PIXMODE
	 */
	default:
	  fprintf (stderr, "LoadDwb: Unknown external format for %s\n",
	                   tex->name);
	break;
      }

      pfTexFormat (pfTex, PFTEX_EXTERNAL_FORMAT, ext);
    }

    /*
     * Just make sure that the mag and min filters are not using MIPMAP
     * filters when this flag [FAST_DEFINE] is set. The other requirement
     * is that the texture size should be a multiple of 2, as given in the
     * texdef2 man pages.
     */

  /* OpenGL doensn't support PFTEX_FAST_DEFINE texture format */
  /* XXX Do the right thing here */

    if (tex->tex.flags.mipmap_filter) {
      fprintf (stderr, 
	  "LoadDwb: Mipmap filters are not supported currently\n");
      /*
       * There's no equivalent call for 
       * texdef (...TX_MIPMAP_FILTER_KERNEL, ...)
       * in Performer. So just skip it for now.
       */
    }

    if (tex->tex.flags.bicubic_filter) {
      fprintf (stderr, 
	  "LoadDwb: Bicubic Filters are not supported currently\n");
      /*
       * Same reason as above.
       */
    }

    if (tex->tex.flags.control_points) {
	int splineType = -1;
	pfVec2 *points = (pfVec2 *) tex->tex.control_points;
	float clamp = tex->tex.control_clamp;

      if (magf == PFTEX_SHARPEN) 
	splineType = PFTEX_SHARPEN_SPLINE;
      else if (magf == PFTEX_MODULATE_DETAIL || magf == PFTEX_ADD_DETAIL)
	splineType = PFTEX_DETAIL_SPLINE;
      else {
	fprintf (stderr, 
	    "LoadDwb: Unable to determine filter spline for Mag filter\n");
      }

      if (splineType != -1)
	pfTexSpline (pfTex, splineType, points, clamp);
    }

    /*
     * Performer does not have the equivalent of TX_TILE.
     * Detail texture is handled in handleFace()
     */
  }

  sharedTexList[sharedTexCount] = pfTex;
  sharedTevList[sharedTexCount] = pfTev;
  dwbRoot->pfTexIndex[id] = sharedTexCount;
  sharedTexCount++;

  return (sharedTexCount-1);
}


/*========================================================================
 *
 * FUNCTION     : applyDetailTexture
 * IN           : ids of the main and detail texture
 * OUT          : nothing
 * DESCRIPTION  : uses pfTexIndex to index into dwbRoot->textureTable and
 *                applies the detail texture
 *========================================================================*/

static int applyDetailTexture (short texture, short detail)
{
    /*
     * At this point of time, we already know that these textures exist
     * and have been loaded by getDwbTexture(). So just get the pfTexture
     * structures by mapping the indices and then apply the Detail
     * texture.
     */

    pfTexture *tex, *det;
    dwbTexture *texInfo = &(dwbRoot->textureTable[texture]);
    dwbTexture *detInfo = &(dwbRoot->textureTable[detail]);
    float j,k,m,n,s;

  tex = sharedTexList[dwbRoot->pfTexIndex[texture]];
  det = sharedTexList[dwbRoot->pfTexIndex[detail]];

  pfTexDetail (tex, PFTEX_DEFAULT, det);

  j = detInfo->tex.detail[0],
  k = detInfo->tex.detail[1],
  m = detInfo->tex.detail[2],
  n = detInfo->tex.detail[3],
  s = detInfo->tex.detail[4]; 

  pfDetailTexTile (det, j, k, m, n, s);

  return 1;
}


/*========================================================================
 *
 * FUNCTION     : makePfGeoState
 * IN           : ptr to a intermediary dwb-geostate struct
 * OUT          : ptr to pfGeoState
 * DESCRIPTION  : make a performer geostate from the passed info.
 *
 *========================================================================*/

static pfGeoState *makePfGeoState ( dwbGeoState *gstate )
{
pfMaterial     *mtl;
pfGeoState     *pfGState;
float          alpha;
groupValues    *vals = &gstate->values;


  pfGState = pfNewGState(Arena);

  if (vals->texture >= 0)
  {
    unsigned int   *image;
    int             comp, sx, sy, sz;

    /* Assume alpha texture requires afunction */
    pfGetTexImage(sharedTexList[vals->texture], &image, &comp, 
                  &sx, &sy, &sz);

    if (comp == 2 || comp == 4)
    {
      /* Use multisample transparency if possible */
      if (GfxType & GFX_MULTISAMPLE)
      {
        pfGStateMode(pfGState, PFSTATE_ALPHAFUNC, PFAF_GEQUAL);
        pfGStateVal(pfGState, PFSTATE_ALPHAREF, 3.0f/255.0f);
        pfGStateMode(pfGState, PFSTATE_TRANSPARENCY, PFTR_ON);
      }
      else
      {
        pfGStateMode(pfGState, PFSTATE_ALPHAFUNC, PFAF_GREATER);
        pfGStateVal(pfGState, PFSTATE_ALPHAREF, 70.0f/255.0f);
        pfGStateMode(pfGState, PFSTATE_TRANSPARENCY, PFTR_HIGH_QUALITY);
      }
    }

    pfGStateAttr(pfGState, PFSTATE_TEXTURE, 
                 sharedTexList[vals->texture]);
    pfGStateAttr(pfGState, PFSTATE_TEXENV, 
                 sharedTevList[vals->texture]);

    pfGStateMode(pfGState, PFSTATE_ENTEXTURE, PF_ON);

  }
  else
  {
    pfGStateMode(pfGState, PFSTATE_ENTEXTURE, PF_OFF); 
  }

  mtl = getPfMaterial(gstate);

  if(mtl != NULL)
  {
    pfGStateAttr ( pfGState, PFSTATE_FRONTMTL, mtl );

    alpha = pfGetMtlAlpha(mtl);
    if (alpha < (1.0 - (1.0/256.0)) )
        pfGStateMode(pfGState, PFSTATE_TRANSPARENCY, PFTR_ON);

    /* Lighting should be enabled by default ?? */
    pfGStateMode(pfGState, PFSTATE_ENLIGHTING, PF_ON);
  }
  else
    pfGStateMode(pfGState, PFSTATE_ENLIGHTING, PF_OFF);

  /* Backface culling should be on by default */
  pfGStateMode(pfGState, PFSTATE_CULLFACE, (vals->backface)?PFCF_BACK:PFCF_OFF);

  /*--------------------------------------------------*/
  /* check for transparent (non-lit) groups           */
  /*--------------------------------------------------*/
  if ( vals->alpha < (1.0 - (1.0/256.0)) && vals->material == -1 )
      pfGStateMode(pfGState, PFSTATE_TRANSPARENCY, PFTR_ON);

  /*--------------------------------------------------*/
  /* check for non-solid geosets.                     */
  /*--------------------------------------------------*/
  if ( vals->drawstyle == DS_CLOSED_WIRE )
      pfGStateMode(pfGState, PFSTATE_ENWIREFRAME, PFTR_ON);

    return ( pfGState );
}


/*========================================================================
 *
 * FUNCTION     : getPfMaterial
 * IN           : ptr to a dwb geostate
 * OUT          : ptr to a Performer Material
 * DESCRIPTION  : Return Performer material corresponding to Dwb material
 *                and create new pfMaterial if necessary. This routine uses
 *                lmcolor mode LMC_AD to reduce number of materials needed.
 *
 *========================================================================*/

static pfMaterial *getPfMaterial ( dwbGeoState *gs )
{
    pfMaterial          *pfm;
    int                 mid;

    if ( !gs->values.binding  )     /* No material lighting */
        return ( NULL );


    if ( dwbRoot->materialTable == NULL ) /* no material table read  */
        gs->values.material = -1;         /* default to std material */

    mid = gs->values.material + 1;     /* + 1 for material == -1 */
    if (mid >= DWB_MAX_MATERIALS)
      {
	fprintf(stderr,"Error: maximum number of materials (%d) has been exceeded \n",
		DWB_MAX_MATERIALS);
	return 0;
      }
    

    if( dwbRoot->materials[mid] == NULL)
    {

        pfm = dwbRoot->materials[mid] = pfNewMtl(Arena);

        if ( gs->values.binding == MB_PER_GROUP )
            pfMtlColorMode(pfm, PFMTL_FRONT, PFMTL_CMODE_COLOR);
        else /* bind per primitive */
            pfMtlColorMode(pfm, PFMTL_FRONT, PFMTL_CMODE_AD);

        if(gs->values.material >= 0)
        {
            dwbMaterial         *dwbmat;

            dwbmat = &(dwbRoot->materialTable[gs->values.material].mat );

            if ( gs->values.binding == MB_PER_GROUP )
                pfMtlColor(pfm, PFMTL_DIFFUSE,  dwbmat->diffuse[0],
                                                dwbmat->diffuse[1],
                                                dwbmat->diffuse[2]);

            pfMtlColor(pfm, PFMTL_AMBIENT,  dwbmat->ambient[0],
                                            dwbmat->ambient[1],
                                            dwbmat->ambient[2]);


            pfMtlColor(pfm, PFMTL_SPECULAR, dwbmat->specular[0],
                                            dwbmat->specular[1],
                                            dwbmat->specular[2]);

            pfMtlColor(pfm, PFMTL_EMISSION, dwbmat->emissive[0],
                                            dwbmat->emissive[1],
                                            dwbmat->emissive[2]);

            pfMtlShininess(pfm, dwbmat->shininess);

            pfMtlAlpha(pfm, dwbmat->alpha);
        }
        else
        {
            /* Use GL default ambient of .2 */
            pfMtlColor(pfm, PFMTL_AMBIENT, .2, .2, .2);

            /* Use rest of GL defaults which are pfMaterial defaults. */
        }
    }

    return ( dwbRoot->materials[mid] );
}


/*========================================================================
 *
 * FUNCTION     : unPackRGBA
 * IN           : pached color & ptrs to rgb
 * OUT          : rgba as 4 shorts
 * DESCRIPTION  : unpack a cpack color definition into constituent rgb values.
 *
 *========================================================================*/

void unPackRGBA ( unsigned int pcolor,float *rgba )
{
    short sr = (pcolor << 24) >> 24;
    short sg = (((pcolor >> 8) << 24) >> 24);
    short sb = (pcolor << 8) >> 24;
    short sa = (pcolor >> 24);

    *(rgba)     = ((float)sr) / 255.0;
    *(rgba + 1) = ((float)sg) / 255.0;
    *(rgba + 2) = ((float)sb) / 255.0;
    *(rgba + 3) = ((float)sa) / 255.0;
}


/*========================================================================
 * Read a Packed vertex record and store it into the supplied polygon record
 *
 *========================================================================*/

static int handlePackedVertex(memPtr *ifp, int vertType, pfdGeom *polygon, short length)
{
int loop;
dwbPackedVertex vert;
float tmpVert[3];
float norm[3];
int vt;

  /* bsplines will not have any polygon length - all information */
  /* is given later in a special record */
  if (currentFacePType == 3)
  {
    memSeek(ifp,length,SEEK_CUR);
    return(1);
  }

  for (loop=0; loop< polygon->numVerts; loop++)
  {
    memRead(&vert,sizeof(dwbPackedVertex),1,ifp);
	swapDwbPackedVertex(&vert);
    USERFUNCS (DB_PACKED_VERTEX, &vert, (pfNode *) NULL, (pfNode *) NULL, ifp);

    copyNyup(tmpVert,vert.coords,dwbRoot->unitChange);
    copyNyup(norm,vert.normal,1.0);
    
    vt = vertType;

    if (vt & TEXTURED_VERT)
    {
      pfCopyVec2 ( polygon->texCoords[0][loop],vert.tex_coords );
      vt -= TEXTURED_VERT;
    }

    switch( vt )
    {
      case VT_FLAT_VERTEX:
        pfCopyVec3 ( polygon->coords[loop],tmpVert );
      break;

      case VT_SHADED_VERTEX:
        pfCopyVec3 ( polygon->coords[loop],tmpVert );
        unPackRGBA ( vert.packed_color,polygon->colors[loop] );
      break;
      case VT_ILLUM_VERTEX:
        pfCopyVec3 ( polygon->coords[loop],tmpVert );
        pfCopyVec3 ( polygon->norms[loop],norm );
      break;
      case VT_SHADED_ILLUM_VERTEX:
        pfCopyVec3 ( polygon->coords[loop],tmpVert );
        pfCopyVec3 ( polygon->norms[loop],norm );
        unPackRGBA ( vert.packed_color,polygon->colors[loop] );
      break;
    }
  }
  return(1);
}


/*========================================================================
 * Read a vertex record and store it into the supplied polygon record
 *
 *========================================================================*/

static int handleVertex(memPtr *ifp, int vertType, pfdGeom *polygon, short length)
{
int loop;
dwbVertex vert;
float tmpVert[3];
float norm[3];
int vt;

  /* bsplines will not have any polygon length - all information */
  /* is given later in a special record */
  if (currentFacePType == 3)
  {
    memSeek(ifp,length,SEEK_CUR);
    return(1);
  }

  for (loop=0; loop< polygon->numVerts; loop++)
  {
    memRead(&vert,sizeof(dwbVertex),1,ifp);
	swapDwbVertex(&vert);
    USERFUNCS (DB_VERTEX, &vert, (pfNode *) NULL, (pfNode *) NULL, ifp);

    copyNyup(tmpVert,vert.coords,dwbRoot->unitChange);
    copyNyup(norm,vert.normal,1.0);
    
    vt = vertType;

    if (vt & TEXTURED_VERT)
    {
      pfCopyVec2 ( polygon->texCoords[0][loop],vert.tex_coords );
      vt -= TEXTURED_VERT;
    }

    switch( vt )
    {
      case VT_FLAT_VERTEX:
        pfCopyVec3 ( polygon->coords[loop],tmpVert );
      break;

      case VT_SHADED_VERTEX:
        pfCopyVec3 ( polygon->coords[loop],tmpVert );
        pfCopyVec3 ( polygon->colors[loop],dwbRoot->colorTable[vert.color] );
        polygon->colors[loop][3] = dwbRoot->currentGroup.alpha;
      break;
      case VT_ILLUM_VERTEX:
        pfCopyVec3 ( polygon->coords[loop],tmpVert );
        pfCopyVec3 ( polygon->norms[loop],norm );
      break;
      case VT_SHADED_ILLUM_VERTEX:
        pfCopyVec3 ( polygon->coords[loop],tmpVert );
        pfCopyVec3 ( polygon->norms[loop],norm );
        pfCopyVec3 ( polygon->colors[loop],dwbRoot->colorTable[vert.color] );
        polygon->colors[loop][3] = dwbRoot->currentGroup.alpha;
      break;
    }
  }
  return(1);
}


/*========================================================================
 * Add a line to the supplied dwbGeoState
 *
 *========================================================================*/

static void addLineToGeoState( dwbGeoState *gstate, pfdGeom *polygon, 
                               int drawStyle)
{
myLines *newPoly;
int closed = 0;

  if (drawStyle == DS_CLOSED_WIRE)
      closed = 1;

  /* need to create a new polygon structure and add that to the line list */
  newPoly = pfCalloc(1,sizeof(myLines),NULL);

  newPoly->numVerts = polygon->numVerts;

  newPoly->cbind = polygon->cbind;

  /* copy the existing polygons values */
  newPoly->coords = pfCalloc(1,sizeof(pfVec3) * (newPoly->numVerts + closed),NULL);
  newPoly->norms = pfCalloc(1,sizeof(pfVec3) * (newPoly->numVerts + closed),NULL);
  newPoly->colors = pfCalloc(1,sizeof(pfVec4) * (newPoly->numVerts + closed),NULL);

  memcpy(newPoly->coords,polygon->coords,sizeof(pfVec3) * newPoly->numVerts);
  memcpy(newPoly->norms,polygon->norms,sizeof(pfVec3) * newPoly->numVerts);
  memcpy(newPoly->colors,polygon->colors,sizeof(pfVec4) * newPoly->numVerts);

  newPoly->next = gstate->lineList;

  /* update the head of list pointer */
  gstate->lineList = newPoly;

  /* if the drawstyle is a closed wire then add in the first point again */
  if (drawStyle == DS_CLOSED_WIRE)
  {
    memcpy(newPoly->coords[newPoly->numVerts],
           polygon->coords[0],sizeof(pfVec3));
    memcpy(newPoly->norms[newPoly->numVerts],
           polygon->norms[0],sizeof(pfVec3));
    memcpy(newPoly->colors[newPoly->numVerts],
           polygon->colors[0],sizeof(pfVec4));
    newPoly->numVerts++;
  }

}


/*========================================================================
 * Process the gstate list and build gsets for all gstates with polygons
 * or lines. Also takes into account whether the faces are decals,billboards,
 * or normal. Finally all gsets are added to the current parent node as 
 *
 *========================================================================*/

static void insertFaces(pfNode *parent,int faceType)
{
int         gset;
pfGeode     *geode;
pfList      *gsetList;
dwbGeoState *gstate;

  /* create the parent geode */
  geode  = pfNewGeode(); 

  for (gstate = dwbRoot->gsList; gstate; gstate = gstate->next)
  {
    /*----------------------------------------------*/
    /* convert face set into geosets and add each   */
    /* one  to the geode. The act of creating the   */
    /* geosets would reset this geostate builder    */
    /*----------------------------------------------*/
    if ( gstate->builder )
    {
      gsetList = (pfList *) pfdBuildGSets(gstate->builder); 
      if (pfGetNum(gsetList) > 0)
      {
        for (gset = 0; gset < pfGetNum(gsetList); gset++)
        {
          pfGeoSet *geoset = (pfGeoSet*)pfGet(gsetList, gset);
          /*------------------------------------------*/
          /* apply current geostate to this geoset.       */
          /*------------------------------------------*/
          pfGSetGState(geoset, gstate->pfgs );
          pfAddGSet(geode, geoset);
        }
      }
    }
    /* now render any lines */
    if (gstate->lineList)
           addLineList(geode,gstate);
  }

  if (pfGetNumGSets(geode) == 0) {
      pfDelete(geode);
  }
  else {
    switch( faceType ) {
      case BILLBOARD:
	pfAddChild(parent,(pfNode *) getBillboard(geode));
      break;

      /* 
       * Since pfLayerBase() and pfLayerDecal() are just convenience
       * routines, we can use pfAddChild() instead, without any problems.
       */

      case NORMAL: 
      case SEQUENCE:
      case LAYER_DECAL:    /* These two are only for the older version */
      case LAYER_BASE:     /* and when the renderGrp flag is set       */
	pfAddChild(parent,(pfNode *) geode);
      break;

      default:
        printf("\nLoadDwb: Internal Error: faceType incorrect\n");
      break;
    }
  }
}


printBindType(int bindType)
{
  switch (bindType) {
  	case PFGS_PER_VERTEX: 
				printf(" PFGS_PER_VERTEX ");
				break;
  	case PFGS_PER_PRIM: 
				printf(" PFGS_PER_PRIM ");
				break;
  	case PFGS_OFF: 
				printf(" PFGS_OFF ");
				break;
  }
}


printPolygonContents(pfdGeom *poly)
{
int loop;

  printf("Polygon Contents\n");
  printf("----------------------\n");

  printf("cbind : ");
  printBindType(poly->cbind);
  printf("\n");

  printf("nbind : ");
  printBindType(poly->nbind);
  printf("\n");

  printf("tbind : ");
  printBindType(poly->tbind[0]);
  printf("\n");

  printf("%d polygons \n",poly->numVerts);

  for (loop=0; loop < poly->numVerts; loop++)
    printf("%.2f, %.2f, %.2f\t: %.2f %.2f %.2f \t: %.2f %.2f %.2f\n",
      poly->coords[loop][0],poly->coords[loop][1],poly->coords[loop][2]
     ,poly->colors[loop][0],poly->colors[loop][1],poly->colors[loop][2],
      poly->norms[loop][0],poly->norms[loop][1], poly->norms[loop][2]);
}


/*========================================================================
 * Add a decal layer onto the current parent position
 * and then stores the children faces after that 
 * Used only by older versions. [v2.1]
 *======================================================================== */

static void handleDecal( pfNode **parent )
{
    pfLayer     *layer;

    /*--------------------------------------------------*/
    /* construct layer Node & geometry nodes to store   */
    /* base & decal geom.                               */
    /*--------------------------------------------------*/
    layer       = pfNewLayer();

    pfAddChild(*parent, (pfNode *) layer);

    insertFaces((pfNode *) layer,LAYER_BASE); 

    /* create a temporary NEW parent for this group */
    /* this parent will be retained until the next  */
    /* pop from the current tree level is received  */
    /* ie until the end of the group */

    *parent = (pfNode *)layer;
}


/*========================================================================
 *
 * FUNCTION	: addLineList
 * IN      	: ptr to current geode & start of a gstate line list.
 * OUT     	: nothing
 * DESCRIPTION  : create a non-indexed geoset for lines. 
 *		  NB. THIS IS A BAD IMPLEMENTATION...
 *		      I am creating many non-indexed geosets. One for each
 *		      poly-line in this geostate.
 *
 *========================================================================*/

static void addLineList ( pfGeode *geode,dwbGeoState *gstate )
{
    pfGeoSet 	*pfgs;
    myLines  	*pptr = gstate->lineList;
    int      	i,j,k,nverts = 0,nlines = 0;
    pfVec3	   *n3, *v3;
    pfVec4	   *c4;
 
    pfgs = pfNewGSet(Arena);

    nlines = 0;
    nverts = 0;

    while ( pptr )
    {
        nlines += pptr->numVerts -1;
        nverts += pptr->numVerts;
        pptr = pptr->next; 
    }


    if (nlines == 0)
      return;

    pfGSetPrimType ( pfgs, PFGS_LINES );
        
    pfGSetNumPrims ( pfgs,nlines ); 
    pfGSetAttr(pfgs, PFGS_TEXCOORD2, PFGS_OFF, NULL, NULL);
    pfGSetAttr(pfgs, PFGS_NORMAL3, PFGS_OFF, NULL, NULL);
    pfGSetAttr(pfgs, PFGS_COLOR4, PFGS_OFF, NULL, NULL);

    if ( gstate->values.binding )
    {
        v3 = (pfVec3*) pfMalloc( sizeof(pfVec3)*nlines * 2,Arena);
        n3 = (pfVec3*) pfMalloc( sizeof(pfVec3)*nlines * 2,Arena);
        c4 = (pfVec4*) pfMalloc( sizeof(pfVec4)*nlines * 2,Arena);

	pptr = gstate->lineList;
	j = k = 0;
    	while ( pptr )
    	{
            for ( i=1; i < pptr->numVerts; i++)
            {
	        PFCOPY_VEC3(v3[j], pptr->coords[i-1]);
	        PFCOPY_VEC3(n3[j], pptr->norms[i-1] );
        	if ( gstate->lineList->cbind == PFGS_PER_VERTEX )
	            PFCOPY_VEC4(c4[j], pptr->colors[i-1] );
	        j++;

	        PFCOPY_VEC3(v3[j], pptr->coords[i]);
	        PFCOPY_VEC3(n3[j], pptr->norms[i] );
        	if ( gstate->lineList->cbind == PFGS_PER_VERTEX )
	            PFCOPY_VEC4(c4[j], pptr->colors[i-1] );
	        j++;

                if ( gstate->lineList->cbind == PFGS_PER_PRIM )
	    	    PFCOPY_VEC4(c4[k], pptr->colors[0] );
	        k ++;
            }
	    
            pptr = pptr->next; 
    	}

        pfGSetAttr(pfgs, PFGS_COORD3,PFGS_PER_VERTEX, v3, NULL);
	pfGSetAttr(pfgs, PFGS_NORMAL3,PFGS_PER_VERTEX, n3, NULL);
	if ( gstate->lineList->cbind == PFGS_PER_VERTEX )
	    pfGSetAttr(pfgs, PFGS_COLOR4,PFGS_PER_VERTEX, c4, NULL);
	else if ( gstate->lineList->cbind == PFGS_PER_PRIM )
	    pfGSetAttr(pfgs, PFGS_COLOR4,PFGS_PER_PRIM, c4, NULL);
    }
    else
    {
        v3 = (pfVec3*) pfMalloc( sizeof(pfVec3)*nlines*2,Arena);
        c4 = (pfVec4*) pfMalloc( sizeof(pfVec4)*nlines*2,Arena);

	pptr = gstate->lineList;
	j = k = 0;
    	while ( pptr )
    	{
            for ( i = 1; i <=(pptr->numVerts -1); i++)
            {
	        PFCOPY_VEC3(v3[j], pptr->coords[i-1]);
        	if ( gstate->lineList->cbind == PFGS_PER_VERTEX )
	            PFCOPY_VEC4(c4[j], pptr->colors[i-1] );
	        j++;

	        PFCOPY_VEC3(v3[j], pptr->coords[i]);
        	if ( gstate->lineList->cbind == PFGS_PER_VERTEX )
	            PFCOPY_VEC4(c4[j], pptr->colors[i] );
	        j++;

                if ( gstate->lineList->cbind == PFGS_PER_PRIM )
	    	    PFCOPY_VEC4(c4[k], pptr->colors[0] );
	        k ++;
	    }

            pptr = pptr->next; 
    	}
    	
        pfGSetAttr(pfgs, PFGS_COORD3,PFGS_PER_VERTEX, v3, NULL);
	if ( gstate->lineList->cbind == PFGS_PER_VERTEX )
	    pfGSetAttr(pfgs, PFGS_COLOR4,PFGS_PER_VERTEX, c4, NULL);
	else
	    pfGSetAttr(pfgs, PFGS_COLOR4,PFGS_PER_PRIM, c4, NULL);
    }
    /* store the current line width */
    
    /* the following doesn't work with pf1.2 && non onyx's.  haven't tried it with 2.0*/
    pfGSetLineWidth(pfgs,lineWidth); 

    /* add callbacks as well as pfGSetLineWidth for now to handle non-onyx apps*/
    if (lineWidth > 1.0)
      {
      int *data;
      
      data = pfCalloc(1,sizeof(int),Arena);
      *data =  (int) lineWidth;
      pfNodeTravFuncs(geode, PFTRAV_DRAW,lineWidthOn,lineWidthOff);
      pfNodeTravData(geode, PFTRAV_DRAW, (void *) data);
      }
    
  
    /*----------------------------------------------*/
    /* apply current geostate to this geoset.	    */
    /*----------------------------------------------*/
    pfGSetGState ( pfgs, gstate->pfgs ); 
    pfAddGSet ( geode, pfgs );
    
    freeLineList ( gstate );
    
}

 
int lineWidthOn ( pfTraverser *trav, void *data )
{
int *size = (int  *) data;

  return 1;
}


int lineWidthOff ( pfTraverser *trav, void *data )
{
float *size = (float  *) data;


  return 1; 
}


static void freeLineList ( dwbGeoState *gstate )
{
    myLines *p,*pnext;

    p = gstate->lineList;
    while (p)
    {
        pnext = p->next;
        pfFree(p->coords);
        pfFree(p->norms);
        pfFree(p->colors);
        p = pnext;
    }
    gstate->lineList = NULL;
}


/* ==========================================================================
 * Set up performer structure that corresponds with the dwb lightpoint
 * definition
 * ========================================================================== */

static int handleLightPoint(memPtr *ifp, pfdGeom *polygon,pfNode **parent, 
			    pfNode *current, int vertType)
{
dwbLightPt lpnt;
pfGeode *geode;
pfGeoSet *gset;
pfGeoState *gstate;
pfLPointState *lpState;
pfVec3 *vertices;
pfVec4 *colors;
pfVec3 *normal;
short opcode,length;  
int loop;
int found;

  memRead(&lpnt,sizeof(dwbLightPt),1,ifp);
  swapDwbLightPt(&lpnt);
  USERFUNCS(DB_LIGHT_PT, &lpnt, NULL, *parent,ifp);

  geode = pfNewGeode ();
  gset = pfNewGSet (Arena);
  pfAddGSet (geode, gset);

  gstate = pfNewGState (Arena);
  lpState = pfNewLPState (Arena);

  pfGStateMode (gstate, PFSTATE_ENLPOINTSTATE, 1);
  pfGStateAttr (gstate, PFSTATE_LPOINTSTATE, lpState);

  pfGStateMode (gstate, PFSTATE_ENLIGHTING, 0);
  pfGStateMode (gstate, PFSTATE_ENFOG, 0); 
  pfGStateMode (gstate, PFSTATE_TRANSPARENCY, PFTR_BLEND_ALPHA);
  pfGStateVal (gstate, PFSTATE_ALPHAREF, 1.0f/255.0f);
  pfGStateMode (gstate, PFSTATE_ALPHAFUNC, PFAF_GREATER);
  pfGStateMode (gstate, PFSTATE_ENTEXTURE, 0);

  /*
   * I should be using the 'pntsize' field from the previous face record.
   * Right now just give it some defaults.
   * Also some of the other Performer 2.0 stuff is not yet supported in 
   * DWB 3.0. So just give it some defaults.
   */

  pfLPStateMode (lpState, PFLPS_SIZE_MODE, PFLPS_SIZE_MODE_ON);
  pfLPStateVal (lpState, PFLPS_SIZE_ACTUAL, lpnt.diam);

  pfLPStateVal (lpState, PFLPS_SIZE_MIN_PIXEL, 0.25f);
  pfLPStateVal (lpState, PFLPS_SIZE_MAX_PIXEL, 4.0f);

  pfLPStateVal (lpState, PFLPS_TRANSP_PIXEL_SIZE, 2.0f);
  pfLPStateVal (lpState, PFLPS_TRANSP_EXPONENT, 1.0f);
  pfLPStateVal (lpState, PFLPS_TRANSP_SCALE, 0.6f);
  pfLPStateVal (lpState, PFLPS_TRANSP_CLAMP, 0.1f); 

  pfLPStateVal (lpState, PFLPS_FOG_SCALE, 0.25f); 

  pfLPStateMode (lpState, PFLPS_RANGE_MODE, PFLPS_RANGE_MODE_TRUE);

  switch (lpnt.directionality)
  {
    case DB_OMNIDIRECTIONAL:
      pfLPStateMode (lpState, PFLPS_DIR_MODE, PFLPS_DIR_MODE_OFF);
    break;

    case DB_UNIDIRECTIONAL:
      pfLPStateMode (lpState, PFLPS_SHAPE_MODE, PFLPS_SHAPE_MODE_UNI);
      pfLPStateMode (lpState, PFLPS_DIR_MODE, PFLPS_DIR_MODE_ON);
    break;

    case DB_BIDIRECTIONAL:
      if (faceBackColor >= 0)
      {
      pfVec4 color;

        pfLPStateMode (lpState, PFLPS_SHAPE_MODE, PFLPS_SHAPE_MODE_BI_COLOR);
        pfCopyVec3 (color, dwbRoot->colorTable[faceBackColor]);
        color[3] = dwbRoot->currentGroup.alpha;    
	pfLPStateBackColor (lpState, color[0], color[1], color[2], color[3]);
      }
      else pfLPStateMode (lpState, PFLPS_SHAPE_MODE, PFLPS_SHAPE_MODE_BI);
      pfLPStateMode (lpState, PFLPS_DIR_MODE, PFLPS_DIR_MODE_ON);
    break;

    default:
      printf("LoadDwb: Error - unknown lightpoint type\n");
      return(0);
  }

  pfLPStateShape (lpState, lpnt.lwidth, lpnt.lheight, 0.0f, 1.0f, 0.1f);
  pfAddChild (*parent, geode);


  /*
   * Configure the geoset with PFGS_POINTS primitive type and PFGS_OVERALL
   * normal binding. Once DWB supports per-lightpoint-normal, I should
   * change this. Similarly, the face level color is used for all the light
   * points
   */

  vertices = pfCalloc (sizeof (pfVec3), polygon->numVerts, Arena);
  normal = pfCalloc (sizeof (pfVec3), 1, Arena);
  colors = pfCalloc (sizeof (pfVec4), polygon->numVerts, Arena);

  pfGSetPrimType (gset, PFGS_POINTS);
  pfGSetGState (gset, gstate);
  pfGSetNumPrims (gset, polygon->numVerts);

  /* 
   * now read in the vertex record associated with this light string 
   */

  found = parseToOpcode(ifp, DB_PUSH);
  if (!found)
  {
    pfNotify(PFNFY_WARN, PFNFY_USAGE, "LoadDwb:Error - file format incorrect \n");
    return (0);
  }
  else dwbPush(parent,current);

  memRead(&length,sizeof(short),1,ifp);
  length = get16(&length);
  memSeek(ifp,length,SEEK_CUR);

  /* next record MUSt be a vertex record */
  memRead(&opcode,sizeof(short),1,ifp);
  opcode = get16(&opcode);
  if (opcode != DB_VERTEX)
  {
    pfNotify(PFNFY_WARN, PFNFY_USAGE, "LoadDwb:Error - file format incorrect \n");
    return (0);
  }

  /* read the vertices into polygon */
  memRead (&length,sizeof(short),1,ifp);
  length = get16(&length);
  handleVertex (ifp, vertType, polygon, length);

  /* add the points to the light points node */
  for (loop=0; loop < polygon->numVerts; loop++)
  {
    PFCOPY_VEC3 (vertices[loop], polygon->coords[loop]);
    PFCOPY_VEC3 (colors[loop], polygon->colors[0]);
    colors[loop][3] = dwbRoot->currentGroup.alpha;
  }
  PFCOPY_VEC3 (normal[0], lpnt.dir);

  pfGSetAttr (gset, PFGS_NORMAL3, PFGS_OVERALL, (void *) normal, NULL);
  pfGSetAttr (gset, PFGS_COORD3, PFGS_PER_VERTEX, (void *) vertices, NULL);
  pfGSetAttr (gset, PFGS_COLOR4, PFGS_PER_VERTEX, (void *) colors, NULL);

  return(1);
}


/* ==========================================================================
 * draw callback for a bspline group. Just calls gl routine
 * ========================================================================== */
 
static int drawBspline ( pfTraverser *trav, void *data )
{
BsplineInfo *bsp = (BsplineInfo *) data;
  pfPushState();
  pfDisable(PFEN_TEXTURE);
  pfDisable(PFEN_LIGHTING);

  glColor4f (bsp->color[0], bsp->color[1], bsp->color[2], bsp->color[3]);
  gluBeginCurve (bsp->nobj);
    gluNurbsCurve (
      bsp->nobj, 
      (GLint) bsp->numKnots, bsp->knots, 
      3, bsp->cntlPnts, 
      4, GL_MAP1_VERTEX_3
    );

  gluEndCurve (bsp->nobj);
  pfPopState();

  return PFTRAV_CONT;
}

 
static int clipRegionOn ( pfTraverser *trav, void *data )
{
ClipRegion *clip = (ClipRegion *) data;

  if (clip->gotOld)
  {
    glEnable (GL_SCISSOR_TEST);
    glScissor (clip->min[0], clip->min[1], 
	       clip->max[0] - clip->min[0] - 1, 
	       clip->max[1] - clip->min[1] - 1);
  }
  
  return PFTRAV_CONT;
}


static int clipRegionOff ( pfTraverser *trav, void *data )
{
ClipRegion *clip = (ClipRegion *) data;

GLfloat projection_matrix[4][4];

static int countdown = 3; /* nothing may be drawn for first three frames */

   if (!countdown)
   {
   glMatrixMode (GL_PROJECTION);
   glGetFloatv (GL_PROJECTION_MATRIX, (GLfloat *) projection_matrix);
   glMatrixMode (GL_MODELVIEW);

    if (clip->gotOld || clipMode == CLIP_RECALCULATE )
    {
      if (!clip->gotOld)
      {
      GLint temp[4];

	glGetIntegerv (GL_SCISSOR_BOX, temp);
	clip->oldmin[0] = temp[0];
	clip->oldmin[1] = temp[1];
	clip->oldmax[0] = temp[0]+temp[2]-1;
	clip->oldmax[1] = temp[1]+temp[3]-1;
      }

      if (clip->oldmin[0] == 0 && clip->oldmax[0] ==0 
          && clip->oldmin[1] == 0 && clip->oldmax[1] ==0)
      {
        printf("Error: zero scrmask \n");
	clip->oldmin[0] =0;
	clip->oldmax[0] =1280;
	clip->oldmin[1] =0;
	clip->oldmax[1] =1024;
      }
    


      clip->channel = pfGetTravChan(trav);
      pfGetChanSize(clip->channel,&clip->chanSize[0],&clip->chanSize[1]);
      pfGetChanOrigin(clip->channel,&clip->chanOrigin[0],&clip->chanOrigin[1]);

      map_viewport (clip,clip->channel,clip->ll,clip->ur,clip->min,clip->max);

      clip->gotOld = 1; 
    }
    glEnable (GL_SCISSOR_BOX);
    glScissor (clip->oldmin[0], clip->oldmin[1], 
	       clip->oldmax[0] - clip->oldmin[0]-1, 
	       clip->oldmax[1] - clip->oldmin[1]-1);
  }
  else
    countdown --;

  return PFTRAV_CONT;
}


/* ==========================================================================
 * Performer callback to ensure that a node with no geometry is switched on
 * ========================================================================== */
int switchOn(pfTraverser *trav, void *data)
{
  return PFTRAV_CONT;
}


/* ==========================================================================
 * Take a group thats a bspline and set up the appropriate callback so that
 * it gets rendered correctly
 * ========================================================================== */

static int handleBSpline(memPtr *ifp, short length, pfNode *current, 
                         pfNode *parent)
{
dwbBSpline bspline;
double *knots,*cntlPnts;
pfVec3 *points;
short opcode;
BsplineInfo *bsp;
pfGroup *grp;
pfSphere sphere;
int i, numPoints;

  /* read the bspline record*/
  memRead(&bspline,length,1,ifp);
  swapDwbBSpline(&bspline);

  /* read the next record which should be a knots record */
  memRead(&opcode,sizeof(short),1,ifp);
  memRead(&length,sizeof(short),1,ifp);

  opcode = get16(&opcode);
  length = get16(&length);

  if (opcode != DB_BSPLINE_KNOTS )
  {
    memSeek(ifp,-4,SEEK_CUR);
    printf("Error: bspline record not followed by knots record \n");
    return(0);
  }

  knots = pfMalloc(length,Arena);
  memRead(knots,length,1,ifp);
  get32v(knots, knots, length);

  /* now read the control points */
  memRead(&opcode,sizeof(short),1,ifp);
  memRead(&length,sizeof(short),1,ifp);
  opcode = get16(&opcode);
  length = get16(&length);

  if (opcode != DB_BSPLINE_CNTRL_PNTS)
  {
    memSeek(ifp,-4,SEEK_CUR);
    printf("Error: bspline record not followed by control points record \n");
    return(0);
  }


  cntlPnts = pfMalloc(length,Arena);
  memRead(cntlPnts,length,1,ifp);
  get32v(cntlPnts, cntlPnts, length);

  /* now do something with the blighters */
  bsp = pfMalloc(sizeof(BsplineInfo),Arena);

  /* setup a pfVec3 array for calculating the bounding sphere */
  numPoints = length/sizeof(double)/3;
  points = (pfVec3 *) pfMalloc (sizeof (pfVec3)*numPoints, Arena);
  for (i=0; i<length/sizeof(double); ++i) {
    points[i/3][i%3] = (float) cntlPnts[i];
  } 

  bsp->numKnots = bspline.numKnots;

  bsp->knots = pfCalloc (bsp->numKnots, sizeof (GLfloat), Arena);
  bsp->cntlPnts = pfCalloc (numPoints*3, sizeof (GLfloat), Arena);
  for (i=0; i<bsp->numKnots; ++i) bsp->knots[i] = (GLfloat) knots[i];
  for (i=0; i<numPoints*3; ++i) bsp->cntlPnts[i] = (GLfloat) cntlPnts[i];
  pfFree (knots);
  pfFree (cntlPnts);
  bsp->nobj = gluNewNurbsRenderer ();

  pfCopyVec4(bsp->color,nextBSplineColor);

  /* make a new node */
  grp = pfNewGroup();
  pfAddChild(parent,grp);

  USERFUNCS (DB_BSPLINE, &bspline, (pfNode *)grp, parent,ifp);
  USERFUNCS (DB_BSPLINE_KNOTS, &knots, (pfNode *)grp, parent,ifp);
  USERFUNCS (DB_BSPLINE_CNTRL_PNTS, &cntlPnts, (pfNode *)grp, parent,ifp);

  pfNodeTravFuncs(grp,PFTRAV_CULL,switchOn, NULL); 
  pfNodeTravFuncs(grp, PFTRAV_DRAW,drawBspline,NULL);
  pfNodeTravData(grp, PFTRAV_DRAW, (void *) bsp);

  /* ----------------------------------------- */
  /* Create a culling sphere around the group  */
  /* ----------------------------------------- */

  pfSphereAroundPts (&sphere, points, numPoints);
  pfNodeBSphere(grp, &sphere, PFBOUND_STATIC);
  pfFree (points);

  return (1);
}


static int handleArc (memPtr *ifp, short length,pfNode *current,
		            pfNode *parent)
{
    dwbArc arc;
    ArcInfo *arcinfo;
    pfGroup *group;

  memRead (&arc, length, 1, ifp);
  swapDwbArc(&arc);
  arcinfo = pfMalloc (sizeof(ArcInfo), Arena);

  pfCopyMat (arcinfo->matrix, arc.matrix);
  pfCopyVec3 (arcinfo->pfNormal, arc.pfNormal);
  arcinfo->drawstyle = arc.drawstyle;
  arcinfo->fRadius = arc.fRadius;
  arcinfo->fStartAngle = arc.fStartAngle;
  arcinfo->fEndAngle = arc.fEndAngle;

  {								
      float tmp_arr[4]; int tmp_int;				
    if ( arc.color == PACKED_COLOR_INDEX )			
    {								
        unPackRGBA ( arc.packed_color,tmp_arr );			
        for ( tmp_int=0; tmp_int<3; tmp_int++ )			
            arcinfo->color[tmp_int] = tmp_arr[tmp_int]; 
    }							
    else							
	pfCopyVec3 ( arcinfo->color,dwbRoot->colorTable[arc.color] );
    arcinfo->color[3] = dwbRoot->currentGroup.alpha;		
  }

  group = pfNewGroup ();
  pfAddChild (parent, group);

  USERFUNCS (DB_ARC, &arc, (pfNode *) group, parent, ifp);

  pfNodeTravFuncs (group, PFTRAV_CULL, switchOn, NULL);
  pfNodeTravFuncs (group, PFTRAV_DRAW, drawArc, NULL);
  pfNodeTravData (group, PFTRAV_DRAW, (void *) arcinfo);

  pfNodeBSphere (group, &groupBoundingSphere, PFBOUND_STATIC);

  return 1;
}


static int drawArc ( pfTraverser *trav, void *data ) 
{
    ArcInfo *arcinfo = (ArcInfo *) data;

  pfPushState();
  pfDisable (PFEN_TEXTURE);
  pfDisable (PFEN_LIGHTING);
  pfPushMatrix();

  glMatrixMode (GL_MODELVIEW);

  pfMultMatrix(arcinfo->matrix);

  /* XXX need to do the right thing here */
    
  pfPopMatrix();
  pfPopState();

  return PFTRAV_CONT;
}


/* ==========================================================================
 * Convert a geode into a pfBillboard and return a pointer to that billboard.
 * The current implementation of Performer 1.2 has a number of problems
 * with Billboards. These are

	1) Only geometry that is XZ planar will be drawn. Anything else
	   is rotated incorrectly. For our case this also means that
	   the geometry MUST be defined Z-up. Any attempt I make to correct
	   foils the performer pfBillboard logic.

	2) Contrary to the manual, _all_ primitives of a geoset are rotated
	   independently, and again only faces that are planar in XZ will
	   be displayed. Effectively, this means that a billboard needs to
	   be a single face in the XZ plane.
 *
 *
 * ========================================================================= */

static pfBillboard *getBillboard(pfGeode *geode)
{
int gset;
pfBox box1,box2,*box,*bigBox;
int nbox;
pfVec3 pnt;
pfVec3 axis;
pfSCS *scs;
pfMatrix mat;
pfBillboard *bill;
 
  /* ------------------------------------ */ 
  /* First find centre point of the geode */
  /* ------------------------------------ */ 
  nbox = pfGetNumGSets(geode);

  /* use pointers to the structures to simplify the next bit */
  box = &box1;
  bigBox = &box2;

  for (gset = 0; gset < nbox; gset++)
  {
    pfGeoSet *geoset = (pfGeoSet*)pfGetGSet(geode, gset);
    /*-------------------------*/
    /* Calculate bounding box  */
    /*-------------------------*/
    pfGSetBBox(geoset, NULL, PFBOUND_DYNAMIC);
    pfGetGSetBBox(geoset,box);
    if (gset == 0)
        memcpy(bigBox,box,sizeof(pfBox));
    else     
        PFBOX_EXTENDBY_BOX(bigBox, box);
  }

  /* ---------------------------------------------------------- */
  /* now that we have the overall bounding box - get the centre */
  /* ---------------------------------------------------------- */
  pnt[0] = (bigBox->min[0] + (bigBox->max[0] - bigBox->min[0])/2.0f);
  pnt[1] = (bigBox->min[1] + (bigBox->max[1] - bigBox->min[1])/2.0f);
  pnt[2] = (bigBox->min[2] + (bigBox->max[2] - bigBox->min[2])/2.0f);

  /* --------------------------------------------------------------- */
  /* translate the geometry so that it will rotate around its origin */
  /* --------------------------------------------------------------- */

  pfMakeTransMat(mat,-pnt[0],-pnt[1],-pnt[2]);
  scs = pfNewSCS(mat);
  pfAddChild(scs,geode);
  pfFlatten(scs,0);

  /* ----------------------------------------------------------- */
  /* need to extract the gsets from the geode and attach them to */
  /* our billboard and set the position of each geoset.          */
  /* ----------------------------------------------------------- */

  bill = pfNewBboard();

  pfBboardMode(bill, PFBB_ROT, PFBB_AXIAL_ROT);
  pfSetVec3(axis, 0.0, 0.0, 1.0);
  pfBboardAxis(bill, axis);

  for (gset = 0; gset < nbox; gset++)
  {
    pfGeoSet *geoset = (pfGeoSet*)pfGetGSet(geode, gset);

    pfAddGSet(bill, geoset);
    pfRemoveGSet(geode, geoset);
    pfBboardPos(  bill,gset, pnt);       
  }

  return(bill);
}


getOrientation( void )
{
  if ( dwbRoot->header.up_axis == Y_UP )
    return(1);
  else
    return(0);

}


convertOrigin( float val[3] )
{
float buff[3];

  copyNyup(buff,val,dwbRoot->unitChange)
  memcpy(val,buff,sizeof(float) *3 );
}


float getUnitChange( void )
{
  return (dwbRoot->unitChange);
}


void getCpackColor( int index, float *cols )
{
  pfCopyVec3(cols, dwbRoot->colorTable[index]);
}



/* ------------------------------------------------------------------------- */
/*                     User Callback Processing                              */
/* ------------------------------------------------------------------------- */


/*
 * set all user callbacks to the standard value -  but just the once
 */

int initUserFuncs(void)
{
int loop;
static int alreadyCalled = 0;

  if (alreadyCalled)
    return 0;
  else
    alreadyCalled = 1;

  for (loop=0;loop< DB_LASTREC - DB_HEADER; loop++)
    userFunctions[loop].func = NULL;
  return 1;
}


/* 
 * Set a user defined callback into the callback table
 * return -1 on failure, 1 on success
 */

int setUserFuncs(int type, int (func)(int type, void *rec, 
                 pfNode *current, pfNode *parent, memPtr *ifp))
{
static int triedToInit=0;

  if (!triedToInit)
  {
    initUserFuncs();
    triedToInit = 1;
  }

  if (type > DB_LASTREC || type < DB_HEADER)
  {
    pfNotify(PFNFY_WARN, PFNFY_USAGE, "Opcode %d is outside of valid range (%d - %d) \n",type,
           DB_HEADER,DB_LASTREC);
    return(-1);
  }

  userFunctions[type - DB_HEADER].func = func;
  return(1);
}


int memEof( memPtr *ptr)
{
  if (ptr->pos >= ptr->length)
    return (1);
  else
    return(0);
}


static void addToFilePath(char *str)
{
const char *existing;
static char *new;
int length;

  existing = pfGetFilePath();
  if (! existing) length = strlen(str);
  else length = strlen (str) + strlen (existing);

  new = pfCalloc( 1,length + 4,NULL);
  if (existing)
  {
    strcat (new, existing);
 
    /* If path does not already end with ":", then add it */
    length = strlen(new);
    if (length > 0 && new[length - 1] != ':')
	strcat(new, ":");
  }
  strcat (new, str);
  pfFilePath(new);
}


/*========================================================================
 *
 * Try and find the file given by looking in the standard places,
 * following CS_PATH, and finally PF_PATH
 * If the file is found then return a pointer to the memPtr otherwise
 * return NULL
 *
 *======================================================================== */

static memPtr *openFile (char *fileName)
{
static char csPath[PF_MAXSTRING] =  "/usr/cs/es4.0/models/:" 
				    "/usr/cs/es4.0/textures:";
#undef memPtr
char path[PF_MAXSTRING];
char *esDir;
FILE *ifp;
int count;
static int firstTime = 1;

  if (firstTime)
  {

    /* append the CS_PATH environment variable */
    if (getenv("CS_PATH"))
      strncat(csPath,getenv("CS_PATH"), PF_MAXSTRING - strlen (csPath));

    esDir = getenv("ES_DIR");
    if (esDir)
    {
        strncat(csPath, ":", PF_MAXSTRING - strlen (csPath));

        strncat(csPath, esDir, PF_MAXSTRING - strlen (csPath));
        strncat(csPath, "/models:", PF_MAXSTRING - strlen (csPath));

        strncat(csPath, esDir, PF_MAXSTRING - strlen (csPath));
        strncat(csPath, "/textures:", PF_MAXSTRING - strlen (csPath));
    }

    /* set the search path */
    addToFilePath(csPath);

    firstTime = 0;
  }

  /*------------------------------------------*/
  /* find the file...                         */
  /*------------------------------------------*/
  if (!pfFindFile(fileName, path, R_OK))
  {
    /* this is causing an intermitent core dump */
    /* commenting out seems to solve the problem */

    pfNotify(PFNFY_WARN,PFNFY_RESOURCE,
            "LoadDwb: Could not find \"%s\" in\n  %s\n", fileName,
	    pfGetFilePath ());

    return (NULL);
  }

  if (pfGetNotifyLevel() >= PFNFY_NOTICE)
      pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "LoadDwb: %s\n", fileName);

  /*------------------------------------------*/
  /* try and open it                          */
  /*------------------------------------------*/
  if ( (ifp = fopen(path, "rb")) == NULL )
  {
      pfNotify(PFNFY_WARN,PFNFY_RESOURCE,
              "LoadDwb: Could not access \"%s\"", path);
      return (NULL);
  }

  /* ok its a file - prepare to load it -init "all" the global variables */
  initLoader();

  /* get the file length */
  fseek(ifp,0,SEEK_END);
  dwbRoot->file.length = ftell(ifp);

  if (dwbRoot->file.length == 0) 
  {
    pfNotify (PFNFY_WARN, PFNFY_RESOURCE,
	      "LoadDwb: File \"%s\" is empty\n", path);
    return (NULL);
  }

  /* create the temporary buffer */
  dwbRoot->file.ptr = pfMalloc(dwbRoot->file.length,NULL);

  /* reset the file ptr */
  fseek(ifp,0,SEEK_SET);

  /* read everything into memory */
  count = fread(dwbRoot->file.ptr,1,dwbRoot->file.length,ifp);

  if (count != dwbRoot->file.length)
  {
    perror("Error Reading file into memory buffer");
    return(NULL);
  }

  fclose(ifp);

  /* initialise the "file" pointer */
  dwbRoot->file.pos = 0;

  return (&dwbRoot->file);

}


int wtoscrn (GLfloat the_matrix[4][4], float world_pt [3], short screen_pt [2])
{
    float xnew, ynew, znew, wnew,wid,hgt;
    short x1, x2, y1, y2;

    xnew = world_pt [0] * the_matrix [0][0] + world_pt [1] *
      the_matrix [1][0] + world_pt [2] * the_matrix [2][0] + the_matrix [3][0];

   ynew = world_pt [0] * the_matrix [0][1] + world_pt [1] *
     the_matrix [1][1] + world_pt [2] * the_matrix [2][1] + the_matrix [3][1];

    znew = world_pt [0] * the_matrix [0][2] + world_pt [1] *
      the_matrix [1][2] + world_pt [2] * the_matrix [2][2] + the_matrix [3][2];

    wnew = world_pt [0] * the_matrix [0][3] + world_pt [1] *
      the_matrix [1][3] + world_pt [2] * the_matrix [2][3] + the_matrix [3][3];

    xnew = xnew / wnew;        /* wnew is an identity that you need */
    ynew = ynew / wnew;


    /*------------------------------------------------------------------*/
    /* if the screen coord needs to be actual physical screen values    */
    /* as opposed to Window relative then add in the origin factor..    */
    /*------------------------------------------------------------------*/

    {
    GLint tmp[4];

      glGetIntegerv (GL_VIEWPORT, tmp);
      x1 = tmp[0];
      x2 = tmp[0] + tmp[2]-1;
      y1 = tmp[1];
      y2 = tmp[1] + tmp[3]-1;
    }

    wid = (float)(x2 - x1);
    hgt = (float)(y2 - y1);

    screen_pt [0] = (int)((xnew+1)/2 * wid + (float)x1);
    screen_pt [1] = (int)((ynew+1)/2 * hgt + (float)y1);

    return 1;
}


/*==============================================================*

   Routine      : (void) map_viewport
   Purpose      : Maps groups world ll & ur coords into screen
                  values for use as either a viewport or scrnmask.
   Parameter(s) : (IP) group_ptr - ptr to group whose viewport is
                       being defined
                  (IP) width - width of viewport, in pixels
                  (IP) height - height of viewport, in pixels
   Returns      : Nothing (void)
   Description  :

 *==============================================================*/

int map_viewport (ClipRegion *clip,pfChannel *chan, float ll[3],float ur[3], short vw_ll[3],short vw_ur[3])
{
   int tmp;

   GLfloat curr_matrix[4][4], view_matrix[4][4], projection_matrix[4][4];

   pfMatrix mat,mat2;
   short left, right, bottom, top;
   int width,height;
   pfCoord coords;
   pfVec3 dst;
   pfMatrix modelPos;

   /* Determine the current matrix */
   glMatrixMode (GL_PROJECTION);
   glGetFloatv (GL_PROJECTION_MATRIX, (GLfloat *) projection_matrix);
   glMatrixMode (GL_MODELVIEW);

   pfCopyMat(mat,projection_matrix);

   /* view matrix contains the 90x rot + channel matrix + channel offset */
   glGetFloatv (GL_MODELVIEW_MATRIX, (GLfloat *) view_matrix);

   pfPreMultMat(mat,view_matrix);

   pfXformPt3(dst,ll,mat);
   wtoscrn (mat, ll, vw_ll);

   pfXformPt3(dst,ur,mat);
   wtoscrn (mat, ur, vw_ur);

   if (vw_ll [0] > vw_ur [0])
   {
      tmp = vw_ll [0];
      vw_ll [0] = vw_ur [0];
      vw_ur [0] = tmp;
   }

   if (vw_ll [1] > vw_ur [1])
   {
      tmp = vw_ll [1];
      vw_ll [1] = vw_ur [1];
      vw_ur [1] = tmp;
   }

   return 1;
}


int printMatrix (GLfloat mat[4][4])
{
    register int i,j;

    printf("\n");
    printf("MATRIX ...\n" );

    for (i=0; i<4; i++)
    {
        for (j=0; j<4; j++)
            printf("%f ",mat[i][j] );
        printf ("\n");
    }

  return 1;
}


setClipMode(int mode)
{
  clipMode = mode;
}


/* return a list of line styles used by the current model */
short *getCurrentLineStyles()
{
  short *list;
  int loop;
  
  
  list = pfCalloc(numLineStyles,sizeof(short),Arena);
  for (loop=0; loop < numLineStyles; loop++)
    list[loop] = lineStyles[loop].style;
  
  return(list);
}


/* return a list of line repeats used by the current model */
short *getCurrentLineRepeats()
{
  short *list;
  int loop;
  
  
  list = pfCalloc(numLineStyles,sizeof(short),Arena);
  for (loop=0; loop < numLineStyles; loop++)
    list[loop] = lineStyles[loop].repeat;
  
  return(list);
  
  
}


/* returns the number of linestyles for the current model */
int getNumLineStyles()
{
  return numLineStyles;
  
}

/* returns a copy of the color table for the current object */
float *getColorTable()
{
  float *table;

    table = pfCalloc(4224*3, sizeof(float), Arena);
    memcpy(table,dwbRoot->colorTable, 4224*3 * sizeof(float) );
    
  
  return table;
  
}

void setLastNodeStatus(int status)
{
    lastNodeIsDrawable = status;
}
