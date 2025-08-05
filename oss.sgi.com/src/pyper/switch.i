
// This file contains the public interface from the Performer
// header file pf/pfSwitch.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pf/pfSwitch.h>
%}




/* pfSwitchVal() */

#define PFSWITCH_ON		-1
#define PFSWITCH_OFF		-2


class pfSwitch : public pfGroup
{
  public:

    %addmethods
    {
      char *__str__()
      {
        static char temp[256];
        const char *name = self->getName();
        float val = self->getVal();
        if (name)
          sprintf(temp, "pfSwitch named '%s' with val %f", name, val);
        else
          sprintf(temp, "unnamed pfSwitch with val %f", val);
        return temp;
      }
      void toggle(void) { self->setVal((self->getVal()==PFSWITCH_ON)?PFSWITCH_OFF:PFSWITCH_ON); }
      // ARGH! UGLY HACK TO FIX BROKEN POLYMOPHISM IN SWIG! - Bram
      pfNode *GetAsNode(void) { return self; }
    }

    int setVal(float val);
    float getVal() const;
    int setValFlux(pfFlux *_valFlux);
    pfFlux* getValFlux() const;

    pfSwitch();
    virtual ~pfSwitch();
};

