
#include "tty.h"
#include "interrupt.h"
#include "pcb.h"
#include "hd.h"
#include "lib.h"
#include "clock.h"
#include "main.h"
#include "types.h"

static volatile u8 hd0Status;
static volatile u8 hd1Status;

static void printHdIdentityInfo(u8 device, u8 chanel);
static void printPartInfo(u8 device, u8 chanel, int sectorNo);
static void printOnePart(u8 device, u8 chanel, u8 *buf, int lba);

// 硬盘ATA0,中断处理程序
static void hdIrqHandler0()
{
    hd0Status = inByte(PORT_HD_PRIMARY_STATUS); //读取 hard disk status, 使硬盘能继续相应中断sen
    sendIntMsgTo(4);                            // taskHd 在PCB中的索引， 参见pcb.c:void initPcb();
}

// 硬盘ATA1,中断处理程序
static void hdIrqHandler1()
{
    hd1Status = inByte(PORT_HD_SECONDARY_STATUS); //读取 hard disk status, 使硬盘能继续相应中断sen
    sendIntMsgTo(4);                              // taskHd 在PCB中的索引， 参见pcb.c:void initPcb();
}

static void initHd()
{
    putIrqHandler(IRQ_IDX_HARDDISK0, hdIrqHandler0); // 指定硬盘中断处理程序
    putIrqHandler(IRQ_IDX_HARDDISK1, hdIrqHandler1); // 指定硬盘中断处理程序

    u8 *hd = (u8 *)0x475; // 获取硬盘个数， BIOS已经将硬盘个数写入内存0x475处
    printf("hd count: %d \n", *hd);
    printHdIdentityInfo(0, 0); // 获取硬盘信息并显示
    printPartInfo(0, 0, 0);    // 获取硬盘分区信息

    // test writeHd and readHd, use shell command xxd to verify
    // u16 buf[512];
    // memSet((u8*)buf, 8, sizeof(buf));
    // writeHd(0, 0, 1, sizeof(buf)/SECTOR_SIZE, buf);
    // memSet((u8*)buf, 0, sizeof(buf));
    // readHd(0, 0, 0, sizeof(buf)/SECTOR_SIZE, buf);
    // for(int i=0;i<sizeof(buf)/2; i++) {
    //     printf("%x", buf[i]);
    // }
}

// 等待硬盘操作完成
void wait4hdReady(u8 chanel)
{
    while (1)
    {
        u8 hdStatus = inByte(chanel ? PORT_HD_SECONDARY_STATUS : PORT_HD_PRIMARY_STATUS);
        if ((hdStatus & HD_STATUS_BSY) == 0)
        {
            break;
        }
    }
}

void hdCmd(u8 device, u8 chanel, u8 sectorCnt, u32 sectorNo, u32 cmd)
{
    wait4hdReady(chanel);
    outByte(0, chanel ? PORT_HD_SECONDARY_FEATURES : PORT_HD_PRIMARY_FEATURES);
    outByte(0, chanel ? PORT_HD_SECONDARY_CONTROL : PORT_HD_PRIMARY_CONTROL);
    outByte(sectorCnt, chanel ? PORT_HD_SECONDARY_SECTOR_COUNT : PORT_HD_PRIMARY_SECTOR_COUNT);
    outByte(sectorNo & 0xff, chanel ? PORT_HD_SECONDARY_LBA_LOW : PORT_HD_PRIMARY_LBA_LOW);
    outByte((sectorNo >> 8) & 0xff, chanel ? PORT_HD_SECONDARY_LBA_MID : PORT_HD_PRIMARY_LBA_MID);
    outByte((sectorNo >> 16) & 0xff, chanel ? PORT_HD_SECONDARY_LBA_HIGH : PORT_HD_PRIMARY_LBA_HIGH);
    outByte(MAKE_DEVICE(1, device, (sectorNo >> 24) & 0x0F), chanel ? PORT_HD_SECONDARY_DEVICE : PORT_HD_PRIMARY_DEVICE);
    outByte(cmd, chanel ? PORT_HD_SECONDARY_COMMAND : PORT_HD_PRIMARY_COMMAND);
}

// 像硬盘发送identity命令，获取硬盘参数，保存到buf中，数据容量为256个word
void identityHd(u8 device, u8 chanel, u16 *buf)
{
    // 像硬盘发生IDENTIFY命令，获取硬盘参数
    hdCmd(device, chanel, 0, 0, HD_CMD_IDENTIFY);
    recvIntMsg(); // 等待硬盘中断发生
    readPort(chanel ? PORT_HD_SECONDARY_DATA : PORT_HD_PRIMARY_DATA, buf, SECTOR_SIZE / 2);
}

void printHdIdentityInfo(u8 device, u8 chanel)
{
    u16 buf[256];
    identityHd(device, chanel, buf);

    printf("Sector Count low: %x , ", buf[60]);
    printf("Sector Count hight: %x \n", buf[61]);
    printf("Capability: %x , ", buf[49]); // bit9=1，表示支持LBA
    printf("Is LBA Surported? %c \n", (buf[49] & 0x200) ? 'T' : 'F');
    printf("Cmd Set: %x , ", buf[83]); // bit10=1，表示支持LBA48
    printf("Is LBA48 Surported? %c \n", (buf[83] & 0x400) ? 'T' : 'F');

    int i;
    char s[48] = "";
    char *p = (char *)&buf[10];
    for (i = 0; i < 20; i += 2)
    {
        s[i + 1] = *p++;
        s[i] = *p++;
    }
    s[i] = 0;
    printf("HD SN :%s , ", s);

    p = (char *)&buf[27];
    for (i = 0; i < 40; i += 2)
    {
        s[i + 1] = *p++;
        s[i] = *p++;
    }
    s[i] = 0;
    printf("HD MODEL :%s \n", s);
}

// 从硬盘设备device中读取sectorNo开始的 sectorCnt个sector数据到buf
void readHd(u8 device, u8 chanel, int sectorNo, int sectorCnt, u16 *buf)
{
    hdCmd(device, chanel, sectorCnt, sectorNo, HD_CMD_READ);
    while (sectorCnt--)
    {
        recvIntMsg(); // 等待硬盘中断发生
        readPort(chanel ? PORT_HD_SECONDARY_DATA : PORT_HD_PRIMARY_DATA, buf, SECTOR_SIZE / 2);
        buf += SECTOR_SIZE / 2;
    }
}

void writeHd(u8 device, u8 chanel, int sectorNo, int sectorCnt, u16 *buf)
{
    hdCmd(device, chanel, sectorCnt, sectorNo, HD_CMD_WRITE);
    while (sectorCnt--)
    {
        wait4hdReady(chanel);
        writePort(chanel ? PORT_HD_SECONDARY_DATA : PORT_HD_PRIMARY_DATA, buf, SECTOR_SIZE / 2);
        recvIntMsg(); // 等待硬盘中断发生
        buf += SECTOR_SIZE / 2;
    }
}

void printPartInfo(u8 device, u8 chanel, int sectorNo)
{
    u16 buf[256];
    readHd(device, chanel, sectorNo, 1, buf);
    int idx = 0x1be;
    u8 *p = (u8 *)buf;
    printf("-----%x----\n", sectorNo);
    printf("1");
    printOnePart(device, chanel, &p[idx], sectorNo);
    printf("2");
    printOnePart(device, chanel, &p[idx + 16], sectorNo);
    printf("3");
    printOnePart(device, chanel, &p[idx + 32], sectorNo);
    printf("4");
    printOnePart(device, chanel, &p[idx + 48], sectorNo);
}

void printOnePart(u8 device, u8 chanel, u8 *buf, int lba)
{
    if (buf[4] == 0)
    {
        return;
    }
    printf("--Bootable:%c, ", buf[0] == 0x80 ? 'Y' : 'N');
    printf("Start Head:%x, ", buf[1]);
    printf("Start Sector:%x, ", buf[2] & 0x3f);
    printf("Start Cliy:%x, ", buf[3] | ((buf[2] & 0xc0) << 2));
    printf("Type: %x, ", buf[4]);
    printf("End Head:%x, ", buf[5]);
    printf("End Sector: %x, ", buf[6] & 0x3f);
    printf("End Cliy:%x, ", buf[7] | ((buf[6] & 0xc0) << 2));
    u32 startLba = *(u32 *)(&buf[8]);
    printf("StartLBA: %x, ", startLba);
    printf("SectorCount: %x \n", *(u32 *)(&buf[12]));

    if (buf[4] == 5) // partion type is extendeds
    {
        printPartInfo(device, chanel, startLba + lba);
    }
}

void taskHd()
{
    initHd();

    char a[2] = "0";
    while (1)
    {
        write(a, green);
        a[0]++;
        if (a[0] > '5')
        {
            a[0] = '0';
        }
        delayMs(1000);
    }
}
