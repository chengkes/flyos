
#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Keymap for US MF-2 keyboard. */
#define MAP_COLS	    3	/* Number of columns in keymap */
#define NR_SCAN_CODES	0x80	/* Number of scan codes (rows in keymap) */

#define KEYBOARD_FLAG_BREAK	    NR_SCAN_CODES		/* Break Code			*/
#define KEYBOARD_FLAG_EXT	    0x0100		/* Normal function keys		*/
#define KEYBOARD_FLAG_SHIFT_L	0x0200		/* Shift key			*/
#define KEYBOARD_FLAG_SHIFT_R	0x0400		/* Shift key			*/
#define KEYBOARD_FLAG_CTRL_L	0x0800		/* Control key			*/
#define KEYBOARD_FLAG_CTRL_R	0x1000		/* Control key			*/
#define KEYBOARD_FLAG_ALT_L	    0x2000		/* Alternate key		*/
#define KEYBOARD_FLAG_ALT_R	    0x4000		/* Alternate key		*/
#define FLAG_PAD	            0x8000		/* keys in num pad		*/

/* Special keys */
#define ESC		    (0x01 + KEYBOARD_FLAG_EXT)	/* Esc		*/
#define TAB		    (0x02 + KEYBOARD_FLAG_EXT)	/* Tab		*/
#define ENTER		(0x03 + KEYBOARD_FLAG_EXT)	/* Enter	*/
#define BACKSPACE	(0x04 + KEYBOARD_FLAG_EXT)	/* BackSpace	*/

#define GUI_L		(0x05 + KEYBOARD_FLAG_EXT)	/* L GUI	*/
#define GUI_R		(0x06 + KEYBOARD_FLAG_EXT)	/* R GUI	*/
#define APPS		(0x07 + KEYBOARD_FLAG_EXT)	/* APPS	*/

/* Shift, Ctrl, Alt */
#define SHIFT_L		(0x08 + KEYBOARD_FLAG_EXT)	/* L Shift	*/
#define SHIFT_R		(0x09 + KEYBOARD_FLAG_EXT)	/* R Shift	*/
#define CTRL_L		(0x0A + KEYBOARD_FLAG_EXT)	/* L Ctrl	*/
#define CTRL_R		(0x0B + KEYBOARD_FLAG_EXT)	/* R Ctrl	*/
#define ALT_L		(0x0C + KEYBOARD_FLAG_EXT)	/* L Alt	*/
#define ALT_R		(0x0D + KEYBOARD_FLAG_EXT)	/* R Alt	*/

/* Lock keys */
#define CAPS_LOCK	(0x0E + KEYBOARD_FLAG_EXT)	/* Caps Lock	*/
#define	NUM_LOCK	(0x0F + KEYBOARD_FLAG_EXT)	/* Number Lock	*/
#define SCROLL_LOCK	(0x10 + KEYBOARD_FLAG_EXT)	/* Scroll Lock	*/

/* Function keys */
#define F1		(0x11 + KEYBOARD_FLAG_EXT)	/* F1		*/
#define F2		(0x12 + KEYBOARD_FLAG_EXT)	/* F2		*/
#define F3		(0x13 + KEYBOARD_FLAG_EXT)	/* F3		*/
#define F4		(0x14 + KEYBOARD_FLAG_EXT)	/* F4		*/
#define F5		(0x15 + KEYBOARD_FLAG_EXT)	/* F5		*/
#define F6		(0x16 + KEYBOARD_FLAG_EXT)	/* F6		*/
#define F7		(0x17 + KEYBOARD_FLAG_EXT)	/* F7		*/
#define F8		(0x18 + KEYBOARD_FLAG_EXT)	/* F8		*/
#define F9		(0x19 + KEYBOARD_FLAG_EXT)	/* F9		*/
#define F10		(0x1A + KEYBOARD_FLAG_EXT)	/* F10		*/
#define F11		(0x1B + KEYBOARD_FLAG_EXT)	/* F11		*/
#define F12		(0x1C + KEYBOARD_FLAG_EXT)	/* F12		*/

/* Control Pad */
#define PRINTSCREEN	(0x1D + KEYBOARD_FLAG_EXT)	/* Print Screen	*/
#define PAUSEBREAK	(0x1E + KEYBOARD_FLAG_EXT)	/* Pause/Break	*/
#define INSERT		(0x1F + KEYBOARD_FLAG_EXT)	/* Insert	*/
#define DELETE		(0x20 + KEYBOARD_FLAG_EXT)	/* Delete	*/
#define HOME		(0x21 + KEYBOARD_FLAG_EXT)	/* Home		*/
#define END		    (0x22 + KEYBOARD_FLAG_EXT)	/* End		*/
#define PAGEUP		(0x23 + KEYBOARD_FLAG_EXT)	/* Page Up	*/
#define PAGEDOWN	(0x24 + KEYBOARD_FLAG_EXT)	/* Page Down	*/
#define UP		    (0x25 + KEYBOARD_FLAG_EXT)	/* Up		*/
#define DOWN		(0x26 + KEYBOARD_FLAG_EXT)	/* Down		*/
#define LEFT		(0x27 + KEYBOARD_FLAG_EXT)	/* Left		*/
#define RIGHT		(0x28 + KEYBOARD_FLAG_EXT)	/* Right	*/

/* ACPI keys */
#define POWER		(0x29 + KEYBOARD_FLAG_EXT)	/* Power	*/
#define SLEEP		(0x2A + KEYBOARD_FLAG_EXT)	/* Sleep	*/
#define WAKE		(0x2B + KEYBOARD_FLAG_EXT)	/* Wake Up	*/

/* Num Pad */
#define PAD_SLASH	(0x2C + KEYBOARD_FLAG_EXT)	/* /		*/
#define PAD_STAR	(0x2D + KEYBOARD_FLAG_EXT)	/* *		*/
#define PAD_MINUS	(0x2E + KEYBOARD_FLAG_EXT)	/* -		*/
#define PAD_PLUS	(0x2F + KEYBOARD_FLAG_EXT)	/* +		*/
#define PAD_ENTER	(0x30 + KEYBOARD_FLAG_EXT)	/* Enter	*/
#define PAD_DOT		(0x31 + KEYBOARD_FLAG_EXT)	/* .		*/
#define PAD_0		(0x32 + KEYBOARD_FLAG_EXT)	/* 0		*/
#define PAD_1		(0x33 + KEYBOARD_FLAG_EXT)	/* 1		*/
#define PAD_2		(0x34 + KEYBOARD_FLAG_EXT)	/* 2		*/
#define PAD_3		(0x35 + KEYBOARD_FLAG_EXT)	/* 3		*/
#define PAD_4		(0x36 + KEYBOARD_FLAG_EXT)	/* 4		*/
#define PAD_5		(0x37 + KEYBOARD_FLAG_EXT)	/* 5		*/
#define PAD_6		(0x38 + KEYBOARD_FLAG_EXT)	/* 6		*/
#define PAD_7		(0x39 + KEYBOARD_FLAG_EXT)	/* 7		*/
#define PAD_8		(0x3A + KEYBOARD_FLAG_EXT)	/* 8		*/
#define PAD_9		(0x3B + KEYBOARD_FLAG_EXT)	/* 9		*/
#define PAD_UP		PAD_8			/* Up		*/
#define PAD_DOWN	PAD_2			/* Down		*/
#define PAD_LEFT	PAD_4			/* Left		*/
#define PAD_RIGHT	PAD_6			/* Right	*/
#define PAD_HOME	PAD_7			/* Home		*/
#define PAD_END		PAD_1			/* End		*/
#define PAD_PAGEUP	PAD_9			/* Page Up	*/
#define PAD_PAGEDOWN	PAD_3			/* Page Down	*/
#define PAD_INS		PAD_0			/* Ins		*/
#define PAD_MID		PAD_5			/* Middle key	*/
#define PAD_DEL		PAD_DOT			/* Del		*/

#endif
