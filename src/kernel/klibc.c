
typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef int   s32;
typedef short s16;
typedef char  s8;

// 描述符
typedef struct _Descriptor {
    u16  	limit;	    // 段界限 1
	u16     base1; 		// 段基址 1
	u8	    base2;		// 段基址 2	
	u8      attr1;      // 属性 1 + 段界限 2 
    u8      attr2;      // 属性 2
	u8	    base3;		// 段基址 3	
} Descriptor;

// 门
typedef struct _Gate {
	u16 offset1; 	// 偏移1  
	u16 selector;	// 选择子 
	u8 attr1;       // 属性 	  
    u8 attr2;       // 属性 P(1),DPL(2),S(1),TYPE(4)
	u16 offset2; 	// 偏移2  
} Gate;


/////////////////////////////////////////////
// 全局变量

// 中断控制器8259A 主片、从片端口号
#define PORT_8259A_PRIMARY1 0x20
#define PORT_8259A_PRIMARY2 0x21
#define PORT_8259A_ATTACH1 0xA0
#define PORT_8259A_ATTACH2 0xA1

// 外部中断对应中断号
#define INT_VECTOR_IRQ0    0x20            
#define INT_VECTOR_IRQ8    0x28

// 中断描述符属性
#define DA_386IGATE    0x8E

// GDT中代码段选择符
#define SELECTOR_C32    0x10

// GDT 大小
#define GDT_SIZE 128
// IDT 大小
#define IDT_SIZE 256

Descriptor gdt[GDT_SIZE];
u8 gdtPtr[6];

Gate idt[IDT_SIZE];
u8 idtPtr[6];

u32 dispPos;        // 字符显示位置
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
void initIdtDescriptor(u8 intNum, u32 offset);
void memCpy(u8* to, u8* from, u32 size);

void osinit(){
    dispPos = 0;

    /// 将GDT从loader移动到kernel
    /// 执行前gdtPtr存放loader中GDT PTR信息
    /// 执行后gdtPtr存放kernel中GDT PTR信息
    memCpy((u8*)&gdt,(u8*) (*((u32*)(gdtPtr+2))), *((u16*)gdtPtr)+1);  
    *((u16*)gdtPtr) = (u16)(sizeof(gdt)-1);         // GDT limit
    *((u32*)(gdtPtr+2))= (u32)&gdt;                 // GDT base

    init8259a();
    buildIdt();    

    for (int i=0; i<12; i++)    dispChar('a', 0x0c); // this is for test 
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

    outByte(0x0fc, PORT_8259A_PRIMARY2);     // 写OCW1, 主片仅打开时钟中断
    outByte(0x0ff, PORT_8259A_ATTACH2);      // 写OCW1, 从片屏蔽所有中断 
}

/// 建立IDT
void buildIdt(){
    for (int i=0; i<IDT_SIZE; i++) {
         initIdtDescriptor(i, (u32)Handler);
    }
    initIdtDescriptor(INT_VECTOR_IRQ0, (u32)clockHandler);
    initIdtDescriptor(INT_VECTOR_IRQ0+1, (u32)keyboardHandler); 

    *((u16*)idtPtr) = (u16)(sizeof(idt)-1);
    *((u32*)(idtPtr+2))= (u32)&idt;   
}

// 初始化 中断号为intNum 的中断描述符
void initIdtDescriptor(u8 intNum, u32 offset){
    idt[intNum].selector = SELECTOR_C32;
    idt[intNum].attr1 = 0;
    idt[intNum].attr2 = DA_386IGATE; 
    idt[intNum].offset1 =   offset & 0xffff ;
    idt[intNum].offset2 = (offset>>16) & 0xffff ;
}

void memCpy(u8* to, u8* from, u32 size){
    for(u32 i=0;i<size; i++){
        to[i] = from[i];
    }
}


void processA(){
    char a = 'A';
    while(1) {
        dispChar(a, 0x0c);
        a++;
        dispPos -= 2;
        for(int i=0;i<0x7fffff;i++) for(int j=0;j<0x2;j++);
    }
}

void processB(){
    char a = 'a';
    while(1) {
        dispChar(a, 0x0c);
        a++;
        dispPos -= 2;
        for(int i=0;i<0x7fffff;i++) for(int j=0;j<0x1;j++);
    }
}

