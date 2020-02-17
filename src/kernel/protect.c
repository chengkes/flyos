
#include "types.h"
#include "main.h"

Descriptor gdt[GDT_SIZE];
u8 gdtPtr[6];

TSS tss;


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

void initProtectMode() {
     //将GDT从loader移动到kernel,执行前gdtPtr存放loader中GDT PTR信息,执行后gdtPtr存放kernel中GDT PTR信息
    memCpy((u8*)&gdt,(u8*) (*((u32*)(gdtPtr+2))), *((u16*)gdtPtr)+1);
    *((u16*)gdtPtr) = (u16)(sizeof(gdt)-1);         // GDT limit
    *((u32*)(gdtPtr+2))= (u32)&gdt;                 // GDT base

    // 初始化TSS 及 TSS描述符
    memSet((u8*)&tss, 0, sizeof(TSS));
    tss.ss0 = GDT_SELECTOR_D32;
    tss.iobase = sizeof(TSS);
    initDescriptor(&gdt[GDT_SELECTOR_TSS>>3],(u32) &tss, sizeof(TSS)-1, DA_386TSS, 0 );

}

