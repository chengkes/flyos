
#include "types.h"
#include "keyboard.h"
#include "main.h"
#include "tty.h"
#include "lib.h"

static Tty tty;

void initTty() {
    tty.currentAddr = tty.startAddr = (u16*) VIDEO_ADDR_BASE;
    tty.cursorRow = tty.cursorCol = 0;
    tty.defaultColor = white;
    tty.limit = 2*MAX_COLS*MAX_ROWS;
    clearScreen();

    addPCB(0, (u32)taskTty, 100);
}

void clearScreen(){
    u8* p = (u8*)tty.startAddr;
    for (int i=0; i<tty.limit; i++) {
        *p++ = 0;
        *p++ = tty.defaultColor;
    }
    tty.cursorRow = tty.cursorCol = 0;
    setCursorPos();
}

void backspace() {
    if (tty.cursorRow==0 && tty.cursorCol ==0) return;

    u8* p = (u8*)(tty.currentAddr +tty.cursorPosX *MAX_COLS + tty.cursorPosY - 1);
    *p++ = 0;
    *p++ = tty.defaultColor; 

    tty.cursorCol --;
    if (tty.cursorCol < 0 ) {
        tty.cursorCol = MAX_COLS -1;
        tty.cursorRow --;
    }
    setCursorPos();
}

void outChar(char c, Color color){
    if (c == '\n') {
         tty.cursorRow ++;
         tty.cursorCol = 0;
    }
    else {
        u8 *p = (u8 *)(tty.currentAddr + tty.cursorRow * MAX_COLS + tty.cursorCol);
        *p++ = c;
        *p++ = color;
        tty.cursorCol++;
        if (tty.cursorCol >= MAX_COLS)
        {
            tty.cursorCol = 0;
            tty.cursorRow++;
        }
    }
    setCursorPos();
}

// 显示字符串
void dispStr(char* p, Color color){
    while(*p != 0) {
        outChar(*p++, color);
    }
}

// 显示整数
void dispInt(u32 a, Color color){
    char b[9]="";
    itos(a, b);
    dispStr(b, color);
}

void setCursorPos (){
    u32 pos = tty.cursorRow*MAX_COLS + tty.cursorCol;
    outByte(CRTC_CURSOR_LOC_L, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos) & 0xff, PORT_DISPLAY_CRTC_DATA);
    outByte(CRTC_CURSOR_LOC_H, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos>>8) & 0xff, PORT_DISPLAY_CRTC_DATA);
}

// todo:通过移动数据，向上滚动一行
void scrollUp() {
    u8* p =(u8*) tty.currentAddr;
    for (int r=0; r<MAX_ROWS-1; r++) { 
        memCpy(p, p+2*MAX_COLS, 2*MAX_COLS);
    }
    
    p +=  2*(MAX_COLS*(MAX_ROWS-1)) ;
    for (int c=0; c<2*MAX_COLS, c+=2) {
        *(p+c)= 0;
        *(p+c+1) = tty.defaultColor;
    }

    tty.cursorRow--;
    setCursorPos();s
}

// todo:通过改变显示起始地址滚屏
void scrollTo(u32 pos){
    outByte(CRTC_START_ADDR_L, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos) & 0xff, PORT_DISPLAY_CRTC_DATA);
    outByte(CRTC_START_ADDR_H, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos>>8) & 0xff, PORT_DISPLAY_CRTC_DATA);
}

void taskTty(){
    while(1) {
        u32 key =keyboardRead(0);
        if (!(key& KEYBOARD_FLAG_EXT)) {  // 是否为可打印字符
            outChar(key & 0x7f, 0x01);
        } else {
            if ( key == DOWN) {
                scrollTo(80*15);
                continue;
            }else if (key == UP) {
                scrollTo(80*0);
                continue;
            }else if (key == LEFT) {
                clearScreen();
                continue;
            }else if (key == BACKSPACE) {
                backspace();
                continue;
            }
            dispInt(key, 0x04);
        }
    }
}
