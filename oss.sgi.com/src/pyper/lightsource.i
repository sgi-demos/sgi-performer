
// This file contains the public interface from the Performer
// header file pf/pfLightSource.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pf/pfLightSource.h>
%}

class pfLightSource : public pfNode
{
public:

    void setColor(int _which, float _r, float _g, float _b)  ;
    void getColor(int _which, float* _r, float* _g, float* _b)  ;
    void setAmbient(float _r, float _g, float _b)  ;
    void getAmbient(float* _r, float* _g, float* _b)  ;
    void setPos(float _x, float _y, float _z, float _w)  ;
    void getPos(float* _x, float* _y, float* _z, float* _w)  ;
    void setAtten(float _a0, float _a1, float _a2)  ;
    void getAtten(float* _a0, float* _a1, float* _a2)  ;
    void setSpotDir(float _x, float _y, float _z)  ;
    void getSpotDir(float* _x, float* _y, float* _z)  ;
    void setSpotCone(float _f1, float _f2)  ;
    void getSpotCone(float* _f1, float* _f2)  ;
    void on()  ;
    void off()  ;
    int isOn()  ;
    void setMode(int mode, int val)  ;
    int getMode(int mode) const  ;
    void setVal(int mode, float val)  ;
    float getVal(int mode) const  ;
    void setAttr(int attr, void *obj)  ;
    void* getAttr(int attr) const  ;
    pfLightSource();
    virtual ~pfLightSource();

    static pfType* 	    getClassType();
};

