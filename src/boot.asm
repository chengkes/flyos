org	07c00h			; 告诉编译器程序加载到7c00处

		; 下面是 FAT12 磁盘的头
	jmp LABEL_START
	nop
	BS_OEMName	DB 'ForrestY'	; OEM String, 必须 8 个字节
	BPB_BytsPerSec	DW 512		; 每扇区字节数
	BPB_SecPerClus	DB 1		; 每簇多少扇区
	BPB_RsvdSecCnt	DW 1		; Boot 记录占用多少扇区
	BPB_NumFATs	DB 2		; 共有多少 FAT 表
	BPB_RootEntCnt	DW 224		; 根目录文件数最大值
	BPB_TotSec16	DW 2880		; 逻辑扇区总数
	BPB_Media	DB 0xF0		; 媒体描述符
	BPB_FATSz16	DW 9		; 每FAT扇区数
	BPB_SecPerTrk	DW 18		; 每磁道扇区数
	BPB_NumHeads	DW 2		; 磁头数(面数)
	BPB_HiddSec	DD 0		; 隐藏扇区数
	BPB_TotSec32	DD 0		; 如果 wTotalSectorCount 是 0 由这个值记录扇区数
	BS_DrvNum	DB 0		; 中断 13 的驱动器号
	BS_Reserved1	DB 0		; 未使用
	BS_BootSig	DB 29h		; 扩展引导标记 (29h)
	BS_VolID	DD 0		; 卷序列号
	BS_VolLab	DB 'Tinix0.01  '; 卷标, 必须 11 个字节
	BS_FileSysType	DB 'FAT12   '	; 文件系统类型, 必须 8个字节  

LABEL_START:	
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, 07c00h
 
	mov	ax, BootMessage 
	push 	BootMessage
	call	DispStr			; 调用显示字符串例程
	and 	sp, 2
	
	;; find file loader.bin
	
	;; FAT12 根目录区长度
	

	jmp	$			; 无限循环


DispStr:
	push	ax
	push	bp
	push	cx
	push	dx
	mov	bp,sp
	mov	ax, [bp+10] 
	mov	bp, ax			; ES:BP = 串地址
	mov	cx, 10			; CX = 串长度
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 000ch		; 页号为0(BH = 0) 黑底红字(BL = 0Ch,高亮)
	mov	dl, 0
	int	10h			; 10h 号中断
 
	pop	dx
	pop	cx
	pop	bp
	pop	ax
	ret 
	
	; 复位软驱
	xor ah, ah	
	xor dl, dl			;驱动器号 A盘
	int 13h
	
	; 读取软盘扇区数据
	mov ax, 19		; 
	mov bl, 18
	div bl
	inc ah	
	mov  cl, ah				;起始扇区号
	mov dh,  al			
	and dh, 1				;磁头号
	shr al, 1
	mov ch,	al			;磁道号
	mov ah, 02h	
	xor dl, dl			;驱动器号 A盘
	mov al, 1 			; 要读扇区数
	mov bx, 8000h
	int 13h
	
	mov si, LOADER_NAME
	mov di, bx
	.goon:
	dec al
	test al, al
	cmps	
	jz   .goon
	mov al, 11
	sub si, LOADER_NAME
	sub di, si
	mov si, LOADER_NAME
	add di, 32
	jmp  .goon


	jmp	$			; 无限循环

LOADER_NAME:  db "loader  bin"
BootMessage:		db	"Hello,FLY world!"
times 	510-($-$$)	db	0	; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 	0xaa55				; 结束标志
