
// This file contains the public interface from the Performer
// header file pf/pfGeoState.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pr/pfGeoState.h>
%}

class pfGeoState
{
public:
    %addmethods
    {
      char *__str__()
      {
        return "pfGeoState";
      }
      pfMaterial* getFrontMaterial()
      {
	return((pfMaterial*)(self->getAttr(PFSTATE_FRONTMTL)));
      }
      void setFrontMaterial(pfMaterial* mtl)
      {
	self->setAttr(PFSTATE_FRONTMTL,(void*)(mtl));
      }
      pfMaterial* getBackMaterial()
      {
	return((pfMaterial*)(self->getAttr(PFSTATE_BACKMTL)));
      }
      void setBackMaterial(pfMaterial* mtl)
      {
	self->setAttr(PFSTATE_BACKMTL,(void*)(mtl));
      }
    }

    pfGeoState();
    virtual ~pfGeoState();

    static pfType* getClassType();

    void	setMode(int _attr, int _a);
    int		getMode(int _attr) const;
    int		getCurMode(int _attr) const;
    int		getCombinedMode(int _attr, const pfGeoState *_combState) const;
    void	setVal(int _attr, float _a);
    float	getVal(int _attr) const;
    float	getCurVal(int _attr) const;
    float	getCombinedVal(int _attr, const pfGeoState *_combState) const;
    void	setInherit(unsigned int _mask);
    unsigned int getInherit() const;
    void	setAttr(int _attr, void* _a);
    void*	getAttr(int _attr) const;
    void*	getCurAttr(int _attr) const;
    void*	getCombinedAttr(int _attr, const pfGeoState *_combState) const;
    void	setFuncs(pfGStateFuncType _preFunc, pfGStateFuncType _postFunc, void *_data);    
    void	getFuncs(pfGStateFuncType *_preFunc, pfGStateFuncType *_postFunc, void **_data) const;
    void	load();
    void	apply();
    void	makeBasic();

};
     



