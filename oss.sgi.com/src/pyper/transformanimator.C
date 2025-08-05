//
// $Source: /oss/CVS/cvs/performer/src/pyper/transformanimator.C,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
//

#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pf/pfDCS.h>

#include "transformanimator.h"   // Our own interface


// helper func

static void RecomputeBoundingBox(pfGeode *geode)
{
  int size = geode->getNumGSets();
  for (int i=0; i<size; i++)
  {
    pfGeoSet *set = geode->getGSet(i);
    set->setBound(0, PFBOUND_DYNAMIC);
  }
  geode->setBound(0, PFBOUND_DYNAMIC);
}




TransformAnimator::TransformAnimator(pfDCS &dcs) :
  TargetDCS(dcs),
  TrajectoryGeode(0),
  TrajectoryGeoSet(0),
  TrajectoryVertices(0),
  TrajectoryColors(0),
  TrajectoryIsInverted(0)
{
  TargetDCS.ref();
  CreateTrajectory();

  UseSplines(true);
}


TransformAnimator::~TransformAnimator()
{
  TargetDCS.unref();
}


void TransformAnimator::UseSplines(bool v)
{
  if (v)
    PosInterp.SetInterpolationType(INTERPOLATE_SPLINE);
  else
    PosInterp.SetInterpolationType(INTERPOLATE_LINEAR);
  OriInterp.SetInterpolationType(INTERPOLATE_LINEAR);
  SclInterp.SetInterpolationType(INTERPOLATE_LINEAR);
}


void TransformAnimator::AddEvent(float time)
{
  pfMatrix m;
  TargetDCS.getMat(m);

  pfQuat q;
  m.getOrthoQuat(q);

  pfVec3 p;
  m.getRow(3,p);

  pfVec3 x;
  m.getRow(0,x);

  float s = x.length();

  PosInterp.AddEvent(time, p);
  OriInterp.AddEvent(time, q);
  SclInterp.AddEvent(time, s);

  UpdateTrajectory();
}


void TransformAnimator::Evaluate(float time)
{
  pfVec3 p = PosInterp.Evaluate(time);
  pfQuat q = OriInterp.Evaluate(time);
  float s=1.0;
  if (SclInterp.GetEventCount())
   s = SclInterp.Evaluate(time);
  pfMatrix m;
  m.makeQuat(q);
  m.setRow(3,p);
  if (s!=1.0)
    m.preScale(s,s,s,m);
  TargetDCS.setMat(m);
}


void TransformAnimator::CreateTrajectory(void)
{
  void *arena = pfGetSharedArena();

  TrajectoryGeode = new pfGeode();
  TrajectoryGeode->setName("TrajectoryGeode");

  pfGeoState *geostate = new(arena) pfGeoState();
  geostate->setMode(PFSTATE_ENLIGHTING, 0);

#if 0
  pfMaterial *material = new pfMaterial();
  material->setColor(PFMTL_DIFFUSE,0.4,0.2,1);
  material->setColor(PFMTL_AMBIENT,0.4,0.2,1);
  geostate->setAttr(PFSTATE_FRONTMTL, material);
#endif

  int numlines = TRAJECTORY_SIZE/2;
  int numverts = TRAJECTORY_SIZE;
  int numindic = TRAJECTORY_SIZE;

  TrajectoryVertices = (pfVec3 *)pfMalloc(sizeof(pfVec3)*numverts, arena);
  ushort *indic = (ushort *)pfMalloc(sizeof(ushort)*numindic, arena);

  int writer=0;
  for (int i=0; i<numindic; i++)
  {
    TrajectoryVertices[writer] = pfVec3(-2+rand()%5,-2+rand()%5,-2+rand()%5);
    indic[writer] = writer;
    writer++;
  }

  TrajectoryGeoSet = new(arena) pfGeoSet;
  TrajectoryGeoSet->setPrimType(PFGS_LINES);
  TrajectoryGeoSet->setNumPrims(numlines);
  TrajectoryGeoSet->setAttr
  (
    PFGS_COORD3,
    PFGS_PER_VERTEX,
    TrajectoryVertices,
    indic
  );

  TrajectoryColors = (pfVec4 *)pfMalloc(sizeof(pfVec4)*numlines,  arena);
  ushort *cindic   =  (ushort *)pfMalloc(sizeof(ushort)*numlines, arena);
  for (int j=0; j<numlines; j++)
  {
    TrajectoryColors[j] = pfVec4(0.4,0.2,1.0,1.0);
    cindic[j] = j;
  }
  TrajectoryGeoSet->setAttr
  (
    PFGS_COLOR4, 
    PFGS_PER_PRIM, 
    TrajectoryColors, 
    cindic
  );

  TrajectoryGeoSet->setGState(geostate);
  TrajectoryGeode->addGSet(TrajectoryGeoSet);
}


void TransformAnimator::UpdateTrajectory(void)
{
  float min = PosInterp.GetMinTime();
  float max = PosInterp.GetMaxTime();
  float delta = max-min;
  if (delta==0)
    return;

  float step = delta / (TRAJECTORY_SIZE-1.0);

  int i;

  for (i=0; i<TRAJECTORY_SIZE; i++)
  {
    float t = min + i*step;
    pfVec3 pos = PosInterp.Evaluate(t);
    if (TrajectoryIsInverted)
    {
      pfQuat ori = OriInterp.Evaluate(t);
      float  scl = SclInterp.Evaluate(t);
      pfMatrix m;
      m.makeQuat(ori);
      m.setRow(3,pos);
      m.preScale(scl,scl,scl,m);
      m.invertAff(m);
      m.getRow(3, TrajectoryVertices[i]);
    }
    else
    {
      TrajectoryVertices[i] = pos;
    }
  }

  // Now update the colors: make the linesegments closest to a 
  // keyframe white.

  for (i=0; i<TRAJECTORY_SIZE/2; i++)
    TrajectoryColors[i] = pfVec4(0.4,0.2,1.0,1.0);

  int linecnt = TRAJECTORY_SIZE/2;
  for (i=0; i<PosInterp.GetEventCount(); i++)
  {
    float t = PosInterp.GetTimeAtEvent(i);
    float dt = t - min;
    if (dt > 1.00001 * delta)
    {
      printf("min=%f, t=%f, dt=%f, delta=%f\n",min,t,dt,delta);
      assert(dt <= delta);
    }
    int linenr = (int) ((dt / delta) * linecnt);
    if (linenr >= linecnt)
      linenr=linecnt-1;
    assert(linenr < linecnt);
    TrajectoryColors[linenr] = pfVec4(1,1,0.2,1);
  }

  RecomputeBoundingBox(TrajectoryGeode);
}


float TransformAnimator::GetClosestKeyTime(float t) const
{
  return PosInterp.GetClosestKeyTime(t);
}


pfNode *TransformAnimator::GetTrajectoryGeometry(void) const
{
  return TrajectoryGeode;
}


bool TransformAnimator::Store(const std::string &fname) const
{
  FILE *f = fopen(fname.c_str(), "wb");
  if (!f) return false;
  Store(f);
  fclose(f);
  return true;
}


void TransformAnimator::Store(FILE *f) const
{
  PosInterp.Store(f);
  OriInterp.Store(f);
  SclInterp.Store(f);
}


bool TransformAnimator::Load(const std::string &fname)
{
  FILE *f = fopen(fname.c_str(), "rb");
  if (!f)
  {
    fprintf(stderr,"File '%s' not found\n", fname.c_str());
    return false;
  }
  Load(f);
  fclose(f);
  UpdateTrajectory();
  return true;
}


void TransformAnimator::Load(FILE *f)
{
  PosInterp.Load(f);
  OriInterp.Load(f);
  SclInterp.Load(f);
}


void TransformAnimator::SetDerivativeScale(float scale)
{
  PosInterp.SetDerivativeScale(scale);
  OriInterp.SetDerivativeScale(scale);
  SclInterp.SetDerivativeScale(scale);
}

void TransformAnimator::SetCircular(bool circ)
{
  PosInterp.SetCircular(circ);
  OriInterp.SetCircular(circ);
  SclInterp.SetCircular(circ);
}


void TransformAnimator::Clear(void)
{
  PosInterp.Clear();
  OriInterp.Clear();
  SclInterp.Clear();
}


bool TransformAnimator::Import(const std::string &fname, float assumed_fps)
{
  FILE *f = fopen(fname.c_str(),"r");
  if (!f)
  {
    fprintf(stderr,"File '%s' not found\n", fname.c_str());
    return false;
  }

  Clear();

  bool quit=false;
  while (!quit)
  {
    float x,y,z,h,p,r;
    int frame;
    float timestamp;
    int mode;
    int retval = fscanf
    (
      f,
      " %f %f %f  %f %f %f  %d  %f  %d ",
      &x, &y, &z, 
      &h, &p, &r,
      &frame,
      &timestamp,
      &mode
    );
    if (retval == EOF)
    {
      quit = true;
    }
    else
    {
      assert(retval==9);
      pfMatrix m;
      m.makeEuler(h,p,r);
      pfQuat q;
      m.getOrthoQuat(q);
      pfVec3 p;
      p.set(x,y,z);
      // timestamps from saranav's path files are useless.
      // severe aliasing occures due to scientific notation.
      // so, we'll reconstruct one by assuming a steady 30 fps
      timestamp = frame / assumed_fps;
      PosInterp.AddEvent(timestamp, p);
      OriInterp.AddEvent(timestamp, q);
    }
  }

  fprintf
  (
    stderr,
    "Read %d keyframes from '%s'\n", 
    PosInterp.GetEventCount(), 
    fname.c_str()
  );

  fclose(f);
  UpdateTrajectory();
  return true;
}

