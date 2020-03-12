BITS 32
[section .bss]  
    resb	2048
StackTop :

EOI                  equ   20h
; 必须与 klibc.c 中定义保持一致
SA_RPL3			     equ   3	 
GDT_SELECTOR_D32     equ   0x08                 ; 数据段 选择子
GDT_SELECTOR_C32     equ   0x10                 ; 代码段 选择子
GDT_SELECTOR_VIDEO   equ   (0x18 | SA_RPL3)     ; 视频段 选择子
GDT_SELECTOR_TSS     equ   20h
PORT_8259A_MASTER1   equ   20h
PORT_8259A_SLAVE1    equ   0A0h   
PORT_8259A_MASTER2   equ   21h
PORT_8259A_SLAVE2    equ   0A1h  

; 与klibc.c 中 PCB结构 中变量位置保持一致
PCB_LDT_SEL   equ   17*4  
PCB_REGS_END  equ   PCB_LDT_SEL
PCB_EAX       equ   7*4

; 与klibc.c 中 TSS结构 中变量位置保持一致
TSS_ESP0      equ   1*4

[section .text]  

global  _start

global  hwint00
global  hwint01
global  hwint02
global  hwint03
global  hwint04
global  hwint05
global  hwint06
global  hwint07
global  hwint08
global  hwint09
global  hwint10
global  hwint11
global  hwint12
global  hwint13
global  hwint14
global  hwint15

global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error

global  int90syscall
global  syscall

extern gdtPtr
extern idtPtr
extern currentPcb
extern tss
extern isInt
extern hwintHandlerTable
extern osinit
extern exceptionHandler
extern sysWrite
extern syscallTable

_start:
    mov     esp, StackTop       ; 重新设置
    ; 重新放置 GDT
    sgdt    [gdtPtr]
    call    osinit
    lgdt    [gdtPtr]
    lidt    [idtPtr]
    mov     ax, GDT_SELECTOR_TSS                
    ltr     ax
    jmp     GDT_SELECTOR_C32: restart  ; 测试GDT是否正确
restart:    ; 进入低特权级，执行进程 
    mov     esp, [currentPcb]           ; 指向即将运行的进程PCB开始处，即寄存器数据开始处
    lldt    [esp + PCB_LDT_SEL]
    lea     eax, [esp + PCB_REGS_END ]   
    mov     [tss + TSS_ESP0], eax   ; TSS.ESP0 指向当前进程寄存器数据末尾处，为中断发生时保存数据做准备
restart_int:
    dec     dword [isInt]
    popad           ; pop语句顺序与klibc.c 中PCB结构定义保持一致
    pop     ds
    pop     es
    pop     fs
    pop     gs  
    iretd
.wait:  
    hlt
    jmp     .wait  

;---------------------------------------------------------
;；----- 8059A 硬件中断处理程序 ---------------------------
%macro hwint_master 1 
    ; 保存进程寄存器数据到PCB, 此时ESP指向PCB中寄存器数据末尾
    push    gs
    push    fs
    push    es
    push    ds
    pushad
    in      al, PORT_8259A_MASTER2
    or      al, 1<<(%1)
    out     PORT_8259A_MASTER2, al   ; 禁用当前中断
    ; SEND EOI 
    mov     al, EOI
    out     PORT_8259A_MASTER1, al 
    ; 判断是否为中断重入     
    inc     dword [isInt]
    cmp     dword [isInt], 0    ; isInt!=0,发生中断重入，内核运行时发生的中断，此时 esp 指向内核堆栈，不能切换进程
    jne     .1                  ; isInt==0,没有中断重入，进程运行时发生的中断，可以进行进程切换
    ; 设置内核寄存器数据     
    mov     esp, StackTop  ; esp 指向内核栈
    mov     ax, ss  
    mov     ds, ax
    mov     es, ax
    mov     fs, ax 
.1:                         
    sti    
    call    [hwintHandlerTable+4*%1]
    cli 
    in      al, PORT_8259A_MASTER2
    and      al, ~(1<<(%1))
    out     PORT_8259A_MASTER2, al   ; 启用当前中断s
    cmp     dword [isInt], 0
    jne     restart_int              
    jmp     restart
%endmacro  
%macro hwint_slave 1
    ; 保存进程寄存器数据到PCB, 此时ESP指向PCB中寄存器数据末尾
    push    gs
    push    fs
    push    es
    push    ds
    pushad
    in      al, PORT_8259A_SLAVE2
    or      al, 1<<(%1 - 8)
    out     PORT_8259A_SLAVE2, al   ; 禁用当前中断
    mov     al, EOI
    out     PORT_8259A_MASTER1, al   ; SEND EOI to master
    nop
    out     PORT_8259A_SLAVE1, al    ; SEND EOI to slave
    ; 判断是否为中断重入     
    inc     dword [isInt]
    cmp     dword [isInt], 0    ; isInt!=0,发生中断重入，内核运行时发生的中断，此时 esp 指向内核堆栈，不能切换进程
    jne     .1                  ; isInt==0,没有中断重入，进程运行时发生的中断，可以进行进程切换
    ; 设置内核寄存器数据     
    mov     esp, StackTop  ; esp 指向内核栈
    mov     ax, ss  
    mov     ds, ax
    mov     es, ax
    mov     fs, ax 
.1:                         
    sti    
    call    [hwintHandlerTable+4*%1]
    cli 
    in      al, PORT_8259A_SLAVE2
    and      al, ~(1<<(%1 - 8))
    out     PORT_8259A_SLAVE2, al   ; 启用当前中断
    cmp     dword [isInt], 0
    jne     restart_int              
    jmp     restart
%endmacro  

hwint00:    hwint_master 0 ; clock
hwint01:    hwint_master 1 ; keyboard
hwint02:    hwint_master 2 ; cascade            
hwint03:    hwint_master 3 ; second serial  串口2
hwint04:    hwint_master 4 ; first serial   串口1
hwint05:    hwint_master 5 ; XT winchester  LPT2
hwint06:    hwint_master 6 ; floppy         
hwint07:    hwint_master 7 ; printer        LPT1

hwint08:    hwint_slave 08 ; 
hwint09:    hwint_slave 09 ;  
hwint10:    hwint_slave 10 ;  
hwint11:    hwint_slave 11 ; 
hwint12:    hwint_slave 12 ; 
hwint13:    hwint_slave 13 ; 
hwint14:    hwint_slave 14 ;  hard disk
hwint15:    hwint_slave 15 ; 

;；-----中断和异常 系统异常处理程序 ---------------------------
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0		; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1		; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2		; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3		; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4		; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5		; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6		; vector_no	= 6
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7		; vector_no	= 7
	jmp	exception
double_fault:
	push	8		; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9		; vector_no	= 9
	jmp	exception
inval_tss:
	push	10		; vector_no	= A
	jmp	exception
segment_not_present:
	push	11		; vector_no	= B
	jmp	exception
stack_exception:
	push	12		; vector_no	= C
	jmp	exception
general_protection:
	push	13		; vector_no	= D
	jmp	exception
page_fault:
	push	14		; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16		; vector_no	= 10h
	jmp	exception
exception:
	call	exceptionHandler
	add	    esp, 4*2	; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	iret

;；-----  int 90h， 系统功能调用 ---------------------------
;; eax  功能号
;; ebx  第一个参数
;; ecx  第二个参数
;; 最后一个参数（第三个）固定为currentPcb
int90syscall: ;  
    ; 保存进程寄存器数据到PCB, 此时ESP指向PCB中寄存器数据末尾
    push    gs
    push    fs
    push    es
    push    ds
    pushad
    lea     esi , [syscallTable+eax*4]
    ; 判断是否为中断重入     
    inc     dword [isInt]
    cmp     dword [isInt], 0   ; isInt!=0, 发生中断重入，内核运行时发生的中断，此时 esp 指向内核堆栈，不能切换进程
    jne     .1                  
    ; 设置内核寄存器数据     
    mov     esp, StackTop  ; esp 指向内核栈
    mov     ax, ss  
    mov     ds, ax
    mov     es, ax
    mov     fs, ax 
.1:                         
    sti    
    push    dword [currentPcb]
    push    ecx
    push    ebx
    call    [esi]
    mov     [currentPcb + PCB_EAX], eax     ; 讲返回值放入PCB的eax中
    add     esp, 3*4
    cli 
    cmp     dword [isInt], 0  
    jne     restart_int 
    jmp     restart

;;; ---------  系统调研功能的实现  ------------------------
INT_VECTOR_SYSCALL  equ     90h
;;---------- main.h -----------------------------------
; void syscall(u32 sysCallIdx ,u32 param1, u32 param2);
syscall:
    push    eax
    push    ebx
    push    ecx
    mov     eax, [esp+12+4]  ;sysCallIdx
    mov     ebx, [esp+12+8]  ; param1
    mov     ecx, [esp+12+12]  ; param2
    int     INT_VECTOR_SYSCALL         
    pop     ecx
    pop     ebx
    pop     eax
    ret