org	07c00h			; 告诉编译器程序加载到7c00处

	; 下面是 FAT12 磁盘的头
	jmp LABEL_START
	nop
	
	%include "fat12.inc"

LABEL_START:	
	mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		sp, 07c00h

	; 清屏
	mov		al, 0
	mov		cx, 0					; 窗口的左上角位置(Y坐标，X坐标)
	mov		dx, 4080h				; 窗口的右下角位置(Y坐标，X坐标)
	mov		ah, 06h
	int		10h

	; 复位软驱
	xor 	ah, ah
	xor 	dl, dl                  ;驱动器号A盘
	int 	13h

	; 从软盘中查找文件 
	push	LoaderName
	call	SearchFile
	add		sp, 2
	test	ax, ax
	jz		.not_found
	; 找到loader文件, 加载到内存 
	push	ax
	push	LoaderBase
	push	LoaderOffset
	call 	LoadFile2Mem
	add		sp, 6
	jmp		LoaderBase:	LoaderOffset			; 跳转到loader

.not_found:
	mov		bp,	NoLoader
	mov		dx,	0201h		; 显示位置（dh,dl） = （行，列）
	mov		cx, NoLoaderLen	; CX = 串长度
	mov		ax, 01301h		; AH = 13,  AL = 01h
	mov		bx, 000ch		; 页号为0(BH = 0) 黑底红字(BL = 0Ch,高亮)
	int		10h				; 10h 号中断 			
	jmp		$				; 无限循环
 
%include "lib.inc"
	
LoaderName: 		db 	"LOADER  BIN"
NoLoader: 			db 	"NoLoader"
NoLoaderLen         equ  $ - NoLoader
times 	510-($-$$)	db	0	; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 	0xaa55					; 结束标志
