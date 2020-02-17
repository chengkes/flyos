
#include "types.h"
#include "keyboard.h"
#include "main.h"
#include "tty.h"

static Tty tty;

void initTty() {
    tty.currentAddr = tty.startAddr = (u16*) VIDEO_ADDR_BASE;
    tty.cursorPosX = tty.cursorPosY = 0;
    tty.defaultColor = white;
    tty.limit = 2*MAX_COLS*MAX_ROWS;

    addPCB(0, (u32)taskTty, 100);
}

void clearScreen(){
    u8* p = tty.startAddr;
    for (int i=0; i<tty.limit; i++) {
        *p++ = 0;
        *p++ = tty.defaultColor;
    }
}

void outChar(char c, Color color){
    u8* p = (tty.currentAddr +tty.cursorPosX *MAX_COLS + tty.cursorPosY);
    *p++ = c;
    *p++ = color; 
    tty.cursorPosY += 1;
    if (tty.cursorPosY >= MAX_COLS) {
        tty.cursorPosY = 0;
        tty.cursorPosX++;
    }
}

// 显示字符串
void dispStr(char* p, u8 color){
    while(*p != 0) {
        if (*p == '\n'){
            // sdispPos += 160 - dispPos % 160;
        }
        else {
            outChar(*p, color);
        }
        p++;
    }
}

// 显示整数
void dispInt(u32 a, u8 color){
    char b[9]="";
    itos(a, b);
    dispStr(b, color);
}

void setCursorPos (){
    u32 pos = tty.cursorPosX*80 + tty.cursorPosY;
    outByte(CRTC_CURSOR_LOC_L, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos) & 0xff, PORT_DISPLAY_CRTC_DATA);
    outByte(CRTC_CURSOR_LOC_H, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos>>8) & 0xff, PORT_DISPLAY_CRTC_DATA);
}

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
            }else if (key == UP) {
                scrollTo(80*0);
            }
            dispInt(key, 0x04);
        }
        outChar(' ', 0x01);
    }
}
