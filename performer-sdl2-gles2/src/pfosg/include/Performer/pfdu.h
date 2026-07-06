/* Performer/pfdu.h - pfosg shim: forwards to the REAL pfdu.h (database
 * utility API: loader entry points, builder modes, scene-graph cleanup).
 * The shim implements pfdLoadFile via the pfb2osg loader; the remaining
 * functions resolve at link time only if referenced. */
#ifndef PFOSG_PFDU_H
#define PFOSG_PFDU_H

#include <Performer/pfutil.h>

/* bits the real pfdu.h expects from pr.h / pfASD.h / the OpenGL Shader */
#ifndef PF_MAX_TEXTURES
#define PF_MAX_TEXTURES 4
#endif
typedef struct pfASDFace     pfASDFace;
typedef struct pfASDLODRange pfASDLODRange;
typedef struct pfASDVert     pfASDVert;
typedef struct islAppearance islAppearance;

#include_next <Performer/pfdu.h>

#endif /* PFOSG_PFDU_H */
