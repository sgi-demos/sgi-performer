/* X11/Xlib.h - pfosg stub (NOT the real Xlib).
 *
 * On IRIX/Linux the Performer headers dragged in Xlib, so shipped sample
 * code uses Display/Window/XEvent freely in its window callbacks.  This stub
 * supplies just enough to compile those callbacks; the event functions are
 * no-ops (XEventsQueued returns 0, so sample X event loops never execute) -
 * real input arrives through SDL2 inside the shim's pfFrame instead. */
#ifndef PFOSG_XLIB_H
#define PFOSG_XLIB_H

#ifdef __cplusplus
extern "C" {
#endif

/* matches the real (Windows-tree) pfutil.h, which typedefs Display as void */
#ifndef PFOSG_DISPLAY_DEFINED
#define PFOSG_DISPLAY_DEFINED
typedef void Display;
#endif
typedef unsigned long XID;
typedef unsigned long Window;
typedef unsigned long KeySym;

#define None       0L
#define KeyPress   2
#define KeyRelease 3

#define KeyPressMask   (1L << 0)
#define KeyReleaseMask (1L << 1)

#define QueuedAlready      0
#define QueuedAfterReading 1
#define QueuedAfterFlush   2

typedef struct {
    int type;
    unsigned long serial;
    int send_event;
    Display* display;
    Window window;
    unsigned int state;
    unsigned int keycode;
} XKeyEvent;

typedef union _XEvent {
    int type;
    XKeyEvent xkey;
    long pad[24];
} XEvent;

typedef struct { int pad; } XComposeStatus;

extern int  XSelectInput(Display* dsp, Window w, long mask);
extern int  XMapWindow(Display* dsp, Window w);
extern int  XEventsQueued(Display* dsp, int mode);
extern int  XNextEvent(Display* dsp, XEvent* ev);
extern int  XLookupString(XKeyEvent* ev, char* buf, int nbytes,
                          KeySym* keysym, XComposeStatus* status);

#ifdef __cplusplus
}
#endif

#endif /* PFOSG_XLIB_H */
