# Performer Town demo data

The classic "Performer Town" sample database from SGI's **OpenGL Performer
3.0** distribution (Windows release, dated 2002-12-10), copied verbatim from
its `Data/town` and `Data/` directories:

- `town_ogl_pfi.pfb` — the town database (Performer Fast Binary, format
  version 18, big-endian/IRIX-written)
- `*.pfi` — texture images (Performer Fast Image)
- `*.path` — vehicle path definitions (blimp, city traffic, tour)
- `esprit.pfb`, `truck.pfb`, `perfBlimp.pfb`, `rocket_tux.pfb` and their
  `*.rgb`/`*.pfi` textures — the vehicles referenced by the town demo config
- `README` — the original SGI readme for the town database

## Provenance

While the core Performer libraries (libpf/libpr) remained proprietary, SGI
deliberately released the utility libraries, database loaders, sample
programs, and demo databases as open/community-shared content:

- SGI ran an open-source Performer project at `oss.sgi.com/projects/performer`
  ([archived landing page](https://web.archive.org/web/20100401034814/http://oss.sgi.com/projects/performer/),
  [archived CVS](https://web.archive.org/web/20171010104701/http://oss.sgi.com/cgi-bin/cvsweb.cgi/performer/))
  covering the utility libs, loaders, and demos — not the core library.
- Unencrypted Town/Village databases shipped as sample data in the
  `performer_friends.sw.town` subsystem from Performer 1.2 onward
  ([SGI Performer FAQ §9](https://rainbow.ldeo.columbia.edu/documentation/sgi-faq/performer/9.html)).
- The shipped loader/utility sources carry SGI's sample-code license
  permitting reproduction and distribution in conjunction with OpenGL
  Performer.

Run the demo from the repository root:

```
./build/apps/perfly/perfly data/town-osg.perfly              # the original perfly
./build/apps/townview/townview data/town/town_ogl_pfi.pfb    # self-contained viewer
```
