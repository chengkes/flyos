KenerlEntry=0x30400  # 必须与boot/loader.asm中一致

QEMUTAG= -fda run/os.img -hda run/hd.img -boot a 

AS=nasm
CC=gcc
LD=ld
ASFLAGS= -f elf -g -F stabs
CCFLAGS= -Wall -m32 -fno-stack-protector -c -I include -ggdb -gstabs+ -nostdinc -fno-builtin
LDFLAGS=  -m elf_i386 -nostdlib -Ttext $(KenerlEntry)    

C_SRC= $(shell find . -name *.c)
C_OBJ= $(patsubst %.c, %.o, $(C_SRC))
S_SRC= $(shell find . -name *.s)
S_OBJ= $(patsubst %.s, %.o, $(S_SRC))

img:run/os.img

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

run/kernel.bin.asm:run/kernel.bin 
	ndisasm $< -o $(KenerlEntry) -u -e 0x400 > $@ 

run/os.img:run/boot.bin run/loader.bin run/kernel.bin run/kernel.bin.asm
	dd if=/dev/zero of=$@ bs=1024 count=1440
	dd if=$< of=$@ conv=notrunc
	sudo mount $@ /mnt/floppy -o loop
	sudo cp run/loader.bin /mnt/floppy
	sudo cp run/kernel.bin /mnt/floppy
	sudo umount /mnt/floppy

clean:
	rm -f run/*.bin run/*.o run/bochsout.txt run/os.img run/kernel.bin.asm $(S_OBJ) $(C_OBJ) 

qemu:run/os.img
	qemu-system-i386 $(QEMUTAG)
debug:run/os.img
	qemu-system-i386 -S -s $(QEMUTAG) &
	sleep 1
	cgdb --command=run/gdbcmd.txt
run:run/os.img     
	bochs -q -f run/bochsrc -rc run/debug.txt
runwin:run/os.img   # for windows
	bochsdbg -q -f run/bochsrc.txt -rc run/debug.txt
