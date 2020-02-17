#include "main.h"
#include "keyboard.h"
#include "tty.h"

void osinit(){
    initProtectMode() ;
    init8259a();
    buildIdt();
    initPcb();
    initClock();
    initKeyboard();
    initTty();

    dispStr("\n\n  Welcome to Fly OS! \n\n", 0x0c);
}

