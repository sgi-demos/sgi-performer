/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
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
 * pfim.c - simple "Performer" loader.
 * 
 * $Revision: 1.73 $ 
 * $Date: 2002/11/07 03:11:55 $
 */

/*
 * This is sample loader that uses on-disk representations that correspond
 * closely to those in Performer itself.  It's useful for demonstrating 
 * pfFonts and similar objects not supported in any standard formats.
 * The .im format itself is likely to change substantially or disappear
 * in future releases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef __linux__
#ifdef	_POSIX_SOURCE
extern char *strdup (const char *s1);
#endif
#endif

#ifdef WIN32
#include <windows.h>
//#include "wintypes.h"

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

static int breakup = 0;
static char GEODE_NODE[] = "geode";
static char BILLBOARD_NODE[] = "bboard";
static char SWITCH_NODE[] = "switch";
static char SEQUENCE_NODE[] = "sequence";
static char ROOT_NODE[] = "root";
static char GROUP_NODE[] = "group";
static char SCS_NODE[] = "scs";
static char DCS_NODE[] = "dcs";
static char FONT[] = "font";
static char TEXT[] = "str_text";
static char LOD_NODE[] = "lod";
static char LODSTATE[] = "lodstate";
static char LOD_LODSTATE[] = "lod_lodstate";
static char LOD_LODSTATE_INDEX[] = "lod_lodstate_index";
static char COLOR[] = "color";
static char COLORS[] = "colors";
static char NO_COLOR[] = "nocolor";
static char POLY[] = "poly";
static char LINE[] = "line";
static char MYPOINT[] = "point";
static char VERTEX[] = "vc";
static char TEX_COORD[] = "tc";
static char NORMAL_COORD[] = "nc";
static char ATTACH[] = "attach";
static char CLIPTEXTURE[] = "cliptex";
static char CLIPCENTER[] = "clipcenter";
static char TEXGENCLIPCENTER[] = "texgenclipcenter";
static char SPHEREFRONT[] = "spherefront";
static char TEXTURE[] = "tex";
static char NO_TEXTURE[] = "notex";
static char BREAKUP[] = "breakup";
static char SHARE[] = "share";
static char NEW_NODE[] = "new";
static char TGEN_REFLECTIVE[] = "tgen_reflective";
static char TGEN_LINEAR[] = "tgen_linear";
static char TGEN_CONTOUR[] = "tgen_contour";
static char TGEN_OFF[] = "tgen_off";
static char TGEN_PARAMS[] = "tgen_params";
static char LIGHT_POINT_NODE[] = "lp";
static char LOAD_GEODE[] = "load_geode";
static char END_STRING[] = "end";
static char LOAD_STATE[] = "load_state";
static char STORE_STATE[] = "store_state";
static char PUSH_STATE[] = "push_state";
static char POP_STATE[] = "pop_state";
static char DEFINE_DEFAULT[] = "define_default_state";
static char RESET_STATE[] = "reset_state";
static char BOUNDING_SPHERE[] = "bounding_sphere";
static char LOAD_FILE[] = "load_file";
static char ASD_NODE[] = "asd";

pfFont *FindFontInList(pfList *list, char *name)
{
    int i,n;
    pfFont *fnt;

    n = pfGetNum(list);
    for(i=0;i<n;i++)
    {
	fnt = pfGet(list,i);
	if ((char *)pfGetFontAttr(fnt,PFFONT_NAME) && 
		(strcmp(name,(char *)pfGetFontAttr(fnt,PFFONT_NAME)) == 0))
	{
	    return fnt;
	}
    }
    return NULL;
}

pfNode *FindInList(pfList *list, char *name)
{
    int i,n;
    pfNode *nd;

    n = pfGetNum(list);
    for(i=0;i<n;i++)
    {
	nd = pfGet(list,i);
	if (pfGetNodeName(nd) && (strcmp(name,pfGetNodeName(nd)) == 0))
	    return nd;
    }
    return NULL;	  
}

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_im (char *fileName)
{
char GEODE_NODE[] = "geode";
char BILLBOARD_NODE[] = "bboard";
char SEQUENCE_NODE[] = "sequence";
char SWITCH_NODE[] = "switch";
char ROOT_NODE[] = "root";
char GROUP_NODE[] = "group";
char SCS_NODE[] = "scs";
 char DCS_NODE[] = "dcs";
 char FONT[] = "font";
 char TEXT[] = "str_text";
 char LOD_NODE[] = "lod";
 char LODSTATE[] = "lodstate";
 char LOD_LODSTATE[] = "lod_lodstate";
 char LOD_LODSTATE_INDEX[] = "lod_lodstate_index";
 char COLOR[] = "color";
 char COLORS[] = "colors";
 char NO_COLOR[] = "nocolor";
 char POLY[] = "poly";
 char LINE[] = "line";
 char MYPOINT[] = "point";
 char VERTEX[] = "vc";
 char TEX_COORD[] = "tc";
 char NORMAL_COORD[] = "nc";
 char ATTACH[] = "attach";
 char TEXTURE[] = "tex";
 char NO_TEXTURE[] = "notex";
 char BREAKUP[] = "breakup";
 char SHARE[] = "share";
 char NEW_NODE[] = "new";
 char TGEN_REFLECTIVE[] = "tgen_reflective";
 char TGEN_LINEAR[] = "tgen_linear";
 char TGEN_CONTOUR[] = "tgen_contour";
 char TGEN_OFF[] = "tgen_off";
 char TGEN_PARAMS[] = "tgen_params";
 char NEW_TEXGEN[] = "new_tgen";
 char NEW_TEXGEN_OFF[] = "new_tgen_off";
 char LIGHT_POINT_NODE[] = "lp";
 char LOAD_GEODE[] = "load_geode";
 char END_STRING[] = "end";
 char LOAD_STATE[] = "load_state";
 char STORE_STATE[] = "store_state";
 char PUSH_STATE[] = "push_state";
 char POP_STATE[] = "pop_state";
 char DEFINE_DEFAULT[] = "define_default_state";
 char RESET_STATE[] = "reset_state";

    FILE		*imFile;
    pfdGeom		*polygon;
    pfList		*nodeList;
    pfList		*fontList;
    pfList		*clipList;
    pfList		*clipNameList;
    pfGroup		*root = NULL;
    int			count;
    int			primType = PFGS_POLYS;
    char		buf[256];
    char 		endBuf[256];
    char		name[256];
    char		fname[1024];
    char		nodeType[256];
    char		nodeName[256];
    char		childName[256];
    char		parentName[256];
    long txgn;
    pfMaterial  *m      	= NULL;
    pfTexGen 	*tgen		= NULL;
    float *params = NULL;
    int			t	= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    if ((imFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    pfdBldrAttr(PFDBLDR_NODE_NAME_COMPARE,(void *)strcmp);
    pfdBldrAttr(PFDBLDR_STATE_NAME_COMPARE,(void *)strcmp);

    /* establish material definition for model */
    m = (pfMaterial*)pfdGetTemplateObject(pfGetMtlClassType());
    pfMtlColor(m, PFMTL_AMBIENT,  0.2f, 0.2f, 0.2f);
    pfMtlColor(m, PFMTL_DIFFUSE,  0.8f, 0.8f, 0.8f);
    pfMtlColor(m, PFMTL_SPECULAR, 1.0f, 1.0f, 1.0f);
    pfMtlShininess(m, 30.0f);
    pfMtlColorMode(m, PFMTL_FRONT, PFMTL_CMODE_OFF);
    pfdBldrStateAttr(PFSTATE_FRONTMTL, m);
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_ON);
    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);

    txgn = pfdAddState(strdup("texgen"),
			8*sizeof(float),NULL,NULL,NULL,NULL,NULL);
    pfdStateCallback(txgn, PFDEXT_DRAW_PREFUNC,pfdPreDrawTexgenExt);
    pfdStateCallback(txgn, PFDEXT_DRAW_POSTFUNC,pfdPostDrawTexgenExt);

    nodeList = pfNewList(sizeof(pfNode *),1,pfGetSharedArena());
    fontList = pfNewList(sizeof(pfNode *),1,pfGetSharedArena());
    clipList = pfNewList(sizeof(pfMPClipTexture*),1,pfGetSharedArena());
    clipNameList = pfNewList(sizeof(pfMPClipTexture*),1,pfGetSharedArena());
    count = 0;
    polygon = pfdNewGeom(300);
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	polygon->tbind[t] = PFGS_OFF;    
    polygon->nbind = PFGS_OFF;
    polygon->cbind = PFGS_OFF;
    strcpy(name,"default");

    while(fscanf(imFile,"%s",buf) > 0)
    {
	if (strcmp(buf, LOAD_GEODE) == 0)
	{
	    fscanf(imFile, "%s", name);
	    pfdSelectBldrName(strdup(name));
	}
	else if (strcmp(buf, PUSH_STATE) == 0)
	{
	    pfdPushBldrState();
	    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"Pushed State");
	}
	else if (strcmp(buf, POP_STATE) == 0)
	{
	    pfdPopBldrState();
	    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"Popped State");
	}
	else if (strcmp(buf, STORE_STATE) == 0)
	{
	    char *stateNameBuf[512];
	    char *sName;
	    fscanf(imFile,"%s",stateNameBuf);
	    sName = strdup((const char *)stateNameBuf);
	    pfdSaveBldrState(sName);
	    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
		     "Saved State Under Name: %s",sName);
	}
	else if (strcmp(buf, LOAD_STATE) == 0)
	{
	    char *stateNameBuf[512];
	    char *sName;
	    fscanf(imFile,"%s",stateNameBuf);
	    sName = strdup((const char *)stateNameBuf);
	    pfdLoadBldrState(sName);
	    free(sName);
	    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
		     "Loaded State Named: %s",sName);
	}
	else if (strcmp(buf, DEFINE_DEFAULT) == 0)
	{
	    pfdCaptureDefaultBldrState();
	}
	else if (strcmp(buf, RESET_STATE) == 0)
	{
	    pfdResetBldrState();
	}
	else if (strcmp(buf, NEW_NODE) == 0)
	{
	    fscanf(imFile, "%s", nodeType);
	    if (strcmp(nodeType, LOD_NODE) == 0)
	    {
		float range,transition;
		int nLODs,i;
		pfLOD *LOD = pfNewLOD();

		fscanf(imFile,"%s",nodeName);
		pfNodeName(LOD,nodeName);

		fscanf(imFile,"%d",&nLODs);
		for(i=0;i<=nLODs;i++)
		{
		    fscanf(imFile,"%f",&range);
		    pfLODRange(LOD,i,range);
		}
		for(i=0;i<=nLODs;i++)
		{
		    fscanf(imFile,"%f",&transition);
		    pfLODTransition(LOD,i,transition);
		}
		pfAdd(nodeList, LOD);
		fscanf(imFile,"%s",endBuf);
	    }
	    else if (strcmp(nodeType, FONT) == 0)
	    {
		int fontType;
		pfFont *fnt = NULL;
		fscanf(imFile,"%s",nodeName);
		fscanf(imFile,"%s %d",childName,&fontType);
		fnt = pfdLoadFont_type1(childName,fontType);
		if (fnt != NULL)
		{
		    pfFontAttr(fnt,PFFONT_NAME,nodeName);
		    pfAdd(fontList, fnt);
		}
		fscanf(imFile,"%s",endBuf);
	    }
	    else if (strcmp(nodeType, TEXT) == 0)
	    {
		int nStrings,i;
		char tmpBuf[8192];
		pfFont *fnt = NULL;
		pfText *txt = pfNewText();
		fscanf(imFile,"%s",nodeName);
		pfNodeName(txt, nodeName);
		fscanf(imFile,"%s", tmpBuf);
		fnt = (pfFont*)FindFontInList(fontList, tmpBuf);
		fscanf(imFile,"%d",&nStrings);
		
		for(i=0;i<nStrings;i++)
		{
		    char c;
		    int j = 0;
		    int done = 0;

		    pfString *curStr = NULL;
/*		    fscanf(imFile,"%s",tmpBuf);*/
		    while(done < 2)
		    {
			c = fgetc(imFile);
			if (c == '|')
			    done++;
			else 
			    done = 0;
			tmpBuf[j++] = c;
		    }
		    tmpBuf[PF_MAX2(j-2,0)] = '\0';
		    curStr = pfNewString(pfGetSharedArena());
		    pfStringString(curStr, tmpBuf);
		    pfStringFont(curStr, fnt);
		    pfAddString(txt,curStr);
		}
		pfAdd(nodeList, txt);
		fscanf(imFile,"%s",endBuf);
	    }
	    else if (strcmp(nodeType, LODSTATE) == 0)
	    {
		pfVec2 rR,rS,tR,tS;
		pfLODState *ls = pfNewLODState();
		fscanf(imFile,"%s",nodeName);
		pfLODStateName(ls,nodeName);
		fscanf(imFile,"%f %f",&rR[0],&rR[1]);
		fscanf(imFile,"%f %f",&rS[0],&rS[1]);
		fscanf(imFile,"%f %f",&tR[0],&tR[1]);
		fscanf(imFile,"%f %f",&tS[0],&tS[1]);
		pfLODStateAttr(ls, PFLODSTATE_RANGE_RANGESCALE, rR[0]);
		pfLODStateAttr(ls, PFLODSTATE_RANGE_RANGEOFFSET, rR[1]);
		pfLODStateAttr(ls, PFLODSTATE_TRANSITION_RANGESCALE, rR[0]);
		pfLODStateAttr(ls, PFLODSTATE_TRANSITION_RANGEOFFSET, tR[1]);
		pfLODStateAttr(ls, PFLODSTATE_RANGE_STRESSSCALE, rS[0]);
		pfLODStateAttr(ls, PFLODSTATE_RANGE_STRESSOFFSET, rS[1]);
		pfLODStateAttr(ls, PFLODSTATE_TRANSITION_STRESSSCALE, rS[0]);
		pfLODStateAttr(ls, PFLODSTATE_TRANSITION_STRESSOFFSET, tS[1]);
		fscanf(imFile,"%s",endBuf);
	    }
	    else if (strcmp(nodeType, LIGHT_POINT_NODE) == 0)
	    {
		int lnum,i;
		float lsize;
		float lh,lp,lr;
		pfVec3 lpos;
		pfVec4 lclr;
		pfLightPoint *lpoint;

		pfNotify(PFNFY_INFO, PFNFY_PRINT, "Defining LP");
		fscanf(imFile,"%s",nodeName);
		fscanf(imFile,"%f",&lsize);
		fscanf(imFile,"%f %f %f", &lh, &lp, &lr);
		fscanf(imFile,"%d",&lnum);
		lpoint = pfNewLPoint(lnum);
		pfNodeName(lpoint, nodeName);
		pfLPointRot(lpoint,lh,lp,lr);
		pfLPointSize(lpoint,lsize);
		for(i=0;i<lnum;i++)
		{
		    fscanf(imFile,"%f %f %f", &lpos[0], &lpos[1], &lpos[2]);
		    fscanf(imFile,"%f %f %f %f", &lclr[0], &lclr[1], &lclr[2], &lclr[3]);
		    pfLPointPos(lpoint,i,lpos);
		    pfLPointColor(lpoint,i,lclr);
		}
		fscanf(imFile,"%s",endBuf);
		pfAdd(nodeList, lpoint);
	    }
	    else if (strcmp(nodeType, ROOT_NODE) == 0)
	    {
		root = pfNewGroup();

		fscanf(imFile,"%s",nodeName);
		pfNodeName(root, nodeName);

		pfAdd(nodeList, root);
		fscanf(imFile,"%s",endBuf);
	    }
	    else if (strcmp(nodeType, GROUP_NODE) == 0)
	    {
		pfGroup *grp = pfNewGroup();

		fscanf(imFile,"%s",nodeName);
		pfNodeName(grp, nodeName);

		pfAdd(nodeList, grp);
		fscanf(imFile,"%s",endBuf);
	    }
	    else if (strcmp(nodeType, SWITCH_NODE) == 0)
	    {
		pfSwitch *sw = pfNewSwitch();

		fscanf(imFile,"%s",nodeName);
		pfNodeName(sw, nodeName);

		pfAdd(nodeList, sw);
		fscanf(imFile,"%s",endBuf);
	    }
	    else if (strcmp(nodeType, GEODE_NODE) == 0)
	    {
		pfGeode *gd = pfNewGeode();

		fscanf(imFile,"%s",nodeName);
		pfNodeName(gd, nodeName);

		pfAdd(nodeList, gd);
		fscanf(imFile,"%s",endBuf);
	    }
	    else if (strcmp(nodeType, ASD_NODE) == 0)
	    {
		pfASD *asd;
		pfGeoState **gsList;
		int i,nGStates;

		fscanf(imFile,"%s",nodeName);
		fscanf(imFile,"%s",fname);
		asd = (pfASD*)pfdLoadFile(fname);
		pfNodeName(asd, nodeName);
		fscanf(imFile,"%d",&nGStates);
		gsList = (pfGeoState**)pfMalloc(nGStates*sizeof(pfGeoState*),
						pfGetSharedArena());
		for(i=0;i<nGStates;i++)
		{
		    pfGeoState *gs = pfNewGState(pfGetSharedArena());
		    sprintf(fname, "%s.%d", nodeName,i);
		    pfdLoadBldrState(fname);
		    pfCopy(gs, (void*)pfdGetBldrGState());
		    gsList[i] = gs;
   		}
		if(nGStates > 0)
		    pfASDGStates(asd,gsList,nGStates);
		pfAdd(nodeList, asd);
		fscanf(imFile,"%s",endBuf);
	    }		
	    else if (strcmp(nodeType, BILLBOARD_NODE) == 0)
	    {
		int rotmode, i;
		float axis[3], pos[3];

		pfBillboard *bb = pfNewBboard();

		fscanf(imFile,"%s",nodeName);
		pfNodeName(bb, nodeName);

		fscanf(imFile, "mode %d", &rotmode);
		pfBboardMode(bb, PFBB_ROT, rotmode);
		fscanf(imFile, "axis %f %f %f", &axis[0], &axis[1], &axis[2]);
		pfBboardAxis(bb, axis);
		while (fscanf(imFile, "pos %d %f %f %f",
			&i, &pos[0], &pos[1], &pos[2]) == 4)
		    pfBboardPos(bb, i, pos);

		pfAdd(nodeList, bb);
		fscanf(imFile,"%s",endBuf);
	    }
	    else if (strcmp(nodeType, SEQUENCE_NODE) == 0)
	    {
		int nFrames,i;
		float time;
		pfSequence *seq = pfNewSeq();

		fscanf(imFile,"%s",nodeName);
		pfNodeName(seq, nodeName);

		fscanf(imFile,"%d",&nFrames);
		for(i=0;i<nFrames;i++)
		{
		    fscanf(imFile,"%f",&time);
		    pfSeqTime(seq, i, time);
		}
		pfSeqDuration(seq,1.0f, -1.0f);
		pfSeqMode(seq,PFSEQ_START);
		pfAdd(nodeList, seq);
		fscanf(imFile,"%s",endBuf);
	    }
	    else if (strcmp(nodeType, DCS_NODE) == 0)
	    {
		pfMatrix mat;
		float tx,ty,tz,rx,ry,rz,sx,sy,sz;
		int i,j;
		pfDCS *DCS = pfNewDCS();

		pfMakeIdentMat(mat);
		fscanf(imFile,"%s",nodeName);
		pfNodeName(DCS, nodeName);
		fscanf(imFile,"%s",endBuf);
		while(strncmp(endBuf,"end",3) != 0)
		{
		    if (strncmp(endBuf,"translate",8)==0)
		    {
			fscanf(imFile,"%f %f %f",&tx,&ty,&tz);
			pfPostTransMat(mat,mat,tx,ty,tz);
		    }
		    else if (strncmp(endBuf,"rotate",6)==0)
		    {
			pfMatrix tmp;
			fscanf(imFile,"%f %f %f",&rx,&ry,&rz);
			pfMakeEulerMat(tmp,rx,ry,rz);
			pfPostMultMat(mat,tmp);
		    }
		    else if (strncmp(endBuf,"scale",5)==0)
		    {		   
			fscanf(imFile,"%f %f %f",&sx,&sy,&sz);
			pfPostScaleMat(mat,mat,sx,sy,sz);
		    }
		    else if (strncmp(endBuf,"matrix",5)==0)
		    {		   
			for(j=0;j<4;j++)
			  for(i=0;i<4;i++)
			    fscanf(imFile,"%f",mat[i][j]);
		    }
		    fscanf(imFile,"%s",endBuf);
		}
		pfDCSMat(DCS,mat);
		pfAdd(nodeList,DCS);
	    }
	    else if (strcmp(nodeType, SCS_NODE) == 0)
	    {
		pfMatrix mat;
		float tx,ty,tz,rx,ry,rz,sx,sy,sz;
		int i,j;
		pfSCS *SCS;

		pfMakeIdentMat(mat);
		fscanf(imFile,"%s",nodeName);
		fscanf(imFile,"%s",endBuf);
		while(strncmp(endBuf,"end",3) != 0)
		{
		    if (strncmp(endBuf,"translate",8)==0)
		    {
			fscanf(imFile,"%f %f %f",&tx,&ty,&tz);
			pfPostTransMat(mat,mat,tx,ty,tz);
		    }
		    else if (strncmp(endBuf,"rotate",6)==0)
		    {
			pfMatrix tmp;
			fscanf(imFile,"%f %f %f",&rx,&ry,&rz);
			pfMakeEulerMat(tmp,rx,ry,rz);
			pfPostMultMat(mat,tmp);
		    }
		    else if (strncmp(endBuf,"scale",5)==0)
		    {		   
			fscanf(imFile,"%f %f %f",&sx,&sy,&sz);
			pfPostScaleMat(mat,mat,sx,sy,sz);
		    }
		    else if (strncmp(endBuf,"matrix",5)==0)
		    {		   
			for(j=0;j<4;j++)
			  for(i=0;i<4;i++)
			    fscanf(imFile,"%f",&mat[i][j]);
		    }
		    fscanf(imFile,"%s",endBuf);
		}
		SCS = pfNewSCS(mat);
		pfNodeName(SCS, nodeName);
		pfAdd(nodeList,SCS);
	    }
	}
	else if (strcmp(buf, LOAD_FILE) == 0)
	{
	    /*
	     * Note that if the file is not .im format,
	     * the file name should contain the appropriate
	     * extension, e.g. call it "myscene.obj.im"
	     * to ensure that the proper converter gets loaded
	     * beforehand.
	     */
	    pfNode *node;
	    pfGroup *protector;
	    pfdBuilder *oldbuilder, *newbuilder;
		/* otherwise if it's a geode, we would kill it at the end */
	    fscanf(imFile, "%s %s", fname, nodeName);

	    /* push builder... */
	    newbuilder = pfdNewBldr();
	    oldbuilder = pfdGetCurBldr();
	    pfdSelectBldr(newbuilder);

	    node = pfdLoadFile(fname);

	    /* pop builder... */
	    pfdSelectBldr(oldbuilder);
	    pfdDelBldr(newbuilder);

	    protector = pfNewGroup();
	    pfAddChild(protector, node);
	    pfNodeName(protector, nodeName);
	    pfAdd(nodeList, protector);
	}
	else if (strcmp(buf, LOD_LODSTATE) == 0)
	{
	    pfLODState *ls;
	    pfLOD *lod;
	    fscanf(imFile,"%s %s",parentName,childName);
	    lod = (pfLOD *)FindInList(nodeList, parentName);
	    ls = pfFindLODState(childName);
	    if (lod && ls)
		pfLODLODState(lod,ls);
	    else if (!lod)
		pfNotify(PFNFY_WARN,PFNFY_PRINT,"No LOD Named: %s",parentName);
	    else 
		pfNotify(PFNFY_WARN,PFNFY_PRINT,"No LODState Named: %s",
			childName);
	}
	else if (strcmp(buf, LOD_LODSTATE_INDEX) == 0)
	{
	    pfLOD *lod;
	    long index;

	    fscanf(imFile,"%s %d",parentName,&index);
	    lod = (pfLOD *)FindInList(nodeList, parentName);
	    pfLODLODStateIndex(lod,index);
	}
	else if (strcmp(buf, BOUNDING_SPHERE) == 0)
	{
	    pfNode *node;
	    pfSphere bsph;
	    fscanf(imFile, "%s %f %f %f  %f",
		   nodeName,
		   &bsph.center[0],
		   &bsph.center[1],
		   &bsph.center[2],
		   &bsph.radius);
	    node = FindInList(nodeList, nodeName);
	    if (node != NULL)
	    {
		pfNodeBSphere(node, &bsph, PFBOUND_STATIC);
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		   "pfNodeBSphere(%s,center=(%.9g,%.9g,%.9g),radius=%.9g)",
		   nodeName,
		   bsph.center[0], bsph.center[1], bsph.center[2], bsph.radius);
	    }
	    else
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "Cannot set bounding sphere: No Node Named \"%s\"",
			 nodeName);
	}
	else if (strcmp(buf, ATTACH) == 0)
	{
	    pfGroup *parent;
	    pfNode *child;

	    fscanf(imFile,"%s %s",parentName,childName);
	    parent = (pfGroup *)FindInList(nodeList, parentName);
	    child = FindInList(nodeList, childName);
	    if (parent && child)
	    {
		pfAddChild(parent,child);
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "pfAddChild(%s,%s)",parentName,childName);
	    }
	    else if (!parent)
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		    "Cannot Attach: No Node Named: %s", parentName);
	    else if (!child)
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		    "Cannot Attach: No Node Named: %s", childName);
	}
	else if (strcmp(buf, SPHEREFRONT) == 0)
	{
	    pfNode *spherefront_node;
	    int size;
	    pfTexture *tex = (pfTexture*)pfdGetBldrStateAttr(PFSTATE_TEXTURE);
	    pfVec2 lonlat[2], st[2];
	    fscanf(imFile, "%s %f,%f,%f,%f %f,%f,%f,%f %d",
	        nodeName,
		&lonlat[0][PF_S], &lonlat[0][PF_T], &st[0][PF_S], &st[0][PF_T],
	        &lonlat[1][PF_S], &lonlat[1][PF_T], &st[1][PF_S], &st[1][PF_T],
	        &size);
	    spherefront_node = pfuNewSphereFrontOld(size, tex, lonlat, st, 1);
	    pfNodeName(spherefront_node, nodeName);
	    pfAdd(nodeList, spherefront_node);
	}
	else if (strncmp(buf, CLIPCENTER, 7) == 0)
	{
	    /*
	     * clipcenter clipcenter_nodeName childName refnodeName
	     * 	   creates the following:
	     *
	     *	       clipcenter_node 	--(user data)--> refnode
	     *		     |
	     *	           child
	     *
	     * clipcenter_node is a pfGroup with one child and:
	     *   -- a precull callback that calculates the point in refnode
	     *	    closest to the eye
	     *	 -- a predraw callback that sets the center of the clipmap
	     *	    to the point calculated in the precull callback.
	     * refnode should be a representation (possibly simplified,
	     * perhaps to a single quad or triangle) of child;
	     * it can be a reference to child itself.
	     */
	    pfNode *child, *refnode;
	    pfTexture *tex;
	    char clipcenter_nodeName[256];
	    char refnodeName[256];

	    fscanf(imFile,"%s %s %s",clipcenter_nodeName,childName,refnodeName);

	    refnode = FindInList(nodeList, refnodeName);
	    child = FindInList(nodeList, childName);

	    if (!refnode || !child)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		    "Cannot attach for clipmap centering: No Node Named: %s",
		    !refnode ? refnodeName : childName);
	    }
	    else if (!pfIsOfType(
		    tex = (pfTexture*)pfdGetBldrStateAttr(PFSTATE_TEXTURE),
		    pfGetClipTextureClassType()))
	    {
		/* do something reasonable to preserve the structure... */
		pfGroup *clipcenter_node = pfNewGroup();
		pfNodeName(clipcenter_node, clipcenter_nodeName);
		pfAdd(nodeList, clipcenter_node);
		pfAddChild(clipcenter_node, child);

		pfNotify(PFNFY_WARN, PFNFY_PRINT,
		    "Cannot attach for clipmap centering--");
		pfNotify(PFNFY_WARN, PFNFY_MORE,
		    "    texture %s is not a clip texture!",
		    pfGetTexName((pfTexture*)tex));
	    }
	    else
	    {
		pfuClipCenterNode *ccn;
		ccn = pfuNewClipCenterNode();
		pfuClipCenterNodeClipTexture(ccn, (pfClipTexture *)tex);
		pfNodeName(ccn, clipcenter_nodeName);
		pfuClipCenterNodeRefNode(ccn, refnode);
		pfAddChild(ccn, child);
		pfAdd(nodeList, ccn);

		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		    "Created clipcenter node %s(%s,%s)",
		    clipcenter_nodeName, childName, refnodeName);
	    }
	}
	else if (strcmp(buf, TEXGENCLIPCENTER) == 0)
	{
            /*
             * clipcenter clipcenter_nodeName childName refnodeName
             *     creates the following:
             *
             *         clipcenter_node  --(user data)--> refnode
             *               |
             *             child
             *
             * clipcenter_node is a pfGroup with one child and:
             *   -- a precull callback that calculates the point in refnode
             *      closest to the eye
             *   -- a predraw callback that sets the center of the clipmap
             *      to the point calculated in the precull callback.
             * refnode should be a representation (possibly simplified,
             * perhaps to a single quad or triangle) of child;
             * it can be a reference to child itself.
             */

	    pfNode *child;
            pfTexture *tex;
            char clipcenter_nodeName[256];
	    pfuTexGenClipCenterNode *ccn;
	    pfTexGen *tgen;

            fscanf(imFile,"%s %s",clipcenter_nodeName,childName);
            child = FindInList(nodeList, childName);
            if (!child)
            {
                pfNotify(PFNFY_WARN, PFNFY_PRINT,
                    "Cannot attach for clipmap centering: No childNode Named: %s",
                    childName);
            }
            else if (!pfIsOfType(
                    tex = (pfTexture*)pfdGetBldrStateAttr(PFSTATE_TEXTURE),
                    pfGetClipTextureClassType()))
            {
                /* do something reasonable to preserve the structure... */
                pfGroup *clipcenter_node = pfNewGroup();
                pfNodeName(clipcenter_node, clipcenter_nodeName);
                pfAdd(nodeList, clipcenter_node);
                pfAddChild(clipcenter_node, child);

                pfNotify(PFNFY_WARN, PFNFY_PRINT,
                    "Cannot attach for clipmap centering--");
                pfNotify(PFNFY_WARN, PFNFY_MORE,
                    "    texture %s is not a clip texture!",
                    pfGetTexName((pfTexture*)tex));
            }
	    else /* using immediate mode */
	    {
	     	ccn = pfuNewTexGenClipCenterNode();
	     	pfuClipCenterNodeClipTexture((pfuClipCenterNode *)ccn, (pfClipTexture *)tex);
	     	pfNodeName(ccn, clipcenter_nodeName);
		tgen = (pfTexGen *)pfdGetBldrStateAttr(PFSTATE_TEXGEN);
		pfuTexGenClipCenterNodeTexGen(ccn, tgen);
                pfAddChild(ccn, child);
                pfAdd(nodeList, ccn);
	    }
	}
	else if (strcmp(buf, BREAKUP) == 0)
	{
	    int		breakup, breakupSize, breakupStrips,breakupBranch;

	    fscanf(imFile, "%d %f %d %d", 
			&breakup, &breakupSize, &breakupStrips,&breakupBranch);
	    pfdBldrMode(PFDBLDR_BREAKUP, breakup);
	    pfdBldrMode(PFDBLDR_BREAKUP_SIZE, breakupSize);
	    pfdBldrMode(PFDBLDR_BREAKUP_BRANCH, breakupBranch);
	    pfdBldrMode(PFDBLDR_BREAKUP_STRIP_LENGTH, breakupStrips);
	}
	else if (strcmp(buf, SHARE) == 0)
	{
	    int share_mask;
	    fscanf(imFile, "%d ", &share_mask);
	    pfdBldrMode(PFDBLDR_SHARE_MASK,share_mask);
	}
	else if (strncmp(buf, TGEN_PARAMS, 8) == 0)
	{ /* add params to a pre-existing texgen */

	    int p;

	    if (params)
		pfDelete(params);
	    params = pfMalloc(8*sizeof(float),pfGetSharedArena());
	    for(p=0;p<8;p++)
		fscanf(imFile,"%f ",&(params[p]));
	    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"Set PX: %.2f %.2f %.2f %.2f",
		     params[0],params[1],params[2],params[3]);
	    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"Set PX: %.2f %.2f %.2f %.2f",
		     params[4],params[5],params[6],params[7]);
	    if (tgen)
	    {
		pfTGenPlane(tgen, PF_S, params[0], params[1], params[2], params[3]);
		pfTGenPlane(tgen, PF_T, params[4], params[5], params[6], params[7]);
	    }
	    /*pfdBldrStateAttr(txgn,params);*/
	}
	else if (strncmp(buf, TGEN_REFLECTIVE, 8) == 0)
	{
	    tgen = (pfTexGen *)pfdGetTemplateObject(pfGetTGenClassType());
	    pfTGenMode(tgen, PF_S, PFTG_SPHERE_MAP);
	    pfTGenMode(tgen, PF_T, PFTG_SPHERE_MAP);
	    pfdBldrStateAttr(PFSTATE_TEXGEN,tgen);
	    pfdBldrStateMode(PFSTATE_ENTEXGEN, PFTR_ON);
	}
	else if (strncmp(buf, TGEN_CONTOUR, 8) == 0)
	{
	    /*pfdBldrStateMode(txgn,PFD_TEXGEN_CONTOUR);*/
	    tgen = (pfTexGen *)pfdGetTemplateObject(pfGetTGenClassType());
	    pfTGenMode(tgen, PF_S, PFTG_EYE_LINEAR);
	    pfTGenMode(tgen, PF_T, PFTG_EYE_LINEAR);
	    if (params)
	    {
		pfTGenPlane(tgen, PF_S, params[0], params[1], params[2], params[3]);
		pfTGenPlane(tgen, PF_T, params[4], params[5], params[6], params[7]);
	    }
	    pfdBldrStateAttr(PFSTATE_TEXGEN,tgen);
	    pfdBldrStateMode(PFSTATE_ENTEXGEN, PFTR_ON);
	}
	else if (strncmp(buf, TGEN_LINEAR, 8) == 0)
	{
	    /*pfdBldrStateMode(txgn,PFD_TEXGEN_LINEAR);*/
	    tgen = (pfTexGen *)pfdGetTemplateObject(pfGetTGenClassType());
	    pfTGenMode(tgen, PF_S, PFTG_OBJECT_LINEAR);
	    pfTGenMode(tgen, PF_T, PFTG_OBJECT_LINEAR);
	    if (params)
	    {
		pfTGenPlane(tgen, PF_S, params[0], params[1], params[2], params[3]);
		pfTGenPlane(tgen, PF_T, params[4], params[5], params[6], params[7]);
	    }
	    pfdBldrStateAttr(PFSTATE_TEXGEN,tgen);
	}
	else if (strncmp(buf, TGEN_OFF, 6) == 0)
	{
	    pfdBldrStateMode(PFSTATE_ENTEXGEN, PFTR_OFF);
	    pfdBldrStateMode(txgn,PFD_TEXGEN_OFF);
	}
	else if (strcmp(buf, NEW_TEXGEN) == 0)
	{
	    int i;
	    pfMatrix mat;
	    pfTexGen *tg = (pfTexGen *)pfdGetTemplateObject(pfGetTGenClassType());
	    for(i=0;i<16;i++)
	    	fscanf(imFile,"%f", &mat[(int)i/4][i%4]);
	    pfTGenPlane(tg, PF_S, mat[0][0], mat[1][0], mat[2][0], mat[3][0]);
	    pfTGenPlane(tg, PF_T, mat[0][1], mat[1][1], mat[2][1], mat[3][1]);
	    pfTGenPlane(tg, PF_R, mat[0][2], mat[1][2], mat[2][2], mat[3][2]);
	    pfTGenPlane(tg, PF_Q, mat[0][3], mat[1][3], mat[2][3], mat[3][3]);
	    pfTGenMode(tg, PF_S, PFTG_OBJECT_LINEAR);
	    pfTGenMode(tg, PF_T, PFTG_OBJECT_LINEAR);
	    pfTGenMode(tg, PF_R, PFTG_OBJECT_LINEAR);
	    pfTGenMode(tg, PF_Q, PFTG_OBJECT_LINEAR);
	    pfdBldrStateAttr(PFSTATE_TEXGEN,tg);
	    pfdBldrStateMode(PFSTATE_ENTEXGEN, PFTR_ON);
	    for(i=0;i<16;i++)
	    {
		if (i%4 == 0)
			printf("\n");
	    	printf("%f ", mat[(int)(i/4)][i%4]);
	    }
		printf("\n");
	}
	else if (strcmp(buf, NEW_TEXGEN_OFF)==0)
	{
	    pfdBldrStateMode(PFSTATE_ENTEXGEN, PFTR_OFF);
	}
	else if (strncmp(buf, TEXTURE, 3) == 0)
	{
	    pfTexture *tex = (pfTexture *)
		pfdGetTemplateObject(pfGetTexClassType());
	    fscanf(imFile, "%s", fname);
	    pfTexName(tex,fname);
	    pfdBldrStateAttr(PFSTATE_TEXTURE,tex);
	    pfdBldrStateMode(PFSTATE_ENTEXTURE, PFTR_ON);
	}
	else if (strncmp(buf, CLIPTEXTURE, 7) == 0)
	{
	    pfClipTexture *cliptex;
	    fscanf(imFile, "%s", fname);
	    cliptex = pfdLoadClipTexture(fname);
	    if (cliptex != NULL)
	    {
		pfTexName((pfTexture *)cliptex,fname); /* XXX is it already? */
		pfdBldrMode(PFDBLDR_PASS_REFERENCES, 1);
		pfdBldrStateAttr(PFSTATE_TEXTURE,cliptex);
		pfdBldrMode(PFDBLDR_PASS_REFERENCES, 0);
		pfdBldrStateMode(PFSTATE_ENTEXTURE, PFTR_ON);
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "im: successfully loaded clip texture from %s:",
			 fname);
		if (pfGetNotifyLevel() >= PFNFY_DEBUG)
		{
		    pfPrint(cliptex, PFTRAV_SELF | PFTRAV_DESCEND,
			    PFPRINT_VB_DEBUG, stderr); /* XXX */
		}
	    }
	    else
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "Can't load clip texture file %s\n", fname);
	}
	else if (strncmp(buf, NO_TEXTURE, 5) == 0)
	{
	    pfTexture *tex = NULL;
	    pfdBldrStateAttr(PFSTATE_TEXTURE,tex);
	    pfdBldrStateMode(PFSTATE_ENTEXTURE, PFTR_OFF);
	}
	else if (strncmp(buf, COLORS, 6) == 0)
	{
	    int i;
	    polygon->cbind = PFGS_PER_VERTEX;
	    for (i=0;i<polygon->numVerts;i++)
		fscanf(imFile, "%f %f %f %f", 
			&polygon->colors[i][0],
			&polygon->colors[i][1],
			&polygon->colors[i][2],
			&polygon->colors[i][3]);
	}
	else if (strncmp(buf, COLOR, 5) == 0)
	{
	    polygon->cbind = PFGS_OVERALL;
	    fscanf(imFile, "%f %f %f %f", 
			&polygon->colors[0][0],
			&polygon->colors[0][1],
			&polygon->colors[0][2],
			&polygon->colors[0][3]);
	}
	else if (strncmp(buf,NO_COLOR,5) == 0)
	{
	    polygon->cbind = PFGS_OFF;
	}
	else if (strncmp(buf, POLY, 4) == 0)
	{
	    fscanf(imFile, "%d", &polygon->numVerts);
	    primType = PFGS_POLYS;
	}
	else if (strncmp(buf, LINE, 4) == 0)
	{
	    fscanf(imFile, "%d", &polygon->numVerts);
	    primType = PFGS_LINES;
	}
	else if (strncmp(buf, MYPOINT, 5) == 0)
	{
	    fscanf(imFile, "%d", &polygon->numVerts);
	    primType = PFGS_POINTS;
	}
	else if (strcmp(buf, VERTEX) == 0)
	{
	    int i;
	    for (i=0;i<polygon->numVerts;i++)
		fscanf(imFile, "%f %f %f", 
			&polygon->coords[i][0],
 			&polygon->coords[i][1], 
 			&polygon->coords[i][2]);
	}
	else if (strcmp(buf, TEX_COORD) == 0)
	{
	    int i;
	    
	    for (i=0;i<polygon->numVerts;i++)
		fscanf(imFile, "%f %f", 
		    &polygon->texCoords[0][i][0], 
		    &polygon->texCoords[0][i][1]); 
	    polygon->tbind[0] = PFGS_PER_VERTEX;
	    for (t = 1 ; t < PF_MAX_TEXTURES ; t ++)
		polygon->tbind[t] = PFGS_OFF;
	}
	else if (strcmp(buf, NORMAL_COORD) == 0)
	{
	    int i;
	    for (i=0;i<polygon->numVerts;i++)
		fscanf(imFile, "%f %f %f", 
			&polygon->norms[i][0],
 			&polygon->norms[i][1], 
 			&polygon->norms[i][2]);
	    polygon->nbind = PFGS_PER_VERTEX;
	}
	else if (strcmp(buf, "end") == 0)
	{
	    polygon->primtype = primType;
	    pfdAddBldrGeom(polygon,1);
	    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
		polygon->tbind[t] = PFGS_OFF;
	    count += polygon->numVerts - 2;
	}
	else if (buf[0] == '#')
	{
	    /*
	     * discard rest of line as comment
	     * (note this only works in the global scope)
	     */
	    int c;
	    while ((c = getc(imFile)) != EOF && c != '\n')
		;
	}
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "Bogus Token: %s in file %s", 
			buf, fileName);
	}
    }
    {
	int i,j,n;
	n = pfGetNum(nodeList);
	for(i=0;i<n;i++)
	{
	    pfNode *node = pfGet(nodeList,i);
	    if (pfIsOfType(node, pfGetGeodeClassType()))
	    {
		char buf[512];
		pfNode *newNode;
		int nParents = pfGetNumParents(node);

		strncpy(buf,pfGetNodeName(node),512);
		newNode = pfdBuildNode(buf);

		for(j=0;j<nParents;j++)
		{
		    pfGroup *parent = pfGetParent(node,0); /* sic */
		    pfReplaceChild(parent,node,newNode);
		}
		pfDelete(node);
	    }
	}
    }
    fclose(imFile);
    pfdDelGeom(polygon);

    pfDelete(nodeList);
    pfDelete(fontList);
    pfDelete(clipList);
    pfDelete(clipNameList);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_im: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input polygons:     %8ld", count);

    return (pfNode *)root;
}
