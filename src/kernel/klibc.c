
/////////////////////////////////////////////

// 中断控制器8259A 主片、从片端口号
#define PORT_8259A_PRIMARY1 0x20
#define PORT_8259A_PRIMARY2 0x21
#define PORT_8259A_ATTACH1 0xA0
#define PORT_8259A_ATTACH2 0xA1

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

// LDT 大小
#define LDT_SIZE 3
// GDT 大小
#define GDT_SIZE 128
// IDT 大小
#define IDT_SIZE 256
// 进程控制块 大小
#define PCB_SIZE 2
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

    u16  ldtSel; // 17
    Descriptor ldt[2];
    u32 entry;    
    char name[16];
    u8 pstack[PROCESS_STACK_SIZE];       
} PCB ;


//////////////////////////////////////
Descriptor gdt[GDT_SIZE];
u8 gdtPtr[6];

Gate idt[IDT_SIZE];
u8 idtPtr[6];

u32 dispPos;        // 字符显示位置

TSS tss;
PCB pcbs[PCB_SIZE];
PCB* currentPcb;

u32 isInt ;    // 是否在处理中断程序
///-----------------------------------

//////////////////////////////////////
// 来自汇编的函数声明
void dispChar(char c, u8 color);
u8 inByte(u16 port);
void outByte(u8 data, u16 port);
void clockHandler();
void Handler();
void keyboardHandler();

///-----------------------------------

void init8259a();
void buildIdt();
void initGate (Gate* p, u16 sel,  u32 offset, u8 attrType, u8 pcount) ;
void initDescriptor(Descriptor * p, u32 base, u32 limit, u8 attrType, u8 attr);
void memCpy(u8* to, u8* from, u32 size);
void memSet(u8* to, u8 val, u32 size);
void initPCB(PCB* pcb, u32 entry, u32 ldtSel);
void processA();
void processB();

void osinit(){
    dispPos = 0;
    isInt = 0;
    /// 将GDT从loader移动到kernel
    /// 执行前gdtPtr存放loader中GDT PTR信息
    /// 执行后gdtPtr存放kernel中GDT PTR信息
    memCpy((u8*)&gdt,(u8*) (*((u32*)(gdtPtr+2))), *((u16*)gdtPtr)+1);  
    *((u16*)gdtPtr) = (u16)(sizeof(gdt)-1);         // GDT limit
    *((u32*)(gdtPtr+2))= (u32)&gdt;                 // GDT base

    init8259a();
    buildIdt();    

    memSet((u8*)&tss, 0, sizeof(TSS));
    tss.ss0 = GDT_SELECTOR_D32;
    tss.iobase = sizeof(TSS);
    initDescriptor(&gdt[GDT_SELECTOR_TSS>>3],(u32) &tss, sizeof(TSS)-1, DA_386TSS, 0 );

    initPCB(&pcbs[0], (u32)processA, GDT_SELECTOR_LDT);
    initPCB(&pcbs[1], (u32)processB, GDT_SELECTOR_LDT + sizeof(Descriptor));
    currentPcb = &pcbs[1];

    for (int i=0; i<4; i++)    dispChar('a', 0x0c); // this is for test 
    dispPos += 4;
}

void initPCB(PCB* pcb, u32 entry, u32 ldtSel) {    
    initDescriptor(&pcb->ldt[LDT_SELECTOR_D32 >> 3], 0, 0xfffff, DA_DRW | DA_DPL1, DA_LIMIT_4K | DA_32);
    initDescriptor(&pcb->ldt[LDT_SELECTOR_C32 >> 3], 0, 0xfffff, DA_CR | DA_DPL1, DA_LIMIT_4K | DA_32);
    initDescriptor(&gdt[ldtSel>>3], (u32)(&pcb->ldt), sizeof(Descriptor)*LDT_SIZE - 1, DA_LDT | DA_DPL1, 0);

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
    outByte(0x11, PORT_8259A_PRIMARY1);     // 写ICW1
    outByte(0x11, PORT_8259A_ATTACH1); 

    outByte(INT_VECTOR_IRQ0, PORT_8259A_PRIMARY2);     // 写ICW2 ,主片IRQ0-IRQ7对应中断号
    outByte(INT_VECTOR_IRQ8, PORT_8259A_ATTACH2);      // 写ICW2 ,从片IRQ8-IRQ15对应中断号

    outByte(0x04, PORT_8259A_PRIMARY2);     // 写ICW3
    outByte(0x02, PORT_8259A_ATTACH2);      // 写ICW3, 从片对应主片的IR号

    outByte(0x01, PORT_8259A_PRIMARY2);     // 写ICW4
    outByte(0x01, PORT_8259A_ATTACH2);

    outByte(0x0fe, PORT_8259A_PRIMARY2);     // 写OCW1, 主片仅打开时钟中断
    outByte(0x0ff, PORT_8259A_ATTACH2);      // 写OCW1, 从片屏蔽所有中断 
}

/// 建立IDT
void buildIdt(){
    for (int i=0; i<IDT_SIZE; i++) {
        initGate(&idt[i], GDT_SELECTOR_C32, (u32)Handler, DA_386IGate, 0);
    }
    initGate(&idt[INT_VECTOR_IRQ0], GDT_SELECTOR_C32, (u32)clockHandler, DA_386IGate, 0);
    initGate(&idt[INT_VECTOR_IRQ0+1], GDT_SELECTOR_C32, (u32)keyboardHandler, DA_386IGate, 0);

    *((u16*)idtPtr) = (u16)(sizeof(idt)-1);
    *((u32*)(idtPtr+2))= (u32)&idt;   
}


// 初始化描述符
void initDescriptor(Descriptor * p, u32 base, u32 limit, u8 attrType, u8 attr){
    p->base1 = 0xffff & base;
    p->base2 = 0xff & (base>>16);
    p->base3 = 0xff & (base>>24);
    p->limit = 0xffff & limit;   
    p->attrType = attrType;
    p->attr2 = (0x0f &(limit>>16) ) | (0xf0 & attr); 
}

// 初始化 门描述符
void initGate (Gate* p, u16 sel,  u32 offset, u8 attrType, u8 pcount) {
    p->selector = sel;
    p->offset1 = offset & 0xffff ;
    p->offset2 = (offset>>16) & 0xffff ;
    p->attrType = attrType;
    p->pcount = pcount;
}

void memCpy(u8* to, u8* from, u32 size){
    for(u32 i=0;i<size; i++) to[i] = from[i];
}

void memSet(u8* to, u8 val, u32 size){
    for(int i=0;i<size; i++) to[i] = val;
}

void processA(){
    char a = 'A';
    while(1) {
        // dispPos = 160*1;
        dispChar(a, 0x0c);
        a++;
        if (a >  'Z') a = 'A';
        for(int i=0;i<0x7fff;i++) for(int j=0;j<0x1;j++);
    }
}

void processB(){
    char a = 'a';
    while(1) {
        // dispPos = 160*2;
        dispChar(a, 0x0c);
        a++;
        if (a >  'z') a = 'a';
        for(int i=0;i<0x7fff;i++) for(int j=0;j<0x1;j++);
    }
}

void showMsg(){
    if ( dispPos > 160*20) dispPos = 160;
    if(isInt != 0) {   // 发生中断重入，内核运行时发生的中断，此时 esp 指向内核堆栈，不能切换进程
        dispChar('=', 0x0c);
        return;
    }

    // ; 没有中断重入，进程运行时发生的中断，可以进行进程切换
    // 0x141cb   0x141ce
    for(int i=0;i<0x141ce;i++) for(int j=0;j<0x1;j++);
    dispChar('~', 0x0c);
    currentPcb++;
    if (currentPcb >= pcbs + PCB_SIZE)  currentPcb=pcbs;
}