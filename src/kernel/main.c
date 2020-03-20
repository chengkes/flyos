#include "main.h"
#include "keyboard.h"
#include "tty.h"
#include "pcb.h"
#include "interrupt.h"
#include "clock.h"

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
    char buf[1024];
    sprintf(buf, "Fail(%s:%d):%s\n", file, line, exp);
    write(buf, red);
    while(1);
    asm volatile ("ud2");
}

