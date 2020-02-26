
#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "types.h"

// 异常对应中断号
#define INT_VECTOR_DIVIDE_ERROR           0x00
#define INT_VECTOR_SINGLE_STEP_EXCEPTION  0x01
#define INT_VECTOR_NMI                    0x02
#define INT_VECTOR_BREAKPOINT_EXCEPTION   0x03
#define INT_VECTOR_OVERFLOW               0x04
#define INT_VECTOR_BOUNDS_CHECK           0x05
#define INT_VECTOR_INVAL_OPCODE           0x06
#define INT_VECTOR_COPR_NOT_AVAILABLE     0x07
#define INT_VECTOR_DOUBLE_FAULT           0x08
#define INT_VECTOR_COPR_SEG_OVERRUN       0x09
#define INT_VECTOR_INVAL_TSS              0x0A
#define INT_VECTOR_SEGMENT_NOT_PRESENT    0x0B
#define INT_VECTOR_STACK_EXCEPTION        0x0C
#define INT_VECTOR_GENERAL_PROTECTION     0x0D
#define INT_VECTOR_PAGE_FAULT             0x0E
#define INT_VECTOR_COPR_ERROR             0x10

// 系统调研中断号
#define INT_VECTOR_SYSCALL  0x90   

#define SYSCALL_IDX_WRITE      0

// 外部中断对应中断号
#define INT_VECTOR_IRQ0    0x20
#define INT_VECTOR_IRQ8    0x28

#define IRQ_HANDLER_IDX_CLOCK     0
#define IRQ_HANDLER_IDX_KEYBOARD  1

void putIrqHandler(u8 no, void* handler);
void putSyscall(u8 no, void* handler);
void init8259a();
void buildIdt();

void enableInt();
void disableInt();

// ----- 来自汇编的函数声明 --------------------
void enableIrq(int);

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

void divide_error();
void single_step_exception();
void nmi();
void breakpoint_exception();
void overflow();
void bounds_check();
void inval_opcode();
void copr_not_available();
void double_fault();
void copr_seg_overrun();
void inval_tss();
void segment_not_present();
void stack_exception();
void general_protection();
void page_fault();
void copr_error();

void int90syscall();

#endif