
#include "types.h"
#include "keyboard.h"
#include "main.h"
#include "tty.h"
#include "lib.h"

static Tty tty[TTY_COUNT];
static u32 currentTtyIdx;

void initTty() {
    currentTtyIdx = 0;
    u32 addr = VIDEO_ADDR_BASE; 
    for(int i=0; i<TTY_COUNT; i++) {
        Tty* p = &tty[i];
        p->currentAddr = p->startAddr = (u16*) addr;
        p->defaultColor = white;
        p->limit = 2*MAX_COLS*MAX_ROWS;
        clearScreen(p);
        addr += p->limit;
    }
    addPCB(0, (u32)taskTty, 100);
}

Tty* getCurrentTty(){
    return &tty[currentTtyIdx];
}

void clearScreen(){
    Tty* t = getCurrentTty();
    u8* p = (u8*)t->startAddr;
    for (int i=0; i<t->limit; i++) {
        *p++ = 0;
        *p++ = t->defaultColor;
    }
    t->cursorRow = t->cursorCol = 0;
    setCursorPos();
}

void backspace() {
    Tty* t = getCurrentTty();
    if (t->cursorRow==0 && t->cursorCol ==0) return;

    u8* p = (u8*)(t->currentAddr +t->cursorRow *MAX_COLS + t->cursorCol - 1);
    *p++ = 0;
    *p++ = t->defaultColor; 

    t->cursorCol --;
    if (t->cursorCol < 0 ) {
        t->cursorCol = MAX_COLS -1;
        t->cursorRow --;
    }
    setCursorPos();
}

void outChar(char c, Color color){
    Tty* t = getCurrentTty();
    if (c == '\n') {
         t->cursorRow ++;
         t->cursorCol = 0;
    }
    else {
        u8 *p = (u8 *)(t->currentAddr + t->cursorRow * MAX_COLS + t->cursorCol);
        *p++ = c;
        *p++ = color;
        t->cursorCol++;
        if (t->cursorCol >= MAX_COLS)
        {
            t->cursorCol = 0;
            t->cursorRow++;
        }
    }
    setCursorPos();
}

// 显示字符串
void dispStr(char* p, Color color){
    while(*p != 0) outChar(*p++, color);
}

// 显示整数
void dispInt(u32 a, Color color){
    char b[9]="";
    itos(a, b);
    dispStr(b, color);
}

void setCursorPos (){
    Tty* t = getCurrentTty();
    if (t->cursorRow >= MAX_ROWS) {
        scrollUp();
    }else {
        u32 pos = t->cursorRow * MAX_COLS + t->cursorCol;
        outByte(CRTC_CURSOR_LOC_L, PORT_DISPLAY_CRTC_ADDR);
        outByte((pos)&0xff, PORT_DISPLAY_CRTC_DATA);
        outByte(CRTC_CURSOR_LOC_H, PORT_DISPLAY_CRTC_ADDR);
        outByte((pos >> 8) & 0xff, PORT_DISPLAY_CRTC_DATA);
    }
}

// 通过移动数据，向上滚动一行
void scrollUp() {
    Tty* t = getCurrentTty();
    u8* p =(u8*) t->currentAddr;
    for (int r=0; r<MAX_ROWS-1; r++) { 
        memCpy(p, p+2*MAX_COLS, 2*MAX_COLS);
        p+= 2*MAX_COLS;
    }
    
    for (int c=0; c<2*MAX_COLS; c+=2) {
        *(p+c)= 0;
        *(p+c+1) = t->defaultColor;
    }

    t->cursorRow--;
    setCursorPos();
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
            }else if(key == ENTER) {
                outChar('\n', white);
                continue;
            }else if (key ==TAB) {
                dispStr("    ", white);
                continue;
            }
            dispInt(key, 0x04);  // todo,  fot tests
        }
    }
}
