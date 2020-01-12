[section .text]  

global  _start

_start:
    mov     al, 'K'
    mov     ah, 0ch
    mov     [gs:(80*2+5)*2], ax
    jmp     $
