BITS 32
[section .bss]  
    times 2048 db 0
StackTop :

PCB_LDT_SEL   equ   18*4                 
PCB_SS        equ   15*4         
PCB_P_ESP     equ   16*4          
PCB_EFLAGS    equ   14*4             
PCB_CS        equ   13*4         
PCB_EIP       equ   12*4 

TSS_ESP0      equ   4

[section .text]  

global  _start
global  dispChar
global  inByte
global  outByte
global  clockHandler
global  Handler
global  keyboardHandler

extern gdtPtr
extern idtPtr
extern dispPos  
extern pcbs
extern tss

extern osinit
extern osmain

_start:
    mov     esp, StackTop       ; 重新设置

    ; 重新放置 GDT
    sgdt    [gdtPtr]
    call    osinit
    lgdt    [gdtPtr]

    lidt    [idtPtr]
    ; jmp     10h: osstart  ; 测试GDT是否正确
; osstart:
    ; mov     al, 'A'
    ; mov     ah, 0ch
    ; mov     [gs:160*1+2*1], ax   

    ; sti

    ; ring0 -> ring1 进入ring3并调用进程
    mov     eax, [pcbs + PCB_LDT_SEL]
    lldt    ax 
    mov     ax, 020h                ; 参见klibc.c 中 GDT_SELECTOR_TSS
    ltr     ax
    push    dword [pcbs + PCB_SS]       ; ss入栈
    push    dword [pcbs + PCB_P_ESP]    ; esp入栈
    push    dword [pcbs + PCB_EFLAGS]   ; eflags入栈
    push    dword [pcbs + PCB_CS]       ; cs入栈,  
    push    dword [pcbs + PCB_EIP]      ; eip入栈
    iretd

.wait:
    hlt
    jmp     .wait  

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 显示字符
; void dispChar(char c, u8 color)
dispChar:
    push    eax
    push    ebx

    mov     al, [esp+8+4]
    mov     ah, [esp+8+8]
    mov     ebx, [dispPos]
    mov     [gs:ebx], ax
    add     ebx, 2
    mov     [dispPos], ebx

    pop     ebx
    pop     eax
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 读端口
; u8 inByte(u16 port)
inByte:
    push    eax
    push    edx

    mov     edx, [esp+8+4]
    in      al, dx

    pop     edx
    pop     eax
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 写端口
; void outByte(u8 data, u16 port)
outByte:
    push    eax
    push    edx

    mov     al, [esp+8+4]
    mov     edx, [esp+8+8]
    out     dx, al
    nop

    pop     edx
    pop     eax
    ret    

;；----- 8059A 硬件中断处理程序 ---------------------------
clockHandler:        
    push    eax
    inc     byte [gs:2]     
    mov     al, 20h
    out     20h, al      ; SEND EOI
    pop     eax  
        
    ; xor     eax, eax    
    ; mov     ax, cs
    ; push    eax
    ; push    processA
    ; retf
    ; mov  dword   [ss:esp] , processA
    iretd

keyboardHandler:
    push    eax
    inc     byte [gs:0] 
    in      al, 60h
    mov     al, 20h
    out     20h, al      ; SEND EOI
    pop     eax
    iretd

Handler:    
    iretd
 
;；----- 系统异常 中断处理程序 ---------------------------



