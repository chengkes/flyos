
#include "pcb.h"
#include "ipc.h"
#include "lib.h"

// int isPcbReady4Receive(u32 idx) {
//     PCB *p = getPcbByIdx(idx);
//     return  p->state == 0;  // todo: thingking
// }

// void unblckPcb(u32 pid) {
//     PCB *p = getPcbById(pid);
//     p->state = 0;
// }

// void blockPcb(u32 state, u32 pid) {
//     PCB *p = getPcbById(pid);
//     p->state = state;
//     schedule();
// }

void sendMsg(Message* m){
    PCB* pDest = getPcbByIdx(m->dest);
    PCB* pSrc = getPcbByIdx(m->source);

    // is pDest ready for recving msg
    if ( pDest->state==0 || pDest->state == PCB_STATE_RECVING ) { // YES, send msg to pDest
        memCpy ((u8*) &pDest->msg, (u8*)m, sizeof(Message));
        // unblock pDest for receiving 
        pDest->state &= ~PCB_STATE_RECVING;
    }else {   // NO, add msg to msg-deque, todo : thinking
        MessageDeque *next = pDest->msgRecvDeque;
        while (next != 0) {
            next = next->next;
        }
        next->data = m;
        next->next = 0;
    }
          
    // block pSrc for Sending
    pSrc->state |= PCB_STATE_SENDING ; 
    schedule();
}

// void receiveMsg(Message* m){
//     // if 
// }

// int sendReceive(Message* m){

// }
