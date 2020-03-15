
#include "tty.h"
#include "interrupt.h"
#include "pcb.h"
#include "hd.h"
#include "lib.h"
#include "clock.h"
#include "main.h"

// 硬盘中断处理程序
void hdIrqHandler() {
    Message msg; 
    msg.sendPcbIdx = PCB_IDX_INTERRUPT ;
    msg.recvPcbIdx = 4;    // taskHd 在PCB中的索引， 参见pcb.c:void initPcb();
    inByte(PORT_HD_PRIMARY_STATUS); //读取 hard disk status, 使硬盘能继续相应中断
    sendMsg(&msg);   // 发送中断消息 
    printf("hdIrqHandler over...<<<<<<<<<<<<< \n");
}

void initHd() {
    u8* hd = (u8*) 0x475; // 获取硬盘个数， BIOS已经将硬盘个数写入内存0x475处
    printf("hd count: %d \n", *hd);
    
    putIrqHandler(IRQ_IDX_HARDDISK, hdIrqHandler); // 指定硬盘中断处理程序
    
    printHdIdentityInfo(0);   // 获取硬盘信息并显示 
    printPartInfo(0, 0);         // 获取硬盘分区信息
}

// 等待硬盘操作完成
void wait4hdReady() {
    while(1) {
        u8 hdStatus = inByte(PORT_HD_PRIMARY_STATUS);
        if ((hdStatus & 0x80) == 0) break; 
    }
}

// 等待硬盘中断发生
void wait4hdInt(){
    Message msg; 
    msg.sendPcbIdx = PCB_IDX_INTERRUPT ;
    msg.recvPcbIdx = 4;    // taskHd 在PCB中的索引， 参见pcb.c:void initPcb();
    sendRecv(RECV, &msg);
    printf("wait4hdInt over....>>>>>>>>>>>> \n");
}

void hdCmd(u8 sectorCnt, u32 sectorNo, u32 device, u32 cmd){
    wait4hdReady();
    outByte(0, PORT_HD_PRIMARY_FEATURES);
    outByte(0 , PORT_HD_PRIMARY_CONTROL); 
    outByte(sectorCnt & 0xff, PORT_HD_PRIMARY_SECTOR_COUNT);
    outByte(sectorNo & 0xff, PORT_HD_PRIMARY_LBA_LOW);
    outByte((sectorNo>>8) & 0xff, PORT_HD_PRIMARY_LBA_MID);
    outByte((sectorNo>>16) & 0xff, PORT_HD_PRIMARY_LBA_HIGH);
    outByte( MAKE_DEVICE(1, device, (sectorNo>>24) & 0x0F ), PORT_HD_PRIMARY_DEVICE);
    outByte(cmd, PORT_HD_PRIMARY_COMMAND);
    wait4hdInt();
}


// 像硬盘发送identity命令，获取硬盘参数，保存到buf中，数据容量为256个word
void identityHd(int device, u16* buf) {
    // 像硬盘发生IDENTIFY命令，获取硬盘参数
    hdCmd(0, 0, 0, HD_CMD_IDENTIFY); 
    readPort(PORT_HD_PRIMARY_DATA, buf, 256);
}

void printHdIdentityInfo(int device) {
    u16 buf[256];
    identityHd(device, buf);

    printf("Sector Count low: %x , " , buf[60]);
    printf("Sector Count hight: %x \n" , buf[61]);
    printf("Capability: %x , " , buf[49]);   // bit9=1，表示支持LBA
    printf("Is LBA Surported? %c \n",  (buf[49] & 0x200)? 'T': 'F' ); 
    printf("Cmd Set: %x , " , buf[83]);         // bit10=1，表示支持LBA48
    printf("Is LBA48 Surported? %c \n", (buf[83] & 0x400)? 'T': 'F' );  

    int i;
    char s[48]="";
    char* p = (char*)&buf[10];
    for(i=0; i<20; i+=2) {
        s[i+1] =*p++;
        s[i] =*p++;
    }
    s[i] = 0;
    printf("HD SN :%s , ", s); 

    p = (char*)&buf[27];
    for(i=0; i<40; i+=2) {
        s[i+1] =*p++;
        s[i] =*p++;
    }
    s[i] = 0;
    printf("HD MODEL :%s \n", s);
}

// 从硬盘设备device中读取sectorNo开始的 sectorCnt个sector数据到buf
void readHd(int device, int sectorNo, int sectorCnt, u16* buf){

    hdCmd(sectorCnt, sectorNo, device, HD_CMD_READ);
    readPort(PORT_HD_PRIMARY_DATA, buf, sectorCnt*SECTOR_SIZE/2 );
}

void printPartInfo(int device, int sectorNo) {
    u16 buf[256];
    readHd(device, sectorNo, 1, buf);
    int idx = 0x1be;
    u8* p = (u8*)buf;

    printOnePart(&p[idx]);
    printOnePart(&p[idx+16]);
    printOnePart(&p[idx+32]);
    printOnePart(&p[idx+48]);
}

void printOnePart(u8* buf) {
    if (buf[4] == 0) return;
    printf("----Bootable: %c, ", buf[0]== 0x80? 'Y': 'N');
    printf("Start Head: %x, ", buf[1]);
    printf("Start Sector: %x, ", buf[2] & 0x3f);
    printf("Start Cliy: %x, ", buf[3] | ((buf[2] & 0xc0)<<2) );
    printf("Type : %x, ", buf[4]  );
    printf("End Head: %x, ", buf[5]);
    printf("End Sector: %x, ", buf[6] & 0x3f);
    printf("End Cliy: %x, ", buf[7] | ((buf[6] & 0xc0)<<2) );
    u32 startLba = *(u32*)(&buf[8]);
    printf("StartLBA: %x, ", startLba );
    printf("SectorCount: %x \n", *(u32*)(&buf[12]) );

    if ( buf[4]== 5) { // partion type is extended 
        printPartInfo(0, startLba  );
    }
}

void taskHd() {
    initHd();

    char a[2] = "0";
    while(1) {
        write(a, green);
        a[0] ++;
        if (a[0] > '5') a[0] = '0';
        delayMs(1000);
    }
}


