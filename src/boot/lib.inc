
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 从FAT12格式软盘根目录区查找文件 
;; 参数： 文件名字符串地址，共11字节，名称与扩展名间空格分开
;; 返回： AX=目录数据偏移地址,没有找到时AX=0
SearchFile:	 
	mov 	bp, sp  
	mov		ax, [BPB_RootEntCnt]
	mov		bl, DirEntCntPerSector
	div		bl						; AH为余数， AL为商
	test	ah, ah					
	jz		.1						; 余数为0
	inc		al	
	xor		ah, ah					; ax = 根目录区占用扇区数
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
	mov		cx, DirEntCntPerSector
.next_dir_entry:
	mov		dx, [ss:bp+2]
	push	dx
	push	si
	push	DIR_NAME_LEN
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
	mov		ax, 0
	jmp		.done
.find_loader:		
	mov		ax, si
	jmp		.done

.done:	
	ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 加载文件到内存
;; 参数1： word, 文件目录数据地址
;; 参赛2， word, 加载到内存地址，段基址
;; 参赛3， word, 加载到内存地址，段内偏移
LoadFile2Mem: 
	push 	ax
	push	bx
	push	cx
	push	dx
	push	bp
	mov 	bp, sp
	add		bp, 10

	mov		ax, [BPB_RootEntCnt]
	mov		bl, DirEntCntPerSector
	div		bl						; AH为余数， AL为商
	test	ah, ah					
	jz		.1						; 余数为0
	inc		al	
	xor		ah, ah					
.1:	
	mov		dx, ax					; DX = 根目录区占用扇区数
	mov		bx, [ss:bp+6]   		
	add		bx, DIR_FstClusOffset	
	mov		bx, [bx]				; BX = 开始簇号
 
	add		ax, bx
	add		ax, DirStartSectNo
	sub		ax, 2					; AX=开始扇区号
	mov		cx, [ss:bp+2]			; 段内偏移

.next:
	push 	1
	push	ax
	mov		ax, [ss:bp+4]
	push	ax				; 段基址	
	push	cx				; 段内偏移
	call	ReadSector
	add		sp, 8

	; bx=下一个簇号,
	push	bx
	call	NextClus
	pop		bx

	cmp		bx, 0FF7h			; 是否为最后一个扇区 
	jz 		.bad_data			; 簇号0FF7h,是坏簇 
	ja		.done				; 簇号大于或等于0xFF8,结束
	xor		ax, ax
	add		ax, dx
	add		ax, bx
	add		ax, DirStartSectNo
	sub		ax, 2		 		;ax = 扇区号
	add		cx, [BPB_BytsPerSec]
	jmp 	.next
.bad_data:

.done:
	pop		bp
	pop		dx
	pop		cx
	pop		bx
	pop 	ax
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FAT12中，根据当前簇号计算下一个簇号
;; 参数1：word，当前簇号
;; 返回：word，下一个簇号
NextClus:
	push 	bp
	push 	ax
	push	bx
	push	cx
	push	dx
	mov		bp, sp
	add		bp, 10
	
	mov		ax, [ss:bp+2]
	mov		cx, ax
	mov		bx, 3
	mul		bx
	shr		ax, 1		
	xor		dx, dx
	mov		bx, 512
	div		bx

	inc		ax
	push	2
	push	ax
	push	ds
	push	BUFF_ADDR
	call	ReadSector
	add		sp, 8

	mov		bx, dx
	add		bx, BUFF_ADDR
	mov 	ax, [ds: bx]

	test	cx, 01h
	jnz		.even      ; 奇数
	jmp		.done
.even: 
	shr		ax, 4
.done:
	and		ax, 0fffh
	mov		[ss:bp+2], ax
	pop		dx
	pop		cx
	pop		bx
	pop		ax
	pop 	bp
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

BUFF_ADDR			EQU	01000H 
LoaderBase			EQU	02000H    
LoaderOffset		EQU	0100h