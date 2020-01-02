ASM= yasm 

run:run/os.img
	bochs -q -f run/bochsrc -rc run/debug.txt

run/os.img:run/boot.bin
	dd if=$< of=$@ conv=notrunc
run/boot.bin:boot.asm
	$(ASM) $< -o $@ 

img:run/os.img

clean:
	rm -f run/*.bin
