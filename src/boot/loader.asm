org 0100h        ;LoaderOffset
LABEL_START:
    mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		sp, LoaderOffset
    mov     ax, 0b800h
    mov     gs, ax

    ; 从软盘中查找kernel文件 
	push	KernelName
	call	SearchFile
	add		sp, 2
	test	ax, ax
	jz		.not_found
    ; 找到kernel, 加载到内存 
	push	ax
	push	KernelBase
	push	kernelOffset
	call 	LoadFile2Mem
	add		sp, 6

    ; 关闭软驱马达
    mov     dx, 03f2h
    xor     al, al
    out     dx, al

    ; 获取内存信息
    call    getMemoryInfo 
    ; todo: 设置分页 setupPagging  

    ; 跳入保护模式
    lgdt    [GdtPtr]
    cli                         ; 关中断
    in      al, 92h
    or	    al, 00000010b
    out     92h, al             ; 打开地址线A20
    mov     eax, cr0
    or      al, 01h
    mov     cr0, eax
    jmp     dword    SelectorC32: LABEL_PM_START+LoaderBase*10h
   
.not_found:
    mov		bp,	NoKernel
	mov		dx,	0301h       ; 显示位置（dh,dl） = （行，列）
	mov		cx, NoKernelLen	; CX = 串长度
	mov		ax, 01301h		; AH = 13,  AL = 01h
	mov		bx, 000ch		; 页号为0(BH = 0) 黑底红字(BL = 0Ch,高亮)
	int		10h				; 10h 号中断 		
    jmp     $ 

memBuf:  times 512 db 0
memBlockSize    db 0
memHeader: db 'BaseAddr          Length            Type' 
memHeaderLen equ    $ - memHeader

getMemoryInfo:
    mov     di, memBuf
    xor     ebx, ebx
.next:
    mov     edx, 0534D4150h
    mov     eax, 0E820h
    mov     ecx, 20
    int     15h     ; 获取内存大小
    jc      .fail
    cmp     ebx, 0
    je      .ok
    add     edi, ecx
    inc     byte [memBlockSize]
    jmp     .next
.fail:
    mov     byte [memBlockSize], 0
.ok:
    ; mov     al, [memBlockSize]
    ; add     al, '0'    
    ; mov     ah, 0Ch  ; 黑底红字 高亮
    ; mov     [gs:160], ax
    call    showMemHeader
    call    showMemInfo
    ret

; 显示内存信息头部标题
showMemHeader:
    mov		bp,	memHeader
	mov		dx,	0201h       ; 显示位置（dh,dl） = （行，列）
	mov		cx, memHeaderLen	; CX = 串长度
	mov		ax, 01301h		; AH = 13,  AL = 01h
	mov		bx, 000ch		; 页号为0(BH = 0) 黑底红字(BL = 0Ch,高亮)
	int		10h				; 10h 号中断 
    ret

; 显示内存块信息 
showMemInfo:
    mov     cl, [memBlockSize]
    mov     bp, 160*3            ; 显示位置
    mov     bx, memBuf          ; 当前显示字符
    mov     ah, 0ch             ; 颜色
.nextBlock:
    cmp     cl, 0
    je      .done
    call    showMemLine
    add     bp, 160
    add     bx, 20    
    dec     cl
    jmp     .nextBlock
.done:
    ret

; 现实内存块信息中的一行
; bp，显示位置
; bx, 要显示的字节地址
showMemLine:
    push    bp
    push    bx
    add     bx, 4
    call    showDword
    add     bp, 8*2
    sub     bx, 4
    call    showDword
    add     bp, 10*2
    add     bx, 12
    call    showDword
    add     bp, 8*2
    sub     bx, 4
    call    showDword
    add     bp, 10*2
    add     bx, 8
    call    showDword
    pop     bx
    pop     bp
    ret

;以16进制显示一个双字
; bp，显示位置
; bx, 要显示的字节地址
showDword:
    push    bp
    add     bx, 3
    call    showByte
    add     bp, 4
    sub     bx, 1
    call    showByte
    add     bp, 4
    sub     bx, 1
    call    showByte
    add     bp, 4
    sub     bx, 1
    call    showByte
    pop     bp
    ret

;以16进制显示一个字节
; bp，显示位置
; bx, 要显示的字节地址
showByte:
    push    ax
    push    cx
    push    bp 
    mov     al, [bx]
    shr     al, 4
    mov     ah, 0ch
    mov     cl, 2
.next:
    and     al, 0fh
    cmp     al, 10
    jb      .less10
    add     al, 'A' - 10
    jmp     .1
.less10:
    add     al, '0'
.1:    
    mov     [gs:bp] , ax
    add     bp, 2
    mov     al, [bx]
    dec     cl
    cmp     cl, 0
    jnz      .next
    pop     bp
    pop     cx
    pop     ax
    ret

%include "fat12.inc"
%include "lib.inc"
    KernelBase          equ 6000h
    kernelOffset        equ 0
    
    KernelName: 		db 	"KERNEL  BIN"
    NoKernel: 			db 	"NoKernel"
    NoKernelLen         equ  $ - NoKernel
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%include "pm.inc"
[section .gdt]
BITS 32
    LABEL_GDT:             Descriptor      0, 0, 0
    LABEL_DESC_D32:        Descriptor      0, 0fffffh, DA_DRW | DA_32 | DA_LIMIT_4K
    LABEL_DESC_C32:        Descriptor      0, 0fffffh, DA_CR | DA_32 | DA_LIMIT_4K
    LABEL_DESC_VIDEO:      Descriptor      0b8000h, 0ffffh, DA_DRWA | DA_DPL3
    GdtLen      equ $ - LABEL_GDT
    GdtPtr      dw GdtLen - 1            ; GDT界限    
                dd LoaderBase*10h + LABEL_GDT   ; GDT基址  
    SelectorD32         equ   LABEL_DESC_D32 - LABEL_GDT 
    SelectorC32         equ   LABEL_DESC_C32 - LABEL_GDT 
    SelectorVideo       equ   LABEL_DESC_VIDEO - LABEL_GDT + SA_RPL3
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[section .stack32]
LABEL_STACK     times  2048  db 0
StackTop        equ LoaderBase*10h + $    

[section .s32]
BITS 32
LABEL_PM_START: 
    mov     ax, SelectorD32
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     ss, ax
    mov     ax, SelectorVideo
    mov     gs, ax
    mov     esp, StackTop

    ;; 重新放置内核
    mov     ebx, [KernelBase*10h+kernelOffset+ePhOff]     ;Program Header开始地址
    add     ebx, KernelBase*10h+kernelOffset 
    mov     dx, [KernelBase*10h+kernelOffset+ePhEntSize ]    ;Program Header entry 大小
    and     edx, 0ffffh
    mov     cx, [KernelBase*10h+kernelOffset+ePhNum ]     ;Program Header数目
.next_ph:   
    mov     ax, [ebx + pFilesize]       ; 段长度
    push    ax  
    mov     eax, [ebx + pOffset]               ; 段在ELF文件中偏移
    add     eax, KernelBase*10h+kernelOffset
    push    eax  
    mov     eax, [ebx  + pVaddr]           ; 段加载后虚拟地址
    push    eax
    call    _memCpy
    add     esp, 10
    dec     cx
    test    cx, cx      ; 是否复制完ELF中所有段
    jz      .done    
    add     ebx, edx    
    jmp     .next_ph
.done:
    push    SelectorC32
    push    dword [KernelBase*10h+kernelOffset+eEntryOff]   ; 内核入口地址 
    retf                            ;  跳转到kernel, 类似 jmp SelectorC32:KenerlEntry  
       
;; ELF文件 常量定义
eEntryOff       equ  24             ; entry位置 偏移 
ePhOff          equ  28             ; ProgramHader位置 偏移  
ePhEntSize      equ  42             ; 每个ProgramHeader大小 偏移
ePhNum          equ  44             ; ProgramHader数目 偏移

pOffset         equ   4         ; 每个ProgramHader中，段起始位置 偏移
pVaddr          equ   8         ; 每个ProgramHader中，段加载后虚拟地址 偏移
pFilesize       equ   16        ; 每个ProgramHader中，段大小 偏移
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 复制一段内存 void MemCpy(void* toAddr, void* fromAddr, u16 )
;; 参数1： 内存目的地址 
;; 参数2： 内存源地址
;; 参数3： 复制内存长度，以字节为单位
_memCpy:
    push    ebp
    push    edi
    push    esi
    push    cx
    mov     ebp, esp
    add     ebp, 14

    mov     edi, [ebp+4]
    mov     esi, [ebp+8]
    mov     cx, [ebp+12]         ;复制内存长度
.goon:
    test    cx, cx
    jz      .done
    movsb    
    dec     cx
    jmp     .goon
.done:
    pop     cx
    pop     esi
    pop     edi
    pop     ebp
    ret

