
#include "types.h"
#include "tty.h"
#include "pcb.h"
#include "keyboard.h"
#include "main.h"
#include "clock.h"
#include "lib.h"
#include "hd.h"
#include "interrupt.h"
#include "fs.h"

// #define PCB_IDX_A  1
// #define PCB_IDX_B  2
// #define PCB_IDX_C  3
// #define PCB_IDX_TASK_HD  4

// LDT中 选择子
#define LDT_SELECTOR_D32        (0x00 | SA_TIL )   // 数据段 选择子
#define LDT_SELECTOR_C32        (0x08 | SA_TIL )   // 代码段 选择子

static PCB pcbs[PCB_SIZE];       // 所有进程
static int pcbCount;
PCB* currentPcb;        // 当前运行的进程

// 测试进程A , read,write 调用
static void processA(){
    char a[2] = "A";
    while(1) {
        u32 key = readKey();
        if (key!=0 && (key& KEYBOARD_FLAG_EXT)==0) {  // 可打印字符 
            a[0] = 0x7f & key;
            write(a, cyan);
        }
    }
}

// 测试进程B ， 发送消息
static void processB(){
    printf( "currentPCbIdx %d \n", getCurrentPcbIdx());
    // 测试 IPC Send msg
    Message msg;
    msg.type = 1; 
    msg.data = 2; 
    msg.recvPcbIdx = 3; // PROCESSC_IDX
    sendRecv(SEND, &msg);

    char a[2] = "a";
    while(1) {
        write(a, blue);
        a[0] ++;
        if (a[0] > 'z') a[0] = 'a';
        delayMs(1000);
    }
}

// 测试进程C， 接收消息
static void processC(){
    printf( "currentPCbIdx %d \n", getCurrentPcbIdx());
    // 测试 IPC recv msg
    Message msg;
    msg.sendPcbIdx = PCB_IDX_ANY;   // 接收谁的消息 
    sendRecv(RECV, &msg);
    assert(msg.type ==1 && msg.data==2);

    char a[2] = "5";
    while(1) {
        write(a, green);
        a[0] ++;
        if (a[0] > '9') a[0] = '5';
        delayMs(1000);
    }
}

// 测试进程D， 文件系统
static void processD(){
    // getCurrentPcbIdx()
    // test write file
    // char filename[] = "test.txt";
    // u32 fid = fopen(filename);
    // fwrite(fid, "this is a test");
    // fclose(fid);

    // test read file
    // u32 fid = fopen(filename);
    // char buf[512];
    // fread(fid, buf);
    // fclose(fid);

    // test  
    // fexist(filename);

    // test delete
    // fremove(fid);
    while(1){};
}

void sendRecv(u32 type, Message* m) {
    syscall(SYSCALL_IDX_SENDRECV , type, (u32)m);
}

void sysSendRecv(u32 type, Message* m, PCB* p) {
    assert(type==SEND || type==RECV);

    if(type==SEND) {
        sendMsg(m);
    }else if(type==RECV) {
        receiveMsg(m);
    }
}

void initPcb() { 
    pcbCount = 0;
    currentPcb = &pcbs[0];

    putSyscall(SYSCALL_IDX_SENDRECV, sysSendRecv);
    
    addPCB((u32)taskTty, 10, 0, SYS_TASK);     
    addPCB((u32)processA, 3, 0, USER_PROCESS);      // 测试进程
    addPCB((u32)processB, 2, 1, USER_PROCESS);      // 测试进程
    addPCB((u32)processC, 5, 2, USER_PROCESS);      // 测试进程
    addPCB((u32)taskHd, 11, 0, SYS_TASK);
    addPCB((u32)processD, 4, 0, USER_PROCESS);      // 测试进程
    addPCB((u32)taskFs, 12, 0, SYS_TASK);       
    
} 

PCB* getCurrentPcb(){
    return currentPcb;
}

u32 getCurrentPcbIdx() {
    return  currentPcb - (PCB*)pcbs;
}

PCB* getPcbByIdx(u32 idx) {
    return &pcbs[idx];
}

// 进程调度算法
void schedule(){
    // ----------方法1： 循环调用 ---------------
    // currentPcb++;
    // if (currentPcb >= pcbs + PCB_SIZE)  currentPcb=pcbs;

    // ----------方法2： 优先级调度 ---------------
    // currentPcb->state != 0 表示进程被阻塞  
    if(currentPcb->ticks > 0 && currentPcb->state == 0) { // 当前进程时钟周期（ticks）未用完，继续运行
        return;
    }
    int maxTicks = 0;
    while (maxTicks == 0) {
        for (int i = 0; i < pcbCount; i++) { // 选择剩余ticks最大的进程
            if (pcbs[i].ticks > maxTicks && pcbs[i].state == 0) {
                maxTicks = pcbs[i].ticks;
                currentPcb = &pcbs[i];
            }
        }
        if (maxTicks == 0){  // 所有进程ticks都用完，重置ticks为优先级（priority),继续调度
            for (int i=0;i<pcbCount; i++) {
                if (pcbs[i].state == 0) pcbs[i].ticks = pcbs[i].priority;               
            }
        }
    }
}

// 添加新进程
// entry, 进程入口地址
// priority, 进程优先级，越大优先级越高
// ttyIdx, TTY索引
// pt, 进程类型，系统任务还是用户进程
void addPCB(u32 entry, u32 priority, u32 ttyIdx, ProcessType pt) {
    if (pcbCount >= PCB_SIZE) return;
    u32 ldtSel = GDT_SELECTOR_LDT + pcbCount*sizeof(Descriptor);
    PCB *pcb = &pcbs[pcbCount];
    u32 dpl = DA_DPL3;
    u32 rpl = SA_RPL3;
    if (pt == SYS_TASK){    
        dpl = DA_DPL1;
        rpl = SA_RPL1;
    }
    initDescriptor(&pcb->ldt[LDT_SELECTOR_D32 >> 3], 0, 0xfffff, DA_DRW | dpl, DA_LIMIT_4K | DA_32);
    initDescriptor(&pcb->ldt[LDT_SELECTOR_C32 >> 3], 0, 0xfffff, DA_CR | dpl, DA_LIMIT_4K | DA_32);
    initDescriptor(&gdt[ldtSel>>3], (u32)(&pcb->ldt), sizeof(Descriptor)*LDT_SIZE - 1, DA_LDT | dpl, 0);
    pcb->priority = pcb->ticks = priority;
    pcb->ldtSel = ldtSel;
    pcb->cs = LDT_SELECTOR_C32 | rpl;
    pcb->gs =pcb->ss = pcb->ds = pcb->es = pcb->fs = LDT_SELECTOR_D32 | rpl;
    pcb->eflags = 0x1202 ;          // IOPL(12,13)=1, IF(9)=1
    pcb->ptype = pt;
    pcb->entry = pcb->eip = entry;
    pcb->p_esp = (u32)(pcb->pstack+PROCESS_STACK_SIZE);
    pcb->ttyIdx = ttyIdx;

    pcb->state = pcb->intMsgCount = 0;
    pcb->pMsg = 0;
    pcb->recvDeque = 0;

    pcbCount++;
}

// 向进程pid发送中断消息
void sendIntMsgTo(u32 pid) {
    assert(pid>=0 && pid<pcbCount);
    PCB *pDest = getPcbByIdx(pid); 
    if ( pDest->state & PCB_STATE_RECV_INT  ){  // pDest is ready
        pDest->intMsgCount = 0;
        // ublock pDest
        pDest->state &= ~PCB_STATE_RECV_INT;
        assert( pDest->state == 0 );
    }else {  
        pDest->intMsgCount = 1;
    }
}


// 接收中断消息
void recvIntMsg() {
    PCB* p = getCurrentPcb();
    if (p->intMsgCount == 0) { // 没有消息到达，等待
        // block current pcb
         p->state |= PCB_STATE_RECV_INT;
         assert(p->state != 0);
         schedule();
         delayMs(1000 / CLOCK_COUNTER0_HZ + 1); // 等待一个时钟周期，保证进程切换出去
    }else {
        p->intMsgCount = 0;
    }
}

// IPC 发送消息
void sendMsg(Message* m){
    assert(m->recvPcbIdx>=0 && m->recvPcbIdx<pcbCount);

    PCB *pDest = getPcbByIdx(m->recvPcbIdx);
    m->sendPcbIdx = getCurrentPcbIdx();
    assert(m->sendPcbIdx != m->recvPcbIdx);

    // is pDest ready for recving msg ?
    if ( (pDest->state & PCB_STATE_RECVING) && 
            (pDest->pMsg->sendPcbIdx == PCB_IDX_ANY 
            || pDest->pMsg->sendPcbIdx == m->sendPcbIdx) ) { // ready, send msg to pDest
        memCpy ((u8*) pDest->pMsg, (u8*)m, sizeof(Message));
        // unblock pDest for receiving 
        pDest->state &= ~PCB_STATE_RECVING;
        assert(pDest->state == 0);
    }else {   // not ready, add msg to msg-deque 
        PCB *pSelf = getCurrentPcb();  
        pSelf->pMsg = m;
        pSelf->recvDeque = 0;
        PCB*  last = pDest;
        while (last->recvDeque != 0) {
            last = last->recvDeque;
        }
        last->recvDeque = pSelf;

        // block pSrc for Sending
        pSelf->state |= PCB_STATE_SENDING;
        assert(pSelf->state != 0);
        schedule();
        delayMs(1000/CLOCK_COUNTER0_HZ+1); // 等待一个时钟周期，保证进程切换出去 
    }
} 

// IPC 接收消息
void receiveMsg(Message* m){
    m->recvPcbIdx = getCurrentPcbIdx();
    assert(m->sendPcbIdx != m->recvPcbIdx);
    assert( (m->sendPcbIdx>=0 && m->sendPcbIdx<pcbCount) 
        ||  m->sendPcbIdx == PCB_IDX_ANY );

    PCB *pSelf = getCurrentPcb();
    // search the recvDeque 
    PCB* p = pSelf;
    while (p->recvDeque!=0) {
        PCB* current = p->recvDeque;
        if (m->sendPcbIdx == PCB_IDX_ANY || m->sendPcbIdx == current->pMsg->sendPcbIdx) { // 消息队列中有待处理消息
            memCpy((u8*)m, (u8*)current->pMsg, sizeof(Message) );
            // 从消息队列移除
            p->recvDeque = current->recvDeque;
            // unblock the send PCB
            PCB *pFrom = getPcbByIdx(m->sendPcbIdx);
            pFrom->state &= ~ PCB_STATE_SENDING;
            assert(pFrom->state == 0);
            return;
        }else {
            p = p->recvDeque;
        }
    }

    // 没有接收到消息, block本进程，等待消息
    pSelf->pMsg = m;
    pSelf->state |= PCB_STATE_RECVING;
    assert(pSelf->state != 0);
    schedule();
    delayMs(1000/CLOCK_COUNTER0_HZ+1); // 等待一个时钟周期，保证进程切换出去 
}
