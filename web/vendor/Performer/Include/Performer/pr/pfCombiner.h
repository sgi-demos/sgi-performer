#ifndef __PF_COMBINER_H__
#define __PF_COMBINER_H__

/*
	The preprocessor directives below make sure that this header
	file is not if the user has disabled the C++ API.
*/
#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

/*
	The pfObject header file is pulled from a different place when
	building the library than when compiling against it.
*/
#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfDispList.h>
#include <Performer/pr/pfLinMath.h>

class DLLEXPORT pfPassList;

#define PFCOMBINER ((pfCombiner*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFCOMBINERBUFFER ((pfCombiner*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfCombiner : public pfObject
{
public:
	// CAPI:basename Combiner

	// Constructors and destructors
	pfCombiner();
	virtual ~pfCombiner();

	static void init();
	static pfType *getClassType() { return classType_; }

	// CAPI:verb
	void apply(void);

	void setGeneralInput(GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);

	void setGeneralOutput(GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum);

	void setFinalInput(GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);

	/* Set the number of active general combiners. */
	void setActiveCombiners(int count);

	/* Set the number of active constant colors. */
	void setActiveConstColors(int count);

	/*
		Set the constant colors either on a per-stage basis
		or globally.
	*/

	// CAPI:verb CombinerConstColor0
	void setConstantColor0(const pfVec4 color);
	// CAPI:verb CombinerConstColor1
	void setConstantColor1(const pfVec4 color);

	// CAPI:verb CombinerGeneralConstColor0
	void setGeneralConstantColor0(GLenum stage, const pfVec4 color);
	// CAPI:verb CombinerGeneralConstColor1
	void setGeneralConstantColor1(GLenum stage, const pfVec4 color);

	// CAPI:verb CombinerFinalConstColor0
	void setFinalConstantColor0(const pfVec4 color);
	// CAPI:verb CombinerFinalConstColor1
	void setFinalConstantColor1(const pfVec4 color);

	// CAPI:virtual
	virtual int print(uint _travMode, uint _verbose, char *prefix, FILE *file);

	// CAPI:verb GetMaxGeneralCombiners
	int getMaxGeneralCombiners(void);
private:
	// immediate mode apply
	friend class pfDispList;
	void pr_imApply(void);

	void applyConstantColors(void);

	// display list mode apply
	void pr_dlApply(pfDispList *dl);

	// class type
	static pfType *classType_;

	int	hasCombiners;

	/* The maximum and active number of general register combiners. */
	GLint maxGeneralCombiners;
	GLint activeGeneralCombiners;
	GLint activeConstantColors;

	/* Declare the state for general register combiners. */
	struct grcstate
	{
		GLenum		input[4];
		GLenum		mapping[4];
		GLenum		componentUsage[4];

		/*
			outScale: {SCALE_BY_ONE_HALF_NV, NONE,
				SCALE_BY_TWO_NV, SCALE_BY_FOUR_NV}

			outBias: {NONE, BIAS_BY_NEGATIVE_ONE_HALF_NV}
		*/
		GLenum		outScale, outBias;

		/*
			These variables can be any of PRIMARY_COLOR_NV,
			SECONDARY_COLOR_NV, SPARE0_NV, SPARE1_NV,
			TEXTURE<i>_ARB, or DISCARD_NV.
		*/
		GLenum		abOutput, cdOutput, sumOutput;

		GLboolean	abDotProduct, cdDotProduct, muxSum;
	};

	struct frcstate
	{
		GLenum	input[7];
		GLenum	mapping[7];
		GLenum	componentUsage[7];
	};

	struct grcstate	*grc, *grcalpha;
	struct frcstate	frc;

	pfVec4	constantColor0, constantColor1;

	void *placeHolder_; //Place holder for data we may need in the future.
};

#endif //__PF_COMBINER_H__
