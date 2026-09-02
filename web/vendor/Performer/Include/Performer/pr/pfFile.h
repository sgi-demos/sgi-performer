//
// Copyright 1995, Silicon Graphics, Inc.
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
// pfFile.h
//
// $Revision: 1.13 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_FILE_H__
#define __PF_FILE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
     
typedef struct
{
    int         valid;
    int         newstate;
    off_t       off;
    int         whence;
    int         nbyte;
    char       *buf;
} Cmd;

extern "C" {     // EXPORT to CAPI
/* ----------------------- pfFile Tokens ----------------------- */

#define PFRTF_NOACTION	    0
#define PFRTF_CREATE	    1
#define PFRTF_OPEN	    2
#define PFRTF_READ	    3
#define PFRTF_WRITE	    4
#define PFRTF_SEEK	    5
#define PFRTF_CLOSE	    6
#define PFRTF_PENDING	    7

#define PFRTF_STATUS	    0
#define PFRTF_CMD	    1
#define PFRTF_BYTES	    2
#define PFRTF_OFFSET	    3
#define PFRTF_PID           4
#define PFRTF_MAXREQ        32

/* ------------------ pfFile Related Functions--------------------- */

extern pfFile* pfOpenFile(char* _fname, int _oflag, ...);

}

#define PFFILE ((pfFile*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFFILEBUFFER ((pfFile*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfFile : public pfObject
{
private:
    // constructors and destructors
    pfFile();

public:
    virtual ~pfFile();
    // CAPI:verb
    static pfFile*	create(char* _fname, mode_t _mode);
    // CAPI:private - for varargs
    static pfFile*	open(char* _fname, int _oflag, ...);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

public:
    // sets and gets
    int	getStatus(int _attr) const;
    
public:
    // other functions
    // CAPI:verb
    int		read(char* _buf, int _nbyte);
    int		write(char* _buf, int _nbyte);
    off_t	seek(off_t off, int _whence);
    int		close();
   

private:
    int		status;
    int		next;
    int		lastact;
    pid_t	pid;
    int		fd;
    int		oflag;
    mode_t      mode;
    char       *fname;
    Cmd		cmd[PFRTF_MAXREQ];
    int		curcmd;
    int		nextcmd;

private:
    static pfType *classType;
public:
    void *_pfFileExtraData;
};

#endif // !__PF_FILE_H__
