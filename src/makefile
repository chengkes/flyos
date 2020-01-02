ASM= yasm 

run/os.img:run/boot.bin
	dd if=$< of=$@ conv=notrunc
run/boot.bin:boot.asm
	$(ASM) $< -o $@ 


clean:
	rm -f run/*.bin
