
#ifndef TTY_H
#define TTY_H

#include "types.h"

// 显示器端口
#define PORT_DISPLAY_CRTC_ADDR  0x3D4
#define PORT_DISPLAY_CRTC_DATA  0x3D5

#define CRTC_CURSOR_LOC_H  0x0E
#define CRTC_CURSOR_LOC_L  0x0F
#define CRTC_START_ADDR_H  0x0C
#define CRTC_START_ADDR_L  0x0D

#define VIDEO_ADDR_BASE     0xb8000

#define MAX_ROWS        25
#define MAX_COLS        80

void initTty();
void taskTty();
void outChar(char ,Color);
void setCursorPos ( );
void dispInt(u32, u8);
void dispStr(char*, u8);

#endif
