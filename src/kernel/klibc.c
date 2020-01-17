
typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef int   s32;
typedef short s16;
typedef char  s8;

typedef struct _GdtPtr {
    u16 limit;     //GDT界限    
    void* base;      //GDT基址  
} GdtPtr;

typedef struct _Descriptor {
    u16  	limit1 ;	    // 段界限 1				(2 字节)
	u16     base1; 			// 段基址 1				(2 字节)
	u8	    base2;			// 段基址 2	    			(1 字节)
	u8      comp;           // 属性 1 + 段界限 2 
    u8      attr2;          // 属性 2		(1 字节)
	u8	    base3;			// 段基址 3				(1 字节)
} Descriptor;

#define GDT_SIZE 128

void memCpy(u8* to, u8* from, u32 size);

Descriptor gdt[GDT_SIZE];
GdtPtr gdtPtr;

/// 将GDT从loader移动到kernel
/// 执行前gdtPtr存放loader中GDT PTR信息
/// 执行后gdtPtr存放kernel中GDT PTR信息
void mvGdt(){
    memCpy((u8*)&gdt, (u8*)gdtPtr.base, gdtPtr.limit+1);
    gdtPtr.base = &gdt;
    gdtPtr.limit = sizeof(gdt)-1;
}

void memCpy(u8* to, u8* from, u32 size){
    for(u32 i=0;i<size; i++){
        to[i] = from[i];
    }
}
