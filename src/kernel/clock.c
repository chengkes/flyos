
#include "types.h"
#include "interrupt.h"
#include "lib.h"
#include "pcb.h"

// 时钟芯片8253端口
#define PORT_CLOCK_COUNTER0  0x40
#define PORT_CLOCK_COUNTER1  0x41
#define PORT_CLOCK_COUNTER2  0x42
#define PORT_CLOCK_CONTROL   0x43

#define CLOCK_DEFAULT_HZ    1193180     // 输入频率
#define CLOCK_COUNTER0_HZ   1000   // 每1ms发生一次时钟中断, 该值必须大于18
#define CLOCK_MODE          0x34    

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
    putIrqHandler(CLOCK_HANDLER_IDX, clockHandler);

    // 初始化时钟中断频率
    outByte(CLOCK_MODE, PORT_CLOCK_CONTROL);
    outByte((CLOCK_DEFAULT_HZ/CLOCK_COUNTER0_HZ) & 0xff, PORT_CLOCK_COUNTER0);             // 先写低位
    outByte(((CLOCK_DEFAULT_HZ/CLOCK_COUNTER0_HZ)>>8) & 0xff, PORT_CLOCK_COUNTER0);        // 再写高位
}

// 延迟t毫秒 
void delayMs(u32 t) {
    u32 t1 = ticks;
    while ((ticks - t1 )*1000/CLOCK_COUNTER0_HZ <= t);
}
