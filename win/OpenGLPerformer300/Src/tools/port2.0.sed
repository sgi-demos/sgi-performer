s,"/usr/src/Performer/include/\(.*\)",<Performer/\1>,

s/pfSegsIsectNode/pfNodeIsectSegs/g

s/pfNewDPool/pfCreateDPool/g
s/pfGetMallocSize/pfGetSize/g
s/pfGetMallocArena/pfGetArena/g

s/pfPtInHalfSpace/pfHalfSpaceContainsPt/g
s/pfPtInBox/pfBoxContainsPt/g
s/pfPtInCyl/pfCylContainsPt/g
s/pfPtInSphere/pfSphereContainsPt/g
s/pfPtInFrust/pfFrustContainsPt/g

s/pfBoxIsectHalfSpace/pfHalfSpaceContainsBox/g
s/pfSphereIsectHalfSpace/pfHalfSpaceContainsSphere/g
s/pfCylIsectHalfSpace/pfHalfSpaceContainsCyl/g

s/pfBoxIsectFrust/pfFrustContainsBox/g
s/pfSphereIsectFrust/pfFrustContainsSphere/g
s/pfCylIsectFrust/pfFrustContainsCyl/g

s/pfBoxIsectBox/pfBoxContainsBox/g
s/pfBoxExtendBox/pfBoxExtendByBox/g
s/PFBOX_EXTEND_BOX/PFBOX_EXTENDBY_BOX/g
s/pfPtExtendBox/pfBoxExtendByPt/g

s/pfSphereIsectSphere/pfSphereContainsSphere/g
s/pfCylIsectSphere/pfSphereContainsCyl/g
s/pfSphereExtendSphere/pfSphereExtendBySphere/g
s/pfCylExtendSphere/pfSphereExtendByCyl/g
s/pfBoxExtendSphere/pfSphereExtendByBox/g

s/pfSegIsectTri/pfTriIsectSeg/g
s/pfSegIsectSphere/pfSphereIsectSeg/g
s/pfSegIsectBox/pfBoxIsectSeg/g
s/pfSegIsectPlane/pfPlaneIsectSeg/g
s/pfSegIsectHalfSpace/pfHalfSpaceIsectSeg/g

s/pfInvertMat/pfInvertFullMat/g
s/pfXformPt3/pfFullXformPt3/g
s,pfMakeRotOntoMat,/* XXX PF2.0 input vectors must already be normalized */ pfMakeVecRotVecMatXXX,g

s/pfGetEnableStatsHw/pfEnableStatsHw/g

s/pfSelectClock/pfClockName/g
s/pfClosestPtsOnSegs/pfClosestPtsOnSeg/g
s/pfOffsetPlane/pfDisplacePlane/g
s/pfGetHyperId/pfGetPipeHyperId/g

# libpfutil name changes: libpfutil->libpfu and builder->geobuilder
s/pfuAddLine/pfdAddLine/g
s/pfuAddPoly/pfdAddPoly/g
s/pfuAddTri/pfdAddTri/g
s/pfuBreakup/pfdBreakup/g
s/pfuBuilderMode/pfdGeoBldrMode/g
s/pfuCursorSel/pfuGUICursorSel/g
s/pfuDelBuilder/pfdDelGeoBldr/g
s/pfuGetBuilderMode/pfdGetGeoBldrMode/g
s/pfuGetCursorSel/pfuGetGUICursorSel/g
s/pfuGetMesherMode/pfdGetMesherMode/g
s/pfuGetNumTris/pfdGetNumTris/g
s/pfuIsFlyboxActive/pfuGetFlyboxActive/g
s/pfuIsWidgetDefaultOnOff/pfuWidgetDefaultOnOff/g
s/pfuMakeGSets/pfdBuildGSets/g
s/pfuMeshGSet/pfdMeshGSet/g
s/pfuMesherMode/pfdMesherMode/g
s/pfuNewBuilder/pfdNewGeoBldri/g
s/pfuPostDrawReflMap/pfdPostDrawReflMap/g
s/pfuPreDrawReflMap/pfdPreDrawReflMap/g
s/pfuTriangulatePoly/pfdTriangulatePoly/g
s/pfuWidgetFunc/pfuWidgetActionFunc/g
s/pfuBuilder/pfdGeoBuilder/g

s/pfuEvent/pfuEventStream/g

# name changes: pfuXformer -> pfiXformer

s/pfiXformer/pfuXformer/g

s/PFUXF_TRACKBALL/PFIXF_TRACKBALL/g
s/PFUXF_FLY/PFIXF_FLY/g
s/PFUXF_DRIVE/PFIXF_DRIVE/g

s/pfuNewXformer/pfiNewXformer/g
s/pfuXformerMode/pfiXformerMode/g
s/pfuGetXformerMode/pfiGetXformerMode/g
s/pfuStopXformer/pfiStopXformer/g
s/pfuXformerAutoInput/pfiXformerAutoInput/g
s/pfuXformerMat/pfiXformerMat/g
s/pfuGetXformerMat/pfiGetXformerMat/g
s/pfuXformerCoord/pfiXformerCoord/g
s/pfuGetXformerCoord/pfiGetXformerCoord/g
s/pfuXformerLimits/pfiXformerLimits/g
s/pfuGetXformerLimits/pfiGetXformerLimits/g
s/pfuXformerCollision/pfiXformerCollision/g
s/pfuGetXformerCollisionStatus/pfiGetXformerCollisionStatus/g
s/pfuUpdateXformer/pfiUpdateXformer/g
s/pfuCollideXformer/pfiCollideXformer/g

# Looks like we've changed this call back to it's original name
#s/pfGetHyperId/pfGetPipeHyperId/g

# Must alter pfLightColor here so that m4 can change this and not go
# infinitely recursive.  See port2.0.m4.
s/pfLightColor/pfFooLightColor/g
s/pfGetLightColor/pfFooGetLightColor/g

s/pfuSaveImage/pfuFooSaveImage/g

s/pfGSetBBox/pfFooGSetBBox/g

# hints for pfPipe and Windowing changes

/pfInitPipe/i\
#error: PORT2.0  pfInitPipe() has been split into pfConfigStage() and pfConfigPWin(). \\\
    - Stage and process initialization is done in the pfStageConfigFunc() callback. \\\
    - Window initialization is now done in the pfPWinConfigFunc() callback.

/winopen()/i\
#error: PORT2.0 Windows can be opened via a pfPipeWindow or given to a pfPipeWindow \\\
    - via pfPWinWSDrawable() in a pfPWinConfigFunc() callback.

/pfGetPipeOrigin/i\
#error: PORT2.0  pfGetPipeOrigin() has been removed. Use pfGetPWinOrigin(pfPipeWindow*).

/pfGetPipeSize/i\
#error: PORT2.0  pfGetPipeSize() now returns size of the pipe screen. Use pfGetPWinSize(pfPipeWindow*).

/pfGetPipeGLXWins/i\
#error: PORT2.0  pfGetPipeGLXWins() has been removed. \\\
    - Windows can be opened via a pfPipeWindow or given to a pfPipeWindow \\\
    - via pfPWinWSDrawable() in a pfPWinConfigFunc() callback.

/pfGetPipeWin/i\
#error: PORT2.0  pfGetPipeGLXWins() has been removed. Use pfPipePWin(pfPipe*).

/pfInitGLXGfx/i\
#error: PORT2.0  pfInitGLXGfx() has been removed. Use pfInitGfx().

/pfuPipeGLXMSConfig/i\
#error: PORT2.0 pfuPipeGLXMSConfig() has been removed. \\\
    - Use pfChoosePWinFBConfig() instead.
    
/pfuPipeGLXWin/i\
#error: PORT2.0 pfuPipeGLXWin() has been removed because it is no longer needed.

/pfuInitInput/i\
#error: PORT2.0 pfuInitInput() now takes a pfPipeWindow* for its first argument.
