
// This file contains the public interface from the Performer
// header file pr/pfString.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{

#include <Performer/pr/pfString.h>

%}

#define PFSTR_JUSTIFY		10
#define PFSTR_FIRST		10
#define PFSTR_MIDDLE		11
#define PFSTR_LAST		12
#define PFSTR_LEFT		PFSTR_FIRST
#define PFSTR_CENTER		PFSTR_MIDDLE
#define PFSTR_RIGHT		PFSTR_LAST

#define PFSTR_CHAR_SIZE		30
#define PFSTR_CHAR		1
#define PFSTR_SHORT		2
#define PFSTR_INT		4
#define PFSTR_AUTO_SPACING	40

class pfString
{
public:
    pfString();
    virtual ~pfString();

    static void	   init();
    static pfType* getClassType(); 

    size_t		getStringLength() const;
    void		setMode(int _mode, int _val);
    int	 		getMode(int _mode) const;
    void		setFont(pfFont* _fnt);
    pfFont*		getFont() const;
    void		setString(const char* _cstr);
    const char*		getString() const;
    const pfGeoSet*	getCharGSet(int _index) const;
    const pfVec3*     	getCharPos(int _index) const;
    void		setSpacingScale(float sx, float sy, float sz);
    void		getSpacingScale(float *sx, float *sy, float *sz) const;
    void		setGState(pfGeoState *gs);
    pfGeoState*	getGState(void) const;
    void		setColor(float r,float g, float b, float a);
    void		getColor(float *r, float *g, float *b, float *a) const;
    void		setBBox(const pfBox* newbox);
    const pfBox*	getBBox(void) const;
    void		setMat(const pfMatrix &_mat);
    void		getMat(pfMatrix &_mat) const;
    void		setIsectMask(uint _mask, int _setMode, int _bitOp);
    uint 		getIsectMask() const;

    void	draw();
    void	flatten();
    int		isect(pfSegSet *segSet, pfHit **hits[]);

};


