ASM= yasm 


run/os.img:run/boot.bin run/loader.bin
	dd if=$< of=$@ conv=notrunc
	sudo mount $@ /mnt/floppy -o loop
	sudo cp run/loader.bin /mnt/floppy
	sudo umount /mnt/floppy
run/boot.bin:boot.asm
	$(ASM) $< -o $@ 
run/loader.bin:loader.asm
	$(ASM) $< -o $@ 

run:run/os.img
	bochs -q -f run/bochsrc -rc run/debug.txt

clean:
	rm -f run/*.bin  run/bochsout.txt
