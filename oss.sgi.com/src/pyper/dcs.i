
// This file contains the public interface from the Performer
// header file pf/pfDCS.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pf/pfDCS.h>
%}


class pfDCS : public pfSCS
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
          sprintf(temp, "pfDCS named '%s' with %d children", name, cnt);
        else
          sprintf(temp, "unnamed pfDCS with %d children", cnt);
        return temp;
      }
      // ARGH! UGLY HACK TO FIX BROKEN POLYMOPHISM IN SWIG! - Bram
      pfNode *GetAsNode(void) { return self; }
    }

    void getMat(pfMatrix& m);

    const pfMatrix* getMatPtr();

    void setMatType(uint val);

    uint getMatType() const;

    void setMat(pfMatrix& m);

    void setCoord(pfCoord *c);

    void setRot(float h, float p, float r);

    void setTrans(float x, float y, float z);

    void setScale(float xs, float ys, float zs);

    pfDCS();

    virtual ~pfDCS();

    static pfType* getClassType();
};

