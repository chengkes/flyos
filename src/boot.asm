org	07c00h			; 告诉编译器程序加载到7c00处

	; 下面是 FAT12 磁盘的头
	jmp LABEL_START
	nop
	BS_OEMName		DB 'ForrestY'	; OEM String, 必须 8 个字节
	BPB_BytsPerSec	DW 512			; 每扇区字节数
	BPB_SecPerClus	DB 1			; 每簇多少扇区
	BPB_RsvdSecCnt	DW 1			; Boot 记录占用多少扇区
	BPB_NumFATs		DB 2			; 共有多少 FAT 表
	BPB_RootEntCnt	DW 224			; 根目录文件数最大值
	BPB_TotSec16	DW 2880			; 逻辑扇区总数
	BPB_Media		DB 0xF0			; 媒体描述符
	BPB_FATSz16		DW 9			; 每FAT扇区数
	BPB_SecPerTrk	DW 18			; 每磁道扇区数
	BPB_NumHeads	DW 2			; 磁头数(面数)
	BPB_HiddSec		DD 0			; 隐藏扇区数
	BPB_TotSec32	DD 0			; 如果 wTotalSectorCount 是 0 由这个值记录扇区数
	BS_DrvNum		DB 0			; 中断 13 的驱动器号
	BS_Reserved1	DB 0			; 未使用
	BS_BootSig		DB 29h			; 扩展引导标记 (29h)
	BS_VolID		DD 0			; 卷序列号
	BS_VolLab		DB 'Tinix0.01  '; 卷标, 必须 11 个字节
	BS_FileSysType	DB 'FAT12   '	; 文件系统类型, 必须 8个字节  

LABEL_START:	
	mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		sp, 07c00h
 
	push 	BootMessage
	push	0103h
	call	DispStr			; 调用显示字符串例程
	add 	sp, 4	
	
	; 复位软驱
    xor 	ah, ah
    xor 	dl, dl                  ;驱动器号A盘
    int 	13h

	; 读取软盘扇区数据
	; push	1			; 读1个扇区的数据
	; push	0			; 从0号扇区开始
	; push	ds			; 数据存放在DS：09000处
	; push	BUFF_ADDR
	; call	ReadSector		
	; add 	sp, 8	

	push 	LOADER_NAME
	push	0203h
	call	DispStr			; 调用显示字符串例程
	add 	sp, 4		

	; 从软盘中读取文件到内存
	push	LOADER_NAME
	push	ds			; 数据存放在DS：08000处
	push	LOADER_ADDR
	call	ReadFile
	add		sp, 6

	jmp		$			; 无限循环

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 显示字符串，字符串长度必须为10
;; 参数1：word，串地址
;; 参数2：word，显示位置，高位=行，低位=列
DispStr:
	push 	ax
	push	bx
	push	cx
	push	dx
	push	bp

	mov		bp, sp
	add		bp, 10
	mov		dx, [ss:bp+2]   	;(DH、DL)＝坐标(行、列)
	mov		bp, [ss:bp+4]   	; ES:BP = 串地址
	mov		cx, 10			; CX = 串长度
	mov		ax, 01301h		; AH = 13,  AL = 01h
	mov		bx, 000ch		; 页号为0(BH = 0) 黑底红字(BL = 0Ch,高亮)
	int		10h			; 10h 号中断 	

	pop		bp
	pop		dx
	pop		cx
	pop		bx
	pop		ax
	ret 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 从FAT12格式软盘中读取文件到内存
;; 参数1， word， 文件名，11个字节，文件名和扩展名用空格分开
;; 参赛2， word,  内存地址，段基址
;; 参赛3， word,  内存地址，段内偏移
ReadFile:
	; todo: 
	
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 读入软盘扇区数据到内存 
;; 参数1， word， 读取扇区数目, 低位有效
;; 参数2， word， 开始扇区号
;; 参赛3， word,  内存地址，段基址
;; 参赛4， word,  内存地址，段内偏移
ReadSector: 	 
	push 	ax
	push	bx
	push	cx
	push	dx
	push	bp
	push 	es

	mov		bp, sp		
	add		bp, 12
	; 根据开始扇区号计算柱面，磁头，扇区
	mov		ax, [ss:bp+6]
	mov		bx, [BPB_SecPerTrk]
	div		bl
	inc		ah
	mov     cl, ah            ; 扇区
	mov		dh, al
	and		dh, 01h		  ; 磁头
	mov		ch, al
	shr		ch, 1             ; 柱面

	mov		bx, [ss:bp+2]	; ES:BX=缓冲区的地址  
	mov		es, [ss:bp+4]	; ES:BX=缓冲区的地址 	
	xor     dl,dl           ; 驱动器号
	     
.next: 	
	mov		al, [ss:bp+8]	; AL=读取扇区数目
	mov     ah, 02h         ; 功能号，读扇区  
	int     13h	
	jc		.next		;  出错后再次尝试		 
	
	pop		es
	pop		bp
	pop		dx	
	pop		cx
	pop		bx
	pop		ax
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

BUFF_ADDR			EQU	09000H
LOADER_ADDR			EQU	08000H
LOADER_NAME: 		db 	"loader  bin"
BootMessage:		db	"Hello,FLY !"
times 	510-($-$$)	db	0	; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 	0xaa55					; 结束标志

