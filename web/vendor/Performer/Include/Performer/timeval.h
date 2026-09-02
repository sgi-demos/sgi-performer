#ifndef	__TIMEVAL_H__
#define	__TIMEVAL_H__

#ifndef	WIN32
#include "sys/time.h"
#else
#include "windows.h"
#endif

int compare_timeval (struct timeval *a, struct timeval *b);
long subtract_timeval (struct timeval *a, struct timeval *b, struct timeval *c);
int add_timeval (struct timeval *a, struct timeval *b, struct timeval *c);

#endif	/* __TIMEVAL_H__ */
