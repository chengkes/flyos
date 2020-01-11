    mov     ax, 0b800h
    mov     es, ax
    mov     al, 'K'
    mov     ah, 0ch
    mov     [es:(80*2+5)*2], ax
    jmp     $