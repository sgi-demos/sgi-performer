/*
 * Copyright 1997 Silicon Graphics, Inc.
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
 * file: dmvtO2.c
 * ----------------
 * 
 *  Video Texturing using the fast path (dmbuffers) on O2
 *
 *  On O2 systems this path for defining textures can be done at
 *  hardware rendering or pixel transfer DMA rates, when the following
 *  conditions are met:
 *  - The DMbuffer, pbuffer and texture object all match in terms of
 *    width, height and depth of RGBA components.
 *  - The texture image is 64x64 or larger.
 *  - The glCopyTexSubImageEXT command is used to copy the entire
 *    image from dmpbuffer to texture.
 *
 *  This example takes video input from the O2 camera and applies it s a texture
 *  in a Performer scene
 *
 *  cc -n32 dmvtO2.c -o dmvtO2 -lpf_ogl -lGL -lX11 -lvl -ldmedia -lfpe
 *
 * Runtime Controls:
 * -----------------
 *	s - toggle spinning of geometry on/off
 *	ESC - exit
 *
 */

#include <X11/keysym.h>
#include <dmedia/vl.h>
#include <dmedia/dm_buffer.h>
#include <dmedia/dm_image.h>
#include <Performer/pf.h>

/* Global Variables */
GLXContext 	dmpc;
GLXPbufferSGIX 	dmPbuf;
DMparams 	*videoFormat;
DMbufferpool    pool;
Display 	*dpy;
Window 		xwin;
GLfloat 	spin = 0;
GLint 		spinning = 1;
int 		texWidth, texHeight;
int 		fieldWidth, fieldHeight;
int 		frameWidth, frameHeight;

pfMatrix idmat[4][4] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

pfVec2          texcoords[]={   {0.0f, 0.0f},
                                {1.0f, 0.0f},
                                {1.0f, 1.0f},
                                {0.0f, 1.0f} };

pfVec3          coords[] ={     {-1.0f,  -1.0f,  0.0f },
                                { 1.0f,  -1.0f,  0.0f },
                                { 1.0f,  1.0f,   0.0f },
                                {-1.0f,  1.0f,   0.0f } };

pfVec4		colors[] ={ {1.0f, 1.0f, 1.0f, 1.0f} };

static pfWindow	*Win=NULL;
pfTexture	*Tex=NULL;
pfDispList 	*SceneDL=NULL;
VLServer	svr;
VLPath		path;
VLNode		src, drn;

static void       DrawScene(void);
static pfDispList *MakeScene(pfGeoSet *geom, void *arena);

void ExitVtO2(void)
{
    glXWaitGL();
    vlEndTransfer(svr, path); /* End the data transfer */
    vlDestroyPath(svr, path); /* Destroy the path, free the memory it used */
    vlCloseVideo(svr); 	      /* Disconnect from the daemon */
    XCloseDisplay(dpy);
    pfExit();
    exit(EXIT_SUCCESS);
}

void CreateWindow(void *arena)
{
    pfFrustum 	*frust;

    GLint attribRGB[] = {PFFB_DOUBLEBUFFER, PFFB_RGBA, None};

    /* Initialize window and viewing frustum */
    Win = pfNewWin(arena);
    pfWinOriginSize(Win, 0, 0, frameWidth, frameHeight);
    pfWinName(Win, "dmvtO2");
    pfWinType(Win, PFWIN_TYPE_X);
    pfOpenWin(Win);
    dpy = pfGetCurWSConnection();
    if (!dpy) {
	pfNotify(PFNFY_FATAL,PFNFY_PRINT, "Can't connect to display\n");
    }
    xwin = pfGetWinWSWindow(Win);
    XSelectInput(dpy, xwin,  KeyPressMask);
    XMapWindow(dpy, xwin);
    XSync(dpy,FALSE);

    frust = pfNewFrust(NULL);
    pfFrustNearFar(frust, frameHeight, 3.0*frameHeight);    
    pfMakePerspFrust(frust, -0.25*frameWidth, 0.25*frameWidth, 
                            -0.25*frameHeight, 0.25*frameHeight);
    pfApplyFrust(frust);
    pfDelete(frust);
    pfLoadMatrix(idmat);
    pfTranslate (0.0f, 0.0f, -2.0*frameHeight);
    pfChooseWinFBConfig(Win, attribRGB);
}

void CreatePbuffer(void)
{
    int i, match, items, a[5];
    GLXFBConfigSGIX *fbConfig;
    int pbAttribs[] = {GLX_DIGITAL_MEDIA_PBUFFER_SGIX, GL_TRUE, None};
    int configAttribs[] = {
        GLX_RENDER_TYPE_SGIX, GLX_RGBA_BIT_SGIX,
	GLX_DRAWABLE_TYPE_SGIX, GLX_PBUFFER_BIT_SGIX,
	None 
    };
	    
    if(!(fbConfig = glXChooseFBConfigSGIX(dpy, DefaultScreen(dpy), configAttribs, &items))){
	pfNotify(PFNFY_FATAL,PFNFY_PRINT, "can't find suitable FBConfig\n");
    }
    
    /* search the list of FBconfigs and find one with RGBA8 and no DEPTH
     * to try and minimise the resources allocated for the pbuffer  
     */
    for(i = 0; i < items; i++){
	glXGetFBConfigAttribSGIX(dpy, fbConfig[i], GLX_RED_SIZE,   &a[0]);
	glXGetFBConfigAttribSGIX(dpy, fbConfig[i], GLX_GREEN_SIZE, &a[1]);
	glXGetFBConfigAttribSGIX(dpy, fbConfig[i], GLX_BLUE_SIZE,  &a[2]);
	glXGetFBConfigAttribSGIX(dpy, fbConfig[i], GLX_ALPHA_SIZE, &a[3]);
	glXGetFBConfigAttribSGIX(dpy, fbConfig[i], GLX_DEPTH_SIZE, &a[4]);
	if(a[0] == 8 && a[1] == 8 && a[2] == 8 && a[3] == 8 && a[4] == 0){ 
            match = i; break;
        }
     }
    
    if(!(dmpc = glXCreateContextWithConfigSGIX(dpy,
	    fbConfig[match], GLX_RGBA_TYPE_SGIX, NULL, GL_TRUE))){
	pfNotify(PFNFY_FATAL,PFNFY_PRINT, "can't create context\n");
    }
    
    if(!(dmPbuf = glXCreateGLXPbufferSGIX(dpy, fbConfig[match], 
		    fieldWidth, fieldHeight, pbAttribs))){
        pfNotify(PFNFY_FATAL,PFNFY_PRINT, "can't create GLXPbuffer\n");
    }
}


void InitTexture(void *arena)
{
    pfEnable(PFEN_TEXTURE);
    pfApplyTEnv(pfNewTEnv(arena));

    Tex = pfNewTex(arena);
    pfTexName(Tex, "Tex");
    pfTexImage(Tex, NULL, 4, texWidth, texHeight, 0);

    pfTexLoadSize(Tex, fieldWidth, fieldHeight);
    pfTexFilter(Tex, PFTEX_MINFILTER, PFTEX_LINEAR);
    pfTexFilter(Tex, PFTEX_MAGFILTER, PFTEX_LINEAR);
    pfTexFormat(Tex, PFTEX_INTERNAL_FORMAT, PFTEX_RGBA_8);
    pfTexLoadMode(Tex, PFTEX_LOAD_SOURCE, PFTEX_SOURCE_FRAMEBUFFER);
    pfTexLoadMode(Tex, PFTEX_LOAD_BASE, PFTEX_BASE_AUTO_SUBLOAD); 
    pfTexRepeat(Tex, PFTEX_WRAP_S, PFTEX_CLAMP);
    pfTexRepeat(Tex, PFTEX_WRAP_T, PFTEX_CLAMP);

    pfApplyTex(Tex);
}

int PowerOf2(int value)
{
    int x = 1;
    while(x <= value) x <<= 1;
    return(x);
}

void SetVLPathControls(void)
{
    VLControlValue val;
    
    val.intVal = VL_PACKING_ABGR_8;
    vlSetControl(svr, path, drn, VL_PACKING, &val);
    
    /* Set control for field rendering.  This conserves on the size of the
     * defined texture and improves performance.
     */
    val.intVal = VL_CAPTURE_NONINTERLEAVED;
    vlSetControl(svr, path, drn, VL_CAP_TYPE, &val);
    
    /* Query memory node for field size*/
    vlGetControl(svr, path, drn, VL_SIZE, &val);
    fieldWidth = val.xyVal.x; fieldHeight = val.xyVal.y;
    
    /* Query video node for frame size*/
    vlGetControl(svr, path, src, VL_SIZE, &val);
    frameWidth = val.xyVal.x; frameHeight = val.xyVal.y;
    texWidth = PowerOf2(fieldWidth); texHeight = PowerOf2(fieldWidth);
    
    val.intVal = VL_FORMAT_RGB;
    vlSetControl(svr, path, drn, VL_FORMAT, &val);
    val.intVal = VL_LAYOUT_GRAPHICS;
    vlSetControl(svr, path, drn, VL_LAYOUT, &val);
}

static void DrawScene(void) 
{
    static pfVec4 clr = {0.1f, 0.0f, 0.4f, 1.0f};
    pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
  
    pfPushMatrix(); 
    pfRotate(PF_X, -spin);
    pfDrawDList(SceneDL);
    pfPopMatrix();
    pfSwapWinBuffers(Win);  

    if (spinning) spin += 0.5;
    if (spin > 360.0) spin -= 360.0;
}

static pfDispList *MakeScene(pfGeoSet *geom, void *arena)
{
    pfDispList *dl;
    pfGeoState *gstate;
    float maxS = fieldWidth / (float) texWidth;
    float maxT = fieldHeight / (float) texHeight;
  
    dl = pfNewDList(PFDL_FLAT, 0, arena);
    
    coords[0][0] = -frameWidth / 2.0;
    coords[0][1] = -frameHeight / 2.0;
    coords[1][0] = frameWidth / 2.0;
    coords[1][1] = -frameHeight / 2.0;
    coords[2][0] = frameWidth / 2.0; 
    coords[2][1] = frameHeight / 2.0;
    coords[3][0] = -frameWidth / 2.0;
    coords[3][1] = frameHeight / 2.0;

    texcoords[1][0] = maxS;
    texcoords[2][0] = maxS;
    texcoords[2][1] = maxT;
    texcoords[3][1] = maxT;

    pfGSetAttr(geom, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(geom, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, NULL);
    pfGSetAttr(geom, PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
    pfGSetPrimType(geom, PFGS_QUADS);
    pfGSetNumPrims(geom, 1);
    
    gstate = pfNewGState (arena);
    pfGSetGState (geom, gstate);
    
    pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
    pfGStateAttr (gstate, PFSTATE_TEXENV, pfNewTEnv (arena));
    pfGStateAttr (gstate, PFSTATE_TEXTURE, Tex);
    
    pfOpenDList(dl);
    pfDrawGSet(geom);
    pfCloseDList();
    
    return dl;
}

void processVideoEvents(VLEvent vle)
{
    switch(vle.reason) {
      case VLTransferError:
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"VLTransferError....\n");
	break;
      case VLSequenceLost:
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"VLSequenceLost....\n");
	break;
      case VLTransferComplete:
	break;
      case VLTransferFailed:
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"VLTransfer FAILED ....\n");
	break;
      case VLStreamPreempted:
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"VLStreamPreempted ....\n");
	ExitVtO2();
	exit(EXIT_SUCCESS);
	/*NOTREACHED*/
      default:
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"didn't handle that event\n");
      break;
    }
}

void redraw(void) 
{
    DMbuffer field;
    VLEvent  vle;

    /* get the next VLEvent */
    if(vlEventRecv(svr, path, &vle) < 0){
	sginap(1);
	pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"no vl events\n");
	return;
    }

    processVideoEvents(vle); 
    vlEventToDMBuffer(&vle, &field);
    
    if(!(glXAssociateDMPbufferSGIX(dpy, dmPbuf, videoFormat, field))){
	pfNotify(PFNFY_FATAL,PFNFY_PRINT, "can't associate DMPbuffer\n");
    }

    /*
     *  Make the dmpbuffer current as a readable, Performer will
     *  copy the texture from it to the current pfTexture because of:
     *  pfTexLoadMode(Tex, PFTEX_LOAD_BASE, PFTEX_BASE_AUTO_SUBLOAD); 
     *  When the size and format of the pbuffer and the texture
     *  are compatible, the O2 implementation allows this copy
     *  to be done by reference.
     */
    if(!(glXMakeCurrentReadSGI(dpy, pfGetWinWSDrawable(Win), dmPbuf, pfGetWinGLCxt(Win)))){
	pfNotify(PFNFY_FATAL,PFNFY_PRINT, "can't make current read\n");
    }

    DrawScene();

    /* Tell VL that we are done using this dmbuffer so it can use it for
     * future frames.  This is not a release of memory it is a release of
     * a lock.
     */
    dmBufferFree(field);
}

void setDMparams(void)
{
    DMparams *poolParams;
    int glBufCt, videoBufCt;

    /* Create image params to correspond to video size*/
    dmParamsCreate(&videoFormat);
    dmSetImageDefaults(videoFormat, fieldWidth, fieldHeight, DM_IMAGE_PACKING_RGBA);
    dmParamsSetEnum(videoFormat, DM_IMAGE_LAYOUT, DM_IMAGE_LAYOUT_GRAPHICS);
    
    /* Create and initialize a dmbuffer pool for use by VL and GL.
     * This is the logic that makes this code example fast path video texturing
     * for O2 ! 
     */
    dmParamsCreate(&poolParams);
    dmBufferSetPoolDefaults(poolParams, 0, 0, DM_FALSE, DM_FALSE);
    
    /* VL and GL are clients of the dmbuffer.  A call to vlDMPoolGetParams
     * or dmBufferGetGLPoolParams *sets* the pools buffer count to
     *	    MAX(previous_ct, required_buffers)
     *
     * where previous_ct is the buffer count of the pool prior to calling
     * the function and required_buffers is the number of buffers required
     * by the client; VL in the vlDMPoolGetParams case and GL in the 
     * dmBufferGetGLPoolParams case.
     *
     * When multiple clients access the pool the buffer count needs to
     * be the sum of the required buffers of all the clients.
     */
    
    /* Get VL pool constraints */
    vlDMPoolGetParams(svr, path, drn, poolParams);
    videoBufCt = dmParamsGetInt(poolParams, DM_BUFFER_COUNT);
    
    /* Get GL pool constraints */
    dmParamsSetInt(poolParams, DM_BUFFER_COUNT, 0);
    dmBufferGetGLPoolParams(videoFormat, poolParams);
    glBufCt = dmParamsGetInt(poolParams, DM_BUFFER_COUNT);
    
    /* Add the buffer counts of GL and VL and set the pool accordingly.
     * Add one buffer for our application (yet another client of the dmbuffer).
     */
    dmParamsSetInt(poolParams, DM_BUFFER_COUNT, videoBufCt + glBufCt + 1); 
    
    dmBufferCreatePool(poolParams, &pool);
    dmParamsDestroy(poolParams);
}

void main(void)
{
    DMbuffer dmbuf;
    void *arena=NULL;
    pfGeoSet *screenGeom=NULL;
    

    /* Setup video path */
    svr = vlOpenVideo(""); 
    src = vlGetNode(svr, VL_SRC, VL_VIDEO, VL_ANY); 
    drn = vlGetNode(svr, VL_DRN, VL_MEM, VL_ANY); 
    path = vlCreatePath(svr, VL_ANY, src, drn); 
    
    /* Set up the hardware for and define the usage of the path */
    vlSetupPaths(svr, (VLPathList)&(path), 1, VL_SHARE, VL_SHARE);
    SetVLPathControls();

    /* Initialize Performer */    
    pfInit();
    
    arena = pfGetSharedArena();
    pfMultiprocess(PFMP_DEFAULT);
    pfConfig();	
    
    CreateWindow(arena);
    CreatePbuffer();
    
    setDMparams(); 

    /* Allocate a dmbuffer and associate with the pbuffer in order to initialize
     * the OpenGL state of the pbuffer
     */
    dmBufferAllocate(pool, &dmbuf);

    glXAssociateDMPbufferSGIX(dpy, dmPbuf, videoFormat, dmbuf);

    /* Initialize DMpbuffer */
    glXMakeCurrent(dpy, dmPbuf, dmpc);
    glViewport(0, 0, frameWidth, frameHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, frameWidth, 0, frameHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT);
    glRasterPos2i(0, 0);
    dmBufferFree(dmbuf);
    glXMakeCurrent(dpy,  pfGetWinWSDrawable(Win), pfGetWinGLCxt(Win));
    
    /* Generate a texture, bind and initialize */
    InitTexture(arena);

    screenGeom = pfNewGSet(arena);
    /* create scene */
    SceneDL = MakeScene(screenGeom, arena);

    vlDMPoolRegister(svr, path, drn, pool); 	/* Register dmbuffer with VL*/
    vlBeginTransfer(svr, path, 0, NULL);	/*Begin the data transfer */ 
    
    while(1){
        if (XPending(dpy)) {
            XEvent event;
            KeySym ks;

            XNextEvent(dpy, &event);
            switch(event.type) {
              case KeyPress:
                XLookupString(&event.xkey, NULL, 0, &ks, NULL);
		switch(ks) {
                  case XK_Escape :
                    ExitVtO2();
		  case XK_s:
		    spinning = !spinning;
		    break;
	         }
	         break;
             default:
                 break;
            }
        }
	redraw();
    }
}

