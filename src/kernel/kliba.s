[section .text] 

global inByte
global outByte 
global insWord
global outsWord 

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
    pop     edx
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 以Word形式读端口
; void insWord(u16 port, u16* buf,u32 size)
insWord:
    push    edi
    push    ecx
    push    edx
    mov     edx, [esp+12+4]  ; port
    mov     edi, [esp+12+8]  ; buf
    mov     ecx, [esp+12+12] ; size
    rep     insw
    pop     edx
    pop     ecx
    pop     edi
    ret

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 以Word形式写端口
; void outsWord(u16 port, u16* buf,u32 size)
outsWord:
    push    edi
    push    ecx
    push    edx
    mov     edx, [esp+12+4]  ; port
    mov     esi, [esp+12+8]  ; buf
    mov     ecx, [esp+12+12] ; size
    rep     outsw
    pop     edx
    pop     ecx
    pop     edi
    ret

