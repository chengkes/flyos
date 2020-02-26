
#ifndef TTY_H
#define TTY_H

#include "types.h"

// todo : 完善其他颜色
typedef enum _Color{
	black = 0x00,
	blue = 0x01,
	green = 0x02,
	cyan = 0x03,		// 青色 
	red = 0x04,
	magenta = 0x03,		// 洋红
	brown = 0x03,		// 棕色
	white = 0x07,
	gray = 0x08
} Color;

#define TTY_INPUT_BUF_SIZE 128
typedef struct _Tty {
	u32 currentAddr;
	u32 startAddr;
	u32 limit;
	int cursorRow;
	int cursorCol;
	Color defaultColor;
	// TTY输入缓冲区
	u32 inputBuf[TTY_INPUT_BUF_SIZE];
	volatile int inputBufCount;
	u32 inputBufHead, inputBufTail;
} Tty;

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

#define TTY_COUNT		3

void initTty();
void taskTty();
void write(char* s, Color c);
// void outChar(Tty*, char ,Color);
// void clearScreen(Tty* );
void setCursorPos(Tty* );
// void dispInt(Tty* t, u32, Color);
void dispStr(Tty* , char*, Color);
void scrollUp(Tty* ) ;
u32 readKey();

#endif
