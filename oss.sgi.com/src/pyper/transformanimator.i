//
// $Source: /oss/CVS/cvs/performer/src/pyper/transformanimator.i,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
//

%{
#include "transformanimator.h"
%}


class TransformAnimator
{
  public:
    TransformAnimator(pfDCS &dcs);
    ~TransformAnimator();
    void UseSplines(bool v);

    void AddEvent(float time);
    void Evaluate(float time);

    float GetClosestKeyTime(float t) const;

    pfNode *GetTrajectoryGeometry(void) const;
    void ShowTrajectoryAsInverted(bool inv);
    void SetDerivativeScale(float s);
    void SetCircular(bool circ);
    void Clear(void);

    bool Store(const std::string &fname) const;
    bool Load(const std::string &fname);
    bool Import(const std::string &fname, float assumed_fps=60.0);
};

// $Log: transformanimator.i,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.6  2001/03/28 08:57:06  jorrit
// Update taken from Bram. Some methods added
//
// Revision 1.5  2001/02/27 10:10:31  bram
// added importer for saranav path files
//
// Revision 1.4  2001/02/26 07:54:47  bram
// fixed path recording
//
// Revision 1.3  2001/02/22 15:47:37  bram
// Improved transform animator.
// Added serializing for ani interpolators.
//
// Revision 1.2  2001/02/07 14:07:03  bram
// added trajectory
//
// Revision 1.1  2000/11/16 13:51:56  bram
// added transform animator
//
