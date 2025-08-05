
// This file contains the public interface from the Performer
// header file pfdb/pfiv.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%section "libpfdu"

/*
 *  this module contains all the tokens for the performer loaders
 *  all headers in pfdb/
 *  only the pfiv for now 
 */


%{
#include <Performer/pfdb/pfiv.h>
%}

#define PFIV_FLATTEN            0
#define PFIV_CONVERT_XFORMS     1
#define PFIV_CONVERT_LIGHTS     2
#define PFIV_CONVERT_TWOSIDE    3
#define PFIV_CLEAN              4


