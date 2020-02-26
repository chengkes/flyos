
#include "pcb.h"
#include "ipc.h"



// int isPcbReady4ReceiveMsg(u32 pid) {
//     PCB *p = getPcbById(pid);
//     return  p->state == 0;
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

// void sendMsg(Message* m){
//     if ( isPcbReady4ReceiveMsg(m->dest)) {
//         // memcpy (m, m->dest -> msg)
//         // unblock(m->dest)
//     }else {
//         // add msg to pcb[m->dest]'s msg-deque
//     }


//    // blockPcb (m->source) ;
// }

// void receiveMsg(Message* m){
//     // if 
// }

// int sendReceive(Message* m){

// }