
#ifndef LIB_H
#define LIB_H

#include "types.h"

void itos(u32 a, char* p);
void memSet(u8* to, u8 val, u32 size);
void memCpy(u8* to, u8* from, u32 size);

// -----  来自汇编 ----------------------
u8 inByte(u16 port);
void outByte(u8 data, u16 port);
// -------------------------------------

#endif