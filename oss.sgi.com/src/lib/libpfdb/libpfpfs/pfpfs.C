/*
 * Copyright 1996, Silicon Graphics, Inc.
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
 * pfpfs.c loader
 *
 * Loads several models and shaders, applying the shaders to the models.  The
 * commands are as follows:
 *
 *   shader <shaderfile>
 *      - loads <shaderfile> and applies it to all subsequently loaded models
 *        until to resetshader command is issued.
 *
 *   model <modelfile>
 *      - loads <modelfile> and applies the currently loaded shader (if any).
 *
 *   resetshader
 *      - turns of the currently loaded shader.
 *
 *   rotate <angle> <axis_x> <axis_y> <axis_z>
 *      - multiplies the current transform by a rotation of <angle> degrees
 *        about the axis <axis_x, axis_y, axis_z>
 *
 *   translate <xlate_x> <xlate_y> <xlate_z>
 *      - multiplies the current transform by the given translation.
 *
 *   scale <scale_x> <scale_y> <scale_z>
 *      - multiplies the current transform by the given scale.
 *
 *   resetxform
 *      - sets the current transform to the identity.
 *
 * All commands can be issued in any order and as many times as necessary.
 *
 *
 * author - Steve Kilthau
 */

#include <stdio.h>
#include <math.h>
#include <malloc.h>

#include <Performer/pr.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>
#include <Performer/pf/pfSCS.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfShader.h>
#include <Performer/pf/pfShaderManager.h>


//
// Exported symbols.
//
extern "C" {
  pfNode* pfdLoadFile_pfs(const char *);
}

class pfsFileList {
public:
  pfsFileList() : _head(0) {}

  ~pfsFileList() {
    elem *last, *e = _head;

    while(e) {
      last = e; 
      e = e->getNextElement();
      delete last;
    } 
  }

  void addShader(pfShader *shader,const char *filename) {
    elem *e;

    if(!_head) {
      _head = new elem();
      e = _head;
    } else {
      elem *last;

      e = _head; 
      while(e) {
        last = e;
        e = e->getNextElement();
      }

      e = new elem();
      last->setNextElement(e);
    }

    e->addElement(shader,filename);
  }

  pfShader *findShader(const char *filename) {
    elem *e = _head;

    while(e) {
      if(!strcmp(e->getFilename(),filename)) 
        return e->getShader();
      // else:
      e = e->getNextElement();
    }

    return 0;
  }

  class elem {
  public:
    elem() : shader(0), filename(0), next(0) {}
    ~elem () { if(filename) free(filename); }

    void addElement(pfShader *shader,const char *filename) {
      this->shader = shader;
      this->filename = strdup(filename);
      next = NULL;
    }
   
    pfShader *getShader() { return shader; }
    char *getFilename() { return filename; }

    elem *getNextElement() { return next; }
    void setNextElement(elem *e) { next = e; }

  private:
    pfShader *shader;
    char *filename;
    elem *next;
  };

  elem *_head;
};

//
// Read and parse the .projtex file.  Set up the texture
// matrix, the texture generator, and the texutre environment.
//
static pfNode*
parsePFS(const char* filename)
{
  // Parse variables
  int      commandRead;
  int      pos = 0;
  int      len = 0;
  char     str[1024];
  float    f0, f1, f2, f3;
  char*    buffer;
  FILE*    ifp;
  pfsFileList fileList;

  // Performer variables
  pfShaderManager* shaderManager;
  pfShader*        shader;
  pfGroup*         group;
  pfMatrix*        xform;

  // Allocate the shader variables and the group node.
  shaderManager = pfGetShaderManager();
  shader        = 0;
  group         = new pfGroup;
  xform         = new pfMatrix;
  xform->makeIdent();

  // Read in the file
  ifp = fopen(filename, "r");
  if (ifp)
  {
    int length;

    fseek(ifp, 0, SEEK_END);
    length = ftell(ifp);
    fseek(ifp, 0, SEEK_SET);
    buffer = new char[length+1];
    fread(buffer, 1, length, ifp);
    buffer[length] = 0;
    fclose(ifp);
  }
  else
  {
    pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdLoadFile_pfs: unable to read file \"%s\"\n", filename);
    return group;
  }

  do
  {
    commandRead = 0;

    // Read a model
    if (sscanf(&buffer[pos], " model %s %n", str, &len) == 1)
    {
      pfNode* model;
      pfSCS*  scs;

      // Load DSO for the input file
      pfdInitConverter(str);

      // Read in the file
      model = pfdLoadFile(str);
      if (model == NULL)
      {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdLoadFile_pfs: unable to load file \"%s\".\n", str);
	delete [] buffer;
	return group;
      }

      // Add the scene to the group
      if (shader)
	shaderManager->applyShader(model, shader);
      scs = new pfSCS(*xform);
      scs->addChild(model);
      group->addChild(scs);

      commandRead = 1;
      pos += len;
      len = 0;
    }

    // Read a shader
    if (sscanf(&buffer[pos], " shader %s %n", str, &len) == 1)
    {
      // Read in the shader
      shader = fileList.findShader(str);
      if(!shader) {
        shader = pfdLoadShader(str);
        if(shader)
          fileList.addShader(shader,str);
      }
      if (!shader)
      {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdLoadFile_pfs: unable to load file \"%s\".\n", str);
	delete [] buffer;
	return group;
      }

      commandRead = 1;
      pos += len;
      len = 0;
    }

    // Read a resetshader command
    if (sscanf(&buffer[pos], " resetshader %n", &len) == 0)
    {
      if (len != 0)
      {
	shader = 0;

	commandRead = 1;
	pos += len;
	len = 0;
      }
    }

    // Read a rotate command
    if (sscanf(&buffer[pos], " rotate %f %f %f %f %n", &f0, &f1, &f2, &f3, &len) == 4)
    {
      xform->postRot(*xform, f0, f1, f2, f3);

      commandRead = 1;
      pos += len;
      len = 0;
    }

    // Read a translate command
    if (sscanf(&buffer[pos], " translate %f %f %f %n", &f0, &f1, &f2, &len) == 3)
    {
      xform->postTrans(*xform, f0, f1, f2);

      commandRead = 1;
      pos += len;
      len = 0;
    }

    // Read a scale command
    if (sscanf(&buffer[pos], " scale %f %f %f %n", &f0, &f1, &f2, &len) == 3)
    {
      xform->postScale(*xform, f0, f1, f2);

      commandRead = 1;
      pos += len;
      len = 0;
    }

    // Read a resetxform command
    if (sscanf(&buffer[pos], " resetxform %n", &len) == 0)
    {
      if (len != 0)
      {
	xform->makeIdent();

	commandRead = 1;
	pos += len;
	len = 0;
      }
    }

  }while(commandRead);

  // Check that the entire file was parsed
  if (buffer[pos] != 0)
  {
    pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdLoadFile_pfs: failed to parse entire file.\n");
    delete [] buffer;
    return group;
  }

  // Delete the internal buffer
  delete [] buffer;

  // Resolve the shaders on the tree
  shaderManager->resolveShaders(group);

  return group;
}


//
// Parse and load the .pfs file.
//
pfNode*
pfdLoadFile_pfs(const char* filename)
{
  char          path[PF_MAXSTRING];
  pfNode*       node = 0;

  // Print notify about loader
  pfNotify( PFNFY_INFO, PFNFY_PRINT, 
	    "pfdLoadFile_pfs: Loading \"%s\".\n", filename );

  // Find the input file
  if (!pfFindFile(filename, path, R_OK))
  {
    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	     "pfdLoadFile_pfs: Could not find file \"%s\"\n", filename);
  }
  else
  {
    // Parse the .pfs input file
    node = parsePFS(path);
  }

  return node;
}



