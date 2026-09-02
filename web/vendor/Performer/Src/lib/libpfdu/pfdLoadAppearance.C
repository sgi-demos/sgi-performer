/*
 * pfdLoadAppearance.C
 *
 * $Revision: 1.4 $
 * $Date: 2002/11/23 00:23:59 $
 *
 * Functions for loading and storing islAppearance descriptions in 
 * files.
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

#include <Performer/pfdu.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Performer/pr/pfStruct.h>

#ifndef _PF_NO_SHADER_

/*
  File format description:

  This is a very simple text based file format that can be read with an XML parser, 
  however we do it ourselves here so that libpfdu does not depend on any current
  XML parsers which are in a state of flux. 
  
  File format: 

  <pf_appearance_description>
    <version> integer </version>
    <ambient_light_shaders> [shader list] </ambient_light_shaders>
    <local_light_shaders> [shader list] </local_light_shaders>
    <distant_light_shaders> [shader list] </distant_light_shaders>
    <surface_shaders> [shader list ]</surface_shaders>
  <pf_appearance_description>



  --------------- shader description -------------------

  A shader list is a collection of these:

  <shader_description>
      <shader_source> [shader body goes here ]  </shader_source>
      <shader_include_path> [include path] </shader_include_path>
      <parameter_list> 
         <parameter>
           <number> </number>
	   <matrix> [16 floats ] </matrix>
	   <float>  [ 1 float ]  </float>
	   <color>  [ 4 floats ] </color>
	   <string> [ string ] </string>
	 </parameter>
      </parameter_list>
  </shader_description>
*/


#define PFD_LOAD_APPEARANCE_VERSION              1

/* Tokens used in reading and writing the file */
#define BEGIN_APPEARANCE_DESCRIPTION "<pf_appearance_description>"
#define END_APPEARANCE_DESCRIPTION   "</pf_appearance_description>"
#define BEGIN_VERSION                "<version>"
#define END_VERSION                  "</version>"

#define BEGIN_AMBIENT                "<ambient_light_shaders>"
#define END_AMBIENT                  "</ambient_light_shaders>"
#define BEGIN_LOCAL                  "<local_light_shaders>"
#define END_LOCAL                    "</local_light_shaders>"
#define BEGIN_DISTANT                "<distant_light_shaders>"
#define END_DISTANT                  "</distant_light_shaders>"
#define BEGIN_SURFACE                "<surface_shaders>"
#define END_SURFACE                  "</surface_shaders>"

#define BEGIN_SHADER_DESCRIPTION "<shader_description>"
#define END_SHADER_DESCRIPTION   "</shader_description>"

#define BEGIN_SHADER "<shader_source>"
#define END_SHADER "</shader_source>"

#define BEGIN_SHADER_INCLUDE "<shader_include_path>"
#define END_SHADER_INCLUDE "</shader_include_path>"

#define BEGIN_SHADER_PARAMETERS "<parameter_list>"
#define END_SHADER_PARAMETERS "</parameter_list>"

#define BEGIN_PARAMETER "<parameter>"
#define END_PARAMETER   "</parameter>"

#define BEGIN_PARAMETER_NUMBER "<number>"
#define END_PARAMETER_NUMBER "</number>"

#define BEGIN_MATRIX "<matrix>"
#define END_MATRIX "</matrix>"

#define BEGIN_FLOAT "<float>"
#define END_FLOAT "</float>"

#define BEGIN_COLOR "<color>"
#define END_COLOR "</color>"

#define BEGIN_STRING "<string>"
#define END_STRING "</string>"

/* 
   Global hash table used to associate file names with appearance pointers.
   This is necessary for saving to pfb, since there is no way to infer a
   name from an appearance, so we'll store the names from loaded files. 
   For newly created appearances, we'll have to make a dummy name.
*/

struct _AppHashElt {
  islAppearance *app_;
  char *name_;
  _AppHashElt *next_;
};

class pfdAppearanceNameHash {
public:
  static pfdAppearanceNameHash *getInstance() {
    if(!instance_)
      instance_ = new pfdAppearanceNameHash();
    return instance_;
  }

  void insertName(islAppearance *appearance, const char *name) {
    /* Do we already have a name? */
    _AppHashElt *found = findElement(appearance);
    
    if(found) {
      /* yes, change the name */
      free(found->name_);
      found->name_ = strdup(name);
    }
    else {
      /* no, make a new entry */
      int bin = hashPointer(appearance);
      _AppHashElt *e = (_AppHashElt *)pfMemory::malloc(sizeof(_AppHashElt),
						       pfGetSharedArena());
      e->app_ = appearance;
      e->name_ = strdup(name);
      e->next_ = bins[bin];
      bins[bin] = e;
      
    }
  }
  
  char *findName(islAppearance *appearance) {
    return findElement(appearance)->name_;
  }
  
 
private:
  /* finds and returns an element with a particular appearance */
  _AppHashElt *findElement(islAppearance *app) {
    int bin = hashPointer(app);
    _AppHashElt *head = bins[bin];
    while(head) {
      _AppHashElt *next = head->next_;
      if(head->app_ == app) return head;
      head = next;
    }
    return NULL;
  }
  
  /* hashes a pointer to a number */
  int hashPointer(islAppearance *app) {
    int val = (int)app;
    /* last 3 bits generally insignificant */
    val >>= 3;
    return val % nBins_;
  }
  
  /* creates empty hash table */
  pfdAppearanceNameHash() {
    nBins_ = 127;
    bins = (_AppHashElt **)pfMemory::malloc(sizeof(_AppHashElt *) * nBins_,
					    pfGetSharedArena());
    for(int i=0; i < nBins_; i++)
      bins[i] = NULL;
  }
  
  /* destroys the hash table */
  ~pfdAppearanceNameHash() {
    for(int i=0; i < nBins_; i++) {
      _AppHashElt *head = bins[nBins_];
      while(head) {
	_AppHashElt *next = head->next_;
	pfMemory::free(head);
	head = next;
      }
    }
    pfMemory::free(bins);
  }

  static pfdAppearanceNameHash* instance_;
  int nBins_;
  _AppHashElt** bins;
};

pfdAppearanceNameHash * pfdAppearanceNameHash::instance_ = NULL;

/*
  This function returns or makes up a name for an appearance */
PFDUDLLEXPORT char *pfdGetAppearanceFilename(islAppearance *app) {
  char *filename = pfdAppearanceNameHash::getInstance()->findName(app);
  static int AppearanceNumber = 0;
  /* did we find a name? */
  if(filename == 0) {
    char buf[100];
    /* no, make one up */
    sprintf(buf, "Unnamed%d.pfAppearance", AppearanceNumber);
    AppearanceNumber++;
    filename = buf;
    /* now, tell the hash table about it */
    pfdAppearanceNameHash::getInstance()->insertName(app, filename);
    /* and return a pointer that will stick around */
    filename = pfdAppearanceNameHash::getInstance()->findName(app);
  }
  
  return filename;
}
   
			      
/**************************** Loading from file ********************************/
#ifdef WIN32
#define stat _stat
#endif

/* Globals used in parsing */
char *buffer = NULL;
int bufferSize = 0;
int bufferIndex = 0;

islAppearance *appearance;
islShader *shader;
islAppearance::ListType currentList;


/* This function removes white space from the buffer */
void eatWhiteSpace() {
  while(((buffer[bufferIndex] == ' ') ||
	 (buffer[bufferIndex] == '\t') ||
	 (buffer[bufferIndex] == '\n') ||
	 (buffer[bufferIndex] == '\r')) 
	&& (bufferIndex < bufferSize))
    bufferIndex++;
}

/* This function eats ASCII numbers */
void eatNumber() {
  while((bufferIndex < bufferSize) &&
	((buffer[bufferIndex] >= '0' && buffer[bufferIndex] <= '9') ||
	 (buffer[bufferIndex] == '.') ||
	 (buffer[bufferIndex] == '-')))
    bufferIndex++;
}

/* This function advances the buffer along the specified string. If
   there is no match, buffer is not advanced */
int 
advanceString(const char *str) {
  int i=0;
  int currentIndex = bufferIndex;
  while(str[i]) {
    if(str[i] == buffer[currentIndex]) {
      if(currentIndex >= bufferSize) {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadAppearance: Ran off end of file expecing \"%s\"", str);
	return 0; //ran off the end
      }
      i++;
      currentIndex++;
    }
    else {
      return 0; //strings dont match
    }
  }
  /* Successfully advanced past 'str' */
  bufferIndex = currentIndex;
  return 1;
}

/* this function returns the index of the next < in the buffer */
int 
getNextTokenIndex() {
  int tokenIndex = bufferIndex;
  while(buffer[tokenIndex] != '<') {
    tokenIndex++;
    if(tokenIndex >= bufferSize) return -1; //ran off end
  }
  return tokenIndex;
}


/*
  This function reads as many parameters as necessary and sets them
  on the current shader 
*/
int 
parseParameterList() {
  eatWhiteSpace();
  while(advanceString(BEGIN_PARAMETER)) { /* loop through all parameters */
    eatWhiteSpace();
    /* expect BEGIN_PARAMETER_NUMBER */
    if(!advanceString(BEGIN_PARAMETER_NUMBER)) {
      pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	       "Expecting %s, got %.15s\n", BEGIN_PARAMETER_NUMBER,
	       &buffer[bufferIndex]);
      return 0;
    }
    /* Now, we need to read an int */
    int number;
    sscanf(&buffer[bufferIndex], "%d", &number);
    eatWhiteSpace();
    eatNumber();
    eatWhiteSpace();
    if(!advanceString(END_PARAMETER_NUMBER)) {
      pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	       "pfdLoadAppearance: Expecting %s, got %.15s\n", END_PARAMETER_NUMBER,
	       &buffer[bufferIndex]);
      return 0;
    }
    eatWhiteSpace();

    /* Now, we expect a parameter value */

    /* Parse a matrix */
    if(advanceString(BEGIN_MATRIX)) {
      int tokenIndex = getNextTokenIndex();
      float m[16];
      eatWhiteSpace();
      sscanf(&buffer[bufferIndex], "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
	     &m[0],  &m[1],  &m[2],  &m[3],
	     &m[4],  &m[5],  &m[6],  &m[7],
	     &m[8],  &m[9],  &m[10], &m[11],
	     &m[12], &m[13], &m[14], &m[15]);
      bufferIndex = tokenIndex;
      eatWhiteSpace();
      if(!advanceString(END_MATRIX)) {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadAppearance: Expecting %s, got %.15s\n", END_MATRIX,
		 &buffer[bufferIndex]);
	return 0;
      }
      shader->setParameterMatrix(number, m);
    }
    /* parse a float */
    else if(advanceString(BEGIN_FLOAT)) {
      int tokenIndex = getNextTokenIndex();
      float f;
      eatWhiteSpace();
      sscanf(&buffer[bufferIndex], "%f", &f);
      bufferIndex = tokenIndex;
      eatWhiteSpace();
      if(!advanceString(END_FLOAT)) {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadAppearance: Expecting %s, got %.15s\n", END_FLOAT,
		 &buffer[bufferIndex]);
	return 0;
      }
      shader->setParameterFloat(number, f);
    }
    /* parse a color */
    else if(advanceString(BEGIN_COLOR)) {
      int tokenIndex = getNextTokenIndex();
      float r, g, b, a;
      eatWhiteSpace();
      sscanf(&buffer[bufferIndex], "%f %f %f %f", &r, &g, &b, &a);
      bufferIndex = tokenIndex;
      eatWhiteSpace();
      if(!advanceString(END_COLOR)) {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadAppearance: Expecting %s, got %.15s\n", END_COLOR,
		 &buffer[bufferIndex]);
	return 0;
      }
      shader->setParameterColor(number, r, g, b, a);
    }
    /* parse a string */
    else if(advanceString(BEGIN_STRING)) {
      int tokenIndex = getNextTokenIndex();
      char *s;
      int size = tokenIndex - bufferIndex;
      s = (char *)malloc(sizeof(char) * (size +1));
      strncpy(s, &buffer[bufferIndex], size);
      s[size] = 0;
      bufferIndex = tokenIndex;
      eatWhiteSpace();
      if(!advanceString(END_STRING)) {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadAppearance: Expecting %s, got %.15s\n", END_STRING,
		 &buffer[bufferIndex]);
	return 0;
      }
      shader->setParameterString(number, s);
    }
    /* parse whatever */
    else {
      pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	       "pfdLoadAppearance: Expecting a parameter type, got %.15s\n", 
	       &buffer[bufferIndex]);
      return 0;
    }
    eatWhiteSpace();
    if(!advanceString(END_PARAMETER)) {
      pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	       "pfdLoadAppearance: Expecting %s, got %.15s\n", END_PARAMETER,
	       &buffer[bufferIndex]);
      return 0;
    }
    eatWhiteSpace();
  }
  return 1;
}


int 
parseShaderList() {
  /* expect BEGIN_SHADER_DESCRIPTION */
  eatWhiteSpace();
  int keepGoing = 1;
  while(keepGoing && advanceString(BEGIN_SHADER_DESCRIPTION)) {
    /* expect BEGIN_SHADER */
    eatWhiteSpace();
    if(!advanceString(BEGIN_SHADER))
      return 0;
    /* Now, everything up to '<' is the shader body */
    int tokenIndex = getNextTokenIndex();
    /* Now, tokenIndex points to beginning of next token */
    int shaderSize = tokenIndex - bufferIndex;
    char *shaderDesc = (char *)malloc(sizeof(char) * (shaderSize+1));
    strncpy(shaderDesc, &buffer[bufferIndex], shaderSize);
    shaderDesc[shaderSize] = 0;
    bufferIndex = tokenIndex;
    /* Given the shader description, lets make a shader */
    shader = new islShader();
    
    appearance->pushShader(currentList, shader);
    /* now, advance index past end of shader */
    
    if(!advanceString(END_SHADER)) return 0;
    
    /* Now, expect shader includes */
    eatWhiteSpace();
    if(!advanceString(BEGIN_SHADER_INCLUDE)) return 0;
    /* Find '<' again */
    tokenIndex = getNextTokenIndex();
    /* Now, tokenIndex points to beginning of next token */
    int includeSize = tokenIndex - bufferIndex;
    char *includes = (char *)malloc(sizeof(char) * (includeSize +1));
    strncpy(includes, &buffer[bufferIndex], includeSize);
    includes[includeSize] = 0;
    bufferIndex = tokenIndex;
    shader->setIncludePath(includes);
    /* now, expect END_SHADER_INCLUDE */
    if(!advanceString(END_SHADER_INCLUDE)) return 0;
    eatWhiteSpace();
    
    /* now that includes are set up, set the body on the shader */
    if(shader->setShader(shaderDesc) == -1) {
      pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	       "pfdLoadAppearance: islShader::setShader failed \n");
      islError error;
      while(shader->getError(error)) {
	pfNotify(PFNFY_WARN, PFNFY_MORE, "  islError: (%s:%d) : %s",
 		 error.getFileName(), error.getLine(), error.getMessage());
      }
    }
    

    /* now, we may have a parameter list */
    if(advanceString(BEGIN_SHADER_PARAMETERS)) {
      eatWhiteSpace();
      if(!parseParameterList()) {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadAppearance: error parsing shader parameter list\n");
	return 0;
      }
      eatWhiteSpace();
      if(!advanceString(END_SHADER_PARAMETERS)) {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "pfdLoadAppearance: Expecting %s, got %.15s\n", END_SHADER_PARAMETERS,
		 &buffer[bufferIndex]);
	return 0;
      }
      eatWhiteSpace();
    }
    
    eatWhiteSpace();
    /* We may have the end of the shader */
    if(advanceString(END_SHADER_DESCRIPTION)) {
      eatWhiteSpace();
      continue;
    }
    /* otherwise, something bad happened */
    else {
      pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	       "pfdLoadAppearance: Parse error. Expecting parameter list or end of shader");
      pfNotify(PFNFY_WARN, PFNFY_MORE,
	       "but got %s\n", &buffer[bufferIndex]);
      return 0;
    }
  }

  return 1;
}


int 
parseAppearance() {
  /* first, eat any white space */
  eatWhiteSpace();
  /* now expect BEGIN_APPEARANCE_DESCRIPTION */
  if(!advanceString(BEGIN_APPEARANCE_DESCRIPTION)) 
    return 0;
  eatWhiteSpace();
  /* now expect BEGIN_VERSION */
  if(!advanceString(BEGIN_VERSION)) return 0;
  eatWhiteSpace();
  /* now expect a version number */
  int version;
  sscanf(&buffer[bufferIndex], "%d", &version);
  eatNumber();
  eatWhiteSpace();
  /* now expect END_VERSION */
  if(!advanceString(END_VERSION)) return 0;
  eatWhiteSpace();
  /* create a new appearance */
  appearance = new islAppearance();
  
  int keepGoing = 1;
  /* next buffer char should be < */
  while(keepGoing && (buffer[bufferIndex] == '<')) {

 
    /* lets see what token we have next */
    char token[100];
    sscanf(&buffer[bufferIndex], "%s>", token);
    /* Now, based on this token, make a decision on which shader list
       to parse or whether we are done */
    
    if(strncmp(token, BEGIN_SURFACE, strlen(BEGIN_SURFACE)) == 0) {
      /* have surface shader list */
      currentList = islAppearance::SURFACE_LIST;
      if(!advanceString(BEGIN_SURFACE)) return 0;
      eatWhiteSpace();
      if(!parseShaderList()) return 0;
      eatWhiteSpace();
      if(!advanceString(END_SURFACE)) return 0;
    }
    else if(strncmp(token, BEGIN_AMBIENT, strlen(BEGIN_AMBIENT)) == 0) {
      /* have ambient shader list */
      currentList = islAppearance::AMBIENTLIGHT_LIST;
      if(!advanceString(BEGIN_AMBIENT)) return 0;
      eatWhiteSpace();
      if(!parseShaderList()) return 0;
      eatWhiteSpace();
      if(!advanceString(END_AMBIENT)) return 0;
    }
    else if(strncmp(token, BEGIN_LOCAL, strlen(BEGIN_LOCAL)) == 0) {
      /* have local light list */
      currentList = islAppearance::LOCALLIGHT_LIST;
      if(!advanceString(BEGIN_LOCAL)) return 0;      
      eatWhiteSpace();
      if(!parseShaderList()) return 0;
      eatWhiteSpace();
      if(!advanceString(END_LOCAL)) return 0;
    }
    else if(strncmp(token, BEGIN_DISTANT, strlen(BEGIN_DISTANT)) == 0) {
      /* have distant light list */
      if(!advanceString(BEGIN_DISTANT)) return 0;
      currentList = islAppearance::DISTANTLIGHT_LIST;
      eatWhiteSpace();
      if(!parseShaderList()) return 0;
      eatWhiteSpace(); 
      if(!advanceString(END_DISTANT)) return 0;
    }
    
    eatWhiteSpace();
    if(strcmp(token, END_APPEARANCE_DESCRIPTION) == 0) {
      /* finished reading appearance */
      keepGoing = 0;
    }
    eatWhiteSpace();
    /* done with appearance */
  }
  

  return 1;
}

PFDUDLLEXPORT islAppearance *
pfdLoadAppearance(const char *filename) {
  
  /* Find the file */
  char path[PF_MAXSTRING];
  if (!pfFindFile(filename, path, R_OK)) {
    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	     "pfdLoadAppearance:  Could not find file \"%s\".",
	     filename);
    return NULL;
  }
  
  pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
	   "pfdLoadAppearance: Loading \"%s\"",
	   path);

  /* First, lets figure out the size of the requested file 
   and allocate a buffer*/
  struct stat fileStat;
  stat(path, &fileStat);
  bufferSize = fileStat.st_size;
  buffer = (char *)malloc(bufferSize);
  
  /*
    Now, read the file into the buffer
  */
  FILE *file = fopen(path, "rb");
  if(!file) {
    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	     "pfdLoadAppearance: Could not open \"%s\" for reading.",
	     filename);
    free(buffer);
    return NULL;
  }
  
  size_t retVal = fread((void *)buffer, bufferSize, 1, file);
  if(retVal != 1) {
    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	     "pfdLoadAppearance: Error reading \"%s\"",
	     filename);
    free(buffer);
    return NULL;
  }
  
  /* Now we have the file, make a token structure from it */
  fclose(file);
  bufferIndex = 0;
  int ret = parseAppearance();
  if(!ret) {
    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	     "pfdLoadAppearance: Error parsing \"%s\"",
	     filename);
    delete appearance;
    free(buffer);
    return NULL;
  }
  free(buffer);

  /* make a record of this appearance's name */
  pfdAppearanceNameHash::getInstance()->insertName(appearance, filename);
  
  return appearance;
  
}

/**************************** SAVING TO FILE **********************************/

/* This function writes a string to the specified file */
int 
writeString(char *str, FILE *file) {
  int len = strlen(str);
  size_t retVal = fwrite(str, sizeof(char), len, file);
  if(retVal != len) {
    //fwrite failed
    return 0;
  }
  return 1;
}

/* This function writes an integer as ASCII to the specified file */
char strbuf[64];
int 
writeInt(int num, FILE *file) {
  sprintf(strbuf, "%d", num);
  return writeString(strbuf, file);
}

/* This function writes a float as ASCII to the specified file */
int 
writeFloat(float num, FILE *file) {
  sprintf(strbuf, "%f", num);
  return writeString(strbuf, file);
}

int 
writeParameter(FILE *file, islShader *shader, int param) {
  writeString(BEGIN_PARAMETER, file);
  writeString("\n", file);
  writeString(BEGIN_PARAMETER_NUMBER, file);
  writeInt(param, file);
  writeString(END_PARAMETER_NUMBER, file);
  writeString("\n", file);
  islShader::ParameterType type = shader->getParameterType(param);
  switch(type) {
  case islShader::PARAMETER_COLOR:
    {
      float r, g, b, a;
      shader->getParameterColor(param, r, g, b, a);
      writeString(BEGIN_COLOR, file);
      writeFloat(r, file);
      writeString(" ", file);
      writeFloat(g, file);
      writeString(" ", file);
      writeFloat(b, file);
      writeString(" ", file);
      writeFloat(a, file);
      writeString(END_COLOR, file);
      writeString("\n", file);
    }
    break;
  case islShader::PARAMETER_MATRIX:
    {
      float m[16];
      shader->getParameterMatrix(param, m);
      writeString(BEGIN_MATRIX, file);
      for(int i=0; i < 16; i++) {
	writeFloat(m[i], file);
	writeString(" ", file);
      }
      writeString(END_MATRIX, file);
      writeString("\n", file);
    }
    break;
  case islShader::PARAMETER_STRING:
    {
      char *str;
      shader->getParameterString(param, str);
      writeString(BEGIN_STRING, file);
      writeString(str, file);
      writeString(END_STRING, file);
      writeString("\n", file);
    }
    break;
  case islShader::PARAMETER_FLOAT:
    {
      float f;
      shader->getParameterFloat(param, f);
      writeString(BEGIN_FLOAT, file);
      writeFloat(f, file);
      writeString(END_FLOAT, file);
      writeString("\n", file);
    }
    break;
  }
  writeString(END_PARAMETER, file);
  writeString("\n", file);
  return 1;
}

int 
writeShaderDescription(FILE *file, islShader *shader) {
  writeString("\n", file);
  writeString(BEGIN_SHADER_DESCRIPTION, file);
  writeString("\n", file);
  /* Write the shader body */
  writeString(BEGIN_SHADER, file);
  writeString(shader->getShader(), file);
  writeString(END_SHADER, file);
  writeString("\n", file);
  
  /* Write the shader include path */
  writeString(BEGIN_SHADER_INCLUDE, file);
  writeString(shader->getIncludePath(), file);
  writeString(END_SHADER_INCLUDE, file);
  writeString("\n", file);

  /* Write the shader parameter list */
  writeString(BEGIN_SHADER_PARAMETERS, file);
  writeString("\n", file);
  int numParams = shader->getNumParameters();
  for(int i=0; i < numParams; i++) {
    writeParameter(file, shader, i);
  }
  writeString(END_SHADER_PARAMETERS, file);
  writeString("\n", file);

  writeString(END_SHADER_DESCRIPTION, file);
  writeString("\n", file);
  return 1;
}

int 
writeAppearanceDescription(FILE *file, islAppearance *app) {
  /* Write appearance description */

  /* Version number */
  writeString(BEGIN_APPEARANCE_DESCRIPTION, file);
  writeString("\n", file);
  writeString(BEGIN_VERSION, file);
  writeInt   (PFD_LOAD_APPEARANCE_VERSION, file);
  writeString(END_VERSION, file);

  /* Shader lists */
  writeString("\n", file);
  
  int numShaders;
  /* Ambient light shaders */
  numShaders = app->getNumShaders(islAppearance::AMBIENTLIGHT_LIST);
  {
    writeString(BEGIN_AMBIENT, file);
    
    for(int i=0; i < numShaders; i++) {
      islShader *shader = app->getShader(islAppearance::AMBIENTLIGHT_LIST, i);
      writeShaderDescription(file, shader);
    }
    writeString(END_AMBIENT, file);
    writeString("\n", file);
  }
  
  /* Distant light shaders */
  numShaders = app->getNumShaders(islAppearance::DISTANTLIGHT_LIST);
  {
    writeString(BEGIN_DISTANT, file);
    
    for(int i=0; i < numShaders; i++) {
      islShader *shader = app->getShader(islAppearance::DISTANTLIGHT_LIST, i);
      writeShaderDescription(file, shader);
    }
    writeString(END_DISTANT, file);
    writeString("\n", file);
  }
  
  /* local light shaders */
  numShaders = app->getNumShaders(islAppearance::LOCALLIGHT_LIST);
  {
    writeString(BEGIN_LOCAL, file);
    
    for(int i=0; i < numShaders; i++) {
      islShader *shader = app->getShader(islAppearance::LOCALLIGHT_LIST, i);
      writeShaderDescription(file, shader);
    }
    writeString(END_LOCAL, file);
    writeString("\n", file);
  }
  
  /* surface shaders */
  numShaders = app->getNumShaders(islAppearance::SURFACE_LIST);
  {
    writeString(BEGIN_SURFACE, file);
    
    for(int i=0; i < numShaders; i++) {
      islShader *shader = app->getShader(islAppearance::SURFACE_LIST, i);
      writeShaderDescription(file, shader);
    }
    writeString(END_SURFACE, file);
    writeString("\n", file);
  }
  
  writeString(END_APPEARANCE_DESCRIPTION, file);
  writeString("\n", file);
  
  fclose(file);
  return 1;
}



PFDUDLLEXPORT int 
pfdStoreAppearance(const char *name, islAppearance *app) {
  
  char *filename = (char *)name;
  if(app == NULL) {
    pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	     "pfdStoreAppearance: Passed null appearance for filename %s",
	     filename);
    return 0;
  }
  
  if(filename == NULL) {
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
	     "pfdStoreAppearance: trying to infer filename");
    filename = pfdGetAppearanceFilename(app);
  }
  else {
    pfdAppearanceNameHash::getInstance()->insertName(app, filename);
  }

  FILE *file = fopen(filename, "wb");
  if(file == NULL) {
    pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	     "pfdStoreAppearance: Could not open %s for writing",
	     filename);
    return 0;
  }
  
  pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
	   "pfdStoreAppearance: Writing \"%s\"", filename);

  int retVal = writeAppearanceDescription(file, app);
  
  if(retVal == 0) {
    pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	     "pfdStoreAppearance: Writing of %s failed\n",
	     filename);
    return 0;
  }

  return 1;
}

#else
/* shader is not present */
PFDUDLLEXPORT islAppearance *pfdLoadAppearance(const char *filename) {
  pfNotify(PFNFY_WARN, PFNFY_RESOURCE, "pfdLoadAppearance is not supported on this platform");
  pfNotify(PFNFY_WARN, PFNFY_MORE, "Valid platforms are Linux and Irix N32");
  return NULL;
}

PFDUDLLEXPORT int pfdStoreAppearance(const char *filename, islAppearance *app) {
  pfNotify(PFNFY_WARN, PFNFY_RESOURCE, "pfdStoreAppearance is not supported on this platform");
  pfNotify(PFNFY_WARN, PFNFY_MORE, "Valid platforms are Linux and Irix N32");
  return 0;
}
/* get the filename that belongs to an isl appearance */
PFDUDLLEXPORT char *pfdGetAppearanceFilename(islAppearance *app) {
  pfNotify(PFNFY_WARN, PFNFY_RESOURCE, "pfdGetAppearanceFilename is not supported on this platform");
  pfNotify(PFNFY_WARN, PFNFY_MORE, "Valid platforms are Linux and Irix N32");
  return NULL;
}
#endif

