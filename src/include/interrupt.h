
#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "types.h"

#define CLOCK_HANDLER_IDX  0
#define KEYBOARD_HANDLER_IDX  1

void putIrqHandler(u8 no, IrqHandler handler);
void init8259a();
void buildIdt();

// ----- 来自汇编的函数声明 --------------------
void enableInt();
void disableInt();

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

#endif