KenerlEntry=0x30400  # 必须与boot/loader.asm中一致

ASMFLAGS= -f elf -g -F stabs
GCCFLAGS= -Wall -m32 -fno-stack-protector -c -I include -ggdb -gstabs+ -nostdinc -fno-builtin
LDFLAGS=  -m elf_i386 -nostdlib -Ttext $(KenerlEntry)    

C_SRC= $(shell find . -name *.c)
C_OBJ= $(patsubst %.c, %.o, $(C_SRC))
S_SRC= $(shell find . -name *.s)
S_OBJ= $(patsubst %.s, %.o, $(S_SRC))

.s.o: 
	nasm $(ASMFLAGS) $< -o $@
.c.o:
	gcc $(GCCFLAGS) $< -o $@ s

run/boot.bin:boot/boot.asm boot/fat12.inc boot/lib.inc
	nasm -I ./boot/ $< -o $@ 
run/loader.bin:boot/loader.asm boot/fat12.inc boot/lib.inc boot/pm.inc
	nasm -I ./boot/ $< -o $@ 
run/kernel.bin: $(S_OBJ) $(C_OBJ) 
	ld $(LDFLAGS) $^ -o $@ 

run/kernel.bin.asm:run/kernel.bin 
	ndisasm $< -o $(KenerlEntry) -u -e 0x400 > $@ 

run/os.img:run/boot.bin run/loader.bin run/kernel.bin run/kernel.bin.asm
	dd if=/dev/zero of=$@ bs=1024 count=1440
	dd if=$< of=$@ conv=notrunc
	sudo mount $@ /mnt/floppy -o loop
	sudo cp run/loader.bin /mnt/floppy
	sudo cp run/kernel.bin /mnt/floppy
	sudo umount /mnt/floppys

clean:
	rm -f run/*.bin run/*.o run/bochsout.txt run/os.img run/kernel.bin.asm $(S_OBJ) $(C_OBJ) 

img:run/os.img

qemu:run/os.img
	qemu-system-i386 -fda run/os.img -boot a
debug:run/os.img
	qemu-system-i386 -S -s -fda run/os.img -boot a &
	sleep 1
	cgdb --command=run/gdbcmd.txt

run:run/os.img     
	bochs -q -f run/bochsrc -rc run/debug.txt
runwin:run/os.img   # for windows
	bochsdbg -q -f run/bochsrc.txt -rc run/debug.txt
