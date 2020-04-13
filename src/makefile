KenerlEntry=0x100400   

QEMUTAG= -m 64M -fda run/os.img -hda run/hd.img -boot a 

AS=nasm
CC=gcc
LD=ld
ASFLAGS= -f elf -g -F stabs
CCFLAGS= -Wall -m32 -fno-stack-protector -c -I include -ggdb -gstabs+ -nostdinc -fno-builtin
LDFLAGS= -m elf_i386 -nostdlib -Ttext $(KenerlEntry)    

C_SRC= $(shell find . -name *.c)
C_OBJ= $(patsubst %.c, %.o, $(C_SRC))
S_SRC= $(shell find . -name *.s)
S_OBJ= $(patsubst %.s, %.o, $(S_SRC))

all:clean run/os.img run/kernel.bin.asm run/loader.bin.asm run/boot.bin.asm

.s.o: 
	$(AS) $(ASFLAGS) $< -o $@
.c.o:
	$(CC) $(CCFLAGS) $< -o $@ 

run/boot.bin:boot/boot.asm boot/fat12.inc boot/lib.inc
	$(AS) -I ./boot/ $< -o $@ 
run/loader.bin:boot/loader.asm boot/fat12.inc boot/lib.inc boot/pm.inc
	$(AS) -I ./boot/ $< -o $@ 
run/kernel.bin: $(S_OBJ) $(C_OBJ) 
	$(LD) $(LDFLAGS) $^ -o $@ 

run/boot.bin.asm:run/boot.bin   # 需要跳过FAT12 header
	ndisasm $< -o 0x7c3e -e 0x3e > $@ 
run/kernel.bin.asm:run/kernel.bin # 跳过GRUB头
	ndisasm $< -o $(KenerlEntry) -u -e 0x400 > $@ 
run/loader.bin.asm:run/loader.bin  # 参见 boot/lib.inc 文件g中哦男 LoaderBase和LoaderOffset的定义	
	ndisasm $< -o 0x2000 -u -e -0x100 > $@  

run/os.img:run/boot.bin run/loader.bin run/kernel.bin
	dd if=/dev/zero of=$@ bs=1024 count=1440  
	dd if=$< of=$@ conv=notrunc  
	sudo mount $@ /mnt/floppy -o loop
	sudo cp run/loader.bin /mnt/floppy  
	sudo cp run/kernel.bin /mnt/floppy  
	sudo umount /mnt/floppy

qemu:run/os.img
	qemu-system-i386 $(QEMUTAG)
debug:run/os.img
	qemu-system-i386 -S -s $(QEMUTAG) &
	sleep 5
	gdb --command=run/gdbcmd.txt
bochs:run/os.img
	bochs -q -f run/bochsrc -rc run/debug.txt
clean:
	rm -f run/*.bin run/*.o run/bochsout.txt run/os.img run/kernel.bin.asm $(S_OBJ) $(C_OBJ) 
