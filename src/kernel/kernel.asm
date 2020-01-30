BITS 32
[section .bss]  
    times 2048 db 0
StackTop :

EOI                  equ   20h
; 必须与 klibc.c 中定义保持一致
SA_RPL3			     equ   3	 
GDT_SELECTOR_D32     equ   0x08                 ; 数据段 选择子
GDT_SELECTOR_C32     equ   0x10                 ; 代码段 选择子
GDT_SELECTOR_VIDEO   equ   (0x18 | SA_RPL3)     ; 视频段 选择子
GDT_SELECTOR_TSS     equ   20h
PORT_8259A_PRIMARY1  equ 20h

; 与klibc.c 中 PCB结构 中变量位置保持一致
PCB_LDT_SEL   equ   17*4  
PCB_REGS_END  equ   PCB_LDT_SEL

; 与klibc.c 中 TSS结构 中变量位置保持一致
TSS_ESP0      equ   1*4

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
extern currentPcb
extern tss

extern showMsg;
extern osinit
extern osmain

_start:
    mov     esp, StackTop       ; 重新设置
    ; 重新放置 GDT
    sgdt    [gdtPtr]
    call    osinit
    lgdt    [gdtPtr]
    lidt    [idtPtr]
    mov     ax, GDT_SELECTOR_TSS                
    ltr     ax
    ; jmp     GDT_SELECTOR_C32: restart  ; 测试GDT是否正确
restart:    ; 进入低特权级，执行进程 
    mov     esp, [currentPcb]           ; 指向即将运行的进程PCB开始处，即寄存器数据开始处
    lldt    [esp + PCB_LDT_SEL]
    lea     eax, [esp + PCB_REGS_END ]   
    mov     [tss + TSS_ESP0], eax   ; TSS.ESP0 指向当前进程寄存器数据末尾处，为中断发生时保存数据做准备
    popad           ; pop语句顺序与klibc.c 中PCB结构定义保持一致
    pop     ds
    pop     es
    pop     fs
    pop     gs  
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
    cli
     ; 保存进程寄存器数据到PCB, 此时ESP指向PCB中寄存器数据末尾
    push    fs
    push    es
    push    ds
    push    gs
    pusha
    ; 设置内核寄存器数据 
    mov     esp, StackTop  ; esp 指向内核栈
    mov     ax, ss  
    mov     ds, ax
    mov     es, ax
    mov     fs, ax 
    ; 执行中断处理程序
    call    showMsg
    inc     byte [gs:2] 
    mov     al, EOI
    out     PORT_8259A_PRIMARY1, al         ; SEND EOI
    ; 进入currentPcb指向的进程    
    jmp     restart

keyboardHandler:
    inc     byte [gs:0] 
    in      al, 60h
    ; SEND EOI
    mov     al, 20h
    out     20h, al      
    iretd

Handler:    
    iretd
  
;；----- 系统异常 中断处理程序 ---------------------------



