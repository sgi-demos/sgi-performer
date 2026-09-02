/*
 * Copyright 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 *
 *	objfnt.c - 	
 *		Support for spline, polygonal, and textured fonts.
 *
 *				Paul Haeberli - 1990
 * $Revision: 1.30 $
 * $Date: 2002/11/06 22:55:38 $
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#ifdef mips
#include <bstring.h>
#else
#include <string.h>
#ifndef WIN32
#include <asm/byteorder.h>
#endif
#endif
#include <Performer/pr.h>
#include <Performer/image.h>
#include <Performer/objfnt.h>

#define mymalloc(a)	pfMalloc(a, pfGetSharedArena())
#define free(a)		pfFree(a)

static void      bboxcalc(short v[2]);
static void      poly_print(short* sptr);
static void      spline_print(short* sptr);
static void      tmesh_draw(short* sptr);
static void      point_draw(short* sptr, int size);
static void      poly_draw(short* sptr);
static void      spline_draw(short* sptr);
static void      drawcurve(short v0[2], short v1[2], short v2[2], short v3[2]);
static void      bezadapt(float x0, float y0, float x1, float y1, 
			  float x2, float y2, float x3, float y3, float beztol);
static void	  initget(void);

/*
  On windows, files opened via fopen are "tweaked" by the so called OS unless
  they are open in binary mode. So, on windows we will fopen everything as
  binary
*/
#ifdef WIN32
#define READ_MODE "rb"
#define WRITE_MODE "wb"
#else
#define READ_MODE "r"
#define WRITE_MODE "w"
#endif


/*-------------------------------------------------------------------------*/


/* the following characters are used by the SEAC command to make */
/* accented (composite) characters in text fonts                 */
pschar accentlist[NACCENT] = {
    "/grave",           0301,   0,
    "/acute",           0302,   0,
    "/circumflex",      0303,   0,
    "/tilde",           0304,   0,
    "/macron",          0305,   0,
    "/breve",           0306,   0,
    "/dotaccent",       0307,   0,
    "/dieresis",        0310,   0,
    "/ring",            0312,   0,
    "/cedilla",         0313,   0,
    "/hungarumlaut",    0315,   0,
    "/ogonek",          0316,   0,
    "/caron",           0317,   0,
    "/dotlessi",        0365,   0
    };

pschar charlist[NASCII] = {/* ISO Latin1 Encoding of standard char set*/
    /* Missing a few characters!! */
    "/space", 		040,	0,
    "/exclam", 		041,	0,
    "/quotedbl", 	042,	0,
    "/numbersign", 	043,	0,
    "/dollar", 		044,	0,
    "/percent", 	045,	0,
    "/ampersand", 	046,	0,
    "/quoteright", 	047,	0,
    "/parenleft", 	050,	0,
    "/parenright", 	051,	0,
    "/asterisk", 	052,	0,
    "/plus", 		053,	0,
    "/comma", 		054,	0,
    "/hyphen", 		055,	0,
    "/period", 		056,	0,
    "/slash", 		057,	0,
    "/zero", 		060,	0,
    "/one", 		061,	0,
    "/two", 		062,	0,
    "/three", 		063,	0,
    "/four", 		064,	0,
    "/five", 		065,	0,
    "/six", 		066,	0,
    "/seven", 		067,	0,
    "/eight", 		070,	0,
    "/nine", 		071,	0,
    "/colon", 		072,	0,
    "/semicolon", 	073,	0,
    "/less",	 	074,	0,
    "/equal",	 	075,	0,
    "/greater",	 	076,	0,
    "/question", 	077,	0,
    "/at",	 	0100,	0,
    "/A",	 	0101,	0,
    "/B",	 	0102,	0,
    "/C",	 	0103,	0,
    "/D",	 	0104,	0,
    "/E",	 	0105,	0,
    "/F",	 	0106,	0,
    "/G",	 	0107,	0,
    "/H",	 	0110,	0,
    "/I",	 	0111,	0,
    "/J",	 	0112,	0,
    "/K",	 	0113,	0,
    "/L",	 	0114,	0,
    "/M",	 	0115,	0,
    "/N",	 	0116,	0,
    "/O",	 	0117,	0,
    "/P",	 	0120,	0,
    "/Q",	 	0121,	0,
    "/R",	 	0122,	0,
    "/S",	 	0123,	0,
    "/T",	 	0124,	0,
    "/U",	 	0125,	0,
    "/V",	 	0126,	0,
    "/W",	 	0127,	0,
    "/X",	 	0130,	0,
    "/Y",	 	0131,	0,
    "/Z",	 	0132,	0,
    "/bracketleft", 	0133,	0,
    "/backslash",	0134,	0,
    "/bracketright", 	0135,	0,
    "/asciicircum",	0136,	0,
    "/underscore", 	0137,	0,
    "/quoteleft", 	0140,	0,
    "/a",	 	0141,	0,
    "/b",	 	0142,	0,
    "/c", 		0143,	0,
    "/d",		0144,	0,
    "/e", 		0145,	0,
    "/f",		0146,	0,
    "/g",	 	0147,	0,
    "/h",	 	0150,	0,
    "/i",	 	0151,	0,
    "/j",	 	0152,	0,
    "/k", 		0153,	0,
    "/l",		0154,	0,
    "/m", 		0155,	0,
    "/n",		0156,	0,
    "/o",	 	0157,	0,
    "/p",	 	0160,	0,
    "/q",	 	0161,	0,
    "/r",	 	0162,	0,
    "/s", 		0163,	0,
    "/t",		0164,	0,
    "/u",		0165,	0,
    "/v", 		0166,	0,
    "/w",		0167,	0,
    "/x",	 	0170,	0,
    "/y",	 	0171,	0,
    "/z",	 	0172,	0,
    "/braceleft", 	0173,	0,
    "/bar",		0174,	0,
    "/braceright",	0175,	0,
    "/asciitilde", 	0176,	0,
    NULL,		0177,	0,
    NULL,		0200,	0,
    NULL,		0201,	0,
    NULL,		0202,	0,
    NULL,		0203,	0,
    NULL,		0204,	0,
    NULL,		0205,	0,
    NULL,		0206,	0,
    NULL,		0207,	0,
    NULL,		0210,	0,
    NULL,		0211,	0,
    NULL,		0212,	0,
    NULL,		0213,	0,
    NULL,		0214,	0,
    NULL,		0215,	0,
    NULL,		0216,	0,
    NULL,		0217,	0,
    "/dotlessi",	0220,	0,	/* added */
    "/grave",		0221,	0,	/* added */
    "/acute",		0222,	0,	/* added */
    "/circumflex",	0223,	0,	/* added */
    "/tilde",		0224,	0,	/* added */
    NULL,		0225,	0,	/* repeated macron */
    "/breve",		0226,	0,	/* added */
    "/dotaccent",	0227,	0,	/* added */
    NULL,		0230,	0,	/* repeated dieresis */
    NULL,		0231,	0,	
    "/ring",		0232,	0,	/* added */
    NULL,		0233,	0,	/* repeated cedilla */
    NULL,		0234,	0,
    "/hungarumlaut",	0235,	0,	/* added */
    "/ogonek",		0236,	0,	/* added */
    "/caron",		0237,	0,	/* added */
    NULL,		0240,	0,	/* repeated space */
    "/exclamdown", 	0241,	0,
    "/cent",	 	0242,	0,
    "/sterling",	0243,	0,
    "/currency",	0244,	0,
    "/yen",		0245,	0,
    "/brokenbar",	0246,	0,
    "/section",		0247,	0,
    "/dieresis",	0250,	0,
    "/copyright",	0251,	0,
    "/ordfeminine",	0252,	0,
    "/guillemotleft",	0253,	0,
    "/logicalnot",	0254,	0,
    "/hyphen",		0255,	0,
    "/registered",	0256,	0,
    "/macron",		0257,	0,
    "/degree",		0260,	0,
    "/plusminus",	0261,	0,
    "/twosuperior",	0262,	0,
    "/threesuperior",	0263,	0,
    "/acute",		0264,	0,
    "/mu",		0265,	0,
    "/paragraph",	0266,	0,
    "/periodcentered",	0267,	0,
    "/cedilla",		0270,	0,
    "/onesuperior",	0271,	0,
    "/ordmasculine",	0272,	0,
    "/guillemotright",	0273,	0,
    "/onequarter",	0274,	0,
    "/onehalf",		0275,	0,
    "/threequarters",	0276,	0,
    "/questiondown",	0277,	0,
    "/Agrave",		0300,	0,
    "/Aacute",		0301,	0,
    "/Acircumflex",	0302,	0,
    "/Atilde",		0303,	0,
    "/Adieresis",	0304,	0,
    "/Aring",		0305,	0,
    "/AE",		0306,	0,
    "/Ccedilla",	0307,	0,
    "/Egrave",		0310,	0,
    "/Eacute",		0311,	0,
    "/Ecircumflex",	0312,	0,
    "/Edieresis",	0313,	0,
    "/Igrave",		0314,	0,
    "/Iacute",		0315,	0,
    "/Icircumflex",	0316,	0,
    "/Idieresis",	0317,	0,
    "/Eth",		0320,	0,
    "/Ntilde",		0321,	0,
    "/Ograve",		0322,	0,
    "/Oacute",		0323,	0,
    "/Ocircumflex",	0324,	0,
    "/Otilde",		0325,	0,
    "/Odieresis",	0326,	0,
    "/multiply",	0327,	0,
    "/Oslash",		0330,	0,
    "/Ugrave",		0331,	0,
    "/Uacute",		0332,	0,
    "/Ucircumflex",	0333,	0,
    "/Udieresis",	0334,	0,
    "/Yacute",		0335,	0,
    "/Thorn",		0336,	0,
    "/germandbls",	0337,	0,
    "/agrave",		0340,	0,
    "/aacute",		0341,	0,
    "/acircumflex",	0342,	0,
    "/atilde",		0343,	0,
    "/adieresis",	0344,	0,
    "/aring",		0345,	0,
    "/ae",		0346,	0,
    "/ccedilla",	0347,	0,
    "/egrave",		0350,	0,
    "/eacute",		0351,	0,
    "/ecircumflex",	0352,	0,
    "/edieresis",	0353,	0,
    "/igrave",		0354,	0,
    "/iacute",		0355,	0,
    "/icircumflex",	0356,	0,
    "/idieresis",	0357,	0,
    "/eth",		0360,	0,
    "/ntilde",		0361,	0,
    "/ograve",		0362,	0,
    "/oacute",		0363,	0,
    "/ocircumflex",	0364,	0,
    "/otilde",		0365,	0,
    "/odieresis",	0366,	0,
    "/divide",		0367,	0,
    "/oslash",		0370,	0,
    "/ugrave",		0371,	0,
    "/uacute",		0372,	0,
    "/ucircumflex",	0373,	0,
    "/udieresis",	0374,	0,
    "/yacute",		0375,	0,
    "/thorn",		0376,	0,
    "/ydieresis",	0377,	0
    };

pschar scharlist[NSYMBL] = {
    "/space",		040,	0,
    "/exclam",		041,	0,
    "/universal",	042,	0,
    "/numbersign",	043,	0,
    "/existential",	044,	0,
    "/percent",		045,	0,
    "/ampersand",	046,	0,
    "/suchthat",	047,	0,
    "/parenleft",	050,	0,
    "/parenright",	051,	0,
    "/asteriskmath",	052,	0,
    "/plus",		053,	0,
    "/comma",		054,	0,
    "/minus",		055,	0,
    "/period",		056,	0,
    "/slash",		057,	0,
    "/zero",		060,	0,
    "/one",		061,	0,
    "/two",		062,	0,
    "/three",		063,	0,
    "/four",		064,	0,
    "/five",		065,	0,
    "/six",		066,	0,
    "/seven",		067,	0,
    "/eight",		070,	0,
    "/nine",		071,	0,
    "/colon",		072,	0,
    "/semicolon",	073,	0,
    "/less",		074,	0,
    "/equal",		075,	0,
    "/greater",		076,	0,
    "/question",	077,	0,
    "/congruent",	0100,	0,
    "/Alpha",		0101,	0,
    "/Beta",		0102,	0,
    "/Chi",		0103,	0,
    "/Delta",		0104,	0,
    "/Epsilon",		0105,	0,
    "/Phi",		0106,	0,
    "/Gamma",		0107,	0,
    "/Eta",		0110,	0,
    "/Iota",		0111,	0,
    "/theta1",		0112,	0,
    "/Kappa",		0113,	0,
    "/Lambda",		0114,	0,
    "/Mu",		0115,	0,
    "/Nu",		0116,	0,
    "/Omicron",		0117,	0,
    "/Pi",		0120,	0,
    "/Theta",		0121,	0,
    "/Rho",		0122,	0,
    "/Sigma",		0123,	0,
    "/Tau",		0124,	0,
    "/Upsilon",		0125,	0,
    "/sigma1",		0126,	0,
    "/Omega",		0127,	0,
    "/Xi",		0130,	0,
    "/Psi",		0131,	0,
    "/Zeta",		0132,	0,
    "/bracketleft",	0133,	0,
    "/therefore",	0134,	0,
    "/bracketright",	0135,	0,
    "/perpendicular",	0136,	0,
    "/underscore",	0137,	0,
    "/radicalex",	0140,	0,
    "/alpha",		0141,	0,
    "/beta",		0142,	0,
    "/chi",		0143,	0,
    "/delta",		0144,	0,
    "/epsilon",		0145,	0,
    "/phi",		0146,	0,
    "/gamma",		0147,	0,
    "/eta",		0150,	0,
    "/iota",		0151,	0,
    "/phi1",		0152,	0,
    "/kappa",		0153,	0,
    "/lambda",		0154,	0,
    "/mu",		0155,	0,
    "/nu",		0156,	0,
    "/omicron",		0157,	0,
    "/pi",		0160,	0,
    "/theta",		0161,	0,
    "/rho",		0162,	0,
    "/sigma",		0163,	0,
    "/tau",		0164,	0,
    "/upsilon",		0165,	0,
    "/omega1",		0166,	0,
    "/omega",		0167,	0,
    "/xi",		0170,	0,
    "/psi",		0171,	0,
    "/zeta",		0172,	0,
    "/braceleft",	0173,	0,
    "/bar",		0174,	0,
    "/braceright",	0175,	0,
    "/similar",		0176,	0,
    NULL,               0177,   0,
    NULL,               0200,   0,
    NULL,               0201,   0,
    NULL,               0202,   0,
    NULL,               0203,   0,
    NULL,               0204,   0,
    NULL,               0205,   0,
    NULL,               0206,   0,
    NULL,               0207,   0,
    NULL,               0210,   0,
    NULL,               0211,   0,
    NULL,               0212,   0,
    NULL,               0213,   0,
    NULL,               0214,   0,
    NULL,               0215,   0,
    NULL,               0216,   0,
    NULL,               0217,   0,
    NULL,               0220,   0,
    NULL,               0221,   0,
    NULL,               0222,   0,
    NULL,               0223,   0,
    NULL,               0224,   0,
    NULL,               0225,   0,
    NULL,               0226,   0,
    NULL,               0227,   0,
    NULL,               0230,   0,
    NULL,               0231,   0,
    NULL,               0232,   0,
    NULL,               0233,   0,
    NULL,               0234,   0,
    NULL,               0235,   0,
    NULL,               0236,   0,
    NULL,               0237,   0,
    NULL,               0240,   0,
    "/Upsilon1",	0241,	0,
    "/minute",		0242,	0,
    "/lessequal",	0243,	0,
    "/fraction",	0244,	0,
    "/infinity",	0245,	0,
    "/florin",		0246,	0,
    "/club",		0247,	0,
    "/diamond",		0250,	0,
    "/heart",		0251,	0,
    "/spade",		0252,	0,
    "/arrowboth",	0253,	0,
    "/arrowleft",	0254,	0,
    "/arrowup",		0255,	0,
    "/arrowright",	0256,	0,
    "/arrowdown",	0257,	0,
    "/degree",		0260,	0,
    "/plusminus",	0261,	0,
    "/second",		0262,	0,
    "/greaterequal",	0263,	0,
    "/multiply",	0264,	0,
    "/proportional",	0265,	0,
    "/partialdiff",	0266,	0,
    "/bullet",		0267,	0,
    "/divide",		0270,	0,
    "/notequal",	0271,	0,
    "/equivalence",	0272,	0,
    "/approxequal",	0273,	0,
    "/ellipsis",	0274,	0,
    "/arrowvertex",	0275,	0,
    "/arrowhorizex",	0276,	0,
    "/carriagereturn",	0277,	0,
    "/aleph",		0300,	0,
    "/Ifraktur",	0301,	0,
    "/Rfraktur",	0302,	0,
    "/weierstrass",	0303,	0,
    "/circlemultiply",	0304,	0,
    "/circleplus",	0305,	0,
    "/emptyset",	0306,	0,
    "/intersection",	0307,	0,
    "/union",		0310,	0,
    "/propersuperset",	0311,	0,
    "/reflexsuperset",	0312,	0,
    "/notsubset",	0313,	0,
    "/propersubset",	0314,	0,
    "/reflexsubset",	0315,	0,
    "/element",		0316,	0,
    "/notelement",	0317,	0,
    "/angle",		0320,	0,
    "/gradient",	0321,	0,
    "/registerserif",	0322,	0,
    "/copyrightserif",	0323,	0,
    "/trademarkserif",	0324,	0,
    "/product",		0325,	0,
    "/radical",		0326,	0,
    "/dotmath",		0327,	0,
    "/logicalnot",	0330,	0,
    "/logicaland",	0331,	0,
    "/logicalor",	0332,	0,
    "/arrowdblboth",	0333,	0,
    "/arrowdblleft",	0334,	0,
    "/arrowdblup",	0335,	0,
    "/arrowdblright",	0336,	0,
    "/arrowdbldown",	0337,	0,
    "/lozenge",		0340,	0,
    "/angleleft",	0341,	0,
    "/registersans",	0342,	0,
    "/copyrightsans",	0343,	0,
    "/trademarksans",	0344,	0,
    "/summation",	0345,	0,
    "/parenlefttp",	0346,	0,
    "/parenleftex",	0347,	0,
    "/parenleftbt",	0350,	0,
    "/bracketlefttp",	0351,	0,
    "/bracketleftex",	0352,	0,
    "/bracketleftbt",	0353,	0,
    "/bracelefttp",	0354,	0,
    "/braceleftmid",	0355,	0,
    "/braceleftbt",	0356,	0,
    "/braceex",		0357,	0,
    "/apple",           0360,   0, 
    "/angleright",	0361,	0,
    "/integral",	0362,	0,
    "/integraltp",	0363,	0,
    "/integralex",	0364,	0,
    "/integralbt",	0365,	0,
    "/parenrighttp",	0366,	0,
    "/parenrightex",	0367,	0,
    "/parenrightbt",	0370,	0,
    "/bracketrighttp",	0371,	0,
    "/bracketrightex",	0372,	0,
    "/bracketrightbt",	0373,	0,
    "/bracerighttp",	0374,	0,
    "/bracerightmid",	0375,	0,
    "/bracerightbt",	0376,	0,
    NULL,               0377,   0
    };

pschar zcharlist[NZAPFD] = {
    "/space",		040,	0,
    "/a1",		041,	0,
    "/a2",		042,	0,
    "/a202",		043,	0,
    "/a3",		044,	0,
    "/a4",		045,	0,
    "/a5",		046,	0,
    "/a119",		047,	0,
    "/a118",		050,	0,
    "/a117",		051,	0,
    "/a11",		052,	0,
    "/a12",		053,	0,
    "/a13",		054,	0,
    "/a14",		055,	0,
    "/a15",		056,	0,
    "/a16",		057,	0,
    "/a105",		060,	0,
    "/a17",		061,	0,
    "/a18",		062,	0,
    "/a19",		063,	0,
    "/a20",		064,	0,
    "/a21",		065,	0,
    "/a22",		066,	0,
    "/a23",		067,	0,
    "/a24",		070,	0,
    "/a25",		071,	0,
    "/a26",		072,	0,
    "/a27",		073,	0,
    "/a28",		074,	0,
    "/a6",		075,	0,
    "/a7",		076,	0,
    "/a8",		077,	0,
    "/a9",		0100,	0,
    "/a10",		0101,	0,
    "/a29",		0102,	0,
    "/a30",		0103,	0,
    "/a31",		0104,	0,
    "/a32",		0105,	0,
    "/a33",		0106,	0,
    "/a34",		0107,	0,
    "/a35",		0110,	0,
    "/a36",		0111,	0,
    "/a37",		0112,	0,
    "/a38",		0113,	0,
    "/a39",		0114,	0,
    "/a40",		0115,	0,
    "/a41",		0116,	0,
    "/a42",		0117,	0,
    "/a43",		0120,	0,
    "/a44",		0121,	0,
    "/a45",		0122,	0,
    "/a46",		0123,	0,
    "/a47",		0124,	0,
    "/a48",		0125,	0,
    "/a49",		0126,	0,
    "/a50",		0127,	0,
    "/a51",		0130,	0,
    "/a52",		0131,	0,
    "/a53",		0132,	0,
    "/a54",		0133,	0,
    "/a55",		0134,	0,
    "/a56",		0135,	0,
    "/a57",		0136,	0,
    "/a58",		0137,	0,
    "/a59",		0140,	0,
    "/a60",		0141,	0,
    "/a61",		0142,	0,
    "/a62",		0143,	0,
    "/a63",		0144,	0,
    "/a64",		0145,	0,
    "/a65",		0146,	0,
    "/a66",		0147,	0,
    "/a67",		0150,	0,
    "/a68",		0151,	0,
    "/a69",		0152,	0,
    "/a70",		0153,	0,
    "/a71",		0154,	0,    
    "/a72",		0155,	0,
    "/a73",		0156,	0,
    "/a74",		0157,	0,
    "/a203",		0160,	0,
    "/a75",		0161,	0,
    "/a204",		0162,	0,
    "/a76",		0163,	0,
    "/a77",		0164,	0,
    "/a78",		0165,	0,
    "/a79",		0166,	0,
    "/a81",		0167,	0,
    "/a82",		0170,	0,
    "/a83",		0171,	0,
    "/a84",		0172,	0,
    "/a97",		0173,	0,
    "/a98",		0174,	0,
    "/a99",		0175,	0,
    "/a100",		0176,	0,
    NULL,               0177,   0,
    NULL,               0200,   0,
    NULL,               0201,   0,
    NULL,               0202,   0,
    NULL,               0203,   0,
    NULL,               0204,   0,
    NULL,               0205,   0,
    NULL,               0206,   0,
    NULL,               0207,   0,
    NULL,               0210,   0,
    NULL,               0211,   0,
    NULL,               0212,   0,
    NULL,               0213,   0,
    NULL,               0214,   0,
    NULL,               0215,   0,
    NULL,               0216,   0,
    NULL,               0217,   0,
    NULL,               0220,   0,
    NULL,               0221,   0,
    NULL,               0222,   0,
    NULL,               0223,   0,
    NULL,               0224,   0,
    NULL,               0225,   0,
    NULL,               0226,   0,
    NULL,               0227,   0,
    NULL,               0230,   0,
    NULL,               0231,   0,
    NULL,               0232,   0,
    NULL,               0233,   0,
    NULL,               0234,   0,
    NULL,               0235,   0,
    NULL,               0236,   0,
    NULL,               0237,   0,
    NULL,               0240,   0,
    "/a101",		0241,	0,
    "/a102",		0242,	0,
    "/a103",		0243,	0,
    "/a104",		0244,	0,
    "/a106",		0245,	0,
    "/a107",		0246,	0,
    "/a108",		0247,	0,
    "/a112",		0250,	0,
    "/a111",		0251,	0,
    "/a110",		0252,	0,
    "/a109",		0253,	0,
    "/a120",		0254,	0,
    "/a121",		0255,	0,
    "/a122",		0256,	0,
    "/a123",		0257,	0,
    "/a124",		0260,	0,
    "/a125",		0261,	0,
    "/a126",		0262,	0,
    "/a127",		0263,	0,
    "/a128",		0264,	0,
    "/a129",		0265,	0,
    "/a130",		0266,	0,
    "/a131",		0267,	0,
    "/a132",		0270,	0,
    "/a133",		0271,	0,
    "/a134",		0272,	0,
    "/a135",		0273,	0,
    "/a136",		0274,	0,
    "/a137",		0275,	0,
    "/a138",		0276,	0,
    "/a139",		0277,	0,
    "/a140",		0300,	0,
    "/a141",		0301,	0,
    "/a142",		0302,	0,
    "/a143",		0303,	0,
    "/a144",		0304,	0,
    "/a145",		0305,	0,
    "/a146",		0306,	0,
    "/a147",		0307,	0,
    "/a148",		0310,	0,
    "/a149",		0311,	0,
    "/a150",		0312,	0,
    "/a151",		0313,	0,
    "/a152",		0314,	0,
    "/a153",		0315,	0,
    "/a154",		0316,	0,
    "/a155",		0317,	0,
    "/a156",		0320,	0,
    "/a157",		0321,	0,
    "/a158",		0322,	0,
    "/a159",		0323,	0,
    "/a160",		0324,	0,
    "/a161",		0325,	0,
    "/a163",		0326,	0,
    "/a164",		0327,	0,
    "/a196",		0330,	0,
    "/a165",		0331,	0,
    "/a192",		0332,	0,
    "/a166",		0333,	0,
    "/a167",		0334,	0,
    "/a168",		0335,	0,
    "/a169",		0336,	0,
    "/a170",		0337,	0,
    "/a171",		0340,	0,
    "/a172",		0341,	0,
    "/a173",		0342,	0,
    "/a162",		0343,	0,
    "/a174",		0344,	0,
    "/a175",		0345,	0,
    "/a176",		0346,	0,
    "/a177",		0347,	0,
    "/a178",		0350,	0,
    "/a179",		0351,	0,
    "/a193",		0352,	0,
    "/a180",		0353,	0,
    "/a199",		0354,	0,
    "/a181",		0355,	0,
    "/a200",		0356,	0,
    "/a182",		0357,	0,
    NULL,               0360,   0,
    "/a201",		0361,	0,
    "/a183",		0362,	0,
    "/a184",		0363,	0,
    "/a197",		0364,	0,
    "/a185",		0365,	0,
    "/a194",		0366,	0,
    "/a198",		0367,	0,
    "/a186",		0370,	0,
    "/a195",		0371,	0,
    "/a187",		0372,	0,
    "/a188",		0373,	0,
    "/a189",		0374,	0,
    "/a190",		0375,	0,
    "/a191",		0376,	0,
    NULL,               0377,   0
    };

static int dopoints;

#ifdef __LITTLE_ENDIAN
static short fget16(FILE *f)
{
   unsigned char a = fgetc(f);
   unsigned char b = fgetc(f);
   if (ferror(f))
      perror("fgetc");
   return (a << 8) | b;
}

static int fget32(FILE *f)
{
   unsigned char a = fgetc(f);
   unsigned char b = fgetc(f);
   unsigned char c = fgetc(f);
   unsigned char d = fgetc(f);
   if (ferror(f))
      perror("fgetc");
   return (a << 24) | (b << 16) | (c << 8) | d;
}
#endif




void 
fntpointmode(int size)
{
    dopoints = size;
}

/*
*	printing stuff for debugging
*
*
*/
void 
printobjfnt(objfnt *fnt)
{
    int i;

    printf("Object font type is %d\n",fnt->type);
    printf("First char is %d\n",fnt->charmin);
    printf("Last char is %d\n",fnt->charmax);
    printf("Num chars is %d\n",fnt->nchars);
    printf("Fnt scale is %d\n\n",fnt->scale);
    for(i=fnt->charmin; i<=fnt->charmax; i++) 
	printobjchar(fnt,i);
}

short*
getcharprog(objfnt *fnt, int c)
{
    int index;
    chardesc *cd;

    index = chartoindex(fnt,c);
    if(index<0)
	return 0;
    cd = fnt->chars+index;
    return cd->data;
}

chardesc*
getchardesc(objfnt *fnt, int c)
{
    int index;

    index = chartoindex(fnt,c);
    if(index<0)
	return 0;
    return fnt->chars+index;
}

int 
chartoindex(objfnt *fnt, int c)
{
    if(c<fnt->charmin)
	return -1;
    if(c>fnt->charmax)
	return -1;
    return c-fnt->charmin;
}

int 
printobjchar(objfnt *fnt, int c)
{
    int index;
    chardesc *cd;
    char *aname;

    index = chartoindex(fnt,c);
    if(index<0)
	return 0;
    cd = fnt->chars+index;
    aname = asciiname(c);
    if(aname)
	printf("Charname: \"%s\"  code is %d\n",aname,c);
    else
	printf("Charname: NULL code is %d\n",c);
    printf("Advance %d %d\n",cd->movex,cd->movey);
    printf("Bbox %d %d to %d %d\n",cd->llx,cd->lly,cd->urx,cd->ury);
    printf("Data %d tokens\n\n",cd->datalen/2);
    if(cd->data) {
	switch(fnt->type) {
	case PO_TYPE:
	    poly_print(cd->data);
	    break;
	case SP_TYPE:
	    spline_print(cd->data);
	    break;
	case TM_TYPE:
	    fprintf(stderr,"printobjchar: can't print meshed fonts yet\n");
	    break;
	default:
	    fprintf(stderr,"printobjchar: bad obj font type\n");
	}
    }
    return 1;
}

void 
applytoobjfntverts(objfnt *fnt, void (*func)(short*))
{
    int c;

    for(c=fnt->charmin; c<=fnt->charmax; c++) 
	applytocharverts(fnt,c,func);
}

void 
applytocharverts(objfnt *fnt, int c, void (*func)(short*))
{
    int index;
    chardesc *cd;
    short *sptr;
    int nverts;

    index = chartoindex(fnt,c);
    if(index<0)
	return;
    cd = fnt->chars+index;
    if(cd->data) {
	sptr = cd->data;
	switch(fnt->type) {
	case TM_TYPE:
	    while(1) {
		switch(*sptr++) {	
		case TM_BGNTMESH:
		case TM_SWAPTMESH:
		case TM_ENDBGNTMESH:
		    break;
		case TM_RETENDTMESH:
		case TM_RET:
		    return;
		default:
		    fprintf(stderr,"applytocharverts: bad TM op\n");
		    return;
		}
		nverts = *sptr++;
		while(nverts--) {
		    (*func)(sptr);
		    sptr+=2;
		}
	    }
	case PO_TYPE:
	    while(1) {
		switch(*sptr++) {	
		case PO_BGNLOOP:
		case PO_ENDBGNLOOP:
		    break;
		case PO_RETENDLOOP:
		case PO_RET:
		    return;
		default:
		    fprintf(stderr,"applytocharverts: bad PO op\n");
		    return;
		}
		nverts = *sptr++;
		while(nverts--) {
		    (*func)(sptr);
		    sptr+=2;
		}
	    }
	case SP_TYPE:
	    while(1) {
		switch(*sptr++) {	
		case SP_MOVETO:
		    (*func)(sptr);
		    sptr+=2;
		    break;
		case SP_LINETO:
		    (*func)(sptr);
		    sptr+=2;
		    break;
		case SP_CURVETO:
		    (*func)(sptr+0);
		    (*func)(sptr+2);
		    (*func)(sptr+4);
		    sptr+=6;
		    break;
		case SP_CLOSEPATH:
		    break;
		case SP_RETCLOSEPATH:
		    return;
		case SP_RET:
		    return;
		default:
		    fprintf(stderr,"applytocharverts: bad SP op\n");
		    return;
		}
	    }
	default:
	    fprintf(stderr,"applytocharverts: bad obj font type\n");
	}
    }
}

/* XXX not quite right PH */

void 
applytocharedges(objfnt *fnt, int c, void (*func)(short*,short*))
{
    int i, index, nverts;
    chardesc *cd;
    short *sptr, *p1, *p2;

    if(fnt->type != PO_TYPE) {
        fprintf(stderr,"applytocharedges only works on PO_TYPE fonts\n");
        exit(1);
    }
    index = chartoindex(fnt,c);
    if(index<0)
        return;
    cd = fnt->chars+index;
    if(cd->data) {
        sptr = cd->data;
        while(1) {
            switch(*sptr++) {
	    case PO_BGNLOOP:
	    case PO_ENDBGNLOOP:
		break;
	    case PO_RETENDLOOP:
	    case PO_RET:
		return;
            }
            nverts = *sptr++;
            for(i=0; i<nverts; i++) {
                p1 = sptr+(2*(i));
                p2 = sptr+(2*((i+1)%nverts));
                (*func)(p1,p2);
            }
            sptr+=(2*nverts);
        }
    }
}

static int xmin, xmax, ymin, ymax;

static void 
bboxcalc(short v[2])
{
    if(v[0]<xmin)
	xmin = v[0];
    if(v[1]<ymin)
	ymin = v[1];
    if(v[0]>xmax)
	xmax = v[0];
    if(v[1]>ymax)
	ymax = v[1];
}

void 
fontbbox(objfnt *fnt, int *llx, int *lly, int *urx, int *ury)
{
    xmin = ymin = NOBBOX;
    xmax = ymax = -NOBBOX;
    applytoobjfntverts(fnt,bboxcalc);
    if(xmin == NOBBOX) {
	*llx = NOBBOX;
	*lly = NOBBOX;
	*urx = NOBBOX;
	*ury = NOBBOX;
    } else {
	*llx = xmin;
	*lly = ymin;
	*urx = xmax;
	*ury = ymax;
    }
}


void 
calccharbboxes(objfnt *fnt)
{
    int c;
    chardesc *cd;

    for(c=fnt->charmin; c<=fnt->charmax; c++) {
	xmin = ymin = NOBBOX;
	xmax = ymax = -NOBBOX;
	applytocharverts(fnt,c,bboxcalc);
	cd = getchardesc(fnt,c);
	if(xmin == NOBBOX) {
	    cd->llx = NOBBOX;
	    cd->lly = NOBBOX;
	    cd->urx = -NOBBOX;
	    cd->ury = -NOBBOX;
	} else {
	    cd->llx = xmin;
	    cd->lly = ymin;
	    cd->urx = xmax;
	    cd->ury = ymax;
	}
    }
}


/*
*	poly_print -
*		Print a poly character
*
*/
void 
poly_print(short *sptr)
{
    int nverts;

    while(1) {
	switch(*sptr++) {	
	case PO_BGNLOOP:
	    printf("bgnloop\n");
	    break;
	case PO_ENDBGNLOOP:
	    printf("endbgnloop\n\n");
	    break;
	case PO_RETENDLOOP:
	    printf("retendloop\n\n");
	    return;
	case PO_RET:
	    printf("ret\n\n");
	    return;
	default:
	    fprintf(stderr,"poly_print: bad PO op\n");
	    return;
	}
    	nverts = *sptr++;
	while(nverts--) {
	    printf("	vert %d %d\n",sptr[0],sptr[1]);
	    sptr+=2;
	}
    }
}


/*
*	spline_print -
*		Print a spline character
*
*/
static void 
spline_print(short *sptr)
{
    while(1) {
	switch(*sptr++) {	
	case SP_MOVETO:
	    printf("beginloop\n");
	    printf("    moveto %d %d\n",sptr[0], sptr[1]);
	    sptr+=2;
	    break;
	case SP_LINETO:
	    printf("    lineto %d %d\n",sptr[0], sptr[1]);
	    sptr+=2;
	    break;
	case SP_CURVETO:
	    printf("    curveto %d %d %d %d %d %d\n",
		   sptr[0],sptr[1],sptr[2],sptr[3],sptr[4],sptr[5]);
	    sptr+=6;
	    break;
	case SP_CLOSEPATH:
	    printf("endloop\n");
	    break;
	case SP_RETCLOSEPATH:
	    printf("endloop\n");
	    printf("ret\n\n");
	    return;
	case SP_RET:
	    printf("ret\n\n");
	    return;
	default:
	    fprintf(stderr,"spline_print: bad SP op\n");
	    return;
	}
    }
}


/*
*	tmesh_draw -
*		Draw a tmesh character
*
*/
static void 
tmesh_draw(short *sptr)
{
    int nverts;

    while(1) {
	switch(*sptr++) {	
	case TM_BGNTMESH:
	    glBegin(GL_TRIANGLE_STRIP);
	    break;
	case TM_SWAPTMESH:
	    printf("swap\n");
	    printf("tmesh_draw - swaptmesh not implemented.\n");
	    break;
	case TM_ENDBGNTMESH:
	    glBegin(GL_TRIANGLE_STRIP);
	    glEnd();
	    break;
	case TM_RETENDTMESH:
	    glEnd();
	    return;
	case TM_RET:
	    glEnd();
	    return;
	default:
	    fprintf(stderr,"tmesh_draw: bad TM op\n");
	    return;
	}
	nverts = *sptr++;
	while(nverts--) {
	    glVertex2sv(sptr);
	    sptr+=2;
	}
    }
}

/*
*	point_draw -
*		Draw the points in a character.
*
*/
static void 
point_draw(short *sptr, int size)
{
    int nverts, vertno;
    int x, y, del;

    del = size/2;
    if(del == 0)
	del = 1;
    while(1) {
	switch(*sptr++) {	
	case PO_BGNLOOP:
	    break;
	case PO_ENDBGNLOOP:
	    break;
	case PO_RETENDLOOP:
	    return;
	case PO_RET:
	    return;
	default:
	    fprintf(stderr,"point_draw: bad PO op\n");
	    return;
	}
    	nverts = *sptr++;
 	vertno = 1;
	while(nverts--) {
	    x = sptr[0];
	    y = sptr[1];
	    if(vertno++ == 1)
		glColor3s(0x0, 0x0, 0x80);
	    else
		glColor3s(0xff, 0x0, 0x0);
	    if(size == 1)
	    {
		glBegin(GL_POINTS);
		glVertex2i(x, y);
		glEnd();
	    }
	    else
		glRectf(x-del,y-del,x+del,y+del);
	    sptr+=2;
	}
    }
}

/*
*	poly_draw -
*		Draw a poly character
*
*/
static void 
poly_draw(short *sptr)
{
    int nverts;

    while(1) {
	switch(*sptr++) {	
	case PO_BGNLOOP:
	    glBegin(GL_LINE_LOOP);
	    break;
	case PO_ENDBGNLOOP:
	    glEnd();
	    glBegin(GL_LINE_LOOP);
	    break;
	case PO_RETENDLOOP:
	    glEnd();
	    return;
	case PO_RET:
	    return;
	default:
	    fprintf(stderr,"poly_draw: bad PO op\n");
	    return;
	}
    	nverts = *sptr++;
	while(nverts--) {
	    glVertex2sv(sptr);
	    sptr+=2;
	}
    }
}

/*
*	spline_draw -
*		Draw a spline character
*
*/
static void 
spline_draw(short *sptr)
{
    while(1) {
	switch(*sptr++) {	
	case SP_MOVETO:
	    glBegin(GL_LINE_LOOP);
	    glVertex2sv(sptr);
	    sptr+=2;
	    break;
	case SP_LINETO:
	    glVertex2sv(sptr);
	    sptr+=2;
	    break;
	case SP_CURVETO:
	    drawcurve(sptr-3,sptr,sptr+2,sptr+4);
	    sptr+=6;
	    break;
	case SP_CLOSEPATH:
	    glEnd();
	    break;
	case SP_RETCLOSEPATH:
	    glEnd();
	    return;
	case SP_RET:
	    return;
	default:
	    fprintf(stderr,"spline_draw: bad SP op\n");
	    return;
	}
    }
}

#define BEZTOL	2.0

static void 
drawcurve(short v0[2], short v1[2], short v2[2], short v3[2])
{
    float x0, y0, x1, y1, x2, y2, x3, y3;

    x0 = v0[0];
    y0 = v0[1];
    x1 = v1[0];
    y1 = v1[1];
    x2 = v2[0];
    y2 = v2[1];
    x3 = v3[0];
    y3 = v3[1];
    bezadapt(x0,y0,x1,y1,x2,y2,x3,y3,BEZTOL);
}

static void 
bezadapt(float x0, float y0, float x1, float y1, 
         float x2, float y2, float x3, float y3, float beztol)
{
    float ax0,ay0,ax1,ay1,ax2,ay2,ax3,ay3;
    float bx0,by0,bx1,by1,bx2,by2,bx3,by3;
    float midx, midy;
    float linx, liny, dx, dy, mag;
    float v[2];
    
    midx = (x0+3*x1+3*x2+x3)/8.0f;
    midy = (y0+3*y1+3*y2+y3)/8.0f;
    linx = (x0+x3)/2.0f;
    liny = (y0+y3)/2.0f;
    dx = midx-linx;
    dy = midy-liny;
    mag = dx*dx+dy*dy;
    if(mag<(beztol*beztol)) {
	v[0] = x3;
	v[1] = y3;
	glVertex2fv(v);
    } else {
	ax0 = x0;
	ay0 = y0;
	ax1 = (x0+x1)/2;
	ay1 = (y0+y1)/2;
	ax2 = (x0+2*x1+x2)/4;
	ay2 = (y0+2*y1+y2)/4;
	ax3 = midx;
	ay3 = midy;
	bezadapt(ax0,ay0,ax1,ay1,ax2,ay2,ax3,ay3,beztol);

	bx0 = midx;
	by0 = midy;
	bx1 = (x1+2*x2+x3)/4;
	by1 = (y1+2*y2+y3)/4;
	bx2 = (x2+x3)/2;
	by2 = (y2+y3)/2;
	bx3 = x3;
	by3 = y3;
	bezadapt(bx0,by0,bx1,by1,bx2,by2,bx3,by3,beztol);
    }
}

int 
isobjfnt(char *name)
{
    FILE *inf;
    int magic;

    inf = fopen(name,READ_MODE);
    if(!inf) {
	fprintf(stderr,"isobjfnt: can't open input file %s\n",name);
	return 0;
    }
#ifndef __LITTLE_ENDIAN
    fread(&magic,sizeof(int),1,inf);
#else
	magic = fget32(inf);
#endif
    fclose(inf);
    if(magic == OFMAGIC)
	return 1;
    else
	return 0;
}

objfnt*
readobjfnt(char *name, void *arena)
{
    FILE 	*inf;
    objfnt 	*fnt;
#ifdef N64
    objfnt64 	fnt64;
#endif
    short 	*sptr;
    int 	i;
    int 	magic;

    inf = fopen(name,READ_MODE);
    if(!inf) {
	fprintf(stderr,"readobjfnt: can't open input file %s\n",name);
	return 0;
    }
#ifndef __LITTLE_ENDIAN
    fread(&magic,sizeof(int), 1, inf);
#else
	magic = fget32(inf);
#endif
    if(magic != OFMAGIC) {
	fprintf(stderr,"readobjfnt: bad magic nuber\n");
	return 0;
    }

    fnt = (objfnt *)pfMalloc(sizeof(objfnt), arena);

#ifndef __LITTLE_ENDIAN
#ifdef N64
    fread(&fnt64, sizeof(objfnt64), 1, inf);
    fnt->type = fnt64.type;
    fnt->charmin = fnt64.charmin;
    fnt->charmax = fnt64.charmax;
    fnt->nchars = fnt64.nchars;
    fnt->scale = fnt64.scale;
#else	
    fread(fnt, sizeof(objfnt), 1, inf);
#endif
#else
    fnt->freeaddr = (void *)fget32(inf);
    fnt->type = fget16(inf);
    fnt->charmin = fget16(inf);
    fnt->charmax = fget16(inf);
    fnt->nchars = fget16(inf);
    fnt->scale = fget16(inf);
    fget16(inf); /* pad */
    fnt->chars = (void *)fget32(inf);
#endif

    fnt->freeaddr = 0;

    fnt->chars = (chardesc *)pfMalloc(fnt->nchars * sizeof(chardesc), arena);
    for(i=0; i<fnt->nchars; i++) 
    {
#ifndef __LITTLE_ENDIAN
#ifdef N64
	chardesc64	char64;
	fread(&char64, sizeof(chardesc64), 1, inf);
	fnt->chars[i].movex = char64.movex;
	fnt->chars[i].movey = char64.movey;
	fnt->chars[i].llx = char64.llx;
	fnt->chars[i].lly = char64.lly;
	fnt->chars[i].urx = char64.urx;
	fnt->chars[i].ury = char64.ury;
	fnt->chars[i].datalen = char64.datalen;
#else	
	fread(&fnt->chars[i], sizeof(chardesc), 1, inf);
#endif
#else
        fnt->chars[i].movex = fget16(inf);
        fnt->chars[i].movey = fget16(inf);
        fnt->chars[i].llx = fget16(inf);
        fnt->chars[i].lly = fget16(inf);
        fnt->chars[i].urx = fget16(inf);
        fnt->chars[i].ury = fget16(inf);
        fnt->chars[i].data = (void *)fget32(inf);
        fnt->chars[i].datalen = fget32(inf);
#endif
    }

    for(i=0; i<fnt->nchars; i++) 
    {
	if(fnt->chars[i].datalen > 0) 
	{
	    sptr = (short *) pfMalloc(fnt->chars[i].datalen, arena);
	    fnt->chars[i].data = sptr;
#ifndef __LITTLE_ENDIAN
	    fread(sptr, fnt->chars[i].datalen, 1, inf);
#else
            {
               int n = fnt->chars[i].datalen/2, k;
               for (k = 0; k < n; ++k)
                  sptr[k] = fget16(inf);
            }
#endif
	} 
	else
	    fnt->chars[i].data = 0;
    }
    calccharbboxes(fnt);

    fclose(inf);
    return fnt;
}

#ifndef __linux__
void 
writeobjfnt(char *name, objfnt *fnt)
{
    FILE *of;
    int i;
    int magic;
#ifdef N64
    objfnt64	fnt64;
#endif

    calccharbboxes(fnt);
    of = fopen(name,WRITE_MODE);
    if(!of) {
	fprintf(stderr,"writeobjfnt: can't open output file %s\n",name);
	return;
    }
    magic = OFMAGIC;
    fwrite(&magic, sizeof(int), 1, of);

#ifdef N64
    fnt64.type = fnt->type;
    fnt64.charmin = fnt->charmin;
    fnt64.charmax = fnt->charmax;
    fnt64.nchars = fnt->nchars;
    fnt64.scale = fnt->scale;
    fwrite(&fnt64, sizeof(objfnt64), 1, of);
#else	
    fwrite(fnt, sizeof(objfnt), 1, of);
#endif

    for(i=0; i<fnt->nchars; i++)
    {
#ifdef N64
	chardesc64	char64;

	char64.movex = fnt->chars[i].movex;
	char64.movey = fnt->chars[i].movey;
	char64.llx = fnt->chars[i].llx;
	char64.lly = fnt->chars[i].lly;
	char64.urx = fnt->chars[i].urx;
	char64.ury = fnt->chars[i].ury;
	char64.datalen = fnt->chars[i].datalen;
	fwrite(&char64, sizeof(chardesc64), 1, of);
#else	
	fwrite(&fnt->chars[i], sizeof(chardesc), 1, of);
#endif
    }
    
    for(i=0; i<fnt->nchars; i++) 
    {
	if(fnt->chars[i].datalen > 0) 
	    fwrite(fnt->chars[i].data, fnt->chars[i].datalen, 1, of);
    }
    fclose(of);
}
#endif

objfnt*
newobjfnt(int type,int charmin,int charmax,int fscale)
{
    objfnt *fnt;

    fnt = (objfnt *)mymalloc(sizeof(objfnt));
    fnt->freeaddr = 0;
    fnt->type = type;
    fnt->charmin = charmin;
    fnt->charmax = charmax;
    fnt->nchars = fnt->charmax-fnt->charmin+1;
    fnt->scale = fscale;
    fnt->chars = (chardesc *)mymalloc(fnt->nchars*sizeof(chardesc));
    bzero(fnt->chars,fnt->nchars*(int)sizeof(chardesc));
    return fnt;
}

void 
freeobjfnt(objfnt *fnt)
{
    int i;
    chardesc *cd;

    cd = fnt->chars;
    for(i=0; i<fnt->nchars; i++) {
	if(cd->data)
	    free(cd->data);
	cd++;
    }
    free(fnt->chars);
    free(fnt);
}

void 
addchardata(objfnt *fnt, int c, short *data, int nshorts)
{
    int index;
    chardesc *cd;

    index = chartoindex(fnt,c);
    if(index<0) {
	fprintf(stderr,"Addchardata bad poop\n");
	return;
    }
    cd = fnt->chars+index;
    fnt->freeaddr = 0;
    cd->datalen = nshorts*(int)sizeof(short);
    cd->data = (short *)mymalloc(cd->datalen);
    memcpy(cd->data,data,cd->datalen);
}

void 
addcharmetrics(objfnt *fnt, int c, int movex, int movey)
{
    int index;
    chardesc *cd;

    index = chartoindex(fnt,c);
    if(index<0) {
	fprintf(stderr,"Addcharmetrics bad poop\n");
	return;
    }
    cd = fnt->chars+index;
    cd->movex = movex;
    cd->movey = movey;
}

int 
drawobjchar(objfnt *fnt, int c)
{
    chardesc *cd;

    cd = getchardesc(fnt,c);
    if(!cd)
	return 0;
    if(cd->data) {
	switch(fnt->type) {
	case TM_TYPE:
	    tmesh_draw(cd->data);
	    break;
	case PO_TYPE:
	    if(dopoints)
		point_draw(cd->data,dopoints);
	    else
		poly_draw(cd->data);
	    break;
	case SP_TYPE:
	    spline_draw(cd->data);
	    break;
	default:
	    fprintf(stderr,"drawobjchar: bad obj font type\n");
	}
    }
    pfTranslate((float)cd->movex,(float)cd->movey,0.0f);
    return 1;
}

int 
drawobjcharbbox(objfnt *fnt, int c)
{
    chardesc *cd;

    cd = getchardesc(fnt,c);
    if(!cd)
	return 0;
/*
    if(cd->llx != NOBBOX) 
	drawrect((float)cd->llx,(float)cd->lly,(float)cd->urx,(float)cd->ury);
*/
    return 1;
}

int 
getcharadvance(objfnt *fnt, int c, int *dx, int *dy)
{
    int index;
    chardesc *cd;

    index = chartoindex(fnt,c);
    if(index<0) {
	*dx = 0;
	*dy = 0;
	return 0;
    } else {
	cd = fnt->chars+index;
	*dx = cd->movex;
	*dy = cd->movey;
	return 1;
    }
}

static float fontsize = 1.0f;
static float xpos, ypos;
static int nfonts;
static char *names[40];
static objfnt *objfnts[40];
static objfnt *curfont;

void 
fontsetsize(float size)
{
    fontsize = size;
}

void 
fontmoveto(float x, float y)
{
    xpos = x;
    ypos = y;
}

void 
fontrmoveto(float x, float y)
{
    xpos += x;
    ypos += y;
}

void 
fontptr(objfnt *fnt)
{
    curfont = fnt;
}

objfnt*
fontname(char *name)
{
    objfnt *fnt;
    int i;

    for(i=0; i<nfonts; i++) {
	if(strcmp(name,names[i]) == 0) {
	    curfont = objfnts[i];
	    return curfont;
	}
    }
    if(nfonts==256) {
	fprintf(stderr,"fontname(\"%s\"): too many fonts\n",name);
	return NULL;
    }
    fnt = readobjfnt(name, NULL);
    if(!fnt) {
	fprintf(stderr,"fontname: can't find font %s\n",name);
	return NULL;
    } else {
	objfnts[nfonts] = fnt;
	names[nfonts] = (char *)mymalloc(strlen(name)+1);
	memcpy(names[nfonts],name,strlen(name)+1);
	nfonts++;
	curfont = fnt;
	return curfont;
    }
}

void 
fontpurge(char *name)
{
    int i, k;

    for(i=0; i<nfonts; i++) {
	if(strcmp(name,names[i]) == 0) {
	    if (curfont == objfnts[i])	{
		if (nfonts > 0)
		    curfont = objfnts[0];
		else 
		    curfont = NULL;
	    }
	    free(objfnts[i]);	    /* XXXX */
	    free(names[i]);
	    for (k = i; k < nfonts - 1; k++) {
		objfnts[k] = objfnts[k+1];
		names[k] = names[k+1];
	    }
	    nfonts--;
	    return;
	}
    }
    fprintf (stderr, "fontfree: couldn't find font %s to delete!\n", name);
}

float
getstringwidth(objfnt *fnt, unsigned char *str)
{
    int width, w, dy;

    width = 0;
    while(*str) {
	getcharadvance(fnt,*str,&w,&dy); 
	str++;
	width += w;
    }
    return width;
}


float 
fontstringwidth(unsigned char *str)
{
    int width, w, dy;

    width = 0;
    while(*str) {
	getcharadvance(curfont,*str,&w,&dy); 
	str++;
	width += w;
    }
    return width*fontsize/curfont->scale;
}

int 
fontshow(unsigned char *str)
{
    int n;

    pfPushMatrix();
    pfTranslate(xpos,ypos,0.0f);
    xpos += fontstringwidth(str);
    pfScale(fontsize/curfont->scale,fontsize/curfont->scale,0.0f);
    n = 0;
    while(*str) {
	n += drawobjchar(curfont,*str); 
	str++;
    }
    pfPopMatrix();
    return n;
}

int 
bboxshow(unsigned char *str)
{
    int n;

    pfPushMatrix();
    pfTranslate(xpos,ypos,0.0f);
    xpos += fontstringwidth(str);
    pfScale(fontsize/curfont->scale,fontsize/curfont->scale,0.0f);
    n = 0;
    while(*str) {
	n += drawobjcharbbox(curfont,*str); 
	str++;
    }
    pfPopMatrix();
    return n;
}

char*
asciiname(int c)
{
    c -= MIN_ASCII;
    if(c>=0 && c<NASCII) {
	if(charlist[c].name)
	    return charlist[c].name+1;
	else
	    return 0;
    } else 
	return 0;
}

int 
hasvertdata(objfnt *fnt, int c)
{
    chardesc *cd;

    cd = getchardesc(fnt,c);
    if(cd->llx == NOBBOX)
	return 0;
    else
	return 1;
}

void 
fontcentershow(unsigned char *str)
{
    float width;
    
    width = fontstringwidth(str);
    fontrmoveto(-width/2.0f,0.0f);
    fontshow(str);
}

void 
fakechar(objfnt *fnt, int c, int width)
{
    short chardata[1];

    chardata[0] = PO_RET;
    addchardata(fnt,c,chardata,1);
    addcharmetrics(fnt,c,width,0);
}

void 
mergefont(objfnt *dest, objfnt *src)
{
    int c;
    chardesc *dcd, *scd;

    for(c=dest->charmin; c<=dest->charmax; c++) {
	if(charexists(src,c)) {
	    dcd = getchardesc(dest,c);
	    scd = getchardesc(src,c);
	    dcd->movex = scd->movex;
	    dcd->movey = scd->movey;
	    dcd->llx = scd->llx;
	    dcd->lly = scd->lly;
	    dcd->urx = scd->urx;
	    dcd->ury = scd->ury;
	    dcd->datalen = scd->datalen;
	    dcd->data = (short *)mymalloc(dcd->datalen);
	    memcpy(dcd->data,scd->data,dcd->datalen);
	}
    }
}

int 
charexists(objfnt *fnt, int c)
{
    chardesc *cd;

    cd=getchardesc(fnt,c);
    if(!cd) 
	return 0;
    else if(cd->data)
	return 1;
    else
	return 0;
}


static texfnt *curtfnt;
static unsigned char *fb;
static unsigned char *gptr;

/*
 *	get metric data into image
 *
 */
static void
initget(void)
{
    gptr = fb;
}

static void 
getbytes(unsigned char *buf, int n)
{
#ifdef __LITTLE_ENDIAN
   switch (n) {
   case 2: 
      buf[0] = gptr[1];
      buf[1] = gptr[0];
      gptr += 2;
      break;
   case 4:
      buf[0] = gptr[3];
      buf[1] = gptr[2];
      buf[2] = gptr[1];
      buf[3] = gptr[0];
      gptr += 4;
      break;
   default:
      fprintf(stderr, "getbytes: n=%d unexpected\n", n);
   }
#else
    while(n--) 
	 *buf++ = *gptr++;
#endif

}

static void 
fixrow(unsigned short *sptr, int n)
{
    while(n--) {
	/* set intensity to max */
#ifdef __LITTLE_ENDIAN
	*sptr = (*sptr<<8) | 0x00ff;
#else
	*sptr |= 0xff00; 
#endif
	sptr++;
    }
}

static void 
stoc(unsigned short *sptr, unsigned char *cptr, int n)
{
    while(n--) {
	if(n>=8) {
	    cptr[0] = sptr[0];
	    cptr[1] = sptr[1];
	    cptr[2] = sptr[2];
	    cptr[3] = sptr[3];
	    cptr[4] = sptr[4];
	    cptr[5] = sptr[5];
	    cptr[6] = sptr[6];
	    cptr[7] = sptr[7];
	    sptr+=8; 
	    cptr+=8;
	    n -= 7;
	} else {
	    *cptr++ = *sptr++;
	}
    }
}

texfnt*
readtexfnt(char *name, void *arena)
{
    texfnt *tfnt;
    IMAGE *image;
    unsigned char *cptr;
    unsigned short *sbuf, *sptr;
    short advancecell, xadvance;
    short llx, lly, urx, ury, ox, oy;
    int i, y, extralines;
    texchardesc *cd;

    tfnt = (texfnt *)pfMalloc(sizeof(texfnt), arena);
    image = iopen(name,"r");
    if(!image) {
	 fprintf(stderr,"textmap: can't open font image %s\n",name);
	 exit(1);
    }
    extralines = image->ysize-image->xsize;
    if(extralines<1) {
	 fprintf(stderr,"textmap: bad input font!!\n");
	 exit(1);
    }
    fb = (unsigned char *)pfMalloc(image->xsize*extralines, arena);
    sbuf = (unsigned short *)pfMalloc(image->xsize*sizeof(short), arena);
    cptr = fb;
    for(y=image->xsize; y<image->ysize; y++) {
	getrow(image,sbuf,y,0);
	stoc(sbuf,cptr,image->xsize);
	cptr += image->xsize;
    }
    initget();
    tfnt->rasxsize = image->xsize;
    tfnt->rasysize = image->xsize;
    getbytes((unsigned char*)&tfnt->charmin,sizeof(short));
    getbytes((unsigned char*)&tfnt->charmax,sizeof(short));
    getbytes((unsigned char*)&tfnt->pixhigh,sizeof(float));
    getbytes((unsigned char*)&advancecell,sizeof(short));
    tfnt->nchars = tfnt->charmax-tfnt->charmin+1;
    tfnt->chars = (texchardesc *)pfMalloc(tfnt->nchars*sizeof(texchardesc), arena);
    tfnt->rasdata = (unsigned short *)pfMalloc(tfnt->rasxsize*tfnt->rasysize*sizeof(unsigned short), arena);
    sptr = tfnt->rasdata;
    for(y=0; y<tfnt->rasysize; y++) {
	getrow(image,sptr,y,0);
	fixrow(sptr,tfnt->rasxsize);
	sptr += tfnt->rasxsize;
    }
    iclose(image);

    cd = tfnt->chars;
    for(i=0; i<tfnt->nchars; i++) {
	getbytes((unsigned char*)&xadvance,sizeof(short));
	getbytes((unsigned char*)&llx,sizeof(short));
	getbytes((unsigned char*)&lly,sizeof(short));
	getbytes((unsigned char*)&urx,sizeof(short));
	getbytes((unsigned char*)&ury,sizeof(short));
	getbytes((unsigned char*)&ox,sizeof(short));
	getbytes((unsigned char*)&oy,sizeof(short));
	cd->movex = xadvance/(float)advancecell;

	if(llx>=0) {
	    cd->haveimage = 1;
	    cd->llx = (llx-ox)/tfnt->pixhigh;
	    cd->lly = (lly-oy)/tfnt->pixhigh;
	    cd->urx = (urx-ox+1)/tfnt->pixhigh;
	    cd->ury = (ury-oy+1)/tfnt->pixhigh;
	    cd->tllx = llx/(float)tfnt->rasxsize;
	    cd->tlly = lly/(float)tfnt->rasysize;
	    cd->turx = (urx+1)/(float)tfnt->rasxsize;
	    cd->tury = (ury+1)/(float)tfnt->rasysize;
	    cd->data[0] = cd->tllx;
	    cd->data[1] = cd->tlly;

	    cd->data[2] = cd->llx;
	    cd->data[3] = cd->lly;

	    cd->data[4] = cd->turx;
	    cd->data[5] = cd->tlly;

	    cd->data[6] = cd->urx;
	    cd->data[7] = cd->lly;

	    cd->data[8] = cd->turx;
	    cd->data[9] = cd->tury;

	    cd->data[10] = cd->urx;
	    cd->data[11] = cd->ury;

	    cd->data[12] = cd->tllx;
	    cd->data[13] = cd->tury;

	    cd->data[14] = cd->llx;
	    cd->data[15] = cd->ury;

	    cd->data[16] = cd->llx;
	    cd->data[17] = cd->lly;
	    cd->data[18] = cd->urx;
	    cd->data[19] = cd->lly;

	    cd->data[20] = cd->urx;
	    cd->data[21] = cd->ury;
	    cd->data[22] = cd->llx;
	    cd->data[23] = cd->ury;

	} else {
	    cd->haveimage = 0;
	}
	cd++;
    }
    free(fb);
    free(sbuf);
    return tfnt;
}


void 
texfont(texfnt *tfnt)
{
#ifdef WIN32
   glBindTexture(GL_TEXTURE_2D, 1);
#else
   glBindTextureEXT(GL_TEXTURE_2D, 1);
#endif

   glTexImage2D(GL_TEXTURE_2D, 0, 2, tfnt->rasxsize, tfnt->rasysize, 0,
		    GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tfnt->rasdata);

    glTexParameterf(GL_TEXTURE_2D,  GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,  GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /*Texture Enviroment Parameters*/
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    curtfnt = tfnt;
}

float 
texstrwidth(char *str)
{
    unsigned int c;
    int charmin, tnchars;
    texfnt *tfnt;
    texchardesc *cdbase, *cd;
    float xpos;

    tfnt = curtfnt;
    if(!tfnt) {
	fprintf(stderr,"texcharstr: no texfont set!!\n");
	return 0.0f;
    }
    charmin = tfnt->charmin;
    tnchars = tfnt->nchars;
    cdbase = tfnt->chars;
    xpos = 0.0f;
    while(*str) {
	c = *str-charmin;
	if(c<tnchars) {
	    cd = cdbase+c;
	    xpos += cd->movex;
	}
	str++;
    }
    return xpos;
}

void 
texcharstr(unsigned char *str)
{
    unsigned int c;
    int charmin, tnchars;
    texfnt *tfnt;
    texchardesc *cdbase, *cd;
    float *fdata, xpos;

    tfnt = curtfnt;
    if(!tfnt) {
	fprintf(stderr,"texcharstr: no texfont set!!\n");
	return;
    }
    charmin = tfnt->charmin;
    tnchars = tfnt->nchars;
    cdbase = tfnt->chars;
    xpos = 0.0f;
    if (str && *str)
    {
    glBegin(GL_QUADS);
    while(*str) {
	c = *str-charmin;
	if(c<tnchars) {
	    cd = cdbase+c;
	    if(cd->haveimage) {
		fdata = cd->data;
		fdata[16] = fdata[2]+xpos;
		fdata[18] = fdata[6]+xpos;
		fdata[20] = fdata[10]+xpos;
		fdata[22] = fdata[14]+xpos;
		glTexCoord2fv(&fdata[0]);
		glVertex2fv(&fdata[16]);
		glTexCoord2fv(&fdata[4]);
		glVertex2fv(&fdata[18]);
		glTexCoord2fv(&fdata[8]);
		glVertex2fv(&fdata[20]);
		glTexCoord2fv(&fdata[12]);
		glVertex2fv(&fdata[22]);
	    }
	    xpos += cd->movex;
	}
	str++;
    }
    glEnd();
    }
}






