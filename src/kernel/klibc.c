
typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef int   s32;
typedef short s16;
typedef char  s8;

// GDT寄存器 结构
typedef struct _GdtPtr {
    u16 limit;       //GDT界限    
    u32 base;      //GDT基址  
} GdtPtr, IdtPtr;

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
	u16 attr;       // 属性	  
	u16 offset2; 	// 偏移2  
} Gate;


/////////////////////////////////////////////
// 全局变量

// 中断控制器8259A 主片、从片端口号
#define PPRT_8259A_PRIMARY1 0x20
#define PPRT_8259A_PRIMARY2 0x21
#define PPRT_8259A_ATTACH1 0xA0
#define PPRT_8259A_ATTACH2 0xA1

#define GDT_SIZE 128
#define IDT_SIZE 256

Descriptor gdt[GDT_SIZE];
GdtPtr gdtPtr;

Gate idt[IDT_SIZE];
IdtPtr idtPtr;
///-----------------------------------


//////////////////////////////////////
// 来自汇编的函数声明
void dispChar(char c, u8 color, u16 loc);
u8 inByte(u16 port);
void outByte(u8 data, u16 port);
///-----------------------------------

//////////////////////////////////////
// 来自C语言的函数声明
void memCpy(u8* to, u8* from, u32 size);
void init8259a();
///-----------------------------------

void osmain(){
    init8259a();

    dispChar('A', 0x0c, 160*1+2*1);
}

/// 初始化中断控制器8259A
void init8259a(){
    outByte(0x11, PPRT_8259A_PRIMARY1);     // 写ICW1
    outByte(0x11, PPRT_8259A_ATTACH1); 

    outByte(0x20, PPRT_8259A_PRIMARY2);     // 写ICW2 ,主片IRQ0-IRQ7对应中断号
    outByte(0x28, PPRT_8259A_ATTACH2);      // 写ICW2 ,从片IRQ8-IRQ15对应中断号

    outByte(0x04, PPRT_8259A_PRIMARY2);     // 写ICW3
    outByte(0x02, PPRT_8259A_ATTACH2);      // 写ICW3, 从片对应主片的IR号

    outByte(0x01, PPRT_8259A_PRIMARY2);     // 写ICW4
    outByte(0x01, PPRT_8259A_ATTACH2);

    outByte(0x0fe, PPRT_8259A_PRIMARY2);     // 写OCW1, 主片仅打开时钟中断
    outByte(0x0ff, PPRT_8259A_ATTACH2);      // 写OCW1, 从片屏蔽所有中断 
}

/// 建立IDT
void buildIdt(){
    for (int i=0; i<IDT_SIZE; i++) {


    }

    idtPtr.base = (u32)&idt;
    idtPtr.limit = sizeof(idt)-1;
}

/// 将GDT从loader移动到kernel
/// 执行前gdtPtr存放loader中GDT PTR信息
/// 执行后gdtPtr存放kernel中GDT PTR信息
void mvGdt(){
    memCpy((u8*)&gdt, (u8*)gdtPtr.base, gdtPtr.limit+1);
    gdtPtr.base = (u32)&gdt;
    gdtPtr.limit = sizeof(gdt)-1;
}

void memCpy(u8* to, u8* from, u32 size){
    for(u32 i=0;i<size; i++){
        to[i] = from[i];
    }
}


