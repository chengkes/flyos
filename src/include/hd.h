#ifndef HD_H
#define HD_H

#define PORT_HD_SECONDARY_DATA          0x170
#define PORT_HD_SECONDARY_FEATURES      0x171
#define PORT_HD_SECONDARY_SECTOR_COUNT  0x172
#define PORT_HD_SECONDARY_LBA_LOW       0x173
#define PORT_HD_SECONDARY_LBA_MID       0x174
#define PORT_HD_SECONDARY_LBA_HIGH      0x175 

/*	PORT_HD_SECONDARY_DEVICEs
	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
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

/* PORT_HD_SECONDARY_STATUS
	Any pending interrupt is cleared whenever this register is read.
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
/*	PORT_HD_SECONDARY_CONTROL
	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
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
#define PORT_HD_SECONDARY_CONTROL       0x376
#define PORT_HD_SECONDARY_ERROR         PORT_HD_SECONDARY_FEATURES
/*   PORT_HD_SECONDARY_COMMAND
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

#define HD_STATUS_BSY	0x80
#define HD_STATUS_DRQ	0x08

#define SECTOR_SIZE 	512

#define MAKE_DEVICE(lba, device, lbaHigh) (0xA0 | ((1 & (lba)) << 6) | ((1 & (device)) << 4) | ((lbaHigh)&0x0f))

// 硬盘分区信息
typedef struct _PartInfo
{
    u32 startSector;
    u32 sectorCnt;
    u16 type;
    u8 isBootable;    
} PartInfo;

#define MAX_PART_COUNT  16 		// 最多支持分区数目
// 硬盘信息
typedef struct _HdInfo
{
	u8 device;
	u8 chanel;
    u32 sectorCnt;
    u16 capability;
    u16 cmdSet;
    char sn[21];
    char model[41];
	u8 partCnt;
	PartInfo partInfo[MAX_PART_COUNT];
} HdInfo;

void taskHd();
int identifyHd(HdInfo* hd);

int readHd(HdInfo* hd, u16* buf, u32 sector, u8 sectorCnt);
int writeHd(HdInfo* hd, u16* buf, u32 sector, u8 sectorCnt);
#endif
