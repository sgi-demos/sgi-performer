// This file contains the public interface from the Performer
// header file pf/pfGeode.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pf/pfGeode.h>
%}

class pfGeode : public pfNode
{
public:

    %addmethods
    {
      char *__str__()
      {
        static char temp[256];
        const char *name = self->getName();
        int cnt = self->getNumGSets();
        if (name)
          sprintf(temp, "pfGeode named '%s' with %d geosets", name, cnt);
        else
          sprintf(temp, "unnamed pfGeode with %d geosets", cnt);
        return temp;
      }
    }

    int addGSet(pfGeoSet *gset);
    int insertGSet(int index, pfGeoSet *gset);
    int replaceGSet(pfGeoSet *oldgs, pfGeoSet *newgs);
    int removeGSet(pfGeoSet *gset);
    pfGeoSet* getGSet(int i) const;
    int getNumGSets() const;
    pfGeode();
    virtual ~pfGeode();
    static pfType* getClassType();
};

