org 0100h

mov ax, 0b800h
mov es, ax
mov al, 'B'
mov ah, 0ch
mov [es:(80*5+5)*2], ax
jmp $
