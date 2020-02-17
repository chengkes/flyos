
#include "types.h"
#include "main.h"

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



