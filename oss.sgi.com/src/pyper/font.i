
// This file contains the public interface from the Performer
// header file pf/pfFont.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.


%{

#include <Performer/pr/pfFont.h>

%}

#define	PFFONT_CHAR_SPACING	0
#define PFFONT_NUM_CHARS	1
#define PFFONT_RETURN_CHAR	2

#define PFFONT_CHAR_SPACING_FIXED	0
#define PFFONT_CHAR_SPACING_VARIABLE	1
#define PFFONT_ASCII		128

#define PFFONT_UNIT_SCALE	0

#define PFFONT_GSTATE		0
#define PFFONT_BBOX		1
#define PFFONT_SPACING		2
#define PFFONT_NAME		3


class pfFont
{
public:
    pfFont();
    virtual ~pfFont();

    static void		init();
    static pfType*	getClassType();
    static pfGeoState*	defaultGState;

    void	setCharGSet(int _ascii, pfGeoSet *_gset);
    pfGeoSet*	getCharGSet(int _ascii);
    void	setCharSpacing(int _ascii, pfVec3 &_spacing);
    const pfVec3*	getCharSpacing(int _ascii);
    void	setAttr(int _which, void *attr);
    void*	getAttr(int _which);
    void	setVal(int _which, float _val);
    float	getVal(int _which);
    void	setMode(int mode, int val);
    int		getMode(int mode);

};

