#ifndef _NEWLIB_GLUE_SYS_STAT_H_
#define _NEWLIB_GLUE_SYS_STAT_H_

#include_next <sys/stat.h>

#define UTIME_NOW  0x3fffffff
#define UTIME_OMIT 0x3ffffffe

#endif
