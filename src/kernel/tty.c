
#include "types.h"
#include "keyboard.h"
#include "interrupt.h"
#include "tty.h"
#include "lib.h"
#include "pcb.h"

static Tty tty[TTY_COUNT];
static volatile int currentTtyIdx;

void initTty() {
    currentTtyIdx = 0;
    u32 addr = 0; 
    for(int i=0; i<TTY_COUNT; i++) {
        Tty* p = &tty[i];
        p->currentAddr = p->startAddr = addr;
        p->defaultColor = white;
        p->limit = 2*MAX_COLS*MAX_ROWS;     
        addr += p->limit;
    }
}

void sysWrite(char* s, Color c, PCB* p) {
    dispStr( &tty[p->ttyIdx], s, c);
}

Tty* getCurrentTty(){
    return &tty[currentTtyIdx];
}

void printf(char* fmt) {
    PCB* pcb = getCurrentPcb();
    Tty* t = & tty[pcb->ttyIdx];
    dispStr(t, fmt, t->defaultColor);
}

void clearScreen(Tty* t ){
    u8* p = (u8*)( t->startAddr + VIDEO_ADDR_BASE);
    for (int i=0; i<t->limit; i++) {
        *p++ = ' ';
        *p++ = t->defaultColor;
    }
    t->currentAddr = t->startAddr;
    t->cursorRow = t->cursorCol = 0;
    setCursorPos(t);
}

void backspace(Tty* t ) {
    if (t->cursorRow==0 && t->cursorCol ==0) return;

    u8* p = (u8*)(VIDEO_ADDR_BASE + t->currentAddr +2*t->cursorRow *MAX_COLS + 2*t->cursorCol - 2);
    *p++ = 0;
    *p++ = t->defaultColor; 

    t->cursorCol --;
    if (t->cursorCol < 0 ) {
        t->cursorCol = MAX_COLS -1;
        t->cursorRow --;
    }
    setCursorPos(t);
}

void outChar(Tty* t ,char c, Color color){
    if (c == '\n') {
         t->cursorRow ++;
         t->cursorCol = 0;
    }
    else {
        u8 *p = (u8 *)(VIDEO_ADDR_BASE + t->currentAddr + 2*t->cursorRow * MAX_COLS + 2*t->cursorCol);
        *p++ = c;
        *p++ = color;
        t->cursorCol++;
        if (t->cursorCol >= MAX_COLS)
        {
            t->cursorCol = 0;
            t->cursorRow++;
        }
    }
    setCursorPos(t);
}

// 显示字符串
void dispStr(Tty* t, char* p, Color color){
    while(*p != 0) outChar(t, *p++, color);
}

// 显示整数
void dispInt(Tty* t, u32 a, Color color){
    char b[9]="";
    itos(a, b);
    dispStr(t, b, color);
}

void setCursorPos (Tty* t ){
    if (t != getCurrentTty()) return;

    if (t->cursorRow >= MAX_ROWS) {
        scrollUp(t);
    }else {
        u32 pos = t->currentAddr/2 + t->cursorRow * MAX_COLS + t->cursorCol;
        disableInt();
        outByte(CRTC_CURSOR_LOC_L, PORT_DISPLAY_CRTC_ADDR);
        outByte((pos)&0xff, PORT_DISPLAY_CRTC_DATA);
        outByte(CRTC_CURSOR_LOC_H, PORT_DISPLAY_CRTC_ADDR);
        outByte((pos >> 8) & 0xff, PORT_DISPLAY_CRTC_DATA);
        enableInt();
    }
}

// 通过移动数据，向上滚动一行
void scrollUp(Tty* t ) {
    u8* p =(u8*) (VIDEO_ADDR_BASE + t->currentAddr);
    for (int r=0; r<MAX_ROWS-1; r++) { 
        memCpy(p, p+2*MAX_COLS, 2*MAX_COLS);
        p+= 2*MAX_COLS;
    }
    
    for (int c=0; c<2*MAX_COLS; c+=2) {
        *(p+c)= 0;
        *(p+c+1) = t->defaultColor;
    }

    t->cursorRow--;
    setCursorPos(t);
}

// 通过改变显示起始地址切换TTY
void activeTty(int idx){
    while (idx< 0)  idx += TTY_COUNT;
    while (idx>= TTY_COUNT)  idx -= TTY_COUNT;
    
    u32 pos =  tty[idx].currentAddr/2 ;
    disableInt();
    outByte(CRTC_START_ADDR_H, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos>>8) & 0xff, PORT_DISPLAY_CRTC_DATA);
    outByte(CRTC_START_ADDR_L, PORT_DISPLAY_CRTC_ADDR );
    outByte( (pos) & 0xff, PORT_DISPLAY_CRTC_DATA);
    enableInt();
    currentTtyIdx = idx ;
    setCursorPos(&tty[currentTtyIdx]);
}

void taskTty(){
    for (int i=0; i<TTY_COUNT; i++) {
        Tty* p = &tty[i];
        clearScreen(p);
        dispStr(p, "TTY-", white);
        dispInt(p, i+1, red);
        outChar(p, '#', red);
    }

    while(1) {
        u32 key =keyboardRead(0);
        if (!(key& KEYBOARD_FLAG_EXT)) {  // 是否为可打印字符
            outChar(getCurrentTty(), key & 0x7f, 0x01);
        } else {
            if ( key == DOWN) {
                activeTty(currentTtyIdx +1);
            // }else if (key == UP) {
            //     activeTty(currentTtyIdx -1);
            // }else if (key == LEFT) {
            //     clearScreen(getCurrentTty());
            // }else if (key == BACKSPACE) {
            //     backspace(getCurrentTty());
            // }else if(key == ENTER) {
            //     outChar(getCurrentTty(), '\n', white);
            // }else if (key ==TAB) {
            //     dispStr(getCurrentTty(), "    ", white);
            }else {
                dispInt(getCurrentTty(), key, red);  // todo,  fot tests
            }
        }
    }
}
