#! /bin/sh

export PYTHONPATH=$PYTHONPATH:../../lib/pyper
export PFPATH=$PFPATH:/usr/share/Performer/data

# req'd for linux Performer 2.3
export LD_PRELOAD="../../lib/pyper/libpypercmodule.so"

# for full screen anti aliasing on nVidia gfx cards
export __GL_ENABLE_FSAA=1
export __GL_FSAA_QUALITY=1
export __GL_SYNC_TO_VBLANK=0



python2.0 -i tst3.py


