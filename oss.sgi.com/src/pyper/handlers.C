#include <sys/time.h>

// For glPipeInstrumentsblablabla
#include <stdio.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <GL/gl.h>


#include "handlers.h"
#include "prt.h"


// Timer related stuff

#define USEC_TO_SEC(usec) (0.000001*(usec))

#define UCLOCK_TO_SECONDS(x) (((double) x)/UCLOCKS_PER_SEC)


static void *ExtractVoidPointer(PyObject *obj)
{
  if (!obj) return 0;
  PyObject *asstring = PyObject_Repr(obj);
  if (!asstring) return 0;
  char *s = PyString_AsString(asstring);
  char *pos = strstr(s, "_");
  assert(pos);
  pos++;
  long retval=0;
  while (*pos != '_')
  {
    retval = (retval << 4);
    int val=0;
    if (*pos >= '0' && *pos <= '9')
      val=*pos - '0';
    if (*pos >= 'a' && *pos <= 'z')
      val=(*pos - 'a') + 10;
    retval = retval + val;
    pos++;
  }
  return (void *)retval;
}


float SecondsSinceFirstCall(void)
{
  static struct timeval *start=0;
  if (!start)
  {
    start = new struct timeval;
    assert(!gettimeofday(start,0));
    return 0.0;
  }
  struct timeval now;
  assert(!gettimeofday(&now,0));
  double  secdiff = now.tv_sec  - start->tv_sec;
  double usecdiff = now.tv_usec - start->tv_usec;
  return secdiff + USEC_TO_SEC(usecdiff);
}


// Event stuff

PyperSelectEvent::PyperSelectEvent() : PyperEvent(PYPER_EVENT_SELECT), is_selected(true) {}

PyperSelectEvent::~PyperSelectEvent() {}

PyperTouchEvent::PyperTouchEvent() : PyperEvent(PYPER_EVENT_TOUCH) {}

PyperTouchEvent::~PyperTouchEvent() {}

PyperTickEvent::PyperTickEvent() : PyperEvent(PYPER_EVENT_TICK) {}

PyperTickEvent::~PyperTickEvent() {}

PyperEvent::PyperEvent(int tp) : evtype(tp)
{
  timestamp = SecondsSinceFirstCall();
}

PyperEvent::~PyperEvent() {}


// RegisteredHandler stuff

RegisteredHandler::RegisteredHandler() :
  pfnode(0),
  func(0),
  evmask(0xffffffff)
{
}


RegisteredHandler::RegisteredHandler
(
  PyObject *a_pfnode, 
  PyObject *a_func, 
  int msk
) :
  pfnode(a_pfnode),
  func(a_func),
  evmask(msk)
{
}


RegisteredHandler::~RegisteredHandler() 
{
}


void RegisteredHandler::QueueEvent(PyObject *ev)
{
  eventqueue.push_front(ev);
  Py_INCREF(ev);
}


void RegisteredHandler::HandleEvents(void)
{
  // Possible infinite loop!
  while (eventqueue.size())
  {
    PyObject *ev = eventqueue.back();
    eventqueue.pop_back();
    // Make a callback to a Python function for this event
    Call(pfnode, ev);
    Py_DECREF(ev);
  }
}


void RegisteredHandler::Call(PyObject *pyobj, PyObject *pyevt)
{
  PyObject *arglist = Py_BuildValue
  (
    "OO",
    pyobj,
    pyevt
  );
  PyObject *result = PyEval_CallObject(func, arglist);
  Py_DECREF(arglist);
  Py_XDECREF(result);
}


// RegisteredHandlers stuff

RegisteredHandlers::RegisteredHandlers() 
{
}


RegisteredHandlers::~RegisteredHandlers() 
{
}


void RegisteredHandlers::Update(void)
{
  std::list<RegisteredHandler>::iterator ip;

  for (ip=handlers.begin(); ip!=handlers.end(); ++ip)
    (*ip).HandleEvents();
}


void RegisteredHandlers::AddHandler(PyObject *pyobj, PyObject *pyfunc, int msk)
{
  printf("adding handler for obj at %p with func addr at %p\n", pyobj, pyfunc);
  handlers.push_back(RegisteredHandler(pyobj, pyfunc, msk));
  Py_INCREF(pyfunc);
}


void RegisteredHandlers::GenerateTickEvents(PyObject *pyevt)
{
  std::list<RegisteredHandler>::iterator ip;
  for (ip=handlers.begin(); ip!=handlers.end(); ++ip)
  {
    if ((*ip).evmask & PYPER_EVENT_TICK)
    {
      (*ip).QueueEvent(pyevt);
    }
  }
}


void RegisteredHandlers::GenerateEvent(PyObject *pyevt, PyObject *pyobj)
{
  // Lookup the eventhandler for pyobj
  RegisteredHandler *handler;
  handler = GetHandlerForObject(pyobj);
  if (!handler)
  {
    handler = GetHandlerForChildObject((pfNode *)ExtractVoidPointer(pyobj));
  }
  if (!handler)
  {
//    printf("No handler found for pyobj at %p, not even a parent's handler\n", pyobj);
    return;
  }

  // TODO test event mask here!
  PyperEvent *ev;
  ev = (PyperEvent *)(ExtractVoidPointer(pyevt));
  int tp = ev->evtype;

  if (tp & handler->evmask)
  {
    handler->QueueEvent(pyevt);
  }
}


RegisteredHandler *RegisteredHandlers::GetHandlerForObject(PyObject *pfnode)
{
  std::list<RegisteredHandler>::iterator ip;
  for (ip=handlers.begin(); ip!=handlers.end(); ++ip)
  {
    if ((*ip).pfnode == pfnode)
      return &(*ip);
  }
  // no handler found!
  return 0;
}


RegisteredHandler *RegisteredHandlers::GetHandlerForChildObject(pfNode *n)
{
  if (!n)
    return 0;

  if (n->getNumParents()==0)
    return 0;

  pfNode *parent = (pfNode *) n->getParent(0);
  if (!parent)
    return 0;

  std::list<RegisteredHandler>::iterator ip;
  for (ip=handlers.begin(); ip!=handlers.end(); ++ip)
  {
    pfNode *asnode = (pfNode *)ExtractVoidPointer(ip->pfnode);
    if (asnode == parent)
      return &(*ip);
  }

  return GetHandlerForChildObject(parent);
}


std::string RegisteredHandlers::GetDesc(void) const
{
  std::string retval="RegisteredHandlers containing handlers for the following nodes:\n";

  std::list<RegisteredHandler>::const_iterator ip;
  for (ip=handlers.begin(); ip!=handlers.end(); ++ip)
  {
    pfNode *asnode = (pfNode *)ExtractVoidPointer(ip->pfnode);
    retval += "node named '";
    retval += asnode->getName();
    retval += "'\n";
  }
  return retval;
}

