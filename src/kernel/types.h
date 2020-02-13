
#ifndef TYPES_H
#define TYPES_H

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

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

#define LDT_SIZE 2          // LDT 大小
#define PROCESS_STACK_SIZE 0x8000           // 进程堆栈 大小
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
    Descriptor ldt[LDT_SIZE];
    u32 entry;                          // 进程入口
    u32 priority;                       // 优先级
    int ticks;
    char name[16];                      // 进程名称
    u8 pstack[PROCESS_STACK_SIZE];      // 进程堆栈
} PCB ;

#define KEY_BUF_SIZE 128
typedef struct _KeyBuf {
    int count;
    u8 buf[KEY_BUF_SIZE];
    u8* head;
    u8* tail;
} KeyBuf;

#endif