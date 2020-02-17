[section .text] 

global  inByte
global  outByte

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

    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 读端口
; u8 inByte(u16 port)
inByte:
    push    edx
    mov     edx, [esp+8]
    in      al, dx
    nop
    nop
    pop     edx
    ret
