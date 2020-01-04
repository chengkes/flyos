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
	
	; 从软盘中查找文件
	push	NoLoader
	call	SearchFile
	pop		ax
	test	ax, ax
	jz		.not_found
	; todo
	jmp 	.done
.not_found:
	push	NoLoader
	push	0201h
	call 	DispStr
	add		sp, 4
.done:
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
;; 从FAT12格式软盘根目录区 查找文件 
;; 参数1， word， 文件名，11个字节，文件名和扩展名用空格分开
;; 返回： 目录数据偏移地址
SearchFile:
	push	ax
	push	bx
	push	cx
	push	dx
	push	bp
	push	si	 
	mov 	bp, sp 
	add		bp, 12

	mov		ax, [BPB_RootEntCnt]
	mov		bl, EntCntPerSector
	div		bl						; AH为余数， AL为商
	test	ah, ah					
	jz		.1						; 余数为0
	inc		al	
	xor		ah, ah
.1:
	mov		bx, DirStartSectNo		; FAT12根目录起始扇区号
.next_sector:
	; 读取扇数据
	push	1			; 读1个扇区的数据
	push	bx			  
	push	ds			  
	push	BUFF_ADDR
	call	ReadSector		
	add 	sp, 8	

	mov		si, BUFF_ADDR
	mov		cx, EntCntPerSector
.next_dir_entry:
	mov		dx,[ss:bp+2]
	push	dx
	push	si
	push	LOADER_NAME_LEN
	call	CmpStr
	add		sp, 4
	pop		dx
	test	dx, dx
	jz		.find_loader		; 找到loader
	; 没有找到loader， 查看下一个根目录
	add		si, DIR_ENTRY_SIZE 
	loop	.next_dir_entry

	inc		bx
	dec		ax
	test 	ax, ax
	jnz	.next_sector

.not_found:	
	mov		word [ss:bp+2], 0
	jmp		.done
.find_loader:		
	mov		word [ss:bp+2], si
	jmp		.done

.done:
	pop		si
	pop		bp
	pop		dx
	pop		cx
	pop		bx
	pop		ax	
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 字符串比较
;; 参数1， word, 第1个字符串地址
;; 参赛2， word, 第2个子讽刺地址
;; 参赛3， word, 字符串长度
;; 结果： 相同时返回0 
CmpStr:
	push	bp
	push	si
	push	di
	push	cx
	
	mov 	bp, sp
	add		bp, 8	
	mov		cx, [ss:bp+2]
	mov		si, [ss:bp+4]
	mov		di, [ss:bp+6]
	cld
	repe 	cmpsb 
	
	mov		[ss:bp+6], cx
	pop		cx
	pop		di
	pop		si
	pop		bp
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
	and		dh, 01h		  	; 磁头
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
LOADER_NAME_LEN		EQU	11
DIR_ENTRY_SIZE		EQU	32				; 每个根目录数据占用字节数
EntCntPerSector		EQU	16				; 每个扇区目录数目
DirStartSectNo		EQU	19				; 根目录数据起始扇区号 ，1+	BPB_NumFATs * BPB_FATSz16	
DIR_FileSizeOffset	EQU 1ch				; 根目录数据中文件大小地址偏移
DIR_FstClusOffset	equ 1ah				; 根目录数据中开始簇号地址偏移

LoaderName: 		db 	"LOADER  BIN"
NoLoader: 			db 	"no loader  "
BootMessage:		db	"Hello,FLY !"
times 	510-($-$$)	db	0	; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 	0xaa55					; 结束标志

