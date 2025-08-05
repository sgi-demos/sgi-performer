//
// This is a supporting file, that implements an event handling mechanism.
// This mechanism is an addition to the Performer package, which does
// not do eventhandling on a per-node basis.
// Also, it provides an animation system for interpolation misc types.
// Consider it to be a bonus addition to PyPer. - Bram
// 

%module libpyperbonus



%{

#include "handlers.h"
#include "scalint.hh"
#include "v3int.hh"
#include "quatint.hh"
#include "strint.hh"

%}


// req'd for interchanging std::string and python string objects

%typemap(python, in) const std::string&, std::string&  
{
  if (!PyString_Check($source)) 
  {
    PyErr_SetString(PyExc_TypeError, "string type is required for parameter");
    return NULL;
  }
  $target = new std::string(PyString_AsString($source), PyString_Size($source));
}

%typemap(python, freearg) const std::string&, std::string&, std::string 
{
  if ($target) delete $source;
}

%typemap(python, out) const std::string&, std::string&, const std::string, std::string  
{
  $target = PyString_FromString(($source)->c_str());
}

%typemap(python, ret) const std::string&, std::string&, std::string  
{
  delete $source;
}



%section "transformanimator"
%include transformanimator.i





const int PYPER_EVENT_TICK    = 1;
const int PYPER_EVENT_SELECT  = 2;
const int PYPER_EVENT_TOUCH   = 4;


%typemap(python,in) PyObject *pyfunc
{
  if (!PyCallable_Check($source)) 
  {
    PyErr_SetString(PyExc_TypeError, "Need a callable object.");
    return 0;
  }
  $target = $source;
}

%typemap(python,in) PyObject *pyobj
{
  $target = $source;
}

%typemap(python,in) PyObject *pyevt
{
  $target = $source;
}



class RegisteredHandlers
{
  public:

  %addmethods
  {
    char *__str__()
    {
      static char temp[512];
      sprintf
      (
        temp,
        self->GetDesc().c_str()
      );
      return temp;
    }
  }

  RegisteredHandlers();
  ~RegisteredHandlers();

  void Update(void);
  void GenerateTickEvents(PyObject *pyevt);
  void GenerateEvent(PyObject *pyevt, PyObject *pyobj);
  void AddHandler(PyObject *pyobj, PyObject *pyfunc, int msk);
}



class PyperEvent
{
  public:
    int   evtype;
    float timestamp;
};



class PyperTickEvent : public PyperEvent
{
  public:
    PyperTickEvent();
    ~PyperTickEvent();
};

class PyperSelectEvent : public PyperEvent
{
  public:
    PyperSelectEvent();
    ~PyperSelectEvent();
    bool is_selected;
};

class PyperTouchEvent : public PyperEvent
{
  public:
    PyperTouchEvent();
    ~PyperTouchEvent();
};



// ANIMATION SUBSYSTEM



enum InterpolationType
{
  INTERPOLATE_NONE,      // no interpolation
  INTERPOLATE_NEAREST,   // nearest neighbour
  INTERPOLATE_LINEAR,    // linear interpolation
  INTERPOLATE_SPLINE     // spline type interpolation
};

class AniInterpolator
{
  public:
    AniInterpolator();
    ~AniInterpolator();

    float GetMinTime(void) const;
    float GetMaxTime(void) const;
    void SetCircular(bool c);
    void SetInterpolationType(InterpolationType i);
    void SetDerivativeScale(float scale);
    float GetClosestKeyTime(float t) const;
};


class ScalarInterpolator : public AniInterpolator
{
  public:
    ScalarInterpolator();
    ~ScalarInterpolator();
    void AddEvent(float time, const float value);
    float Evaluate(float time);
    bool Store(const std::string &fname) const;
    bool Load(const std::string &fname);
};

class pfVec3Interpolator : public AniInterpolator
{
  public:
    pfVec3Interpolator();
    ~pfVec3Interpolator();
    void AddEvent(float time, const pfVec3 &val);
    %new pfVec3 Evaluate(float time);
    void Evaluation(float time, pfVec3 &v);
    %new pfVec3 EvaluateDerivative(float time);
    bool Store(const std::string &fname) const;
    bool Load(const std::string &fname);
};

class pfQuatInterpolator : public AniInterpolator
{
  public:
    pfQuatInterpolator();
    ~pfQuatInterpolator();
    void AddEvent(float time, const pfQuat &value);
    %new pfQuat Evaluate(float time);
    void Evaluation(float time, pfQuat &q);
    bool Store(const std::string &fname) const;
    bool Load(const std::string &fname);
};

class StringInterpolator : public AniInterpolator
{
  public:
    StringInterpolator();
    ~StringInterpolator();
    void AddEvent(float time, const std::string &value);
    std::string Evaluate(float time);
    bool Store(const std::string &fname) const;
    bool Load(const std::string &fname);
};

