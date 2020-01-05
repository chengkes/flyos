; org 0100h
    jmp LABEL_START
    times 512 db 0
LABEL_START:
    mov ax, 0b800h
    mov es, ax
    mov al, 'B'
    mov ah, 0ch
    mov [es:(80*5+5)*2], ax
    jmp $
