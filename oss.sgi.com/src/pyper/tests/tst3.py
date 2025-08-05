import os
import whrandom

from libpyper import *
from libpyperbonus import *

pfInit()
pfFilePath
(
  "/home/sara/bram/PerformerCursus/pf_data:"
  "/home/sara/bram/saradist/data/models"
)
pfdInitConverter(".flt")
pfConfig()

pfPhase(PFPHASE_FREE_RUN)
#pfPhase(PFPHASE_FLOAT)
#pfPhase(PFPHASE_LOCK)
#pfPhase(PFPHASE_LIMIT)

def make_chaotic_orientation_interpolator(dt, statecount) :
  interpol = pfQuatInterpolator()
  interpol.SetCircular(1)
  interpol.SetInterpolationType(INTERPOLATE_LINEAR)
  for i in range(statecount) :
    axis = pfVec3()
    axis.set(whrandom.uniform(-1,1), whrandom.uniform(-1,1), whrandom.uniform(-1,1))
    axis.normalize()
    q = pfQuat()
    q.makeRot(whrandom.uniform(-180, 180) ,axis[0],axis[1],axis[2])
    interpol.AddEvent((dt / statecount) * i, q)
    if i == 0 :
      interpol.AddEvent(dt, q)
  return interpol


def make_chaotic_pos_interpolator(dt, statecount, center, maxdelta) :
  interpol = pfVec3Interpolator()
  interpol.SetCircular(1)
  interpol.SetInterpolationType(INTERPOLATE_SPLINE)
  for i in range(statecount) :
    p = pfVec3()
    rx = whrandom.uniform(-maxdelta[0], maxdelta[0])
    ry = whrandom.uniform(-maxdelta[0], maxdelta[0])
    rz = whrandom.uniform(-maxdelta[0], maxdelta[0])
    p.set(center[0]+rx,center[1]+ry,center[2]+rz)
    interpol.AddEvent((dt / statecount) * i, p)
    if i == 0 :
      interpol.AddEvent(dt, p)
  return interpol


rot_ipol = make_chaotic_orientation_interpolator(8.0, 5)

center = pfVec3()
center.set(0,0,0)
maxdelta = pfVec3()
maxdelta.set(8,8,8)
pos_ipol = make_chaotic_pos_interpolator(8.0, 5, center, maxdelta)


#
# Make a scene
#

pipe = pfGetPipe(0)
win  = pfPipeWindow(pipe)

win.setName("hello world")
win.setSize(400,400)
win.open()
pfuInitInput(win, PFUINPUT_X)

sun = pfLightSource()

dcs = pfDCS()
dcs.setName("esprit dcs")
scene = pfScene()
scene.addChild(dcs)
scene.addChild(sun)

nod = pfdLoadFile("esprit.flt")
dcs.addChild(nod)

chan = pfChannel(pipe)
chan.setScene(scene)

hpr = pfVec3()
hpr.set(0,0,0)
xyz = pfVec3()
xyz.set(0,-27,0)

chan.setView(xyz, hpr)



#
# Event handler
#

def myeventhandler(node, event) :
  if (event.evtype == PYPER_EVENT_TICK) :
    p=pfVec3();
    q=pfQuat();
    pos_ipol.Evaluation(event.timestamp, p)
    rot_ipol.Evaluation(event.timestamp, q)
    m = pfMatrix()
    m.makeQuat(q)
    m.setRow(3,p)
    node.setMat(m)
    del m
    del p
    del q
  if (event.evtype == PYPER_EVENT_TOUCH) :
    print node, " was touched!"




handlers = RegisteredHandlers()
handlers.AddHandler(dcs, myeventhandler, PYPER_EVENT_TICK)

ev = PyperTouchEvent()
handlers.GenerateEvent(ev, dcs)



start = os.times()[4]
overallstart = start

framecount = 10000

for i in range(framecount):
  pfFrame()
  ev = PyperTickEvent()
  handlers.GenerateTickEvents(ev)
  handlers.Update()
  if ((i+1)%100 == 0) :
    now = os.times()[4]
    delta = now - start
    fps = 100.0 / delta
    print fps, "fps"
    start = now
  del ev

now=os.times()[4]
delta = now - overallstart
fps = framecount / delta

print "overall fps has been: ", fps

pfExit()

