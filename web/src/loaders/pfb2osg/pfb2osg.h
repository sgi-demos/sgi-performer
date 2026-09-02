/* pfb2osg - direct .pfb (Performer Fast Binary) to OSG scene-graph loader.
 * Stage-A bring-up path; see pfb2osg.cpp for scope and simplifications. */
#ifndef PFB2OSG_H
#define PFB2OSG_H

#include <osg/Image>
#include <osg/Node>
#include <osg/ref_ptr>
#include <string>

/* Returns the scene root, or null on failure (diagnostics on stderr). */
osg::ref_ptr<osg::Node> pfb2osgLoadFile(const std::string& path);

/* Loads a Performer .pfi image file (also used by pfLoadTexFile in the
 * pfosg shim).  Returns null on failure. */
osg::ref_ptr<osg::Image> pfb2osgLoadPfiImage(const std::string& path);

/* Loads an SGI image-library file (.rgb/.rgba/.bw/.la, bpc=1) — the
 * fallback for static OSG builds that carry no osgDB image plugins. */
osg::ref_ptr<osg::Image> pfb2osgLoadRgbImage(const std::string& path);

#endif
