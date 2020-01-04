ASM= yasm 


run/os.img:run/boot.bin
	dd if=$< of=$@ conv=notrunc
run/boot.bin:boot.asm
	$(ASM) $< -o $@ 

run:run/os.img
	bochs -q -f run/bochsrc -rc run/debug.txt

clean:
	rm -f run/*.bin  run/bochsout.txt
