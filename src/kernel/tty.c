
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
    clearScreen();

    addPCB(0, (u32)taskTty, 100);
}

void clearScreen(){
    u8* p = (u8*)tty.startAddr;
    for (int i=0; i<tty.limit; i++) {
        *p++ = 0;
        *p++ = tty.defaultColor;
    }
    tty.cursorPosX = tty.cursorPosY = 0;
    setCursorPos();
}

// todo 
void backspace() {
    if (tty.cursorPosX==0 && tty.cursorPosY ==0) return;

    // u8* p = (u8*)(tty.currentAddr +tty.cursorPosX *MAX_COLS + tty.cursorPosY - 1);
    // *p++ = 0;
    // *p++ = tty.defaultColor; 

    tty.cursorPosY --;
    if (tty.cursorPosY < 0 ) {
        if (tty.cursorPosX > 0) {
            tty.cursorPosY = MAX_COLS -1;
            tty.cursorPosX --;
        }else {
            tty.cursorPosX = tty.cursorPosY = 0;
        }
    }
    setCursorPos();
}

void outChar(char c, Color color){
    u8* p = (u8*)(tty.currentAddr +tty.cursorPosX *MAX_COLS + tty.cursorPosY);
    *p++ = c;
    *p++ = color; 
    tty.cursorPosY += 1;
    if (tty.cursorPosY >= MAX_COLS) {
        tty.cursorPosY = 0;
        tty.cursorPosX++;
    }

    setCursorPos();
}

// 显示字符串
void dispStr(char* p, Color color){
    while(*p != 0) {
        if (*p == '\n'){
            tty.cursorPosX ++;
            tty.cursorPosY = 0;            
        }
        else {
            outChar(*p, color);
        }
        p++;
    }
    setCursorPos();
}

// 显示整数
void dispInt(u32 a, Color color){
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
