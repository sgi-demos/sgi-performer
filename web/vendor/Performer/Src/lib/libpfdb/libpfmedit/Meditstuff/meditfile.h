/*--------------------------Message start-------------------------------

This software is copyright (C) 1994-1995 by Medit Productions


This software is provided under the following terms, if you do not
agree to all of these terms, delete this software now. Any use of
this software indicates the acceptance of these terms by the person
making said usage ("The User").

1) Medit Productions provides this software "as is" and without
warranty of any kind. All consequences of the use of this software
fall completely on The User.

2) This software may be copied without restriction, provided that an
unaltered and clearly visible copy of this message is placed in any
copies of, or derivatives of, this software.


---------------------------Message end--------------------------------*/




/************************************************************************
Tokens in Medit files...
************************************************************************/

enum {
BlockEnd=0,
BinaryFile,
AsciiFile,

FMaterials=10,
FMat,
FMAmbient,
FMDiffuse,
FMSpecular,
FMEmissive,
FMShine,
FMAlpha,
FMName,

FVertex = 20,
FVConstruction,
FVCoord,
FVTexture,
FVNormal,
FVColour,
FVList,
FVIndex,

FPolygon = 30,
FPMaterial,
FPLighting,
FPNormal,
FPTexture,
FTMaterial,
FPTextureMat,
FPLightGroup,
FPFlags,

FObject = 40,
FSubObject,
FLod,
FSwitchOut,
FObjectId,
FSubObjectId,
FGridIndex,
FGridSteps,
FObjectAttributes,
FBeadVisible,

FScenePush = 50,
FScenePop,
FSceneBranch,
FSceneObject,
FSceneInstance,
FSceneMatrix,
FSceneVisible,
FSceneFolded,

FData = 60,
FTexture,
FTextureDir,

FView = 70,
FViewThreeDee,
FViewAxis,
FViewViewMatrix,
FViewProjectMatrix,
FViewGridMatrix,
FViewMagnification,
FViewCOG,

FTextureRef = 80,
FMinFilter,
FMagFilter,
FFastTex,
FOldClamp,
FClampTexX,
FClampTexY,

FNextOne = 100,		/* Leave lots of space for texture attributes... */

LastToken
};

/* Types of data item */
enum {
ExtinctFileVar0,
ExtinctFileVar1,
ExtinctFileVar2,
DCurrentObject,
ExtinctFileVar3,
ExtinctFileVar4,
DSceneGridIndex,
DSceneGridSteps,
DNoViewports,
DObjectIsolated,
DFileCreateId,
DFilePublicDomain,
DFileTitle,
DFileAuthor,
DFileCompany,
DFileContact,
DFileComment1,
DFileComment2,
DFileComment3,
DFileComment4,
DOriginalFilePath,
DHideSubObjects,
DHideScene

};
