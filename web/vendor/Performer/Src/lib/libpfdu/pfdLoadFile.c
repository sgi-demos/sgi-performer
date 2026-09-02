/*
 * pfdLoadFile.c
 *
 * $Revision: 1.108 $
 * $Date: 2002/10/19 00:18:59 $
 *
 * Load a database file into an OpenGL Performer in-memory data 
 * structure using the filename's extension to intuit the file 
 * format. The file is sought in the directories named in the
 * active performer file search path (see pfFilePath for more
 * details). 
 *
 *
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
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <dlfcn.h>
#endif
#ifdef __linux__
#include <limits.h>   /* for PATH_MAX */
#endif
#ifdef WIN32
#include <ctype.h>
#include <io.h>
#define PATH_MAX _MAX_PATH /* in <stdlib.h> */
#endif
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

#ifdef WIN32
#define RTLD_LAZY 0 /* not used ... */
#define DLLERROR_STRLEN 1024
static TCHAR errorStr[DLLERROR_STRLEN + 1];

static
const char *dlerror()
{
  if(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
		   FORMAT_MESSAGE_IGNORE_INSERTS,
		   NULL,
		   GetLastError(),
		   MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
		   (LPTSTR)/*&*/errorStr,
		   DLLERROR_STRLEN, 
		   NULL))
    return errorStr;
  else
    return "unable to determine error";
}
#endif

#ifdef WIN32
#define DSO_EXTENSION ".dll"
#define PATH_SEPARATOR_CHAR ';'
#define DIRECTORY_SEPARATOR_CHAR '\\'
#else /* unix */
#define DSO_EXTENSION ".so"
#define PATH_SEPARATOR_CHAR ':'
#define DIRECTORY_SEPARATOR_CHAR '/'
#endif

typedef void		(*initFunc)(void);

typedef pfNode* 	(*loadFunc)(const char *fileName);
typedef int	 	(*storeFunc)(pfNode *, const char *fileName);

typedef void	 	(*loadNeededDSOsFunc)(const char *fileName);

typedef pfNode*	 	(*convertFromFunc)(void*);
typedef void*	 	(*convertToFunc)(pfNode*);

typedef void		(*setConverterModeFunc)(int mode, int val);
typedef int		(*getConverterModeFunc)(int mode);
typedef void		(*setConverterAttrFunc)(int which, void *attr);
typedef void*		(*getConverterAttrFunc)(int which);
typedef void		(*setConverterValFunc)(int which, float val);
typedef float		(*getConverterValFunc)(int which);

typedef struct
{
    void *			dso;
    initFunc			initConverter;
    loadFunc			loadFile;
    storeFunc			storeFile;
    loadNeededDSOsFunc		loadNeededDSOs;
    convertToFunc		convertTo;
    convertFromFunc		convertFrom;
    setConverterModeFunc	setMode;
    getConverterModeFunc	getMode;
    setConverterAttrFunc	setAttr;
    getConverterAttrFunc	getAttr;
    setConverterValFunc		setVal;
    getConverterValFunc		getVal;
    char *			name;
} ConverterDSO;

#define MAX_DSO_COUNT 128

static int		nDSOs = 0;
static ConverterDSO	DSO[MAX_DSO_COUNT];


/*
 * FUNCTION:
 *	pfdExitConverter() 
 *
 * DESCRIPTION:
 *	Discard any database loaders that have been dynamically
 *	linked into the current executable. The only reason to
 *	call this function is to save swap space.
 */

extern PFDUDLLEXPORT int
pfdExitConverter (const char *ext)
{
    int		src, dst;
    char	*tmp;
    
    /* look for final "." in filename */
    if ((tmp = strrchr(ext, '.')) != NULL)
	ext = tmp+1;
    
    
    /* release all file-loader DSOs */
    for (dst = 0, src = 0; src < nDSOs;)
    {
	if (ext == NULL || strcmp(ext, DSO[src].name) == 0)
	{
	    if (DSO[src].dso != NULL)
#ifndef WIN32
		dlclose(DSO[src].dso);
#else
                FreeLibrary(DSO[src].dso);
#endif
	    free(DSO[src].name);
	    
	}
	else
	{
	    if (dst < src)
		DSO[dst] = DSO[src];
	    dst++;
	}
	src++;
    }
    
    nDSOs -= (src - dst);
    /* return number of loaders released */
    return (src - dst);
}

/* XXX - aliases are a hack until more general performer environment is available */
static pfList *ExtList = NULL;
static pfList *ExtAliasList = NULL;
static int AliasFirstTime = 1;

extern PFDUDLLEXPORT void
pfdInitAliases(void)
{
    AliasFirstTime = 0;
    ExtList = pfNewList(sizeof(const char *), 8, pfGetSharedArena());
    ExtAliasList = pfNewList(sizeof(const char *), 8, pfGetSharedArena());
    pfdAddExtAlias("ascii","phd");
    pfdAddExtAlias("env","iv");
    pfdAddExtAlias("vrml","wrl");
    pfdAddExtAlias("default","iv");
    pfdAddExtAlias("stl","stlb");
}

extern PFDUDLLEXPORT void
pfdAddExtAlias(const char *ext, const char *alias)
{
    char *tmp;
    if (AliasFirstTime)
	pfdInitAliases();
    
    /* look for final "." in filename */
    if ((tmp = strrchr(ext, '.')) != NULL)
	ext = tmp+1;
    
    pfAdd(ExtList, strdup(ext));
    pfAdd(ExtAliasList, strdup(alias));
}

extern PFDUDLLEXPORT const char *
pfdGetExtAlias(const char *ext)
{
    int i;
    char *tmp;

    if (AliasFirstTime)
	pfdInitAliases();

    /* look for final "." in filename */
    if ((tmp = strrchr(ext, '.')) != NULL)
	ext = tmp+1;

    for(i=0;i<pfGetNum(ExtList);i++)
	if (strcmp(pfGet(ExtList, i), ext) == 0)
	    return pfGet(ExtAliasList, i);
    return ext;
}

/*
 * For every dir in path, call sgidlopen(dir/filename).
 * Return the first one that succeeds.
 * Do NOT call this with an absolute pathname;
 * it will do something nonsensical.
 */
static void *
dlopen_version_path(const char *path,
		    const char *filename, int mode,
		    const char *version,
		    int flags, int verbose,
		    char fullname[PATH_MAX])
{
    void *handle;
    const char *dir;

    fullname[0] = '\0';

    for (dir = path; *dir; dir = path) {
	while (*path != '\0' && *path != PATH_SEPARATOR_CHAR)
	    path++;

	if (path - dir > 0 &&
	    path - dir + 1 + strlen(filename) + 1 <= PATH_MAX)
	{
	    (void) sprintf(fullname, "%.*s%c%s", path - dir, dir, DIRECTORY_SEPARATOR_CHAR, filename);
	    if (verbose)
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "pfdFindConverterDSO() - trying \"%s\" version \"%s\"",
			 fullname, version);

#ifndef mips
#ifdef WIN32
	    if(_access(fullname,4) != -1) // check for read permission
	      handle = LoadLibrary(fullname);
	    else
	      handle = NULL;
#else
	    handle = dlopen (fullname, mode);
#endif
            if (handle != NULL) 
		return handle;
            else 
                if(verbose)
		    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
			     "  dlopen said: %s\n", dlerror());	
#else
	    handle = sgidlopen_version(fullname, mode, version, flags);
	    if (handle != NULL) 
	    {
		/*
		 * The following is necessary due to an RLD bug
		 * that causes it to accept versionless DSOs...
		 * It happens that we actually want to accept them,
		 * but only for Performer2.0.
		 */
		char *got_version = sgigetdsoversion(filename);
		if (got_version == NULL || got_version[0] == '\0')
		{
		    pfNotify(PFNFY_WARN, PFNFY_PRINT,
			"Using loader DSO %s", fullname);
		    pfNotify(PFNFY_WARN, PFNFY_MORE,
			"   which has no version number;");
		    pfNotify(PFNFY_WARN, PFNFY_MORE,
		        "   please give it one by linking with ");
		    pfNotify(PFNFY_WARN, PFNFY_MORE,
			"   \"-set_version %s\"", version);
		}

		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "pfdFindConverterDSO() - using DSO \"%s\"", fullname);

		return handle;
	    }
#endif
	    if (verbose)
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "  dlopen said: %s\n", dlerror());
	}

	if(*path == PATH_SEPARATOR_CHAR)
	  path++;
    }
    return NULL;
}

static ConverterDSO *
_pfdFindConverterDSO(const char *fileName, int warnIfNotFound)
{
    ConverterDSO 	*cDSO		= NULL;
    const char	*s		= NULL;
    char        *newPath	= NULL;
    char 	*libraryPath	= NULL;
    char 	*pfLibraryPath	= NULL;
    char 	*rootPath	= NULL;
    char	*version	= NULL;
    char	*versionuse	= NULL;
    char	versionpart[80];
    int 	i;
    char	initFuncName[PF_MAXSTRING];
    char	loadFuncName[PF_MAXSTRING];
    char	storeFuncName[PF_MAXSTRING];
    char	loadNeededDSOsFuncName[PF_MAXSTRING];
    char	convertToFuncName[PF_MAXSTRING];
    char	convertFromFuncName[PF_MAXSTRING];
    char	setConverterModeFuncName[PF_MAXSTRING];
    char	getConverterModeFuncName[PF_MAXSTRING];
    char	setConverterAttrFuncName[PF_MAXSTRING];
    char	getConverterAttrFuncName[PF_MAXSTRING];
    char	setConverterValFuncName[PF_MAXSTRING];
    char	getConverterValFuncName[PF_MAXSTRING];
    char	dsoName[PF_MAXSTRING];
    char	dsoPath[PATH_MAX];
    char	*glStyle;
    int		verbose = FALSE;
    const char	*ext, *dsoext;
#ifdef WIN32
    HKEY        hKey = NULL;
    char        thePath[PF_MAXSTRING+1];
    DWORD       strSize = PF_MAXSTRING;
#endif
    
    /* get the loader type from the file's extension */
    
    /* look for final "." in filename */
    if ((ext = strrchr(fileName, '.')) == NULL)
	/* no dot, assume it's just the extension */
	ext = fileName;
    else 
  	/* advance "ext" past the period character */
	++ext;
    
    if (strlen(ext) < 1)
      pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdFindConverterDSO() - Trying default file type extension. "
		 "Conversion may not be possible!");
    
    ext = pfdGetExtAlias(ext);
    
    /* is the loader for this file type already available ? */
    for (i = 0; i < nDSOs; i++)
	if (DSO[i].name && (strcmp(ext, DSO[i].name) == 0))
	    return &DSO[i];
    
    sprintf(initFuncName,"pfdInitConverter_%s", ext);
    sprintf(loadFuncName,"pfdLoadFile_%s", ext);
    sprintf(storeFuncName,"pfdStoreFile_%s", ext);
    sprintf(loadNeededDSOsFuncName,"pfdLoadNeededDSOs_%s", ext);
    sprintf(convertToFuncName,"pfdConvertTo_%s", ext);
    sprintf(convertFromFuncName,"pfdConvertFrom_%s", ext);
    sprintf(setConverterModeFuncName,"pfdConverterMode_%s", ext);
    sprintf(getConverterModeFuncName,"pfdGetConverterMode_%s", ext);
    sprintf(setConverterAttrFuncName,"pfdConverterAttr_%s", ext);
    sprintf(getConverterAttrFuncName,"pfdGetConverterAttr_%s", ext);
    sprintf(setConverterValFuncName,"pfdConverterVal_%s", ext);
    sprintf(getConverterValFuncName,"pfdGetConverterVal_%s", ext);
    
    /* is the loader for this file type linked into the executable ? */
    cDSO = &DSO[PF_MIN2(nDSOs, MAX_DSO_COUNT-1)];
    
#ifndef WIN32
    cDSO->dso = dlopen(NULL, RTLD_LAZY);
#else
    cDSO->dso = GetModuleHandle(NULL);
#define dlsym(a,b) GetProcAddress((a),(b))
#endif
    if (cDSO->dso != NULL) {
      cDSO->initConverter  = (initFunc)	dlsym(cDSO->dso, initFuncName);
      cDSO->loadFile  = (loadFunc)		dlsym(cDSO->dso, loadFuncName);
      cDSO->storeFile  = (storeFunc)		dlsym(cDSO->dso, storeFuncName);
      cDSO->loadNeededDSOs = (loadNeededDSOsFunc)dlsym(cDSO->dso, loadNeededDSOsFuncName);
      cDSO->convertTo  = (convertToFunc)	dlsym(cDSO->dso, convertToFuncName);
      cDSO->convertFrom  = (convertFromFunc)	dlsym(cDSO->dso, convertFromFuncName);
      cDSO->setMode = (setConverterModeFunc)	dlsym(cDSO->dso, setConverterModeFuncName);
      cDSO->getMode = (getConverterModeFunc)	dlsym(cDSO->dso, getConverterModeFuncName);
      cDSO->setAttr = (setConverterAttrFunc)	dlsym(cDSO->dso, setConverterAttrFuncName);
      cDSO->getAttr = (getConverterAttrFunc)	dlsym(cDSO->dso, getConverterAttrFuncName);
      cDSO->setVal = (setConverterValFunc)	dlsym(cDSO->dso, setConverterValFuncName);
      cDSO->getVal = (getConverterValFunc)	dlsym(cDSO->dso, getConverterValFuncName);
      
      /* if initConverter exists, call it so the loader can execute appropriate setup */
      if (cDSO->initConverter != NULL) {
	(*cDSO->initConverter)();
      }
      
      if (cDSO->loadFile != NULL)
	{
	  /* if loadFile was not null, we actually do have the converter loaded in, so
	     increment converter count and return the dso. */
	  nDSOs++;
	  cDSO->name = strdup(ext);
	  return cDSO;
	}
    }
    
    /* at this point, loader is not linked into executable */

    if (!strcmp(ext, "pfa"))
    {
	/*
	 * Special case for pfa-- the pfa functions are included in
	 * the pfb DSO, but pfa is *not* an alias for pfb. So we have the
	 * function names right (pfa) but we want to look for the DSO
	 * using pfb instead.
	 */
	dsoext = "pfb";
    }
    else
	dsoext = ext;
    
    /* Must find a dso for this extension */
#ifdef WIN32
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, PF_REGISTRY_SUBKEY, (DWORD)NULL,
		 KEY_EXECUTE, &hKey) != ERROR_SUCCESS)
      hKey = NULL;
#endif
    
    /* get file search paths for use in locating loader */
    if (
#if defined(N64)
	(s = getenv("PFLD_LIBRARY64_PATH")) != NULL ||
#elif defined(N32)
	(s = getenv("PFLD_LIBRARYN32_PATH")) != NULL ||
#endif /* N32 */
	(s = getenv("PFLD_LIBRARY_PATH")) != NULL)
    {
	verbose = TRUE;
	pfLibraryPath = strdup(s);
    }
    else {
#ifndef WIN32
	pfLibraryPath = strdup("");
#else
      if(hKey &&
	 RegQueryValueEx(hKey, "PFLD Library Path", NULL, NULL, (LPBYTE)thePath, &strSize) ==
	 ERROR_SUCCESS)
	pfLibraryPath = strdup(thePath);
      else
	pfLibraryPath = strdup("");
#endif
    }

    
    /* get file search paths for use in locating loader */
    if (
#if defined(N64)
	(s = getenv("LD_LIBRARY64_PATH")) != NULL ||
#elif defined(N32)
	(s = getenv("LD_LIBRARYN32_PATH")) != NULL ||
#endif /* N32 */
        (s = getenv("LD_LIBRARY_PATH")) != NULL)
	libraryPath = strdup(s);
    else {
#ifndef WIN32
	libraryPath = strdup("");
#else
	if(hKey &&
	   RegQueryValueEx(hKey,"LD Library Path", NULL, NULL, (LPBYTE)thePath, &strSize) ==
	   ERROR_SUCCESS)
	  libraryPath = strdup(thePath);
	else
	  libraryPath = strdup("");
#endif
    }
    
    if ((s = getenv("PFHOME")) != NULL)
	rootPath = strdup(s);
    else {
#ifndef WIN32
	rootPath = strdup("");
#else
	if(hKey &&
	   RegQueryValueEx(hKey, "Home", NULL, NULL, (LPBYTE)thePath, &strSize) ==
	   ERROR_SUCCESS)
	  rootPath = strdup(thePath);
	else
	  rootPath = strdup("");	   
#endif
    }

#ifdef WIN32
    if(hKey)
      RegCloseKey(hKey);
#endif
    
    /* form new search path for locating loader */
    newPath = malloc(1024 + strlen(libraryPath) + 4*strlen(rootPath));
#if defined(N64)
    sprintf(newPath, 
	    "%s"
	    ":."
	    ":%s"
	    ":%s/usr/lib64/libpfdb"
	    ":%s/usr/share/Performer/lib/libpfdb",
	    pfLibraryPath, libraryPath, rootPath, rootPath);
#elif defined(N32)
    sprintf(newPath, 
	    "%s"
	    ":."
	    ":%s"
	    ":%s/usr/lib32/libpfdb"
	    ":%s/usr/share/Performer/lib/libpfdb",
	    pfLibraryPath, libraryPath, rootPath, rootPath);
#elif defined(WIN32)
    sprintf(newPath,
	    "%s"
	    ";."
	    ";%s"
	    ";%s\\lib\\libpfdb"
	    ";%s\\lib"
	    ";%s\\share\\Performer\\lib\\libpfdb"
	    ";%s\\lib\\Debug\\libpfdb"
	    ";%s\\lib\\Debug",
	    pfLibraryPath, libraryPath, rootPath, rootPath, rootPath, rootPath, rootPath);
#else /* is O32 */
    sprintf(newPath, 
	    "%s"
	    ":."
	    ":%s"
	    ":%s/usr/lib/libpfdb"
	    ":%s/libpfdb"
	    ":%s/usr/share/Performer/lib/libpfdb",
	    pfLibraryPath, libraryPath, rootPath, libraryPath, rootPath);
#endif /* O32 */

    free(pfLibraryPath);
    free(libraryPath);
    free(rootPath);

    /* print new search path */
    {
	char *body = strdup(newPath);
	char *head = body;
	char *tail = NULL;
	
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "");
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		 "pfdFindConverterDSO() - DSO search path is:");
	while (*head && (tail = strchr(head, PATH_SEPARATOR_CHAR)) != NULL)
	{
	    *tail = '\0';
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "  \"%s%c\"", head, PATH_SEPARATOR_CHAR);
	    head = tail + 1;
	}
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, "  \"%s\"", head);
	
	free(body);
    }
    
    /*
     * Get the DSO interface version of libpfdu.so
     */
#ifndef sgi
    sprintf(dsoName, "libpf%s"DSO_EXTENSION, dsoext);
    cDSO->dso = dlopen_version_path(newPath, dsoName, RTLD_LAZY, versionuse, 0, verbose, dsoPath);
#else
    version = sgigetdsoversion("libpfdu.so");
    if (version == NULL || version[0] == '\0')
    {
	version = "sgi5.0";
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		"pfdFindConverterDSO() - can't get version of libpfdu.so, "
		"using %s", version);
    }
    else
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		"pfdFindConverterDSO() - version of libpfdu.so is %s",
		version);
    }

    /* 
     * munging the returned version string to extract sgiX.x from 
     * sgiX.x:sgiX.x:sgiX.x..  
     *
     * we cannot strtok 'version' as it is memory used by sgigetdsoversion,
     * so we copy it into versionpart.  
     *
     */

    strncpy(versionpart, version, 80);
    versionpart[79]='\0';
    versionuse=strtok(versionpart,":");
    
    /*
     * look for both optimized and debug versions of loader
     * accept non gl specifier as an OpenGL loader
     */
    
    /* look for OPT (no extension) version of DSO */
    sprintf(dsoName, "libpf%s"DSO_EXTENSION, dsoext);
    cDSO->dso = dlopen_version_path(newPath, dsoName, RTLD_LAZY, versionuse, 0, verbose, dsoPath);
    if (cDSO->dso == NULL)
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
	    "pfdFindConverterDSO() - Did not find optimized DSO \"%s\"", dsoName);

    /* look for DBG ("-g" extension) version of DSO */
    if (cDSO->dso == NULL)
    {
        sprintf(dsoName, "libpf%s-g"DSO_EXTENSION, dsoext);
	cDSO->dso = dlopen_version_path(newPath, dsoName, RTLD_LAZY, versionuse, 0, verbose, dsoPath);
	if (cDSO->dso == NULL)
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		"pfdFindConverterDSO() - Did not find debug DSO \"%s\"", dsoName);
    }
#endif /* __linux__ */

    free(newPath);

    /*
     * try to load the file using the loader DSO
     */
    if (cDSO->dso != NULL)
    {
	/*
	 *  We have needed to load the Converter DSO.  If pfConfig()
	 *  has already been called this could cause a problem.
	 */
	if (pfIsConfiged())
	{
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		     "pfdFindConverterDSO(%s) called after pfConfig().",
		     fileName);
	    pfNotify(PFNFY_NOTICE, PFNFY_MORE,
		     "This might cause problems.");
	    pfNotify(PFNFY_NOTICE, PFNFY_MORE,
		     "Call pfdInitConverter(%s) before pfConfig() so that "
		     "all performer processes will have the converter dso "
		     "in their process space.", fileName);
	}

	nDSOs++;
	
	cDSO->initConverter  = (initFunc)		dlsym(cDSO->dso, initFuncName);
	cDSO->loadFile  = (loadFunc)		dlsym(cDSO->dso, loadFuncName);
	cDSO->storeFile  = (storeFunc)		dlsym(cDSO->dso, storeFuncName);
	cDSO->loadNeededDSOs = (loadNeededDSOsFunc)dlsym(cDSO->dso, loadNeededDSOsFuncName);
	cDSO->convertTo  = (convertToFunc)	dlsym(cDSO->dso, convertToFuncName);
	cDSO->convertFrom  = (convertFromFunc)	dlsym(cDSO->dso, convertFromFuncName);
	cDSO->setMode = (setConverterModeFunc)	dlsym(cDSO->dso, setConverterModeFuncName);
	cDSO->getMode = (getConverterModeFunc)	dlsym(cDSO->dso, getConverterModeFuncName);
	cDSO->setAttr = (setConverterAttrFunc)	dlsym(cDSO->dso, setConverterAttrFuncName);
	cDSO->getAttr = (getConverterAttrFunc)	dlsym(cDSO->dso, getConverterAttrFuncName);
	cDSO->setVal = (setConverterValFunc)	dlsym(cDSO->dso, setConverterValFuncName);
	cDSO->getVal = (getConverterValFunc)	dlsym(cDSO->dso, getConverterValFuncName);
	if (cDSO->initConverter != NULL)
	{
	    (*cDSO->initConverter)();
	}
	if (cDSO->loadFile != NULL)
	{
	    cDSO->name = strdup(ext);
	    return cDSO;
	}

	if (cDSO->loadFile == NULL)
	{
	    pfNotify(PFNFY_WARN, PFNFY_ASSERT,
		"pfdFindConverterDSO() - Function \"%s\" not defined in DSO \"%s\"", 
		    loadFuncName, dsoPath);
	}
	return cDSO;
    }

    if (!strcmp(ext, "iv"))
    {	
	/* 
	 * special case for Inventor loader.  If default loader fails 
	 * to load (probably because libInventor.so.3 was not present), 
	 * try the 2.0 version
	 */
	cDSO = _pfdFindConverterDSO("iv20", warnIfNotFound);
	if (cDSO != NULL)
	{
	    cDSO->name = "iv";
	    return cDSO;
	}
    }	

    pfNotify(warnIfNotFound ? PFNFY_WARN : PFNFY_DEBUG, PFNFY_PRINT,
	     "pfdFindConverterDSO() - Could not load DSO for extension \"%s\"", 
	     ext);

    return NULL;
}

extern PFDUDLLEXPORT ConverterDSO *
pfdFindConverterDSO(const char *fileName)
{
#ifdef WIN32 /* XXX Alex - get rid of this and use GetFullPathName() */
    int i;
    ConverterDSO* dso = NULL;
    char *x = fileName? strdup(fileName) : NULL;

    if(!x) return NULL;
    for(i=0; i<strlen(x); i++)
      x[i] = tolower(x[i]);
    dso = _pfdFindConverterDSO(x,1);
    free(x);
    return dso;
#else
    return _pfdFindConverterDSO(fileName, 1);
#endif
}

extern PFDUDLLEXPORT void
pfdConverterMode(const char *ext, int mode, int value)
{
    ConverterDSO *dso = pfdFindConverterDSO(ext);
    if (dso && dso->setMode)
	(*dso->setMode)(mode, value);
}

extern PFDUDLLEXPORT int
pfdGetConverterMode(const char *ext, int mode)
{
    ConverterDSO *dso = pfdFindConverterDSO(ext);
    if (dso && dso->setMode)
	return (*dso->getMode)(mode);
    return -1;
}

extern PFDUDLLEXPORT void
pfdConverterAttr(const char *ext, int which, void *attr)
{
    ConverterDSO *dso = pfdFindConverterDSO(ext);
    if (dso && dso->setAttr)
	(*dso->setAttr)(which, attr);
}

extern PFDUDLLEXPORT void *
pfdGetConverterAttr(const char *ext, int which)
{
    ConverterDSO *dso = pfdFindConverterDSO(ext);
    if (dso && dso->getAttr)
	return (*dso->getAttr)(which);
    return NULL;
}

extern PFDUDLLEXPORT void
pfdConverterVal(const char *ext, int which, float val)
{
    ConverterDSO *dso = pfdFindConverterDSO(ext);
    if (dso && dso->setVal)
	(*dso->setVal)(which, val);
}

extern PFDUDLLEXPORT float
pfdGetConverterVal(const char *ext, int which)
{
    ConverterDSO *dso = pfdFindConverterDSO(ext);
    if (dso && dso->getVal)
	return (*dso->getVal)(which);
    return 0.;
}

/*
 * FUNCTION:
 *	pfdPrintSceneGraphStats()
 *
 * DESCRIPTION:
 *	Print some simple statistics about the primitives in the
 *	scene graph, using pfNotify.
 *	The intent of this function is to modularize some code
 *	duplicated often in the loader functions themselves.
 */

extern PFDUDLLEXPORT void
pfdPrintSceneGraphStats (pfNode *node, double elapsedTime)
{
    double totalPrims;

    /* statistics information */
    static float ftmp[4];
    static uint qtmp[4] =
    {
        PFFSTATS_BUF_CUR | PFSTATSVAL_GFX_GEOM_TRIS,
        PFFSTATS_BUF_CUR | PFSTATSVAL_GFX_GEOM_LINES,
        PFFSTATS_BUF_CUR | PFSTATSVAL_GFX_GEOM_POINTS,
        NULL
    };

    /* only traverse if the scene graph loaded successfully */
    if (node != NULL)
    {
	int				numPrimTypes = 0;
        pfFrameStats            *fs = pfNewFStats();

        /* traverse scene graph and tabulate contents */
        pfuTravCountDB(node, fs);

        /* extract values from tabulated statistics */
        pfMQueryFStats(fs, qtmp, ftmp, 0);

        /* print information about scene graph contents */
        pfNotify(PFNFY_INFO, PFNFY_MORE, "  Scene-graph statistics:");
	if (ftmp[0] != 0)
	{
	    ++numPrimTypes;
	    pfNotify(PFNFY_INFO, PFNFY_MORE, "    Triangles:          %8d",
		(long)ftmp[0]);
	}
	if (ftmp[1] != 0)
	{
	    ++numPrimTypes;
	    pfNotify(PFNFY_INFO, PFNFY_MORE, "    Lines:              %8d",
		(long)ftmp[1]);
	}
	if (ftmp[2] != 0)
	{
	    ++numPrimTypes;
	    pfNotify(PFNFY_INFO, PFNFY_MORE, "    Points:             %8d",
		(long)ftmp[2]);
	}

	totalPrims = ftmp[0] + ftmp[1] + ftmp[2];
	if (numPrimTypes > 1)
	{
	    pfNotify(PFNFY_INFO, PFNFY_MORE, "    Total primitives:   %8d",
		(long)totalPrims);
	}

        /* print elapsed time and loading rate */
        if (elapsedTime > 0.0)
        {
            pfNotify(PFNFY_INFO, PFNFY_MORE,
		"    loading time:       %12.3f sec", elapsedTime);

            if (totalPrims > 0.0f)
                pfNotify(PFNFY_INFO, PFNFY_MORE, "    loading rate"
		    ":       %12.3f prims/sec", totalPrims/elapsedTime);
        }

	pfNotify(PFNFY_INFO, PFNFY_MORE, NULL);

        /* discard stats structure */
        pfDelete(fs);
    }
}


/*
 * FUNCTION:
 *	pfdInitConverter() 
 *
 * DESCRIPTION:
 *	If the loader for the specified extension is not already
 *	in the image, open the DSO now.  Useful for mapping loader
 *	routines into all processes before pfConfig().  Necessary
 *	if loaders contain functions (including destructors) that
 *	may be invoked in different Performer processes.
 */

extern PFDUDLLEXPORT int
pfdInitConverter(const char *_ext)
{
    char *ext;
    ConverterDSO *dso;

    if ((dso = pfdFindConverterDSO(_ext)) == NULL)
	return 0;

    if (dso && dso->loadNeededDSOs)
	(*dso->loadNeededDSOs)(_ext);

    /*
     * In the case of multiple extensions (e.g. foo.im.closest)
     * attempt to initialize the converter for all of the extensions
     * (but return 1 regardless of their success or failure,
     * since we already know the final one succeeded).
     */
    if ((ext = strrchr(_ext, '.')) != NULL)
    {
	char buf[PATH_MAX];
	strncpy(buf, _ext, ext-_ext);
	buf[ext-_ext] = '\0';
	while ((ext = strrchr(buf, '.')) != NULL)
	{
	    if (strrchr(ext, '/') != NULL)
		break;
	    dso = _pfdFindConverterDSO(buf, 0);
	    if (dso && dso->loadNeededDSOs)
		(*dso->loadNeededDSOs)(buf);
	    *ext = '\0';
	}
    }

    return 1;
}

/*
 * FUNCTION:
 *	pfdLoadFile() 
 *
 * DESCRIPTION:
 *	Load a database file into an OpenGL Performer in-memory data 
 *	structure using the filename's extension to intuit the file 
 *	format. The file is sought in the directories named in the
 *	active performer file search path (see pfFilePath for more
 *	details). 
 */

extern PFDUDLLEXPORT pfNode *
pfdLoadFile(const char *fileName)
{
    ConverterDSO *dso  = 	NULL;
    pfNode *node = 	NULL;
    double startTime;
    double elapsedTime;
    const char *ext;
#ifdef WIN32
    char *ext2;
    int i;
#endif

    /* look for final "." in filename */
    if ((ext = strrchr(fileName, '.')) == NULL)
	/* no dot, assume it's just the extension */
	ext = fileName;
    else 
  	/* advance "ext" past the period character */
	++ext;

#ifdef WIN32 /* XXX Alex -- get rid of this and use GetFullPathName() */
    ext2 = strdup(ext);
    if(ext2) {
      for(i=0; i<strlen(ext2); i++)
        ext2[i] = tolower(ext[i]); 
      ext = ext2;
    }
#endif

    /* Get the ConverterDSO struct for this extension */
    dso = pfdFindConverterDSO(fileName);

    if (dso && dso->loadFile != NULL)
    {
#ifdef	VERBOSE_NOTIFICATION
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile");
	pfNotify(PFNFY_NOTICE, PFNFY_MORE,  "  Loader name:           pfdLoadFile_%s", ext);
	pfNotify(PFNFY_NOTICE, PFNFY_MORE,  "  File name:             %s", fileName);
#endif

	/* Call loader and collect elapsed time */
	startTime = pfGetTime();
	node = (*dso->loadFile)(fileName);	/* invoke loader */
	elapsedTime = pfGetTime() - startTime;

	/* Print some simple statistics on the scene graph */
	pfdPrintSceneGraphStats(node, elapsedTime);

    }
    else
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadFile() - Unable to load file %s because of problem finding pfdLoadFile_%s",
		 fileName, ext);

    /* reset builder geometry and graphics state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

#ifdef WIN32
    if(ext2) free(ext2);
#endif

    /* return pointer to in-memory hierarchy */
    return node;
}


/*
 * FUNCTION:
 *	pfdStoreFile() 
 *
 * DESCRIPTION:
 *	Store a database file into an OpenGL Performer in-memory data 
 *	structure using the filename's extension to intuit the file 
 *	format. The file is sought in the directories named in the
 *	active performer file search path (see pfFilePath for more
 *	details). 
 */

extern PFDUDLLEXPORT int
pfdStoreFile(pfNode *root, const char *fileName)
{
    ConverterDSO *dso  = 	NULL;
    const char *ext;

    /* look for final "." in filename */
    if ((ext = strrchr(fileName, '.')) == NULL)
	/* no dot, assume it's just the extension */
	ext = fileName;
    else 
  	/* advance "ext" past the period character */
	++ext;

    /* Get the ConverterDSO struct for this extension */
    dso = pfdFindConverterDSO(fileName);

    if (dso == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdStoreFile() - No converter available for %s\n", fileName);
	return FALSE;
    }
	
    if (dso->storeFile == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdStoreFile() - Converter for %s does not provide pfdStoreFile_%s\n", ext, ext);
	return FALSE;
    }

    return dso->storeFile(root, fileName);
}


/*
 * FUNCTION:
 *	pfdConvertFrom() 
 *
 * DESCRIPTION:
 *	Load a database file into an OpenGL Performer in-memory data 
 *	structure from another in-memory representation
 */

extern PFDUDLLEXPORT pfNode *
pfdConvertFrom(void *root, const char *ext)
{
    ConverterDSO *dso  = 	NULL;
    char *tmp;

    /* look for final "." in filename */
    if ((tmp = strrchr(ext, '.')) != NULL)
	ext = tmp+1;

    /* Get the ConverterDSO struct for this extension */
    dso = pfdFindConverterDSO(ext);

    if (dso == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdConvertFrom() - No converter available for %s\n", ext);
	return NULL;
    }
	
    if (dso->convertFrom == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdConvertFrom() - Converter for %s does not provide a pfdConvertFrom_%s\n", ext, ext);
	return NULL;
    }
 
    /* reset builder geometry and graphics state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    return dso->convertFrom(root);
}

/*
 * FUNCTION:
 *	pfdConvertTo() 
 *
 * DESCRIPTION:
 *	Load a database file into an OpenGL Performer in-memory data 
 *	structure to another in-memory representation
 */

extern PFDUDLLEXPORT void*
pfdConvertTo(pfNode *root, const char *ext)
{
    ConverterDSO *dso  = 	NULL;
    char *tmp;

    /* look for final "." in filename */
    if ((tmp = strrchr(ext, '.')) != NULL)
	ext = tmp+1;

    /* Get the ConverterDSO struct for this extension */
    dso = pfdFindConverterDSO(ext);

    if (dso == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdConvertTo() - No converter available for %s\n", ext);
	return NULL;
    }
	
    if (dso->convertTo == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdConvertTo() - Converter for %s does not provide a pfdConvertTo_%s\n", ext, ext);
	return NULL;
    }

    return dso->convertTo(root);
}


/*
 *  User callback function registry.
 */

typedef struct user_func_s {
    void *func;
    char *name;
    char *dso_name;
} user_func_t;

typedef struct ufr_s {
    user_func_t *funcs;
    int num;
    int size;
} ufr_t;

static ufr_t *ufr = NULL;


static void
_pfdInitUserFuncRegistry(void)
{
    ufr = (ufr_t*)pfMalloc(sizeof(ufr_t), pfGetSharedArena());
    ufr->funcs = NULL;
    ufr->num = 0;
    ufr->size = 0;
}


/*
 * FUNCTION:
 *	pfdRegisterUserFunc() 
 *
 * DESCRIPTION:
 *	Register a user callback function.
 */
PFDUDLLEXPORT int
pfdRegisterUserFunc(void *func, const char *name, const char *dso_name)
{
    int i;

    if (ufr == NULL)
	_pfdInitUserFuncRegistry();

    if (name == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfdRegisterUserFunc:  Name must not be NULL.");
	return(-1);
    }

    for (i = 0; i < ufr->num; i++)
	if (strcmp(name, ufr->funcs[i].name) == 0)
	{
	    if (func != NULL)
	    {
		if (ufr->funcs[i].dso_name)
		    pfFree(ufr->funcs[i].dso_name);
		if (dso_name)
		{
		    ufr->funcs[i].dso_name =
			(char *)pfMalloc(strlen(dso_name)+1, pfGetArena(ufr));
		    strcpy(ufr->funcs[i].dso_name, dso_name);
		}
		else
		    ufr->funcs[i].dso_name = NULL;
		ufr->funcs[i].func = func;
	    }
	    else /* func == NULL */
	    {
		/*
		 *  NULL func means unregister the function name.
		 */
		pfFree(ufr->funcs[i].name);
		if (ufr->funcs[i].dso_name)
		    pfFree(ufr->funcs[i].dso_name);
		for (i++; i < ufr->num; i++)
		{
		    ufr->funcs[i-1].func = ufr->funcs[i].func;
		    ufr->funcs[i-1].name = ufr->funcs[i].name;
		    ufr->funcs[i-1].dso_name = ufr->funcs[i].dso_name;
		}
		ufr->num--;
	    }
	}

    if (ufr->num == ufr->size)
    {
	if (ufr->size == 0)
	{
	    ufr->size = 8;
	    ufr->funcs = (user_func_t*)pfMalloc(ufr->size * sizeof(user_func_t),
					        pfGetArena(ufr));
	}
	else
	{
	    ufr->size <<= 1;
	    ufr->funcs = (user_func_t*)pfRealloc(ufr->funcs,
						 ufr->size * sizeof(user_func_t));
	}
    }

    ufr->funcs[ufr->num].func = func;
    ufr->funcs[i].name = (char *)pfMalloc(strlen(name)+1, pfGetArena(ufr));
    strcpy(ufr->funcs[i].name, name);
    if (dso_name)
    {
	ufr->funcs[i].dso_name = (char *)pfMalloc(strlen(dso_name)+1,
						  pfGetArena(ufr));
	strcpy(ufr->funcs[i].dso_name, dso_name);
    }
    else
	ufr->funcs[ufr->num].dso_name = NULL;
    ufr->num++;

    return(0);
}


/*
 * FUNCTION:
 *	pfdGetRegisteredUserFunc() 
 *
 * DESCRIPTION:
 *	Return the name and dso_name of a registered user func.
 */
PFDUDLLEXPORT int
pfdGetRegisteredUserFunc(void *func, char **name, char **dso_name)
{
    int i;

    if (ufr == NULL)
	_pfdInitUserFuncRegistry();

    for (i = 0; i < ufr->num; i++)
	if (func == ufr->funcs[i].func)
	{
	    *name = ufr->funcs[i].name;
	    *dso_name = ufr->funcs[i].dso_name;
	    return TRUE;
	}

    *name = NULL;
    *dso_name = NULL;
    return FALSE;
}


/*
 * FUNCTION:
 *	pfdIsRegisteredUserFunc() 
 *
 * DESCRIPTION:
 *	Is func a registerd user func.
 */
PFDUDLLEXPORT int
pfdIsRegisteredUserFunc(void *func)
{
    int i;

    if (ufr == NULL)
	_pfdInitUserFuncRegistry();

    for (i = 0; i < ufr->num; i++)
	if (func == ufr->funcs[i].func)
	    return TRUE;

    return FALSE;
}


/*
 * FUNCTION:
 *	pfdFindRegisteredUserFunc() 
 *
 * DESCRIPTION:
 *	Find and return the user func with name.
 */
PFDUDLLEXPORT void *
pfdFindRegisteredUserFunc(char *name)
{
    int i;

    if (ufr == NULL)
	_pfdInitUserFuncRegistry();

    for (i = 0; i < ufr->num; i++)
	if (strcmp(name, ufr->funcs[i].name) == 0)
	    return ufr->funcs[i].func;

    return NULL;
}
