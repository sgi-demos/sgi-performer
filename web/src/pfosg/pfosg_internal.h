/* pfosg_internal.h - shared state between pfosg.cpp (core: window, viewer,
 * channel, scene graph) and pfosg_perfly.cpp (input, xformer, stubs). */
#ifndef PFOSG_INTERNAL_H
#define PFOSG_INTERNAL_H

#include <osg/Geometry>
#include <osg/Group>
#include <osg/Texture2D>
#include <osgViewer/Viewer>

#include <SDL.h>

#include <string>
#include <vector>

struct PfOsgESky;

/* one pfGeoSet: raw attribute pointers as the app supplied them, compiled
 * lazily into the osg::Geometry by compileGSet (pfosg.cpp) */
struct PfOsgGSet {
    osg::ref_ptr<osg::Geometry> geom;
    int ptype = 3;                  /* PFGS_* primitive type; 3 = PFGS_TRIS */
    int nprims = 0;
    int* lengths = nullptr;
    struct Attr {
        int binding = 0;            /* PFGS_OFF */
        const float* data = nullptr;
        const ushort* ilist = nullptr;
    } attr[4];
    /* pfGSetDrawMode / draw attributes (pfpfb.c reads these back) */
    bool flatShade = false;
    bool wireframe = false;
    bool compileGL = false;
    float lineWidth = 1.0f;
    float pntSize = 1.0f;
    unsigned isectMask = 0xffffffff;
    bool dirty = true;
};

/* one pfLPointState: just the sizing, captured for the light-point
 * rendering path (pfosg.cpp compileGSet / pfGStateAttr); the rest of the
 * light-point machinery (direction lobes, fog punch-through, callig) is
 * accepted and ignored */
struct PfOsgLPState {
    float sizeActual = 0.25f;      /* world-space diameter */
    float sizeMinPixel = 1.0f;
    float sizeMaxPixel = 64.0f;
    int sizeMode = 1;              /* PFLPS_SIZE_MODE_ON */
};

/* one pfTexture */
struct PfOsgTex {
    osg::ref_ptr<osg::Texture2D> tex;
    osg::ref_ptr<osg::Image> img;
    int repeatS = 0, repeatT = 0;   /* PFTEX_* wrap tokens, 0 = default */
    int minFilt = 0, magFilt = 0;
    int intFormat = 0, extFormat = 0, imgFormat = 0;
    int comp = 0, sx = 0, sy = 0, sz = 0;
    unsigned int* imgData = nullptr;
};

typedef void (*PfosgChanFunc)(struct pfChannel*, void*);
typedef void (*PfosgPWinFunc)(struct pfPipeWindow*);

/* one Performer channel.  The first pfNewChan is the main scene channel
 * (fields mostly live in PfOsgState for historical reasons); later channels
 * are auxiliary 2D overlays (the libpfutil GUI panel) whose CULL/DRAW
 * callbacks the frame loop invokes with raw GL after the OSG scene render. */
struct PfOsgChan {
    bool isMain = false;
    float vpL = 0, vpR = 1, vpB = 0, vpT = 1;   /* window fractions */
    bool ortho = false;
    float orthoL = 0, orthoR = 1, orthoB = 0, orthoT = 1;
    float nearD = 1, farD = 10000;
    bool drawOn = true;                 /* pfChanTravMode(PFTRAV_DRAW) */
    unsigned statsClasses = 0x2;        /* PFFSTATS_ENPFTIMES default */
    PfosgChanFunc cullFunc = nullptr;
    PfosgChanFunc drawFunc = nullptr;
};

struct PfOsgState {
    bool inited = false;
    double startTicks = 0.0;

    std::vector<std::string> filePath;
    std::vector<osg::ref_ptr<osg::Referenced>> keep;
    std::vector<PfOsgGSet*> gsets;

    SDL_Window* window = nullptr;
    SDL_GLContext glctx = nullptr;
    std::string winName = "OpenGL Performer";
    int winX = 0, winY = 0, winW = 1280, winH = 960;
    bool winOpen = false;
    PfosgPWinFunc winConfigFunc = nullptr;

    osg::ref_ptr<osg::Group> root;      /* viewer root: [esky, scene] */
    osg::ref_ptr<osg::Group> scene;
    float fovH = 45.0f, fovV = -1.0f;
    float nearD = 1.0f, farD = 10000.0f;
    osg::Vec3d eye, hpr;
    bool haveView = false;
    osg::Matrixd viewMat;           /* Performer eye-to-world (row-vector) */
    bool haveViewMat = false;
    PfOsgESky* esky = nullptr;
    PfosgChanFunc drawFunc = nullptr;
    bool defaultLighting = true;

    osg::ref_ptr<osgViewer::Viewer> viewer;
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> gw;
    long frameCount = 0;

    std::vector<PfOsgChan*> chans;      /* [0] = main scene channel */

    /* F9 debug pause: freezes pfGetTime so simulation (vehicles, tether,
     * xformer) halts while rendering continues */
    bool paused = false;
    double pausedAt = 0.0;          /* frozen sim-time while paused */

    /* light-point statesets awaiting a projection-scaled osg::Point
     * (desktop GL path; sized in applyChannel once the window exists) */
    std::vector<std::pair<osg::ref_ptr<osg::StateSet>, PfOsgLPState>>
        pendingLPointSS;

    /* frame timing + scene counts for pfDrawChanStats */
    static const int STATS_DTS = 128;
    float statsDt[STATS_DTS] = {0};     /* recent frame times, seconds */
    int statsDtHead = 0;
    float statsAppMs = 0, statsCullMs = 0, statsDrawMs = 0;
    long statsTris = 0, statsVerts = 0, statsGeodes = 0, statsDrawables = 0;
};

extern PfOsgState pfosgState;

/* input hooks, implemented in pfosg_perfly.cpp, called from pfFrame */
void pfosgInputBeginFrame(double now);
void pfosgInputSDLEvent(const SDL_Event& ev, double now);

/* immediate-mode draw phase (aux channel CULL/DRAW callbacks): pf state and
 * matrix calls route to raw GL while this is true (pfosg_gui.cpp) */
extern bool pfosgInDrawPhase;

/* run aux (overlay) channels' CULL/DRAW callbacks with raw GL; called from
 * pfFrame after the OSG scene render, before swap (pfosg.cpp) */
void pfosgRunAuxChannels(void);

/* resolve a pfChannel handle to its shim struct (pfosg.cpp) */
PfOsgChan* pfosgChanOf(struct pfChannel* ch);

/* pfIsOfType helper for scene-graph class cookies (pfosg_gui.cpp):
 * returns 1/0 for a recognized cookie, -1 if the cookie isn't ours */
int pfosgIsOfNodeClass(void* obj, void* type);

/* geoset compile hook (pfosg.cpp) needed by intersection/xformer code */
void pfosgCompileDirtyGSets(void);

/* resolve a filename against the pfFilePath search list (pfosg.cpp) */
std::string pfosgResolveFile(const char* name);

#endif /* PFOSG_INTERNAL_H */
