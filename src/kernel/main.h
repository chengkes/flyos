
#ifndef MAIN_H
#define MAIN_H

#include "types.h"

// 中断控制器8259A 主片、从片端口号
#define PORT_8259A_MASTER1 0x20
#define PORT_8259A_MASTER2 0x21
#define PORT_8259A_SLAVE1 0xA0
#define PORT_8259A_SLAVE2 0xA1
// 外部中断对应中断号
#define INT_VECTOR_IRQ0    0x20
#define INT_VECTOR_IRQ8    0x28

// 显示器端口
#define PORT_DISPLAY_CRTC_ADDR  0x3D4
#define PORT_DISPLAY_CRTC_DATA  0x3D5

#define CRTC_CURSOR_LOC_H  0x0E
#define CRTC_CURSOR_LOC_L  0x0F
#define CRTC_START_ADDR_H  0x0C
#define CRTC_START_ADDR_L  0x0D

/*----------------------------------------------------------------------------
; 描述符类型值说明
; 其中:
;       DA_  : Descriptor Attribute
;       D    : 数据段
;       C    : 代码段
;       S    : 系统段
;       R    : 只读
;       RW   : 读写
;       A    : 已访问
;       其它 : 可按照字面意思理解
;----------------------------------------------------------------------------*/
#define  DA_32			0x40	// D?B: 32 位段
#define  DA_LIMIT_4K	0x80	// G: 段界限粒度为 4K 字节
#define  DA_DPL0		0x00	// DPL = 0
#define  DA_DPL1		0x20	// DPL = 1
#define  DA_DPL2		0x40	// DPL = 2
#define  DA_DPL3		0x60	// DPL = 3
/*----------------------------------------------------------------------------
; 存储段描述符类型值说明
;----------------------------------------------------------------------------*/
#define DA_DR			0x90	// 存在的只读数据段类型值
#define DA_DRW			0x92	// 存在的可读写数据段属性值
#define DA_DRWA			0x93	// 存在的已访问可读写数据段类型值
#define DA_C			0x98	// 存在的只执行代码段属性值
#define DA_CR			0x9A	// 存在的可执行可读代码段属性值
#define DA_CCO			0x9C	// 存在的只执行一致代码段属性值
#define DA_CCOR			0x9E	// 存在的可执行可读一致代码段属性值
/*----------------------------------------------------------------------------
; 系统段描述符类型值说明
;----------------------------------------------------------------------------*/
#define DA_LDT			  0x82	// 局部描述符表段类型值
#define DA_TaskGate		  0x85	// 任务门类型值
#define DA_386TSS		  0x89	// 可用 386 任务状态段类型值
#define DA_386CGate		  0x8C	// 386 调用门类型值
#define DA_386IGate		  0x8E	// 386 中断门类型值
#define DA_386TGate		  0x8F	// 386 陷阱门类型值
/*----------------------------------------------------------------------------
; 选择子类型值说明
; 其中:
;       SA_  : Selector Attribute
;----------------------------------------------------------------------------*/
#define SA_RPL0			0	// ┓
#define SA_RPL1			1	// ┣ RPL
#define SA_RPL2			2	// ┃
#define SA_RPL3			3	// ┛
#define SA_TIG			0	// TI=0, GDT
#define SA_TIL			4	// TI=1, LDT

// GDT中 选择子
#define GDT_SELECTOR_D32        0x08    // 数据段 选择子
#define GDT_SELECTOR_C32        0x10    // 代码段 选择子
#define GDT_SELECTOR_VIDEO      (0x18 | SA_RPL3)    // 视频段 选择子
#define GDT_SELECTOR_TSS        0x20    // TSS 选择子
#define GDT_SELECTOR_LDT        0x28    // LDT 选择子

// LDT中 选择子
#define LDT_SELECTOR_D32        (0x00 | SA_TIL | SA_RPL1)   // 数据段 选择子
#define LDT_SELECTOR_C32        (0x08 | SA_TIL | SA_RPL1)   // 代码段 选择子


// 时钟芯片8253端口
#define PORT_CLOCK_COUNTER0  0x40
#define PORT_CLOCK_COUNTER1  0x41
#define PORT_CLOCK_COUNTER2  0x42
#define PORT_CLOCK_CONTROL   0x43

#define CLOCK_DEFAULT_HZ    1193180     // 输入频率
#define CLOCK_COUNTER0_HZ   1000   // 每1ms发生一次时钟中断, 该值必须大于18
#define CLOCK_MODE          0x34    

// 键盘端口
#define PORT_KEYBOARD_DATA  0x60
#define PORT_KEYBOARD_STATE 0x64

//////////////////////////////////////
// 来自汇编的函数声明
void dispChar(char c, u8 color);
u8 inByte(u16 port);
void outByte(u8 data, u16 port);
void hwint00();
void hwint01();
void hwint02();
void hwint03();
void hwint04();
void hwint05();
void hwint06();
void hwint07();
void hwint08();
void hwint09();
void hwint10();
void hwint11();
void hwint12();
void hwint13();
void hwint14();
void hwint15();

void Handler();

///-----------------------------------
void setCursorPos ( );
void addPCB(u32 num, u32 entry, u32 priority);
void buildIdt();
void clockHandler();
void defaultHwintHandler();
void dispInt(u32);
void dispStr(char*);
void init8259a();
void initGate (Gate* p, u16 sel,  u32 offset, u8 attrType, u8 pcount) ;
void initDescriptor(Descriptor * p, u32 base, u32 limit, u8 attrType, u8 attr);
void keyboardHandler();
void schedule();
void delayMs(u32);
void taskTty();
u32 keyboardRead(u32 mask);
void memSet(u8* to, u8 val, u32 size);
void memCpy(u8* to, u8* from, u32 size);

// todo
void processB();
void processC();

#endif