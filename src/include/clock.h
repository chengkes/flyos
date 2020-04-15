#ifndef CLOCK_H
#define CLOCK_H

// 时钟芯片8253端口
#define PORT_CLOCK_COUNTER0  0x40
#define PORT_CLOCK_COUNTER1  0x41
#define PORT_CLOCK_COUNTER2  0x42
#define PORT_CLOCK_CONTROL   0x43

#define CLOCK_DEFAULT_HZ    1193180     // 输入频率
#define CLOCK_COUNTER0_HZ   1000        //每秒发生时钟中断次数，该值必须大于18 
#define CLOCK_MODE          0x34        


u32 getTicks();
void initClock();
void delayMs(u32);
void wait4Schedule();
u32 getScheduleTicks() ;

#endif