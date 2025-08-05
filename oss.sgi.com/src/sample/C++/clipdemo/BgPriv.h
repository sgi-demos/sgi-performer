/*
 * Copyright 1997, 1998, 1999, 2000, Silicon Graphics, Inc.
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

#ifndef __BG_H__
#define __BG_H__


#include <Performer/pf.h>

#define FBPORT "/dev/ttyd2"

/* from lv3.h */
#include <time.h>

#define FLYBOX    1
#define BEEBOX    2
#define CEREALBOX 3
#define CAB       4
#define DRIVEBOX  5

#define FB_NOBLOCK 1
#define FB_BLOCK 2

#define AIC1   0x01
#define AIC2   0x02
#define AIC3   0x04
#define AIC4   0x08
#define AIC5   0x10
#define AIC6   0x20
#define AIC7   0x40
#define AIC8   0x80

#define AOC1   0x01
#define AOC2   0x02
#define AOC3   0x04

#define DIC1    0x10
#define DIC2    0x20
#define DIC3    0x40

#define DOC1    0x10
#define DOC2    0x20
#define DOC3    0x40

#define BAUD576 0x70
#define BAUD384 0x60
#define BAUD192 0x50
#define BAUD96  0x40
#define BAUD48  0x30
#define BAUD24  0x20
#define BAUD12  0x10

#define OFFSET  0x21

/*
 *  Define some commands
 */

#define BURST       'B'    /* Burst mode                  */
#define BURST_SET   'b'    /* Burst mode rate set         */
#define CONT        'c'    /* Continuous buffered         */
#define DEFAULT     'd'    /* Reset to Default            */
#define PACKET      'p'    /* One input and one output    */
#define ONCE        'o'    /* One input                   */
#define ONCE_CS     'O'    /* One input with check sum    */
#define RESET_FB    'r'    /* Reset 3 chars with offset   */
#define RESET_FB_O  'R'    /* Reset (rev 2.2 no offset)   */
#define STOP        'S'    /* Stop burst mode             */
#define SETUP       's'    /* Setup rev 3.0 eprom         */
#define TEST1       'T'    /* Test (and copyright)        */
#define TEST2       't'    /* Test (and copy, and rev #)  */

typedef struct rs_struct
{
  int  wrt;     /* write error */
  int  rd;      /* read error  */
  int  len;     /* string length error  */
  int  nl;      /* last char error  */
  int  cycles;  /* numer of cycles */
  int  thou;    /* thousands of cycles */
} RS_ERR;

typedef struct REVISION
{
   int    major;          /*  Software major revision             */
   int    minor;          /*  Software minor revision             */
   int    bug;            /*  Software bug revision               */
   char   alpha;          /*  EPROM alpha revision                */
   int    year;
}revision;

/*
 *  For v3.0 software, define a new structure
 */
typedef struct BGLV_STRUCT
{
   int    n_analog_in;    /*  Number of analog inputs (8 max)     */
   int    analog_in;      /*  Analog input selector               */
   int    n_dig_in;       /*  Number of digital inputs (24 max)   */
   int    dig_in;         /*  Digital input selector              */
   int    n_analog_out;   /*  Number of analog outputs (3 max)    */
   int    analog_out;     /*  Analog out channel selector         */
   int    n_dig_out;      /*  Number of digital outputs (24 max)  */
   int    dig_out;        /*  Digital output selector             */
   float  ain[8];         /*  Analog input data                   */
   int    aout[3];        /*  Analog output data                  */
   int    din[3];         /*  Digital input data                  */
   int    dout[3];        /*  Digital output data                 */
   long   count;
   int    str_len;        /*  Length of string to expect          */
   int    baud;           /*  Baud rate selected                  */
   char   mode[2];        /*  Mode to send - rev 2.2              */
   time_t tag;    
   int    port;
   int    box_type;       /*  Device type                         */
   int    sp_fd;          /*  Serial port file descriptor         */
   revision   Rev;        /*  Software major revision             */
}bglv;


/* end from lv3.h */

void setup_lv ();
int open_lv (bglv *);
void close_lv(bglv *);
int set_baud (int);
int init_lv (bglv *);
int get_ack (int);

int check_rev(bglv *);
int parse_year(char *);
int check_setup(bglv *);
int w_lv(int, char *);
int r_lv(bglv *);
int convert_serial(bglv *, char *);
void no_answer(void);
void init_timer();



#endif
