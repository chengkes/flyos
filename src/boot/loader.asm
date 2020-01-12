org LoaderOffset
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
    KernelBase          equ 3000h
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
    mov     esp, StackTop

    mov     ax, SelectorVideo
    mov     gs, ax
    mov	    ah, 0Ch				; 0000: 黑底    1100: 红字
	mov	    al, 'P'
    mov     [gs:(80*5+9)*2] , ax
    jmp     $
