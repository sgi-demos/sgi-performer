
// This file contains the public interface from the Performer
// header file pf/pfText.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{

#include <Performer/pf/pfText.h>

%}

// This is the pfText node

class pfText : public pfNode
{
public:
    %addmethods
    {
      char *__str__()
      {
        static char temp[256];
        const char *name = self->getName();
        if (name)
          sprintf(temp, "pfText named '%s'", name);
        else
          sprintf(temp, "unnamed pfText");
        return temp;
      }
    }

    int addString(pfString *str);
    int insertString(int index, pfString *str);
    int replaceString(pfString *oldgs, pfString *newgs);
    int removeString(pfString *str);
    pfString* getString(int i) const;
    int getNumStrings() const;

    pfText();
    virtual ~pfText();

    static void	   init();
    static pfType* getClassType();
};


