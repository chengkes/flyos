#ifndef IPC_H
#define IPC_H

#define ANY_PCB  (PCB_SIZE+1)

#define IPC_MSG_GET_TICKS 1

typedef struct _Message
{
    u32 type;       // 消息类型
    u32 source;     // 消息来源
    u32 dest ;      // 消息目标
    u32 retValue; // 返回值
} Message;

void sendMsg(Message* m);

void receiveMsg(Message* m);

#endif