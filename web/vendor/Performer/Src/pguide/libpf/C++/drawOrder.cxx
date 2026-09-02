
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfEarthSky.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pf/pfLightSource.h>

pfScene *scene;

/*===========================================================================*/
pfGeoState *
make_gstate (pfMaterial *material)
/*===========================================================================*/
{
    pfGeoState  *gs;

    gs = new pfGeoState;

    gs->makeBasic();

    gs->setMode(PFSTATE_ENTEXTURE,PF_OFF);
    gs->setMode(PFSTATE_ENLIGHTING,PF_ON);
    // gs->setMode(PFSTATE_CULLFACE,PFCF_OFF );
    gs->setAttr(PFSTATE_FRONTMTL, material);
    gs->setAttr(PFSTATE_BACKMTL, material);

    return gs;
}

/*===========================================================================*/
pfMaterial *
make_material (float r, float g, float b)
/*===========================================================================*/
{
    pfMaterial  *material;

    material = new pfMaterial;

    material -> setColor(PFMTL_AMBIENT,  r, g, b);
    material -> setColor(PFMTL_DIFFUSE,  r, g, b);
    material -> setColor(PFMTL_SPECULAR, r, g, b);
    material -> setShininess(1.0f);

    return material;
}


/*===========================================================================*/
void
drawFunc(pfChannel *chan, void *data)
/*===========================================================================*/
{
    chan->clear();
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    pfDraw();
}

/*===========================================================================*/
pfNode *
createObject(
  pfVec3 pos, pfVec2 size, pfGeoState *gstate)
/*===========================================================================*/
{

  // Set up geoset
  pfVec3 *coords = (pfVec3*) new(4*sizeof(pfVec3)) pfMemory;

  coords[0].set(pos[PF_X]-size[PF_X], pos[PF_Y], pos[PF_Z]-size[PF_Y]);
  coords[1].set(pos[PF_X]+size[PF_X], pos[PF_Y], pos[PF_Z]-size[PF_Y]);
  coords[2].set(pos[PF_X]+size[PF_X], pos[PF_Y], pos[PF_Z]+size[PF_Y]);
  coords[3].set(pos[PF_X]-size[PF_X], pos[PF_Y], pos[PF_Z]+size[PF_Y]);
  
  pfGeoSet *gset = new pfGeoSet;
  gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, 0);
  gset->setPrimType(PFGS_QUADS);
  gset->setNumPrims(1);

  // Attach gstate
  gset->setGState(gstate);

    // add to scene graph
  pfGeode *geode = new pfGeode;
  geode->addGSet(gset);

  pfGroup *group = new pfGroup;
  group -> addChild (geode);
 
  return group;
}

/*===========================================================================*/
int
main (int argc, char *argv[])
/*===========================================================================*/
{
    pfGeoState	*red_gstate, *green_gstate;
    
    // Initialize Performer
    pfInit();	
    
    // Use default multiprocessing mode based on number of
    // processors.
    //
#if 1
    pfMultiprocess( PFMP_APP_CULL_DRAW );
#else
    pfMultiprocess( PFMP_APP_CULLDRAW );
#endif
    
    // Configure multiprocessing mode and start parallel
    // processes.
    //

    pfConfig();			

    // Configure and open GL window
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setName("IRIS Performer");
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setOriginSize(0, 0, 600, 600);
    pw->open();
    pfFrame();

    // Create Scene
    scene = new pfScene;

    // Create and configure a pfChannel.
    pfChannel *chan = new pfChannel(p);
    chan->setScene(scene);
    chan->setNearFar(10.0f, 10000.0);
    chan->setFOV(45.0f, -1.0f);

    // set draw callback to allow us to disable depth test.
    chan->setTravFunc(PFTRAV_DRAW, drawFunc);

    chan->setBinSort(PFSORT_OPAQUE_BIN, PFSORT_BY_STATE, NULL);
    chan->setBinSort(PFSORT_TRANSP_BIN, PFSORT_BY_STATE, NULL);

    // ============================================
    // Make gstates.
    // ============================================

    red_gstate = make_gstate(make_material(1.0, 0.0, 0.0));
    green_gstate = make_gstate(make_material(0.0, 1.0, 0.0));

    // ============================================
    // Red object
    // ============================================

    pfVec3 pos(0.0f,200.0f,0.0f);
    pfVec2 quad_size (40.0f,10.0f);

    pfNode *object_0 = createObject(pos, quad_size, red_gstate);

    // ============================================
    // Red object
    // ============================================

    pos.set(0.0f,200.0f,0.0f);
    quad_size.set(30.0f,20.0f);

    pfNode *object_1 = createObject(pos, quad_size, red_gstate);

    // ============================================
    // Green object
    // ============================================

    pos.set(0.0f,200.0f,0.0f);
    quad_size.set(20.0f,30.0f);

    pfNode *object_2 = createObject
			(pos, quad_size, green_gstate);

    // ============================================
    // Green object
    // ============================================

    pos.set(0.0f,200.0f,0.0f);
    quad_size.set(10.0f,40.0f);

    pfNode *object_3 = createObject
			(pos, quad_size, green_gstate);

    // ============================================
    // Build scene graph hierarchy
    // ============================================

    pfGroup 	*group_0 = new pfGroup;
    pfGroup 	*group_1 = new pfGroup;

    scene -> addChild (group_0);
    scene -> addChild (group_1);

    group_0 -> addChild (object_0); // Red
    group_0 -> addChild (object_2); // Green

    group_1 -> addChild (object_1); // Red
    group_1 -> addChild (object_3); // Green


    // ============================================
    // Add a light source
    // ============================================

    pfLightSource *light = new pfLightSource;
    light -> setColor(PFLT_AMBIENT, 1.0f, 1.0f, 1.0f);
    light -> setColor(PFLT_DIFFUSE, 1.0f, 1.0f, 1.0f);
    scene -> addChild (light);
 
    // ============================================
    // Loop forever.
    // ============================================
    
    while (1)
    {
	// Go to sleep until next frame time.
	pfSync();		
	
	// Initiate cull/draw for this frame.
	pfFrame();
    }
    
    // Terminate parallel processes and exit.
    pfExit();
    
    return 0;
}
