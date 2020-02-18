org 0100h        ;LoaderOffset
LABEL_START:
    mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		sp, LoaderOffset

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

    ; todo , 获取内存信息， setupPagging  

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

%include "fat12.inc"
%include "lib.inc"
    KernelBase          equ 7000h
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
    mov     dx, [KernelBase*10h+kernelOffset+ePhEntSize ]   
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
    jmp     SelectorC32: KenerlEntry        ; 跳转到kernel

;; ELF文件 变量定义
KenerlEntry     equ  0x30400    ; 必须与makefile中一致
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
    