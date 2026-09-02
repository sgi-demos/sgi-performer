//
// Copyright 2001, 2002 Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfvXml.h
//
// $Revision: 1.5 $
// $Date: 2002/11/11 02:01:02 $
//

#ifndef __PFV_XML_H__
#define __PFV_XML_H__

///////////////////////////////////////////////////////////////////////////////

#include <Performer/pf.h>
#include <Performer/pr/pfList.h>

#include <Performer/pfv/pfvDLL.h>

///////////////////////////////////////////////////////////////////////////////

class pfvXmlNode;

///////////////////////////////////////////////////////////////////////////////

class PFV_DLLEXPORT pfvXmlNode
{

public:
	
    static pfvXmlNode* parseFile(char* filename);

    pfvXmlNode();
    ~pfvXmlNode();

    /* name */    
    char* getName() {return name;}
    void setName(char* _name);
    int nameCmp(char* str);
    int nameCaseSensitiveCmp(char* str);

    /* attributes */
    int getNumAttr() {return listAttr?listAttr->getNum():0;}
    int getAttr(int i, char** _name, char** val);
    char* getAttr(char* _name);    
    int setAttr(char* _name, char* val);
    int findAttr(char* _name);
    int removeAttr(char* _name);
    int removeAttr(int i);
    int setAttr(const char **atts);

    /* value */    
    char* getValue() {return value;}
    void setValue(char* val);

    char* getString() {return value;}
    void setString(char* val){setValue(val);}

    
    int getBool(int* v);
    
    int getInt(int* v);
	
    int getIntArray(int num, int* dst);

    int getFloat(float* f);
    
    int getFloatArray(int num, float* dst);
    
    int getInputMngrKey(int* key);

    int getNotifyLevel(int* nl);
    
    /* parent */
    pfvXmlNode* getParent() {return parent;}
    /*void setParent(pfvXmlNode*parent);*/
    
    /* children */
    int getNumChildren() {return listChildren?listChildren->getNum():0;}
    pfvXmlNode* getChild(int i);
    pfvXmlNode* findChild(char* _name);
    int addChild(pfvXmlNode*child);
    int addLeaf(char* _name, char* val);
    
    int removeChild(pfvXmlNode* child);
    int removeChild(int i);
        
    int write(char* filename);
    int write(FILE*fp, int indent);
	
	
private:

    pfvXmlNode* travLoadExtRefs();
    void setParent(pfvXmlNode* _parent);

    char* name;
    pfList* listAttr;

    char* value;
    
    pfvXmlNode* parent;
    pfList* listChildren;

};

///////////////////////////////////////////////////////////////////////////////

#endif // end of __PFV_XML_H__
