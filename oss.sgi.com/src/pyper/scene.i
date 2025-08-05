
// This file contains the public interface from the Performer
// header file pf/pfScene.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pf/pfScene.h>
%}


class pfScene : public pfGroup
{
public:

    void setGState(pfGeoState *gs);

    pfGeoState* getGState() const;

    void setGStateIndex(int gs);

    int getGStateIndex() const;

    pfScene();

    virtual ~pfScene();

    static pfType* getClassType();

};

