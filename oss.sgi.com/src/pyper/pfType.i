
// This file contains the public interface from the Performer
// header file pr/pfType.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{

#include <Performer/pr/pfType.h>

%}


class pfType
{


/*
    void* operator 	new(size_t s);
    void operator 	delete(void*) {};
*/

public:
    %addmethods
    {
      char *__str__()
      {
        static char temp[256];
        const char *name = self->getName();
        if (name)
          sprintf(temp, "pfType named '%s'", name);
        else
          sprintf(temp, "unnamed pfType");
        return temp;
      }

    }
    pfType(pfType *parent, char *name);
    ~pfType();


    pfType *getParent();
 
    int	isDerivedFrom(pfType *ancestor);
    static void setMaxTypes(int _n);
    const char *getName();
};






