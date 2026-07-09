# webfly (web tether) + unified GLES2 path — status

**Goal:** SGI's `perfly` compiled byte-for-byte unmodified, running the shipped
`town-tether.perfly` config in the browser (Emscripten/WebGL, GUI-less) via the
pfosg shim on OSG-GLES2. The camera rides the Esprit through Performer Town.

**Current state: WORKING.** The town renders in the browser, textured, with
tree-billboard cutouts, at interactive frame rates; the camera rides the
Esprit exactly as on desktop. All changes uncommitted.

---

## The black-screen bug, solved (2026-07-08)

The long-standing "town loads, camera rides, framebuffer black" mystery was
**never a rasterization, shader, camera, or geometry problem**. Hooking the
WebGL context from the browser showed ~110k triangles drawing per frame with
zero GL errors and the uber-shader program bound — into a **zero-width
viewport**: `glViewport(934, 0, 0, 1500)`.

Root cause chain:
1. `town-tether.perfly` runs `-g 1` → GUI on, `GUI_VERTICAL` format.
2. perfly's `updateGUI(TRUE)` (gui.c) computes the 3D master channel's
   viewport **from** `pfuGetGUIViewport`: in the vertical format, the scene's
   *left edge* is the GUI panel's *right edge* `r`.
3. The web stub `pfuGetGUIViewport` (pfosg_web_stubs.cpp) ignored what
   `pfuGUIViewport` had stored and returned a fabricated horizontal strip
   `(0, 1, 0, 0.2)` → `r = 1.0` → master channel viewport `(1, 1, 0, 1)` —
   zero pixels wide. Everything drew; nothing was visible.

Desktop was immune because the real libpfutil gui.c faithfully round-trips
the viewport. Same lesson as the cullDelta widget bug: **stubs must
round-trip whatever perfly writes through them.**

Fix: the stub now stores the GUI viewport's left/bottom edge and reports the
GUI as occupying **zero area** there (r=l, t=b), so `updateGUI` hands the
scene the full window — which is what GUI-less means.

## Fixed in the same pass

- **osg::State cache corruption from the aux-channel raw-GL sandbox.**
  `pfosgRunAuxChannels` brackets perfly's DRAW callbacks with
  `glPushAttrib/glPopAttrib` — inert stubs on GLES2, so real GL calls made
  inside (this file's `glViewport`/`glBindBuffer(0)`, `pfBasicState`'s
  `glDisable(GL_DEPTH_TEST/GL_CULL_FACE/GL_BLEND)` every frame from perfly's
  message drawing) leak into the next frame behind OSG's back. On web, after
  the callbacks run, the shim re-syncs: buffer-binding cache zeroed, and the
  four GLES2-enable caches written with **GL truth** via
  `State::applyMode(mode, glIsEnabled(mode))`.

  ⚠️ Lesson: `State::dirtyAllModes()` is the WRONG re-sync here — it blindly
  *flips* `last_applied_value`. OSG's own end-of-stage revert had already
  left the depth-test cache coherent (false/false); the flip made it
  true/false, so OSG skipped `glEnable(GL_DEPTH_TEST)` forever → the whole
  scene rendered painter's-order (mountains over buildings, car body over
  the world). Sampling `glIsEnabled` into `applyMode` is the correct sync.
  `GL_DEPTH_TEST ON` is additionally asserted on the scene root's stateset
  (webglSetupRoot) as belt-and-braces.
- **Alpha test (cutout billboards).** GLES2 has no fixed-function
  `glAlphaFunc`; town's trees drew as opaque white/yellow quads. The shim's
  `PFSTATE_ALPHAFUNC` handler now also sets a `pfAlphaRef` uniform
  (GREATER/GEQUAL funcs), and the uber fragment shader discards below it.
- **`pfBasicState` FFP enums** gated off on web (each was a per-call
  `GL_INVALID_ENUM` on WebGL).
- **`--emrun` removed** from the link flags (it clobbered the shell's
  `Module.arguments` and turned every console line into an HTTP POST; the
  warning-flood stall it caused is gone — ~17 fps now). Plain
  `webfly.html` URLs work; stdout/stderr go to the browser console.
- Debug `fprintf`s removed from `pfFilePath()` / `pfdLoadFile()`.

## Known cosmetics (not blockers)

- `.rgb` (SGI image) textures aren't readable by the static web OSG build
  (no sgi plugin) — affected geometry falls back to the 1×1 white default
  texture. The town model itself is `.pfi`-textured and unaffected.
- `osg::setNotifyLevel(FATAL)` stays on for the web build (SGI databases
  carry FFP state OSG would warn about per drawable per frame). Note it also
  hides GLSL compile logs — flip to `WARN` when debugging shaders.

## M3 lead-in: unified GLES2 path, native + web (2026-07-08)

The GLES2 render path is now ONE code path behind `PFOSG_GLES2` (set by
CMake), with three build flavors:

| flavor | tree | OSG | GL |
|---|---|---|---|
| desktop GL | `build/` | Homebrew (FFP) | Apple GL 2.1 |
| **native GLES2** | `build-gles2/` | `external/OpenSceneGraph/build-gles2` | **ANGLE (Metal)** |
| web | `build-web/` | `external/OpenSceneGraph/build-em` | WebGL1 |

Native GLES2 perfly runs the tether demo correctly — verified by screenshot,
renderer string `ANGLE (Apple, Apple M3, OpenGL 4.1 Metal)`. Web verified
unchanged after the refactor. `__EMSCRIPTEN__` now guards only the truly
web-only bits (Asyncify yield, emscripten.h, web-shell concerns).

Native GLES2 specifics:
- OSG-GLES2 native build: same flags as build-em, plus
  `-DOPENGL_HEADER1='#include <GLES2/gl2.h>'` (OSG's cmake assumes iOS on
  APPLE+GLES2), ANGLE include/libs from `~/Github/opengl-for-mac`,
  `-DDYNAMIC_OPENTHREADS=OFF`, CoreFoundation+Foundation frameworks.
- `src/pfosg/include/GL/gl.h` grew a GLES2 branch: `<GLES2/gl2.h>` + legacy
  FFP declarations/tokens (matching pfosg_gles_compat.cpp) so the overlay
  code and SGI loaders compile without desktop GL.
- SDL needs `SDL_HINT_OPENGL_ES_DRIVER=1` (set in openWindow) and the run
  needs `DYLD_FALLBACK_LIBRARY_PATH=$HOME/Github/opengl-for-mac/lib`
  (ANGLE dylib install names are CWD-relative).
- **glReadPixels(GL_RGB) silently fails on GLES default framebuffers**
  (only RGBA8 is guaranteed) — the screenshot path reads RGBA on GLES2 and
  falls back to PPM when no osgDB image plugin is linked. Capture must also
  happen BEFORE SDL_GL_SwapWindow (post-swap reads are undefined; black on
  ANGLE/Metal, worked by luck on Apple GL).
- Debug knobs: `PFOSG_GL_PROBE=<frame>` one-shot GL state dump,
  `PFOSG_OSG_NOTIFY=<level>` un-silences OSG (shows GLSL logs).

## GLES2 overlay renderer (2026-07-08, same day)

The GUI panel, stats, and messages now draw on the GLES2 flavors — SGI's
libpfutil `gui.c` compiles in UNMODIFIED (as on desktop) because
pfosg_gles_compat.cpp is no longer no-op stubs but a **batched
immediate-mode mini-GL**: MODELVIEW/PROJECTION matrix stacks, glBegin/glEnd
vertex+color batching drawn through a small color-only shader, QUADS →
triangles + POLYGON → fan conversion (GLES2 draws the other modes natively),
and glPushAttrib/glPopAttrib real save/restore of the state the overlays
touch.  Everything a batch clobbers (program, buffer binding, attrib
enables) is queried and restored so osg::State's caches stay truthful.
glGetIntegerv/glGetFloatv are interposed via the shim GL/gl.h (which now
serves web too, not just native) so legacy queries (GL_MATRIX_MODE,
GL_PROJECTION_MATRIX) answer from the mini-GL stacks.
pfosg_web_stubs.cpp is deleted — one TU set for all three flavors.

Web gotcha: gui.c redraws damage-only, assuming the framebuffer persists
across swaps; WebGL invalidates it every composite (ANGLE: undefined).  The
shim calls `pfuRedrawGUI()` each frame on GLES2 builds so the panel fully
redraws.  Verified: full panel on native GLES2 (screenshot: every widget,
sliders, checkboxes, 5x7-font labels) and on web; desktop unchanged.

## .rgb textures + vsync (2026-07-08, same day)

- **SGI `.rgb` images load on the GLES2 flavors** — `pfb2osgLoadRgbImage`
  (src/loaders/pfb2osg) implements the SGI image-library format (512-byte
  BE header, channel-planar, verbatim + RLE, bpc=1) and both dispatch
  points (`pfLoadTexFile`, pfb2osg's `loadPfi` non-pfi branch) fall back to
  it when `osgDB::readImageFile` has no plugin.  Zero texture-load warnings
  on native GLES2 now (was: truck*, perfbann white).  Desktop still prefers
  the osgDB plugin — identical behavior.
- **ANGLE vsync**: `SDL_GL_SetSwapInterval(1)` reports success and the
  native build runs ~65 fps with the GUI (failure now logged as
  "vsync unavailable" if it ever regresses).

## Stoplights lit + cycling (2026-07-08, same day)

The town's stoplight bulbs are **light points** — `PFGS_POINTS` geosets
(pure red/yellow/green) under `pfSequence` nodes with authored phase times
(20s red / 3s yellow / 10s green), one per intersection.  Two bugs kept
them dark on every flavor:

1. **Point size never applied.**  The geosets carry `pntSize = 0` (real
   Performer sizes light points from the pfLPointState, which the shim
   stubs), so they rasterized as 1px-or-nothing.  `compileGSet` now sizes
   POINTS geosets (authored size, or a 6px lit-bulb default): `osg::Point`
   on desktop, a `pfPointSize` uniform + `gl_PointSize` in the uber vertex
   shader on GLES2.
2. **Sequence frame times orphaned.**  pfpfb sets per-frame times
   (`pfSeqTime`) BEFORE attaching children, and `osg::Sequence::addChild`
   inserts a default time ahead of them — every frame ended up on the 1s
   default and the authored phases were never used.  `PfOsgSequence`
   (pfosg_pfb.cpp) attaches children without touching the preloaded
   frame-time list.

Verified: green/amber bulbs lit at intersections on native GLES2 and
desktop (street lamps got their lit points too); sequences run
(PFSEQ_START in the database, osg::Sequence updates in the traversal).

## Build & run

```
# web
source ~/Github/emsdk/emsdk_env.sh
cmake --build build-web --target webfly
python3 -m http.server 8092 --directory build-web/apps/webfly
# open: http://localhost:8092/webfly.html

# native GLES2 (ANGLE)
cmake -B build-gles2 -DPF_GLES2_OSG=$PWD/external/OpenSceneGraph
cmake --build build-gles2 --target perfly
DYLD_FALLBACK_LIBRARY_PATH=$HOME/Github/opengl-for-mac/lib \
  ./build-gles2/apps/perfly/perfly data/town-tether.perfly
```
