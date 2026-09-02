//
// pfTextureValidMap.h
//

#ifndef __PF_TEXTURE_VALID_MAP_H__
#define __PF_TEXTURE_VALID_MAP_H__

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <Performer/pr.h>
#include <Performer/pr/pfObject.h>

#define _PFBITS(A)		(sizeof(A) * 8)
#define _PFBIT(A,i)	(((A)[(i)/_PFBITS(*(A))] >> ((i)%_PFBITS(*(A)))) & 1)
#define _PFHOWMANY(x,mod)	(((x)+((mod)-1))/(mod))
#define _PFROUNDUP(x,mod)	(((x)+((mod)-1))/(mod)*(mod))
#define _PFROUNDDOWN(x,mod)	((x)/(mod)*(mod))
#define _PFISPOWEROF2(x) (((x)&((x)-1))==0)
#define _PFINORDER5(a,b,c,d,e) ((a)<=(b)&&(b)<=(c)&&(c)<=(d)&&(d)<=(e)&&(e)<=(f))

class DLLEXPORT pfTextureValidMapLevel {
public:
    //CAPI:private
    //
    // Read-only constants specified by the constructor...
    //  All of these are in texels.
    //  All of these must be powers of 2, and
    //       1 <= quantumS <= clipSizeS <= mapSizeS <= virtSizeS
    //       1 <= quantumT <= clipSizeT <= mapSizeT <= virtSizeT
    //
    int virtSizeS, virtSizeT;	// size of virtual image in texels
    int mapSizeS, mapSizeT;	// size of represented area in texels
    int clipSizeS, clipSizeT;	// clip size for this level in texels
    int quantumS, quantumT;	// each bit in the array represents this
				// number of texels.  All tex loads must
				// be in multiples of this.
    //
    // Constants derived from the above constants,
    // for quick (and correct even if lhs is negative)
    // div and mod operations
    // 
    int virtSizeMaskS, virtSizeMaskT;	// virt size minus 1
    int mapSizeMaskS, mapSizeMaskT;	// map size minus 1
    int logQuantumS, logQuantumT;	// log base 2 of quantum

    //
    // Contents of the map... each 1 in the array is
    // a quantum of texels that is known to be valid
    // (where "valid" simply means that someone has called the
    // recordTexLoad() member function with an area including that quantum,
    // and there have been no
    // subsequent recordTexLoads() of quanta with the same remainder
    // mod the clip size).
    //
    // Note that the map cannot simultaneously store valid bits
    // for two quanta that are more than mapSize texels apart,
    // so the map can "lose track" of valid data if the map region
    // moves off and then back on to a quantum.
    // (Every call to recordTexLoad() causes the map region to be automatically
    // adjusted by the smallest possible delta so that it encloses the
    // specified load region.)
    //
    // But this is not a problem, since no known texture load scheduling
    // algorithms expect texels that are much more than the clip size
    // apart to be simultaneously valid.
    // In fact, as of this writing, Performer won't expect texels
    // more than *exactly* the clip size apart to be simultaneously valid,
    // so for validity checking, the map size could be
    // limited to exactly the clip size.
    // But for debugging (in the case that an area that should have been
    // texloaded got missed) it is often good to know which texels
    // of the surrounding area are valid.
    // Therefore a good choice for the map size is twice the clip size.
    //
    int *bitarray;

    //
    // Variables...
    //	  The area represented by the map at any given time
    //    is always (in texels from the lower-left corner of the entire
    //    virtual image):
    //		[mapOrigin .. mapOrigin+mapSize]
    //	  All of these values are always multiples of quantum.
    //
    int mapOriginS, mapOriginT;
    int tracing; // if set, print every tex load

    //
    // Constructors and destructors and stuff...
    // construct and destruct member functions
    // are provided so that arrays of objects
    // can be created and the elements constructed individually
    // (or destructed independent of the memory deallocation).
    //
    pfTextureValidMapLevel(int virtSizeS, int virtSizeT,
		      int mapSizeS, int mapSizeT,
		      int clipSizeS, int clipSizeT,
		      int quantumS, int quantumT,
		      void *arena)
    {
	construct(virtSizeS, virtSizeT, mapSizeS, mapSizeT, clipSizeS, clipSizeT, quantumS, quantumT, arena);
    }
    // WARNING: The no-arg constructor is *only* for creation of arrays
    // using "new" as just described.  It leaves the structure
    // completely uninitialized, so it is an error to access any
    // members or call any member functions except construct()
    // until construct() has been called.
    //
    pfTextureValidMapLevel() /* WARNING! see above */
    {
    }
    ~pfTextureValidMapLevel()
    {
	destruct();
    }
    void destruct()
    {
	pfMemory::unrefDelete(bitarray);
    }

    void construct(int virtSizeS, int virtSizeT,
		   int mapSizeS, int mapSizeT,
		   int clipSizeS, int clipSizeT,
		   int quantumS, int quantumT,
		   void *arena)
    {
	/*
	assert(_PFINORDER5(1, quantumS, clipSizeS, mapSizeS, virtSizeS));
	assert(_PFINORDER5(1, quantumT, clipSizeT, mapSizeT, virtSizeT));
	*/
	PFASSERTALWAYS(_PFISPOWEROF2(virtSizeS));
	PFASSERTALWAYS(_PFISPOWEROF2(virtSizeT));
	PFASSERTALWAYS(_PFISPOWEROF2(clipSizeS));
	PFASSERTALWAYS(_PFISPOWEROF2(clipSizeT));
	PFASSERTALWAYS(_PFISPOWEROF2(mapSizeS));
	PFASSERTALWAYS(_PFISPOWEROF2(mapSizeT));
	PFASSERTALWAYS(_PFISPOWEROF2(quantumS));
	PFASSERTALWAYS(_PFISPOWEROF2(quantumT));
	this->virtSizeS = virtSizeS;
	this->virtSizeT = virtSizeT;
	this->mapSizeS = mapSizeS;
	this->mapSizeT = mapSizeT;
	this->clipSizeS = clipSizeS;
	this->clipSizeT = clipSizeT;
	this->quantumS = quantumS;
	this->quantumT = quantumT;

	mapSizeMaskS = mapSizeS-1;
	mapSizeMaskT = mapSizeT-1;
	virtSizeMaskS = virtSizeS-1;
	virtSizeMaskT = virtSizeT-1;
	logQuantumS = floorlog2int(quantumS);
	logQuantumT = floorlog2int(quantumT);

	mapOriginS = 0;
	mapOriginT = 0;

	tracing = 0;

	int n = _PFHOWMANY(mapSizeS/quantumS*mapSizeT/quantumT, _PFBITS(*bitarray));
	bitarray = (int *)pfMemory::malloc(n*sizeof(int), arena);
	PFASSERTALWAYS(bitarray != NULL);
	pfMemory::ref(bitarray);
	clear();
    }

    void setOrigin(int newOriginS, int newOriginT)
    {
	PFASSERTALWAYS(0 <= newOriginS && newOriginS+mapSizeS <= virtSizeS);
	PFASSERTALWAYS(0 <= newOriginT && newOriginT+mapSizeT <= virtSizeT);

	int intersect_s0 = PF_MAX2(mapOriginS, newOriginS);
	int intersect_t0 = PF_MAX2(mapOriginT, newOriginT);
	int intersect_s1 = PF_MIN2(mapOriginS+mapSizeS, newOriginS+mapSizeS);
	int intersect_t1 = PF_MIN2(mapOriginT+mapSizeT, newOriginT+mapSizeT);


	if (intersect_s1<=intersect_s0
	 || intersect_t1<=intersect_t0)
	{
	    clearBitsNoWrap(0, 0, mapSizeS, mapSizeT);
	}
	else
	{
	    if (newOriginT < mapOriginT)
	    {
		/*
		 * Clear part of new area below old area
		 */
		clearBitsNoWrapS(0, newOriginT, mapSizeS, mapOriginT);
	    }
	    else if (newOriginT > mapOriginT)
	    {
		/*
		 * Clear part of new area above old area
		 */
		clearBitsNoWrapS(0, mapOriginT+mapSizeT, mapSizeS, newOriginT+mapSizeT);
	    }
	    if (newOriginS < mapOriginS)
	    {
		/*
		 * Clear part of new area to left (but not above or below)
		 * old area
		 */
		 clearBits(newOriginS, intersect_t0,
			   intersect_s0, intersect_t1);
	    }
	    else if (newOriginS > mapOriginS)
	    {
		/*
		 * Clear part of new area to right (but not above or below)
		 * new area
		 */
		 clearBits(intersect_s1, intersect_t0,
			   newOriginS+mapSizeS, intersect_t1);
	    }
	}

	mapOriginS = newOriginS;
	mapOriginT = newOriginT;
    }

    void setTracing(int _tracing)
    {
	tracing = _tracing;
    }

    /*
     * Methods for updating...
     */
    void clear()
    {
	clearBitsNoWrap(0, 0, mapSizeS, mapSizeT);
    }
    int isValid(int s0, int t0, int ns, int nt)
    {
	int s1 = s0+ns, t1 = t0+nt; /* internally we think in these terms */

	if (s1<=s0 || t1<=t0)
	    return 1; // empty region is always considered set

	if (s0 < mapOriginS || s1 > mapOriginS+mapSizeS
	 || t0 < mapOriginT || t1 > mapOriginT+mapSizeT)
	    return 0;
	return areBitsSet(s0, t0, s1, t1);
    }
    void recordTexLoad(int s0, int t0, int ns, int nt)
    {
	int s1 = s0+ns, t1 = t0+nt; /* internally we think in these terms */

	if (tracing)
	    fprintf(stderr, "%dx%d tex load %d,%d .. %d,%d\n",
		virtSizeS, virtSizeT, s0, t0, s1, t1);

	PFASSERTALWAYS(0 <= s0 && s0 <= s1 && s1 <= virtSizeS);
	PFASSERTALWAYS(0 <= t0 && t0 <= t1 && t1 <= virtSizeT);
	if (s0 == s1 || t0 == t1)
	    return;
	PFASSERTALWAYS(s1-s0 <= mapSizeS);
	PFASSERTALWAYS(t1-t0 <= mapSizeT);
	   
	/*
	 * Adjust map boundaries if necessary...
	 */
	if (s0 < mapOriginS || s1 > mapOriginS+mapSizeS
	 || t0 < mapOriginT || t1 > mapOriginT+mapSizeT)
	{
	    int newOriginS, newOriginT;
	    if (s0 < mapOriginS)
		newOriginS = s0;
	    else if (s1 > mapOriginS+mapSizeS)
		newOriginS = s1-mapSizeS;
	    else
		newOriginS = mapOriginS;
	    if (t0 < mapOriginT)
		newOriginT = t0;
	    else if (t1 > mapOriginT+mapSizeT)
		newOriginT = t1-mapSizeT;
	    else
		newOriginT = mapOriginT;
	    setOrigin(newOriginS, newOriginT);
	}

	/*
	 * If map is strictly bigger than clip size,
	 * need to clear the bits representing the
	 * texels we are clobbering from other texture areas
	 * that are equal (modulo the clipsize)
	 * to these new texel positions.
	 */
	int startS = s0, startT = t0;
	while (startS-clipSizeS+(s1-s0) > mapOriginS)
	    startS -= clipSizeS;
	while (startT-clipSizeT+(t1-t0) > mapOriginT)
	    startT -= clipSizeT;
	int s, t;
	for (t = startT; t < mapOriginT+mapSizeT; t += clipSizeT)
	for (s = startS; s < mapOriginS+mapSizeS; s += clipSizeS)
	{
	    if (s != s0 || t != t0)
	    {
		/*
		fprintf(stderr, "%dx%d: To load %d,%d..%d,%d, clearing %d,%d..%d,%d\n",
			virtSizeS, virtSizeT,
			s0,t0,s1,t1,
			PF_MAX2(s,mapOriginS),
			PF_MAX2(t,mapOriginT),
			PF_MIN2(s+(s1-s0),mapOriginS+mapSizeS),
			PF_MIN2(t+(t1-t0),mapOriginT+mapSizeT));
		*/

		clearBits(PF_MAX2(s,mapOriginS),
			  PF_MAX2(t,mapOriginT),
			  PF_MIN2(s+(s1-s0),mapOriginS+mapSizeS),
			  PF_MIN2(t+(t1-t0),mapOriginT+mapSizeT));
	    }
	    else
	    {
		/*
		fprintf(stderr, "%dx%d: Don't need to clear myself %d,%d..%d,%d\n",
			virtSizeS, virtSizeT,
			s0,t0,s1,t1);
		*/
	    }
	}

#ifdef NO_WRAPPING_LOADS /* this is true in real life, but I want to test */
	PFASSERTALWAYS((s0&~mapSizeMaskS) == ((s1-1)&~mapSizeMaskS));
	PFASSERTALWAYS((t0&~mapSizeMaskT) == ((t1-1)&~mapSizeMaskT));
	setBitsNoWrap(s0&mapSizeMaskS,
		      t0&mapSizeMaskT,
		      ((s1-1)&mapSizeMaskS)+1,
		      ((t1-1)&mapSizeMaskT)+1);
#else
	setBits(s0, t0, s1, t1);
#endif
    }

    void print(FILE *fp = stdout, int physical = 0)
    {
	fprintf(stdout, "   Map range = %d,%d .. %d,%d of %d,%d .. %d,%d\n",
		mapOriginS, mapOriginT, mapOriginS+mapSizeS, mapOriginT+mapSizeT,
		0,0, virtSizeS, virtSizeT);

	int s, t;

	printf("   +");
	for (s = 0; s < mapSizeS; s += quantumS)
	    printf("-");
	printf("+\n");

	int tstart = (mapSizeT/quantumT)&1 ? mapSizeT-quantumT
					    : mapSizeT-2*quantumT;
	for (t = tstart; t >= 0; t -= 2*quantumT)
	{
	    printf("   |");
	    for (s = 0; s < mapSizeS; s += quantumS)
	    {
		int upperbit, lowerbit;

		if (physical) /* show physical layout of the map, don't wrap */
		{
		    upperbit = (t==mapSizeT-quantumT ? 0 : _PFBIT(bitarray,
			(t/quantumT+1) * mapSizeS/quantumS + s/quantumS));
		    lowerbit = _PFBIT(bitarray,
			(t/quantumT+0) * mapSizeS/quantumS + s/quantumS);
		}
		else
		{
		    upperbit = (t==mapSizeT-quantumT ? 0 : _PFBIT(bitarray,
			(((t+mapOriginT+1*quantumT)&mapSizeMaskT)/quantumT)
				* mapSizeS/quantumS
		       + ((s+mapOriginS)&mapSizeMaskS)/quantumS));
		    lowerbit = (t==mapSizeT-quantumT ? 0 : _PFBIT(bitarray,
			(((t+mapOriginT+0*quantumT)&mapSizeMaskT)/quantumT)
				* mapSizeS/quantumS
		       + ((s+mapOriginS)&mapSizeMaskS)/quantumS));
		}

		if (upperbit)
		    if (lowerbit)
			fprintf(fp, ":");
		    else
			fprintf(fp, "\"");
		else
		    if (lowerbit)
			fprintf(fp, ".");
		    else
			fprintf(fp, " ");
	    }
	    fprintf(fp, "|\n");
	}

	printf("   +");
	for (s = 0; s < mapSizeS; s += quantumS)
	    printf("-");
	printf("+\n");

	/*
	int i;
	int n = _PFHOWMANY(mapSizeS/quantumS*mapSizeT/quantumT, _PFBITS(*bitarray));
	n = PF_MIN2(n, 10);
	for (i = 0; i < n; ++i)
	    fprintf(fp, "Bitarray[%d] = %#x\n", i, bitarray[i]);
	*/
    }

    // pfMemory's virtual
    int print(uint /*_travMode*/, uint /*_verbose*/, char *prefix, FILE *file)
    {
	printf("%spfValidMapLevel: 0x%p\n", prefix, this);
	print(file); // lame implementation for now
	return TRUE;
    }

private:
    // XXX virtually the same code is duplicated three times below--
    // XXX need to combine all these into a macro!
    int areBitsSetNoWrap(int s0, int t0, int s1, int t1)
    {
	PFASSERTALWAYS(0 <= s0 && s0 <= s1 && s1 <= mapSizeS);
	PFASSERTALWAYS(0 <= t0 && t0 <= t1 && t1 <= mapSizeT);

	PFASSERTALWAYS((s0&(quantumS-1))==0&&(t0&(quantumT-1))==0&&(s1&(quantumS-1))==0&&(t1&(quantumT-1))==0);
	s0 >>= logQuantumS;
	t0 >>= logQuantumT;
	s1 >>= logQuantumS;
	t1 >>= logQuantumT;

	/* Now the args are actual bit positions (rather than
	   texel positions) */

	int t;
	for (t = t0; t < t1; ++t)
	{
	    int rowstart = t*(mapSizeS>>logQuantumS);
	    int outer_s0word = (rowstart+s0)/_PFBITS(*bitarray);
	    int inner_s0word = _PFHOWMANY(rowstart+s0, _PFBITS(*bitarray));
	    int inner_s1word = (rowstart+s1)/_PFBITS(*bitarray);
	    int outer_s1word = _PFHOWMANY(rowstart+s1, _PFBITS(*bitarray));

	    int sword;
	    if (inner_s0word <= inner_s1word)
	    {
		/*
		 * normal case... possibly one partial word,
		 * followed by zero or more full words, followed by possibly
		 * one partial word
		 */
		if (outer_s0word != inner_s0word)
		    if (~bitarray[outer_s0word] & ~((1<<((rowstart+s0)&(_PFBITS(*bitarray)-1)))-1))
			return 0;
		for (sword = inner_s0word; sword < inner_s1word; ++sword)
		    if (~bitarray[sword] != 0)
			return 0;
		if (outer_s1word != inner_s1word)
		    if (~bitarray[inner_s1word] & ((1<<((rowstart+s1)&(_PFBITS(*bitarray)-1)))-1))
			return 0;
	    }
	    else
	    {
		/*
		 * Only one word and it's only partial.
		 */
		if (~bitarray[outer_s0word] &
			(~((1<<((rowstart+s0)&(_PFBITS(*bitarray)-1)))-1)
		        & ((1<<((rowstart+s1)&(_PFBITS(*bitarray)-1)))-1)))
		    return 0;
	    }
	}
	return 1;
    }
    int areBitsSetNoWrapS(int s0, int t0, int s1, int t1)
    {
	if ((t0&~mapSizeMaskT) < ((t1-1)&~mapSizeMaskT))
	{
	    return areBitsSetNoWrap(s0, t0&mapSizeMaskT, s1, mapSizeT)
		&& areBitsSetNoWrap(s0, 0, s1, ((t1-1)&mapSizeMaskT)+1);
	}
	else
	{
	    return areBitsSetNoWrap(s0, t0&mapSizeMaskT, s1, ((t1-1)&mapSizeMaskT)+1);
	}
    }
    int areBitsSet(int s0, int t0, int s1, int t1)
    {
	if (s1<=s0 || t1<=t0)
	    return 1; // empty region is always considered set
		      // also, the arithmetic assumes the region
		      // has nonzero size
	if (s1-s0 > mapSizeS || t1-t0 > mapSizeS)
	    return 0;
	if ((s0&~mapSizeMaskS) < ((s1-1)&~mapSizeMaskS))
	{
	    return areBitsSetNoWrapS(s0&mapSizeMaskS, t0, mapSizeS, t1)
	        && areBitsSetNoWrapS(0, t0, ((s1-1)&mapSizeMaskS)+1, t1);
	}
	else 
	{
	    return areBitsSetNoWrapS(s0&mapSizeMaskS, t0, ((s1-1)&mapSizeMaskS)+1,t1);
	}
    }
    void setBitsNoWrap(int s0, int t0, int s1, int t1)
    {
	PFASSERTALWAYS(0 <= s0 && s0 <= s1 && s1 <= mapSizeS);
	PFASSERTALWAYS(0 <= t0 && t0 <= t1 && t1 <= mapSizeT);
	/* XXX should this be an assertion or not? */
	/* PFASSERTALWAYS((s0&(quantumS-1))==0&&(t0&(quantumT-1))==0&&(s1&(quantumS-1))==0&&(t1&(quantumT-1))==0); */
	if ((s0&(quantumS-1))!=0
	 || (t0&(quantumT-1))!=0
	 || (s1&(quantumS-1))!=0
	 || (t1&(quantumT-1))!=0)
	{
	    fprintf(stderr, "pf WARNING: %dx%d level: load s0=%d,t0=%d,s1=%d,t1=%d not aligned to quanta %d,%d, not setting bits\n",
		virtSizeS, virtSizeT, s0,t0,s1,t1,quantumS, quantumT);
	    return;
	}
	    
	s0 >>= logQuantumS;
	t0 >>= logQuantumT;
	s1 >>= logQuantumS;
	t1 >>= logQuantumT;

	int t;
	for (t = t0; t < t1; ++t)
	{
	    int rowstart = t*(mapSizeS>>logQuantumS);
	    int outer_s0word = (rowstart+s0)/_PFBITS(*bitarray);
	    int inner_s0word = _PFHOWMANY(rowstart+s0, _PFBITS(*bitarray));
	    int inner_s1word = (rowstart+s1)/_PFBITS(*bitarray);
	    int outer_s1word = _PFHOWMANY(rowstart+s1, _PFBITS(*bitarray));

	    int sword;
	    if (inner_s0word <= inner_s1word)
	    {
		/*
		 * normal case... possibly one partial word,
		 * followed by zero or more full words, followed by possibly
		 * one partial word
		 */
		if (outer_s0word != inner_s0word)
		    bitarray[outer_s0word] |= ~((1<<((rowstart+s0)&(_PFBITS(*bitarray)-1)))-1);
		for (sword = inner_s0word; sword < inner_s1word; ++sword)
		    bitarray[sword] = ~0;
		if (outer_s1word != inner_s1word)
		    bitarray[inner_s1word] |= (1<<((rowstart+s1)&(_PFBITS(*bitarray)-1)))-1;
	    }
	    else
	    {
		/*
		 * Only one word and it's only partial.
		 */
		bitarray[outer_s0word] |=
			(~((1<<((rowstart+s0)&(_PFBITS(*bitarray)-1)))-1)
		        & ((1<<((rowstart+s1)&(_PFBITS(*bitarray)-1)))-1));
	    }
	}
    }
    void setBitsNoWrapS(int s0, int t0, int s1, int t1)
    {
	if ((t0&~mapSizeMaskT) < ((t1-1)&~mapSizeMaskT))
	{
	    setBitsNoWrap(s0, t0&mapSizeMaskT, s1, mapSizeT);
	    setBitsNoWrap(s0, 0, s1, ((t1-1)&mapSizeMaskT)+1);
	}
	else
	{
	    setBitsNoWrap(s0, t0&mapSizeMaskT, s1, ((t1-1)&mapSizeMaskT)+1);
	}
    }
    void setBits(int s0, int t0, int s1, int t1)
    {
	if (s1<=s0 || t1<=t0)
	    return; // the arithmetic assumes the region
		    // has nonzero size
	s1 = PF_MIN2(s1, s0+mapSizeS);
	t1 = PF_MIN2(t1, t0+mapSizeT);
	    
	if ((s0&~mapSizeMaskS) < ((s1-1)&~mapSizeMaskS))
	{
	    setBitsNoWrapS(s0&mapSizeMaskS, t0, mapSizeS, t1);
	    setBitsNoWrapS(0, t0, ((s1-1)&mapSizeMaskS)+1, t1);
	}
	else 
	{
	    setBitsNoWrapS(s0&mapSizeMaskS, t0, ((s1-1)&mapSizeMaskS)+1,t1);
	}
    }
    void clearBitsNoWrap(int s0, int t0, int s1, int t1)
    {
	PFASSERTALWAYS(0 <= s0 && s0 <= s1 && s1 <= mapSizeS);
	PFASSERTALWAYS(0 <= t0 && t0 <= t1 && t1 <= mapSizeT);
	PFASSERTALWAYS((s0&(quantumS-1))==0&&(t0&(quantumT-1))==0&&(s1&(quantumS-1))==0&&(t1&(quantumT-1))==0);
	s0 >>= logQuantumS;
	t0 >>= logQuantumT;
	s1 >>= logQuantumS;
	t1 >>= logQuantumT;

	/* Now the args are actual bit positions (rather than
	   texel positions) */

	int t;
	for (t = t0; t < t1; ++t)
	{
	    int rowstart = t*(mapSizeS>>logQuantumS);
	    int outer_s0word = (rowstart+s0)/_PFBITS(*bitarray);
	    int inner_s0word = _PFHOWMANY(rowstart+s0, _PFBITS(*bitarray));
	    int inner_s1word = (rowstart+s1)/_PFBITS(*bitarray);
	    int outer_s1word = _PFHOWMANY(rowstart+s1, _PFBITS(*bitarray));

	    int sword;
	    if (inner_s0word <= inner_s1word)
	    {
		/*
		 * normal case... possibly one partial word,
		 * followed by zero or more full words, followed by possibly
		 * one partial word
		 */
		if (outer_s0word != inner_s0word)
		    bitarray[outer_s0word] &= ((1<<((rowstart+s0)&(_PFBITS(*bitarray)-1)))-1);
		for (sword = inner_s0word; sword < inner_s1word; ++sword)
		    bitarray[sword] = 0;
		if (outer_s1word != inner_s1word)
		    bitarray[inner_s1word] &= ~((1<<((rowstart+s1)&(_PFBITS(*bitarray)-1)))-1);
	    }
	    else
	    {
		/*
		 * Only one word and it's only partial.
		 */
		bitarray[outer_s0word] &=
			~(~((1<<((rowstart+s0)&(_PFBITS(*bitarray)-1)))-1)
		         & ((1<<((rowstart+s1)&(_PFBITS(*bitarray)-1)))-1));
	    }
	}
    }
    void clearBitsNoWrapS(int s0, int t0, int s1, int t1)
    {
	if ((t0&~mapSizeMaskT) < ((t1-1)&~mapSizeMaskT))
	{
	    clearBitsNoWrap(s0, t0&mapSizeMaskT, s1, mapSizeT);
	    clearBitsNoWrap(s0, 0, s1, ((t1-1)&mapSizeMaskT)+1);
	}
	else
	{
	    clearBitsNoWrap(s0, t0&mapSizeMaskT, s1, ((t1-1)&mapSizeMaskT)+1);
	}
    }
    void clearBits(int s0, int t0, int s1, int t1)
    {
	if (s1<=s0 || t1<=t0)
	    return; // the arithmetic assumes the region
		    // has nonzero size
	s1 = PF_MIN2(s1, s0+mapSizeS);
	t1 = PF_MIN2(t1, t0+mapSizeT);

	if ((s0&~mapSizeMaskS) < ((s1-1)&~mapSizeMaskS))
	{
	    clearBitsNoWrapS(s0&mapSizeMaskS, t0, mapSizeS, t1);
	    clearBitsNoWrapS(0, t0, ((s1-1)&mapSizeMaskS)+1, t1);
	}
	else 
	{
	    clearBitsNoWrapS(s0&mapSizeMaskS, t0, ((s1-1)&mapSizeMaskS)+1,t1);
	}
    }
    static int floorlog2int(int x)
    {
	int count = -1;
	while (x > 0) {
	    x >>= 1;
	    count++;
	}
	return count;
    }
}; /* end class DLLEXPORT pfTextureValidMapLevel */

#define PFTEXTUREVALIDMAP ((pfTextureValidMap*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFTEXTUREVALIDMAPBUFFER ((pfTextureValidMap*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfTextureValidMap : public pfObject {
public:
    // CAPI:basename TextureValidMap

    //
    // Constructors and destructors and stuff...
    //
    // WARNING! see corresponding warning for pfTextureValidMapLevel constructor
    // XXX but note that the pfObject and pfMemory fields
    // XXX will be left uninitialized! So it's not okay to
    // XXX create an array of these.
    // 
    //CAPI:private
    pfTextureValidMap()
    {
    }
    //CAPI:public

    pfTextureValidMap(int virtSizeS, int virtSizeT, int mapSizeS, int mapSizeT, int clipSizeS, int clipSizeT, int quantumS, int quantumT)
    {
	construct(virtSizeS, virtSizeT,
		  mapSizeS, mapSizeT,
		  clipSizeS, clipSizeT,
		  quantumS, quantumT,
		  getArena());
    }

    void construct(int virtSizeS, int virtSizeT, int mapSizeS, int mapSizeT, int clipSizeS, int clipSizeT, int quantumS, int quantumT, void *arena)
    {
	nlevels = floorlog2int(PF_MAX2(virtSizeS,virtSizeT))+1;
	/* XXX what's the syntax for new(getArena())? couldn't seem to get it right */
	levels = (pfTextureValidMapLevel *)
	   pfMemory::malloc(sizeof(pfTextureValidMapLevel)*nlevels, arena);
	PFASSERTALWAYS(levels != NULL);
	pfMemory::ref(levels);
	int i;
	for (i = 0; i < nlevels; ++i)
	{
	    int levelSizeS = PF_MAX2(virtSizeS >> i, 1);
	    int levelSizeT = PF_MAX2(virtSizeT >> i, 1);
	    levels[i].construct(levelSizeS, levelSizeT,
				PF_MIN2(levelSizeS, mapSizeS),
				PF_MIN2(levelSizeT, mapSizeT),
				PF_MIN2(levelSizeS, clipSizeS),
				PF_MIN2(levelSizeT, clipSizeT),
				PF_MIN3(levelSizeS, clipSizeS, quantumS),
				PF_MIN3(levelSizeT, clipSizeT, quantumT),
				arena);
	}
    }

    ~pfTextureValidMap()
    {
	destruct();
    }
    void destruct()
    {
	if (levels != NULL)
	    pfMemory::unrefDelete(levels);
    }

    void recordTexLoad(int level, int s0, int t0, int ns, int nt)
    {
	PFASSERTALWAYS(0 <= level && level < nlevels);
	levels[level].recordTexLoad(s0, t0, ns, nt);
    }

    void setTracing(int _tracing)
    {
	int i;
	for (i = 0; i < nlevels; ++i)
	    levels[i].setTracing(_tracing);
    }

    void print(FILE *fp, int minLevelSize, int maxLevelSize)
    {
	int i;
	for (i = 0; i < nlevels; ++i)
	{
	    if (levels[i].virtSizeS < minLevelSize
	     || levels[i].virtSizeS > maxLevelSize
	     || levels[i].virtSizeT < minLevelSize
	     || levels[i].virtSizeT > maxLevelSize)
		continue;

	    fprintf(fp, "%dx%d level %d:\n", levels[i].virtSizeS,
					     levels[i].virtSizeT,
					     i);
	    levels[i].print();
	}
    }

    // pfMemory's virtual
    int print(uint /*_travMode*/, uint _verbose, char *prefix, FILE *file)
    {
	printf("%spfTextureValidMap: 0x%p\n", prefix, this);
	print(file, 0, _verbose); // hack-- number of levels is verbose value
	return TRUE;
    }

    /* XXX not sure about this API */
    int isValidCenter(int centerS, int centerT, int invalidBorder, int verbose)
    {
	int invalidLevels[64]; /* probably big enough forever */
	int n_invalidLevels = 0;
	int i;
	for (i = 0; i < nlevels; ++i)
	{
	    int s0, s1, t0, t1;
	    if (levels[i].clipSizeS == levels[i].virtSizeS) /* it's a pyramid level */
	    {
		s0 = t0 = 0;
		s1 = t1 = levels[i].clipSizeS;
	    }
	    else /* it's a roaming clip level */
	    {
		/* careful of overflow here! e.g. if the level center
		   was calculated as:
		   center * levels[i].virtSize / levels[0].virtSize */
		int levelCenterS = centerS /
		    (levels[0].virtSizeS / levels[i].virtSizeS);
		int levelCenterT = centerT /
		    (levels[0].virtSizeT / levels[i].virtSizeT);

		s0 = (levelCenterS - levels[i].clipSizeS/2) &~
		     ((1<<levels[i].logQuantumS)-1);
		t0 = (levelCenterT - levels[i].clipSizeT/2) &~
		     ((1<<levels[i].logQuantumT)-1);

		s1 = s0 + levels[i].clipSizeS;
		t1 = t0 + levels[i].clipSizeT;
		s0 += invalidBorder;
		t0 += invalidBorder;
		s1 -= invalidBorder;
		t1 -= invalidBorder;
		s0 = PF_CLAMP(s0, 0, levels[i].virtSizeS);
		t0 = PF_CLAMP(t0, 0, levels[i].virtSizeT);
		s1 = PF_CLAMP(s1, 0, levels[i].virtSizeS);
		t1 = PF_CLAMP(t1, 0, levels[i].virtSizeT);
	    }

	    if (!levels[i].isValid(s0, t0, s1-s0, t1-t0))
		invalidLevels[n_invalidLevels++] = i;
	}
	if (verbose)
	{
	    if (n_invalidLevels > 0)
	    {
		printf("Levels ");
		int i;
		for (i = 0; i < n_invalidLevels; ++i)
		    printf("%d%s",
			   invalidLevels[i],
			   i<n_invalidLevels-1 ? ", " : "");
		printf(" invalid");
	    }
	    else
		printf("All levels valid");
	    printf(" at center=%d,%d, iborder=%d\n",
			centerS, centerT, invalidBorder);
	}
	return n_invalidLevels == 0;
    }

    int isValid(int level, int s0, int t0, int ns, int nt)
    {
	PFASSERTALWAYS(0 <= level && level < nlevels);
	return levels[level].isValid(s0, t0, ns, nt);
    }

public: /* XXX not sure they should be; they're read-only */
    int nlevels;
    pfTextureValidMapLevel *levels; /* number like openGL-- level 0 is finest */
private:
    static int floorlog2int(int x)
    {
	int count = -1;
	while (x > 0) {
	    x >>= 1;
	    count++;
	}
	return count;
    }
}; /* end class DLLEXPORT pfTextureValidMap */

#endif /* __PF_TEXTURE_VALID_MAP_H__ */
