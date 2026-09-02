/*
 * Copyright 2002, Silicon Graphics, Inc.
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

#ifndef __PFV_LOADER_H__
#define __PFV_LOADER_H__

///////////////////////////////////////////////////////////////////////////////

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#include <Performer/pfv/pfvXml.h>
#include <Performer/pfv/pfvViewer.h>
#include <Performer/pfv/pfvDisplay.h>

///////////////////////////////////////////////////////////////////////////////

class pfGroup;
class pfNode;
class pfDCS;

///////////////////////////////////////////////////////////////////////////////

extern void pfvReplaceString( char**oldstr, char*newstr);

///////////////////////////////////////////////////////////////////////////////

class pfvmLoaderModel : public pfvObject
{
public:	
    pfvmLoaderModel(pfvXmlNode*xml=NULL);	
    ~pfvmLoaderModel();	

    static void init();
    static pfType* getClassType(){ return classType; }
    virtual const char* getTypeName() const {return "pfvmLoaderModel";}

    //int parseXml(pfvXmlNode*xml);
    int setXmlField(pfvXmlNode*xml);

    void preConfig() {pfdInitConverter(fileName);}
    void postConfig(pfGroup*root=NULL);

    char* getFilePath() { return filePath?filePath:(char*)""; }
    void setFilePath(char*path){pfvReplaceString(&filePath,path);}
	
    char* getFileName() { return fileName?fileName:(char*)""; }
    void setFileName(char*fName){pfvReplaceString(&fileName,fName);}

    int getAddToScene(){ return addToScene; }
    void setAddToScene(int s){ addToScene=s;} 
	
    void setTrans(pfVec3& xyz){trans.copy(xyz);}
    void setRot(pfVec3& hpr){rot.copy(hpr);}
    void setScale(pfVec3& scaleXYZ){scale.copy(scaleXYZ);}
    void setScale(float s){scale.set(s,s,s);}

    void getTrans(pfVec3& xyz){xyz.copy(trans);}
    void getRot(pfVec3& hpr){hpr.copy(rot);}
    void getScale(pfVec3& scaleXYZ){scaleXYZ.copy(scale);}

    pfDCS* getDCS() { return dcs; }
    pfNode* getRootNode() { return root; }
	
private:
    static pfType* classType;
    void construct();
	
    char* filePath;
    char* fileName;

    int addToScene;
	
    pfDCS* dcs;
    pfVec3 trans;
    pfVec3 rot;
    pfVec3 scale;

    pfNode* root;
	
};

///////////////////////////////////////////////////////////////////////////////

class pfvmLoader : public pfvModule
{
public:
    pfvmLoader(pfvXmlNode*xlm=NULL);
    ~pfvmLoader();
	
    static void init();
    static pfType* getClassType(){ return classType; }
    virtual const char* getTypeName() const {return "pfvmLoader";}

    int parseXml(pfvXmlNode*xml);
    int setXmlField(pfvXmlNode*xml);
	
    void preConfig();
    void postConfig();

    char* getFilePath() { return filePath?filePath:(char*)""; }
    void setFilePath(char*path){pfvReplaceString(&filePath,path);}
	
    int getNumModels() { return listModels->getNum(); }
    pfvmLoaderModel* getModel(int i){ 
        return (pfvmLoaderModel*)(listModels->get(i));
    }
	
    pfvmLoaderModel* findModel(char*_name){
        return (pfvmLoaderModel*)pfvObject::find(listModels,_name); 
    }

    int addModel(pfvmLoaderModel* model)
    {
	if(model==NULL)return -1;
        listModels->add(model);
        return listModels->getNum();
    }

    pfvmLoaderModel* addModel(char* filename)
    {  
        pfvmLoaderModel* model = new pfvmLoaderModel;
	if(model==NULL) return NULL;
	model->setFileName(filename);
        listModels->add(model);
        return model;
    }

protected:
    static pfType *classType;

private:
    void construct();

    char* filePath;
    pfList* listModels;
};

///////////////////////////////////////////////////////////////////////////////

#endif /* end of __PFV_LOADER_H__ */


