
![SGI Performer Logo](oglperformer.jpg)

![Performer Town](performer-town.jpg)

## SGI Performer
_The open parts of OpenGL Performer, and a port that runs SGI's Performer demos on OpenSceneGraph, natively and in the browser_

SGI never released the Performer core library (libpf, libpr) as source. It did release the headers, the sample programs (perfly and the rest), the utility libraries, the database loaders, and the Performer Town demo database, both with the product and through its open-source project at oss.sgi.com. This repository holds those open parts and a port built on them:

- [web/](web/README.md): the port. SGI's `perfly` and sample programs compiled unmodified against the Performer API, with the core library reimplemented on the open-source [OpenSceneGraph](https://github.com/openscenegraph/OpenSceneGraph) by the pfosg shim, on SDL2 and OpenGL ES 2 for native and Emscripten builds. [Performer Town in the browser](https://sgi-demos.github.io/sgi-performer/web/apps/webfly/web/).
- [web/vendor/Performer/](web/vendor/Performer/README.md): the headers, samples, utility libraries, loaders, and tools from the OpenGL Performer 3.0 for Windows release (2002-12-10), verbatim.
- [web/data/town/](web/data/town/README.md): the Performer Town database, textures, paths, and vehicles from the same release.
- [oss.sgi.com/](oss.sgi.com/): SGI's open-source Performer tree, from an [archive.org mirror](https://web.archive.org/web/20171010104701/http://oss.sgi.com/cgi-bin/cvsweb.cgi/performer/) of the oss.sgi.com CVS.

Not included: the OpenGL Performer 3.0 product itself (the core libraries, executables, installer, and license manager). The port does not use it; everything it needs is in the open parts above.

## License

The Performer headers, sample programs (perfly and the others), utility libraries, database loaders, and the Performer Town data are SGI's, reproduced verbatim under the sample-code license in each file. That license permits reproduction, distribution, and derivative works on three conditions: keep the notice, do not use SGI's name in advertising, and use the code only "in conjunction with OpenGL Performer". The first two are met. The third is not met literally: here the samples are compiled against the Performer API reimplemented on OpenSceneGraph, because OpenGL Performer itself no longer runs on any current platform. The project believes this is within the spirit of the license: SGI released these same sources through its open-source Performer project at oss.sgi.com, the condition existed to keep the samples from seeding a competing scene-graph product, and this is a noncommercial port of SGI's own demos onto a free scene graph so that they can still be seen. The pfosg shim, loaders, and build files are the project's own work under the Apache License 2.0; OpenSceneGraph is under the OSGPL. The full provenance and fair-use record for the sgi-demos repositories is in [sgi-demos/docs/COPYRIGHT.md](https://github.com/sgi-demos/sgi-demos/blob/main/docs/COPYRIGHT.md).

This project is not affiliated with or endorsed by Hewlett Packard Enterprise or Silicon Graphics. OpenGL Performer and the SGI logo are trademarks of their owner, used here only to identify the software being preserved. Rights holders with concerns can open an issue.
