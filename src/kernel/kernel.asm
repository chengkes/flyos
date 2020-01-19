[section .bss]  
    times 2048 db 0
StackTop :



[section .text]  

global  _start
global  dispChar
global  inByte
global  outByte

extern gdtPtr
extern mvGdt
extern osmain

_start:
    mov     esp, StackTop       ; 重新设置

    ; 重新放置 GDT
    sgdt    [gdtPtr]
    call    mvGdt
    lgdt    [gdtPtr]

    call    osmain

    ; todo: build idt

    push    160*2+2*2
    push    0ch
    push    'H'
    call    dispChar
    add     esp, 12
    hlt
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 显示字符
; void dispChar(char c, u8 color, u16 loc)
dispChar:
    mov     al, [esp+4]
    mov     ah, [esp+8]
    mov     di, [esp+12]
    mov     [gs:di], ax
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 读端口
; u8 inByte(u16 port)
inByte:
    mov     dx, [esp+4]
    in      al, dx
    nop
    nop
    nop
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 写端口
; void outByte(u8 data, u16 port)
outByte:
    mov     al, [esp+4]
    mov     dx, [esp+8]
    out     dx, al
    nop
    nop
    nop
    ret    
