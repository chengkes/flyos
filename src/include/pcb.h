#ifndef PCB_H
#define PCB_H

#include "types.h"

typedef struct _Message {
    u32 sendPcbIdx;        // 消息发送方进程idx
    u32 recvPcbIdx;       // 消息接收方进程idx
    u32 type;       // 消息类型
    u32 data;       // 发送数据
} Message;

#define PCB_STATE_SENDING 0x02
#define PCB_STATE_RECVING 0x04

#define PCB_IDX_INTERRUPT   (PCB_SIZE+1)
#define PCB_IDX_ANY         (PCB_SIZE+2)

#define IPC_MSG_GET_TICKS 1

#define PCB_SIZE 0x7f             // 进程控制块 大小

typedef enum _ProcessType {
    USER_PROCESS,
    SYS_TASK
} ProcessType;

#define LDT_SIZE 2          // LDT 大小
#define PROCESS_STACK_SIZE 0x8000           // 进程堆栈 大小
// 进程控制块
typedef struct _PCB {
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;    // 第7个

    u32 ds;
    u32 es;
    u32 fs;
    u32 gs;

    u32 eip;
    u32 cs;
    u32 eflags;
    u32 p_esp;
    u32 ss;
 
    u16  ldtSel;                        //第17个，此处至开头的数据位置应保持不变
    Descriptor ldt[LDT_SIZE];
    u32 entry;                          // 进程入口
    u32 priority;                       // 优先级
    int ticks;
    char name[16];                      // 进程名称
    u8 pstack[PROCESS_STACK_SIZE];      // 进程堆栈
    u32 ttyIdx;                         // TTY索引
    ProcessType ptype;                  // 系统任务 or 用户进程

    Message* pMsg;  // 指向本进程要发送或接收的消息
    u32 state;      // 进程状态， state == 0表示在运行
    u32 intMsgCount; // 待接收中断消息个数
    struct _PCB* recvDeque; // 等待本进程接收的消息队列
} PCB ;

void addPCB(u32 entry, u32 priority,u32 ttyIdx, ProcessType ptype);
void initPcb();
PCB* getCurrentPcb();
void schedule();
PCB* getPcbByIdx(u32 idx);
void receiveMsg(Message*);
void sendMsg(Message*);

#endif
