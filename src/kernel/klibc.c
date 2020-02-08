
/////////////////////////////////////////////

// 中断控制器8259A 主片、从片端口号
#define PORT_8259A_MASTER1 0x20
#define PORT_8259A_MASTER2 0x21
#define PORT_8259A_SLAVE1 0xA0
#define PORT_8259A_SLAVE2 0xA1
// 外部中断对应中断号
#define INT_VECTOR_IRQ0    0x20            
#define INT_VECTOR_IRQ8    0x28
// 时钟8253芯片端口
#define PORT_CLOCK_COUNTER0  0x40
#define PORT_CLOCK_COUNTER1  0x41
#define PORT_CLOCK_COUNTER2  0x42
#define PORT_CLOCK_CONTROL   0x43

#define CLOCK_DEFAULT_HZ    1193180
#define CLOCK_COUNTER0_HZ   1000   // 每1ms发生一次时钟中断, 该值必须大于18
#define CLOCK_MODE          0x34

// 键盘端口
#define PORT_KEYBOARD_DATA  0x60
#define PORT_KEYBOARD_STATE 0x64

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

KeyBuf keyBuf;

/* Keymap for US MF-2 keyboard. */
#define MAP_COLS	3	/* Number of columns in keymap */
#define NR_SCAN_CODES	0x80	/* Number of scan codes (rows in keymap) */

#define KEYBOARD_FLAG_BREAK	    NR_SCAN_CODES		/* Break Code			*/
#define KEYBOARD_FLAG_EXT	    0x0100		/* Normal function keys		*/
#define KEYBOARD_FLAG_SHIFT_L	0x0200		/* Shift key			*/
#define KEYBOARD_FLAG_SHIFT_R	0x0400		/* Shift key			*/
#define KEYBOARD_FLAG_CTRL_L	0x0800		/* Control key			*/
#define KEYBOARD_FLAG_CTRL_R	0x1000		/* Control key			*/
#define KEYBOARD_FLAG_ALT_L	    0x2000		/* Alternate key		*/
#define KEYBOARD_FLAG_ALT_R	    0x4000		/* Alternate key		*/
#define FLAG_PAD	            0x8000		/* keys in num pad		*/

/* Special keys */
#define ESC		    (0x01 + KEYBOARD_FLAG_EXT)	/* Esc		*/
#define TAB		    (0x02 + KEYBOARD_FLAG_EXT)	/* Tab		*/
#define ENTER		(0x03 + KEYBOARD_FLAG_EXT)	/* Enter	*/
#define BACKSPACE	(0x04 + KEYBOARD_FLAG_EXT)	/* BackSpace	*/

#define GUI_L		(0x05 + KEYBOARD_FLAG_EXT)	/* L GUI	*/
#define GUI_R		(0x06 + KEYBOARD_FLAG_EXT)	/* R GUI	*/
#define APPS		(0x07 + KEYBOARD_FLAG_EXT)	/* APPS	*/

/* Shift, Ctrl, Alt */
#define SHIFT_L		(0x08 + KEYBOARD_FLAG_EXT)	/* L Shift	*/
#define SHIFT_R		(0x09 + KEYBOARD_FLAG_EXT)	/* R Shift	*/
#define CTRL_L		(0x0A + KEYBOARD_FLAG_EXT)	/* L Ctrl	*/
#define CTRL_R		(0x0B + KEYBOARD_FLAG_EXT)	/* R Ctrl	*/
#define ALT_L		(0x0C + KEYBOARD_FLAG_EXT)	/* L Alt	*/
#define ALT_R		(0x0D + KEYBOARD_FLAG_EXT)	/* R Alt	*/

/* Lock keys */
#define CAPS_LOCK	(0x0E + KEYBOARD_FLAG_EXT)	/* Caps Lock	*/
#define	NUM_LOCK	(0x0F + KEYBOARD_FLAG_EXT)	/* Number Lock	*/
#define SCROLL_LOCK	(0x10 + KEYBOARD_FLAG_EXT)	/* Scroll Lock	*/

/* Function keys */
#define F1		(0x11 + KEYBOARD_FLAG_EXT)	/* F1		*/
#define F2		(0x12 + KEYBOARD_FLAG_EXT)	/* F2		*/
#define F3		(0x13 + KEYBOARD_FLAG_EXT)	/* F3		*/
#define F4		(0x14 + KEYBOARD_FLAG_EXT)	/* F4		*/
#define F5		(0x15 + KEYBOARD_FLAG_EXT)	/* F5		*/
#define F6		(0x16 + KEYBOARD_FLAG_EXT)	/* F6		*/
#define F7		(0x17 + KEYBOARD_FLAG_EXT)	/* F7		*/
#define F8		(0x18 + KEYBOARD_FLAG_EXT)	/* F8		*/
#define F9		(0x19 + KEYBOARD_FLAG_EXT)	/* F9		*/
#define F10		(0x1A + KEYBOARD_FLAG_EXT)	/* F10		*/
#define F11		(0x1B + KEYBOARD_FLAG_EXT)	/* F11		*/
#define F12		(0x1C + KEYBOARD_FLAG_EXT)	/* F12		*/

/* Control Pad */
#define PRINTSCREEN	(0x1D + KEYBOARD_FLAG_EXT)	/* Print Screen	*/
#define PAUSEBREAK	(0x1E + KEYBOARD_FLAG_EXT)	/* Pause/Break	*/
#define INSERT		(0x1F + KEYBOARD_FLAG_EXT)	/* Insert	*/
#define DELETE		(0x20 + KEYBOARD_FLAG_EXT)	/* Delete	*/
#define HOME		(0x21 + KEYBOARD_FLAG_EXT)	/* Home		*/
#define END		    (0x22 + KEYBOARD_FLAG_EXT)	/* End		*/
#define PAGEUP		(0x23 + KEYBOARD_FLAG_EXT)	/* Page Up	*/
#define PAGEDOWN	(0x24 + KEYBOARD_FLAG_EXT)	/* Page Down	*/
#define UP		    (0x25 + KEYBOARD_FLAG_EXT)	/* Up		*/
#define DOWN		(0x26 + KEYBOARD_FLAG_EXT)	/* Down		*/
#define LEFT		(0x27 + KEYBOARD_FLAG_EXT)	/* Left		*/
#define RIGHT		(0x28 + KEYBOARD_FLAG_EXT)	/* Right	*/

/* ACPI keys */
#define POWER		(0x29 + KEYBOARD_FLAG_EXT)	/* Power	*/
#define SLEEP		(0x2A + KEYBOARD_FLAG_EXT)	/* Sleep	*/
#define WAKE		(0x2B + KEYBOARD_FLAG_EXT)	/* Wake Up	*/

/* Num Pad */
#define PAD_SLASH	(0x2C + KEYBOARD_FLAG_EXT)	/* /		*/
#define PAD_STAR	(0x2D + KEYBOARD_FLAG_EXT)	/* *		*/
#define PAD_MINUS	(0x2E + KEYBOARD_FLAG_EXT)	/* -		*/
#define PAD_PLUS	(0x2F + KEYBOARD_FLAG_EXT)	/* +		*/
#define PAD_ENTER	(0x30 + KEYBOARD_FLAG_EXT)	/* Enter	*/
#define PAD_DOT		(0x31 + KEYBOARD_FLAG_EXT)	/* .		*/
#define PAD_0		(0x32 + KEYBOARD_FLAG_EXT)	/* 0		*/
#define PAD_1		(0x33 + KEYBOARD_FLAG_EXT)	/* 1		*/
#define PAD_2		(0x34 + KEYBOARD_FLAG_EXT)	/* 2		*/
#define PAD_3		(0x35 + KEYBOARD_FLAG_EXT)	/* 3		*/
#define PAD_4		(0x36 + KEYBOARD_FLAG_EXT)	/* 4		*/
#define PAD_5		(0x37 + KEYBOARD_FLAG_EXT)	/* 5		*/
#define PAD_6		(0x38 + KEYBOARD_FLAG_EXT)	/* 6		*/
#define PAD_7		(0x39 + KEYBOARD_FLAG_EXT)	/* 7		*/
#define PAD_8		(0x3A + KEYBOARD_FLAG_EXT)	/* 8		*/
#define PAD_9		(0x3B + KEYBOARD_FLAG_EXT)	/* 9		*/
#define PAD_UP		PAD_8			/* Up		*/
#define PAD_DOWN	PAD_2			/* Down		*/
#define PAD_LEFT	PAD_4			/* Left		*/
#define PAD_RIGHT	PAD_6			/* Right	*/
#define PAD_HOME	PAD_7			/* Home		*/
#define PAD_END		PAD_1			/* End		*/
#define PAD_PAGEUP	PAD_9			/* Page Up	*/
#define PAD_PAGEDOWN	PAD_3			/* Page Down	*/
#define PAD_INS		PAD_0			/* Ins		*/
#define PAD_MID		PAD_5			/* Middle key	*/
#define PAD_DEL		PAD_DOT			/* Del		*/

u32 keymap[NR_SCAN_CODES * MAP_COLS] = {

/* scan-code			!Shift		Shift		E0 XX	*/
/* ==================================================================== */
/* 0x00 - none		*/	0,		0,		0,
/* 0x01 - ESC		*/	ESC,		ESC,		0,
/* 0x02 - '1'		*/	'1',		'!',		0,
/* 0x03 - '2'		*/	'2',		'@',		0,
/* 0x04 - '3'		*/	'3',		'#',		0,
/* 0x05 - '4'		*/	'4',		'$',		0,
/* 0x06 - '5'		*/	'5',		'%',		0,
/* 0x07 - '6'		*/	'6',		'^',		0,
/* 0x08 - '7'		*/	'7',		'&',		0,
/* 0x09 - '8'		*/	'8',		'*',		0,
/* 0x0A - '9'		*/	'9',		'(',		0,
/* 0x0B - '0'		*/	'0',		')',		0,
/* 0x0C - '-'		*/	'-',		'_',		0,
/* 0x0D - '='		*/	'=',		'+',		0,
/* 0x0E - BS		*/	BACKSPACE,	BACKSPACE,	0,
/* 0x0F - TAB		*/	TAB,		TAB,		0,
/* 0x10 - 'q'		*/	'q',		'Q',		0,
/* 0x11 - 'w'		*/	'w',		'W',		0,
/* 0x12 - 'e'		*/	'e',		'E',		0,
/* 0x13 - 'r'		*/	'r',		'R',		0,
/* 0x14 - 't'		*/	't',		'T',		0,
/* 0x15 - 'y'		*/	'y',		'Y',		0,
/* 0x16 - 'u'		*/	'u',		'U',		0,
/* 0x17 - 'i'		*/	'i',		'I',		0,
/* 0x18 - 'o'		*/	'o',		'O',		0,
/* 0x19 - 'p'		*/	'p',		'P',		0,
/* 0x1A - '['		*/	'[',		'{',		0,
/* 0x1B - ']'		*/	']',		'}',		0,
/* 0x1C - CR/LF		*/	ENTER,		ENTER,		PAD_ENTER,
/* 0x1D - l. Ctrl	*/	CTRL_L,		CTRL_L,		CTRL_R,
/* 0x1E - 'a'		*/	'a',		'A',		0,
/* 0x1F - 's'		*/	's',		'S',		0,
/* 0x20 - 'd'		*/	'd',		'D',		0,
/* 0x21 - 'f'		*/	'f',		'F',		0,
/* 0x22 - 'g'		*/	'g',		'G',		0,
/* 0x23 - 'h'		*/	'h',		'H',		0,
/* 0x24 - 'j'		*/	'j',		'J',		0,
/* 0x25 - 'k'		*/	'k',		'K',		0,
/* 0x26 - 'l'		*/	'l',		'L',		0,
/* 0x27 - ';'		*/	';',		':',		0,
/* 0x28 - '\''		*/	'\'',		'"',		0,
/* 0x29 - '`'		*/	'`',		'~',		0,
/* 0x2A - l. SHIFT	*/	SHIFT_L,	SHIFT_L,	0,
/* 0x2B - '\'		*/	'\\',		'|',		0,
/* 0x2C - 'z'		*/	'z',		'Z',		0,
/* 0x2D - 'x'		*/	'x',		'X',		0,
/* 0x2E - 'c'		*/	'c',		'C',		0,
/* 0x2F - 'v'		*/	'v',		'V',		0,
/* 0x30 - 'b'		*/	'b',		'B',		0,
/* 0x31 - 'n'		*/	'n',		'N',		0,
/* 0x32 - 'm'		*/	'm',		'M',		0,
/* 0x33 - ','		*/	',',		'<',		0,
/* 0x34 - '.'		*/	'.',		'>',		0,
/* 0x35 - '/'		*/	'/',		'?',		PAD_SLASH,
/* 0x36 - r. SHIFT	*/	SHIFT_R,	SHIFT_R,	0,
/* 0x37 - '*'		*/	'*',		'*',    	0,
/* 0x38 - ALT		*/	ALT_L,		ALT_L,  	ALT_R,
/* 0x39 - ' '		*/	' ',		' ',		0,
/* 0x3A - CapsLock	*/	CAPS_LOCK,	CAPS_LOCK,	0,
/* 0x3B - F1		*/	F1,		F1,		0,
/* 0x3C - F2		*/	F2,		F2,		0,
/* 0x3D - F3		*/	F3,		F3,		0,
/* 0x3E - F4		*/	F4,		F4,		0,
/* 0x3F - F5		*/	F5,		F5,		0,
/* 0x40 - F6		*/	F6,		F6,		0,
/* 0x41 - F7		*/	F7,		F7,		0,
/* 0x42 - F8		*/	F8,		F8,		0,
/* 0x43 - F9		*/	F9,		F9,		0,
/* 0x44 - F10		*/	F10,		F10,		0,
/* 0x45 - NumLock	*/	NUM_LOCK,	NUM_LOCK,	0,
/* 0x46 - ScrLock	*/	SCROLL_LOCK,	SCROLL_LOCK,	0,
/* 0x47 - Home		*/	PAD_HOME,	'7',		HOME,
/* 0x48 - CurUp		*/	PAD_UP,		'8',		UP,
/* 0x49 - PgUp		*/	PAD_PAGEUP,	'9',		PAGEUP,
/* 0x4A - '-'		*/	PAD_MINUS,	'-',		0,
/* 0x4B - Left		*/	PAD_LEFT,	'4',		LEFT,
/* 0x4C - MID		*/	PAD_MID,	'5',		0,
/* 0x4D - Right		*/	PAD_RIGHT,	'6',		RIGHT,
/* 0x4E - '+'		*/	PAD_PLUS,	'+',		0,
/* 0x4F - End		*/	PAD_END,	'1',		END,
/* 0x50 - Down		*/	PAD_DOWN,	'2',		DOWN,
/* 0x51 - PgDown	*/	PAD_PAGEDOWN,	'3',		PAGEDOWN,
/* 0x52 - Insert	*/	PAD_INS,	'0',		INSERT,
/* 0x53 - Delete	*/	PAD_DOT,	'.',		DELETE,
/* 0x54 - Enter		*/	0,		0,		0,
/* 0x55 - ???		*/	0,		0,		0,
/* 0x56 - ???		*/	0,		0,		0,
/* 0x57 - F11		*/	F11,		F11,		0,	
/* 0x58 - F12		*/	F12,		F12,		0,	
/* 0x59 - ???		*/	0,		0,		0,	
/* 0x5A - ???		*/	0,		0,		0,	
/* 0x5B - ???		*/	0,		0,		GUI_L,	
/* 0x5C - ???		*/	0,		0,		GUI_R,	
/* 0x5D - ???		*/	0,		0,		APPS,	
/* 0x5E - ???		*/	0,		0,		0,	
/* 0x5F - ???		*/	0,		0,		0,
/* 0x60 - ???		*/	0,		0,		0,
/* 0x61 - ???		*/	0,		0,		0,	
/* 0x62 - ???		*/	0,		0,		0,	
/* 0x63 - ???		*/	0,		0,		0,	
/* 0x64 - ???		*/	0,		0,		0,	
/* 0x65 - ???		*/	0,		0,		0,	
/* 0x66 - ???		*/	0,		0,		0,	
/* 0x67 - ???		*/	0,		0,		0,	
/* 0x68 - ???		*/	0,		0,		0,	
/* 0x69 - ???		*/	0,		0,		0,	
/* 0x6A - ???		*/	0,		0,		0,	
/* 0x6B - ???		*/	0,		0,		0,	
/* 0x6C - ???		*/	0,		0,		0,	
/* 0x6D - ???		*/	0,		0,		0,	
/* 0x6E - ???		*/	0,		0,		0,	
/* 0x6F - ???		*/	0,		0,		0,	
/* 0x70 - ???		*/	0,		0,		0,	
/* 0x71 - ???		*/	0,		0,		0,	
/* 0x72 - ???		*/	0,		0,		0,	
/* 0x73 - ???		*/	0,		0,		0,	
/* 0x74 - ???		*/	0,		0,		0,	
/* 0x75 - ???		*/	0,		0,		0,	
/* 0x76 - ???		*/	0,		0,		0,	
/* 0x77 - ???		*/	0,		0,		0,	
/* 0x78 - ???		*/	0,		0,		0,	
/* 0x78 - ???		*/	0,		0,		0,	
/* 0x7A - ???		*/	0,		0,		0,	
/* 0x7B - ???		*/	0,		0,		0,	
/* 0x7C - ???		*/	0,		0,		0,	
/* 0x7D - ???		*/	0,		0,		0,	
/* 0x7E - ???		*/	0,		0,		0,
/* 0x7F - ???		*/	0,		0,		0
};

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
void delayMs(u32);
void taskTty();
u32 keyboardRead(u32 mask);

// todo
void processB();
void processC();

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

    outByte(0x0fc, PORT_8259A_MASTER2);     // 写OCW1, 主片打开时钟中断、键盘终端
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
    u8 scan_code =inByte(PORT_KEYBOARD_DATA);

    if (keyBuf.count < KEY_BUF_SIZE) {
        *(keyBuf.head++) = scan_code;
        if (keyBuf.head >= keyBuf.buf + KEY_BUF_SIZE) {
            keyBuf.head = keyBuf.buf;
        }
        keyBuf.count++;
    }   
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

// 
void delayMs(u32 t) {
    u32 t1 = ticks;
    while ((ticks - t1 )*1000/CLOCK_COUNTER0_HZ <= t);
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

void taskTty(){
    while(1) {
        u32 key =keyboardRead(0);
        if (!(key& KEYBOARD_FLAG_EXT)) {  // 是否为可打印字符
            dispChar(key & 0x7f, 0x01);
        } else {
            dispColor = 0x04;
            dispInt(key);
        }
        dispChar(' ', 0x01);
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


u32 keyboardRead(u32 mask){
    u8 scan_code = getByteFromKeybuf();
    
    if (scan_code == 0xE1) { 
        if (getByteFromKeybuf() == 0x1D)
        {
            if (getByteFromKeybuf() == 0x45)
            {
                if (getByteFromKeybuf() == 0xE1)
                {
                    if (getByteFromKeybuf() == 0x9D)
                    {
                        if (getByteFromKeybuf() == 0xC5)
                        {
                            return PAUSEBREAK | (mask & 0xffffff00);
                        }
                    }
                }
            }
        }      
    }

    u32 startWithE0 = 0;
    if (scan_code == 0xE0)
    {
        scan_code = getByteFromKeybuf();
        if (scan_code== 0x2A)
        {
            if (getByteFromKeybuf() == 0xE0)
            {
                if (getByteFromKeybuf() == 0x37)
                {
                    return PRINTSCREEN | (mask & 0xffffff00);  // keydown 
                }
            }
        }else if (scan_code == 0xB7)
        {
            if (getByteFromKeybuf() == 0xE0)
            {
                if (getByteFromKeybuf() == 0xAA)
                {
                    return PRINTSCREEN | (mask & 0xffffff80);  // keyup
                }
            }
        }else {
            startWithE0 = 1;
        }
    }
    
    u32 col = 0;
    if (startWithE0) {
        col = 2;
    }else if ( mask & (KEYBOARD_FLAG_SHIFT_L | KEYBOARD_FLAG_SHIFT_R)) {
        col = 1;
    }
    
    u32 isKeyup = scan_code & KEYBOARD_FLAG_BREAK;
    u32 key = keymap[ (scan_code & 0x7F)*MAP_COLS +col];
    u32 flags = 0;
    // todo keyup remove, keydown add
    if (key == CTRL_R )  {
        flags |= KEYBOARD_FLAG_CTRL_R;
        key = 0;
    } else if (key == ALT_R ){
        flags |=  KEYBOARD_FLAG_CTRL_R;
        key = 0;
    } else if (key == SHIFT_R){
        flags |=  KEYBOARD_FLAG_SHIFT_R;
        key = 0;
    } else if (key == CTRL_L){
        flags |=  KEYBOARD_FLAG_CTRL_L;
        key = 0;
    } else if (key == ALT_L){
        flags |=  KEYBOARD_FLAG_ALT_L;
        key = 0;
    } else if (key == SHIFT_L){
        flags |=  KEYBOARD_FLAG_SHIFT_L;
        key = 0;
    } 

    if (key == 0 ) { 
        if (isKeyup && mask == flags) {          
            return KEYBOARD_FLAG_EXT | (mask & 0xffffff00) | isKeyup; // 单独按下 shift\alt\ctrl 
        }

        if (isKeyup) {   // 取消 shift\alt\ctrl 标记
            mask &= (~flags);
        }else {          // 添加 shift\alt\ctrl 标记
            mask |= flags;
        }
    }

    if (!key || isKeyup) {  
        return keyboardRead(mask);
    }else { 
        return key | (mask & 0xffffff00) | isKeyup; 
    }

    // 不响应的按键： shift\alt\ctrl的按下不响应，其他案件的弹起不响应
    // if ( (!isKeyup && !key) 
    //     || (isKeyup && key) ) {  
    //     return keyboardRead(mask);
    // }else { // 响应的按键： shift\alt\ctrl的弹起，其他案件的按下
    //     if (key == 0) mask = mask | KEYBOARD_FLAG_EXT ; // shift\alt\ctrl键应为不可打印
    //     return key | (mask & 0xffffff00) | isKeyup; 
    // }
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
