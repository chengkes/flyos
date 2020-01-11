; load kernel to memory
LABEL_START:
    mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		sp, StackTop

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

    ; todo , its a test
    mov     ax, 0b800h
    mov     es, ax
    mov     al, 'K'
    mov     ah, 0ch
    mov     [es:(80*2+5)*2], ax

    ; 跳入保护模式
    

    jmp     $
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
    StackTop            equ 9fffh
    KernelName: 		db 	"KERNEL  O  "
    NoKernel: 			db 	"NoKernel"
    NoKernelLen         equ  $ - NoKernel

%include "pm.inc"
[section .gdt]
    LABEL_GDT:             Descriptor      0, 0, 0
    LABEL_DESC_D32:        Descriptor      0, 0fffffh, DA_DRW | DA_32 | DA_LIMIT_4K
    LABEL_DESC_C32:        Descriptor      0, 0fffffh, DA_CR | DA_32 | DA_LIMIT_4K
    LABEL_DESC_VIDEO:      Descriptor      0b8000h, 0ffffh, DA_DRWA
    GdtPtr      dw $ - LABEL_GDT - 1            ; GDT界限    
                dd LoaderBase*10h + LABEL_GDT   ; GDT基址  
    SelectorD32         equ   LABEL_DESC_D32 - LABEL_GDT 
    SelectorC32         equ   LABEL_DESC_C32 - LABEL_GDT 
    SelectorVideo       equ   LABEL_DESC_VIDEO - LABEL_GDT + SA_RPL3
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[section .s32]

    jmp $

