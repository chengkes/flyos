
#include "types.h"
#include "tty.h"
#include "pcb.h"
#include "keyboard.h"
#include "main.h"

// LDT中 选择子
#define LDT_SELECTOR_D32        (0x00 | SA_TIL )   // 数据段 选择子
#define LDT_SELECTOR_C32        (0x08 | SA_TIL )   // 代码段 选择子

#define PCB_SIZE 128             // 进程控制块 大小
static PCB pcbs[PCB_SIZE];       // 所有进程
static int pcbCount;
PCB* currentPcb;        // 当前运行的进程

// 测试进程A , todo:添加READ系统调用
static void processA(){
    char a[2] = "A";
    while(1) {
        u32 key = readKey();
        if (key!=0 && (key& KEYBOARD_FLAG_EXT)==0) {  // 可打印字符 
            a[0] = 0x7f & key;
            printf(a, cyan);
        }
        // delayMs(1000);
    }
}

// 测试进程B
static void processB(){
    char a[2] = "a";
    while(1) {
        printf(a, blue);
        a[0] ++;
        if (a[0] > 'z') a[0] = 'a';
        delayMs(1000);
    }
}

// 测试进程C
static void processC(){
    char a[2] = "0";
    while(1) {
        printf(a, green);
        a[0] ++;
        if (a[0] > '9') a[0] = '0';
        delayMs(1000);
    }
}

void initPcb(){
    pcbCount = 0;
    currentPcb = &pcbs[0];
    
    addPCB((u32)taskTty, 1, 0, sys_task);
    addPCB((u32)processB, 5, 1, user_process);  // todo: 测试代码
    addPCB((u32)processC, 2, 2, user_process);
    addPCB((u32)processA, 3, 0, user_process);    
}

PCB* getCurrentPcb(){
    return currentPcb;
}

// 进程调度算法
void schedule(){
    // ---------- 循环调用 ---------------
    // currentPcb++;
    // if (currentPcb >= pcbs + PCB_SIZE)  currentPcb=pcbs;

    // ---------- 优先级调度 ---------------
    if(currentPcb->ticks > 0) {
        return;
    }
    int maxTicks = 0;
    for (int i=0;i<pcbCount; i++) {
        if (pcbs[i].ticks > maxTicks) {
            maxTicks = pcbs[i].ticks;
            currentPcb = & pcbs[i];
        }
    }
    if (maxTicks == 0){
        for (int i=0;i<pcbCount; i++) {
            pcbs[i].ticks = pcbs[i].priority;
            if (pcbs[i].priority > currentPcb->priority) {
                currentPcb = & pcbs[i];
            }
        }
    }
}

// 添加新进程
// entry, 进程入口地址
// priority, 进程优先级，越大优先级越高
// ttyIdx, TTY索引
// pt, 进程类型，系统任务还是用户进程
void addPCB(u32 entry, u32 priority, u32 ttyIdx, ProcessType pt) {
    if (pcbCount >= PCB_SIZE) return;
    u32 ldtSel = GDT_SELECTOR_LDT + pcbCount*sizeof(Descriptor);
    PCB *pcb = &pcbs[pcbCount];
    u32 dpl = DA_DPL3;
    u32 rpl = SA_RPL3;
    if (pt == sys_task){    
        dpl = DA_DPL1;
        rpl = SA_RPL1;
    }
    initDescriptor(&pcb->ldt[LDT_SELECTOR_D32 >> 3], 0, 0xfffff, DA_DRW | dpl, DA_LIMIT_4K | DA_32);
    initDescriptor(&pcb->ldt[LDT_SELECTOR_C32 >> 3], 0, 0xfffff, DA_CR | dpl, DA_LIMIT_4K | DA_32);
    initDescriptor(&gdt[ldtSel>>3], (u32)(&pcb->ldt), sizeof(Descriptor)*LDT_SIZE - 1, DA_LDT | dpl, 0);
    pcb->priority = pcb->ticks = priority;
    pcb->ldtSel = ldtSel;
    pcb->cs = LDT_SELECTOR_C32 | rpl;
    pcb->gs =pcb->ss = pcb->ds = pcb->es = pcb->fs = LDT_SELECTOR_D32 | rpl;
    pcb->eflags = 0x1202 ;          // IOPL(12,13)=1, IF(9)=1
    pcb->ptype = pt;
    pcb->entry = pcb->eip = entry;
    pcb->p_esp = (u32)(pcb->pstack+PROCESS_STACK_SIZE);
    pcb->ttyIdx = ttyIdx;

    pcbCount++;
}
