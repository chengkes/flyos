
#ifndef TYPES_H
#define TYPES_H

#define OUT      // 表示参数会被修改，便于阅读代码
#define NULL		0

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

#endif