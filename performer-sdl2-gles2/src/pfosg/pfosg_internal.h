/* pfosg_internal.h - shared state between pfosg.cpp (core: window, viewer,
 * channel, scene graph) and pfosg_perfly.cpp (input, xformer, stubs). */
#ifndef PFOSG_INTERNAL_H
#define PFOSG_INTERNAL_H

#include <osg/Group>
#include <osgViewer/Viewer>

#include <SDL.h>

#include <string>
#include <vector>

struct PfOsgGSet;
struct PfOsgESky;

typedef void (*PfosgChanFunc)(struct pfChannel*, void*);
typedef void (*PfosgPWinFunc)(struct pfPipeWindow*);

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
};

extern PfOsgState pfosgState;

/* input hooks, implemented in pfosg_perfly.cpp, called from pfFrame */
void pfosgInputBeginFrame(double now);
void pfosgInputSDLEvent(const SDL_Event& ev, double now);

/* geoset compile hook (pfosg.cpp) needed by intersection/xformer code */
void pfosgCompileDirtyGSets(void);

#endif /* PFOSG_INTERNAL_H */
