
![SGI Performer Logo](oglperformer.jpg)

![Performer Town](performer-town.jpg)

## SGI Performer
_SGI Performer binaries and source archive_

As far as I can tell, the only source released for Performer was for the demos and utility libraries.  SGI ported Performer to Windows and Linux and called it "OpenGL Performer".

Currently, this archive contains two versions:

1. The open components of OpenGL Performer 3.0 for Windows (2002-12-10 release): headers, sample programs, utility libraries, database loaders, tools, and the Performer Town sample data, under [web/vendor/Performer](web/vendor/Performer/README.md) and [web/data/town](web/data/town/README.md).  These carry SGI's sample-code license.  The Performer core library (libpf, libpr) is reimplemented on the open-source [OpenSceneGraph](https://github.com/openscenegraph/OpenSceneGraph) library by the pfosg shim in [web/](web/README.md), so SGI's demos compile unmodified against the Performer API; see [sgi-demos/docs/COPYRIGHT.md](https://github.com/sgi-demos/sgi-demos/blob/main/docs/COPYRIGHT.md).

2. SGI's oss.sgi.com Performer source.  This can also be found at [archive.org](https://web.archive.org/web/20171010104701/http://oss.sgi.com/cgi-bin/cvsweb.cgi/performer/), so consider this archive a more friendly Github mirror of this old CVSweb mirror.


## License

The Performer headers, sample programs (perfly and the others), utility libraries, database loaders, and the Performer Town data are SGI's, reproduced verbatim under the sample-code license in each file. That license permits reproduction, distribution, and derivative works on three conditions: keep the notice, do not use SGI's name in advertising, and use the code only "in conjunction with OpenGL Performer". The first two are met. The third is not met literally: here the samples are compiled against the Performer API reimplemented on OpenSceneGraph, because OpenGL Performer itself no longer runs on any current platform. The project believes this is within the spirit of the license: SGI released these same sources through its open-source Performer project at oss.sgi.com, the condition existed to keep the samples from seeding a competing scene-graph product, and this is a noncommercial port of SGI's own demos onto a free scene graph so that they can still be seen. The pfosg shim, loaders, and build files are the project's own work under the Apache License 2.0; OpenSceneGraph is under the OSGPL. The full provenance and fair-use record for the sgi-demos repositories is in [sgi-demos/docs/COPYRIGHT.md](https://github.com/sgi-demos/sgi-demos/blob/main/docs/COPYRIGHT.md).

This project is not affiliated with or endorsed by Hewlett Packard Enterprise or Silicon Graphics. OpenGL Performer and the SGI logo are trademarks of their owner, used here only to identify the software being preserved. Rights holders with concerns can open an issue.
