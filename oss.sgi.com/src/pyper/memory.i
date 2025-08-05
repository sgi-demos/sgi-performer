
// This file contains the public interface from the Performer
// header file pr/pfMemory.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{

#include <Performer/pr/pfMemory.h>

%}

class pfMemory
{
  public:
    pfMemory();
    ~pfMemory();
    int ref();
    int unref();
    int getRef();
};



