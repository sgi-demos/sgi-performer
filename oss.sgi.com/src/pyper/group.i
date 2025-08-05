
// This file contains the public interface from the Performer
// header file pf/pfNode.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pf/pfGroup.h>
%}

class pfGroup : public pfNode
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
          sprintf(temp, "pfGroup named '%s' with %d children", name, cnt);
        else
          sprintf(temp, "unnamed pfGroup with %d children", cnt);
        return temp;
      }
    }

    int addChild(pfNode *child);

    int insertChild(int  index, pfNode *child);

    int removeChild(pfNode *child);

    int replaceChild(pfNode *oldn, pfNode *newn);

    int bufferAddChild(pfNode *child);

    int bufferRemoveChild(pfNode *child);

    pfNode* getChild(int  i) const;

    int getNumChildren() const;

    int searchChild(pfNode *n) const;

    pfGroup();

    virtual ~pfGroup();

    static pfType* getClassType();

};

