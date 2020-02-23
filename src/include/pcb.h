#ifndef PCB_H
#define PCB_H

#include "types.h"

typedef enum _ProcessType {
    user_process,
    sys_task
} ProcessType;

#define LDT_SIZE 2          // LDT 大小
#define PROCESS_STACK_SIZE 0x8000           // 进程堆栈 大小
// 进程控制块
typedef struct _PCB {
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;    // 第7个

    u32 ds;
    u32 es;
    u32 fs;
    u32 gs;

    u32 eip;
    u32 cs;
    u32 eflags;
    u32 p_esp;
    u32 ss;
 
    u16  ldtSel;                        //第17个，此处至开头数据的位置需要保持不变
    Descriptor ldt[LDT_SIZE];
    u32 entry;                          // 进程入口
    u32 priority;                       // 优先级
    int ticks;
    char name[16];                      // 进程名称
    u8 pstack[PROCESS_STACK_SIZE];      // 进程堆栈
    u32 ttyIdx;                         // TTY索引
    ProcessType ptype;
} PCB ;

void addPCB(u32 entry, u32 priority,u32 ttyIdx, ProcessType ptype);
void initPcb();
PCB* getCurrentPcb();
void schedule();

#endif