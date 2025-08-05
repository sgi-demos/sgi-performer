
// This file contains the public interface from the Performer
// header file pf/pfPipeWindow.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pf/pfPipeWindow.h>
%}

class pfPipeWindow
{
public:

    void setName(const char *name);
    const char* getName();
    void setWSConnectionName(const char *name);
    const char* getWSConnectionName();
    void setMode(int mode, int val);
    int getMode(int _mode);
    void setWinType(uint type);
    uint getWinType() const;
    pfState* getCurState();
    void setAspect(int x, int y);
    void getAspect(int *x, int *y);
    void setOriginSize(int xo, int yo, int xs, int ys);
    void setOrigin(int xo, int yo);
    void getOrigin(int *xo, int *yo);
    void getScreenOrigin(int *xo, int *yo);
    void setSize(int xs, int ys);
    void getSize(int *xs, int *ys);
    void setFullScreen();
    void getCurOriginSize(int *xo, int *yo, int *xs, int *ys);
    void getCurScreenOriginSize(int *xo, int *yo, int *xs, int *ys);
    void setOverlayWin(pfWindow *ow);
    pfWindow* getOverlayWin();
    void setStatsWin(pfWindow *sw);
    pfWindow* getStatsWin();
    void setScreen(int screen);
    int getScreen();
    void setIndex(int index);
    int getIndex();
    pfWindow* getSelect();
    void setSwapBarrier(int _barrierName);
    int getSwapBarrier();
    void setWinList(pfList *_wl);
    pfList* getWinList() const;
    int attachWin(pfWindow *w1);
    int detachWin(pfWindow *w1);
    int attach(pfPipeWindow *pw1);
    int detach(pfPipeWindow *pw1);
    void attachSwapGroup(pfPipeWindow *_w1);
    void attachWinSwapGroup(pfWindow *_w1);
    void detachSwapGroup();
    pfWindow* select();
    void swapBuffers();
    int isOpen();
    int isManaged();
    int inSwapGroup();
    int query(int _which, int *_dst);
    int mQuery(int *_which, int *_dst);

    pfPipe* getPipe();
    int getPipeIndex() const;
    void setConfigFunc(pfPWinFuncType _func);
    pfPWinFuncType getConfigFunc();
    int getChanIndex(pfChannel *_chan);
    void config();
    void open();
    void close();
    void closeGL();
    int removeChan(pfChannel *_chan);
    int addChan(pfChannel *_chan);
    void insertChan(int _where, pfChannel *_chan);
    int moveChan(int _where, pfChannel *_chan);
    pfChannel* getChan(int which);
    int getNumChans() const;
    int addPVChan(pfPipeVideoChannel *_vchan);
    void setPVChan(int _num, pfPipeVideoChannel *_vchan);
    void removePVChan(pfPipeVideoChannel *_vchan);
    void removePVChanIndex(int _num);
    pfPipeVideoChannel* getPVChan(int _num);
    pfPipeVideoChannel* getPVChanId(int _num);
    pfPipeVideoChannel* getPosPVChan(int _x, int _y);
    int getPVChanIndex(pfPipeVideoChannel *_vchan);
    int getNumPVChans() const;
    void bindPVChans();
    void unbindPVChans();

    pfPipeWindow(pfPipe *p);
    virtual ~pfPipeWindow();

    static pfType* getClassType();
 
};

