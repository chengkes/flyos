#include "main.h"
#include "keyboard.h"
#include "tty.h"
#include "interrupt.h"

void osinit(){
    initProtectMode() ;
    init8259a();
    buildIdt();
    initPcb();
    initClock();
    initKeyboard();
    initTty();

    dispStr("\n  ", 0x0c);
}

