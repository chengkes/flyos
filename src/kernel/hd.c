
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
    printf("hd send interrupt msg over \n");
}
  
void initHd() {
    printf("hello hard disk!\n");
    u8* hd = (u8*) 0x475; // 获取硬盘个数， BIOS已经将硬盘个数写入内存0x475处
    printf("hd count: %d \n", *hd);
    
    putIrqHandler(IRQ_IDX_HARDDISK, hdIrqHandler); // 指定硬盘中断处理程序
    
    printHdIdentityInfo(0);   // 获取硬盘信息并显示 
    printPartInfo(0);         // 获取硬盘分区信息
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
    receiveMsg(&msg);                   // 收到硬盘中断消息，表明命令执行完毕
}

// 像硬盘发送identity命令，获取硬盘参数，保存到buf中，数据容量为256个word
void identityHd(int device, u16* buf) {
    wait4hdReady();    
    
    // 像硬盘发生IDENTIFY命令，获取硬盘参数
    outByte(0, PORT_HD_PRIMARY_CONTROL);   // let the Interrupt Enable
    outByte(MAKE_DEVICE(0, device, 0), PORT_HD_PRIMARY_DEVICE);
    outByte(HD_CMD_IDENTIFY, PORT_HD_PRIMARY_COMMAND);

    wait4hdInt();
    readPort(PORT_HD_PRIMARY_DATA, buf, 256);
}

void printHdIdentityInfo(int device) {
    u16 buf[256];
    identityHd(device, buf);

    printf("Sector Count low: %x \n" , buf[60]);
    printf("Sector Count hight: %x \n" , buf[61]);
    printf("Capability: %x \n" , buf[49]);   // bit9=1，表示支持LBA
    printf("Is LBA Surported? %c \n",  (buf[49] & 0x200)? 'T': 'F' ); 
    printf("Cmd Set: %x \n" , buf[83]);         // bit10=1，表示支持LBA48
    printf("Is LBA48 Surported? %c \n", (buf[83] & 0x400)? 'T': 'F' );  

    int i;
    char s[48]="";
    char* p = (char*)&buf[10];
    for(i=0; i<20; i+=2) {
        s[i+1] =*p++;
        s[i] =*p++;
    }
    s[i] = 0;
    printf("HD SN :%s \n", s); 

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
    wait4hdReady();   
    printf("wait4hdReady over \n");

    outByte(0, PORT_HD_PRIMARY_FEATURES);
    outByte(0 , PORT_HD_PRIMARY_CONTROL); 
    outByte(sectorCnt & 0xff, PORT_HD_PRIMARY_SECTOR_COUNT);
    outByte(sectorNo & 0xff, PORT_HD_PRIMARY_LBA_LOW);
    outByte((sectorNo>>8) & 0xff, PORT_HD_PRIMARY_LBA_MID);
    outByte((sectorNo>>16) & 0xff, PORT_HD_PRIMARY_LBA_HIGH);
    outByte( MAKE_DEVICE(1, device, (sectorNo>>24) & 0x0F ), PORT_HD_PRIMARY_DEVICE);
    outByte(HD_CMD_READ, PORT_HD_PRIMARY_COMMAND);
    wait4hdInt();

    printf("wait4hdInt over \n");
    readPort(PORT_HD_PRIMARY_DATA, buf, sectorCnt*SECTOR_SIZE/2 );
}

void printPartInfo(int device) {
    u16 buf[256];
    readHd(0, 0, 1, buf);
    int idx = 0x1be;
    u8* p = (u8*)buf;
    printf("%x %x %x \n", p[idx], p[idx+1], p[idx+2]); 
    
}

void taskHd() {
    initHd();

    while (1)
    {
        ;
    }
}


