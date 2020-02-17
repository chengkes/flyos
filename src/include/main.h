
#ifndef MAIN_H
#define MAIN_H

#include "types.h"

// -----  PROTECT  --------------------

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

#define GDT_SIZE 128
extern Descriptor gdt[GDT_SIZE];

void initProtectMode() ;
void initGate (Gate* p, u16 sel,  u32 offset, u8 attrType, u8 pcount) ;
void initDescriptor(Descriptor * p, u32 base, u32 limit, u8 attrType, u8 attr);

// ----- 来自汇编的函数声明 --------------------
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

// ----- CLOCK --------------------
void initClock();
void delayMs(u32);
// ----------------------------

// ----- PCB --------------------
void initPcb();
void schedule();
PCB* getCurrentPcb();
void addPCB(u32 num, u32 entry, u32 priority);
// ----------------------------

// ----- INTERRUPT --------------------
#define CLOCK_HANDLER_IDX  0
#define KEYBOARD_HANDLER_IDX  1

void putIrqHandler(u8 no, IrqHandler handler);
void init8259a();
void buildIdt();
// ----------------------------

// -----  LIB  --------------------
void itos(u32 a, char* p);
void memSet(u8* to, u8 val, u32 size);
void memCpy(u8* to, u8* from, u32 size);
// ----------------------------

#endif