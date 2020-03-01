
#include "tty.h"
#include "interrupt.h"
#include "pcb.h"
#include "hd.h"
#include "lib.h"
#include "clock.h"

// 硬盘中断处理程序
void hdIrqHandler() {
    Message msg; 
    msg.sendPcbIdx = PCB_IDX_INTERRUPT ;
    msg.recvPcbIdx = 4;    // taskHd 在PCB中的索引， 参见pcb.c:void initPcb();
    msg.data = inByte(PORT_HD_PRIMARY_CMD_STATUS); // hard disk status
    // printf("int hd Irq, hd status: %d\n", msg.data);
    sendMsg(&msg); 
}
  
void initHd() {
    write("hello hard disk!\n", white);
    u8* hd = (u8*) 0x475; // 获取硬盘个数， BIOS已经将硬盘个数写入内存0x475处
    printf("hd count: %d \n", *hd);
    
    putIrqHandler(IRQ_IDX_HARDDISK, hdIrqHandler); // 指定硬盘中断处理程序
    // 打开硬盘中断，硬盘中断在从片，所以必须先打开级联 ， 
    // setIrq(IRQ_IDX_CASCADE, 1);
    // setIrq(IRQ_IDX_HARDDISK, 1);

    // 获取硬盘信息并显示 
    while(1) {
        u8 hdStatus = inByte(PORT_HD_PRIMARY_CMD_STATUS);
        printf("hard disk status: %d\n", hdStatus);
        if ((hdStatus & 0x80) == 0) break; 
    }

    outByte(0, PORT_HD_PRIMARY_CONTROL);
    outByte(0xA0, PORT_HD_PRIMARY_CMD_DEVICE);
    outByte(HD_CMD_IDENTIFY, PORT_HD_PRIMARY_CMD_COMMAND);
    Message msg; 
    msg.sendPcbIdx = PCB_IDX_INTERRUPT ;
    msg.recvPcbIdx = 4;    // taskHd 在PCB中的索引， 参见pcb.c:void initPcb();
    receiveMsg(&msg);
    printf("hard disk is ready...\n");
    
    u16 buf[256];
    readPort(PORT_HD_PRIMARY_CMD_DATA, buf, 256);
    printf("SectorCount low: %x \n" , buf[60]);
    printf("SectorCount hight: %x \n" , buf[61]);
    printf("capability: %x \n" , buf[49]);
    printf("cmd set: %x \n" , buf[83]);
    
}

void taskHd() {
    initHd();

    while (1)
    {
        ;
    }
}


