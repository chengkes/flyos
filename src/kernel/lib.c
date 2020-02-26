
#include "types.h"

void memCpy(u8* to, u8* from, u32 size){
    for(u32 i=0;i<size; i++) to[i] = from[i];
}

void memSet(u8* to, u8 val, u32 size){
    for(int i=0;i<size; i++) to[i] = val;
}

// 整数转换为base进制字符串
void itos(u32 a, u32 base, char* p){
    char b[32]="";
    int w = 0;

    while (a>0) {
        b[w] = a%base;
        if (b[w] >=10 ) {
            b[w] += 'A' - 10;
        }else {
            b[w] += '0';
        }
        a /= base;
        w++;
    }

    *p++ = '0';
    while (w >=0) {
        *p++ = b[--w];
    }    
    *p = 0;
}


