#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memleak.h"
#include <rtthread.h>

#ifdef MEM_LEAK_CHECK

typedef struct _user_memleak_t
{
    const char thread[8];
    const char function[16];
    int   line;
    
    void* mem;
    int   size;
} user_memleak_t;

static user_memleak_t g_memleaks[MAX_MEMLEAK_NUM];

/**
 * 1:success; 0:fail
 */
int list_memleak_insert(const char *thread, const char* function, void* mem, int size, int line)
{
    int i;
    rt_uint32_t level;

    level = rt_hw_interrupt_disable();
    for(i = 0; i < MAX_MEMLEAK_NUM; i++)
    {
        if (g_memleaks[i].mem != NULL) continue;
        g_memleaks[i].mem = mem;
        g_memleaks[i].size = size;
        g_memleaks[i].line = line;

        if (thread)
            strncpy(g_memleaks[i].thread, thread, sizeof(g_memleaks[i].thread)-1);
        else
            strncpy(g_memleaks[i].thread, "thread", sizeof(g_memleaks[i].thread)-1);

        if (function)
            strncpy(g_memleaks[i].function, function, sizeof(g_memleaks[i].function)-1);
        else
            strncpy(g_memleaks[i].function, "function", sizeof(g_memleaks[i].function)-1);

        rt_hw_interrupt_enable(level);
        return 1;
    }
    rt_hw_interrupt_enable(level);
    return 0;
}

/**
 * 1:success; 0:fail
 */
int list_memleak_delete(void* mem)
{
    int i;
    rt_uint32_t level;
    
    level = rt_hw_interrupt_disable();    
    for(i = 0; i < MAX_MEMLEAK_NUM; i++)
    {
        if (g_memleaks[i].mem != mem) continue;
        
        g_memleaks[i].mem = NULL;
        memset(g_memleaks[i].function, 0, sizeof(g_memleaks[i].function));
        memset(g_memleaks[i].thread, 0, sizeof(g_memleaks[i].thread));

        rt_hw_interrupt_enable(level);

        return 1;
    }
    rt_hw_interrupt_enable(level);
    return 0;
}

int list_memleak_show(void)
{
    int i;
    rt_uint32_t level;

    level = rt_hw_interrupt_disable();

    memleak_log("[index]  ---mem---  ---size---  ---thread--- ---function---  ---line---\r\n");
    
    for(i = 0; i < MAX_MEMLEAK_NUM; i++)
    {
        if (g_memleaks[i].mem == NULL) continue;

        rt_kprintf("[  %3d] ", i);

        rt_kprintf(" 0x%08x   ", g_memleaks[i].mem);

        if (g_memleaks[i].size < 1024)
            rt_kprintf("%5d", g_memleaks[i].size);
        else if (g_memleaks[i].size < 1024 * 1024)
            rt_kprintf("%4dK", g_memleaks[i].size / 1024);
        else
            rt_kprintf("%4dM", g_memleaks[i].size / (1024 * 1024));

        rt_kprintf("      %-8.*s  ", 8, g_memleaks[i].thread);
        rt_kprintf("  %-16.*s  ", 16, g_memleaks[i].function);
        rt_kprintf(" %4d", g_memleaks[i].line);
        rt_kprintf("\n");
        
        // memleak_log("  %p  %d  [%-8s]   %-16s  %4d \r\n", g_memleaks[i].mem, g_memleaks[i].size, g_memleaks[i].thread, g_memleaks[i].function, g_memleaks[i].line);
    }
    rt_hw_interrupt_enable(level);
    return 1;    
}
MSH_CMD_EXPORT(list_memleak_show, dump memory leak information);
MSH_CMD_EXPORT_ALIAS(list_memleak_show, memleak_show, memleak_show);

int list_memleak_cleanall(void)
{
  memset(&g_memleaks[0], 0, sizeof(g_memleaks));
  return 0;    
}
INIT_APP_EXPORT(list_memleak_cleanall);

#endif /* MEM_LEAK_CHECK */
