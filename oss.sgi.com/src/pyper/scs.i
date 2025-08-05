
// This file contains the public interface from the Performer
// header file pf/pfSCS.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pf/pfSCS.h>
%}

class pfSCS : public pfGroup
{
public:
    %addmethods
    {
      char *__str__()
      {
        static char temp[256];
        const char *name = self->getName();
	int cnt = self->getNumChildren();
        if (name)
          sprintf(temp, "pfSCS named '%s' with %d children", name, cnt);
        else
          sprintf(temp, "unnamed pfSCS with %d children", cnt);
        return temp;
      }
    }

    void getMat(pfMatrix& m);

    const pfMatrix* getMatPtr();

    pfSCS(pfMatrix& m);

    virtual ~pfSCS();

    static pfType* getClassType();

};

