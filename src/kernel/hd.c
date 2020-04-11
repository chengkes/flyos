
#include "tty.h"
#include "interrupt.h"
#include "pcb.h"
#include "hd.h"
#include "lib.h"
#include "clock.h"
#include "main.h"
#include "types.h"

static u8 hdStatus[2]; // 支持2个硬盘通道

static void printHdInfo(HdInfo *p);
static void getHdInfo(HdInfo *p);
static void getOnePartInfo(u8 *buf, int lba, HdInfo *p);
static void getPartInfo(int sectorNo, HdInfo *p);

static int _readHd(u8 device, u8 chanel, u32 sectorNo, u8 sectorCnt, u16 *buf);

// 硬盘ATA0,中断处理程序 int14
static void hdIrqHandler0()
{
    hdStatus[0] = inByte(PORT_HD_PRIMARY_STATUS); //读取 hard disk status, 使硬盘能继续相应中断sen
    sendIntMsgTo(PCB_IDX_HD);                     // taskHd 在PCB中的索引， 参见pcb.c:void initPcb();
}

// 硬盘ATA1,中断处理程序 int15
static void hdIrqHandler1()
{
    hdStatus[1] = inByte(PORT_HD_SECONDARY_STATUS); //读取 hard disk status, 使硬盘能继续相应中断sen
    sendIntMsgTo(PCB_IDX_HD);                       // taskHd 在PCB中的索引， 参见pcb.c:void initPcb();
}

// 识别硬盘， 返回硬盘个数
int identifyHd(HdInfo *hd)
{
    Message msg;
    msg.recvPcbIdx = PCB_IDX_HD;
    msg.data = (u32)hd; // 硬盘详细信息由HD task直接写入地址hd
    msg.type = HD_CMD_IDENTIFY;
    sendRecv(SEND, &msg);

    msg.sendPcbIdx = PCB_IDX_HD;
    sendRecv(RECV, &msg); // 等待hd task处理完毕
    return msg.data;      // 硬盘个数
}

static int _identifyHd(OUT HdInfo *allHd)
{
    u8 hdCount = *((u8 *)0x475); // 硬盘个数， BIOS已经将硬盘个数写入内存0x475处
    printf("hd count: %d \n", hdCount);
    for (int i = 0; i < hdCount; ++i)
    {
        HdInfo *p = &allHd[i];
        p->device = i & 1;
        p->chanel = i >> 1;
        p->partCnt = 0;
        getHdInfo(p);
        getPartInfo(0, p);
        printHdInfo(p); // 打印硬盘信息
    }

    // test writeHd and readHd, use shell command xxd to verify
    // u16 buf[SECTOR_SIZE];
    // memSet((u8 *)buf, 0, sizeof(buf));
    // // _writeHd(0, 0, 1, 1,  buf);

    // _readHd(0, 0, 1, 2, buf);
    // for(int i=0;i<sizeof(buf)/2; i++) {
    //     printf("%x", buf[i]);
    //     if (i %16==0) printf("\n");
    // }

    // printf("read over...................");
    return hdCount;
}

// 等待硬盘状态
static void wait4hdStatus(u8 chanel, u8 status, u8 value)
{
    while (1)
    {
        u8 hdStatus = inByte(chanel ? PORT_HD_SECONDARY_STATUS : PORT_HD_PRIMARY_STATUS);
        if ((hdStatus & status) == value)
        {
            break;
        }
    }
}

static void hdCmd(u8 device, u8 chanel, u8 sectorCnt, u32 sectorNo, u32 cmd)
{
    wait4hdStatus(chanel, HD_STATUS_BSY, 0);
    outByte(0, chanel ? PORT_HD_SECONDARY_FEATURES : PORT_HD_PRIMARY_FEATURES);
    outByte(0, chanel ? PORT_HD_SECONDARY_CONTROL : PORT_HD_PRIMARY_CONTROL);
    outByte(sectorCnt, chanel ? PORT_HD_SECONDARY_SECTOR_COUNT : PORT_HD_PRIMARY_SECTOR_COUNT);
    outByte(sectorNo & 0xff, chanel ? PORT_HD_SECONDARY_LBA_LOW : PORT_HD_PRIMARY_LBA_LOW);
    outByte((sectorNo >> 8) & 0xff, chanel ? PORT_HD_SECONDARY_LBA_MID : PORT_HD_PRIMARY_LBA_MID);
    outByte((sectorNo >> 16) & 0xff, chanel ? PORT_HD_SECONDARY_LBA_HIGH : PORT_HD_PRIMARY_LBA_HIGH);
    outByte(MAKE_DEVICE(1, device, (sectorNo >> 24) & 0x0F), chanel ? PORT_HD_SECONDARY_DEVICE : PORT_HD_PRIMARY_DEVICE);
    outByte(cmd, chanel ? PORT_HD_SECONDARY_COMMAND : PORT_HD_PRIMARY_COMMAND);
}

// 获取硬盘信息
static void getHdInfo(HdInfo *p)
{
    // 像硬盘发生IDENTIFY命令，获取硬盘参数
    hdCmd(p->device, p->chanel, 0, 0, HD_CMD_IDENTIFY);
    recvIntMsg(); // 等待硬盘中断发生
    u16 buf[256];
    insWord(p->chanel ? PORT_HD_SECONDARY_DATA : PORT_HD_PRIMARY_DATA, buf, SECTOR_SIZE / 2);

    p->sectorCnt = (buf[61] << 16) | buf[60];
    p->capability = buf[49]; // bit9=1，表示支持LBAs
    p->cmdSet = buf[83];     // bit10=1，表示支持LBA48

    int i;
    char *s = p->sn;
    char *b = (char *)&buf[10];
    for (i = 0; i < 20; i += 2)
    {
        s[i + 1] = *b++;
        s[i] = *b++;
    }
    s[i] = 0;

    s = p->model;
    b = (char *)&buf[27];
    for (i = 0; i < 40; i += 2)
    {
        s[i + 1] = *b++;
        s[i] = *b++;
    }
    s[i] = 0;
}

static void printHdInfo(HdInfo *p)
{
    printf("Sector Count : %x , ", p->sectorCnt);
    printf("Capability: %x , ", p->capability); // bit9=1，表示支持LBA
    printf("Is LBA Surported? %c \n", (p->capability & 0x200) ? 'T' : 'F');
    printf("Cmd Set: %x , ", p->cmdSet); // bit10=1，表示支持LBA48
    printf("Is LBA48 Surported? %c \n", (p->cmdSet & 0x400) ? 'T' : 'F');
    printf("HD SN :%s , ", p->sn);
    printf("HD MODEL :%s \n", p->model);

    printf("part count: %d \n", p->partCnt);
    for (int i = 0; i < p->partCnt; ++i)
    {
        PartInfo *pt = &p->partInfo[i];
        printf("%d -- boot:%x, start: %d, count:%d, type:%x \n", i, pt->isBootable, pt->startSector, pt->sectorCnt, pt->type);
    }
}

// 从硬盘hd中第sector个扇区开始，读取sectorCnt个扇区到buf中
int readHd(u8 device, u8 chanel, u32 sectorNo, u8 sectorCnt, void *buf)
{
    Message msg;
    msg.recvPcbIdx = PCB_IDX_HD;
    msg.type = HD_CMD_READ;
    msg.data = (u32)buf;
    msg.param1 = sectorNo;
    msg.param2 = (sectorCnt<<16) & (device<<8) & chanel; 
    sendRecv(SEND, &msg);

    msg.sendPcbIdx = PCB_IDX_HD;
    sendRecv(RECV, &msg);

    return 0;
}

// 从硬盘设备device中读取sectorNo开始的 sectorCnt个sector数据到buf
static int _readHd(u8 device, u8 chanel, u32 sectorNo, u8 sectorCnt, u16 *buf)
{
    hdCmd(device, chanel, sectorCnt, sectorNo, HD_CMD_READ);
    recvIntMsg(); // 等待硬盘中断发生
    insWord(chanel ? PORT_HD_SECONDARY_DATA : PORT_HD_PRIMARY_DATA, buf, sectorCnt * SECTOR_SIZE / 2);

    return 0;
}

// 将buf中数据写入硬盘hd中第sectorNo个扇区开始的sectorCnt个扇区中
int writeHd(u8 device, u8 chanel, u32 sectorNo, u8 sectorCnt, void *buf)
{
    Message msg;
    msg.recvPcbIdx = PCB_IDX_HD;
    msg.type = HD_CMD_WRITE;
    msg.data = (u32)buf;
    msg.param1 = sectorNo;
    msg.param2 = (sectorCnt<<16) & (device<<8) & chanel; 
    sendRecv(SEND, &msg);

    msg.sendPcbIdx = PCB_IDX_HD;
    sendRecv(RECV, &msg);

    return 0;
}
// 向硬盘中第sectorNo个扇区开始，写入size个字节
void _writeHd(u8 device, u8 chanel, int sectorNo, u8 sectorCnt, u16 *buf)
{
    hdCmd(device, chanel, sectorCnt, sectorNo, HD_CMD_WRITE);
    wait4hdStatus(chanel, HD_STATUS_DRQ, HD_STATUS_DRQ);
    outsWord(chanel ? PORT_HD_SECONDARY_DATA : PORT_HD_PRIMARY_DATA, buf, sectorCnt * SECTOR_SIZE / 2);
    recvIntMsg(); // 等待硬盘中断发生
}

// 获取硬盘分区信息
static void getPartInfo(int sectorNo, HdInfo *p)
{
    u16 buf[256];
    _readHd(p->device, p->chanel, sectorNo, 1, buf);
    int idx = 0x1be;
    u8 *b = (u8 *)buf;

    getOnePartInfo(&b[idx], sectorNo, p);
    getOnePartInfo(&b[idx + 16], sectorNo, p);
    getOnePartInfo(&b[idx + 32], sectorNo, p);
    getOnePartInfo(&b[idx + 48], sectorNo, p);
}

static void getOnePartInfo(u8 *buf, int lba, HdInfo *p)
{
    if (buf[4] == 0)
    {
        return;
    }
    assert(p->partCnt < MAX_PART_COUNT);
    PartInfo *pt = &(p->partInfo[(p->partCnt)++]);
    pt->isBootable = buf[0];
    pt->type = buf[4];
    pt->startSector = *(u32 *)(&buf[8]) + lba;
    pt->sectorCnt = *(u32 *)(&buf[12]);
    if (buf[4] == 5) // partion type is extendeds
    {
        getPartInfo(pt->startSector, p);
    }

    // printf("Start Head:%x, ", buf[1]);
    // printf("Start Sector:%x, ", buf[2] & 0x3f);
    // printf("Start Cliy:%x, ", buf[3] | ((buf[2] & 0xc0) << 2));
    // printf("End Head:%x, ", buf[5]);
    // printf("End Sector: %x, ", buf[6] & 0x3f);
    // printf("End Cliy:%x, ", buf[7] | ((buf[6] & 0xc0) << 2));
}

void taskHd()
{
    putIrqHandler(IRQ_IDX_HARDDISK0, hdIrqHandler0); // 指定硬盘中断处理程序
    putIrqHandler(IRQ_IDX_HARDDISK1, hdIrqHandler1); // 指定硬盘中断处理程序

    char a[2] = "0";
    while (1)
    {
        Message msg;
        msg.sendPcbIdx = PCB_IDX_ANY;
        sendRecv(RECV, &msg);

        Message r;
        r.recvPcbIdx = msg.sendPcbIdx;
        if (msg.type == HD_CMD_IDENTIFY)
        {
            HdInfo* hd = (HdInfo *)msg.data;
            r.data = _identifyHd(hd);
            sendRecv(SEND, &r);
        }
        else if (msg.type == HD_CMD_READ)
        { 
            u8  sectorCnt = (msg.param2 >> 16) & 0xff;
            u8  device =  (msg.param2 >> 8) & 0xff;
            u8  chanel = msg.param2  & 0xff; 
            r.data = _readHd(device, chanel, msg.param1, sectorCnt, (u16 *)msg.data);
            sendRecv(SEND, &r);
        }
        else if (msg.type == HD_CMD_WRITE)
        {
            u8  sectorCnt = (msg.param2 >> 16) & 0xff;
            u8  device =  (msg.param2 >> 8) & 0xff;
            u8  chanel = msg.param2  & 0xff; 
            _writeHd(device, chanel, msg.param1,sectorCnt, (u16 *)msg.data);
            sendRecv(SEND, &r); 
        }

        write(a, green);
        a[0]++;
        if (a[0] > '5')
        {
            a[0] = '0';
        }
        delayMs(1000);
    }
}
