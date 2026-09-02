/* compat shim: <malloc.h> does not exist on macOS/BSD; everything the
 * Performer headers want from it lives in <stdlib.h>. */
#pragma once
#include <stdlib.h>
