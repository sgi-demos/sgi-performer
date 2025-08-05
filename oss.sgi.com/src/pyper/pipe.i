
// This file contains the public interface from the Performer
// header file pf/pfPipe.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pf/pfPipe.h>
%}

extern pfPipe*	pfGetPipe(int _pipeNum);
extern int	pfInitPipe(pfPipe *_pipe, pfPipeFuncType _configFunc);

class pfPipe
{
public:

    void setSwapFunc(pfPipeSwapFuncType func);
    pfPipeSwapFuncType getSwapFunc() const;
    float getLoad() const;
    void getSize(int  *xs, int  *ys) const;
    void setScreen(int scr);
    int getScreen() const;
    void setWSConnectionName(const char *name);
    const char* getWSConnectionName() const;
    pfMPClipTexture* getMPClipTexture(int  i) const;
    int getNumMPClipTextures() const;
    pfChannel* getChan(int  i) const;
    int getNumChans() const;
    pfPipeWindow* getPWin(int  i) const;
    int getNumPWins() const;
    void setIncrementalStateChanNum(int _num);
    int getIncrementalStateChanNum() const;
    int getHyperId() const;
    void bindPVChans();
    void unbindPVChans();
    int movePWin(int  where, pfPipeWindow *_pw);
    void addMPClipTexture(pfMPClipTexture *clip);
    int removeMPClipTexture(pfMPClipTexture *clip);
    void setTotalTexLoadTime(float _totalTexLoadTime);
    float getTotalTexLoadTime() const;
    void setSize(int xs, int ys);
    virtual ~pfPipe();
    static pfType* getClassType();
};

