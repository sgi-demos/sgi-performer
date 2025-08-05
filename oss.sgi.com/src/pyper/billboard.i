// This file contains the public interface from the Performer
// header file pf/pfBillboard.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pf/pfBillboard.h>

%}

/* these defines are ripped from the <Performer/pr/pfSprite.h> 
 * and needed for the billboard modes - daniel
 */

#define PFSPRITE_ROT                    0

#define PFSPRITE_AXIAL_ROT              0
#define PFSPRITE_POINT_ROT_EYE          1
#define PFSPRITE_POINT_ROT_WORLD        2

/* 
 *  ended ripped part 
 */


/* Modes */
#define PFBB_ROT                PFSPRITE_ROT

/* Rotation values */
#define PFBB_AXIAL_ROT          PFSPRITE_AXIAL_ROT              
#define PFBB_POINT_ROT_EYE      PFSPRITE_POINT_ROT_EYE
#define PFBB_POINT_ROT_WORLD    PFSPRITE_POINT_ROT_WORLD        


class pfBillboard : public pfGeode
{
public:

/*
    %addmethods
    {
      char *__str__()
      {
        static char temp[256];
        const char *name = self->getName();
        int cnt = self->getNumGSets();
        if (name)
          sprintf(temp, "pfBillboard named '%s' with %d geosets", name, cnt);
        else
          sprintf(temp, "unnamed pfBillboard with %d geosets", cnt);
        return temp;
      }
    }
*/
    void setAxis(const pfVec3& axis);
    void getAxis(pfVec3& axis);

    void setMode(int  _mode, int  _val);
    int getMode(int  _mode);

    void setPos(int  i, const pfVec3& pos);
    void getPos(int  i, pfVec3& pos);
    void setPosFlux(pfFlux *_flux);
    pfFlux* getPosFlux();

    pfBillboard();
    virtual ~pfBillboard();
    static pfType* getClassType();
};







