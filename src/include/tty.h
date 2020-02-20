
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

typedef struct _Tty {
	u16* currentAddr;
	u16* startAddr;
	u32 limit;
	int cursorRow;
	int cursorCol;
	Color defaultColor;
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

void initTty();
void taskTty();
void outChar(char ,Color);
void clearScreen();
void setCursorPos();
void dispInt(u32, Color);
void dispStr(char*, Color);
void scrollUp() ;

#endif
