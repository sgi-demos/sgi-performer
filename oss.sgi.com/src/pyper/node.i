
// This file contains the public interface from the Performer
// header file pf/pfNode.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{

#include <Performer/pf/pfNode.h>

%}


// Grab a Python function object as a Python object
%typemap(python,in) pyObject *pyfunc
{
  if (!PyCallable_Check($source)) 
  {
    PyErr_SetString(PyExc_TypeError, "Need a callable object.");
    return 0;
  }
  $target = $source;
}


class pfNode
{
public:


    %addmethods
    {
      char *__str__()
      {
        static char temp[256];
        const char *name = self->getName();
        if (name)
          sprintf(temp, "pfNode named '%s'", name);
        else
          sprintf(temp, "unnamed pfNode");
        return temp;
      }

      // Is this still req'd? - Bram
      pfGroup* castToGroup() 
      { 
        pfGroup *retval = dynamic_cast<pfGroup *>(self);
        if (!retval)
        {
          fprintf(stderr,"WARNING! object at %p is not a pfGroup. DO NOT CAST.\n", self);
	  return 0;
        }
        return((pfGroup*)(self)); 
      }
      pfDCS* castToDCS() { return((pfDCS*)(self)); }
      pfSCS* castToSCS() { return((pfSCS*)(self)); }
      pfGeode* castToGeode() { return((pfGeode*)(self)); }
      pfBillboard* castToBillboard() { return((pfBillboard*)(self)); }	

    }

    virtual ~pfNode();

    pfNode* find(const char *_name, pfType *_type);

    int setName(const char *name);

    const char* getName() const;

    unsigned int getTravMask(int which) const;

    void setTravMask(int which, unsigned int mask, int setMode, int bitOp);

    void setBufferMode(int mode, int val);

    int getBufferMode(int mode) const;

    pfGroup* getParent(int i) const;

    int getNumParents() const;

    void setBound(pfSphere *sph, int mode);

    int getBound(pfSphere *sph);

    pfNode* lookup(const char *_name, pfType* _type);

    int isect(pfSegSet *segSet, pfHit **hits[]);

    int flatten(int _mode);

    virtual int         isOfType(pfType *_type);

    pfType*             getType() const;
    const char*         getTypeName() const;

    int ref(void);
    int unref(void);
    int getRef(void);
};


