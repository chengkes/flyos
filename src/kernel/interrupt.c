#include "types.h"
#include "lib.h"
#include "interrupt.h"
#include "main.h"

// 中断控制器8259A 主片、从片端口号
#define PORT_8259A_MASTER1 0x20
#define PORT_8259A_MASTER2 0x21
#define PORT_8259A_SLAVE1 0xA0
#define PORT_8259A_SLAVE2 0xA1

// IDT 大小
#define IDT_SIZE 0xff
static Gate idt[IDT_SIZE];
u8 idtPtr[6];

// 硬件中断总个数
#define IRQ_COUNT 16
void* hwintHandlerTable[IRQ_COUNT];  // 硬件中断处理程序

#define SYS_CALL_COUNT 16
void* syscallTable[SYS_CALL_COUNT];

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

    outByte(0xF8, PORT_8259A_MASTER2);     // 写OCW1, 主片打开键盘、时钟,级联
    outByte(0x3F, PORT_8259A_SLAVE2);      // 写OCW1, 从片打开硬盘(14,15)
}

void putIrqHandler(u8 no, void* handler){
    assert(no>=0 && no<IRQ_COUNT);
    hwintHandlerTable[no] = handler;
}

void putSyscall(u8 no, void* handler){
    assert(no>=0 && no<SYS_CALL_COUNT);
    syscallTable[no] = handler;
}

/// 建立IDT
void buildIdt(){
    initGate(&idt[INT_VECTOR_DIVIDE_ERROR         ], GDT_SELECTOR_C32, (u32)divide_error          , DA_386IGate, 0);              
    initGate(&idt[INT_VECTOR_SINGLE_STEP_EXCEPTION], GDT_SELECTOR_C32, (u32)single_step_exception , DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_NMI                  ], GDT_SELECTOR_C32, (u32)nmi                   , DA_386IGate, 0);              
    initGate(&idt[INT_VECTOR_BREAKPOINT_EXCEPTION ], GDT_SELECTOR_C32, (u32)breakpoint_exception  , DA_386IGate, 0);      
    initGate(&idt[INT_VECTOR_OVERFLOW             ], GDT_SELECTOR_C32, (u32)overflow              , DA_386IGate, 0);          
    initGate(&idt[INT_VECTOR_BOUNDS_CHECK         ], GDT_SELECTOR_C32, (u32)bounds_check          , DA_386IGate, 0);          
    initGate(&idt[INT_VECTOR_INVAL_OPCODE         ], GDT_SELECTOR_C32, (u32)inval_opcode          , DA_386IGate, 0);                  
    initGate(&idt[INT_VECTOR_COPR_NOT_AVAILABLE   ], GDT_SELECTOR_C32, (u32)copr_not_available    , DA_386IGate, 0);          
    initGate(&idt[INT_VECTOR_DOUBLE_FAULT         ], GDT_SELECTOR_C32, (u32)double_fault          , DA_386IGate, 0);              
    initGate(&idt[INT_VECTOR_COPR_SEG_OVERRUN     ], GDT_SELECTOR_C32, (u32)copr_seg_overrun      , DA_386IGate, 0);      
    initGate(&idt[INT_VECTOR_INVAL_TSS            ], GDT_SELECTOR_C32, (u32)inval_tss             , DA_386IGate, 0);                  
    initGate(&idt[INT_VECTOR_SEGMENT_NOT_PRESENT  ], GDT_SELECTOR_C32, (u32)segment_not_present   , DA_386IGate, 0);      
    initGate(&idt[INT_VECTOR_STACK_EXCEPTION      ], GDT_SELECTOR_C32, (u32)stack_exception       , DA_386IGate, 0);      
    initGate(&idt[INT_VECTOR_GENERAL_PROTECTION   ], GDT_SELECTOR_C32, (u32)general_protection    , DA_386IGate, 0);  
    initGate(&idt[INT_VECTOR_PAGE_FAULT           ], GDT_SELECTOR_C32, (u32)page_fault            , DA_386IGate, 0);                  
    initGate(&idt[INT_VECTOR_COPR_ERROR           ], GDT_SELECTOR_C32, (u32)copr_error            , DA_386IGate, 0);              

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

    initGate(&idt[INT_VECTOR_SYSCALL], GDT_SELECTOR_C32, (u32)int90syscall, DA_386IGate | DA_DPL3, 0);

    *((u16*)idtPtr) = (u16)(sizeof(idt)-1);
    *((u32*)(idtPtr+2))= (u32)&idt;
}

void exceptionHandler(int vec_no, int err_code, int eip, int cs, int eflags) {
    
}

// 获取IRQ状态
int isIrqEnable(int irq) {
    assert(irq>=0 && irq<=15);
    
    u16 port = PORT_8259A_MASTER2;
    if (irq>=8) {
        port = PORT_8259A_SLAVE2;
        irq -= 8;
    }
    irq = 1 << irq;

    return (inByte(port) & irq) == 0 ;
}

//  启用或禁用IRQ 
void setIrq(int irq, int enable) {
    assert(irq>=0 && irq<=15);
    
    u16 port = PORT_8259A_MASTER2;
    if (irq>=8) {
        port = PORT_8259A_SLAVE2;
        irq -= 8;
    }
    irq = 1 << irq;

    // asm volatile("pushf");   // 保存标志寄存器，主要是中断标志
    // asm volatile("cli");
    if (enable) {  // 启用IRQ，令ocw1的irq位=0
        outByte(inByte(port) & ~irq, port); 
    }else{   // 禁用IRQ，令ocw1的irq位=1
        outByte(inByte(port) | irq, port);
    } 
    // asm volatile("popf");   // 恢复标志寄存器，主要是中断标志
}
