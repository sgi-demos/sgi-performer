/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * This source code ("Source Code") was originally derived from a
 * code base owned by Silicon Graphics, Inc. ("SGI")
 * 
 * LICENSE: SGI grants the user ("Licensee") permission to reproduce,
 * distribute, and create derivative works from this Source Code,
 * provided that: (1) the user reproduces this entire notice within
 * both source and binary format redistributions and any accompanying
 * materials such as documentation in printed or electronic format;
 * (2) the Source Code is not to be used, or ported or modified for
 * use, except in conjunction with OpenGL Performer; and (3) the
 * names of Silicon Graphics, Inc.  and SGI may not be used in any
 * advertising or publicity relating to the Source Code without the
 * prior written permission of SGI.  No further license or permission
 * may be inferred or deemed or construed to exist with regard to the
 * Source Code or the code base of which it forms a part. All rights
 * not expressly granted are reserved.
 * 
 * This Source Code is provided to Licensee AS IS, without any
 * warranty of any kind, either express, implied, or statutory,
 * including, but not limited to, any warranty that the Source Code
 * will conform to specifications, any implied warranties of
 * merchantability, fitness for a particular purpose, and freedom
 * from infringement, and any warranty that the documentation will
 * conform to the program, or any warranty that the Source Code will
 * be error free.
 * 
 * IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
 * LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
 * ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
 * SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
 * OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
 * PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
 * OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
 * USE, THE SOURCE CODE.
 * 
 * Contact information:  Silicon Graphics, Inc., 
 * 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
 * or:  http://www.sgi.com
 */

/*
 * configfile.c
 */

#include "perfly.h"
#include "cmdline.h"
#include <Performer/pfdu.h>
#include <ctype.h>
#include <malloc.h>

#define streq(a,b) (strcmp(a,b) == 0)
#define strprefix(a,pre) (strncmp(a,pre,strlen(pre)) == 0)

#ifdef _PF_MULTI_VIEWSTATE
SharedViewStates *AllViewStates;
#endif

/* 
 * This code parses a configuration file to initialize perfly.
 * It it can be used instead of command line arguments,
 * and supports multiple views and vehicles as well.
 * XXX models specified in a config file are currently not shared
 * or optimized, so it's better to specify them on the command line.
 */

static int parseConfigFile(const char *filename, int execute);
static int parseConfigFileP(FILE *fp, char *end_string, int execute);

/* Read a string, optionally surrounded by single or double quotes
   (otherwise terminated by whitespace).
   Allows the special sequences \n, \t, \b, \r, \f,
   \0 (not followed by a digit) \<three digit octal number>
   Backslash followed by any other character represents that character
   (so that spaces, quotes, and backslashes can be embedded in any string).
   Returns TRUE on success, FALSE on error (including unexpected end-of-file),
   EOF on non-error end-of-file (or end-of-line if read_past_eol is FALSE).
 */
static int readWord(FILE *fp, char *buf, int bufsiz, int read_past_eol)
{
    int c, current_quotechar = '\0';

    if (bufsiz <= 0)
	return FALSE;

    /* discard spaces... */
    do
    {
	if ((c = getc(fp)) == EOF) return EOF;
	/* an unquoted # begins a comment that continues to end-of-line */
	if (c == '#')
	    do
		if ((c = getc(fp)) == EOF) return EOF;
	    while (c != '\n');
	if (!read_past_eol && c == '\n') return EOF;
	/* tolerate escaped newlines even if !read_past_eol */
	if (c == '\\')
	{
	    if ((c = getc(fp)) == EOF) return EOF;
	    if (c != '\n')
	    {
		(void)ungetc(c, fp);
		c = '\\';
	    }
	}
    }
    while (isspace(c));
    (void)ungetc(c, fp);

    while ((c = getc(fp)) != EOF)
    {
	if (current_quotechar != '\0')	/* in quotes */
	{
	    if (c == current_quotechar)
	    {
		current_quotechar = '\0';
		continue;
	    }
	    if (c == '\n')
		return FALSE;	/* no quoted strings across newlines */
	}
	else				/* not in quotes */
	{
	    /* an unquoted # begins a comment that continues to end-of-line */
	    if (c == '#')
		do
		    if ((c = getc(fp)) == EOF) return EOF;
		while (c != '\n');
	    if (c == '"' || c == '\'')
	    {
		current_quotechar = c;
		continue;
	    }
	    else if (isspace(c))
	    {
		ungetc(c, fp);
		if (bufsiz <= 0) { buf[-1] = '\0'; return FALSE; }
		*buf = '\0';
		return TRUE;
	    }
	}

	if (c == '\\')
	{
	    if ((c = getc(fp)) == EOF) return FALSE;
	    switch (c) {
		case 'n': c = '\n'; break;
		case 't': c = '\t'; break;
		case 'b': c = '\b'; break;
		case 'r': c = '\r'; break;
		case 'f': c = '\f'; break;
		case '\n':
			/* escaped newline is same as space */
			{
			    ungetc(' ', fp);
			    if (bufsiz <= 0) { buf[-1] = '\0'; return FALSE; }
			    *buf = '\0';
			    return TRUE;
			}
		default:
		    if (isdigit(c))
		    {
			int n = c;
			if ((c = getc(fp)) == EOF) return FALSE;
			if (isdigit(c))
			{
			    n = 8*n + (c-'0');
			    if ((c = getc(fp)) == EOF) return FALSE;
			    if (!isdigit(c)) return FALSE;
			    n = 8*n + (c-'0');
			}
			else
			    ungetc(c, fp); /* only one digit */
			c = n;
		    }
	    }
	}
	if (bufsiz <= 0) { buf[-1] = '\0'; return FALSE; }
	*buf++ = c;
	bufsiz--;
    }
    return FALSE;	/* unexpected end-of-file is an error */
}

/*
    Free the result of readArgv().
*/
static void freeArgv(char **argv)
{
    char **p;
    if (argv != NULL) {
	for (p = argv; *p != NULL; p++)
	    free(*p);
	free(argv);
    }
}

#define POWEROF2(x) (((x)&((x)-1))==0)

/*
    Read an argv from the file, up to a non-escaped newline.
    On success, returns TRUE and the argc, argv; the argv should
    be freed with freeArgv().
    On failure, returns FALSE and sets argc,argv to -1,NULL.
    If argv0 is not NULL, argv[0] is set to it and parsing starts at argv[1].
 */
static int readArgv(FILE *fp, const char *argv0, int *Argc, char ***Argv)
{
    char buf[1024];
    int val;
    int argc = 0;
    char **argv = malloc(2*sizeof(*argv));
    if (argv == NULL)
	goto error;

    while (argv0 ? (strcpy(buf, argv0), argv0 = NULL, TRUE)
		 : (val = readWord(fp, buf, sizeof(buf), 0)) == TRUE)
    {
	++argc;
	if (POWEROF2(argc))
	{
	    /* allocate enough space for any argc up to (but not including)
	       the next power of 2 */
	    char **newargv = realloc(argv, ((2*argc-1)+1) * sizeof(*argv));
	    if (newargv == NULL)
		goto error;
	    argv = newargv;
	}
	if ((argv[argc-1] = malloc(strlen(buf)+1)) == NULL)
	    goto error;
	strcpy(argv[argc-1], buf);
	argv[argc] = NULL;
    }
    if (val == FALSE)
	goto error;
    *Argc = argc;
    *Argv = argv;
    return TRUE;
error:
    freeArgv(argv);
    *Argc = -1;
    *Argv = NULL;
    return FALSE;
}

static int include(FILE *fp, int execute)
{
    char nameBuf[1024];
    if (readWord(fp, nameBuf, sizeof(nameBuf), 0) != TRUE)
	return FALSE;
    return parseConfigFile(nameBuf, execute);
}

static int cmdline(FILE *fp, int execute)
{
    int argc;
    char **argv;
    char **files;
    if (!readArgv(fp, "cmdline", &argc, &argv) || argc < 1)
    {
	freeArgv(argv);
	return FALSE;
    }

    /* if in initialization pass, just init converters and return */
    if (!execute)
    {
	/* save and restore the global ViewState, since we don't want it
	   to be altered by initialization */
	SharedViewState temp = *ViewState;
	char **files;
	int saved_InitXYZ = InitXYZ;
	int saved_InitHPR = InitHPR;
	int saved_InitFOV = InitFOV;
	(void)processCmdLine(argc, argv, &files);
	*ViewState = temp;
	InitXYZ = saved_InitXYZ;
	InitHPR = saved_InitHPR;
	InitFOV = saved_InitFOV;

	while (*files != NULL)
	    pfdInitConverter(*files++);
	freeArgv(argv);
	return TRUE;
    }

    (void)processCmdLine(argc, argv, &files);
    while (*files != NULL)
	pfAddChild(ViewState->sceneGroup, pfdLoadFile(*files++));
	/* XXX doesn't get optimized or shared */
    freeArgv(argv);
    return TRUE;
}

#ifdef _PF_MULTI_VIEWSTATE
/*
 * Adds a new scene to the global scene list, with the given name.
 * Calls processCmdLine to parse ViewState parameters and models,
 * then reads location and vehicle commands up to the token "end_scene".
 */
static int scene(FILE *fp, int execute)
{
    void *arena = pfGetSharedArena();
    int returnval;
    extern int InitXYZ, InitHPR, InitFOV;
    int saved_InitXYZ, saved_InitHPR, saved_InitFOV;
    int saved_NumFiles;
    char **saved_DatabaseFiles;
    int argc;
    char **argv;
    if (!readArgv(fp, NULL, &argc, &argv) || argc < 1)
    {
	freeArgv(argv);
	return FALSE;
    }

    {
	int i;
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		 "Argv for scene named '%s':\n", argv[0]);
	for (i = 0; i < argc; ++i)
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "   %3d: '%s'\n", i, argv[i]);
    }

    /* if in initialization pass, just init converters and return */
    if (!execute)
    {
	/* save and restore the global ViewState, since we don't want it
	   to be altered by initialization */
	SharedViewState temp = *ViewState;
	char **files;
	saved_InitXYZ = InitXYZ;
	saved_InitHPR = InitHPR;
	saved_InitFOV = InitFOV;
	(void)processCmdLine(argc, argv, &files);
	*ViewState = temp;
	InitXYZ = saved_InitXYZ;
	InitHPR = saved_InitHPR;
	InitFOV = saved_InitFOV;

	while (*files != NULL)
	    pfdInitConverter(*files++);
	freeArgv(argv);
	return parseConfigFileP(fp, "end_scene", execute);
    }

    /* we are being called from initSceneGraph() which is
       iterating through the global NumFiles and DatabaseFiles,
       so we need to save those values and restore them
       after possible recursive calls to initSceneGraph()
       from InitScene() which we call below.
     */
    saved_NumFiles = NumFiles;
    saved_DatabaseFiles = DatabaseFiles;
    saved_InitXYZ = InitXYZ;
    saved_InitHPR = InitHPR;
    saved_InitFOV = InitFOV;

    /* AllViewStates[0] is used to save initial state... */
    if (AllViewStates->nstates == 0)
	AllViewStates->states[AllViewStates->nstates++] = (SharedViewState *)
		pfMalloc(sizeof(SharedViewState), arena);
    *AllViewStates->states[0] = *ViewState;

    NumFiles = processCmdLine(argc, argv, &DatabaseFiles);

    InitScene(); /* uses global ViewState, NumFiles, DatabaseFiles */

    returnval = parseConfigFileP(fp, "end_scene", execute);

    /* Append the modified ViewState to the array of execute */
    if (AllViewStates->nstates == MAXVIEWSTATES)
	pfNotify(PFNFY_FATAL, PFNFY_PRINT,
		 "More than %d scenes?  Increase MAXVIEWSTATES in %s",
		 MAXVIEWSTATES, __FILE__);
    AllViewStates->states[AllViewStates->nstates] = (SharedViewState *)
	    pfMalloc(sizeof(SharedViewState), arena);
    *AllViewStates->states[AllViewStates->nstates] = *ViewState;
    strncpy(AllViewStates->statenames[AllViewStates->nstates], argv[0], 256);
    AllViewStates->statenames[AllViewStates->nstates][255] = '\0';
    AllViewStates->nstates++;

    freeArgv(argv);

    /* restore globals */
    *ViewState = *AllViewStates->states[0];
    NumFiles = saved_NumFiles;
    DatabaseFiles = saved_DatabaseFiles;
    InitXYZ = saved_InitXYZ;
    InitHPR = saved_InitHPR;
    InitFOV = saved_InitFOV;

    return returnval;
}
#endif /* _PF_MULTI_VIEWSTATE */

static int loc(FILE *fp, int execute)
{
    char nameBuf[1024];
    pfCoord coord;
    if (readWord(fp, nameBuf, sizeof(nameBuf), 0) != TRUE)
	return FALSE;
    if (fscanf(fp, "%f %f %f   %f %f %f",
	       &coord.xyz[0], &coord.xyz[1], &coord.xyz[2],
	       &coord.hpr[0], &coord.hpr[1], &coord.hpr[2]) != 6)
	return FALSE;

    if (!execute)
	return TRUE; /* this is the initialization pass; just parse */

    if (strlen(nameBuf)+1 > sizeof(ViewState->initViewNames[0]))
	return FALSE;
    if (ViewState->numInitViews == MAX_LOCS)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "Only %d locations allowed per scene; \"%s\" discarded",
		 MAX_LOCS, nameBuf);
	return TRUE; /* a warning is enough */
    }

    if (ViewState->numInitViews == 0 && (InitXYZ || InitHPR))
    {
	/* A position has been specified on the command line--
	   leave it as "Default" index 0, and move on to index 1 */
	ViewState->numInitViews++;
    }

    strcpy(ViewState->initViewNames[ViewState->numInitViews], nameBuf);
    ViewState->initViews[ViewState->numInitViews] = coord;
    /* make sure initViews[0] won't get overwritten with auto position */
    if (ViewState->numInitViews == 0) {
	InitXYZ = 1;
	InitHPR = 1;
    }
    ViewState->numInitViews++;

    return TRUE;
}

typedef struct pfuPathAndTime {
    pfuPath *path;
    double time;
} pfuPathAndTime;

/* callback to update vehicle position */
static int vehicleDCSAppFunc(pfTraverser *trav, void *data)
{
    pfDCS *dcs = (pfDCS *)pfGetTravNode(trav);
    pfuPathAndTime *pathAndTime = (pfuPathAndTime *)data;
    pfuPath *path = pathAndTime->path;
    double prevTime = pathAndTime->time;
    double thisTime = pfGetFrameTimeStamp();
    pfCoord coord;
    if (prevTime == 0.)
	prevTime = thisTime - .0001;
    pfuFollowPath(path, thisTime - prevTime, coord.xyz, coord.hpr);
    pathAndTime->time = thisTime;
    pfDCSCoord(dcs, &coord);
    return PFTRAV_CONT;
}

static int tetherview(FILE *fp, int execute)
{
    char nameBuf[1024], paramsbuf[1024];
    pfCoord coord;

    if (readWord(fp, nameBuf, sizeof(nameBuf), 0) != TRUE)
	return FALSE;
    /* initialize so that trailing values may be omitted */
    pfSetVec3(coord.xyz, 0.0f, 0.0f, 0.0f);
    pfSetVec3(coord.hpr, 0.0f, 0.0f, 0.0f);
    if (fgets(paramsbuf, sizeof(paramsbuf), fp) != NULL)
	sscanf(paramsbuf, "%f %f %f %f %f %f",
		&coord.xyz[0],
		&coord.xyz[1],
		&coord.xyz[2],
		&coord.hpr[0],
		&coord.hpr[1],
		&coord.hpr[2]);

    /* if in initialization pass, just return */
    if (!execute)
	return TRUE;

    if (strlen(nameBuf)+1 > sizeof(ViewState->tetherViewNames[0]))
	return FALSE;
    if (ViewState->numTetherViews == MAX_TETHERVIEWS)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "Only %d tether views allowed per scene; \"%s\" discarded",
		 MAX_TETHERVIEWS, nameBuf);
	return TRUE; /* a warning is enough */
    }

    pfMakeCoordMat(ViewState->tetherViews[ViewState->numTetherViews],
		   &coord);
    strcpy(ViewState->tetherViewNames[ViewState->numTetherViews], nameBuf);
    ViewState->numTetherViews++;
    return TRUE;
}

static int vehicle(FILE *fp, int execute)
{
    char nameBuf[1024], objnameBuf[1024], pathnameBuf[1024], paramsbuf[1024];
    pfDCS *thisDCS;	/* transform for current path position */
    pfSCS *thisSCS;	/* object aligning & scale */
    pfNode *thisVehicle;
    pfuPathAndTime *thisPathAndTime;
    pfCoord thisSCScoord;
    float thisSCSscale;
    pfMatrix thisSCSmatrix;

    if (readWord(fp, nameBuf, sizeof(nameBuf), 0) != TRUE)
	return FALSE;
    if (readWord(fp, objnameBuf, sizeof(objnameBuf), 0) != TRUE)
	return FALSE;
    if (readWord(fp, pathnameBuf, sizeof(pathnameBuf), 0) != TRUE)
	return FALSE;

    /* initialize so that trailing values may be omitted */
    pfSetVec3(thisSCScoord.xyz, 0.0f, 0.0f, 0.0f);
    pfSetVec3(thisSCScoord.hpr, 0.0f, 0.0f, 0.0f);
    thisSCSscale = 1.0f;

    if (fgets(paramsbuf, sizeof(paramsbuf), fp) != NULL)
	sscanf(paramsbuf, "%f %f %f %f %f %f %f",
		&thisSCScoord.xyz[0],
		&thisSCScoord.xyz[1],
		&thisSCScoord.xyz[2],
		&thisSCScoord.hpr[0],
		&thisSCScoord.hpr[1],
		&thisSCScoord.hpr[2],
		&thisSCSscale);

    /* if in initialization pass, just init converter and return */
    if (!execute)
    {
	pfdInitConverter(objnameBuf);
	return TRUE;
    }

    if (strlen(nameBuf)+1 > sizeof(ViewState->initViewNames[0]))
	return FALSE;
    if (ViewState->numVehicles == MAX_VEHICLES)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "Only %d vehicles allowed per scene; \"%s\" discarded",
		 MAX_VEHICLES, nameBuf);
	return TRUE; /* a warning is enough */
    }

    thisVehicle = pfdLoadFile(objnameBuf);
    if (thisVehicle == NULL)
    {
	/* error message printed already... */
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "loadConfigFile: Not adding vehicle \"%s\"", nameBuf);
	return TRUE;	/* not a fatal error */
    }

    thisPathAndTime = (pfuPathAndTime *)pfMalloc(sizeof(*thisPathAndTime),
						     NULL);
    if (thisPathAndTime == NULL)
    {
	pfDelete(thisVehicle);
	pfNotify(PFNFY_FATAL, PFNFY_PRINT,
		"loadConfigFile: pathAndTime allocation failed");
	return FALSE;
    }
    thisPathAndTime->time = 0.;
    thisPathAndTime->path = pfuNewPath();
    if (thisPathAndTime == NULL)
    {
	pfDelete(thisVehicle);
	pfDelete(thisPathAndTime);
	return FALSE;	/* error message printed already */
    }
    
    if (pfuAddFile(thisPathAndTime->path, pathnameBuf) < 0)
    {
	/* error message printed already... */
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "loadConfigFile: Not adding vehicle \"%s\"", nameBuf);
	pfDelete(thisVehicle);
	/* pfuDeletePath(path); XXX function does not exist! */
	return TRUE;	/* not a fatal error */
    }

    pfMakeCoordMat(thisSCSmatrix, &thisSCScoord);
    pfPreScaleMat(thisSCSmatrix, thisSCSscale, thisSCSscale, thisSCSscale, 
		  thisSCSmatrix);

    thisDCS = pfNewDCS();
    thisSCS = pfNewSCS(thisSCSmatrix);
    pfAddChild(ViewState->sceneGroup, thisDCS);
    pfAddChild(thisDCS, thisSCS);
    pfAddChild(thisSCS, thisVehicle);

    pfNodeTravFuncs(thisDCS, PFTRAV_APP, vehicleDCSAppFunc, NULL);
    pfNodeTravData(thisDCS, PFTRAV_APP, thisPathAndTime);

    ViewState->vehicleDCSs[ViewState->numVehicles] = thisDCS;
    strcpy(ViewState->vehicleNames[ViewState->numVehicles], nameBuf);
    ViewState->numVehicles++;

    return TRUE;
}

/* ARGSUSED (suppress compiler warn) */
static int badToken(FILE *fp, int execute)
{
    return FALSE;
}


typedef struct {
    char *str;
    int (*parse)(FILE *fp, int execute);
    char *usage; /* for usage message */
} TokStr;


#define MAX_TOKEN_SIZE 256
/* 
 * None of the token strings in this array can be larger than
 * MAX_TOKEN_SIZE - 1
 */
static TokStr TokenStrings[] = {
    {"cmdline", cmdline, "<perfly command line options and models>"},
    {"include", include, "\"otherfile.perfly\""},
#ifdef _PF_MULTI_VIEWSTATE
    {"scene", scene, "\"Name of Scene\" [perfly flags] [models]\\n <commands> \\n end_scene"},
#endif /* _PF_MULTI_VIEWSTATE */
    {"loc", loc, "\"Name of Location\"  x y z  h p r (floats)"},
    {"vehicle", vehicle, "\"Name of Vehicle\" objfile.name pathfile.name [x y z h p r [scale]]"},
    {"tetherview", tetherview, "\"Name of Tether View\" [x y z [h p r]]"},

    {"", badToken, "invalid token name"},
/*    {"", ,""}, */
};

static int
parseConfigFileP(FILE *fp, char *end_string, int execute)
{
    char str[1024];
    TokStr *token;
    for (;;)
    {
	switch (readWord(fp, str, sizeof(str), 1))
	{
	    case EOF: /* hit the end of the file */
		return end_string == NULL ? TRUE : FALSE;
	    case FALSE:
		fprintf(stderr, "Couldn't parse token\n");
		return FALSE;
	    case TRUE:
		if (end_string != NULL && streq(str, end_string))
		    return TRUE;

	        /* dumb but easy linear search */
	        for(token = TokenStrings;*token->str; token++)
		{
		    if(streq(str, token->str)) {
			if (!token->parse(fp, execute))
			{
			    pfNotify(PFNFY_WARN, PFNFY_USAGE,
				     "parseConfigFile: Error while parsing or executing command \"%s\".\n", str);
			    pfNotify(PFNFY_WARN, PFNFY_MORE, "Usage: %s %s\n",
					    str, token->usage);
			    return FALSE;
			}
			break;
		    }
	        }
		if (!*token->str) /* i.e. if it wasn't found in the for loop */
		{
		    fprintf(stderr, "Invalid token: %s\n", str);
		    return FALSE;
		}
	}
    }
}

/*
 * Return a value indicating sucess or failure
 * parse the configuration file given, and set the ViewState directly.
 * If ViewState is NULL, then this is the initialization pass,
 * which means just call pfdInitConverter() for each loadable
 * file referred to in the config file.
 */
static int parseConfigFile(const char *fname, int execute)
{
    FILE *fp;
    int returnval;

    if((fp = pfdOpenFile(fname)) == NULL)
	return FALSE;
    
    returnval = parseConfigFileP(fp, NULL, execute);

    (void)fclose(fp);

    return returnval;
}

extern int loadConfigFile(const char *file)
{
    int retval = parseConfigFile(file,1);
    if (!retval)
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "Loading config file \"%s\" failed.", file);
    return retval;
}

extern void initConfigFile(const char *file)
{
#ifdef _PF_MULTI_VIEWSTATE
    if (AllViewStates == NULL)
    {
	void *arena = pfGetSharedArena();
	AllViewStates = pfMalloc(sizeof(*AllViewStates), arena);
	AllViewStates->nstates = 0;
	AllViewStates->states = (SharedViewState **)
	   pfMalloc(MAXVIEWSTATES * sizeof(*AllViewStates->states), arena);
	AllViewStates->statenames = (char (*)[256])
	   pfMalloc(MAXVIEWSTATES * sizeof(*AllViewStates->statenames), arena);
	strcpy(AllViewStates->statenames[0], "Default");
    }
#endif
    if (!parseConfigFile(file, NULL))
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "Loading config file \"%s\" failed.", file);
}
