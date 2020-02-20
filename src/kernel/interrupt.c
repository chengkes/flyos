#include "types.h"
#include "lib.h"
#include "interrupt.h"
#include "main.h"

// 中断控制器8259A 主片、从片端口号
#define PORT_8259A_MASTER1 0x20
#define PORT_8259A_MASTER2 0x21
#define PORT_8259A_SLAVE1 0xA0
#define PORT_8259A_SLAVE2 0xA1
// 外部中断对应中断号
#define INT_VECTOR_IRQ0    0x20
#define INT_VECTOR_IRQ8    0x28

// IDT 大小
#define IDT_SIZE 256
static Gate idt[IDT_SIZE];
u8 idtPtr[6];

// 硬件中断总个数
#define IRQ_COUNT 16
IrqHandler* hwintHandlerTable[IRQ_COUNT];  // 硬件中断处理程序

// 硬件中断默认处理程序
static void defaultHwintHandler(){}

/// 初始化中断控制器8259A
void init8259a(){
    outByte(0x11, PORT_8259A_MASTER1);     // 写ICW1
    outByte(0x11, PORT_8259A_SLAVE1);

    outByte(INT_VECTOR_IRQ0, PORT_8259A_MASTER2);     // 写ICW2 ,主片IRQ0-IRQ7对应中断号
    outByte(INT_VECTOR_IRQ8, PORT_8259A_SLAVE2);      // 写ICW2 ,从片IRQ8-IRQ15对应中断号

    outByte(0x04, PORT_8259A_MASTER2);     // 写ICW3
    outByte(0x02, PORT_8259A_SLAVE2);      // 写ICW3, 从片对应主片的IR号

    outByte(0x01, PORT_8259A_MASTER2);     // 写ICW4
    outByte(0x01, PORT_8259A_SLAVE2);

    outByte(0x0fc, PORT_8259A_MASTER2);     // 写OCW1, 主片打开时钟中断、键盘终端
    outByte(0x0ff, PORT_8259A_SLAVE2);      // 写OCW1, 从片屏蔽所有中断
}

void putIrqHandler(u8 no, IrqHandler handler){
    hwintHandlerTable[no] = handler;
}

/// 建立IDT
void buildIdt(){
    for (int i=0; i<IDT_SIZE; i++) {
        initGate(&idt[i], GDT_SELECTOR_C32, (u32)Handler, DA_386IGate, 0);
    }

    initGate(&idt[INT_VECTOR_IRQ0+0], GDT_SELECTOR_C32, (u32)hwint00, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ0+1], GDT_SELECTOR_C32, (u32)hwint01, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ0+2], GDT_SELECTOR_C32, (u32)hwint02, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ0+3], GDT_SELECTOR_C32, (u32)hwint03, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ0+4], GDT_SELECTOR_C32, (u32)hwint04, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ0+5], GDT_SELECTOR_C32, (u32)hwint05, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ0+6], GDT_SELECTOR_C32, (u32)hwint06, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ0+7], GDT_SELECTOR_C32, (u32)hwint07, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ8+0], GDT_SELECTOR_C32, (u32)hwint08, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ8+1], GDT_SELECTOR_C32, (u32)hwint09, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ8+2], GDT_SELECTOR_C32, (u32)hwint10, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ8+3], GDT_SELECTOR_C32, (u32)hwint11, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ8+4], GDT_SELECTOR_C32, (u32)hwint12, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ8+5], GDT_SELECTOR_C32, (u32)hwint13, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ8+6], GDT_SELECTOR_C32, (u32)hwint14, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ8+7], GDT_SELECTOR_C32, (u32)hwint15, DA_386IGate, 0);

    for (int i=2 ;i<IRQ_COUNT; i++) {
        hwintHandlerTable[i] = defaultHwintHandler;
    }

    *((u16*)idtPtr) = (u16)(sizeof(idt)-1);
    *((u32*)(idtPtr+2))= (u32)&idt;
}


