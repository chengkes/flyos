#include "main.h"
#include "keyboard.h"
#include "tty.h"
#include "pcb.h"
#include "interrupt.h"

void osinit(){
    initProtectMode();
    init8259a();
    buildIdt();
    initPcb();
    initClock();
    initKeyboard();
    initTty();
}

void assert_fail(char* exp, char* file, int line) {
    // wr("ASSERT FAIL: ", red);
    // printf(file, red);
    // printf(line, red);
    // printf(exp, red); 
    while(1);
}