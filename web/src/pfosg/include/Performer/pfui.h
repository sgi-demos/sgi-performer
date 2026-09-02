/* Performer/pfui.h - pfosg shim: forwards to the REAL pfui.h (pfiXformer
 * API).  In C-API mode the xformer types are opaque, so the shim supplies
 * its own motion-model implementation behind the original declarations. */
#ifndef PFOSG_PFUI_H
#define PFOSG_PFUI_H

#include <Performer/pfutil.h>

#include_next <Performer/pfui.h>

#endif /* PFOSG_PFUI_H */
