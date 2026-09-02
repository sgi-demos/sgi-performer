# sed command file for Performer 2.1 API changes

# libpr name changes: PFTEXLOAD_ -> PFTLOAD_
# defines change from SRC -> SOURCE
# and DST -> DEST
# with new pfTexLoad object

s/PFTEXLOAD_SRC/PFTLOAD_SOURCE/g
s/PFTEXLOAD_DST/PFTLOAD_DEST/g
s/PFTEXLOAD_SRCTYPE/PFTLOAD_SOURCE/g
s/PFTEXLOAD_DSTTYPE/PFTLOAD_DEST/g
s/PFTEXLOAD_SRC_LEVEL/PFTLOAD_SOURCE_LEVEL/g
s/PFTEXLOAD_SRC_S/PFTLOAD_SOURCE_S/g
s/PFTEXLOAD_SRC_T/PFTLOAD_SOURCE_T/g
s/PFTEXLOAD_SRC_R/PFTLOAD_SOURCE_R/g
s/PFTEXLOAD_DST_LEVEL/PFTLOAD_DEST_LEVEL/g
s/PFTEXLOAD_DST_S/PFTLOAD_DEST_S/g
s/PFTEXLOAD_DST_T/PFTLOAD_DEST_T/g
s/PFTEXLOAD_DST_R/PFTLOAD_DEST_R/g
s/PFTEXLOAD_SRC_IMAGEARRAY/PFTLOAD_SOURCE_IMAGEARRAY/g
s/PFTEXLOAD_SRC_IMAGETILE/PFTLOAD_SOURCE_IMAGETILE/g
s/PFTEXLOAD_SRC_TEXTURE/PFTLOAD_SOURCE_TEXTURE/g
s/PFTEXLOAD_SRC_VIDEO/PFTLOAD_SOURCE_VIDEO/g
s/PFTEXLOAD_SRC_FRAMEBUFFER/PFTLOAD_SOURCE_FRAMEBUFFER/g
s/PFTEXLOAD_DST_TEXTURE/PFTLOAD_DEST_TEXTURE/g
s/PFTEXLOAD_SYNC_SRC/PFTLOAD_SYNC_SOURCE/g
s/PFTEXLOAD_DIRTY_SRCTYPE/PFTLOAD_DIRTY_SOURCETYPE/g
s/PFTEXLOAD_DIRTY_DSTTYPE/PFTLOAD_DIRTY_DESTTYPE/g
s/PFTEXLOAD_DIRTY_SRC/PFTLOAD_DIRTY_SOURCE/g
s/PFTEXLOAD_DIRTY_DST/PFTLOAD_DIRTY_DEST/g

s/PFTEXLOAD_/PFTLOAD_/g


s/3DCursor/2DCursor/g
s/PFU_CURSOR_3D/PFU_CURSOR_2D/g
s/pfuInitUtil/pfuInit/g
s/pfuFreeCPUs/pfuFreeAllCPUs/g


/pfdBuildASDMesh/i\
#error: PORT2.2  pfdBuildASDMesh() has been removed.  Use pfdBuildASD() or one of the libpfdb ASD loaders.

/pfdReadASDMesh/i\
#error: PORT2.2  pfdReadASDMesh() has been removed.  Use pfdLoadFile_evt().

/pfSubloadTexLevel/i\
#error: PORT2.2  pfSubloadTex[Level]()  - insert (-1) or width of source image for 6th argument.

s/pfNewState()/pfNewState(NULL)/g

s/ValidRegionOrigin/TexRegionOrg/g
s/ValidRegionSize/TexRegionSize/g

s/pfImageCacheOrigin/pfImageCacheMemRegionOrg/g
s/pfGetImageCacheOrigin/pfGetImageCacheMemRegionOrg/g
s/pfImageCacheSize/pfImageCacheMemRegionSize/g
s/pfGetImageCacheSize/pfGetImageCacheMemRegionSize/g

s/ValidRegionDstSize/TexSize/g
s/getValidRegionOffset/getTexRegionOffset/g
s/pfGetImageCacheValidRegionOffset/pfGetImageCacheTexRegionOffset/g
s/ValidRegionDst/Tex/g

/setVirtualSize/i\
#error: PORT2.2 pfImageCache::setVirtualSize() should be changed to pfImageCache::setImageSize

/getVirtualSize/i\
#error: PORT2.2 pfImageCache::getVirtualSize() should be changed to pfImageCache::getImageSize

/pfImageCacheVirtualSize/i\
#error: PORT2.2 pfImageCacheVirtualSize() should be changed to pfImageCacheImageSize

/pfGetImageCacheVirtualSize/i\
#error: PORT2.2 pfGetImageCacheVirtualSize() should be changed to pfGetImageCacheImageSize

/setCenter/i\
#error: PORT2.2 pfImageCache::setCenter() is obsolete. If this this an image cache member function, you should use setMemRegionOrg() with an offset instead.

/pfImageCacheCenter/i\
#error: PORT2.2 pfImageCacheCenter() is obsolete. If this this an image cache member function, you should use pfImageCacheMemRegionOrg() with an offset instead.

/pfGetImageCacheCenter/i\
#error: PORT2.2 pfGetImageCacheCenter() is obsolete. If this this an image cache member function, you should use pfGetImageCacheMemRegionOrg() then offset by half of size.

/setValidRegionCenter/i\
#error: PORT2.2 pfImageCache::setValidRegionCenter() is obsolete. If this this an image cache member function, you should use setTexRegionOrg() with an offset instead.

/getValidRegionCenter/i\
#error: PORT2.2 pfImageCache::setValidRegionCenter() is obsolete. If this this an image cache member function, you should use getTexRegionOrg() then offset by half of size.

/pfImageCacheValidRegionCenter/i\
#error: PORT2.2 pfImageCacheValidRegionCenter() is obsolete. If this this an image cache member function, you should use pfImageCacheTexRegionOrg() with an offset instead.

/pfGetImageCacheValidRegionCenter/i\
#error: PORT2.2 pfGetImageCacheValidRegionCenter() is obsolete. If this this an image cache member function, you should use pfGetImageCacheTexRegionOrg() then offset by half of size.

/setValidRegionOffset/i\
#error: PORT2.2 setValidRegionOffset() - no longer a valid call; offset is always computed by image cache.

/pfImageCacheValidRegionOffset/i\
#error: PORT2.2 setValidRegionOffset() - no longer a valid call; offset is always computed by image cache.

/pfImageCacheMaxUpdateSize/i\
#error: PORT2.2 pfImageCacheMaxUpdateSize() - is obsolete. please remove.
