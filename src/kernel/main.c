#include "main.h"
#include "types.h"
#include "keyboard.h"

static KeyBuf keyBuf;

#define PCB_SIZE 3              // 进程控制块 大小
static PCB pcbs[PCB_SIZE];     // 所有进程
PCB* currentPcb;        // 当前运行的进程

// GDT 大小
#define GDT_SIZE 128
static Descriptor gdt[GDT_SIZE];
u8 gdtPtr[6];

// IDT 大小
#define IDT_SIZE 256
static Gate idt[IDT_SIZE];
u8 idtPtr[6];

u32 dispPos;        // 字符显示位置
static u8 dispColor;       // 字符显示颜色

TSS tss;

// 硬件中断总个数
#define IRQ_COUNT 16
u32 hwintHandlerTable[IRQ_COUNT];  // 硬件中断处理程序

static u32 ticks;      // 时钟中断发生次数
u32 isInt ;     // 是否在处理中断程序

void osinit(){
    dispPos = 0;
    dispColor = 0x0c;
    isInt = 0;
    ticks = 0;

    keyBuf.count = 0;
    keyBuf.tail = keyBuf.head = keyBuf.buf;

    //将GDT从loader移动到kernel,执行前gdtPtr存放loader中GDT PTR信息,执行后gdtPtr存放kernel中GDT PTR信息
    memCpy((u8*)&gdt,(u8*) (*((u32*)(gdtPtr+2))), *((u16*)gdtPtr)+1);
    *((u16*)gdtPtr) = (u16)(sizeof(gdt)-1);         // GDT limit
    *((u32*)(gdtPtr+2))= (u32)&gdt;                 // GDT base

    init8259a();
    buildIdt();

    // 初始化时钟中断频率
    outByte(CLOCK_MODE, PORT_CLOCK_CONTROL);
    outByte((CLOCK_DEFAULT_HZ/CLOCK_COUNTER0_HZ) & 0xff, PORT_CLOCK_COUNTER0);             // 先写低位
    outByte(((CLOCK_DEFAULT_HZ/CLOCK_COUNTER0_HZ)>>8) & 0xff, PORT_CLOCK_COUNTER0);        // 再写高位

    // 初始化TSS 及 TSS描述符
    memSet((u8*)&tss, 0, sizeof(TSS));
    tss.ss0 = GDT_SELECTOR_D32;
    tss.iobase = sizeof(TSS);
    initDescriptor(&gdt[GDT_SELECTOR_TSS>>3],(u32) &tss, sizeof(TSS)-1, DA_386TSS, 0 );

    // 添加进程
    addPCB(0, (u32)taskTty, 100);
    addPCB(1, (u32)processB, 500);
    addPCB(2, (u32)processC, 200);
    currentPcb = &pcbs[0];

    dispStr("\n\n  Welcome to Fly OS! \n\n");
}

#pragma region  // 保护模式


// 初始化 段 描述符
void initDescriptor(Descriptor * p, u32 base, u32 limit, u8 attrType, u8 attr){
    p->base1 = 0xffff & base;
    p->base2 = 0xff & (base>>16);
    p->base3 = 0xff & (base>>24);
    p->limit = 0xffff & limit;
    p->attrType = attrType;
    p->attr2 = (0x0f &(limit>>16) ) | (0xf0 & attr);
}

// 初始化 门 描述符
void initGate (Gate* p, u16 sel,  u32 offset, u8 attrType, u8 pcount) {
    p->selector = sel;
    p->offset1 = offset & 0xffff ;
    p->offset2 = (offset>>16) & 0xffff ;
    p->attrType = attrType;
    p->pcount = pcount;
}

#pragma endregion 

#pragma region  // 中断

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


// 硬件中断默认处理程序
void defaultHwintHandler(){}

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

    hwintHandlerTable[0] = (u32)clockHandler;
    hwintHandlerTable[1] = (u32)keyboardHandler;
    for (int i=2 ;i<IRQ_COUNT; i++) {
        hwintHandlerTable[i] = (u32)defaultHwintHandler;
    }

    *((u16*)idtPtr) = (u16)(sizeof(idt)-1);
    *((u32*)(idtPtr+2))= (u32)&idt;
}


#pragma endregion

#pragma region  TTY
////////////////  TTY    ///////////////////////////////////

void setCursorPos (){
    u32 pos = dispPos/ 2 ;
    outByte(CRTC_CURSOR_LOC_L, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos) & 0xff, PORT_DISPLAY_CRTC_DATA);
    outByte(CRTC_CURSOR_LOC_H, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos>>8) & 0xff, PORT_DISPLAY_CRTC_DATA);
}

void scrollTo(u32 pos){
    outByte(CRTC_START_ADDR_L, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos) & 0xff, PORT_DISPLAY_CRTC_DATA);
    outByte(CRTC_START_ADDR_H, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos>>8) & 0xff, PORT_DISPLAY_CRTC_DATA);
}

void taskTty(){
    while(1) {
        u32 key =keyboardRead(0);
        if (!(key& KEYBOARD_FLAG_EXT)) {  // 是否为可打印字符
            dispChar(key & 0x7f, 0x01);
        } else {
            if ( key == DOWN) {
                scrollTo(80*15);
            }else if (key == UP) {
                scrollTo(80*0);
            }
            dispColor = 0x04;
            dispInt(key);
        }
        dispChar(' ', 0x01);
    }
}

#pragma endregion
//////////////////////////////////////////////////////

#pragma region  进程管理  

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

// 测试进程B
void processB(){
    char a = 'a';
    while(1) {
        // dispChar(a++, 0x0f);
        if (a >  'z') a = 'a';
    }
}

// 测试进程C
void processC(){
    char a = '0';
    while(1) {
        // dispChar(a++, 0x01);
        if (a >  '9') a = '0';
    }
}

#pragma endregion

// 键盘中断处理程序
void keyboardHandler(){
    u8 scan_code =inByte(PORT_KEYBOARD_DATA);

    if (keyBuf.count < KEY_BUF_SIZE) {
        *(keyBuf.head++) = scan_code;
        if (keyBuf.head >= keyBuf.buf + KEY_BUF_SIZE) {
            keyBuf.head = keyBuf.buf;
        }
        keyBuf.count++;
    }
}

u8 getByteFromKeybuf(){
    while (keyBuf.count <=0);

    u8 scanCode = *(keyBuf.tail ++);
    // dispColor = 0X02; dispInt(scanCode); // todo
    keyBuf.count --;
    if (keyBuf.tail >= keyBuf.buf + KEY_BUF_SIZE) {
        keyBuf.tail = keyBuf.buf;
    }
    return scanCode;
}

// 时钟中断处理程序
void clockHandler(){
    ticks++;
    (currentPcb->ticks)--;
    if(isInt != 0) {   // 发生中断重入，内核运行时发生的中断，此时 esp 指向内核堆栈，不能切换进程
        return;
    }

    // 没有中断重入，进程运行时发生的中断，可以进行进程切换
    schedule();
}

// 延迟t毫秒 
void delayMs(u32 t) {
    u32 t1 = ticks;
    while ((ticks - t1 )*1000/CLOCK_COUNTER0_HZ <= t);
}

void memCpy(u8* to, u8* from, u32 size){
    for(u32 i=0;i<size; i++) to[i] = from[i];
}

void memSet(u8* to, u8 val, u32 size){
    for(int i=0;i<size; i++) to[i] = val;
}

// 整数转换为16进制字符串
void itos(u32 a, char* p){
    u8 preZero = 1;
    for (int i=0;i<8; i++) {
        char w = (a>>(28-i*4)) & 0x0f;
        if (w>=10) {
            *(p++) = w +'A' - 10;
            preZero = 0;
        } else if(w==0 && preZero){
            continue;
        }else {
            *(p++) = w +'0';
            preZero = 0;
        }
    }
}

// 显示字符串
void dispStr(char* p){
    while(*p != 0) {
        if (*p == '\n'){
            dispPos += 160 - dispPos % 160;
        }
        else {
            dispChar(*p, dispColor);
        }
        p++;
    }
}

// 显示整数
void dispInt(u32 a){
    char b[9]="";
    itos(a, b);
    dispStr(b);
}
