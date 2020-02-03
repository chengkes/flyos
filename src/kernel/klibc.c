
/////////////////////////////////////////////

// 中断控制器8259A 主片、从片端口号
#define PORT_8259A_MASTER1 0x20
#define PORT_8259A_MASTER2 0x21
#define PORT_8259A_SLAVE1 0xA0
#define PORT_8259A_SLAVE2 0xA1
#define PORT_KEYBOARD_DATA 0x60

// 外部中断对应中断号
#define INT_VECTOR_IRQ0    0x20            
#define INT_VECTOR_IRQ8    0x28

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

// LDT中 选择子
#define LDT_SELECTOR_D32        (0x00 | SA_TIL | SA_RPL1)   // 数据段 选择子
#define LDT_SELECTOR_C32        (0x08 | SA_TIL | SA_RPL1)   // 代码段 选择子

// GDT中 选择子
#define GDT_SELECTOR_D32        0x08    // 数据段 选择子
#define GDT_SELECTOR_C32        0x10    // 代码段 选择子
#define GDT_SELECTOR_VIDEO      (0x18 | SA_RPL3)    // 视频段 选择子
#define GDT_SELECTOR_TSS        0x20    // TSS 选择子
#define GDT_SELECTOR_LDT        0x28    // LDT 选择子

// 硬件中断总个数
#define IRQ_COUNT 16

// LDT 大小
#define LDT_SIZE 3
// GDT 大小
#define GDT_SIZE 128
// IDT 大小
#define IDT_SIZE 256
// 进程控制块 大小
#define PCB_SIZE 3
// 进程堆栈 大小
#define PROCESS_STACK_SIZE 0x8000

//////////////////////////////////////

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef int   s32;
typedef short s16;
typedef char  s8;

// 描述符
typedef struct _Descriptor {
    u16  	limit;	    // 段界限1
	u16     base1; 		// 段基址1
	u8	    base2;		// 段基址2	
	u8      attrType;   // P, DPL(2), S, TYPE(4)
    u8      attr2;      // G, D/B, 0, AVL, 段界限2(4)
	u8	    base3;		// 段基址3	
} Descriptor;

// 门
typedef struct _Gate {
	u16 offset1; 	// 偏移1  
	u16 selector;	// 选择子 
	u8 pcount;      // 属性, 参数个数	  
    u8 attrType;    // 属性 P(1),DPL(2),S(1),TYPE(4)
	u16 offset2; 	// 偏移2  
} Gate;

typedef struct _TSS {
	u32	backlink;
	u32	esp0;		/* stack pointer to use during interrupt */
	u32	ss0;		/*   "   segment  "  "    "        "     */
	u32	esp1;
	u32	ss1;
	u32	esp2;
	u32	ss2;
	u32	cr3;
	u32	eip;
	u32	flags;
	u32	eax;
	u32	ecx;
	u32	edx;
	u32	ebx;
	u32	esp;
	u32	ebp;
	u32	esi;
	u32	edi;
	u32	es;
	u32	cs;
	u32	ss;
	u32	ds;
	u32	fs;
	u32	gs;
	u32	ldt;
	u16	trap;
	u16	iobase;	// I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图  
	// u8	iomap[2]; 
}TSS;    

// 进程控制块
typedef struct _PCB {
    u32 edi;  
    u32 esi;
    u32 ebp;
    u32 esp;  
    u32 ebx; 
    u32 edx;
    u32 ecx;
    u32 eax;

    u32 ds;
    u32 es;
    u32 fs;
    u32 gs;   

    u32 eip;   
    u32 cs;   
    u32 eflags;
    u32 p_esp;    
    u32 ss;

    u16  ldtSel;                        //第17个，此处至开头数据的位置需要保持不变
    Descriptor ldt[2];  
    u32 entry;                          // 进程入口
    u32 priority;                       // 优先级
    u32 ticks;  
    char name[16];                      // 进程名称
    u8 pstack[PROCESS_STACK_SIZE];      // 进程堆栈
} PCB ;


//////////////////////////////////////
Descriptor gdt[GDT_SIZE];
u8 gdtPtr[6];

Gate idt[IDT_SIZE];
u8 idtPtr[6];

u32 dispPos;        // 字符显示位置
u8 dispColor;       // 字符显示颜色

TSS tss;
PCB pcbs[PCB_SIZE];     // 所有进程
PCB* currentPcb;        // 当前运行的进程

u32 isInt ;     // 是否在处理中断程序
u32 ticks;      // 时钟中断发生次数
u32 hwintHandlerTable[IRQ_COUNT];  // 硬件中断处理程序
///-----------------------------------

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
void addPCB(u32 num, u32 entry, u32 priority);
void buildIdt();
void clockHandler();
void defaultHwintHandler();
void dispInt(u32);
void dispStr(char*);
void init8259a();
void itos(u32 a, char* p);
void initGate (Gate* p, u16 sel,  u32 offset, u8 attrType, u8 pcount) ;
void initDescriptor(Descriptor * p, u32 base, u32 limit, u8 attrType, u8 attr);
void keyboardHandler();
void memCpy(u8* to, u8* from, u32 size);
void memSet(u8* to, u8 val, u32 size);
void schedule();

// todo
void processA();
void processB();
void processC();

void osinit(){
    dispPos = 0;
    dispColor = 0x0c;
    isInt = 0;
    ticks = 0;

    //将GDT从loader移动到kernel,执行前gdtPtr存放loader中GDT PTR信息,执行后gdtPtr存放kernel中GDT PTR信息
    memCpy((u8*)&gdt,(u8*) (*((u32*)(gdtPtr+2))), *((u16*)gdtPtr)+1);  
    *((u16*)gdtPtr) = (u16)(sizeof(gdt)-1);         // GDT limit
    *((u32*)(gdtPtr+2))= (u32)&gdt;                 // GDT base

    init8259a();
    buildIdt();    

    // 初始化TSS 及 TSS描述符
    memSet((u8*)&tss, 0, sizeof(TSS));
    tss.ss0 = GDT_SELECTOR_D32;
    tss.iobase = sizeof(TSS);
    initDescriptor(&gdt[GDT_SELECTOR_TSS>>3],(u32) &tss, sizeof(TSS)-1, DA_386TSS, 0 );

    // 添加进程
    addPCB(0, (u32)processA, 1);
    addPCB(1, (u32)processB, 5);
    addPCB(2, (u32)processC, 0);
    currentPcb = &pcbs[0];

    dispStr("\n\n  Welcome to Fly OS! \n\n");
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

    outByte(0x0fd, PORT_8259A_MASTER2);     // 写OCW1, 主片仅打开时钟中断
    outByte(0x0ff, PORT_8259A_SLAVE2);      // 写OCW1, 从片屏蔽所有中断 
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

    hwintHandlerTable[0] = (u32)clockHandler;
    hwintHandlerTable[1] = (u32)keyboardHandler;
    for (int i=2 ;i<IRQ_COUNT; i++) {
        hwintHandlerTable[i] = (u32)defaultHwintHandler;
    }

    *((u16*)idtPtr) = (u16)(sizeof(idt)-1);
    *((u32*)(idtPtr+2))= (u32)&idt;   
}

 // 硬件中断默认处理程序
 void defaultHwintHandler(){}

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

// 键盘中断处理程序， todo
void keyboardHandler(){  
    dispChar('!', dispColor);
    u8 k =inByte(PORT_KEYBOARD_DATA);
    dispInt(k);
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

void memCpy(u8* to, u8* from, u32 size){
    for(u32 i=0;i<size; i++) to[i] = from[i];
}

void memSet(u8* to, u8 val, u32 size){
    for(int i=0;i<size; i++) to[i] = val;
}

// 整数转换为16进制字符串
void itos(u32 a, char* p){
    for (int i=0;i<8; i++) {
        char w = (a>>(28-i*4)) & 0x0f;
        if (w>=10) {
            *(p+i) = w +'A' - 10;
        }else {
            *(p+i) = w +'0';
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

// 测试进程AQ1
void processA(){
    while(1) {
        if (dispPos > 160*24) dispPos = 160;
        // dispInt(ticks);
        // dispChar('-', dispColor);
        for(int i=0;i<0x7fff;i++) for(int j=0;j<0x1;j++);
    }
}

// 测试进程B
void processB(){
    char a = 'a';
    while(1) {
        dispChar(a++, 0x0f);
        if (a >  'z') a = 'a';
        for(int i=0;i<0x7fff;i++) for(int j=0;j<0x1;j++);
    }
}

// 测试进程C
void processC(){
    char a = '0';
    while(1) {
        dispChar(a++, 0x01);
        if (a >  '9') a = '0';
        for(int i=0;i<0x7fff;i++) for(int j=0;j<0x1;j++);
    }
}
