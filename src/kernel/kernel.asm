[section .bss]  
    times 2048 db 0
StackTop :

[section .text]  

global  _start

extern gdtPtr
extern mvGdt

_start:
    mov     esp, StackTop       ; 重新设置

    ; 重新放置 GDT
    sgdt    [gdtPtr]
    call    mvGdt
    lgdt    [gdtPtr]
    
    mov     al, 'K'
    mov     ah, 0ch
    mov     [gs:(80*2+5)*2], ax
    jmp     $
    

