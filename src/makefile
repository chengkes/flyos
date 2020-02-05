ASM=yasm 
KenerlEntry=0x30400			# ; 必须与boot/loader.asm中一致

run/os.img:run/boot.bin run/loader.bin  run/kernel.bin run/kernel.bin.asm
	dd if=/dev/zero of=$@ bs=1024 count=1440
	dd if=$< of=$@ conv=notrunc
	sudo mount $@ /mnt/floppy -o loop
	sudo cp run/loader.bin /mnt/floppy
	sudo cp run/kernel.bin /mnt/floppy
	sudo umount /mnt/floppy

run/boot.bin:boot/boot.asm boot/fat12.inc boot/lib.inc
	$(ASM)   $< -o $@ 

run/loader.bin:boot/loader.asm boot/fat12.inc boot/lib.inc boot/pm.inc
	$(ASM)   $< -o $@ 

run/kernel.bin:run/kernel.o run/klibc.o
	ld -m elf_i386 -s -Ttext $(KenerlEntry) $^ -o $@ 

run/kernel.o:kernel/kernel.asm kernel/klibc.c
	$(ASM)  -f elf $< -o $@

run/klibc.o:kernel/klibc.c kernel/kernel.asm
	gcc -m32 -fno-stack-protector -c $< -o $@ 

run/kernel.bin.asm:run/kernel.bin 
	ndisasm $<  -o $(KenerlEntry) -u -e 0x400 > $@ 

clean:
	rm -f run/*.bin run/*.o  run/bochsout.txt run/os.img

compile:run/boot.bin run/loader.bin  run/kernel.bin run/kernel.bin.asm

run:run/os.img
	bochs -q -f run/bochsrc -rc run/debug.txt

runwin:run/os.img   # for windows
	bochsdbg -q -f run/bochsrc.txt -rc run/debug.txt
