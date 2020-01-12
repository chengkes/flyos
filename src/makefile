ASM= yasm 

run/os.img:run/boot.bin run/loader.bin run/kernel.bin
	dd if=$< of=$@ conv=notrunc
	sudo mount $@ /mnt/floppy -o loop
	sudo cp run/loader.bin /mnt/floppy
	sudo cp run/kernel.bin /mnt/floppy
	sudo umount /mnt/floppy

run/boot.bin:boot/boot.asm boot/fat12.inc boot/lib.inc
	$(ASM) $< -o $@ 

run/loader.bin:boot/loader.asm boot/fat12.inc boot/lib.inc boot/pm.inc
	$(ASM) $< -o $@ 

run/kernel.bin:run/kernel.o
	ld -m elf_i386 -s $< -o $@ 

run/kernel.o:kernel/kernel.asm
	$(ASM) -f elf $< -o $@

run:run/os.img
	bochs -q -f run/bochsrc -rc run/debug.txt

clean:
	rm -f run/*.bin run/*.o  run/bochsout.txt
