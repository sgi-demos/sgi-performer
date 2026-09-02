divert(-1)

#define
#divert
#dnl

undefine(`include')
undefine(`changecom')
undefine(`decr')
undefine(`defn')
undefine(`divnum')
undefine(`dumpdef')
undefine(`errprint')
undefine(`eval')
undefine(`ifdef')
undefine(`ifelse')
undefine(`include')
undefine(`incr')
undefine(`index')
undefine(`len')
undefine(`m4exit')
undefine(`m4wrap')
undefine(`maketemp')
undefine(`popdef')
undefine(`pushdef')
undefine(`shift')
undefine(`sinclude')
undefine(`substr')
undefine(`syscmd')
undefine(`sysval')
undefine(`traceoff')
undefine(`traceon')
undefine(`translit')
undefine(`undivert')

#
# m4 definitions to rewrite 1.2 programs as 2.0 programs
#


#define(pfuGLXMapcolor, 
#/* 
# * PORT: argument changed from R, G, B to pfVec3 
# *
# * 1.2 API: void pfu+GLXMapcolor(pfuXDisplay *dsp, pfuXWindow w, long loc, float r, float g, float b);
# * 2.0 API: void pfu+GLXMapcolors(pfuXDisplay *dsp, pfuXWindow w, pfVec3 *clrs, int loc, int num);
# */
#do  { 
#    pfVec3 _temp; 
#    pfSetVec3(_temp, $4, $5, $6); 
#    pfu+GLXMapcolors($1, $2, _temp, $3, 1); 
#} while(0))

#
# change name and argument order
#
define(pfuGetPanelSize,pfuGetPanelOriginSize($1, $2, $4, $3, $5))

#define(pfuGetWidgetSize,
#/* 
# * PORT: function now returns both origin and size
# *
# * 1.2 API: long pfuGetWidgetSize(pfuWidget *_w, long *_xs, long *_ys);
# * 2.0 API: void pfuGetWidgetDim(pfuWidget *_w, int *_xo, int *_yo, int *_xs, int *_ys);
# */
#do  { 
#    float _xo;
#    float _y0;
#    pfuGetWidgetDim($1, &_xo, &_yo, $2, $3); 
#} while(0))

#
# call general function with class specified in argument
#
define(pfFindBboard, (pfBillboard*)pfLookupNode($1, pfGetBboardClassType()))
define(pfFindDCS, (pfDCS*)pfLookupNode($1, pfGetDCSClassType()))
define(pfFindGeode, (pfGeode*)pfLookupNodeXXX($1, pfGetGeodeClassType() /* XXX PF2.0 pfLookup() includes subclasses, e.g. pfBillboard */))
define(pfFindGroup, (pfGroup*)pfLookupNodeXXX($1, pfGetGroupClassType() /* XXX PF2.0 pfLookup() includes subclasses, e.g. pfLOD */))
define(pfFindLOD, (pfLOD*)pfLookupNode($1, pfGetLODClassType()))
define(pfFindLPoint, (pfLightPoint*)pfLookupNode($1, pfGetLPointClassType()))
define(pfFindLSource, (pfLightSource*)pfLookupNode($1, pfGetLSourceClassType()))
define(pfFindLayer, (pfLayer*)pfLookupNode($1, pfGetLayerClassType()))
define(pfFindPart, (pfPartion*)pfLookupNode($1, pfGetPartClassType()))
define(pfFindSCS, (pfSCS*)pfLookupNodeXXX($1, pfGetSCSClassType() /* XXX PF2.0 pfLookup() includes subclasses, e.g. pfDCS */))
define(pfFindScene, (pfScene*)pfLookupNode($1, pfGetSceneClassType()))
define(pfFindSeq, (pfSequence*)pfLookupNode($1, pfGetSeqClassType()))
define(pfFindSwitch, (pfSwitch*)pfLookupNode($1, pfGetSwitchClassType()))

#define(PF_UNSUPPORTED,
#/* 
# * PORT: this function is no longer supported
# */
#$1_NO_LONGER_SUPPORTED)

#define(pfuCollideEllipse, PF_UNSUPPORTED($0))
#define(pfuCollideSweep, PF_UNSUPPORTED($0))

#
# change name: a OPs b -> b OPedBy a
#
define(pfBoxExtendBox, pfBoxExtendByBox($1, $2))
define(pfCylExtendSphere, pfSphereExtendByCyl($1, $2))
define(pfPtExtendBox, pfBoxExtendByPt($1, $2))
define(pfPtExtendSphere, pfSphereExtendByPt($1, $2))
define(pfSphereExtendSphere, pfSphereExtendBySphere($1, $2))

#
# change name and argument order: a OPs b -> b OPedBy a
#
define(pfBoxIsectBox, pfBoxContainsBox($2, $1))
define(pfBoxIsectFrust, pfFrustContainsBox($2, $1))
define(pfBoxIsectHalfSpace, pfHalfSpaceContainsBox($2, $1))
define(pfCylIsectFrust, pfFrustContainsCyl($2, $1))
define(pfCylIsectHalfSpace, pfHalfSpaceContainsCyl($2, $1))
define(pfCylIsectSphere, pfSphereContainsCyl($2, $1))
define(pfPtInBox, pfBoxContainsPt($2, $1))
define(pfPtInCyl, pfCylContainsPt($2, $1))
define(pfPtInFrust, pfFrustContainsPt($2, $1))
define(pfPtInHalfSpace, pfHalfSpaceContainsPt($2, $1))
define(pfPtInSphere, pfSphereContainsPt($2, $1))
define(pfSphereIsectFrust, pfFrustContainsCyl($2, $1))
define(pfSphereIsectHalfSpace, pfHalfSpaceContainsSphere($2, $1))
define(pfSphereIsectSphere, pfSphereContainsSphere($2, $1))
define(pfSegIsectBox, pfBoxIsectSeg($2, $1, $3, $4))
define(pfSegIsectHalfSpace, pfHalfSpaceIsectSeg($2, $1, $3, $4))
define(pfSegIsectPlane, pfPlaneIsectSeg($2, $1, $3))
define(pfSegIsectSphere, pfSphereIsectSeg($2, $1, $3, $4))
define(pfSegIsectTri, pfTriIsectSeg($2, $3, $4, $1, $5))
define(pfSegsIsectGSet, pfGSetIsectSegs($2, $1, $3))
define(pfSegsIsectNode, pfNodeIsectSegs($2, $1, $3))

define(pfChanCullFunc, pfChanTravFunc($1, PFTRAV_CULL, $2))
define(pfChanDrawFunc, pfChanTravFunc($1, PFTRAV_DRAW, $2))
define(pfGetChanCullFunc, pfGetChanTravFunc($1, PFTRAV_CULL))
define(pfGetChanDrawFunc, pfGetChanTravFunc($1, PFTRAV_DRAW))

# Substitute for "foo" functions because name of Func hasn't changed; a
# direct substitution using the same name would have made m4 loop.
# See port2.0.sed.
define(pfFooLightColor, pfLightColor($1, PFLT_DIFFUSE, $2, $3, $4))
define(pfFooGetLightColor, pfGetLightColor($1, PFLT_DIFFUSE, $2, $3, $4))
define(pfLightAmbient, pfLightColor($1, PFLT_AMBIENT, $2, $3, $4))
define(pfGetLightAmbient, pfGetLightColor($1, PFLT_AMBIENT, $2, $3, $4))

define(pfQueryStats, prQueryStats($1, $2, $3, 0 /* size of dest buffer */))
define(pfMQueryStats, prQueryStats($1, $2, $3, 0 /* size of dest buffer */))

define(pfuFooSaveImage, pfuSaveImage($1, $2, $3, $4, $5, 0))

define(pfFooGSetBBox, pfGSetBBox($1, $2, PFBOUND_DYNAMIC))

# fix possible later munchings of "define"
undefine(`define')

# fix possible later munchings of "`'"
changequote(@&$!{,}@&$)
# must use @&$!{ and }@&$! to quote strings through rest of script.

# don't munch a variable called "changequote"
undefine(@&$!{changequote}@&$!)

# don't munch a string including "undefine"
undefine(@&$!{undefine}@&$!)

divert(0)dnl
