/* sys/sysmp.h - pfosg stub for the IRIX multiprocessor API.  sysmp() calls
 * degrade to "one processor / success". */
#ifndef PFOSG_SYSMP_H
#define PFOSG_SYSMP_H

#define MP_NPROCS    1
#define MP_NAPROCS   2
#define MP_PGSIZE    14

extern int sysmp(int cmd, ...);

#endif
