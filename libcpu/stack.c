/*
 * File      : stack.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2013, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 */
#include <rtthread.h>
#include <string.h>

/*****************************/
/* CPU Mode                  */
/*****************************/
#define USERMODE        0x10
#define FIQMODE         0x11
#define IRQMODE         0x12
#define SVCMODE         0x13
#define ABORTMODE       0x17
#define UNDEFMODE       0x1b
#define MODEMASK        0x1f
#define NOINT           0xc0

/**
 * This function will initialize thread stack
 *
 * @param tentry the entry of thread
 * @param parameter the parameter of entry
 * @param stack_addr the beginning stack address
 * @param texit the function will be called when thread exit
 *
 * @return stack address
 */
rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter,
                             rt_uint8_t *stack_addr, void *texit)
{
    rt_uint32_t *stk;

    //stk      = (rt_uint32_t*)stack_addr;
    stack_addr += sizeof(rt_uint32_t);
    stack_addr  = (rt_uint8_t *)RT_ALIGN_DOWN((rt_uint32_t)stack_addr, 8);
    stk  = (rt_uint32_t *)stack_addr;

    *(--stk) = (rt_uint32_t)tentry;         /* entry point */
    *(--stk) = (rt_uint32_t)texit;          /* lr */
    *(--stk) = 0xdeadbeef;                  /* r12 */
    *(--stk) = 0xdeadbeef;                  /* r11 */
    *(--stk) = 0xdeadbeef;                  /* r10 */
    *(--stk) = 0xdeadbeef;                  /* r9 */
    *(--stk) = 0xdeadbeef;                  /* r8 */
    *(--stk) = 0xdeadbeef;                  /* r7 */
    *(--stk) = 0xdeadbeef;                  /* r6 */
    *(--stk) = 0xdeadbeef;                  /* r5 */
    *(--stk) = 0xdeadbeef;                  /* r4 */
    *(--stk) = 0xdeadbeef;                  /* r3 */
    *(--stk) = 0xdeadbeef;                  /* r2 */
    *(--stk) = 0xdeadbeef;                  /* r1 */
    *(--stk) = (rt_uint32_t)parameter;      /* r0 : argument */
	/* cpsr */
	if ((rt_uint32_t)tentry & 0x01)
		*(--stk) = SVCMODE | 0x20;			/* thumb mode */
	else
		*(--stk) = SVCMODE;					/* arm mode   */

    /* return task's current stack address */
    return (rt_uint8_t *)stk;
}

void rt_hw_stack_print(rt_thread_t thread)
{
    extern void fiq_handler();
    extern char __rt_init_end;

    uint32_t call_stack_buf[32] = {0};
    uint32_t code_start_addr;
    uint32_t code_end_addr;
    uint32_t stack_start_addr;
    uint32_t stack_end_addr;
    uint32_t sp;
    uint32_t pc;
    int call_stack_index = 0;

    /* TODO: it should be more portable to calculate code space, like _stext _etext */
    code_start_addr = (uint32_t)fiq_handler;
    code_end_addr = (uint32_t)(&__rt_init_end);

    if (NULL == thread) {
        thread = rt_thread_self();
    }

    stack_start_addr = (uint32_t)thread ->stack_addr;
    stack_end_addr = thread->stack_size + stack_start_addr;
    sp = (uint32_t)thread->sp;
    rt_kprintf("task=%.8s,stack=[0x%x,0x%x],sp=0x%x,stackoverflow=%d\n", thread->name, stack_start_addr, stack_end_addr, sp, sp < stack_start_addr);
    for (; sp < stack_end_addr; sp += sizeof(size_t))
    {
        pc = *((uint32_t *) sp);
        /* ARM9 using thumb instruction, so the pc must be an odd number */
        if ((pc & 1) == 0) {
            continue;
        }
        if ((code_start_addr < pc) && (pc < code_end_addr)) {
            call_stack_buf[call_stack_index] = pc - 1;
            call_stack_index++;
        }
    }
    if (call_stack_index > 0)
    {
        int index;
        rt_kprintf("addr2line.exe -e rtthread.elf -a -f");
        for (index = 0; index < call_stack_index; index++) {
            rt_kprintf(" %lx", call_stack_buf[index]);
        }
        rt_kprintf("\n");
    }
}

void rt_stack_print(int argc, char **argv)
{
    struct rt_object_information *info;
    struct rt_thread *thread;
    struct rt_list_node *node;
    size_t length;

    if (argc < 2)
    {
        rt_kprintf("usage: stack task_name|all_tasks\n");
        rt_hw_stack_print(NULL);
        return;
    }

    info = rt_object_get_information(RT_Object_Class_Thread);
    if (NULL == info)
    {
        rt_kprintf("can't find thread list\n");
        return;
    }

    length = strlen((const char *)argv[1]);
    if (length <= 0)
    {
        rt_kprintf("bad thread name\n");
        return;
    }
    if ((length == strlen("all_tasks")) && (0 == strncmp(argv[1], "all_tasks", length)))
    {
        for (node = info->object_list.next; node != &info->object_list; node = node->next)
        {
            thread = rt_list_entry(node, struct rt_thread, list);
            rt_hw_stack_print(thread);
        }
        return;
    }
    if (length > RT_NAME_MAX)
    {
        length = RT_NAME_MAX;
    }

    for (node = info->object_list.next; node != &info->object_list; node = node->next)
    {
        thread = rt_list_entry(node, struct rt_thread, list);
        if (0 != strncmp(argv[1], thread->name, length))
        {
            continue;
        }
        rt_hw_stack_print(thread);
        break;
    }
}

MSH_CMD_EXPORT_ALIAS(rt_stack_print, stack, rt_hw_stack_print);
