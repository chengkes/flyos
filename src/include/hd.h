#ifndef HD_H
#define HD_H

#define PORT_HD_SECONDARY_DATA          0x170
#define PORT_HD_SECONDARY_FEATURES      0x171
#define PORT_HD_SECONDARY_SECTOR_COUNT  0x172
#define PORT_HD_SECONDARY_LBA_LOW       0x173
#define PORT_HD_SECONDARY_LBA_MID       0x174
#define PORT_HD_SECONDARY_LBA_HIGH      0x175 

/*	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
	+-----+-----+-----+-----+-----+-----+-----+-----+
	|  1  |  L  |  1  | DRV | HS3 | HS2 | HS1 | HS0 |
	+-----+-----+-----+-----+-----+-----+-----+-----+
	         |           |   \_____________________/
	         |           |              |
	         |           |              `------------ If L=0, Head Select.
	         |           |                                    These four bits select the head number.
	         |           |                                    HS0 is the least significant.
	         |           |                            If L=1, HS0 through HS3 contain bit 24-27 of the LBA.
	         |           `--------------------------- Drive. When DRV=0, drive 0 (master) is selected. 
	         |                                               When DRV=1, drive 1 (slave) is selected.
	         `--------------------------------------- LBA mode. This bit selects the mode of operation.
 	                                                            When L=0, addressing is by 'CHS' mode.
 	                                                            When L=1, addressing is by 'LBA' mode.
*/
#define PORT_HD_SECONDARY_DEVICE        0x176

/* 	Any pending interrupt is cleared whenever this register is read.
	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
	+-----+-----+-----+-----+-----+-----+-----+-----+
	| BSY | DRDY|DF/SE|  #  | DRQ |     |     | ERR |
	+-----+-----+-----+-----+-----+-----+-----+-----+
	   |     |     |     |     |     |     |     |
	   |     |     |     |     |     |     |     `--- 0. Error.(an error occurred)
	   |     |     |     |     |     |     `--------- 1. Obsolete.
	   |     |     |     |     |     `--------------- 2. Obsolete.
	   |     |     |     |     `--------------------- 3. Data Request. (ready to transfer data)
	   |     |     |     `--------------------------- 4. Command dependent. (formerly DSC bit)
	   |     |     `--------------------------------- 5. Device Fault / Stream Error.
	   |     `--------------------------------------- 6. Drive Ready.
	   `--------------------------------------------- 7. Busy. If BSY=1, no other bits in the register are valid.
*/
#define PORT_HD_SECONDARY_STATUS        0x177   
/*	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
	+-----+-----+-----+-----+-----+-----+-----+-----+
	| HOB |  -  |  -  |  -  |  -  |SRST |-IEN |  0  |
	+-----+-----+-----+-----+-----+-----+-----+-----+
	   |                             |     |
	   |                             |     `--------- Interrupt Enable.
	   |                             |                  - IEN=0, and the drive is selected,
	   |                             |                    drive interrupts to the host will be enabled.
	   |                             |                  - IEN=1, or the drive is not selected,
	   |                             |                    drive interrupts to the host will be disabled.
	   |                             `--------------- Software Reset.
	   |                                                - The drive is held reset when RST=1.
	   |                                                  Setting RST=0 re-enables the drive.
	   |                                                - The host must set RST=1 and wait for at least
	   |                                                  5 microsecondsbefore setting RST=0, to ensure
	   |                                                  that the drive recognizes the reset.
	   `--------------------------------------------- HOB (High Order Byte)
	                                                    - defined by 48-bit Address feature set.
*/ 
#define PORT_HD_SECONDARY_CONTROL           0x376
#define PORT_HD_SECONDARY_ERROR         PORT_HD_SECONDARY_FEATURES
/*
	+--------+---------------------------------+-----------------+
	| Command| Command Description             | Parameters Used |
	| Code   |                                 | PC SC SN CY DH  |
	+--------+---------------------------------+-----------------+
	| ECh  @ | Identify Drive                  |             D   |
	| 91h    | Initialize Drive Parameters     |    V        V   |
	| 20h    | Read Sectors With Retry         |    V  V  V  V   |
	| E8h  @ | Write Buffer                    |             D   |
	+--------+---------------------------------+-----------------+

	KEY FOR SYMBOLS IN THE TABLE:
	===========================================-----=========================================================================
	PC    Register 1F1: Write Precompensation	@     These commands are optional and may not be supported by some drives.
	SC    Register 1F2: Sector Count		D     Only DRIVE parameter is valid, HEAD parameter is ignored.
	SN    Register 1F3: Sector Number		D+    Both drives execute this command regardless of the DRIVE parameter.
	CY    Register 1F4+1F5: Cylinder low + high	V     Indicates that the register contains a valid paramterer.
	DH    Register 1F6: Drive / Head
*/
#define PORT_HD_SECONDARY_COMMAND       PORT_HD_SECONDARY_STATUS

#define PORT_HD_PRIMARY_DATA          0x1F0
#define PORT_HD_PRIMARY_FEATURES      0x1F1
#define PORT_HD_PRIMARY_SECTOR_COUNT  0x1F2
#define PORT_HD_PRIMARY_LBA_LOW       0x1F3
#define PORT_HD_PRIMARY_LBA_MID       0x1F4
#define PORT_HD_PRIMARY_LBA_HIGH      0x1F5
#define PORT_HD_PRIMARY_DEVICE        0x1F6
#define PORT_HD_PRIMARY_STATUS        0x1F7            
#define PORT_HD_PRIMARY_CONTROL       0x3F6
#define PORT_HD_PRIMARY_ERROR         PORT_HD_PRIMARY_FEATURES
#define PORT_HD_PRIMARY_COMMAND       PORT_HD_PRIMARY_STATUS

#define HD_CMD_IDENTIFY		0xEC
#define HD_CMD_READ			0x20
#define HD_CMD_WRITE		0x30

#define SECTOR_SIZE 	512

#define MAKE_DEVICE(lba,device,lbaHigh) (0xA0 | ((1 & (lba))<<6) | ((1 & (device))<<4) | ((lbaHigh) & 0x0f))

void taskHd();
#endif
