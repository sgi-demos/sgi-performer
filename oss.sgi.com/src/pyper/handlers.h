
// from STL
#include <string>
#include <list>    // simple list
#include <deque>   // double ended queue

// from SGI OpenGL|Performer
#include <Performer/pf/pfNode.h>

// from CWI's Python
#include <Python.h>


const int PYPER_EVENT_TICK    = 1;
const int PYPER_EVENT_SELECT  = 2;
const int PYPER_EVENT_TOUCH   = 4;


class PyperEvent
{
  public:
    PyperEvent(int tp);
    ~PyperEvent();
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

class RegisteredHandler
{
  public:
    RegisteredHandler();
    RegisteredHandler(PyObject *a_pfnode, PyObject *a_func, int msk);
    ~RegisteredHandler();
    void QueueEvent(PyObject *ev);
    void HandleEvents(void);
    void Call(PyObject *pyobj, PyObject *pyevt);

    PyObject *pfnode;
    PyObject *func;
    int       evmask;
    std::deque<PyObject*> eventqueue;
};


class RegisteredHandlers
{
  public:
    RegisteredHandlers();
    ~RegisteredHandlers();

    void Update(void);
    void GenerateTickEvents(PyObject *pyevt);
    void GenerateEvent(PyObject *pyevt, PyObject *pyobj);
    void AddHandler(PyObject *pyobj, PyObject *pyfunc, int msk);
    std::string GetDesc(void) const;

  protected:

    RegisteredHandler *GetHandlerForObject(PyObject *pfnode);
    RegisteredHandler *GetHandlerForChildObject(pfNode *n);

    std::list<RegisteredHandler> handlers;
};


