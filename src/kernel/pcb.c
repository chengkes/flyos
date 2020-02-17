
#include "types.h"
#include "main.h"

// LDT中 选择子
#define LDT_SELECTOR_D32        (0x00 | SA_TIL | SA_RPL1)   // 数据段 选择子
#define LDT_SELECTOR_C32        (0x08 | SA_TIL | SA_RPL1)   // 代码段 选择子

#define PCB_SIZE 3              // 进程控制块 大小
static PCB pcbs[PCB_SIZE];     // 所有进程
PCB* currentPcb;        // 当前运行的进程


// 测试进程B
static void processB(){
    char a = 'a';
    while(1) {
        // dispChar(a++, 0x0f);
        if (a >  'z') a = 'a';
    }
}

// 测试进程C
static void processC(){
    char a = '0';
    while(1) {
        // dispChar(a++, 0x01);
        if (a >  '9') a = '0';
    }
}

void initPcb(){
    // 添加进程
    addPCB(1, (u32)processB, 500);
    addPCB(2, (u32)processC, 200);
    currentPcb = &pcbs[0];
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
    for (int i=0;i<PCB_SIZE; i++) {
        if (pcbs[i].ticks > maxTicks) {
            maxTicks = pcbs[i].ticks;
            currentPcb = & pcbs[i];
        }
    }
    if (maxTicks == 0){
        for (int i=0;i<PCB_SIZE; i++) {
            pcbs[i].ticks = pcbs[i].priority;
            if (pcbs[i].priority > currentPcb->priority) {
                currentPcb = & pcbs[i];
            }
        }
    }
}

// 添加进程
// num, 进程号， 必须是从0开始的连续数，且小于PCB_SIZE
// entry, 进程入口地址
// priority, 进程优先级，越大优先级越高
void addPCB(u32 num, u32 entry, u32 priority) {
    if (num >= PCB_SIZE) return;
    u32 ldtSel = GDT_SELECTOR_LDT + num*sizeof(Descriptor);
    PCB *pcb = &pcbs[num];
    initDescriptor(&pcb->ldt[LDT_SELECTOR_D32 >> 3], 0, 0xfffff, DA_DRW | DA_DPL1, DA_LIMIT_4K | DA_32);
    initDescriptor(&pcb->ldt[LDT_SELECTOR_C32 >> 3], 0, 0xfffff, DA_CR | DA_DPL1, DA_LIMIT_4K | DA_32);
    initDescriptor(&gdt[ldtSel>>3], (u32)(&pcb->ldt), sizeof(Descriptor)*LDT_SIZE - 1, DA_LDT | DA_DPL1, 0);

    pcb->priority = pcb->ticks = priority;
    pcb->ldtSel = ldtSel;
    pcb->cs = LDT_SELECTOR_C32;
    pcb->ss = pcb->ds = pcb->es = pcb->fs = LDT_SELECTOR_D32;
    pcb->gs = GDT_SELECTOR_VIDEO;
    pcb->eflags = 0x1202 ; // IOPL=1, IF=1
    pcb->entry = pcb->eip = entry;
    pcb->p_esp = (u32)(pcb->pstack+PROCESS_STACK_SIZE);
}

