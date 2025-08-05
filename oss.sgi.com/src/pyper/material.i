
// This file contains the public interface from the Performer
// header file pr/pfMaterial.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{

#include <Performer/pr/pfMaterial.h>

%}

/* pfMaterial Sides */
#define PFMTL_FRONT             0x1
#define PFMTL_BACK              0x2
#define PFMTL_BOTH              0x3

class pfMaterial
{
public:
    %addmethods
    {

      char *__str__()
      {
      	static char temp[512];
	static float ar,ag,ab, dr,dg,db, er,eg,eb, sr,sg,sb;
	self->getColor(PFMTL_AMBIENT ,&ar,&ag,&ab);	
	self->getColor(PFMTL_DIFFUSE ,&dr,&dg,&db);
	self->getColor(PFMTL_EMISSION,&er,&eg,&eb);
	self->getColor(PFMTL_SPECULAR,&sr,&sg,&sb);

      	sprintf
      	(
	  temp,
	  "pf Material:\n"
	  "alpha %1.2f , shininess %1.2f \n"
	  "ambient  rgb [%1.2f,%1.2f,%1.2f]\n"
	  "diffuse  rgb [%1.2f,%1.2f,%1.2f]\n"
	  "emission rgb [%1.2f,%1.2f,%1.2f]\n"
	  "specular rgb [%1.2f,%1.2f,%1.2f]\n",
	  self->getAlpha(),self->getShininess(),
	  ar,ag,ab,
	  dr,dg,db,
	  er,eg,eb,
	  sr,sg,sb
        );
        return temp;
      }

    }

    pfMaterial();
    ~pfMaterial();

    // sets and gets
    void	setSide(int _side);
    int 	getSide();
    
    void	setAlpha(float _alpha);
    float	getAlpha();

    void	setShininess(float _shininess);
    float	getShininess();

    void	setColor(int _acolor, float _r, float _g, float _b);
    void	getColor(int _acolor, float* _r, float* _g, float* _b);

    void	setColorMode(int _side, int _mode);
    int		getColorMode(int _side);
    
};









