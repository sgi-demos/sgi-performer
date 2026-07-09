/* pfosg_web_stubs.cpp - Emscripten-only.  SGI's libpfutil gui.c (the GUI
 * panel) draws with immediate-mode GL that WebGL lacks, so it is excluded
 * from the web build; perfly still calls its pfu* GUI API to create widgets
 * and read their default values back into ViewState (the cullDelta lesson),
 * so these stubs round-trip widget values while drawing nothing.  This is
 * the shim's original pre-gui.c GUI surface, resurrected for the web. */

#include <Performer/pf.h>
#include <Performer/pfutil.h>

#include <cstdlib>
#include <cstring>

extern "C" {

/* widgets round-trip their value/on/selection/label/action so perfly's
 * defaults survive into ViewState; nothing is drawn */
struct PfuWidgetImpl {
    int id = 0, type = 0, on = 0, selection = 0;
    float value = 0, defValue = 0, minv = 0, maxv = 1;
    char label[128] = {0};
    void (*actionFunc)(pfuWidget*) = nullptr;
};

void pfuInitGUI(pfPipeWindow*) {}
void pfuExitGUI(void) {}
void pfuEnableGUI(int) {}
void pfuUpdateGUI(pfuMouse*) {}
void pfuRedrawGUI(void) {}
void pfuResetGUI(void) {}
int  pfuInGUI(int, int) { return 0; }

/* The GUI draws nothing on the web, so report it as occupying ZERO area at
 * the corner perfly assigned it.  perfly's updateGUI() computes the 3D
 * master channel's viewport FROM these values (GUI_VERTICAL: scene left
 * edge = GUI right edge r; GUI_HORIZONTAL: scene bottom = GUI top t), so a
 * fabricated answer is fatal: the old hardcoded r=1 gave the scene a
 * left==right==1 viewport — zero pixels wide, hence the all-black canvas
 * while every triangle "drew" fine.  Collapsing r to l (and t to b) hands
 * the scene the full window, which is what GUI-less means. */
static float guiVpL = 0.0f, guiVpB = 0.0f;
void pfuGUIViewport(float l, float /*r*/, float b, float /*t*/)
{
    guiVpL = l;
    guiVpB = b;
}
void pfuGetGUIViewport(float* l, float* r, float* b, float* t)
{
    if (l) *l = guiVpL; if (r) *r = guiVpL;
    if (b) *b = guiVpB; if (t) *t = guiVpB;
}
pfChannel* pfuGetGUIChan(void) { return nullptr; }
pfHighlight* pfuGetGUIHlight(void)
{
    static pfHighlight* hl = (pfHighlight*)calloc(1, 64);
    return hl;
}
void pfuGUICursor(int, int) {}
void pfuUpdateGUICursor(void) {}
/* pfuCursorType / pfuGetCursorType are in pfosg_perfly.cpp */

pfuPanel* pfuNewPanel(void) { return (pfuPanel*)calloc(1, 32); }
void pfuEnablePanel(pfuPanel*) {}
void pfuGetPanelOriginSize(pfuPanel*, float* x, float* y, float* xs, float* ys)
{
    if (x) *x = 0; if (y) *y = 0; if (xs) *xs = 0; if (ys) *ys = 0;
}

pfuWidget* pfuNewWidget(pfuPanel*, int type, int id)
{
    PfuWidgetImpl* w = new PfuWidgetImpl;
    w->type = type;
    w->id = id;
    return (pfuWidget*)w;
}
void pfuWidgetDim(pfuWidget*, int, int, int, int) {}
void pfuGetWidgetDim(pfuWidget*, int* x, int* y, int* xs, int* ys)
{
    if (x) *x = 0; if (y) *y = 0; if (xs) *xs = 0; if (ys) *ys = 0;
}
void pfuWidgetLabel(pfuWidget* w, const char* label)
{
    strncpy(((PfuWidgetImpl*)w)->label, label ? label : "",
            sizeof(((PfuWidgetImpl*)w)->label) - 1);
}
void pfuWidgetRange(pfuWidget* w, int, float minv, float maxv, float val)
{
    PfuWidgetImpl* wi = (PfuWidgetImpl*)w;
    wi->minv = minv; wi->maxv = maxv; wi->value = wi->defValue = val;
}
void pfuWidgetValue(pfuWidget* w, float v) { ((PfuWidgetImpl*)w)->value = v; }
float pfuGetWidgetValue(pfuWidget* w) { return ((PfuWidgetImpl*)w)->value; }
void pfuWidgetDefaultValue(pfuWidget* w, float v)
{
    PfuWidgetImpl* wi = (PfuWidgetImpl*)w;
    wi->defValue = wi->value = v;      /* default flows straight to value */
}
void pfuWidgetOnOff(pfuWidget* w, int on) { ((PfuWidgetImpl*)w)->on = on; }
void pfuWidgetDefaultOnOff(pfuWidget* w, int on)
{
    ((PfuWidgetImpl*)w)->on = on;
}
int  pfuIsWidgetOn(pfuWidget* w) { return ((PfuWidgetImpl*)w)->on; }
void pfuWidgetActionFunc(pfuWidget* w, pfuWidgetActionFuncType func)
{
    ((PfuWidgetImpl*)w)->actionFunc = (void (*)(pfuWidget*))func;
}
pfuWidgetActionFuncType pfuGetWidgetActionFunc(pfuWidget* w)
{
    return (pfuWidgetActionFuncType)((PfuWidgetImpl*)w)->actionFunc;
}
int  pfuGetWidgetId(pfuWidget* w) { return ((PfuWidgetImpl*)w)->id; }
void pfuWidgetSelections(pfuWidget*, pfuGUIString*, int*,
                         void (**)(pfuWidget*), int) {}
void pfuWidgetSelection(pfuWidget* w, int index)
{
    ((PfuWidgetImpl*)w)->selection = index;
}
int  pfuGetWidgetSelection(pfuWidget* w) { return ((PfuWidgetImpl*)w)->selection; }
void pfuHideWidget(pfuWidget*) {}
void pfuUnhideWidget(pfuWidget*) {}
void pfuEnableWidget(pfuWidget*) {}
void pfuDisableWidget(pfuWidget*) {}

/* drawing entry points: no-ops on the web (no immediate-mode GL) */
void pfuDrawGUI(pfChannel*, void*) {}
void pfuCullGUI(pfChannel*, void*) {}
void pfuDrawMessage(pfChannel*, const char*, int, int, float, float,
                    int, int) {}
void pfuDrawMessageCI(pfChannel*, const char*, int, int, float, float,
                      int, int, int) {}
void pfuDrawTree(pfChannel*, pfNode*, float*) {}
void pfuDrawChanDVRBox(pfChannel*) {}
/* pfuPreDrawStyle / pfuPostDrawStyle are in pfosg_perfly.cpp */

}   /* extern "C" */
