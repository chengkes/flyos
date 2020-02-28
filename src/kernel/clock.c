
#include "types.h"
#include "interrupt.h"
#include "lib.h"
#include "pcb.h"
#include "clock.h"

static volatile u32 ticks;      // 时钟中断发生次数
volatile u32 isInt ;     // 是否在处理中断程序

// 时钟中断处理程序
static void clockHandler(){
    ticks++;
    PCB* pcb = getCurrentPcb();
    (pcb->ticks)--;
    if(isInt != 0) {   // 发生中断重入，内核运行时发生的中断，此时 esp 指向内核堆栈，不能切换进程
        return;
    }

    // 没有中断重入，进程运行时发生的中断，可以进行进程切换
    schedule();
}

void initClock(){
    isInt = 0;
    ticks = 0;     
    putIrqHandler(IRQ_HANDLER_IDX_CLOCK, clockHandler);

    // 初始化时钟中断频率
    outByte(CLOCK_MODE, PORT_CLOCK_CONTROL);
    outByte((CLOCK_DEFAULT_HZ/CLOCK_COUNTER0_HZ) & 0xff, PORT_CLOCK_COUNTER0);             // 先写低位
    outByte(((CLOCK_DEFAULT_HZ/CLOCK_COUNTER0_HZ)>>8) & 0xff, PORT_CLOCK_COUNTER0);        // 再写高位
}

u32 getTicks(){
    return ticks;
}

// 延迟t毫秒 
void delayMs(u32 t) {
    u32 t1 = ticks;
    while ((ticks - t1 )*1000/CLOCK_COUNTER0_HZ <= t);
}
