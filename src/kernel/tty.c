
#include "types.h"
#include "keyboard.h"
#include "main.h"
#include "tty.h"

static Tty tty;
u16 dispPos;

void initTty() {
    tty.currentAddr = tty.startAddr = (u16*) VIDEO_ADDR_BASE;
    tty.cursorPosX = tty.cursorPosY = 0;
    tty.defaultColor = white;
    tty.limit = 2*80*25;

    addPCB(0, (u32)taskTty, 100);
}

void clearScreen(){
    u8* p = tty.startAddr;
    for (int i=0; i<tty.limit; i++) {
        *p++ = 0;
        *p++ = tty.defaultColor;
    }
}

// void outChar(char c){

// }

void outChar(char c, Color color){

}

// 显示字符串
void dispStr(char* p, u8 color){
    while(*p != 0) {
        if (*p == '\n'){
            dispPos += 160 - dispPos % 160;
        }
        else {
            dispChar(*p, color);
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
    u32 pos = dispPos/ 2 ;
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
            dispChar(key & 0x7f, 0x01);
        } else {
            if ( key == DOWN) {
                scrollTo(80*15);
            }else if (key == UP) {
                scrollTo(80*0);
            }
            dispInt(key, 0x04);
        }
        dispChar(' ', 0x01);
    }
}
