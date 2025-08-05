from os import *
from libpyper import *
from libpyperbonus import *

pfInit();
pfFilePath
(
  "/home/sara/bram/PerformerCursus/pf_data:"
  "/home/sara/bram/saradist/data/models"
);
pfdInitConverter(".flt");
pfdInitConverter(".pfb");
pfdInitConverter(".obj");
pfdInitConverter(".iv");
pfdInitConverter(".dxf");
pfdInitConverter(".sv");
pfdInitConverter(".wrl");
pfConfig();

def make_v3interpolator() :
  interpol = pfVec3Interpolator();
  print interpol;
  interpol.SetInterpolationType(INTERPOLATE_SPLINE);
  interpol.SetCircular(1);
  p0 = pfVec3(); p0.set(-1,0,0);
  p1 = pfVec3(); p1.set(0,-1,0);
  p2 = pfVec3(); p2.set(1,0,0);
  p3 = pfVec3(); p3.set(0,1,0);

  interpol.AddEvent(0, p0);
  interpol.AddEvent(1, p1);
  interpol.AddEvent(2, p2);
  interpol.AddEvent(3, p3);
  interpol.AddEvent(4, p0);
  return interpol;

ipol1 = make_v3interpolator()
p = pfVec3()

ipol1.SetInterpolationType(INTERPOLATE_SPLINE)
ipol1.Evaluation(0.5, p)
print p

ipol1.SetInterpolationType(INTERPOLATE_LINEAR)
ipol1.Evaluation(0.5, p)
print p


def make_quatinterpolator() :
  interpol = pfQuatInterpolator();
  interpol.SetCircular(1);
  interpol.SetInterpolationType(INTERPOLATE_LINEAR);
#  interpol.SetInterpolationType(INTERPOLATE_SPLINE);
  q0 = pfQuat(); q0.makeRot( 0,0,0,1);
  q1 = pfQuat(); q1.makeRot(45,1,0,0);
  q2 = pfQuat(); q2.makeRot(45,0,1,0);
  q3 = pfQuat(); q3.makeRot(45,0,0,1);

  interpol.AddEvent(0, q0);
  interpol.AddEvent(1, q1);
  interpol.AddEvent(2, q0);
  interpol.AddEvent(3, q2);
  interpol.AddEvent(4, q0);
  interpol.AddEvent(5, q3);
  interpol.AddEvent(6, q0);
  return interpol;

ipol2 = make_quatinterpolator()
q = pfQuat()
ipol2.Evaluation(0.5, q)
print q



#
# Make a scene
#

pipe = pfGetPipe(0);
win  = pfPipeWindow(pipe);

win.setName("hello world");
win.setSize(400,400);
win.open();
pfuInitInput(win, PFUINPUT_X);

sun = pfLightSource();

dcs = pfDCS();
dcs.setName("esprit dcs");
scene = pfScene();
scene.addChild(dcs);
scene.addChild(sun);

nod = pfdLoadFile("esprit.flt");
dcs.addChild(nod)

chan = pfChannel(pipe);
chan.setScene(scene);

hpr = pfVec3();
hpr.set(0,0,0);
xyz = pfVec3();
xyz.set(0,-7,0);

chan.setView(xyz, hpr);
chan.setStatsMode(1,0);



#
# Event handler
#

def myeventhandler(node, event) :
  if (event.evtype == PYPER_EVENT_TICK) :
    q = ipol2.Evaluate(event.timestamp);
    m = pfMatrix();
    m.makeQuat(q);
    node.setMat(m);
  if (event.evtype == PYPER_EVENT_TOUCH) :
    print node, " was touched!";




handlers = RegisteredHandlers();
handlers.AddHandler(dcs, myeventhandler, PYPER_EVENT_TICK);

ev = PyperTouchEvent();
handlers.GenerateEvent(ev, dcs);



start = times()[4];
overallstart = start;

framecount = 1000;

for i in range(framecount):
  pfFrame();
  chan.drawStats();
  ev = PyperTickEvent();
  handlers.GenerateTickEvents(ev);
  handlers.Update();
  if ((i+1)%100 == 0) :
    now = times()[4];
    delta = now - start;
    fps = 100.0 / delta;
    print fps, "fps";
    start = now;
  del ev;

now=times()[4];
delta = now - overallstart;
fps = framecount / delta;

print "overall fps has been: ", fps;

pfExit();

