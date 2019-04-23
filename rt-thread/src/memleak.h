#ifndef __MEMLEAK_H__
#define __MEMLEAK_H__

#include <stdlib.h>
#include <stdio.h>
#include "rtconfig.h"

#ifdef MEM_LEAK_CHECK

#define memleak_log         rt_kprintf
#define MAX_MEMLEAK_NUM     60

int list_memleak_insert(const char *thread, const char* function, void* mem, int size, int line);
int list_memleak_delete(void* mem);
int list_memleak_show(void);
int list_memleak_cleanall(void);

#endif /* MEM_LEAK_CHECK */
#endif