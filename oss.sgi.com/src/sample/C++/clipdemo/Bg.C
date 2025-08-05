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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>


#include <sys/schedctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>

#include <fcntl.h>
#include <termio.h>


#include "BgPriv.h"

#define CLAMP(var) (var < .05 && var > -.05 ? 0 : var)


void rf_quit(void);

// Global data
static bglv bgdata;

RS_ERR rs_err;

void ExitFLYBOX(void)
{
    close_lv (&bgdata);
}


void ReadFLYBOX(float *xjoy, float *yjoy, float *yaw, float *accel1, float *accel2 )
{
    int st;
    float scale, f, s;
    float sin, cos;
    int i, j;

	st = w_lv(bgdata.sp_fd, "o");

	st = r_lv(&bgdata);

        // joy x
        *xjoy = bgdata.ain[0];
	// joy y
        *yjoy = bgdata.ain[1];
	// joy twist
	*yaw = bgdata.ain[2];
	// lever 1
	*accel1 = bgdata.ain[3];
	// lever2
	*accel2 = bgdata.ain[4];

	/*
        for ( j = 0; j <=2; j++ ) {
            if ( bgdata.dig_in & 0x10 << j ) {
                for ( i = 0; i < 8; i++ ) {
                    if ( (bgdata.din[j]>>i) & 0x1 )
                        printf("1");
                    else
                        printf("0");
                }
                printf("  ");
            }
        }

        printf("\n");
	*/
} 


void InitFLYBOX ()
{
    int st;

    // Defaults to 5 analog, and 16 discretes
    bgdata.analog_in = 0;
    bgdata.analog_in = AIC1 | AIC2 | AIC3 | AIC4 | AIC5;

    bgdata.dig_in = 0;
    bgdata.dig_in = DIC1 | DIC2;

    // Set the baud rate
    bgdata.baud = BAUD192;

    // Open the port & drivers
    st = open_lv (&bgdata);
    if (st < 0) {
	printf("Unable to open port\n");
	exit(-1);
    }

    // Send the init string
    st = init_lv(&bgdata);
    if ( st < 0 ) {
	check_setup(&bgdata);
	printf("Invalid setup requested.  Bye\n");
	exit(-1);
    }

}

int open_lv (bglv *bgp)
{
    int  st;
    char  port[4];
    char pt[32];
    char *ep;

    rs_err.wrt = 0;
    rs_err.rd = 0;
    rs_err.len = 0;
    rs_err.nl = 0;
    rs_err.cycles = 0;
    rs_err.thou = 0;

    // Initialize port
    if ( ep = getenv("FBPORT") )
	sprintf(pt,"%s",ep);
    else 
	sprintf(pt,"%s",FBPORT);

    port[0] = pt[strlen(pt)-1];
    bgp->port = atoi(port);

    printf ("****** trying to open port %s\n", pt);

    bgp->sp_fd = open(pt, O_RDWR|O_NDELAY);

    if (bgp->sp_fd < 0) {
	perror(pt);
	return(-1);
    }

    st = set_baud(bgp->sp_fd);

    st = check_rev(bgp);
    if ( st < 0 )
	return(st);
    else
	return(0);

} // End open_lv

int set_baud (int sp_fd)
{
   struct termio tios;
   int st;

   st = ioctl(sp_fd,TCGETA,&tios);
   tios.c_iflag = IGNBRK|IXON|IXOFF;
   tios.c_oflag = 0;
   tios.c_lflag = ICANON;

   tios.c_cflag = B19200|CS8|CREAD|CLOCAL;
   tios.c_cflag = CS8|CREAD|CLOCAL;
   //tios.c_ospeed = B19200;

   st = ioctl(sp_fd,TCSETAF,&tios);
   return(st);


} // End set_baud


int init_lv(bglv *bgp)
{
    char c1, c2, c3, str[5];
    int  st, i;

    st = check_setup(bgp);
    if ( st < 0 )
	return(st);

    //  Compute the number of channels requested, and the 
    //  appropriate string length.

    //  Analog inputs
    bgp->n_analog_in = 0;
    for ( i=0; i < 8; i++)
	if ( (bgp->analog_in >> i) & 0x1 )
	    bgp->n_analog_in++;

    //  Digital inputs
    switch(bgp->dig_in) {
	case 0x0:
	    bgp->n_dig_in = 0;
	    break;
	case 0x10:
	case 0x20:
	case 0x40:
	    bgp->n_dig_in = 8;
	    break;
	case 0x30:
	case 0x50:
	case 0x60:
	    bgp->n_dig_in = 16;
	    break;
	case 0x70:
	    bgp->n_dig_in = 24;
	    break;
    }

    //  Digital outputs
    switch(bgp->dig_out) {
	case 0x0:
	    bgp->n_dig_out = 0;
	    break;
	case 0x10:
	case 0x20:
	case 0x40:
	    bgp->n_dig_out = 8;
	    break;
	case 0x30:
	case 0x50:
	case 0x60:
	    bgp->n_dig_out = 16;
	    break;
	case 0x70:
	    bgp->n_dig_out = 24;
	    break;
    }


    //  Analog outputs
    bgp->n_analog_out = 0;
    if ( bgp->analog_out > 0 ) {
	for ( i=0; i < 3; i++)
	    if ( (bgp->analog_out >> i) & 0x1 )
		bgp->n_analog_out++;
    }


    //  Set the string length for receiving data
    bgp->str_len  = 2 + (2*bgp->n_analog_in) + (bgp->n_dig_in/4);


    //  First character has the baud rate and the lower 4 analog ins.
    c1 =  bgp->baud;
    c1 |= (bgp->analog_in & 0xf);

    //  Second character has the digital inputs and the upper 4 analog ins
    c2 =  bgp->dig_in;
    c2 |= (bgp->analog_in & 0xf0) >> 4;

    if ( bgp->Rev.major == 3 ) {
	str[0] = 's';

	//  Third character (for rev 3 eproms only, has the digital outs (-F)
	//  and analog outs (-3G)
	c3 =  bgp->analog_out & 0xf;
	c3 |= bgp->dig_out & 0xf0;

	// Add the OFFSET to each character to make sure they are not control
	// characters
	str[1] = c1 + OFFSET;
	str[2] = c2 + OFFSET;
	str[3] = c3 + OFFSET;
	str[4] = '\0';
	st = w_lv(bgp->sp_fd, str);

	//  Make sure that the LV got the setup !
	st = get_ack(bgp->sp_fd);

	//  If we have a rev 3.00 eprom, just don't check the return
	//  value - just proceed and assume things are OK.
	//  (Bug fixed in 3.01)
	if ( bgp->Rev.bug != 0 ) {
	    if ( st < 0 )
		return(st);
	}

    } else if ( bgp->Rev.major == 2 ) {
	if ( bgp->Rev.minor == 2 ) {
	    //  For rev 2.2 EPROMS use an 'R' and no offset -- so make 
	    // sure c1 and c2 are not flow control characters !
	    str[0] = 'R';
	    str[1] = c1;
	    str[2] = c2;
	} else if ( bgp->Rev.minor >= 3 ) {
	    //  For rev 2.3 EPROMS use an 'r' and offset the characters
	    str[0] = 'r';
	    str[1] = c1 + OFFSET;
	    str[2] = c2 + OFFSET;
        }
        str[3] = '\0';
        st = w_lv(bgp->sp_fd, str);
    }
 
    st = set_baud(bgp->sp_fd);
 
    return(0);
}


int get_ack (int sp_fd)
{
   int st;
   int i = 0;
   int chars = 2;
   char str[36];
 
   st = read(sp_fd,str,chars);
   if (st < 0) {
      printf("get_ack():  read error\n");
      return(-1);
   }
   while ( st != 2 && i < 10000) {
      sginap(1);
      st = read(sp_fd,str,chars);
      i++;
   }
   if ( i > 10000 )
      printf("Timeout %d chars in buffer \n", chars);
 
   if (str[0] == 'a' ) { 
      printf("Setup OK\n"); 
      return (0); 
   } else if (str[0] == 'f' ) { 
      printf("Setup failed\n"); 
      return (-1); 
   } else { 
      printf("Unexpected respons: %s\n", str); 
      return (-2); 
   }
   //return(st);
}



static char Cpy[] = "Copyright (c), BG Systems";

int check_rev(bglv *bgp)
{
   int st;
   int chars_read = 0;
   char str[64];

/*
 *  Send a "T" and see if the Box responds
 */
   st = write(bgp->sp_fd, "T", 2);
   sginap(100); 
   chars_read = read(bgp->sp_fd, str, 44);

/*
 *  If chars_read <= 0, looks like we have a Rev 1.x EPROM
 */
   if (chars_read <= 0)
   {
      no_answer();
      return(-1);
   }
   else
   {
/*
 *  Check the string length
 */
      if ( chars_read != 44 )
      {
         printf("Unexpected characters:  %d  %s\n", chars_read, str);
         return(-1);
      }
      else
      {
/*
 *  Check that it is the Copyright string
 */
         if ( strncmp(str, Cpy, strlen(Cpy)) != 0 )
         {
            printf("Unexpected characters:  %d  %s\n", chars_read, str);
            return(-1);
         }
         else
         {
/*
 *  If we go this far, we should have the right string 
 */
            bgp->Rev.year  = parse_year(str);
            bgp->Rev.major = str[38]-48;
            bgp->Rev.minor = str[40]-48;
            bgp->Rev.bug   = str[41]-48;
            bgp->Rev.alpha = str[42];
	    /*
            printf("%s %d  Revision %d.%d%d%c\n", Cpy, bgp->Rev.year,
                      bgp->Rev.major,  bgp->Rev.minor,
                      bgp->Rev.bug, bgp->Rev.alpha );
		      */
         }
      } 
   }
 
   return (bgp->Rev.major);
}

int parse_year(char *s)
{
   int i = 0;
   char yr[12];

   while ( *s != '1' )
      *s++;
   yr[i] = *s;
   while ( *s != ' ' && *s != ',' )
      yr[i++] = *s++;
   yr[i] = '\0';
   return(atoi(yr));
}

int check_setup(bglv *bgp)
{
   int i;
   int st = 0;

/*
 *  This routine checks the EPROM revision against the
 *  requested setup, and attempts to identify inconsistencies !
 */

   if ( bgp->Rev.major == 2 )
   {
      if ( bgp->analog_out != 0x0 )
      {
         printf("  Analog outputs not supported by LV816\n");
         st = -1;
      }
      if ( bgp->dig_out != 0x0 )
      {
         printf("  Digital outputs not supported by LV816\n");
         st = -2;
      }
      if ( bgp->dig_in & 0x40 )
      {
         printf("  Digital inputs 19-24 not supported by LV816\n");
         st = -3;
      }
   }
   else if ( bgp->Rev.major == 3 )
   {
      switch(bgp->Rev.alpha)
      {
       case 'e':
         printf("LV824-E\n");
         if ( bgp->analog_out != 0x0 )
         {
            printf("  Analog outputs not supported\n");
            st = -1;
         }
         if ( bgp->dig_out != 0x0 )
         {
            printf("  Digital outputs not supported\n");
            st = -2;
         }
         break;
       case 'f':
         printf("LV824-F\n");
         if ( bgp->analog_out != 0x0 )
         {
            printf("  Analog outputs not supported\n");
            st = -2;
         }
         break;
       case 'g':
         printf("LV824-G\n");
         break;
       default:
         st = -3;
         printf("Not an LV824 board\n");
         break;
      }
      if ( st < 0 )
         return(st);
/*
 *  Check also for conflict in the digital channels
 */

      if ( bgp->dig_in && bgp->dig_out )
      {
         for ( i = 0; i < 3; i++ )
         {
            if ( ( (bgp->dig_in >> i) &0x1 ) 
                 && ( (bgp->dig_out >> i) &0x1 ) )
            {

printf("Invalid set-up requested.\n");
printf("  Digital input group %d AND output group %d selected\n",                        i+1, i+1);

printf("\n\n  Digital channels can be set in groups of 8 as\n");
printf("  either inputs or outputs.\n");
printf("  Of course you can (for example) set the bottom 8\n");
printf("  to inputs DIC1 and the top 16 to outputs DOC2 | DOC3\n");

               st = -5;
               return(st);
            }
         }
      }
   }
   return(st);
}

void no_answer()
{
   printf("\nWriting a 'T' to the Box produced no answer.  \n");
   printf("\n");
   printf("The expected string was not returned from the BG box.\n");
   printf("Here are some possible problems:\n");
   printf("   1. Check power to Box\n");
   printf("   2. Check the serial cable\n");
   printf("   3. Check the environment variable FBPORT\n");
   printf("      - does it match the connected serial port ?\n");
   printf("   4. Is the serial port configured as a terminal ? \n");
   printf("      - if so use \"System Manager\" to disconnect the port\n");
   printf("   5. You have an old FlyBox (serial no. less than 60) \n");
   printf("         which has a revision 1.0 EPROM.  Call BG Systems.\n");

   printf("\n\n");
}

int w_lv(int sp_fd, char *mode)
{
    int st;

    st = write(sp_fd, mode, strlen(mode));
    if (st < 0)
	rs_err.wrt++;
    return(st);
}




int r_lv(bglv *bgp)
{
   int st;
   int i = 0;
   char str[36];
 
   rs_err.cycles++;
   if( rs_err.cycles % 1000 == 0 ) {
      rs_err.cycles = 0;
      rs_err.thou++;
   }
 
   st = read(bgp->sp_fd,str,bgp->str_len);
   if (st < 0) {
      rs_err.rd++;
      printf("r_lv():  read error\n");
      return(-1);
   }
   while ( st != bgp->str_len && i < 100) {
      st = read(bgp->sp_fd,str,bgp->str_len);
      i++;
   }

   if ( i > 0 )
      //printf("%d read attempts.  \n", i);
 
   if (str[0] != 'B' || str[bgp->str_len - 1] != '\n') {
      printf("%d:  %s\n", st, str);
      rs_err.rd++;
      return(-1);
   }

   st = convert_serial(bgp, str);
 
   return st;
}


int convert_serial(bglv *bgp, char *str)
{
   int i, digp, j;
   int k = 0;
   float tmp[8];

   digp = 0;

    //  Load the digital input values into dioval
   k = 1 + bgp->n_dig_in/4;
   if ( k > 1) {
      i = 1;
      for ( j = 2; j >= 0; j-- ) {
         if ( bgp->dig_in & 0x10<<j ) {
            digp = 0x0f & (str[i++]-0x21);
            digp = (digp << 4) | 0x0f & (str[i++]-0x21);
            bgp->din[j] = digp;
         }
      }
   }

 //  Load the 8 analog values into inbuf
   for (i = k; i < bgp->str_len - 2; i += 2) {
      digp = ((0x3f & (str[i]-0x21)) << 6) |
                    (0x3f & (str[i+1]-0x21));
      tmp[(i-k)/2] = -1.0 + (2.0 * digp/4095);
   }
   for ( i = 0, k = 0; k < 8; k++ ) {
      if ( bgp->analog_in >> k &0x1 ) {
         bgp->ain[k] = tmp[i];
	 i++;
      }
   }

   digp =  ((0x0f & (str[22]-0x21)) <<  4) |
           (0x0f & (str[23]-0x21));
   return (0);
}

void close_lv(bglv *bgp)
{
    int att;
    int st;

    bgp->baud = BAUD192;
    st = init_lv(bgp);

    att = 1000*rs_err.thou + rs_err.cycles;
    close(bgp->sp_fd);
    //printf("\nRead Attempts:  %d\n", att);
    //printf("\nErrors Detected\n");
    //printf("Read        Write    \n");
    //printf("%5d      %5d     \n",rs_err.rd, rs_err.wrt);

}
