#ifndef _PF_ISL_HEADER
#define _PF_ISL_HEADER

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfByteBank.h>
#include <Performer/pr.h>
#include <Performer/pr/pfType.h>
struct _pfTexCoordSub;
/*
  This class is passed into the user's callback for generating texture
  coordinates for pfGeoSets that are requested by an OpenGL|Shader
  texture coordinate code
*/

#define PFISLTEXCOORDDATA ((pfISLTexCoordData*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFISLTEXCOORDDATABUFFER ((pfISLTexCoordData*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfISLTexCoordData : public pfObject {
 public:
  pfISLTexCoordData();
  ~pfISLTexCoordData();
  
  /* Performer type checking functions */
  static void	   init();
  static pfType* getClassType() { return classType; }

  /* This function returns a pointer to temporary memory that is 
     only valid for the current frame. Once the frame is drawn, 
     it disappears
  */
  void *borrowMemory(int size);
  
  /*
    This function returns the appearance currently being
    rendered.
  */
  islAppearance* getAppearance();
  
  /*
    This function returns the geoset that is currently being shaded 
  */
  pfGeoSet* getGSet();
  /*
    This function returns the current modelview matrix
  */
  pfMatrix* getModelview();
  /*
    This function returns the current viewing matrix
  */
  pfMatrix *getViewmat();
 PFINTERNAL:
  //CAPI:private
  void setAppearance(islAppearance *app);
  void setGSet(pfGeoSet *gset);
  void setModelview(pfMatrix *mat);
  void setViewmat(pfMatrix *mat);
  void setByteBank(pfByteBank *bb);
  void reset();
  
 private:
  static pfType *classType;
  islAppearance *appearance_;
  pfGeoSet      *gset_;
  pfMatrix      *modelView_;
  pfMatrix      *viewMat_;
  pfByteBank    *bb_;
};
  
#endif
  
